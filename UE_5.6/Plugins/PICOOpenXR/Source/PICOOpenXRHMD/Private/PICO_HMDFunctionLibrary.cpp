// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_HMDFunctionLibrary.h"
#include "PICO_HMDModule.h"

DEFINE_LOG_CATEGORY(LogPICOOpenXRHMD);

bool UHMDFunctionLibraryPICO::GetDevicePoseForTimePICO(const EControllerHand Hand, bool UseDefaultTime, FTimespan Timespan, bool& OutTimeWasUsed, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityRadPerSec, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float InWorldToMetersScale)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetDevicePoseForTime(Hand, UseDefaultTime, Timespan, OutTimeWasUsed, OutOrientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityRadPerSec, OutbProvidedLinearAcceleration, OutLinearAcceleration, InWorldToMetersScale);
}

EHMDWornState::Type UHMDFunctionLibraryPICO::GetHMDWornStatePICO(bool& ResultValid)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetHMDWornState(ResultValid);
}

bool UHMDFunctionLibraryPICO::GetSupportedDisplayRefreshRatesPICO(TArray<float>& DisplayRefreshRates)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetSupportedDisplayRefreshRates(DisplayRefreshRates);
}

bool UHMDFunctionLibraryPICO::GetFieldOfViewPICO(float& OutHFOVInDegrees, float& OutVFOVInDegrees)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetFieldOfView(OutHFOVInDegrees, OutVFOVInDegrees);
}

bool UHMDFunctionLibraryPICO::GetInterpupillaryDistancePICO(float& IPD)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetInterpupillaryDistance(IPD);
}

bool UHMDFunctionLibraryPICO::GetCurrentDisplayRefreshRatePICO(float& DisplayRefreshRate, bool DoubleCheck)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetCurrentDisplayRefreshRate(DisplayRefreshRate, DoubleCheck);
}

bool UHMDFunctionLibraryPICO::SetDisplayRefreshRatePICO(float DisplayRefreshRate)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().SetDisplayRefreshRate(DisplayRefreshRate);
}

UDelegateManagerPICO* UHMDFunctionLibraryPICO::PICODelegateManager = nullptr;
UDelegateManagerPICO* UHMDFunctionLibraryPICO::GetDelegateManagerPICO()
{
	if (PICODelegateManager == nullptr)
	{
		PICODelegateManager = NewObject<UDelegateManagerPICO>();
		PICODelegateManager->AddToRoot();
	}
	return PICODelegateManager;
}

void UHMDFunctionLibraryPICO::EnableContentProtectPICO(bool Enable)
{
	FPICOOpenXRHMDModule::Get().GetXRPlugin().EnableContentProtect(Enable);
}

bool UHMDFunctionLibraryPICO::SetPerformanceSettingsPICO(EPerfSettingsDomainPICO Domain, EPerfSettingsLevelPICO Level)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().SetPerformanceLevel((int)Domain, (int)Level);
}

void UHMDFunctionLibraryPICO::SetBaseRotationAndBaseOffsetPICO(FRotator Rotation, FVector BaseOffset, EOrientPositionSelector::Type Options)
{
	FPICOOpenXRHMDModule::Get().GetXRPlugin().SetBaseRotationAndBaseOffset(Rotation, BaseOffset, Options);
}

void UHMDFunctionLibraryPICO::GetBaseRotationAndBaseOffsetPICO(FRotator& OutRotation, FVector& OutBaseOffset)
{
	FPICOOpenXRHMDModule::Get().GetXRPlugin().GetBaseRotationAndBaseOffset(OutRotation, OutBaseOffset);
}

FTimespan UHMDFunctionLibraryPICO::GetDisplayTimePICO()
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetDisplayTime();
}

bool UHMDFunctionLibraryPICO::IsStationaryBoundaryModePICO(bool& bIsStationary)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().IsStationaryBoundaryMode(bIsStationary);
}

