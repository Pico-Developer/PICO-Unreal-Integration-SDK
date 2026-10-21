// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_MotionTracking.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"
#include "OpenXRCore.h"
#include "IOpenXRHMDModule.h"

FMotionTrackingPICO::FMotionTrackingPICO()
{
}

void FMotionTrackingPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_PICO_MOTION_TRACKING_EXTENSION_NAME);
}

const void* FMotionTrackingPICO::OnGetSystem(XrInstance InInstance, const void* InNext)
{
	xrRequestMotionTrackerDevicePICO = nullptr;
	xrGetMotionTrackerBatteryStatePICO = nullptr;
	xrLocateMotionTrackerPICO = nullptr;

	bSupportMotionTracking = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_MOTION_TRACKING_EXTENSION_NAME);
	if (bSupportMotionTracking)
	{
		XrResult Result = XR_SUCCESS;
		Result = xrGetInstanceProcAddr(InInstance, "xrRequestMotionTrackerDevicePICO", (PFN_xrVoidFunction*)&xrRequestMotionTrackerDevicePICO);
		Result = XR_SUCCEEDED(Result) ? xrGetInstanceProcAddr(InInstance, "xrGetMotionTrackerBatteryStatePICO", (PFN_xrVoidFunction*)&xrGetMotionTrackerBatteryStatePICO) : Result;
		Result = XR_SUCCEEDED(Result) ? xrGetInstanceProcAddr(InInstance, "xrLocateMotionTrackerPICO", (PFN_xrVoidFunction*)&xrLocateMotionTrackerPICO) : Result;

		bSupportMotionTracking = XR_SUCCEEDED(Result) &&
			xrRequestMotionTrackerDevicePICO &&
			xrGetMotionTrackerBatteryStatePICO &&
			xrLocateMotionTrackerPICO;
	}
	return InNext;
}

void FMotionTrackingPICO::PostCreateSession(XrSession InSession)
{
	static FName SystemName(TEXT("OpenXR"));
	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
	{
		XRTrackingSystem = GEngine->XRSystem.Get();
	}
	Session = InSession;
	PredictedTime = 0;
	BaseSpace = XR_NULL_HANDLE;
}

void FMotionTrackingPICO::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
{
	if (!bSupportMotionTracking || InHeader == nullptr)
	{
		return;
	}

	switch (InHeader->type)
	{
	case XR_TYPE_EVENT_DATA_REQUEST_MOTION_TRACKER_COMPLETE_PICO:
	{
		const XrEventDataRequestMotionTrackerCompletePICO* MotionTrackerComplete = reinterpret_cast<const XrEventDataRequestMotionTrackerCompletePICO*>(InHeader);
		if (MotionTrackerComplete != nullptr)
		{
			TArray<int64> IDs;
			const bool bResult = XR_SUCCEEDED(MotionTrackerComplete->result);
			if (bResult)
			{
				IDs.Reserve((int32)MotionTrackerComplete->trackerCount);
				for (uint32 i = 0; i < MotionTrackerComplete->trackerCount; i++)
				{
					IDs.Add(MotionTrackerComplete->trackerIds[i]);
				}
			}

			UMovementFunctionLibraryPICO::OnRequestMotionTrackerCompletePICO.ExecuteIfBound(IDs, bResult);
		}
	}
	break;
	case XR_TYPE_EVENT_DATA_MOTION_TRACKER_CONNECTION_STATE_CHANGED_PICO:
	{
		const XrEventDataMotionTrackerConnectionStateChangedPICO* ConnectionStateChanged = reinterpret_cast<const XrEventDataMotionTrackerConnectionStateChangedPICO*>(InHeader);
		if (ConnectionStateChanged != nullptr)
		{
			UMovementFunctionLibraryPICO::OnMotionTrackerConnectionStateChangedPICO.ExecuteIfBound(
				(int64)ConnectionStateChanged->trackerId,
				ConnectionStateChanged->state == XrMotionTrackerConnectionStatePICO::XR_MOTION_TRACKER_CONNECTION_STATE_CONNECTED_PICO);
		}
	}
	break;
	case XR_TYPE_EVENT_DATA_MOTION_TRACKER_POWER_KEY_EVENT_PICO:
	{
		const XrEventDataMotionTrackerPowerKeyEventPICO* PowerKey = reinterpret_cast<const XrEventDataMotionTrackerPowerKeyEventPICO*>(InHeader);
		if (PowerKey != nullptr)
		{
			UMovementFunctionLibraryPICO::OnDataMotionTrackerPowerKeyStateChangedPICO.ExecuteIfBound(
				(int64)PowerKey->trackerId,
				PowerKey->isLongClick == XR_TRUE);
		}
	}
	break;
	default:
		break;
	}
}

