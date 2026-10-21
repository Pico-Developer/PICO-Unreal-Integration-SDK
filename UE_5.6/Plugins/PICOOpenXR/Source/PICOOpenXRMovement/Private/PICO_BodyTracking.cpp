// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_BodyTracking.h"
#include "PICO_MovementModule.h"
#include "OpenXRCore.h"
#include "IOpenXRHMDModule.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#endif

FBodyTrackingPICO::FBodyTrackingPICO()
{
	Locations.SetNum(XR_BODY_JOINT_COUNT_BD);
	PostureFlags.SetNum(XR_BODY_JOINT_COUNT_BD);
	Velocities.SetNum(XR_BODY_JOINT_COUNT_BD);
	Accelerations.SetNum(XR_BODY_JOINT_COUNT_BD);
}

void FBodyTrackingPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_BD_BODY_TRACKING_EXTENSION_NAME);
}

void FBodyTrackingPICO::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_PICO_BODY_TRACKING2_EXTENSION_NAME);
}

const void* FBodyTrackingPICO::OnGetSystem(XrInstance InInstance, const void* InNext)
{
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateBodyTrackerBD", (PFN_xrVoidFunction*)&xrCreateBodyTrackerBD));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyBodyTrackerBD", (PFN_xrVoidFunction*)&xrDestroyBodyTrackerBD));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrLocateBodyJointsBD", (PFN_xrVoidFunction*)&xrLocateBodyJointsBD));
	
	bSupportBodyTracking2 = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_BODY_TRACKING2_EXTENSION_NAME);
	if (bSupportBodyTracking2)
	{
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrStartBodyTrackingCalibrationAppPICO", (PFN_xrVoidFunction*)&xrStartBodyTrackingCalibrationAppPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetBodyTrackingStatePICO", (PFN_xrVoidFunction*)&xrGetBodyTrackingStatePICO));
	}
	Instance = InInstance;
	return InNext;
}

void FBodyTrackingPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	XrSystemBodyTrackingPropertiesBD BodyTrackingSystemProperties = { (XrStructureType)XR_TYPE_SYSTEM_BODY_TRACKING_PROPERTIES_BD };
	XrSystemProperties systemProperties{ XR_TYPE_SYSTEM_PROPERTIES,&BodyTrackingSystemProperties };
	XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &systemProperties));
	bCurrentDeviceSupportBodyTracking = BodyTrackingSystemProperties.supportsBodyTracking == XR_TRUE;
}

void FBodyTrackingPICO::PostCreateSession(XrSession InSession)
{
	Session = InSession;
	PredictedTime = 0;
	BaseSpace = XR_NULL_HANDLE;
}

void FBodyTrackingPICO::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
	PredictedTime = DisplayTime;
	BaseSpace = TrackingSpace;
}

bool FBodyTrackingPICO::TryGetBodyState(FBodyStatePICO& outBodyState, float WorldToMeters, bool QueryAcc, bool QueryVel, bool QueryPostureFlag)
{
	if (bBodyTrackerIsRunning && xrLocateBodyJointsBD != nullptr && BodyTracker != XR_NULL_HANDLE && BaseSpace != XR_NULL_HANDLE)
	{
		XrBodyJointsLocateInfoBD GetInfo = { (XrStructureType)XR_TYPE_BODY_JOINTS_LOCATE_INFO_BD };
		GetInfo.baseSpace = BaseSpace;
		GetInfo.time = PredictedTime;

		outBodyState.IsActive = false;
		outBodyState.BaseJointsData.SetNum(Locations.Num());
		XrBodyJointLocationsBD BodyData = { (XrStructureType)XR_TYPE_BODY_JOINT_LOCATIONS_BD };
		BodyData.jointLocationCount = Locations.Num();
		BodyData.jointLocations = Locations.GetData();

		if (QueryAcc)
		{
			outBodyState.Accelerations.SetNum(Accelerations.Num());
			const void* Next = BodyData.next;
			XrBodyJointAccelerationsPICO accelerations{ XR_TYPE_BODY_JOINT_ACCELERATIONS_PICO };
			accelerations.jointCount = Accelerations.Num();
			accelerations.jointAccelerations = Accelerations.GetData();
			accelerations.next = &Next;
			BodyData.next = &accelerations;
		}

		if (QueryVel)
		{
			outBodyState.Velocities.SetNum(Velocities.Num());
			const void* Next = BodyData.next;
			XrBodyJointVelocitiesPICO velocities{ XR_TYPE_BODY_JOINT_VELOCITIES_PICO };
			velocities.jointCount = Velocities.Num();
			velocities.jointVelocities = Velocities.GetData();
			velocities.next = &Next;
			BodyData.next = &velocities;
		}

		if (QueryPostureFlag)
		{
			outBodyState.PostureFlags.SetNum(PostureFlags.Num());
			const void* Next = BodyData.next;
			XrBodyTrackingPostureFlagsDataPICO flags{ XR_TYPE_BODY_TRACKING_POSTURE_FLAGS_DATA_PICO };
			flags.jointCount = PostureFlags.Num();
			flags.postureFlag = PostureFlags.GetData();
			flags.next = &Next;
			BodyData.next = &flags;
		}

		if (XR_SUCCEEDED(xrLocateBodyJointsBD(BodyTracker, &GetInfo, &BodyData)))
		{
			outBodyState.IsActive = BodyData.allJointPosesTracked == XR_TRUE;

			if (outBodyState.IsActive)
			{
				for (int i = 0; i < XR_BODY_JOINT_COUNT_BD; ++i)
				{
					outBodyState.BaseJointsData[i].Joint = (EBodyJointPICO)i;
					outBodyState.BaseJointsData[i].bIsValid = Locations[i].locationFlags & (XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_POSITION_VALID_BIT);
					outBodyState.BaseJointsData[i].Orientation = FRotator(ToFQuat(Locations[i].pose.orientation));
					outBodyState.BaseJointsData[i].Position = ToFVector(Locations[i].pose.position, WorldToMeters);

					if (QueryAcc)
					{
						outBodyState.Accelerations[i].bIsLinearValid = Accelerations[i].accelerationFlags & XR_SPACE_ACCELERATION_LINEAR_VALID_BIT_PICO;
						outBodyState.Accelerations[i].bIsAngularValid = Accelerations[i].accelerationFlags & XR_SPACE_ACCELERATION_ANGULAR_VALID_BIT_PICO;
						outBodyState.Accelerations[i].LinearAcc = ToFVector(Accelerations[i].linearAcceleration, WorldToMeters);
						outBodyState.Accelerations[i].AngularAcc = ToFVector(Accelerations[i].angularAcceleration, WorldToMeters);
					}

					if (QueryVel)
					{
						outBodyState.Velocities[i].bIsLinearValid = Velocities[i].velocityFlags & XR_SPACE_VELOCITY_LINEAR_VALID_BIT;
						outBodyState.Velocities[i].bIsAngularValid = Velocities[i].velocityFlags & XR_SPACE_VELOCITY_ANGULAR_VALID_BIT;
						outBodyState.Velocities[i].LinearVel = ToFVector(Velocities[i].linearVelocity, WorldToMeters);
						outBodyState.Velocities[i].AngularVel = ToFVector(Velocities[i].angularVelocity, WorldToMeters);
					}

					if (QueryPostureFlag)
					{
						outBodyState.PostureFlags[i].bStomp = PostureFlags[i] & XR_BODY_TRACKING_POSTURE_STOMP_PICO;
						outBodyState.PostureFlags[i].bStatic = PostureFlags[i] & XR_BODY_TRACKING_POSTURE_STATIC_PICO;
					}
				}
			}
			return true;
		}
	}

	outBodyState.IsActive = false;
	return false;
}

