// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_MRExtension.h"
#include "IOpenXRHMDModule.h"
#include "OpenXRCore.h"
#include "PICOOpenXRRuntimeSettings.h"
#include <initializer_list>

namespace
{
	bool AreExtensionsAvailable(const TArray<const ANSICHAR*>& Extensions)
	{
		if (!IOpenXRHMDModule::IsAvailable())
		{
			return true;
		}

		const IOpenXRHMDModule& OpenXRHMDModule = IOpenXRHMDModule::Get();
		for (const ANSICHAR* Extension : Extensions)
		{
			if (!OpenXRHMDModule.IsExtensionAvailable(UTF8_TO_TCHAR(Extension)))
			{
				return false;
			}
		}

		return true;
	}

	bool IsSettingEnabled(const TCHAR* ConfigKey)
	{
		return UPICOOpenXRRuntimeSettings::GetBoolConfigByKey(ConfigKey);
	}

	bool HasAnyEnabledSubmodule(bool bEnablePlane, bool bEnableMesh, bool bEnableAnchor, bool bEnableSceneCapture,bool bEnableLightEstimation)
	{
		return bEnablePlane || bEnableMesh || bEnableAnchor || bEnableSceneCapture||bEnableLightEstimation;
	}

	bool HasAnyActiveSubmodule(bool bSpatialPlaneActive, bool bSpatialMeshActive, bool bSpatialAnchorActive, bool bSceneCaptureActive,bool bLightEstimationActive)
	{
		return bSpatialPlaneActive || bSpatialMeshActive || bSpatialAnchorActive || bSceneCaptureActive||bLightEstimationActive;
	}
}

FMRModuleExtensionPICO::FMRModuleExtensionPICO()
{
}

void FMRModuleExtensionPICO::Register()
{
	RegisterOpenXRExtensionModularFeature();
}

void FMRModuleExtensionPICO::Unregister()
{
	UnregisterOpenXRExtensionModularFeature();
}

template <typename T>
void FMRModuleExtensionPICO::TryActivateSubmodule(bool bIsEnabled, T& ExtensionPlugin, bool& bIsActive, TArray<const ANSICHAR*>& OutExtensions)
{
	if (!bIsEnabled)
	{
		return;
	}

	TArray<const ANSICHAR*> RequiredExtensions;
	ExtensionPlugin.GetRequiredExtensions(RequiredExtensions);

	if (!AreExtensionsAvailable(RequiredExtensions))
	{
		return;
	}

	OutExtensions.Append(RequiredExtensions);
	bIsActive = true;
}

bool FMRModuleExtensionPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	bSpatialSensingActive = false;
	bSpatialPlaneActive = false;
	bSpatialMeshActive = false;
	bLightEstimationActive = false;
	bSpatialAnchorActive = false;
	bSceneCaptureActive = false;
	bSpatialAnchorSharingRequested = false;

	const bool bEnablePlane = IsSettingEnabled(TEXT("bEnablePlane"));
	const bool bEnableMesh = IsSettingEnabled(TEXT("bEnableMesh"));
	const bool bEnableAnchor = IsSettingEnabled(TEXT("bEnableAnchor"));
	const bool bEnableCloudAnchor = bEnableAnchor && IsSettingEnabled(TEXT("bEnableCloudAnchor"));
	const bool bEnableLightEstimation = IsSettingEnabled(TEXT("bEnableLightEstimation"));
	const bool bEnableSceneCapture = IsSettingEnabled(TEXT("bEnableSceneCapture"));

	if (!HasAnyEnabledSubmodule(bEnablePlane, bEnableMesh, bEnableAnchor, bEnableSceneCapture,bEnableLightEstimation))
	{
		return false;
	}

	TArray<const ANSICHAR*> BaseRequiredExtensions;
	SpatialSensingPICO.GetRequiredExtensions(BaseRequiredExtensions);
	if (!AreExtensionsAvailable(BaseRequiredExtensions))
	{
		return false;
	}

	OutExtensions.Append(BaseRequiredExtensions);

	TryActivateSubmodule(bEnablePlane, SpatialPlanePICO, bSpatialPlaneActive, OutExtensions);
	TryActivateSubmodule(bEnableMesh, SpatialMeshPICO, bSpatialMeshActive, OutExtensions);
	TryActivateSubmodule(bEnableAnchor, SpatialAnchorPICO, bSpatialAnchorActive, OutExtensions);
	TryActivateSubmodule(bEnableSceneCapture, SceneCapturePICO, bSceneCaptureActive, OutExtensions);
	TryActivateSubmodule(bEnableLightEstimation, LightEstimationPICO, bLightEstimationActive, OutExtensions);

	if (bSpatialAnchorActive && bEnableCloudAnchor)
	{
		TArray<const ANSICHAR*> OptionalExtensions;
		SpatialAnchorPICO.GetOptionalExtensions(OptionalExtensions);
		bSpatialAnchorSharingRequested = AreExtensionsAvailable(OptionalExtensions);
	}

	bSpatialSensingActive = HasAnyActiveSubmodule(bSpatialPlaneActive, bSpatialMeshActive, bSpatialAnchorActive, bSceneCaptureActive,bLightEstimationActive);
	if (!bSpatialSensingActive)
	{
		return false;
	}

	return true;
}

