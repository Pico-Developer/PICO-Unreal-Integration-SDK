#include "ECS/PICOSpatialECSTextComponent.h"

#include "Engine/Texture2D.h"
#include "PICOSpatialFunctionLibrary.h"
#include "UObject/Package.h"

#if PLATFORM_ANDROID
#include "PICOSpatial.h"
#include "PICOSpatialUtility.h"

static spatialpack::HorizontalAlignment ToSpHorizontalAlignment(EPICOSpatialTextHorizontalAlignment InValue)
{
	switch (InValue)
	{
	case EPICOSpatialTextHorizontalAlignment::Left:
		return spatialpack::HorizontalAlignment::LEFT;
	case EPICOSpatialTextHorizontalAlignment::Center:
		return spatialpack::HorizontalAlignment::CENTER;
	case EPICOSpatialTextHorizontalAlignment::Right:
		return spatialpack::HorizontalAlignment::RIGHT;
	case EPICOSpatialTextHorizontalAlignment::Justify:
		return spatialpack::HorizontalAlignment::JUSTIFY;
	case EPICOSpatialTextHorizontalAlignment::Flush:
		return spatialpack::HorizontalAlignment::FLUSH;
	default:
		return spatialpack::HorizontalAlignment::LEFT;
	}
}

static spatialpack::VerticalAlignment ToSpVerticalAlignment(EPICOSpatialTextVerticalAlignment InValue)
{
	switch (InValue)
	{
	case EPICOSpatialTextVerticalAlignment::Top:
		return spatialpack::VerticalAlignment::TOP;
	case EPICOSpatialTextVerticalAlignment::Center:
		return spatialpack::VerticalAlignment::CENTER;
	case EPICOSpatialTextVerticalAlignment::Bottom:
		return spatialpack::VerticalAlignment::BOTTOM;
	default:
		return spatialpack::VerticalAlignment::CENTER;
	}
}

static spatialpack::HorizontalOverflowMode ToSpHorizontalOverflowMode(EPICOSpatialTextHorizontalOverflowMode InValue)
{
	switch (InValue)
	{
	case EPICOSpatialTextHorizontalOverflowMode::Wrap:
		return spatialpack::HorizontalOverflowMode::WRAP;
	case EPICOSpatialTextHorizontalOverflowMode::Clip:
		return spatialpack::HorizontalOverflowMode::CLIP;
	case EPICOSpatialTextHorizontalOverflowMode::Ellipsis:
		return spatialpack::HorizontalOverflowMode::ELLIPSIS;
	case EPICOSpatialTextHorizontalOverflowMode::Overflows:
		return spatialpack::HorizontalOverflowMode::OVERFLOWS;
	default:
		return spatialpack::HorizontalOverflowMode::WRAP;
	}
}

static spatialpack::VerticalOverflowMode ToSpVerticalOverflowMode(EPICOSpatialTextVerticalOverflowMode InValue)
{
	switch (InValue)
	{
	case EPICOSpatialTextVerticalOverflowMode::Clip:
		return spatialpack::VerticalOverflowMode::CLIP;
	case EPICOSpatialTextVerticalOverflowMode::Overflows:
		return spatialpack::VerticalOverflowMode::OVERFLOWS;
	default:
		return spatialpack::VerticalOverflowMode::CLIP;
	}
}

static spatialpack::Vector4f ToSpVector4f(const FVector4& InValue)
{
	return spatialpack::Vector4f{ static_cast<float>(InValue.X), static_cast<float>(InValue.Y), static_cast<float>(InValue.Z), static_cast<float>(InValue.W) };
}
#endif

void UPICOSpatialECSTextComponent::setTextWidth(float InTextWidth)
{
	TextWidth = InTextWidth;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setTextWidth(TextWidth / 100.0f);
	}
#endif
}

void UPICOSpatialECSTextComponent::setTextHeight(float InTextHeight)
{
	TextHeight = InTextHeight;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setTextHeight(TextHeight / 100.0f);
	}
#endif
}

void UPICOSpatialECSTextComponent::setText(const FString& InText)
{
	Text = FText::FromString(InText);
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setText(TCHAR_TO_UTF8(*InText));
	}
#endif
}

void UPICOSpatialECSTextComponent::setFontSize(int32 InFontSize)
{
	FontSize = InFontSize;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setFontSize(static_cast<uint32_t>(FMath::Max(FontSize, 0)));
	}
#endif
}

void UPICOSpatialECSTextComponent::setTextColor(const FLinearColor& InTextColor)
{
	TextColor = InTextColor;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setTextColor(ToSpColor(TextColor));
	}
#endif
}

