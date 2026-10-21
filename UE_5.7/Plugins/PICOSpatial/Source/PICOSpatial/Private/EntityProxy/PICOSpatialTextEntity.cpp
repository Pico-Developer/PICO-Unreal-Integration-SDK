// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialTextEntity.h"
#include "Engine/Font.h"

void FTextEntityProxy::ConstructTextEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	DirtyStates.Init(false, 7);

	if (bInitialize)
	{
		bLastVisibleState = bVisible;
	}

	if (bVisible)
	{
		if (!bLastVisibleState)
		{
			bTransformDirty = true; // During the invisible time, the transform may be changed, but we have missed the update, so we need to force update the transform when it becomes visible again.
		}
		bLastVisibleState = true;
	}
	else
	{
		bLastVisibleState = false;
		return;
	}
	if (const UTextRenderComponent* TextComponent = Cast<UTextRenderComponent>(SceneComponent))
	{
		const FText& NewText = TextComponent->Text;
		const spatialpack::HorizontalAlignment& NewHorizontalAlignment = HorizontalAlignmentConvert(TextComponent->HorizontalAlignment);
		const spatialpack::VerticalAlignment& NewVerticalAlignment = VerticalAlignmentConvert(TextComponent->VerticalAlignment);
		const spatialpack::Color4f& NewTextRenderColor = ToSpColor(TextComponent->TextRenderColor);

		DirtyStates[0] = !Text.EqualTo(NewText);
		DirtyStates[1] = HorizontalAlignment != NewHorizontalAlignment;
		DirtyStates[2] = VerticalAlignment != NewVerticalAlignment;
		DirtyStates[3] = IsColorDirty(TextRenderColor, NewTextRenderColor);
		
		Text = NewText;
		HorizontalAlignment = NewHorizontalAlignment;
		VerticalAlignment = NewVerticalAlignment;
		TextRenderColor = NewTextRenderColor;

		if (const UFont* Font = TextComponent->Font.Get())
		{
			const float& NewHeight = Font->GetStringHeightSize( * (TextComponent->Text.ToString())) * TextComponent->WorldSize * TextComponent->InvDefaultSize * TextComponent->YScale;
			const float& NewWidth = Font->GetStringSize( * (TextComponent->Text.ToString())) * TextComponent->WorldSize * TextComponent->InvDefaultSize * TextComponent->XScale;
			const uint32& NewFontSize = NewHeight;
			DirtyStates[4] = Height != NewHeight;
			DirtyStates[5] = Width != NewWidth;
			DirtyStates[6] = FontSize != NewFontSize;

			Height = NewHeight;
			Width = NewWidth;
			FontSize = NewFontSize;
		}	
	}

	if (!bInitialize)
	{
		bool bOffsetTransform = false;

		if (bTransformDirty)
		{
			StoredTransform = Transform;
			bOffsetTransform = true;
		}
		else if (DirtyStates[4] || DirtyStates[5])
		{
			bOffsetTransform = true;
		}

		if (bOffsetTransform)
		{
			if (!bTransformDirty)
			{
				Transform = StoredTransform;
				bTransformDirty = true;
			}

			switch (HorizontalAlignment)
			{
			case spatialpack::HorizontalAlignment::LEFT:
				Transform.SetLocation(Transform.GetLocation() + Transform.Rotator().RotateVector(FVector(0, Width * VHScale / -2.0f, 0)));
				break;
			case spatialpack::HorizontalAlignment::RIGHT:
				Transform.SetLocation(Transform.GetLocation() + Transform.Rotator().RotateVector(FVector(0, Width * VHScale / 2.0f, 0)));
				break;
			}
			switch (VerticalAlignment)
			{
			case spatialpack::VerticalAlignment::TOP:
				Transform.SetLocation(Transform.GetLocation() + Transform.Rotator().RotateVector(FVector(0, 0, Height * VHScale / -2.0f)));
				break;
			case spatialpack::VerticalAlignment::BOTTOM:
				Transform.SetLocation(Transform.GetLocation() + Transform.Rotator().RotateVector(FVector(0, 0, Height * VHScale / 2.0f)));
				break;
			}
			RotateTransform(Transform, FRotator(0, -90, 0));
			Transform.SetScale3D(FVector(Transform.GetScale3D().Y, Transform.GetScale3D().X, Transform.GetScale3D().Z));
		}
	}
}

FTextEntityProxy::FTextEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	: FPrimitiveEntityProxy(InParentEntityProxy, SceneComponent)
	, StoredTransform(Transform)
{
	ConstructTextEntityData(SceneComponent, true);
}

