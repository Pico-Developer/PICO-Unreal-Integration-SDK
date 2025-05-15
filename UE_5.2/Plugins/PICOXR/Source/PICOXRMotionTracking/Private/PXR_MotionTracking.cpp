// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#include "PXR_MotionTracking.h"
#include "PXR_MotionTrackingModule.h"
#include "PXR_HMDPrivate.h"
#include "PXR_HMD.h"
#include "PXR_InputFunctionLibrary.h"
#include "PXR_PluginWrapper.h"
#include "Logging/MessageLog.h"
#include "PXR_MotionTrackingUtility.h"
#include "PXR_Utils.h"
#include "PXR_Log.h"


#define LOCTEXT_NAMESPACE "PICOXRMotionTracking"

bool PICOXRMotionTracking::GetFaceTrackingSupported(bool& Supported, TArray<EPXRFaceTrackingMode>& SupportedModes)
{
	bool bResult = false;
	bool supported = false;
	int modesCount = 0;
	bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetFaceTrackingSupported(&supported, &modesCount, nullptr));
	Supported = supported;
	if (modesCount > 0)
	{
		SupportedModes.Reset(modesCount);
		PxrFaceTrackingMode* modes = new PxrFaceTrackingMode[5];
		bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetFaceTrackingSupported(&supported, &modesCount, modes));
		for (int i = 0; i < modesCount; i++)
		{
			SupportedModes.Emplace(EPXRFaceTrackingMode(modes[i]));
		}
		delete[] modes;
	}
	return bResult;
}

bool PICOXRMotionTracking::GetFaceTrackingState(bool& IsTracking, FPXRFaceTrackingState& TrackingState)
{
	bool bResult = false;
	bool isTracking = false;
	PxrFaceTrackingState state;
	state.apiVersion = PXR_FACE_TRACKING_API_VERSION;
	bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetFaceTrackingState(&isTracking, &state));
	IsTracking = isTracking;
	TrackingState.CurrentTrackingMode = EPXRFaceTrackingMode(state.currentTrackingMode);
	TrackingState.TrackingStateCode = EPXRTrackingStateCode(state.code);
	return bResult;
}

bool PICOXRMotionTracking::StartFaceTracking(const FPXRFaceTrackingStartInfo& StartInfo)
{
	bool bResult = false;
	PxrFaceTrackingStartInfo info;
	info.apiVersion = PXR_FACE_TRACKING_API_VERSION;
	info.mode = PxrFaceTrackingMode(StartInfo.StartMode);
	bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().StartFaceTracking(&info));
	return bResult;
}

bool PICOXRMotionTracking::StopFaceTracking(const FPXRFaceTrackingStopInfo& StopInfo)
{
	bool bResult = false;
	PxrFaceTrackingStopInfo info;
	info.apiVersion = PXR_FACE_TRACKING_API_VERSION;
	info.pause = StopInfo.Pause;
	bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().StopFaceTracking(&info));
	return bResult;
}

bool PICOXRMotionTracking::GetFaceTrackingData(const FPXRFaceTrackingDataGetInfo& GetInfo, FPXRFaceTrackingData& OutFaceTrackingData)
{
	bool bResult = false;

	PxrFaceTrackingDataGetInfo getInfo;
	getInfo.apiVersion = PXR_FACE_TRACKING_API_VERSION;
	getInfo.displayTime = GetInfo.DisplayTime;
	getInfo.flags = PXR_FACE_DEFAULT;

	PxrFaceTrackingData data;
	data.apiVersion = PXR_FACE_TRACKING_API_VERSION;
	data.blendShapeWeight = OutFaceTrackingData.BlendShapeWeights.GetData();

	bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetFaceTrackingData1(&getInfo, &data));

	OutFaceTrackingData.TimeStamp = data.timestamp;
	OutFaceTrackingData.LaughingProb = data.laughingProb;
	OutFaceTrackingData.EyeValid = data.eyeValid;
	OutFaceTrackingData.FaceValid = data.faceValid;

	return bResult;
}

bool PICOXRMotionTracking::GetEyeTrackingSupported(bool& Supported, TArray<EPXREyeTrackingMode>& SupportedModes)
{
	bool bResult = false;
	bool supported = false;
	int modesCount = 0;
	bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetEyeTrackingSupported(&supported, &modesCount, nullptr));
	Supported = supported;
	if (modesCount > 0)
	{
		SupportedModes.Reset(modesCount);
		PxrEyeTrackingMode* modes = new PxrEyeTrackingMode[modesCount];
		bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetEyeTrackingSupported(&supported, &modesCount, modes));
		for (int i = 0; i < modesCount; i++)
		{
			SupportedModes.Emplace(EPXREyeTrackingMode(modes[i]));
		}
		delete[] modes;
	}
	return bResult;
}

bool PICOXRMotionTracking::GetEyeTrackingState(bool& IsTracking, FPXREyeTrackingState& TrackingState)
{
	bool bResult = false;
	bool isTracking = false;
	PxrEyeTrackingState state;
	state.apiVersion = PXR_EYE_TRACKING_API_VERSION;
	bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetEyeTrackingState(&isTracking, &state));
	IsTracking = isTracking;
	TrackingState.CurrentTrackingMode = EPXREyeTrackingMode(state.currentTrackingMode);
	TrackingState.TrackingStateCode = EPXRTrackingStateCode(state.code);
	return bResult;
}

