// Copyright® PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <filesystem>

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PXR_VerificationObject.generated.h"

UENUM(BlueprintType)
enum class EVerificationLevel : uint8
{
	None,
	Error,
	Warning,
	Recommended,
};
/**
 * 
 */
UCLASS(ClassGroup = Verification, Blueprintable)
class UVerificationObject : public UObject
{
	GENERATED_BODY()
public:
	UVerificationObject();
	UVerificationObject(const FText& InDisplayName,
	const FText& InDescription,
	const EVerificationLevel& InVerificationLevel,
	bool InNeedRestartEditor);

	UFUNCTION(BlueprintCallable,Category="Verification")
	FText GetDisplayName() const { return DisplayName; }
	
	UFUNCTION(BlueprintCallable,Category="Verification")
	FText GetDescription() const { return Description; }
	
	virtual void PerformVerificationFix();

	virtual bool IsVerificationFixed();

	virtual bool RequiresEditorRestart();
	
	inline EVerificationLevel GetLevel() const { return Level; }
private:
	FText DisplayName;
	FText Description;

protected:
	EVerificationLevel Level;
	bool bNeedRestartEditor;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class USetMSAAObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	USetMSAAObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "Enable_MSAA", "Enable MSAA"),
				  NSLOCTEXT("Verifications", "Enable_MSAA_Description", "Only supported with forward shading. MSAA sample count is controlled by r. MSAACount."),
				  EVerificationLevel::Warning,
				  false
				  ){}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UEnableMultiviewObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	UEnableMultiviewObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "Enable_Multiview", "Enable Multiview"),
				  NSLOCTEXT("Verifications", "Enable_Multiview_Description", "Enable single-pass stereoscopic rendering on mobile platforms."),
				  EVerificationLevel::Error,
				  true
				  ) {}
	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UEnableMobileHDRObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	UEnableMobileHDRObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "Disable_MobileHDR", "Disable MobileHDR"),
				  NSLOCTEXT("Verifications", "Disable_MobileHDR_Description", "If true, mobile pipelines include a full post-processing pass with tonemapping. Disable this setting for a performance boost and to enable stereoscopic rendering optimizations. Changing this setting requires restarting the editor."),
				  EVerificationLevel::Warning,
				  false
				  ) {}
	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UEnableVulkanObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	UEnableVulkanObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "UseVulkan_DisplayName", "Vulkan Rendering Backend"),
				  NSLOCTEXT("Verifications", "UseVulkan_Description", "PICO recommends using Vulkan as the rendering backend for all mobile apps."),
				  EVerificationLevel::Error,
				  false
				  ) {}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UHalfPrecisionFloatObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	UHalfPrecisionFloatObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "UseHalfPrecisionFloat_DisplayName", "Half Precision Float"),
				  NSLOCTEXT("Verifications", "UseHalfPrecisionFloat_Description", "Half precision float for better performance."),
				  EVerificationLevel::Recommended,
				  true
				  ) {}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UDisableAmbientOcclusionObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	UDisableAmbientOcclusionObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "DisableAmbientOcclusion_DisplayName", "Disable Ambient Occlusion"),
				  NSLOCTEXT("Verifications", "DisableAmbientOcclusion_Description", "Ambient occlusion has performance issues. We recommend disabling it."),
				  EVerificationLevel::Recommended,
				  true
				  ) {}
	
	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UEnableOcclusionCullingObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	UEnableOcclusionCullingObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "EnableOcclusionCulling_DisplayName", "Enable Occlusion Culling"),
				  NSLOCTEXT("Verifications", "EnableOcclusionCulling_Description", "Occlusion culling can provide significant performance gains."),
				  EVerificationLevel::Recommended,
				  true
				  ) {}
	
	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UDisableMovableSpotlightShadowsObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	UDisableMovableSpotlightShadowsObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "DisableMovableSpotlightShadows_DisplayName", "Disable MovableSpotlightShadows"),
				  NSLOCTEXT("Verifications", "DisableMovableSpotlightShadows_Description", "Enabling MovableSpotlightShadows will cause rendering errors when ForwardShading is enabled"),
				  EVerificationLevel::Error,
				  true
				  ) {}
	
	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};


UCLASS(ClassGroup = Verification, Blueprintable)
class UDisableOpenXRPluginObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	UDisableOpenXRPluginObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "DisableOpenXRPlugin_DisplayName", "Disable OpenXR Plugin"),
				  NSLOCTEXT("Verifications", "DisableOpenXRPlugin_Description", "There is a conflict between the OpenXR Plugin and the PICO XR Plugin, which needs to be closed."),
				  EVerificationLevel::Error,
				  true
				  ) {}
	
	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
private:
	FString PluginName = "OpenXR";
	bool bApplied = false;
};

constexpr int32 MinAndroidAPILevel = 29;
constexpr int32 TargetAndroidAPILevel = 29;
UCLASS(ClassGroup = Verification, Blueprintable)
class USetAndroidSDKMinimumObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	USetAndroidSDKMinimumObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "SetAndroidSDKMinimum_DisplayName", "Android SDK Minimum Version"),
				  FText::Format(NSLOCTEXT("Verifications", "SetAndroidSDKMinimum_Description","PICO XR SDK targeting minimum Android10.0 is required or Android 10.0 API Level,Minimum Android API level must be at least {0}."),MinAndroidAPILevel),
				  EVerificationLevel::Error,
				  false
				  ) {}
	
	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class USetAndroidSDKTargetObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	USetAndroidSDKTargetObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "SetAndroidSDKTarget_DisplayName", "Android SDK Target Version"),
				  FText::Format(NSLOCTEXT("Verifications", "SetAndroidSDKTarget_Description","Target Android API level must be at least {0}."),TargetAndroidAPILevel),
				  EVerificationLevel::Error,
				  false
				  ) {}
	
	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class USetArm64CPUObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	USetArm64CPUObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "SetArm64CPU_DisplayName", "Arm64 CPU Architecture"),
				  NSLOCTEXT("Verifications", "SetArm64CPU_Description","Requires 64-bit applications."),
				  EVerificationLevel::Error,
				  false
				  ) {}
	
	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UDisableTouchInterfaceObject : public UVerificationObject
{
	GENERATED_BODY()
public:
	UDisableTouchInterfaceObject()
			: UVerificationObject(
				  NSLOCTEXT("Verifications", "DisableTouchInterface_DisplayName", "Disable Touch Interface"),
				  NSLOCTEXT("Verifications", "DisableTouchInterface_Description","Touch Interface is not properly compatible with VR inputs."),
				  EVerificationLevel::Error,
				  false
				  ) {}
	
	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};