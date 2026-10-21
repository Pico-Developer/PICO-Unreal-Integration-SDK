// Copyright® PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_VerificationObject.h"
#include "PICO_PortalTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PICO_PortalFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PICOOPENXRPORTAL_API UPICOPortalFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PICO|Portal")
	static void StartUpdatePortalTick();
	
	UFUNCTION(BlueprintCallable, Category = "PICO|Portal")
	static bool IsShowPortalOnEditorStart();

	UFUNCTION(BlueprintCallable, Category = "PICO|Portal")
	static bool IsBehaviorDataCollectionEnabled();

	UFUNCTION(BlueprintCallable, Category = "PICO|Portal")
	static void SetShowPortalOnEditorStart(bool bShowPortalOnEditorStart);

	UFUNCTION(BlueprintCallable, Category = "PICO|Portal")
	static void SetBehaviorDataCollectionEnabled(bool bEnableBehaviorDataCollection);

	UFUNCTION(BlueprintCallable, Category = "PICO|Portal")
	static void LaunchPICOSettings();

	UFUNCTION(BlueprintCallable, Category = "PICO|Portal")
	static void LaunchPICOPerformanceSettings();

	UFUNCTION(BlueprintPure, Category = "PICO|Portal")
	static FString GetPICOSDKVersionString();

	// AppLog
	UFUNCTION(BlueprintCallable, Category = "PICO|Portal")
	static void AddAppLogByType(EAppLogTypePICO AppLogType,const FString& AppLogString = "");
	
	UFUNCTION(BlueprintPure, Category = "PICO|Verificaton")
	static TArray<UPICOVerificationObject*> GetFixedVerificationObjects();

	UFUNCTION(BlueprintPure, Category = "PICO|Verificaton")
	static TArray<UPICOVerificationObject*> GetRequiredVerificationObjects();
	
	UFUNCTION(BlueprintCallable, Category = "PICO|Verificaton")
	static class UPICOVerificatonEventManager* PXR_GetVerificationEventManager();

	static bool HasVerificationNeedRestart();

	static void SetVerificationNeedRestart(bool bRestart);

	UFUNCTION(BlueprintCallable, Category = "PICO|Verificaton")
	static void FixAllVerifications();
	
	UFUNCTION(BlueprintPure, Category = "PICO|Verification")
	static bool IsXRPluginVerified();

	UFUNCTION(BlueprintCallable, Category = "PICO|Verification")
	static void FixXRPluginConfig();

	UFUNCTION(BlueprintPure, Category = "PICO|Verification")
	static bool IsVulkanVerified();

	UFUNCTION(BlueprintCallable, Category = "PICO|Verification")
	static void FixVulkanConfig();
	
	UFUNCTION(BlueprintPure, Category = "PICO|Verification")
	static bool IsMultiviewVerified();

	UFUNCTION(BlueprintCallable, Category = "PICO|Verification")
	static void FixMultiviewConfig();
	
	UFUNCTION(BlueprintPure, Category = "PICO|Verification")
	static bool IsAndroidSDKVersionVerified();

	UFUNCTION(BlueprintCallable, Category = "PICO|Verification")
	static void FixAndroidSDKVersionConfig();
private:
	static bool bShowRestartEditorButton;

	// AppLog
	static FString XRPortal;
	static FString XRToolKit;
	static FString XRSettings;
	static FString XREngineVersion;
	static FString XRSDKVersion;
	static FString SDKType;
	static FString EngineType;
	static FString PICOXRSDK;
	static FString OpenXRSDK;
	static FString OfficialEngine;
	static FString PICOCustomEngine;

	// AppLog events
	static FString XRPortalStartUp;
	static FString ProjectVerificationStartUp;
	static FString XRPICOSettingsStartUp;
	static FString PICODeveloperCenter;
	static FString PICOEmulator;
	static FString GetStartedSample;
	static FString PlatformSample;
	static FString SpatialAudioSample;
	static FString MRSample;
	static FString HandTrackingSample;
	static FString ControllerInteraction;
	static FString BodyTracking;
	static FString PICODeveloperWebsite;
	static FString DownloadFromPICO_Official;
	static FString DownloadFromUnreal_Fab;
	static FString DownloadPICOForkOfUnrealEngine;
	
	static FString SpatialMeshActor;
	static FString SpatialAnchorActor;
	static FString SceneCapturesGenerator;
	static FString PICOXRPawn;
	static FString HandTrackingComponent;
	static FString EyeTrackingComponent;
	static FString FaceTrackingComponent;
	
	static FString MobileHDR;
	static FString DeferredShading;
	static FString ForwardShading;
	static FString LateLatching;
	static FString MovableSpotlight;
	static FString ApplicationSpaceWarp;
	static FString ToneMapping;
	static FString FXAA;
	static FString TAA;
	static FString MSAA;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRVerificatonUpdatedDelegate);

UCLASS()
class UPICOVerificatonEventManager : public UObject
{
	GENERATED_BODY()
public:
	
	static UPICOVerificatonEventManager* GetInstance();
	
	UPROPERTY(BlueprintAssignable)
	FPXRVerificatonUpdatedDelegate VerificatonUpdatedDelegate;
};