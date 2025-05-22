// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_VerificationSubsystem.h"


#include "EditorUtilitySubsystem.h"
#include "ISettingsModule.h"
#include "PXR_VerificatonFunctionLibrary.h"


template<typename VerificationObject>
void UPXR_VerificationSubsystem::RegisterObject()
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

void UPXR_VerificationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	GetMutableDefault<UEditorUtilitySubsystem>()->StartupObjects.Add(FString(TEXT("/PICOXR/Blueprint/UI/EUB_MenuEntry.EUB_MenuEntry")));

	FixedVerificationObjects.Empty();
	RequiredVerificationObjects.Empty();
	RegisterObject<USetMSAAObject>();
	RegisterObject<UEnableMultiviewObject>();
	
	RegisterObject<UEnableVulkanObject>();
	RegisterObject<UHalfPrecisionFloatObject>();
	RegisterObject<UDisableAmbientOcclusionObject>();
	//Todo:Confirm gain
	//RegisterObject<UEnableOcclusionCullingObject>();
	RegisterObject<UDisableOpenXRPluginObject>();
	RegisterObject<USetAndroidSDKMinimumObject>();
	RegisterObject<USetAndroidSDKTargetObject>();
	RegisterObject<USetArm64CPUObject>();
	RegisterObject<UDisableTouchInterfaceObject>();

#ifndef PICO_CUSTOM_ENGINE
	RegisterObject<UEnableMobileHDRObject>();
	RegisterObject<UDisableMovableSpotlightShadowsObject>();
#endif
}

bool UPXR_VerificationSubsystem::Refresh(bool& bOutNeedRestartEditor)
{
	bool bHasChanged = false;
	
	auto MoveObjectBetweenArrays = [&](TArray<UVerificationObject*>& sourceArray, TArray<UVerificationObject*>& targetArray, bool (*condition)(UVerificationObject*))
	{
		for (auto It = sourceArray.CreateIterator(); It; ++It)
		{
			UVerificationObject* Object = *It;
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
	MoveObjectBetweenArrays(RequiredVerificationObjects, FixedVerificationObjects, [](UVerificationObject* Obj) { return Obj->IsVerificationFixed(); });
	
	//FixedVerificationObjects To RequiredVerificationObjects
	MoveObjectBetweenArrays(FixedVerificationObjects, RequiredVerificationObjects, [](UVerificationObject* Obj) { return!Obj->IsVerificationFixed(); });
	
	return bHasChanged;
}

