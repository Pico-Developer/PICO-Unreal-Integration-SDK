// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_MovementFunctionLibrary.h"

class FBodyTrackingPICO
{
public:
	FBodyTrackingPICO();
	virtual ~FBodyTrackingPICO() {}

	void GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions);
	void GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions);
	const void* OnGetSystem(XrInstance InInstance, const void* InNext);
	void PostGetSystem(XrInstance InInstance, XrSystemId InSystem);
	void PostCreateSession(XrSession InSession);
	void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace);
	
	bool TryGetBodyState(FBodyStatePICO& outBodyState, float WorldToMeters = 100.0f, bool QueryAcc = false, bool QueryVel = false, bool QueryPostureFlag = false);
	bool IsBodyTrackingEnabled();
	bool IsBodyTrackingSupported();
	bool StartBodyTracking(EBodyTrackingModePICO Mode);
	bool StopBodyTracking();

	bool StartBodyTrackingCalibApp();
	bool GetBodyTrackingState(EBodyTrackingStatusPICO& Status, EBodyTrackingErrorCodePICO& Error);

private:

	PFN_xrCreateBodyTrackerBD xrCreateBodyTrackerBD = nullptr;
	PFN_xrDestroyBodyTrackerBD xrDestroyBodyTrackerBD = nullptr;
	PFN_xrLocateBodyJointsBD xrLocateBodyJointsBD = nullptr;
	bool bCurrentDeviceSupportBodyTracking = false;
	XrInstance Instance = XR_NULL_HANDLE;
	XrSession Session = XR_NULL_HANDLE;
	XrBodyTrackerBD BodyTracker = XR_NULL_HANDLE;
	bool bBodyTrackerIsRunning = false;
	bool bSupportBodyTracking2 = false;

	XrTime PredictedTime = 0;
	XrSpace BaseSpace = XR_NULL_HANDLE;
	TArray<XrBodyJointLocationBD> Locations;
	TArray<XrBodyTrackingPosturePICO> PostureFlags;
	TArray<XrBodyJointVelocityPICO> Velocities;
	TArray<XrBodyJointAccelerationPICO> Accelerations;
	PFN_xrStartBodyTrackingCalibrationAppPICO xrStartBodyTrackingCalibrationAppPICO = nullptr;
	PFN_xrGetBodyTrackingStatePICO xrGetBodyTrackingStatePICO = nullptr;
};
