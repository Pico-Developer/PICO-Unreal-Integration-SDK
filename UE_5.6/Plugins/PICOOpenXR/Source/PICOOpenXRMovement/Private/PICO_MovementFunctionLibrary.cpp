// Fill out your copyright notice in the Description page of Project Settings.

#include "PICO_MovementFunctionLibrary.h"
#include "PICO_MovementModule.h"

bool UMovementFunctionLibraryPICO::TryGetBodyStatePICO(FBodyStatePICO& outBodyState, float WorldToMeters, bool QueryAcc, bool QueryVel, bool QueryPostureFlag)
{
	return FPICOOpenXRMovementModule::Get().GetBodyTrackingPICOExtension().TryGetBodyState(outBodyState, WorldToMeters, QueryAcc, QueryVel, QueryPostureFlag);
}

bool UMovementFunctionLibraryPICO::IsBodyTrackingEnabledPICO()
{
	return FPICOOpenXRMovementModule::Get().GetBodyTrackingPICOExtension().IsBodyTrackingEnabled();
}

bool UMovementFunctionLibraryPICO::IsBodyTrackingSupportedPICO()
{
	return FPICOOpenXRMovementModule::Get().GetBodyTrackingPICOExtension().IsBodyTrackingSupported();
}

bool UMovementFunctionLibraryPICO::StartBodyTrackingPICO(EBodyTrackingModePICO Mode)
{
	return FPICOOpenXRMovementModule::Get().GetBodyTrackingPICOExtension().StartBodyTracking(Mode);
}

bool UMovementFunctionLibraryPICO::StopBodyTrackingPICO()
{
	return FPICOOpenXRMovementModule::Get().GetBodyTrackingPICOExtension().StopBodyTracking();
}

bool UMovementFunctionLibraryPICO::StartBodyTrackingCalibAppPICO()
{
	return FPICOOpenXRMovementModule::Get().GetBodyTrackingPICOExtension().StartBodyTrackingCalibApp();
}

bool UMovementFunctionLibraryPICO::GetBodyTrackingStatePICO(EBodyTrackingStatusPICO& Status, EBodyTrackingErrorCodePICO& Error)
{
	return FPICOOpenXRMovementModule::Get().GetBodyTrackingPICOExtension().GetBodyTrackingState(Status, Error);
}

FOnRequestMotionTrackerCompletePICO UMovementFunctionLibraryPICO::OnRequestMotionTrackerCompletePICO;
bool UMovementFunctionLibraryPICO::RequestMotionTrackerDevicePICO(int DeviceCount, const FOnRequestMotionTrackerCompletePICO& InOnRequestMotionTrackerCompletePICO)
{
	OnRequestMotionTrackerCompletePICO = InOnRequestMotionTrackerCompletePICO;
	return FPICOOpenXRMovementModule::Get().GetMotionTrackingPICOExtension().RequestMotionTrackerDevice(DeviceCount);
}

bool UMovementFunctionLibraryPICO::GetMotionTrackerBatteryStatePICO(int64 ID, float& BatteryLevel, EChargingStatePICO& State)
{
	return FPICOOpenXRMovementModule::Get().GetMotionTrackingPICOExtension().GetMotionTrackerBatteryState(ID, BatteryLevel, State);
}

bool UMovementFunctionLibraryPICO::LocateMotionTrackerPICO(int64 ID, FRotator& OutRotation, FVector& OutPosition, FVector& OutLinearVelocity, FVector& OutAngularVelocity, FVector& OutLinearAcceleration, FVector& OutAngularAcceleration, bool GetVelAndAcc)
{
	return FPICOOpenXRMovementModule::Get().GetMotionTrackingPICOExtension().LocateMotionTracker(ID, OutRotation, OutPosition, OutLinearVelocity, OutAngularVelocity, OutLinearAcceleration, OutAngularAcceleration, GetVelAndAcc);
}

FOnMotionTrackerConnectionStateChangedPICO UMovementFunctionLibraryPICO::OnMotionTrackerConnectionStateChangedPICO;
FOnDataMotionTrackerPowerKeyStateChangedPICO UMovementFunctionLibraryPICO::OnDataMotionTrackerPowerKeyStateChangedPICO;
void UMovementFunctionLibraryPICO::BindOnMotionTrackerConnectionStateChangedPICO(const FOnMotionTrackerConnectionStateChangedPICO& Delegate)
{
	OnMotionTrackerConnectionStateChangedPICO = Delegate;
}

void UMovementFunctionLibraryPICO::BindOnDataMotionTrackerPowerKeyStateChangedPICO(const FOnDataMotionTrackerPowerKeyStateChangedPICO& Delegate)
{
	OnDataMotionTrackerPowerKeyStateChangedPICO = Delegate;
}

