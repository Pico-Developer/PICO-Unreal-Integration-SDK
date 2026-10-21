// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "HeadMountedDisplayTypes.h"
#include "IOpenXRExtensionPlugin.h"

/**
  * OpenXR HandTracking
  */
class FHandTrackingPICO :
	public IOpenXRExtensionPlugin
{
public:
	struct FHandState : public FNoncopyable
	{
		FHandState();

		XrHandTrackerEXT HandTracker{};
		XrHandJointLocationEXT JointLocations[XR_HAND_JOINT_COUNT_EXT];
		XrHandJointVelocityEXT JointVelocities[XR_HAND_JOINT_COUNT_EXT];
		XrHandJointVelocitiesEXT Velocities{ XR_TYPE_HAND_JOINT_VELOCITIES_EXT };
		XrHandJointLocationsEXT Locations{ XR_TYPE_HAND_JOINT_LOCATIONS_EXT };
		XrHandTrackingScaleFB Scale{ XR_TYPE_HAND_TRACKING_SCALE_FB };

		// Transforms are cached in Unreal Tracking Space
		FTransform KeypointTransforms[EHandKeypointCount];
		float Radii[EHandKeypointCount];
		FVector LinearVelocity[EHandKeypointCount];
		FVector AngularVelocity[EHandKeypointCount];
		float HandScale = 1.0f;
		bool ReceivedJointPoses = false;
	};

public:
	FHandTrackingPICO();
	virtual ~FHandTrackingPICO() override;

	/** IOpenXRExtensionPlugin */
	virtual FString GetDisplayName() override
	{
		return FString(TEXT("HandTrackingPICO"));
	}
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual bool GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual const void* OnGetSystem(XrInstance InInstance, const void* InNext) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual const void* OnBeginSession(XrSession InSession, const void* InNext) override;
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace) override;

private:
	XrSession Session = XR_NULL_HANDLE;
	XrSpace TrackingSpace = XR_NULL_HANDLE;
	XrTime DisplayTime = 0;
	bool bHandTrackingRunning = false;
	bool bHandTrackingAvailable = false;

	PFN_xrCreateHandTrackerEXT xrCreateHandTrackerEXT = nullptr;
	PFN_xrDestroyHandTrackerEXT xrDestroyHandTrackerEXT = nullptr;
	PFN_xrLocateHandJointsEXT xrLocateHandJointsEXT = nullptr;

	class IXRTrackingSystem* XRTrackingSystem = nullptr;

	FHandState HandStates[2];

	FHandState& GetLeftHandState();
	FHandState& GetRightHandState();

public:
	bool StartHandTracking();
	void StopHandTracking();
	bool UpdateHandTrackingData();
	bool IsHandTrackingRunning() { return bHandTrackingRunning; }
	bool GetHandTrackingData(EControllerHand Hand, TArray<FVector>& OutPositions, TArray<FQuat>& OutRotations, TArray<float>& OutRadii, TArray<FVector>& LinearVelocity, TArray<FVector>& AngularVelocity, float& Scale) const;
	bool GetHandTrackingMeshScale(EControllerHand Hand, float& Scale);
	const FHandState& GetLeftHandState() const;
	const FHandState& GetRightHandState() const;
	bool IsHandTrackingSupportedByDevice() const;
};

DEFINE_LOG_CATEGORY_STATIC(PICOOpenXRHandTracking, Display, All);