void FMotionTrackingPICO::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
	if (InSession != Session)
	{
		return;
	}
	PredictedTime = DisplayTime;
	BaseSpace = TrackingSpace;
}

bool FMotionTrackingPICO::RequestMotionTrackerDevice(int DeviceCount)
{
	if (Session != XR_NULL_HANDLE && bSupportMotionTracking && xrRequestMotionTrackerDevicePICO != nullptr && DeviceCount > 0)
	{
		if (XR_SUCCEEDED(xrRequestMotionTrackerDevicePICO(Session, DeviceCount)))
		{
			return true;
		}
	}
	return false;
}

bool FMotionTrackingPICO::GetMotionTrackerBatteryState(int64 ID, float& BatteryLevel, EChargingStatePICO& State)
{
	if (Session != XR_NULL_HANDLE && bSupportMotionTracking && xrGetMotionTrackerBatteryStatePICO != nullptr)
	{
		XrMotionTrackerBatteryStatePICO BatteryState = { XR_TYPE_MOTION_TRACKER_BATTERY_STATE_PICO };
		if (XR_SUCCEEDED(xrGetMotionTrackerBatteryStatePICO(Session, (XrMotionTrackerIdPICO)ID, &BatteryState)))
		{
			BatteryLevel = BatteryState.batteryLevel;
			State = (EChargingStatePICO)BatteryState.chargingState;
			return true;
		}
	}
	return false;
}

bool FMotionTrackingPICO::LocateMotionTracker(int64 ID, FRotator& OutRotation, FVector& OutPosition, FVector& OutLinearVelocity, FVector& OutAngularVelocity, FVector& OutLinearAcceleration, FVector& OutAngularAcceleration, bool GetVelAndAcc)
{
	if (Session != XR_NULL_HANDLE && bSupportMotionTracking && xrLocateMotionTrackerPICO != nullptr && BaseSpace != XR_NULL_HANDLE)
	{
		XrMotionTrackerLocationInfoPICO LocationInfo = { XR_TYPE_MOTION_TRACKER_LOCATION_INFO_PICO };
		LocationInfo.baseSpace = BaseSpace;
		LocationInfo.time = PredictedTime;
		XrMotionTrackerSpaceLocationPICO Location = { XR_TYPE_MOTION_TRACKER_SPACE_LOCATION_PICO };
		XrMotionTrackerSpaceVelocityPICO Velocity = { XR_TYPE_MOTION_TRACKER_SPACE_VELOCITY_PICO };
		if (GetVelAndAcc)
		{
			Location.next = &Velocity;
		}
		if (XR_SUCCEEDED(xrLocateMotionTrackerPICO(Session, (XrMotionTrackerIdPICO)ID, &LocationInfo, &Location)))
		{
			const XrSpaceLocationFlags ValidFlags = XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_POSITION_VALID_BIT;

			if ((Location.locationFlags & ValidFlags) != ValidFlags)
			{
				return false;
			}

			OutRotation = ToFQuat(Location.pose.orientation).Rotator();
			float WorldToMeter = XRTrackingSystem ? XRTrackingSystem->GetWorldToMetersScale() : 100.0f;
			OutPosition = ToFVector(Location.pose.position, WorldToMeter);

			if (GetVelAndAcc)
			{
				OutLinearVelocity = FVector::ZeroVector;
				OutAngularVelocity = FVector::ZeroVector;
				OutLinearAcceleration = FVector::ZeroVector;
				OutAngularAcceleration = FVector::ZeroVector;

				if ((Velocity.velocityFlags & XR_SPACE_VELOCITY_LINEAR_VALID_BIT) != 0)
				{
					OutLinearVelocity = ToFVector(Velocity.linearVelocity, WorldToMeter);
					OutLinearAcceleration = ToFVector(Velocity.linearAcceleration, WorldToMeter);
				}
				if ((Velocity.velocityFlags & XR_SPACE_VELOCITY_ANGULAR_VALID_BIT) != 0)
				{
					OutAngularVelocity = ToFVector(Velocity.angularVelocity, WorldToMeter);
					OutAngularAcceleration = ToFVector(Velocity.angularAcceleration, WorldToMeter);
				}
			}

			return true;
		}
	}
	return false;
}
