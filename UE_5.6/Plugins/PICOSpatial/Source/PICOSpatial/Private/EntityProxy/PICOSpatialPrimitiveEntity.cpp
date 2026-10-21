// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialPrimitiveEntity.h"
#include "PhysicsEngine/BodySetup.h"
#include "PICOSpatial.h"

static TAutoConsoleVariable<bool> CVarPICOEnableGroundingShadow(
	TEXT("PICO.EnableGroundingShadow"),
	true,
	TEXT("false: No grounding shadow\n")
	TEXT("true: Use grounding shadow (default)\n"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarPICOEnableDynamicShadow(
	TEXT("PICO.EnableDynamicShadow"),
	false,
	TEXT("false: No dynamic shadow\n")
	TEXT("true: Use dynamic shadow (default)\n"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarPICOOverrideRenderInMainPass(
	TEXT("PICO.OverrideRenderInMainPass"),
	false,
	TEXT("true:  PICO Spatial takes over rendering for primitive components - UE main pass is disabled\n")
	TEXT("false: PICO Spatial does not interfere with UE rendering, components keep their original bRenderInMainPass (default)\n"),
	ECVF_Default);

void FPrimitiveEntityProxy::ConstructPrimitiveEntityData(USceneComponent* SceneComponent, bool bInitialize)
{
	if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(SceneComponent))
	{
		if (bInitialize)
		{
			bCreateSimpleCollision = false;
			bCreateComplexCollision = false;		
			UBodySetup* BodySetup = const_cast<UPrimitiveComponent * >(PrimitiveComponent)->GetBodySetup();
			if (BodySetup)
			{
				Collision = BodySetup->AggGeom;
				CollisionTraceFlag = BodySetup->CollisionTraceFlag;
			}
			bDynamic = PrimitiveComponent->Mobility == EComponentMobility::Movable ? true : false;
			bCastDynamicShadow = PrimitiveComponent->CastShadow && PrimitiveComponent->bCastDynamicShadow;
			// Snapshot the original value before any PICO modification
			bOriginalRenderInMainPass = PrimitiveComponent->bRenderInMainPass;
			bDidOverrideRenderInMainPass = false;
		}

		bEnableCollision = bEnableInput = PrimitiveComponent->ComponentHasTag("EnableInputPICO") ? true : false;
		bAutoHover = PrimitiveComponent->ComponentHasTag("AutoHoverPICO") ? true : false;
	}
}

FPrimitiveEntityProxy::~FPrimitiveEntityProxy()
{
	// Restore bRenderInMainPass to its original value if PICO changed it.
	// TODO: If a third party also modifies bRenderInMainPass while this proxy is alive,
	// the restore here may overwrite that change. We currently assume a single default
	// configuration and that developers do not modify bRenderInMainPass at runtime.
	if (bDidOverrideRenderInMainPass)
	{
		// Owner is TWeakObjectPtr<USceneComponent> (base class), automatically nulled by GC.
		// TODO: If a third party also modifies bRenderInMainPass while this proxy is alive,
		// the restore here may overwrite that change. We currently assume a single default
		// configuration and that developers do not modify bRenderInMainPass at runtime.
		UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Owner.Get());
		if (IsValid(PrimitiveComponent) && PrimitiveComponent->bRenderInMainPass != bOriginalRenderInMainPass)
		{
			PrimitiveComponent->SetRenderInMainPass(bOriginalRenderInMainPass);
			UE_LOG(LogPICOSpatial, Verbose,
				TEXT("PICOSpatial: Restored bRenderInMainPass on '%s': -> %d"),
				*PrimitiveComponent->GetReadableName(), (int32)bOriginalRenderInMainPass);
		}
	}
}

FPrimitiveEntityProxy::FPrimitiveEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent)
	:FSceneEntityProxy(InParentEntityProxy, SceneComponent)
{
	ConstructPrimitiveEntityData(SceneComponent, true);

	// Apply main-pass rendering override once at construction.
	// When PICO.OverrideRenderInMainPass is true, PICO Spatial takes over rendering
	// and UE main-pass rendering is disabled (bRenderInMainPass -> false).
	// When false, UE rendering is not touched.
	if (CVarPICOOverrideRenderInMainPass.GetValueOnGameThread())
	{
		UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Owner.Get());
		if (IsValid(PrimitiveComponent) && PrimitiveComponent->bRenderInMainPass)
		{
			PrimitiveComponent->SetRenderInMainPass(false);
			bDidOverrideRenderInMainPass = true;
			UE_LOG(LogPICOSpatial, Verbose,
				TEXT("PICOSpatial: Overrode bRenderInMainPass on '%s': 1 -> 0 (PICO proxy rendering)"),
				*PrimitiveComponent->GetReadableName());
		}
	}
}