void UPICOSpatialECSTextComponent::setUnderLine(bool bInUnderLine)
{
	bUnderline = bInUnderLine;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setUnderLine(bUnderline);
	}
#endif
}

void UPICOSpatialECSTextComponent::setBackgroundResource(UTexture2D* InBackgroundTexture)
{
	BackgroundTexture = InBackgroundTexture;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		spatialpack::TextComponent TextComponent = GetCachedNativeEntity().getOrAddTextComponent();
		if (UTexture2D* Texture = BackgroundTexture.Get())
		{
			spatialpack::TextureResource* TextureResource = FPICOSpatial::Get().CacheTextureResource(Texture->GetPackage()->GetPathName());
			if (TextureResource && TextureResource->isValid())
			{
				TextComponent.setBackgroundResource(*TextureResource);
			}
			else
			{
				TextComponent.setBackgroundResource(spatialpack::TextureResource());
				UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Text] Background texture native resource invalid. Obj=%s Texture=%s"),
					*GetName(),
					*Texture->GetPathName());
			}
		}
		else
		{
			TextComponent.setBackgroundResource(spatialpack::TextureResource());
		}
	}
#endif
}

void UPICOSpatialECSTextComponent::setBackgroundColor(const FLinearColor& InBackgroundColor)
{
	BackgroundColor = InBackgroundColor;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setBackgroundColor(ToSpColor(BackgroundColor));
	}
#endif
}

void UPICOSpatialECSTextComponent::setBackgroundCornerRadius(float InBackgroundCornerRadius)
{
	BackgroundCornerRadius = InBackgroundCornerRadius;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setBackgroundCornerRadius(BackgroundCornerRadius / 100.0f);
	}
#endif
}

void UPICOSpatialECSTextComponent::setHorizontalAlignment(EPICOSpatialTextHorizontalAlignment InHorizontalAlignment)
{
	HorizontalAlignment = InHorizontalAlignment;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setHorizontalAlignment(ToSpHorizontalAlignment(HorizontalAlignment));
	}
#endif
}

void UPICOSpatialECSTextComponent::setVerticalAlignment(EPICOSpatialTextVerticalAlignment InVerticalAlignment)
{
	VerticalAlignment = InVerticalAlignment;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setVerticalAlignment(ToSpVerticalAlignment(VerticalAlignment));
	}
#endif
}

void UPICOSpatialECSTextComponent::setHorizontalOverflowMode(EPICOSpatialTextHorizontalOverflowMode InHorizontalOverflowMode)
{
	HorizontalOverflowMode = InHorizontalOverflowMode;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setHorizontalOverflowMode(ToSpHorizontalOverflowMode(HorizontalOverflowMode));
	}
#endif
}

void UPICOSpatialECSTextComponent::setVerticalOverflowMode(EPICOSpatialTextVerticalOverflowMode InVerticalOverflowMode)
{
	VerticalOverflowMode = InVerticalOverflowMode;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setVerticalOverflowMode(ToSpVerticalOverflowMode(VerticalOverflowMode));
	}
#endif
}

void UPICOSpatialECSTextComponent::setEdgeInsets(const FVector4& InEdgeInsets)
{
	EdgeInsets = InEdgeInsets;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setEdgeInsets(ToSpVector4f(EdgeInsets / 100.0f));
	}
#endif
}

void UPICOSpatialECSTextComponent::setRendererEnabled(bool bInRendererEnabled)
{
	bRendererEnabled = bInRendererEnabled;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		GetCachedNativeEntity().getOrAddTextComponent().setRendererEnabled(bRendererEnabled);
	}
#endif
}

#if PLATFORM_ANDROID
void UPICOSpatialECSTextComponent::OnInitializeNativeComponent(spatialpack::SceneEntity& InNativeEntity)
{
	UE_LOG(LogPICOSpatial, Verbose, TEXT("[ECS][Text] OnInitializeNativeComponent UUID=%llu"), (uint64)InNativeEntity.getEntityUUID());
	InNativeEntity.getOrAddTextComponent();
}

