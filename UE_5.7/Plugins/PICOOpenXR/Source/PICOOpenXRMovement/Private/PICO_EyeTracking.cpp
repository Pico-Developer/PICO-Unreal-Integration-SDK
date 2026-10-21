// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_EyeTracking.h"
#include "PICO_MovementModule.h"
#include "OpenXRCore.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"
#include "IOpenXRHMDModule.h"
#include "EyeTrackerFunctionLibrary.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#endif

FEyeTrackingPICO::FEyeTrackingPICO()
{
}

bool FEyeTrackingPICO::IsEyeTrackerSupported(bool& Supported)
{
	Supported = bIsEyeTrackerSupported;
	return bIsEyeTrackerSupported;
}

void FEyeTrackingPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_PICO_EYE_TRACKER_EXTENSION_NAME);
}

void FEyeTrackingPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateEyeTrackerPICO", (PFN_xrVoidFunction*)&xrCreateEyeTrackerPICO));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyEyeTrackerPICO", (PFN_xrVoidFunction*)&xrDestroyEyeTrackerPICO));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetEyeDataPICO", (PFN_xrVoidFunction*)&xrGetEyeDataPICO));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetEyeGazePICO", (PFN_xrVoidFunction*)&xrGetEyeGazePICO));

	bIsEyeTrackerSupported = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_EYE_TRACKER_EXTENSION_NAME);
	if (IOpenXRHMDModule::Get().IsExtensionEnabled(XR_EXT_EYE_GAZE_INTERACTION_EXTENSION_NAME))
	{
		XrSystemEyeGazeInteractionPropertiesEXT EyeGazeInteractionProperties = { XR_TYPE_SYSTEM_EYE_GAZE_INTERACTION_PROPERTIES_EXT };
		XrSystemProperties systemProperties = { XR_TYPE_SYSTEM_PROPERTIES, &EyeGazeInteractionProperties };
		XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &systemProperties));
		bIsEyeTrackerSupported = bIsEyeTrackerSupported && EyeGazeInteractionProperties.supportsEyeGazeInteraction == XR_TRUE;
	}
}

void FEyeTrackingPICO::PostCreateSession(XrSession InSession)
{
	Session = InSession;
	EyeTracker = XR_NULL_HANDLE;
	bIsEyeTrackingRunning = false;
	DisplayTime = 0;
	TrackingSpace = XR_NULL_HANDLE;
}

const void* FEyeTrackingPICO::OnBeginSession(XrSession InSession, const void* InNext)
{
	static FName SystemName(TEXT("OpenXR"));
	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
	{
		XRTrackingSystem = GEngine->XRSystem.Get();
	}
	return InNext;
}

void FEyeTrackingPICO::UpdateDeviceLocations(XrSession InSession, XrTime InDisplayTime, XrSpace InTrackingSpace)
{
	if (InSession != Session)
	{
		return;
	}

	this->DisplayTime = InDisplayTime;
	this->TrackingSpace = InTrackingSpace;
}

bool FEyeTrackingPICO::StartEyeTracking()
{
	if (bIsEyeTrackingRunning && EyeTracker != XR_NULL_HANDLE)
	{
		return true;
	}

	if (Session != XR_NULL_HANDLE && bIsEyeTrackerSupported && xrCreateEyeTrackerPICO != nullptr)
	{
		UE_LOG(PICOOpenXRMovement, Log, TEXT("Start EyeTracking!"));
		XrEyeTrackerCreateInfoPICO EyeTrackerCreateInfoPICO = { XR_TYPE_EYE_TRACKER_CREATE_INFO_PICO };
		XrResult Result = xrCreateEyeTrackerPICO(Session, &EyeTrackerCreateInfoPICO, &EyeTracker);
		if (XR_SUCCEEDED(Result) && EyeTracker != XR_NULL_HANDLE)
		{
			bIsEyeTrackingRunning = true;
			return true;
		}
	}
	return false;
}

bool FEyeTrackingPICO::StopEyeTracking()
{
	if (EyeTracker == XR_NULL_HANDLE)
	{
		bIsEyeTrackingRunning = false;
		return true;
	}

	if (xrDestroyEyeTrackerPICO != nullptr)
	{
		UE_LOG(PICOOpenXRMovement, Log, TEXT("Stop EyeTracking!"));
		XrResult Result = xrDestroyEyeTrackerPICO(EyeTracker);
		if (XR_SUCCEEDED(Result))
		{
			EyeTracker = XR_NULL_HANDLE;
			bIsEyeTrackingRunning = false;
			return true;
		}
		return false;
	}

	return false;
}