void FPrimitiveEntityProxy::UpdateEntityProxy(USceneComponent* SceneComponent)
{
	FSceneEntityProxy::UpdateEntityProxy(SceneComponent);
	ConstructPrimitiveEntityData(SceneComponent, false);
}

bool FPrimitiveEntityProxy::InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer)
{
	if (FSceneEntityProxy::InitializeNativeEntity(InCurrentScene, SpatialContainer))
	{
		if (NativeEntity.Get()->IsValid())
		{
			switch (CollisionTraceFlag)
			{
			case CTF_UseDefault:
				bCreateSimpleCollision = true;
				break;
			case CTF_UseSimpleAndComplex:
				bCreateSimpleCollision = true;
				bCreateComplexCollision = true;
				break;
			case CTF_UseSimpleAsComplex:
				bCreateSimpleCollision = true;
				break;
			case CTF_UseComplexAsSimple:
				bCreateComplexCollision = true;
				break;
			}
#if PLATFORM_ANDROID
			ShapeResources.Empty();
			if (bCreateSimpleCollision)
			{
				for (const FKBoxElem& BoxElem : Collision.BoxElems)
				{
					spatialpack::ShapeResource ShapeResource;
					ShapeResource = spatialpack::ShapeResource::createBox(ToSpVector(FVector(BoxElem.X == 0 ? 0.0104 : BoxElem.X, BoxElem.Y == 0 ? 0.0104 : BoxElem.Y, BoxElem.Z == 0 ? 0.0104 : BoxElem.Z), 100.0f)).offsetByTranslationAndRotation(ToSpQuat(BoxElem.Rotation.Quaternion()), ToSpVector(BoxElem.Center, 100.0f));
					ShapeResources.Add(ShapeResource);
				}

				for (const FKSphereElem& SphereElem : Collision.SphereElems)
				{
					spatialpack::ShapeResource ShapeResource;
					ShapeResource = spatialpack::ShapeResource::createSphere(SphereElem.Radius / 100.0f).offsetByTranslation(ToSpVector(SphereElem.Center, 100.0f));
					ShapeResources.Add(ShapeResource);
				}

				for (const FKSphylElem& SphylElem : Collision.SphylElems)
				{
					spatialpack::ShapeResource ShapeResource;
					ShapeResource = spatialpack::ShapeResource::createCapsule(SphylElem.Length / 100.0f, SphylElem.Radius / 100.0f).offsetByTranslationAndRotation(ToSpQuat(SphylElem.Rotation.Quaternion()), ToSpVector(SphylElem.Center, 100.0f));
					ShapeResources.Add(ShapeResource);
				}
			}

			spatialpack::SceneEntity& entity = NativeEntity.Get()->entity;

			if (CVarPICOEnableGroundingShadow->GetBool())
			{
				entity.getOrAddGroundShadowComponent().setReceivesShadowEnabled(true);
				entity.getOrAddGroundShadowComponent().setCastsShadowEnabled(bCastDynamicShadow);
			}
			if (CVarPICOEnableDynamicShadow->GetBool())
			{
				entity.getOrAddDynamicShadowComponent().setReceivesShadowEnabled(true);
				entity.getOrAddDynamicShadowComponent().setCastsShadowEnabled(bCastDynamicShadow);
			}

#endif // PLATFORM_ANDROID
			return true;
		}
	}
	return false;
}

void FPrimitiveEntityProxy::UpdateNativeEntity()
{
	FSceneEntityProxy::UpdateNativeEntity();
#if PLATFORM_ANDROID
	spatialpack::SceneEntity entity = NativeEntity.Get()->entity;
	if (!entity.isValid())
	{
		return;
	}

	if (bEnableInput && !entity.hasComponent(spatialpack::InteractableComponent::getType()))
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("Set Component:%s Enable Input."), *Name);
		entity.getOrAddInteractableComponent();
	}
	else if (!bEnableInput && entity.hasComponent(spatialpack::InteractableComponent::getType()))
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("Set Component:%s Disable Input."), *Name);
		entity.removeInteractableComponent();
	}

	if (bEnableCollision && !entity.hasComponent(spatialpack::CollisionComponent::getType()))
	{
		entity.getOrAddCollisionComponent();
		for (const spatialpack::ShapeResource& ShapeResource : ShapeResources)
		{
			entity.getOrAddCollisionComponent().addShapeResource(ShapeResource);
		}
	}
	else if (!bEnableCollision && entity.hasComponent(spatialpack::CollisionComponent::getType()))
	{
		entity.removeCollisionComponent();
	}

	if (bAutoHover && !entity.hasComponent(spatialpack::HoverEffectComponent::getType()))
	{
		entity.getOrAddHoverEffectComponent();
	}
	else if (!bAutoHover && entity.hasComponent(spatialpack::HoverEffectComponent::getType()))
	{
		entity.removeHoverEffectComponent();
	}
#endif // PLATFORM_ANDROID
}