bool PICOXRMotionTracking::StartEyeTracking(const FPXREyeTrackingStartInfo& StartInfo)
{
	bool bResult = false;
	PxrEyeTrackingStartInfo info;
	info.apiVersion = PXR_EYE_TRACKING_API_VERSION;
	info.mode = PxrEyeTrackingMode((int8)StartInfo.StartMode);
	info.needCalibration = StartInfo.NeedCalibration;
	bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().StartEyeTracking1(&info));
	return bResult;
}

bool PICOXRMotionTracking::StopEyeTracking(const FPXREyeTrackingStopInfo& StopInfo)
{
	bool bResult = false;
	PxrEyeTrackingStopInfo info;
	info.apiVersion = PXR_EYE_TRACKING_API_VERSION;
	bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().StopEyeTracking1(&info));
	return bResult;
}

bool PICOXRMotionTracking::GetEyeTrackingData(float WorldToMetersScale, const FPXREyeTrackingDataGetInfo& GetInfo, FPXREyeTrackingData& OutEyeTrackingData)
{
	bool bResult = false;

	PxrEyeTrackingDataGetInfo getInfo;
	getInfo.apiVersion = PXR_EYE_TRACKING_API_VERSION;
	getInfo.displayTime = GetInfo.DisplayTime;
	getInfo.flags = PXR_EYE_DEFAULT;
	if (GetInfo.QueryOrientation) getInfo.flags |= PXR_EYE_POSITION;
	if (GetInfo.QueryPosition)	  getInfo.flags |= PXR_EYE_ORIENTATION;

	PxrEyeTrackingData1 data;
	FMemory::Memzero(&data, sizeof(data));
	data.apiVersion = PXR_EYE_TRACKING_API_VERSION;
	bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetEyeTrackingData1(&getInfo, &data));
	if (bResult)
	{
		for (uint8 i = 0u; i < static_cast<uint8>(PxrPerEyeUsage::eyeCount); i++)
		{
			const auto& EyeData = data.eyeDatas[i];
			OutEyeTrackingData.PerEyeDatas[i].EyeType = (EPICOEye)i;
			OutEyeTrackingData.PerEyeDatas[i].Position = ToFVector(EyeData.pose.position) * WorldToMetersScale;
			OutEyeTrackingData.PerEyeDatas[i].Orientation = ToFQuat(EyeData.pose.orientation).Rotator();
			OutEyeTrackingData.PerEyeDatas[i].bIsPoseValid = EyeData.isPoseValid;
			OutEyeTrackingData.PerEyeDatas[i].Openness = EyeData.openness;
			OutEyeTrackingData.PerEyeDatas[i].bIsOpennessValid = EyeData.isOpennessValid;
		}
	}
	return bResult;
}

bool PICOXRMotionTracking::GetEyeOpenness(float& LeftEyeOpenness, float& RightEyeOpenness)
{
	bool bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetEyeOpenness(&LeftEyeOpenness, &RightEyeOpenness));
	return bResult;
}

bool PICOXRMotionTracking::GetEyePupilInfo(FPXREyePupilInfo& EyePupilInfo)
{
	PxrEyePupilInfo PxrPupilInfo;
	bool bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetEyePupilInfo(&PxrPupilInfo));
	if (bResult)
	{
		EyePupilInfo.LeftEyePupilDiameter = PxrPupilInfo.leftEyePupilDiameter;
		EyePupilInfo.LeftEyePupilPosition.X = PxrPupilInfo.leftEyePupilPosition[0];
		EyePupilInfo.LeftEyePupilPosition.Y = PxrPupilInfo.leftEyePupilPosition[1];

		EyePupilInfo.RightEyePupilDiameter = PxrPupilInfo.rightEyePupilDiameter;
		EyePupilInfo.RightEyePupilPosition.X = PxrPupilInfo.rightEyePupilPosition[0];
		EyePupilInfo.RightEyePupilPosition.Y = PxrPupilInfo.rightEyePupilPosition[1];
	}
	return bResult;
}

bool PICOXRMotionTracking::GetPerEyePose(int64& Timestamp, FTransform& LeftEyePose, FTransform& RightEyePose)
{
	int64_t PoseTimestamp = 0;
	PxrPosef Left, Right;
	bool bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetPerEyePose(&PoseTimestamp, &Left, &Right));
	if (bResult)
	{
		Timestamp = static_cast<int64>(PoseTimestamp);

		LeftEyePose.SetLocation(ToFVector(Left.position));
		LeftEyePose.SetRotation(ToFQuat(Left.orientation));

		RightEyePose.SetLocation(ToFVector(Right.position));
		RightEyePose.SetRotation(ToFQuat(Right.orientation));
	}
	return bResult;
}

bool PICOXRMotionTracking::GetEyeBlink(int64& Timestamp, bool& bLeftBlink, bool& bRightBlink)
{
	int64_t BlinkTimestamp = 0;
	bool bResult = PXRP_SUCCESS(FPICOXRHMDModule::GetPluginWrapper().GetEyeBlink(&BlinkTimestamp, &bLeftBlink, &bRightBlink));
	Timestamp = static_cast<int64>(BlinkTimestamp);
	return bResult;
}