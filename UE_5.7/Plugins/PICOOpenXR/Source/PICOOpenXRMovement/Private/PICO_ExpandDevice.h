// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_MovementFunctionLibrary.h"

class FExpandDevicePICO
{
public:
	FExpandDevicePICO();
	virtual ~FExpandDevicePICO() {}

	void GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions);
	const void* OnGetSystem(XrInstance InInstance, const void* InNext);
	void PostCreateSession(XrSession InSession);

	bool EnumerateExpandDevice(TArray<int64>& IDs);
	bool SetExpandDeviceMotorVibrate(int64 ID, int64 Duration, int32 Frequency, float Amp);
	bool GetExpandDeviceBatteryState(int64 ID, float& BatteryLevel, EChargingStatePICO& ChargingState);
	bool SetExpandDeviceCustomDataCapability(bool Enable);
	bool SetExpandDeviceCustomData(const TArray<FExpandDeviceDataPICO>& Datas);
	bool GetExpandDeviceCustomData(TArray<FExpandDeviceDataPICO>& Datas);
private:
	XrSession Session = XR_NULL_HANDLE;
	bool bSupportExpandDeviceEXT = false;
    bool bCustomDataCapability = false;

	PFN_xrEnumerateExpandDevicePICO xrEnumerateExpandDevicePICO = nullptr;
	PFN_xrSetExpandDeviceMotorVibratePICO xrSetExpandDeviceMotorVibratePICO = nullptr;
	PFN_xrGetExpandDeviceBatteryStatePICO xrGetExpandDeviceBatteryStatePICO = nullptr;
	PFN_xrSetExpandDeviceCustomDataCapabilityPICO xrSetExpandDeviceCustomDataCapabilityPICO = nullptr;
	PFN_xrSetExpandDeviceCustomDataPICO xrSetExpandDeviceCustomDataPICO = nullptr;
	PFN_xrGetExpandDeviceCustomDataPICO xrGetExpandDeviceCustomDataPICO = nullptr;
};