bool UMovementFunctionLibraryPICO::EnumerateExpandDevicePICO(TArray<int64>& IDs)
{
	return FPICOOpenXRMovementModule::Get().GetFExpandDevicePICOExtension().EnumerateExpandDevice(IDs);
}

bool UMovementFunctionLibraryPICO::SetExpandDeviceMotorVibratePICO(int64 ID, int64 Duration, int32 Frequency, float Amp)
{
	return FPICOOpenXRMovementModule::Get().GetFExpandDevicePICOExtension().SetExpandDeviceMotorVibrate(ID, Duration, Frequency, Amp);
}

bool UMovementFunctionLibraryPICO::GetExpandDeviceBatteryStatePICO(int64 ID, float& BatteryLevel, EChargingStatePICO& ChargingState)
{
	return FPICOOpenXRMovementModule::Get().GetFExpandDevicePICOExtension().GetExpandDeviceBatteryState(ID, BatteryLevel, ChargingState);
}

bool UMovementFunctionLibraryPICO::SetExpandDeviceCustomDataCapabilityPICO(bool Enable)
{
	return FPICOOpenXRMovementModule::Get().GetFExpandDevicePICOExtension().SetExpandDeviceCustomDataCapability(Enable);
}

bool UMovementFunctionLibraryPICO::SetExpandDeviceCustomDataPICO(const TArray<FExpandDeviceDataPICO>& Datas)
{
	return FPICOOpenXRMovementModule::Get().GetFExpandDevicePICOExtension().SetExpandDeviceCustomData(Datas);
}

bool UMovementFunctionLibraryPICO::GetExpandDeviceCustomDataPICO(TArray<FExpandDeviceDataPICO>& Datas)
{
	return FPICOOpenXRMovementModule::Get().GetFExpandDevicePICOExtension().GetExpandDeviceCustomData(Datas);
}

bool UMovementFunctionLibraryPICO::GetFaceTrackingSupportedPICO(TArray<EFaceTrackingModePICO>& SupportedModes)
{
	return FPICOOpenXRMovementModule::Get().GetFaceTrackingPICOExtension().GetFaceTrackingSupported(SupportedModes);
}

bool UMovementFunctionLibraryPICO::StartFaceTrackingPICO(EFaceTrackingModePICO Mode)
{
	return FPICOOpenXRMovementModule::Get().GetFaceTrackingPICOExtension().StartFaceTracking(Mode);
}

bool UMovementFunctionLibraryPICO::StopFaceTrackingPICO()
{
	return FPICOOpenXRMovementModule::Get().GetFaceTrackingPICOExtension().StopFaceTracking();
}

bool UMovementFunctionLibraryPICO::SetFaceTrackingCurrentModePICO(EFaceTrackingModePICO Mode)
{
	return FPICOOpenXRMovementModule::Get().GetFaceTrackingPICOExtension().SetFaceTrackingCurrentMode(Mode);
}

bool UMovementFunctionLibraryPICO::GetFaceTrackingCurrentModePICO(EFaceTrackingModePICO& Mode)
{
	return FPICOOpenXRMovementModule::Get().GetFaceTrackingPICOExtension().GetFaceTrackingCurrentMode(Mode);
}

bool UMovementFunctionLibraryPICO::GetFaceTrackingDataPICO(int64 DisplayTime, FFaceStatePICO& outState)
{
	return FPICOOpenXRMovementModule::Get().GetFaceTrackingPICOExtension().GetFaceTrackingData(DisplayTime, outState);
}

bool UMovementFunctionLibraryPICO::IsEyeTrackerSupportedPICO(bool& Supported)
{
	return FPICOOpenXRMovementModule::Get().GetEyeTrackingPICOExtension().IsEyeTrackerSupported(Supported);
}

bool UMovementFunctionLibraryPICO::IsEyeTrackingRunningPICO()
{
	return FPICOOpenXRMovementModule::Get().GetEyeTrackingPICOExtension().IsEyeTrackingRunning();
}

bool UMovementFunctionLibraryPICO::StartEyeTrackingPICO()
{
	return FPICOOpenXRMovementModule::Get().GetEyeTrackingPICOExtension().StartEyeTracking();
}

bool UMovementFunctionLibraryPICO::StopEyeTrackingPICO()
{
	return FPICOOpenXRMovementModule::Get().GetEyeTrackingPICOExtension().StopEyeTracking();
}

bool UMovementFunctionLibraryPICO::GetEyeTrackingDataPICO(FEyeDataPICO& LeftEye, FEyeDataPICO& RightEye, bool& bDepthValid, float& DepthConfidence, float& Depth, bool QueryGazeData, FEyeTrackerGazeData& OutGazeData)
{
	return FPICOOpenXRMovementModule::Get().GetEyeTrackingPICOExtension().GetEyeTrackingData(LeftEye, RightEye, bDepthValid, DepthConfidence, Depth, QueryGazeData, OutGazeData);
}


