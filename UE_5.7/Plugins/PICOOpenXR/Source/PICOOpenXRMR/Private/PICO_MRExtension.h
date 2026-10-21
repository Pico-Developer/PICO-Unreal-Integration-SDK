// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IOpenXRExtensionPlugin.h"
#include "PICO_LightEstimation.h"
#include "PICO_MRTypes.h"
#include "PICO_MR.h"
#include "PICO_SpatialPlane.h"
#include "PICO_SpatialMesh.h"
#include "PICO_SpatialAnchor.h"
#include "PICO_SceneCapture.h"

class FMRModuleExtensionPICO : public IOpenXRExtensionPlugin
{
public:
	FMRModuleExtensionPICO();
	virtual ~FMRModuleExtensionPICO() override {}

	void Register();
	void Unregister();

	/** IOpenXRExtensionPlugin */
	virtual FString GetDisplayName() override
	{
		return FString(TEXT("MRModuleExtensionPICO"));
	}
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual bool GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual void PostGetSystem(XrInstance InInstance, XrSystemId InSystem) override;
	virtual void PostCreateSession(XrSession InSession) override;
	virtual void OnDestroySession(XrSession InSession) override;
	virtual void OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader) override;
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace) override;

	// Accessors for individual extension instances
	FSpatialSensingExtensionPICO& GetSpatialSensingPICO() { return SpatialSensingPICO; }
	FSpatialPlaneExtensionPICO& GetSpatialPlanePICO() { return SpatialPlanePICO; }
	FSpatialMeshExtensionPICO& GetSpatialMeshPICO() { return SpatialMeshPICO; }
	FSpatialAnchorExtensionPICO& GetSpatialAnchorPICO() { return SpatialAnchorPICO; }
	FSceneCaptureExtensionPICO& GetSceneCapturePICO() { return SceneCapturePICO; }
	FLightEstimationExtensionPICO& GetLightEstimationPICO() { return LightEstimationPICO; }

private:
	template <typename T>
	static void TryActivateSubmodule(bool bIsEnabled, T& ExtensionPlugin, bool& bIsActive, TArray<const ANSICHAR*>& OutExtensions);

	FSpatialSensingExtensionPICO SpatialSensingPICO;
	FSpatialPlaneExtensionPICO SpatialPlanePICO;
	FSpatialMeshExtensionPICO SpatialMeshPICO;
	FSpatialAnchorExtensionPICO SpatialAnchorPICO;
	FSceneCaptureExtensionPICO SceneCapturePICO;
	FLightEstimationExtensionPICO LightEstimationPICO;

	bool bSpatialSensingActive = false;
	bool bSpatialPlaneActive = false;
	bool bSpatialMeshActive = false;
	bool bSpatialAnchorActive = false;
	bool bSceneCaptureActive = false;
	bool bLightEstimationActive = false;

	bool bSpatialAnchorSharingRequested = false;
};