bool FTextEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FPrimitiveEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
		if (NativeEntity.Get()->IsValid())
		{
			if (bTransformDirty)
			{
				switch (HorizontalAlignment)
				{
				case spatialpack::HorizontalAlignment::LEFT:
					Transform.SetLocation(Transform.GetLocation() + Transform.Rotator().RotateVector(FVector(0, Width * VHScale * Transform.GetScale3D().Y / -2.0f, 0)));
					break;
				case spatialpack::HorizontalAlignment::RIGHT:
					Transform.SetLocation(Transform.GetLocation() + Transform.Rotator().RotateVector(FVector(0, Width * VHScale * Transform.GetScale3D().Y / 2.0f, 0)));
					break;
				}
				switch (VerticalAlignment)
				{
				case spatialpack::VerticalAlignment::TOP:
					Transform.SetLocation(Transform.GetLocation() + Transform.Rotator().RotateVector(FVector(0, 0, Height * VHScale * Transform.GetScale3D().Z / -2.0f)));
					break;
				case spatialpack::VerticalAlignment::BOTTOM:
					Transform.SetLocation(Transform.GetLocation() + Transform.Rotator().RotateVector(FVector(0, 0, Height * VHScale * Transform.GetScale3D().Z / 2.0f)));
					break;
				}
				RotateTransform(Transform, FRotator(0, -90, 0));
				Transform.SetScale3D(FVector(Transform.GetScale3D().Y, Transform.GetScale3D().X, Transform.GetScale3D().Z));
			}
			return true;
		}
	}
	return false;
}

void FTextEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FPrimitiveEntityProxy::UpdateEntityProxy(SceneComponent);
	ConstructTextEntityData(SceneComponent, false);
}

void FTextEntityProxy::UpdateNativeEntity()
{
	FPrimitiveEntityProxy::UpdateNativeEntity();
#if PLATFORM_ANDROID
	spatialpack::SceneEntity entity = NativeEntity.Get()->entity;
	if (!entity.isValid())
	{
		return;
	}

	if (DirtyStates[0])
	{
		entity.getOrAddTextComponent().setText(TCHAR_TO_UTF8( * Text.ToString()));
	}

	if (DirtyStates[1])
	{
		entity.getOrAddTextComponent().setHorizontalAlignment(HorizontalAlignment);
	}

	if (DirtyStates[2])
	{
		entity.getOrAddTextComponent().setVerticalAlignment(VerticalAlignment);
	}

	if (DirtyStates[3])
	{
		entity.getOrAddTextComponent().setTextColor(TextRenderColor);
	}

	if (DirtyStates[4])
	{
		entity.getOrAddTextComponent().setTextHeight(Height * VHScale / 100.0f);
	}

	if (DirtyStates[5])
	{
		entity.getOrAddTextComponent().setTextWidth(Width * VHScale / 100.0f);
	}

	if (DirtyStates[6])
	{
		entity.getOrAddTextComponent().setFontSize(FontSize * FontSizeScale);
	}

	if (bVisible != entity.getOrAddTextComponent().isRendererEnabled())
	{
		entity.getOrAddTextComponent().setRendererEnabled(bVisible);
	}
#endif // PLATFORM_ANDROID
}

spatialpack::HorizontalAlignment FTextEntityProxy::HorizontalAlignmentConvert(const EHorizTextAligment& HAligment)
{
	switch (HAligment)
	{
	case EHTA_Left:
		return spatialpack::HorizontalAlignment::LEFT;
		break;
	case EHTA_Center:
		return spatialpack::HorizontalAlignment::CENTER;
		break;
	case EHTA_Right:
		return spatialpack::HorizontalAlignment::RIGHT;
		break;
	default:
		return spatialpack::HorizontalAlignment::LEFT;
		break;
	}
}

spatialpack::VerticalAlignment FTextEntityProxy::VerticalAlignmentConvert(const EVerticalTextAligment& VAligment)
{
	switch (VAligment)
	{
	case EVRTA_TextTop:
		return spatialpack::VerticalAlignment::TOP;
		break;
	case EVRTA_TextCenter:
		return spatialpack::VerticalAlignment::CENTER;
		break;
	case EVRTA_TextBottom:
		return spatialpack::VerticalAlignment::BOTTOM;
		break;
	default:
		return spatialpack::VerticalAlignment::BOTTOM;
		break;
	}
}

bool FTextEntityProxy::IsColorDirty(const spatialpack::Color4f& ColorA, const spatialpack::Color4f& ColorB)
{
	return !((ColorA.r == ColorB.r) && (ColorA.g == ColorB.g) && (ColorA.b == ColorB.b) && (ColorA.a == ColorB.a));
}
