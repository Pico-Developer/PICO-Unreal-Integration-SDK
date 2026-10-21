// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_HandTracking.h"
#include "PICO_IHandTrackingModule.h"

#include "IXRTrackingSystem.h"
#include "Engine/Engine.h"
#include "OpenXRCore.h"
#include "IOpenXRHMDModule.h"
#include "PICOOpenXRRuntimeSettings.h"

#define LOCTEXT_NAMESPACE "PICOOpenXRHandTracking"

// These enum's must match.
static_assert(EHandKeypoint::Palm == static_cast<EHandKeypoint>(XR_HAND_JOINT_PALM_EXT), "EHandKeypoint enum does not match XrHandJointEXT.");
static_assert(EHandKeypoint::Wrist == static_cast<EHandKeypoint>(XR_HAND_JOINT_WRIST_EXT), "EHandKeypoint enum does not match XrHandJointEXT.");
static_assert(EHandKeypoint::ThumbMetacarpal == static_cast<EHandKeypoint>(XR_HAND_JOINT_THUMB_METACARPAL_EXT), "EHandKeypoint enum does not match XrHandJointEXT.");
static_assert(EHandKeypoint::IndexTip == static_cast<EHandKeypoint>(XR_HAND_JOINT_INDEX_TIP_EXT), "EHandKeypoint enum does not match XrHandJointEXT.");
static_assert(EHandKeypoint::LittleTip == static_cast<EHandKeypoint>(XR_HAND_JOINT_LITTLE_TIP_EXT), "EHandKeypoint enum does not match XrHandJointEXT.");

class FPICOOpenXRHandTrackingModule :
	public IPICOOpenXRHandTrackingModule
{
public:
	FPICOOpenXRHandTrackingModule() {}

	virtual void StartupModule() override
	{
		IPICOOpenXRHandTrackingModule::StartupModule();
	}

	virtual void ShutdownModule() override
	{
		IPICOOpenXRHandTrackingModule::ShutdownModule();
	}

	virtual FHandTrackingPICO* GetHandTrackingPICO() override
	{
		return &HandTrackingPICO;
	}

private:
	FHandTrackingPICO HandTrackingPICO;
};

IMPLEMENT_MODULE(FPICOOpenXRHandTrackingModule, PICOOpenXRHandTracking);


FHandTrackingPICO::FHandTrackingPICO()
{
	// Register modular feature manually
	IModularFeatures::Get().RegisterModularFeature(IOpenXRExtensionPlugin::GetModularFeatureName(), static_cast<IOpenXRExtensionPlugin*>(this));

	// We're implicitly requiring that the OpenXRPlugin has been loaded and
	// initialized at this point.
	if (!IOpenXRHMDModule::Get().IsAvailable())
	{
		UE_LOG(PICOOpenXRHandTracking, Error, TEXT("Error - OpenXRHMDPlugin isn't available"));
	}
}

FHandTrackingPICO::~FHandTrackingPICO()
{
	// Unregister modular feature manually
	IModularFeatures::Get().UnregisterModularFeature(IOpenXRExtensionPlugin::GetModularFeatureName(), static_cast<IOpenXRExtensionPlugin*>(this));
}

bool FHandTrackingPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (UPICOOpenXRRuntimeSettings::GetBoolConfigByKey(TEXT("bIsHandTrackingUsed")) && UPICOOpenXRRuntimeSettings::GetBoolConfigByKey(TEXT("bIsPICOHandTrackingModuleUsed")))
	{
		OutExtensions.Add(XR_EXT_HAND_TRACKING_EXTENSION_NAME);
		return true;
	}
	return false;
}

bool FHandTrackingPICO::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_FB_HAND_TRACKING_MESH_EXTENSION_NAME);
	return true;
}

const void* FHandTrackingPICO::OnGetSystem(XrInstance InInstance, const void* InNext)
{
	// Store extension open xr calls to member function pointers for convenient use.
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateHandTrackerEXT", (PFN_xrVoidFunction*)&xrCreateHandTrackerEXT));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyHandTrackerEXT", (PFN_xrVoidFunction*)&xrDestroyHandTrackerEXT));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrLocateHandJointsEXT", (PFN_xrVoidFunction*)&xrLocateHandJointsEXT));

	return InNext;
}