bool FEyeTrackingPICO::GetEyeTrackingData(FEyeDataPICO& LeftEye, FEyeDataPICO& RightEye, bool& bDepthValid, float& DepthConfidence, float& Depth, bool QueryGazeData, FEyeTrackerGazeData& OutGazeData)
{
	if (bIsEyeTrackingRunning && XRTrackingSystem && TrackingSpace != XR_NULL_HANDLE && xrGetEyeDataPICO != nullptr && xrGetEyeGazePICO != nullptr)
	{
		XrEyeTrackerDataPICO EyeTrackingData = { XR_TYPE_EYE_TRACKER_DATA_PICO };
		XrEyeTrackerDataInfoPICO EyeTrackerDataInfo = { XR_TYPE_EYE_TRACKER_DATA_INFO_PICO };
		XrResult Result = xrGetEyeDataPICO(EyeTracker, &EyeTrackerDataInfo, &EyeTrackingData);
		bool bOpennessValid = LeftEye.bOpennessValid = RightEye.bOpennessValid = XR_SUCCEEDED(Result);
		if (bOpennessValid)
		{
			LeftEye.Openness = EyeTrackingData.leftEyeData.openness;
			RightEye.Openness = EyeTrackingData.rightEyeData.openness;
		}

		XrEyeTrackerGazeInfoPICO EyeTrackerGazeInfoPICO = { XR_TYPE_EYE_TRACKER_GAZE_INFO_PICO };
		EyeTrackerGazeInfoPICO.baseSpace = TrackingSpace;
		EyeTrackerGazeInfoPICO.time = DisplayTime;
		EyeTrackerGazeInfoPICO.flags = XR_EYE_TRACKER_LEFT_BIT_PICO | XR_EYE_TRACKER_RIGHT_BIT_PICO;
		XrEyeTrackerGazeDepthPICO EyeGazeDepthPICO = { XR_TYPE_EYE_TRACKER_GAZE_DEPTH_PICO };
		XrEyeTrackerGazePICO EyeGazePICO = { XR_TYPE_EYE_TRACKER_GAZE_PICO, &EyeGazeDepthPICO };

		Result = xrGetEyeGazePICO(EyeTracker, &EyeTrackerGazeInfoPICO, &EyeGazePICO);
		if (XR_SUCCEEDED(Result))
		{
			const float WorldToMeters = XRTrackingSystem->GetWorldToMetersScale();
			const FTransform& TrackingToWorldTransform = XRTrackingSystem->GetTrackingToWorldTransform();

			LeftEye.bPoseValid = EyeGazePICO.leftEyeGaze.isValid == XR_TRUE;
			if (LeftEye.bPoseValid)
			{
				const XrPosef& Pose = EyeGazePICO.leftEyeGaze.pose;
				const FTransform EyeTrackerTransform = ToFTransform(Pose, WorldToMeters);
				const FTransform EyeTransform = EyeTrackerTransform * TrackingToWorldTransform;
				LeftEye.Location = EyeTransform.GetLocation();
				LeftEye.Rotation = EyeTransform.GetRotation().Rotator();
			}

			RightEye.bPoseValid = EyeGazePICO.rightEyeGaze.isValid == XR_TRUE;
			if (RightEye.bPoseValid)
			{
				const XrPosef& Pose = EyeGazePICO.rightEyeGaze.pose;
				const FTransform EyeTrackerTransform = ToFTransform(Pose, WorldToMeters);
				const FTransform EyeTransform = EyeTrackerTransform * TrackingToWorldTransform;
				RightEye.Location = EyeTransform.GetLocation();
				RightEye.Rotation = EyeTransform.GetRotation().Rotator();
			}

			bDepthValid = EyeGazeDepthPICO.isValid == XR_TRUE;
			if(bDepthValid)
			{
				DepthConfidence = EyeGazeDepthPICO.depthConfidence;
				Depth = EyeGazeDepthPICO.gazeDepth * WorldToMeters;
			}

			return QueryGazeData ? UEyeTrackerFunctionLibrary::GetGazeData(OutGazeData) : true;
		}
		else
		{
			LeftEye.bPoseValid = RightEye.bPoseValid = false;
			bDepthValid = false;
		}
	}
	return false;
}
