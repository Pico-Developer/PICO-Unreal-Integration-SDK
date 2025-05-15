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
