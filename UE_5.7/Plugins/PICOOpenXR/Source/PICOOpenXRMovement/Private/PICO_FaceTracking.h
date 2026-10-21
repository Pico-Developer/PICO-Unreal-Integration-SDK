// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "PICO_MovementFunctionLibrary.h"
#include "CoreMinimal.h"

class FFaceTrackingPICO
{
public:
	FFaceTrackingPICO();
	virtual ~FFaceTrackingPICO();

	void GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions);
	const void* OnGetSystem(XrInstance InInstance, const void* InNext);
	void PostGetSystem(XrInstance InInstance, XrSystemId InSystem);
	void PostCreateSession(XrSession InSession);
	void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace);

	bool GetFaceTrackingSupported(TArray<EFaceTrackingModePICO>& Modes);
	bool StartFaceTracking(EFaceTrackingModePICO Mode);
	bool StopFaceTracking();
	bool SetFaceTrackingCurrentMode(EFaceTrackingModePICO Mode);
	bool GetFaceTrackingCurrentMode(EFaceTrackingModePICO& Mode);
	bool GetFaceTrackingData(int64 DisplayTime, FFaceStatePICO& outState);

private:
	bool bSupportsFaceTracking = false;
	bool bFaceTrackingFunctionsAvailable = false;

	XrSession Session;
	XrTime Time;
	int32 FaceTrackerRefCount = 0;
	EFaceTrackingModePICO FaceTrackingMode = EFaceTrackingModePICO::Default;

	PFN_xrEnumerateFacialSimulationModesBD xrEnumerateFacialSimulationModesBD = nullptr;
	PFN_xrCreateFaceTrackerBD xrCreateFaceTrackerBD = nullptr;
	PFN_xrDestroyFaceTrackerBD xrDestroyFaceTrackerBD = nullptr;
	PFN_xrGetFacialSimulationDataBD xrGetFacialSimulationDataBD = nullptr;
	PFN_xrSetFacialSimulationModeBD xrSetFacialSimulationModeBD = nullptr;
	PFN_xrGetFacialSimulationModeBD xrGetFacialSimulationModeBD = nullptr;

	XrFaceTrackerBD FaceTracker = XR_NULL_HANDLE;
};