bool FMRModuleExtensionPICO::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (bSpatialAnchorActive && bSpatialAnchorSharingRequested)
	{
		SpatialAnchorPICO.GetOptionalExtensions(OutExtensions);
	}

	return true;
}

void FMRModuleExtensionPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	if (bSpatialSensingActive)
	{
		SpatialSensingPICO.PostGetSystem(InInstance, InSystem);
	}

	if (bSpatialPlaneActive)
	{
		SpatialPlanePICO.PostGetSystem(InInstance, InSystem);
	}

	if (bSpatialMeshActive)
	{
		SpatialMeshPICO.PostGetSystem(InInstance, InSystem);
	}

	if (bSpatialAnchorActive)
	{
		SpatialAnchorPICO.PostGetSystem(InInstance, InSystem);
	}

	if (bSceneCaptureActive)
	{
		SceneCapturePICO.PostGetSystem(InInstance, InSystem);
	}

	if (bLightEstimationActive)
	{
		LightEstimationPICO.PostGetSystem(InInstance, InSystem);
	}
}

void FMRModuleExtensionPICO::PostCreateSession(XrSession InSession)
{
	if (bSpatialSensingActive)
	{
		SpatialSensingPICO.PostCreateSession(InSession);
	}

	if (bSpatialPlaneActive)
	{
		SpatialPlanePICO.PostCreateSession(InSession);
	}

	if (bSpatialMeshActive)
	{
		SpatialMeshPICO.PostCreateSession(InSession);
	}

	if (bSpatialAnchorActive)
	{
		SpatialAnchorPICO.PostCreateSession(InSession);
	}

	if (bSceneCaptureActive)
	{
		SceneCapturePICO.PostCreateSession(InSession);
	}

	if (bLightEstimationActive)
	{
		LightEstimationPICO.PostCreateSession(InSession);
	}
}

void FMRModuleExtensionPICO::OnDestroySession(XrSession InSession)
{
	if (bSpatialPlaneActive)
	{
		SpatialPlanePICO.OnDestroySession(InSession);
	}

	if (bSpatialMeshActive)
	{
		SpatialMeshPICO.OnDestroySession(InSession);
	}

	if (bSpatialAnchorActive)
	{
		SpatialAnchorPICO.OnDestroySession(InSession);
	}

	if (bSceneCaptureActive)
	{
		SceneCapturePICO.OnDestroySession(InSession);
	}

	if (bSpatialSensingActive)
	{
		SpatialSensingPICO.OnDestroySession(InSession);
	}

	if (bLightEstimationActive)
	{
		LightEstimationPICO.OnDestroySession(InSession);
	}
}

void FMRModuleExtensionPICO::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
{
	if (bSpatialSensingActive)
	{
		SpatialSensingPICO.OnEvent(InSession, InHeader);
	}
}

void FMRModuleExtensionPICO::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
	if (bSpatialSensingActive)
	{
		SpatialSensingPICO.UpdateDeviceLocations(InSession, DisplayTime, TrackingSpace);
	}

	if (bSpatialPlaneActive)
	{
		SpatialPlanePICO.UpdateDeviceLocations(InSession, DisplayTime, TrackingSpace);
	}

	if (bSpatialMeshActive)
	{
		SpatialMeshPICO.UpdateDeviceLocations(InSession, DisplayTime, TrackingSpace);
	}

	if (bSpatialAnchorActive)
	{
		SpatialAnchorPICO.UpdateDeviceLocations(InSession, DisplayTime, TrackingSpace);
	}

	if (bSceneCaptureActive)
	{
		SceneCapturePICO.UpdateDeviceLocations(InSession, DisplayTime, TrackingSpace);
	}

	if (bLightEstimationActive)
	{
		LightEstimationPICO.UpdateDeviceLocations(InSession, DisplayTime, TrackingSpace);
	}
}
