// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialSceneCaptureEntity.h"
#include "Engine/TextureRenderTarget2D.h"
#include "CoreGlobals.h"

namespace
{
	FORCEINLINE void ApplySceneCaptureNativeTransformCorrection(FTransform& InTransform)
	{
		InTransform.SetRotation(InTransform.GetRotation() * FRotator(0, 90, 0).Quaternion());
		InTransform.SetScale3D(FVector(InTransform.GetScale3D().Y, InTransform.GetScale3D().X, InTransform.GetScale3D().Z));
	}
}

void FSceneCaptureEntityProxy::ConstructSceneCaptureEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	USceneCaptureComponent2D* SceneCaptureComponent2D = Cast<USceneCaptureComponent2D>(SceneComponent);
	if (SceneCaptureComponent2D == nullptr)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[SceneCaptureProxy] ConstructSceneCaptureEntityData skipped: component is not USceneCaptureComponent2D. ProxyName=%s Component=%s"),
			*Name,
			SceneComponent ? *SceneComponent->GetName() : TEXT("<null>"));
		return;
	}

	const bool bPreviousIsCapturing = bIsCapturing;
	const bool bPreviousShowDebugPlane = bShowDebugPlane;
	const bool bPreviousManualCaptureOneFrame = bManualCaptureOneFrame;
	const bool bPreviousCaptureEveryFrame = bCaptureEveryFrame;
	const bool bPreviousCaptureOnMove = bCaptureOnMove;
	const int PreviousX = X;
	const int PreviousY = Y;

	if (bInitialize)
	{
	}
	else
	{
		if (bTransformDirty)
		{
			ApplySceneCaptureNativeTransformCorrection(Transform);
		}
	}

	if (SceneCaptureComponent2D->TextureTarget)
	{
		X = SceneCaptureComponent2D->TextureTarget->SizeX;
		Y = SceneCaptureComponent2D->TextureTarget->SizeY;
		Ratio = (float)X / (float)Y;
	}

	FOV = SceneCaptureComponent2D->FOVAngle;
	Near = SceneCaptureComponent2D->bOverride_CustomNearClippingPlane ? SceneCaptureComponent2D->CustomNearClippingPlane : GNearClippingPlane;
	Far = SceneCaptureComponent2D->MaxViewDistanceOverride == -1 ? 1000000.0f : SceneCaptureComponent2D->MaxViewDistanceOverride;

	bCaptureEveryFrame = SceneCaptureComponent2D->bCaptureEveryFrame;
	bCaptureOnMove = SceneCaptureComponent2D->bCaptureOnMovement;
	bUseAA = SceneCaptureComponent2D->ShowFlags.AntiAliasing;
	bShowDebugPlane = false;
	bManualCaptureOneFrame = false;

	bIsCapturing = (X > 0 && Y > 0 && SceneCaptureComponent2D->IsVisible());

	if (bIsCapturing)
	{
		bShowDebugPlane = SceneCaptureComponent2D->ComponentHasTag("ShowDebugPlanePICO");
		if (!bCaptureEveryFrame && !bCaptureOnMove)
		{
			bManualCaptureOneFrame = SceneCaptureComponent2D->ComponentHasTag("ManualCaptureOneFramePICO");
			if (bManualCaptureOneFrame)
			{
				SceneCaptureComponent2D->ComponentTags.Remove("ManualCaptureOneFramePICO");
			}
		}
	}

	FString TagsString;
	for (int32 Index = 0; Index < SceneCaptureComponent2D->ComponentTags.Num(); ++Index)
	{
		if (Index > 0)
		{
			TagsString += TEXT(",");
		}
		TagsString += SceneCaptureComponent2D->ComponentTags[Index].ToString();
	}

	const bool bStateChanged =
		bInitialize ||
		bPreviousIsCapturing != bIsCapturing ||
		bPreviousShowDebugPlane != bShowDebugPlane ||
		bPreviousManualCaptureOneFrame != bManualCaptureOneFrame ||
		bPreviousCaptureEveryFrame != bCaptureEveryFrame ||
		bPreviousCaptureOnMove != bCaptureOnMove ||
		PreviousX != X ||
		PreviousY != Y;

	if (!bIsCapturing)
	{
		FString Reason;
		if (X == 0 || Y == 0)
		{
			Reason += FString::Printf(TEXT("InvalidRenderTargetSize(%d,%d) "), X, Y);
		}
		if (!SceneCaptureComponent2D->IsVisible())
		{
			Reason += TEXT("ComponentNotVisible ");
		}
		if (!bCaptureEveryFrame && !bCaptureOnMove)
		{
			Reason += TEXT("CaptureModeDisabled ");
		}

		if (bInitialize || bStateChanged)
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("[SceneCaptureProxy] Not capturing. Name=%s Component=%s Reason=%s Tags=[%s]"),
				*Name,
				*SceneCaptureComponent2D->GetName(),
				Reason.IsEmpty() ? TEXT("Unknown") : *Reason,
				*TagsString);
		}
	}

	if ((bInitialize || bStateChanged) && SceneCaptureComponent2D->ComponentHasTag("ShowDebugPlanePICO") && !bIsCapturing)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[SceneCaptureProxy] ShowDebugPlanePICO tag detected but proxy is not capturing. Name=%s Component=%s CaptureEveryFrame=%d CaptureOnMove=%d Size=%dx%d Visible=%d"),
			*Name,
			*SceneCaptureComponent2D->GetName(),
			bCaptureEveryFrame ? 1 : 0,
			bCaptureOnMove ? 1 : 0,
			X,
			Y,
			SceneCaptureComponent2D->IsVisible() ? 1 : 0);
	}
}

