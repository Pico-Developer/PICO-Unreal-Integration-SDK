// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once

#include "PXR_MotionTrackingTypes.h"

struct PICOXRMOTIONTRACKING_API PICOXRMotionTracking
{
	static bool GetFaceTrackingSupported(bool& Supported, TArray<EPXRFaceTrackingMode>& SupportedModes);
	static bool GetFaceTrackingState(bool& IsTracking, FPXRFaceTrackingState& TrackingState);
	static bool StartFaceTracking(const FPXRFaceTrackingStartInfo& StartInfo);
	static bool StopFaceTracking(const FPXRFaceTrackingStopInfo& StopInfo);
	static bool GetFaceTrackingData(const FPXRFaceTrackingDataGetInfo& GetInfo, FPXRFaceTrackingData& OutFaceTrackingData);

	static bool GetEyeTrackingSupported(bool& Supported, TArray<EPXREyeTrackingMode>& SupportedModes);
	static bool GetEyeTrackingState(bool& IsTracking, FPXREyeTrackingState& TrackingState);
	static bool StartEyeTracking(const FPXREyeTrackingStartInfo& StartInfo);
	static bool StopEyeTracking(const FPXREyeTrackingStopInfo& StopInfo);
	static bool GetEyeTrackingData(float WorldToMetersScale, const FPXREyeTrackingDataGetInfo& GetInfo, FPXREyeTrackingData& OutEyeTrackingData);
	static bool GetEyeOpenness(float& LeftEyeOpenness, float& RightEyeOpenness);
	static bool GetEyePupilInfo(FPXREyePupilInfo& EyePupilInfo);
	static bool GetPerEyePose(int64& Timestamp, FTransform& LeftEyePose, FTransform& RightEyePose);
	static bool GetEyeBlink(int64& Timestamp, bool& bLeftBlink, bool& bRightBlink);

};
