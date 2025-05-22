// Copyright® PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PXR_VerificationObject.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PXR_VerificatonFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PICOXRPROJECTVERIFICATION_API UUPICOXRVerificationFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "PXR|Verificaton")
	static TArray<UVerificationObject*> GetFixedVerificationObjects();

	UFUNCTION(BlueprintPure, Category = "PXR|Verificaton")
	static TArray<UVerificationObject*> GetRequiredVerificationObjects();
	
	UFUNCTION(BlueprintCallable, Category = "PXR|Verificaton")
	static class UPICOXRVerificatonEventManager* PXR_GetVerificationEventManager();

	static bool HasVerificationNeedRestart();

	static void SetVerificationNeedRestart(bool bRestart);

	UFUNCTION(BlueprintCallable, Category = "PXR|Verificaton")
	static void UpdateVerificationStatus();

	UFUNCTION(BlueprintCallable, Category = "PXR|Verificaton")
	static void FixAllVerifications();
	
	UFUNCTION(BlueprintCallable, Category = "PXR|Verification")
	static bool IsShowPortalOnEditorStart();

	UFUNCTION(BlueprintCallable, Category = "PXR|Verification")
	static void SetShowPortalOnEditorStart(bool bShowPortalOnEditorStart);

	UFUNCTION(BlueprintCallable, Category = "PXR|Verification")
	static void LaunchPICOSettings();

	UFUNCTION(BlueprintPure, Category = "PXR|Verification")
	static FString GetVersionString();

	UFUNCTION(BlueprintPure, Category = "PXR|Verification")
	static bool IsXRPluginVerified();

	UFUNCTION(BlueprintCallable, Category = "PXR|Verification")
	static void FixXRPluginConfig();

	UFUNCTION(BlueprintPure, Category = "PXR|Verification")
	static bool IsVulkanVerified();

	UFUNCTION(BlueprintCallable, Category = "PXR|Verification")
	static void FixVulkanConfig();
	
	UFUNCTION(BlueprintPure, Category = "PXR|Verification")
	static bool IsMultiviewVerified();

	UFUNCTION(BlueprintCallable, Category = "PXR|Verification")
	static void FixMultiviewConfig();
	
	UFUNCTION(BlueprintPure, Category = "PXR|Verification")
	static bool IsAndroidSDKVersionVerified();

	UFUNCTION(BlueprintCallable, Category = "PXR|Verification")
	static void FixAndroidSDKVersionConfig();
	
private:
	static bool bShowRestartEditorButton;
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRVerificatonUpdatedDelegate);

UCLASS()
class UPICOXRVerificatonEventManager : public UObject
{
	GENERATED_BODY()
public:
	
	static UPICOXRVerificatonEventManager* GetInstance();
	
	UPROPERTY(BlueprintAssignable)
	FPXRVerificatonUpdatedDelegate VerificatonUpdatedDelegate;
};