bool FBodyTrackingPICO::IsBodyTrackingEnabled()
{
	return bBodyTrackerIsRunning;
}

bool FBodyTrackingPICO::IsBodyTrackingSupported()
{
	return bCurrentDeviceSupportBodyTracking;
}

bool FBodyTrackingPICO::StartBodyTracking(EBodyTrackingModePICO Mode)
{
	if (bCurrentDeviceSupportBodyTracking && Session != XR_NULL_HANDLE && BodyTracker == XR_NULL_HANDLE && xrCreateBodyTrackerBD != nullptr)
	{
		XrBodyTrackerCreateInfoBD CreateInfo = { (XrStructureType)XR_TYPE_BODY_TRACKER_CREATE_INFO_BD };
		CreateInfo.jointSet = (XrBodyJointSetBD)Mode;
		if (XR_FAILED(xrCreateBodyTrackerBD(Session, &CreateInfo, &BodyTracker)))
		{
			return false;
		}
		UE_LOG(PICOOpenXRMovement, Log, TEXT("Created BodyTracker."));
		bBodyTrackerIsRunning = true;
		return true;
	}
	return false;
}

bool FBodyTrackingPICO::StopBodyTracking()
{
	if (BodyTracker != XR_NULL_HANDLE)
	{
		if (xrDestroyBodyTrackerBD == nullptr)
		{
			return false;
		}

		UE_LOG(PICOOpenXRMovement, Log, TEXT("Destroyed BodyTracker."));
		XrResult Result = xrDestroyBodyTrackerBD(BodyTracker);
		if (XR_SUCCEEDED(Result))
		{
			BodyTracker = XR_NULL_HANDLE;
			bBodyTrackerIsRunning = false;
			return true;
		}
		return false;
	}
	return false;
}

bool FBodyTrackingPICO::StartBodyTrackingCalibApp()
{
	if (Session != XR_NULL_HANDLE && bSupportBodyTracking2)
	{
		XrResult Result = xrStartBodyTrackingCalibrationAppPICO(Session);
		if (XR_SUCCEEDED(Result))
		{
			return true;
		}
	}
	return false;
}

bool FBodyTrackingPICO::GetBodyTrackingState(EBodyTrackingStatusPICO& Status, EBodyTrackingErrorCodePICO& Error)
{
	if (Session != XR_NULL_HANDLE && bSupportBodyTracking2)
	{
		XrBodyTrackingStatePICO StatusCode = { (XrStructureType)XR_TYPE_BODY_TRACKING_STATE_PICO };
		XrResult Result = xrGetBodyTrackingStatePICO(Session, &StatusCode);
		if (XR_SUCCEEDED(Result))
		{
			Status = (EBodyTrackingStatusPICO)(StatusCode.status);
			Error = (EBodyTrackingErrorCodePICO)(StatusCode.message);
			return true;
		}
	}
	return false;
}