FSceneCaptureEntityProxy::FSceneCaptureEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	:FSceneEntityProxy(InParentEntityProxy, SceneComponent)
{
	ConstructSceneCaptureEntityData(SceneComponent, true);
}

FSceneCaptureEntityProxy::~FSceneCaptureEntityProxy()
{
}

void FSceneCaptureEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FSceneEntityProxy::UpdateEntityProxy(SceneComponent);
	ConstructSceneCaptureEntityData(SceneComponent, false);
}

bool FSceneCaptureEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FSceneEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
		if (bTransformDirty)
		{
			ApplySceneCaptureNativeTransformCorrection(Transform);
		}
#if PLATFORM_ANDROID
		if (NativeEntity.Get()->IsValid() && InCurrentScene && !DebugPlaneOffsetEntity.isValid())
		{
			DebugPlaneOffsetEntity = InCurrentScene->createEntity();
			DebugPlaneOffsetEntity.setName(TCHAR_TO_UTF8(*(Name + TEXT("_PreviewOffset"))));
			DebugPlaneOffsetEntity.setParent(NativeEntity.Get()->entity);
			DebugPlaneOffsetEntity.getOrAddTransformComponent().setPosition(ToSpVector(FVector::ZeroVector, 100.0f));
			DebugPlaneOffsetEntity.getOrAddTransformComponent().setRotation(ToSpQuat(FRotator(0, 0, 90).Quaternion()));
			if (SpatialContainer)
			{
				SpatialContainer->addEntity(DebugPlaneOffsetEntity);
			}
		}
#endif
		return true;
	}

	UE_LOG(LogPICOSpatial, Warning, TEXT("[SceneCaptureProxy] Native entity initialization failed. Name=%s Scene=%p SpatialContainer=%p"),
		*Name,
		InCurrentScene,
		SpatialContainer);
	return false;
}