bool UHMDFunctionLibraryPICO::GetVirtualBoundaryStatusPICO(bool& bIsReady, bool& bIsEnable, bool& bIsVisible)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetVirtualBoundaryStatus(bIsReady, bIsEnable, bIsVisible);
}

bool UHMDFunctionLibraryPICO::SetVirtualBoundaryEnablePICO(bool bEnable)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().SetVirtualBoundaryEnable(bEnable);
}

bool UHMDFunctionLibraryPICO::SetVirtualBoundaryVisiblePICO(bool bVisible)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().SetVirtualBoundaryVisible(bVisible);
}

bool UHMDFunctionLibraryPICO::SetVirtualBoundarySeeThroughVisiblePICO(bool bVisible)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().SetVirtualBoundarySeeThroughVisible(bVisible);
}

bool UHMDFunctionLibraryPICO::BoundaryIntersectPointOrNodePICO(bool bPoint, EControllerHand Node, FVector Point, EBoundaryTypePICO BoundaryType, bool& Valid, bool& IsTriggering, float& ClosestDistance, FVector& ClosestPoint, FVector& ClosestPointNormal, float InWorldToMetersScale)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().BoundaryintersectPointOrNode(bPoint, Node, Point, BoundaryType, Valid, IsTriggering, ClosestDistance, ClosestPoint, ClosestPointNormal);
}

bool UHMDFunctionLibraryPICO::GetBoundaryGeometryPICO(EBoundaryTypePICO BoundaryType, bool& Valid, TArray<FVector>& Points, float InWorldToMetersScale)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetBoundaryGeometry(BoundaryType, Valid, Points, InWorldToMetersScale);
}

void UHMDFunctionLibraryPICO::EnableMRCDebugModePICO(UObject* WorldContextObject, bool Enable, bool ViewInHMD, bool UseCustomTransform, const FTransform& Pose, bool UseCustomCameraInfo, int Width, int Height, float Fov)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		FPICOOpenXRHMDModule::Get().GetXRPlugin().EnableMRCDebugMode(World, Enable, ViewInHMD, UseCustomTransform, Pose, UseCustomCameraInfo, Width, Height, Fov);
	}
}

void UHMDFunctionLibraryPICO::DisableMRCForegroundLayerPICO(UObject* WorldContextObject, bool Disable)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull))
	{
		FPICOOpenXRHMDModule::Get().GetXRPlugin().DisableMRCForegroundLayer(World, Disable);
	}
}

void UHMDFunctionLibraryPICO::PauseMRCPICO(bool Pause)
{
	FPICOOpenXRHMDModule::Get().GetXRPlugin().PauseMRC(Pause);
}

bool UHMDFunctionLibraryPICO::GetViewportSizePICO(FIntPoint& Size)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetViewportSize(Size);
}

FIntPoint UHMDFunctionLibraryPICO::GetDefaultRenderTargetSizePICO()
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetDefaultRenderTargetSize();
}

FIntPoint UHMDFunctionLibraryPICO::GetCurrentRenderTargetSizePICO()
{
	uint32 X, Y;
	FPICOOpenXRHMDModule::Get().GetXRPlugin().GetCurrentRenderTargetSize(X, Y);
	return FIntPoint(X, Y);
}

bool UHMDFunctionLibraryPICO::SetProjectionLayerColorMatrix3x3fPICO(bool Enable, FVector3f ColumnA, FVector3f ColumnB, FVector3f ColumnC)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().SetProjectionLayerColorMatrix3x3f(Enable, ColumnA, ColumnB, ColumnC);
}

bool UHMDFunctionLibraryPICO::GetBatteryStateDisplayPICO(EControllerHand Hand, bool& Valid, float& BatteryLevel, bool& Charging, bool& PluggedIn, bool& NoBattery)
{
	return FPICOOpenXRHMDModule::Get().GetXRPlugin().GetBatteryStateDisplay(Hand, Valid, BatteryLevel, Charging, PluggedIn, NoBattery);
}