const void* FHandTrackingPICO::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	XrSystemHandTrackingPropertiesEXT HandTrackingSystemProperties{
	XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT };
	XrSystemProperties systemProperties{ XR_TYPE_SYSTEM_PROPERTIES,
										&HandTrackingSystemProperties };
	XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &systemProperties));

	bHandTrackingAvailable = HandTrackingSystemProperties.supportsHandTracking == XR_TRUE;

	return InNext;
}

const void* FHandTrackingPICO::OnBeginSession(XrSession InSession, const void* InNext)
{
	static FName SystemName(TEXT("OpenXR"));
	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
	{
		XRTrackingSystem = GEngine->XRSystem.Get();
	}
	Session = InSession;
	return InNext;
}

void FHandTrackingPICO::UpdateDeviceLocations(XrSession InSession, XrTime InDisplayTime, XrSpace InTrackingSpace)
{
	TrackingSpace = InTrackingSpace;
	DisplayTime = InDisplayTime;
}

FHandTrackingPICO::FHandState::FHandState()
{
	Velocities.jointCount = XR_HAND_JOINT_COUNT_EXT;
	Velocities.jointVelocities = JointVelocities;

	Scale.next = &Velocities;
	Scale.currentOutput = 1.0f;

	Locations.next = &Scale;
	Locations.jointCount = XR_HAND_JOINT_COUNT_EXT;
	Locations.jointLocations = JointLocations;
}

FHandTrackingPICO::FHandState& FHandTrackingPICO::GetLeftHandState()
{
	return HandStates[0];
}

FHandTrackingPICO::FHandState& FHandTrackingPICO::GetRightHandState()
{
	return HandStates[1];
}

bool FHandTrackingPICO::StartHandTracking()
{
	if (bHandTrackingAvailable && !bHandTrackingRunning && Session != XR_NULL_HANDLE)
	{
		// Create a hand tracker for left hand that tracks default set of hand joints.
		FHandState& LeftHandState = GetLeftHandState();
		{
			XrHandTrackerCreateInfoEXT CreateInfo{ XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT };
			CreateInfo.hand = XR_HAND_LEFT_EXT;
			CreateInfo.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;
			XR_ENSURE(xrCreateHandTrackerEXT(Session, &CreateInfo, &LeftHandState.HandTracker));
		}

		// Create a hand tracker for left hand that tracks default set of hand joints.
		FHandState& RightHandState = GetRightHandState();
		{
			XrHandTrackerCreateInfoEXT CreateInfo{ XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT };
			CreateInfo.hand = XR_HAND_RIGHT_EXT;
			CreateInfo.handJointSet = XR_HAND_JOINT_SET_DEFAULT_EXT;
			XR_ENSURE(xrCreateHandTrackerEXT(Session, &CreateInfo, &RightHandState.HandTracker));
		}
		bHandTrackingRunning = true;
		return true;
	}
	return false;
}

void FHandTrackingPICO::StopHandTracking()
{
	if (!bHandTrackingRunning)
	{
		return;
	}

	// Create a hand tracker for left hand that tracks default set of hand joints.
	FHandState& LeftHandState = GetLeftHandState();
	if (LeftHandState.HandTracker != XR_NULL_HANDLE)
	{
		XR_ENSURE(xrDestroyHandTrackerEXT(LeftHandState.HandTracker));
	}

	// Create a hand tracker for left hand that tracks default set of hand joints.
	FHandState& RightHandState = GetRightHandState();
	if (RightHandState.HandTracker != XR_NULL_HANDLE)
	{
		XR_ENSURE(xrDestroyHandTrackerEXT(RightHandState.HandTracker));
	}
	bHandTrackingRunning = false;
}