void UPICOSpatialECSTextComponent::ApplyAllPropertiesToNative()
{
	if (!HasNativeEntity())
	{
		return;
	}

	UE_LOG(LogPICOSpatial, Log, TEXT("[ECS][Text] ApplyAllPropertiesToNative UUID=%llu Text='%s' W=%f H=%f Font=%d Renderer=%d"),
		(uint64)GetCachedNativeEntity().getEntityUUID(),
		*Text.ToString(),
		TextWidth,
		TextHeight,
		FontSize,
		bRendererEnabled ? 1 : 0);

	spatialpack::TextComponent TextComponent = GetCachedNativeEntity().getOrAddTextComponent();
	TextComponent.setTextWidth(TextWidth / 100.0f);
	TextComponent.setTextHeight(TextHeight / 100.0f);
	TextComponent.setText(TCHAR_TO_UTF8(*Text.ToString()));
	TextComponent.setFontSize(static_cast<uint32_t>(FMath::Max(FontSize, 0)));
	TextComponent.setTextColor(ToSpColor(TextColor));
	TextComponent.setUnderLine(bUnderline);
	TextComponent.setBackgroundColor(ToSpColor(BackgroundColor));
	TextComponent.setBackgroundCornerRadius(BackgroundCornerRadius / 100.0f);
	TextComponent.setHorizontalAlignment(ToSpHorizontalAlignment(HorizontalAlignment));
	TextComponent.setVerticalAlignment(ToSpVerticalAlignment(VerticalAlignment));
	TextComponent.setHorizontalOverflowMode(ToSpHorizontalOverflowMode(HorizontalOverflowMode));
	TextComponent.setVerticalOverflowMode(ToSpVerticalOverflowMode(VerticalOverflowMode));
	TextComponent.setEdgeInsets(ToSpVector4f(EdgeInsets / 100.0f));
	TextComponent.setRendererEnabled(bRendererEnabled);

	if (UTexture2D* Texture = BackgroundTexture.Get())
	{
		spatialpack::TextureResource* TextureResource = FPICOSpatial::Get().CacheTextureResource(Texture->GetPackage()->GetPathName());
		if (TextureResource && TextureResource->isValid())
		{
			TextComponent.setBackgroundResource(*TextureResource);
		}
		else
		{
			TextComponent.setBackgroundResource(spatialpack::TextureResource());
			UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Text] Background texture native resource invalid during apply. Obj=%s Texture=%s"),
				*GetName(),
				*Texture->GetPathName());
		}
	}
	else
	{
		TextComponent.setBackgroundResource(spatialpack::TextureResource());
	}
}
#endif

float UPICOSpatialECSTextComponent::getTextWidth() const
{
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		spatialpack::SceneEntity Entity = GetCachedNativeEntity();
		return Entity.getOrAddTextComponent().getTextWidth() * 100.0f;
	}
#endif
	return TextWidth;
}

float UPICOSpatialECSTextComponent::getTextHeight() const
{
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		spatialpack::SceneEntity Entity = GetCachedNativeEntity();
		return Entity.getOrAddTextComponent().getTextHeight() * 100.0f;
	}
#endif
	return TextHeight;
}

FString UPICOSpatialECSTextComponent::getText() const
{
	return Text.ToString();
}

int32 UPICOSpatialECSTextComponent::getFontSize() const
{
	return FontSize;
}

FLinearColor UPICOSpatialECSTextComponent::getTextColor() const
{
	return TextColor;
}

bool UPICOSpatialECSTextComponent::getUnderLine() const
{
	return bUnderline;
}

UTexture2D* UPICOSpatialECSTextComponent::getBackgroundResource() const
{
	return BackgroundTexture.Get();
}

FLinearColor UPICOSpatialECSTextComponent::getBackgroundColor() const
{
	return BackgroundColor;
}

float UPICOSpatialECSTextComponent::getBackgroundCornerRadius() const
{
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		spatialpack::SceneEntity Entity = GetCachedNativeEntity();
		return Entity.getOrAddTextComponent().getBackgroundCornerRadius() * 100.0f;
	}
#endif
	return BackgroundCornerRadius;
}

EPICOSpatialTextHorizontalAlignment UPICOSpatialECSTextComponent::getHorizontalAlignment() const
{
	return HorizontalAlignment;
}

EPICOSpatialTextVerticalAlignment UPICOSpatialECSTextComponent::getVerticalAlignment() const
{
	return VerticalAlignment;
}

EPICOSpatialTextHorizontalOverflowMode UPICOSpatialECSTextComponent::getHorizontalOverflowMode() const
{
	return HorizontalOverflowMode;
}

EPICOSpatialTextVerticalOverflowMode UPICOSpatialECSTextComponent::getVerticalOverflowMode() const
{
	return VerticalOverflowMode;
}

FVector4 UPICOSpatialECSTextComponent::getEdgeInsets() const
{
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		spatialpack::SceneEntity Entity = GetCachedNativeEntity();
		const spatialpack::Vector4f Insets = Entity.getOrAddTextComponent().getEdgeInsets();
		return FVector4(Insets.x * 100.0f, Insets.y * 100.0f, Insets.z * 100.0f, Insets.w * 100.0f);
	}
#endif
	return EdgeInsets;
}

bool UPICOSpatialECSTextComponent::getRendererEnabled() const
{
	return bRendererEnabled;
}
