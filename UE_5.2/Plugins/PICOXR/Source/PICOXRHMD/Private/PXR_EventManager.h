// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved.

#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Delegates/Delegate.h"
#include "PXR_EventManager.generated.h"

//ControllerDelegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRDeviceMainChangedDelegate,int32,Handness);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRHandnessChangedDelegate, int32, Handness);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPXRDeviceConnectChangedDelegate,int32,Handness,int32,State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRInputDeviceChangedDelegate,int32,State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRBatteryStateChangedDelegate,int32,State);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRLongHomeKey);
DECLARE_MULTICAST_DELEGATE(PXRRawLongHomeKey);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRResume);
//HMDDelegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRIpdChanged,float,NewIpd);
//SystemDisplayRateDelegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRRefreshRateChanged, float, NewRate);
//BodyTracking Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPXRDeviceFitnessBandBatteryDelegate,int32,Id,int32,BatteryState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPXRDeviceFitnessBandConnectionDelegate,int32,Count,int32,ConnectionState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRDeviceFitnessRecalibrationBandDelegate);

UCLASS()
class UPICOXREventManager : public UObject
{
	GENERATED_BODY()
public:
	
	static UPICOXREventManager* GetInstance();
	
	UPROPERTY(BlueprintAssignable)
	FPXRDeviceMainChangedDelegate DeviceMainChangedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRHandnessChangedDelegate HandnessChangedDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FPXRDeviceConnectChangedDelegate DeviceConnectChangedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRIpdChanged IpdChangedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRLongHomeKey LongHomePressedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRResume ResumeDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRRefreshRateChanged RefreshRateChangedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRInputDeviceChangedDelegate InputDeviceChangedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRBatteryStateChangedDelegate BatteryStateChangedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRDeviceFitnessBandBatteryDelegate FitnessBandBatteryDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRDeviceFitnessBandConnectionDelegate FitnessBandConnectionDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRDeviceFitnessRecalibrationBandDelegate FitnessRecalibrationBandDelegate;

	PXRRawLongHomeKey RawLongHomePressedDelegate;
};
