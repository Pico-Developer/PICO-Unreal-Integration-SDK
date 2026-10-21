// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_PortalSubsystem.h"


#include "EditorUtilitySubsystem.h"
#include "PICO_BodyTrackingComponent.h"
#include "PICO_FaceTrackingComponent.h"
#include "PICO_HandTrackingComponent.h"
#include "PICO_PortalFunctionLibrary.h"
#include "PICO_SceneCapturesGenerator.h"
#include "PICO_SpatialAnchorActor.h"
#include "PICO_SpatialMeshActor.h"
#include "Engine/RendererSettings.h"

template<typename VerificationObject>
void UPICO_PortalSubsystem::RegisterObject()
{
	VerificationObject* Obj=NewObject<VerificationObject>();

	if (Obj->IsVerificationFixed())
	{
		FixedVerificationObjects.Emplace(Obj);
	}
	else
	{
		RequiredVerificationObjects.Emplace(Obj);
	}
}

void UPICO_PortalSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GetMutableDefault<UEditorUtilitySubsystem>()->StartupObjects.Reset();
	GetMutableDefault<UEditorUtilitySubsystem>()->StartupObjects.Add(FString(TEXT("/PICOOpenXR/Blueprints/UI/EUB_MenuEntry.EUB_MenuEntry")));

	FixedVerificationObjects.Empty();
	RequiredVerificationObjects.Empty();
	RegisterObject<UPICOSetMSAAObject>();
	RegisterObject<UPICOEnableMultiviewObject>();
	
	RegisterObject<UPICOEnableVulkanObject>();
	RegisterObject<UPICOHalfPrecisionFloatObject>();
	RegisterObject<UPICODisableAmbientOcclusionObject>();
	RegisterObject<UPICOSetAndroidSDKMinimumObject>();
	RegisterObject<UPICOSetAndroidSDKTargetObject>();
	RegisterObject<UPICOSetArm64CPUObject>();
	RegisterObject<UPICODisableTouchInterfaceObject>();

	RegisterObject<UPICOEnableMobileHDRObject>();
	RegisterObject<UPICODisableMovableSpotlightShadowsObject>();

	const URendererSettings* Settings = GetDefault<URendererSettings>();
	if (Settings->bMobilePostProcessing)
	{
		UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::MobileHDR);
	}
	if (Settings->MobileShadingPath == EMobileShadingPath::Deferred)
	{
		UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::DeferredShading);
	}
	if (Settings->MobileShadingPath == EMobileShadingPath::Forward)
	{
		UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::ForwardShading);
	}
	if (Settings->bMobileAllowMovableSpotlightShadows)
	{
		UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::MovableSpotlight);
	}
	if (Settings->MobileAntiAliasing == EMobileAntiAliasingMethod::Type::MSAA)
	{
		UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::MSAA);
	}
	else if (Settings->MobileAntiAliasing == EMobileAntiAliasingMethod::Type::FXAA)
	{
		UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::FXAA);
	}
	else if (Settings->MobileAntiAliasing == EMobileAntiAliasingMethod::Type::TemporalAA)
	{
		UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::TAA);
	}
#ifdef PICO_CUSTOM_ENGINE
	if (Settings->bMobileSupportSpaceWarp)
	{
		UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::ApplicationSpaceWarp);
	}
#endif

	// Check Engine Version
	FString UnrealMajorVersion = FString::FromInt(ENGINE_MAJOR_VERSION);
	FString UnrealMinorVersion = FString::FromInt(ENGINE_MINOR_VERSION);
	FString UnrealPatchVersion = FString::FromInt(ENGINE_PATCH_VERSION);
	FString EngineVersion = UnrealMajorVersion + "." + UnrealMinorVersion + "." + UnrealPatchVersion;
	FString SDKVersion=UPICOPortalFunctionLibrary::GetPICOSDKVersionString();

	UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::EngineVersion,EngineVersion);
	UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::SDKVersion,SDKVersion);
	UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::SDKType);
	UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::EngineType);
}

void UPICO_PortalSubsystem::Deinitialize()
{
	Super::Deinitialize();

	UWorld* World = GWorld;
	if (World&&OnActorSpawnedHandle.IsValid())
	{
		World->RemoveOnActorSpawnedHandler(OnActorSpawnedHandle);
	}
}

bool UPICO_PortalSubsystem::Refresh(bool& bOutNeedRestartEditor)
{
	UWorld* World = GWorld;
	if (World&&!OnActorSpawnedHandle.IsValid())
	{
		OnActorSpawnedHandle = World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateUObject(this, &UPICO_PortalSubsystem::HandleOnActorSpawned));
	}
	
	bool bHasChanged = false;
	
	auto MoveObjectBetweenArrays = [&](TArray<UPICOVerificationObject*>& sourceArray, TArray<UPICOVerificationObject*>& targetArray, bool (*condition)(UPICOVerificationObject*))
	{
		for (auto It = sourceArray.CreateIterator(); It; ++It)
		{
			UPICOVerificationObject* Object = *It;
			if (condition(Object))
			{
				bOutNeedRestartEditor=Object->RequiresEditorRestart();
				
				targetArray.Add(Object);
				It.RemoveCurrent();
				bHasChanged = true;
				break;
			}
		}
	};
	
	// RequiredVerificationObjects To FixedVerificationObjects
	MoveObjectBetweenArrays(RequiredVerificationObjects, FixedVerificationObjects, [](UPICOVerificationObject* Obj) { return Obj->IsVerificationFixed(); });
	
	//FixedVerificationObjects To RequiredVerificationObjects
	MoveObjectBetweenArrays(FixedVerificationObjects, RequiredVerificationObjects, [](UPICOVerificationObject* Obj) { return!Obj->IsVerificationFixed(); });
	
	return bHasChanged;
}

void UPICO_PortalSubsystem::HandleOnActorSpawned(AActor* Actor)
{
	if (Actor&&Actor->GetClass()->IsChildOf(ASceneCapturesGeneratorPICO::StaticClass()))
	{
		UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::SceneCapturesGenerator);
	}
	else if (Actor&&Actor->GetClass()->IsChildOf(ASpatialMeshActorPICO::StaticClass()))
	{
		UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::SpatialMeshActor);
	}
	else if (Actor&&Actor->GetClass()->IsChildOf(ASpatialAnchorActorPICO::StaticClass()))
	{
		UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::SpatialAnchorActor);
	}
	
	for (auto Component : Actor->GetComponents())
	{
		if (Component&&Component->GetClass()->IsChildOf(UHandTrackingComponentPICO::StaticClass()))
		{
			UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::HandTrackingComponent);
		}
		if (Component&&Component->GetClass()->IsChildOf(UBodyTrackingComponentPICO::StaticClass()))
		{
			UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::BodyTrackingComponent);
		}
		else if (Component&&Component->GetClass()->IsChildOf(UFaceTrackingComponentPICO::StaticClass()))
		{
			UPICOPortalFunctionLibrary::AddAppLogByType(EAppLogTypePICO::FaceTrackingComponent);
		}
	}
}

