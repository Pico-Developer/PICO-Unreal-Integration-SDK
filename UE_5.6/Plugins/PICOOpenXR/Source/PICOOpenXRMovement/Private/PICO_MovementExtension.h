// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IOpenXRExtensionPlugin.h"
#include "PICO_FaceTracking.h"
#include "PICO_BodyTracking.h"
#include "PICO_MotionTracking.h"
#include "PICO_EyeTracking.h"
#include "PICO_ExpandDevice.h"

class FMovementExtensionPICO : public IOpenXRExtensionPlugin
{
public:
	FMovementExtensionPICO();
	virtual ~FMovementExtensionPICO() override {}

	void Register();
	void Unregister();

	/** IOpenXRExtensionPlugin */
	virtual FString GetDisplayName() override
	{
		return FString(TEXT("MovementExtensionPICO"));
	}
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual bool GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual const void* OnGetSystem(XrInstance InInstance, const void* InNext) override;
	virtual void PostGetSystem(XrInstance InInstance, XrSystemId InSystem) override;
	virtual void PostCreateSession(XrSession InSession) override;
	virtual const void* OnBeginSession(XrSession InSession, const void* InNext) override;
	virtual void OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader) override;
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace) override;

	// Accessors for individual extension instances
	FFaceTrackingPICO& GetFaceTrackingPICO() { return FaceTrackingPICO; }
	FBodyTrackingPICO& GetBodyTrackingPICO() { return BodyTrackingPICO; }
	FMotionTrackingPICO& GetMotionTrackingPICO() { return MotionTrackingPICO; }
	FEyeTrackingPICO& GetEyeTrackingPICO() { return EyeTrackingPICO; }
	FExpandDevicePICO& GetExpandDevicePICO() { return ExpandDevicePICO; }

private:
	template <typename T>
	static void TryActivateSubmodule(bool bIsEnabled, T& ExtensionPlugin, bool& bIsActive, TArray<const ANSICHAR*>& OutExtensions);

	template <typename T>
	static void AppendActiveSubmoduleOptionalExtensions(bool bIsActive, T& ExtensionPlugin, TArray<const ANSICHAR*>& OutExtensions);

	FFaceTrackingPICO FaceTrackingPICO;
	FBodyTrackingPICO BodyTrackingPICO;
	FMotionTrackingPICO MotionTrackingPICO;
	FEyeTrackingPICO EyeTrackingPICO;
	FExpandDevicePICO ExpandDevicePICO;

	bool bFaceTrackingActive = false;
	bool bBodyTrackingActive = false;
	bool bMotionTrackingActive = false;
	bool bEyeTrackingActive = false;
	bool bExpandDeviceActive = false;
};