bool FHandTrackingPICO::UpdateHandTrackingData()
{
	if (!bHandTrackingAvailable || !bHandTrackingRunning)
	{
		return false;
	}

	XrHandJointsLocateInfoEXT LocateInfo{ XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT };
	LocateInfo.baseSpace = TrackingSpace;
	LocateInfo.time = DisplayTime;

	const float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();

	for (int i = 0; i < 2; ++i)
	{
		FHandState& HandState = HandStates[i];

		XR_ENSURE(xrLocateHandJointsEXT(HandState.HandTracker, &LocateInfo, &HandState.Locations));

		HandState.ReceivedJointPoses = HandState.Locations.isActive == XR_TRUE;
		if (HandState.ReceivedJointPoses) {

			static_assert(XR_HAND_JOINT_PALM_EXT == 0 && XR_HAND_JOINT_LITTLE_TIP_EXT == XR_HAND_JOINT_COUNT_EXT - 1, "XrHandJointEXT enum is not as assumed for the following loop!");
			for (int j = 0; j < XR_HAND_JOINT_COUNT_EXT; ++j)
			{
				const XrHandJointLocationEXT& JoinLocation = HandState.JointLocations[j];
				const XrPosef& Pose = JoinLocation.pose;
				const FTransform Transform = ToFTransform(Pose, WorldToMetersScale);
				HandState.KeypointTransforms[j] = Transform;
				HandState.Radii[j] = JoinLocation.radius * WorldToMetersScale;
				HandState.LinearVelocity[j] = ToFVector(HandState.JointVelocities[j].linearVelocity, WorldToMetersScale);
				HandState.AngularVelocity[j] = ToFVector(HandState.JointVelocities[j].angularVelocity, WorldToMetersScale);
				// velocities would go here
			}
			HandState.HandScale = HandState.Scale.currentOutput;
		}
	}
	return true;
}

bool FHandTrackingPICO::GetHandTrackingData(EControllerHand Hand, TArray<FVector>& OutPositions, TArray<FQuat>& OutRotations, TArray<float>& OutRadii, TArray<FVector>& LinearVelocity, TArray<FVector>& AngularVelocity, float& Scale) const
{
	if (!bHandTrackingAvailable)
	{
		return false;
	}

	if (Hand != EControllerHand::Left && Hand != EControllerHand::Right)
	{
		return false;
	}

	const FHandTrackingPICO::FHandState& HandState = (Hand == EControllerHand::Left) ? GetLeftHandState() : GetRightHandState();

	if (!HandState.ReceivedJointPoses)
	{
		return false;
	}

	OutPositions.Empty(EHandKeypointCount);
	OutRotations.Empty(EHandKeypointCount);
	const FTransform& TrackingToWorldTransform = XRTrackingSystem->GetTrackingToWorldTransform();
	for (int i = 0; i < EHandKeypointCount; ++i)
	{
		FTransform KeypointWorldTransform = HandState.KeypointTransforms[i] * TrackingToWorldTransform;
		OutPositions.Add(KeypointWorldTransform.GetLocation());
		OutRotations.Add(KeypointWorldTransform.GetRotation());
	}

	OutRadii.Empty(EHandKeypointCount);
	for (int i = 0; i < EHandKeypointCount; ++i)
	{
		OutRadii.Add(HandState.Radii[i]);
	}

	LinearVelocity.Empty(EHandKeypointCount);
	AngularVelocity.Empty(EHandKeypointCount);
	for (int i = 0; i < EHandKeypointCount; ++i)
	{
		LinearVelocity.Add(TrackingToWorldTransform.TransformVector(HandState.LinearVelocity[i]));
		AngularVelocity.Add(TrackingToWorldTransform.TransformVector(HandState.AngularVelocity[i]));
	}

	Scale = HandState.HandScale;

	return true;
}

bool FHandTrackingPICO::GetHandTrackingMeshScale(EControllerHand Hand, float& Scale)
{
	Scale = 1.0f;

	if (!bHandTrackingAvailable)
	{
		return false;
	}

	if (Hand != EControllerHand::Left && Hand != EControllerHand::Right)
	{
		return false;
	}

	const FHandTrackingPICO::FHandState& HandState = (Hand == EControllerHand::Left) ? GetLeftHandState() : GetRightHandState();

	if (!HandState.ReceivedJointPoses)
	{
		return false;
	}

	Scale = HandState.HandScale;

	return true;
}

const FHandTrackingPICO::FHandState& FHandTrackingPICO::GetLeftHandState() const
{
	return HandStates[0];
}

const FHandTrackingPICO::FHandState& FHandTrackingPICO::GetRightHandState() const
{
	return HandStates[1];
}

bool FHandTrackingPICO::IsHandTrackingSupportedByDevice() const
{
	return bHandTrackingAvailable;
}

#undef LOCTEXT_NAMESPACE