void FSceneCaptureEntityProxy::UpdateNativeEntity()
{
	FSceneEntityProxy::UpdateNativeEntity();
#if PLATFORM_ANDROID
	spatialpack::SceneEntity entity = NativeEntity.Get()->entity;
	if (!entity.isValid())
	{
		return;
	}

	if (!ParentEntity)
	{
		return;
	}

	if (entity.isValid() && ParentEntity)
	{
		if (bIsCapturing && !entity.hasComponent(spatialpack::SceneCaptureComponent::getType()))
		{
			UE_LOG(LogPICOSpatial, Log, TEXT("[SceneCaptureProxy] Start capture. Name=%s Size=%dx%d Ratio=%.3f FOV=%.2f Near=%.2f Far=%.2f CaptureEveryFrame=%d CaptureOnMove=%d UseAA=%d ShowDebugPlane=%d ManualCaptureOneFrame=%d"),
				*Name,
				X,
				Y,
				Ratio,
				FOV,
				Near,
				Far,
				bCaptureEveryFrame ? 1 : 0,
				bCaptureOnMove ? 1 : 0,
				bUseAA ? 1 : 0,
				bShowDebugPlane ? 1 : 0,
				bManualCaptureOneFrame ? 1 : 0);

			spatialpack::RenderTextureDescriptor rtDesc;
			rtDesc.setWidth(X).setHeight(Y).setTexturePixelFormat(spatialpack::TexturePixelFormat::RGBA8_SRGB);
			NativeRT = spatialpack::TextureResource::createRenderTexture(rtDesc);

			spatialpack::SceneCaptureComponent SceneCapture = entity.getOrAddSceneCaptureComponent();
			SceneCapture.setColorRenderTarget(NativeRT);

			SceneCapture.setPerspectiveCameraParameters(FOV, Ratio, Near / 100.0f, Far / 100.0f);
			SceneCapture.setUseGlobalSkybox(true);
			SceneCapture.setShadowingEnabled(true);
			SceneCapture.setAntiAliasing(bUseAA ? 4 : 1);
			SceneCapture.setUseGlobalIBL(true);

			if (bCaptureEveryFrame)
			{
				SceneCapture.setCaptureMode(spatialpack::SceneCaptureMode::LOOP);
				SceneCapture.setCaptureInterval(0);
			}
			else if(bCaptureOnMove)
			{
				SceneCapture.setCaptureMode(spatialpack::SceneCaptureMode::ON_TRANSFORM_CHANGED);
			}
			else
			{
				SceneCapture.setCaptureMode(spatialpack::SceneCaptureMode::MANUAL);
			}
		}
		else if (!bIsCapturing && entity.hasComponent(spatialpack::SceneCaptureComponent::getType()))
		{
			entity.getOrAddSceneCaptureComponent().clearRenderList();
			DebugPlaneWidthMeters = 0.0f;
			DebugPlaneHeightMeters = 0.0f;
			DebugPlaneMaterial = spatialpack::Material();
			DebugPlaneMaterialResourceId = 0;
			entity.removeSceneCaptureComponent();
		}

		if (bIsCapturing)
		{
			if (entity.hasComponent(spatialpack::SceneCaptureComponent::getType()) && ParentEntity->entity.isValid())
			{
				spatialpack::SceneCaptureComponent SceneCapture = entity.getOrAddSceneCaptureComponent();
				SceneCapture.clearRenderList();
				SceneCapture.addEntityToRenderList(ParentEntity->entity);
			}
			else
			{
				if (entity.hasComponent(spatialpack::SceneCaptureComponent::getType()))
				{
					entity.getOrAddSceneCaptureComponent().clearRenderList();
				}
			}

			if (bManualCaptureOneFrame)
			{
				spatialpack::SceneCaptureUtility::captureOnce(entity);
			}

			if (bShowDebugPlane)
			{
				const float DebugPlaneBaseHeightMeters = 4.0f;
				const float SafeRatio = Ratio > KINDA_SMALL_NUMBER ? Ratio : 1.0f;
				const float DebugPlaneWidth = DebugPlaneBaseHeightMeters * SafeRatio;
				const float DebugPlaneHeight = DebugPlaneBaseHeightMeters;
				spatialpack::SceneEntity& DebugPlaneEntity = DebugPlaneOffsetEntity.isValid() ? DebugPlaneOffsetEntity : entity;
				const bool bDebugPlaneMeshChanged = !DebugPlaneEntity.hasComponent(spatialpack::ModelComponent::getType())
					|| !FMath::IsNearlyEqual(DebugPlaneWidthMeters, DebugPlaneWidth)
					|| !FMath::IsNearlyEqual(DebugPlaneHeightMeters, DebugPlaneHeight);

				if (bDebugPlaneMeshChanged)
				{
					DebugPlaneEntity.getOrAddModelComponent().setMesh(spatialpack::MeshResource::createPlane(DebugPlaneWidth, DebugPlaneHeight, 0.0f, spatialpack::Axis::Y));
					DebugPlaneWidthMeters = DebugPlaneWidth;
					DebugPlaneHeightMeters = DebugPlaneHeight;
				}

				if (DebugPlaneEntity.hasComponent(spatialpack::CollisionComponent::getType()))
				{
					DebugPlaneEntity.removeCollisionComponent();
				}
				DebugPlaneEntity.getOrAddGroundShadowComponent().setReceivesShadowEnabled(false);
				DebugPlaneEntity.getOrAddGroundShadowComponent().setCastsShadowEnabled(false);
				DebugPlaneEntity.getOrAddDynamicShadowComponent().setReceivesShadowEnabled(false);
				DebugPlaneEntity.getOrAddDynamicShadowComponent().setCastsShadowEnabled(false);

				const uint64 CurrentNativeRTResourceId = NativeRT.isValid() ? NativeRT.getResourceId() : 0;
				if (!DebugPlaneMaterial.isValid() || DebugPlaneMaterialResourceId != CurrentNativeRTResourceId)
				{
					auto SceneCaptureDebugUnlitMaterial = spatialpack::UnlitMaterial::create(spatialpack::MaterialBlendingMode::OPAQUE);
					SceneCaptureDebugUnlitMaterial.setBaseColorTextureParameter(spatialpack::MaterialTextureParameter{ NativeRT });
					DebugPlaneMaterial = SceneCaptureDebugUnlitMaterial.toMaterial();
					DebugPlaneMaterialResourceId = CurrentNativeRTResourceId;
					DebugPlaneEntity.getOrAddModelComponent().setMaterial(DebugPlaneMaterial);
				}
			}
			else if ((DebugPlaneOffsetEntity.isValid() && DebugPlaneOffsetEntity.hasComponent(spatialpack::ModelComponent::getType()))
				|| entity.hasComponent(spatialpack::ModelComponent::getType()))
			{
				DebugPlaneWidthMeters = 0.0f;
				DebugPlaneHeightMeters = 0.0f;
				DebugPlaneMaterial = spatialpack::Material();
				DebugPlaneMaterialResourceId = 0;
				if (DebugPlaneOffsetEntity.isValid() && DebugPlaneOffsetEntity.hasComponent(spatialpack::ModelComponent::getType()))
				{
					DebugPlaneOffsetEntity.removeModelComponent();
				}
				if (entity.hasComponent(spatialpack::ModelComponent::getType()))
				{
					entity.removeModelComponent();
				}
			}
		}
	}
#endif
}
