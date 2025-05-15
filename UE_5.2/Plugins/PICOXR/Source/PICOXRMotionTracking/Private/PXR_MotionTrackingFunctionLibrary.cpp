// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#include "PXR_MotionTrackingFunctionLibrary.h"
#include "PXR_HMDPrivate.h"
#include "PXR_MotionTracking.h"
#include "PXR_HMD.h"
#include "PXR_InputFunctionLibrary.h"

bool UPICOXRMotionTrackingFunctionLibrary::GetFaceTrackingSupported(bool& Supported, TArray<EPXRFaceTrackingMode>& SupportedModes)
{
	return PICOXRMotionTracking::GetFaceTrackingSupported(Supported, SupportedModes);
}

bool UPICOXRMotionTrackingFunctionLibrary::GetFaceTrackingState(bool& IsTracking, FPXRFaceTrackingState& TrackingState)
{
	return PICOXRMotionTracking::GetFaceTrackingState(IsTracking, TrackingState);
}

bool UPICOXRMotionTrackingFunctionLibrary::StartFaceTracking(const FPXRFaceTrackingStartInfo& StartInfo)
{
	return PICOXRMotionTracking::StartFaceTracking(StartInfo);
}

bool UPICOXRMotionTrackingFunctionLibrary::StopFaceTracking(const FPXRFaceTrackingStopInfo& StopInfo)
{
	return PICOXRMotionTracking::StopFaceTracking(StopInfo);
}

bool UPICOXRMotionTrackingFunctionLibrary::GetFaceTrackingData(const FPXRFaceTrackingDataGetInfo& GetInfo, FPXRFaceTrackingData& OutFaceTrackingData)
{
	return PICOXRMotionTracking::GetFaceTrackingData(GetInfo, OutFaceTrackingData);
}

bool UPICOXRMotionTrackingFunctionLibrary::GetEyeTrackingSupported(bool& Supported, TArray<EPXREyeTrackingMode>& SupportedModes)
{
	return PICOXRMotionTracking::GetEyeTrackingSupported(Supported, SupportedModes);
}

bool UPICOXRMotionTrackingFunctionLibrary::GetEyeTrackingState(bool& IsTracking, FPXREyeTrackingState& TrackingState)
{
	return PICOXRMotionTracking::GetEyeTrackingState(IsTracking, TrackingState);
}

bool UPICOXRMotionTrackingFunctionLibrary::StartEyeTracking(const FPXREyeTrackingStartInfo& StartInfo)
{
	return PICOXRMotionTracking::StartEyeTracking(StartInfo);
}

bool UPICOXRMotionTrackingFunctionLibrary::StopEyeTracking(const FPXREyeTrackingStopInfo& StopInfo)
{
	return PICOXRMotionTracking::StopEyeTracking(StopInfo);
}

bool UPICOXRMotionTrackingFunctionLibrary::GetEyeTrackingData(float WorldToMetersScale, const FPXREyeTrackingDataGetInfo& GetInfo, FPXREyeTrackingData& OutEyeTrackingData)
{
	return PICOXRMotionTracking::GetEyeTrackingData(WorldToMetersScale, GetInfo, OutEyeTrackingData);
}

bool UPICOXRMotionTrackingFunctionLibrary::GetEyeOpenness(float& LeftEyeOpenness, float& RightEyeOpenness)
{
	return PICOXRMotionTracking::GetEyeOpenness(LeftEyeOpenness, RightEyeOpenness);
}

bool UPICOXRMotionTrackingFunctionLibrary::GetEyePupilInfo(FPXREyePupilInfo& EyePupilInfo)
{
	return PICOXRMotionTracking::GetEyePupilInfo(EyePupilInfo);
}

bool UPICOXRMotionTrackingFunctionLibrary::GetPerEyePose(int64& Timestamp, FTransform& LeftEyePose, FTransform& RightEyePose)
{
	return PICOXRMotionTracking::GetPerEyePose(Timestamp, LeftEyePose, RightEyePose);
}

bool UPICOXRMotionTrackingFunctionLibrary::GetEyeBlink(int64& Timestamp, bool& bLeftBlink, bool& bRightBlink)
{
	return PICOXRMotionTracking::GetEyeBlink(Timestamp, bLeftBlink, bRightBlink);
}

bool UPICOXRMotionTrackingFunctionLibrary::PXR_GetBodyTrackingSupported(bool& Supported, TArray<EPXRBodyTrackingMode>& SupportedModes)
{
	return PICOXRMotionTracking::GetBodyTrackingSupported(Supported,SupportedModes);
}

bool UPICOXRMotionTrackingFunctionLibrary::PXR_GetBodyTrackingState(bool& IsTracking, FPXRBodyTrackingState& TrackingState)
{
	return PICOXRMotionTracking::GetBodyTrackingState(IsTracking,TrackingState);
}

bool UPICOXRMotionTrackingFunctionLibrary::PXR_StartBodyTracking(const FPXRBodyTrackingStartInfo& StartInfo)
{
	return PICOXRMotionTracking::StartBodyTracking(StartInfo);
}

bool UPICOXRMotionTrackingFunctionLibrary::PXR_StopBodyTracking(const FPXRBodyTrackingStopInfo& StopInfo)
{
	return PICOXRMotionTracking::StopBodyTracking(StopInfo);
}

bool UPICOXRMotionTrackingFunctionLibrary::PXR_GetBodyTrackingData(float WorldToMetersScale, const FPXRBodyTrackingDataGetInfo& GetInfo, FPXRBodyTrackingData& BodyTrackingData)
{
	return PICOXRMotionTracking::GetBodyTrackingData(WorldToMetersScale,GetInfo,BodyTrackingData);
}

bool UPICOXRMotionTrackingFunctionLibrary::PXR_StartBodyTrackingCalibApp()
{
	EPXRMotionTrackerType CurrentType={};

	if (PXR_GetMotionTrackerType(CurrentType))
	{
		switch (CurrentType) {
		case EPXRMotionTrackerType::PxrUnknown:
			{
				return false;
			}
			break;
		case EPXRMotionTrackerType::PxrSwift_1:
			{
				return PICOXRMotionTracking::StartBodyTrackingCalibApp(PICOXRMotionTracking::GetAndroidPackageName(),0);
			}
			break;
		case EPXRMotionTrackerType::PxrSwift_2:
			{
				return PICOXRMotionTracking::StartBodyTrackingCalibApp(PICOXRMotionTracking::GetAndroidPackageName(),1);
			}
			break;
		default: ;
		}
	}
	
	return false;
}

bool UPICOXRMotionTrackingFunctionLibrary::PXR_GetMotionTrackerConnectState(FPXRMotionConnectState& connectState)
{
	return PICOXRMotionTracking::GetMotionTrackerConnectState(connectState);
}

bool UPICOXRMotionTrackingFunctionLibrary::PXR_GetMotionTrackerType(EPXRMotionTrackerType& trackerType)
{
	return PICOXRMotionTracking::GetMotionTrackerType(trackerType);
}

bool UPICOXRMotionTrackingFunctionLibrary::PXR_GetMotionTrackerMode(EPXRMotionTrackerMode& trackerMode)
{
	return PICOXRMotionTracking::GetMotionTrackerMode(trackerMode);
}

bool UPICOXRMotionTrackingFunctionLibrary::PXR_GetMotionTrackerLocations(float WorldToMetersScale, const FString& trackerSN, FPXRMotionTrackerLocations& locations)
{
	if (!trackerSN.IsEmpty())
	{
		return PICOXRMotionTracking::GetMotionTrackerLocations(WorldToMetersScale,trackerSN,locations);
	}

	return false;
}
