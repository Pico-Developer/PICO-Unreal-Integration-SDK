// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_MovementFunctionLibrary.h"

class FMotionTrackingPICO
{
public:
	FMotionTrackingPICO();
	virtual ~FMotionTrackingPICO() {}

	void GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions);
	const void* OnGetSystem(XrInstance InInstance, const void* InNext);
	void PostCreateSession(XrSession InSession);
	void OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader);
	void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace);

	bool RequestMotionTrackerDevice(int DeviceCount);
	bool GetMotionTrackerBatteryState(int64 ID, float& BatteryLevel, EChargingStatePICO& State);
	bool LocateMotionTracker(int64 ID, FRotator& OutRotation, FVector& OutPosition, FVector& OutLinearVelocity, FVector& OutAngularVelocity, FVector& OutLinearAcceleration, FVector& OutAngularAcceleration, bool GetVelAndAcc = false);

private:
	class IXRTrackingSystem* XRTrackingSystem = nullptr;
	XrSession Session = XR_NULL_HANDLE;
	XrTime PredictedTime = 0;
	XrSpace BaseSpace = XR_NULL_HANDLE;
	bool bSupportMotionTracking = false;

	PFN_xrRequestMotionTrackerDevicePICO xrRequestMotionTrackerDevicePICO = nullptr;
	PFN_xrGetMotionTrackerBatteryStatePICO xrGetMotionTrackerBatteryStatePICO = nullptr;
	PFN_xrLocateMotionTrackerPICO xrLocateMotionTrackerPICO = nullptr;
};
