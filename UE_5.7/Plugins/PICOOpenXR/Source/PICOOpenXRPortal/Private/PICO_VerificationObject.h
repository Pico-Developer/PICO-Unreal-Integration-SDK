// Copyright® PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PICO_PortalTypes.h"
#include "PICO_VerificationObject.generated.h"
/**
 * 
 */
UCLASS(ClassGroup = Verification, Blueprintable)
class UPICOVerificationObject : public UObject
{
	GENERATED_BODY()

public:
	UPICOVerificationObject();
	UPICOVerificationObject(const FText& InDisplayName,
	                    const FText& InDescription,
	                    const EVerificationLevelPICO& InVerificationLevel,
	                    bool InNeedRestartEditor);

	UFUNCTION(BlueprintCallable, Category="Verification")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintCallable, Category="Verification")
	FText GetDescription() const { return Description; }

	virtual void PerformVerificationFix();

	virtual bool IsVerificationFixed();

	virtual bool RequiresEditorRestart();

	inline EVerificationLevelPICO GetLevel() const { return Level; }

private:
	FText DisplayName;
	FText Description;

protected:
	EVerificationLevelPICO Level;
	bool bNeedRestartEditor;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICOSetMSAAObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICOSetMSAAObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "Enable_MSAA", "Enable MSAA"),
			NSLOCTEXT("Verifications", "Enable_MSAA_Description", "Only supported with forward shading. MSAA sample count is controlled by r. MSAACount."),
			EVerificationLevelPICO::Warning,
			false
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICOEnableMultiviewObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICOEnableMultiviewObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "Enable_Multiview", "Enable Multiview"),
			NSLOCTEXT("Verifications", "Enable_Multiview_Description", "Enable single-pass stereoscopic rendering on mobile platforms."),
			EVerificationLevelPICO::Error,
			true
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICOEnableMobileHDRObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICOEnableMobileHDRObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "Disable_MobileHDR", "Disable MobileHDR"),
			NSLOCTEXT("Verifications", "Disable_MobileHDR_Description",
			          "If true, mobile pipelines include a full post-processing pass with tonemapping. Disable this setting for a performance boost and to enable stereoscopic rendering optimizations. Changing this setting requires restarting the editor."),
			EVerificationLevelPICO::Warning,
			true
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICOEnableVulkanObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICOEnableVulkanObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "UseVulkan_DisplayName", "Vulkan Rendering Backend"),
			NSLOCTEXT("Verifications", "UseVulkan_Description", "PICO recommends using Vulkan as the rendering backend for all mobile apps."),
			EVerificationLevelPICO::Error,
			false
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICOHalfPrecisionFloatObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICOHalfPrecisionFloatObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "UseHalfPrecisionFloat_DisplayName", "Half Precision Float"),
			NSLOCTEXT("Verifications", "UseHalfPrecisionFloat_Description", "Half precision float for better performance."),
			EVerificationLevelPICO::Recommended,
			true
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICODisableAmbientOcclusionObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICODisableAmbientOcclusionObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "DisableAmbientOcclusion_DisplayName", "Disable Ambient Occlusion"),
			NSLOCTEXT("Verifications", "DisableAmbientOcclusion_Description", "Ambient occlusion has performance issues. We recommend disabling it."),
			EVerificationLevelPICO::Recommended,
			true
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICOEnableOcclusionCullingObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICOEnableOcclusionCullingObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "EnableOcclusionCulling_DisplayName", "Enable Occlusion Culling"),
			NSLOCTEXT("Verifications", "EnableOcclusionCulling_Description", "Occlusion culling can provide significant performance gains."),
			EVerificationLevelPICO::Recommended,
			true
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICODisableMovableSpotlightShadowsObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICODisableMovableSpotlightShadowsObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "DisableMovableSpotlightShadows_DisplayName", "Disable MovableSpotlightShadows"),
			NSLOCTEXT("Verifications", "DisableMovableSpotlightShadows_Description", "Enabling MovableSpotlightShadows will cause rendering errors when ForwardShading is enabled"),
			EVerificationLevelPICO::Error,
			true
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};


UCLASS(ClassGroup = Verification, Blueprintable)
class UPICODisablePICOXRPluginObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICODisablePICOXRPluginObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "DisablePICOXRPlugin_DisplayName", "Disable PICO XR Plugin"),
			NSLOCTEXT("Verifications", "DisablePICOXRPlugin_Description", "There is a conflict between the PICO XR Plugin and the OpenXR Plugin, which needs to be closed."),
			EVerificationLevelPICO::Error,
			true
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;

private:
	FString PluginName = "PICOXR";
	bool bApplied = false;
};

constexpr int32 MinAndroidAPILevel = 29;
constexpr int32 TargetAndroidAPILevel = 29;

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICOSetAndroidSDKMinimumObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICOSetAndroidSDKMinimumObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "SetAndroidSDKMinimum_DisplayName", "Android SDK Minimum Version"),
			FText::Format(NSLOCTEXT("Verifications", "SetAndroidSDKMinimum_Description",
			                        "PICO XR SDK targeting minimum Android10.0 is required or Android 10.0 API Level,Minimum Android API level must be at least {0}."), MinAndroidAPILevel),
			EVerificationLevelPICO::Error,
			false
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICOSetAndroidSDKTargetObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICOSetAndroidSDKTargetObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "SetAndroidSDKTarget_DisplayName", "Android SDK Target Version"),
			FText::Format(NSLOCTEXT("Verifications", "SetAndroidSDKTarget_Description", "Target Android API level must be at least {0}."), TargetAndroidAPILevel),
			EVerificationLevelPICO::Error,
			false
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICOSetArm64CPUObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICOSetArm64CPUObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "SetArm64CPU_DisplayName", "Arm64 CPU Architecture"),
			NSLOCTEXT("Verifications", "SetArm64CPU_Description", "Requires 64-bit applications."),
			EVerificationLevelPICO::Error,
			false
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};

UCLASS(ClassGroup = Verification, Blueprintable)
class UPICODisableTouchInterfaceObject : public UPICOVerificationObject
{
	GENERATED_BODY()

public:
	UPICODisableTouchInterfaceObject()
		: UPICOVerificationObject(
			NSLOCTEXT("Verifications", "DisableTouchInterface_DisplayName", "Disable Touch Interface"),
			NSLOCTEXT("Verifications", "DisableTouchInterface_Description", "Touch Interface is not properly compatible with VR inputs."),
			EVerificationLevelPICO::Error,
			false
		)
	{
	}

	virtual void PerformVerificationFix() override;
	virtual bool IsVerificationFixed() override;
};
