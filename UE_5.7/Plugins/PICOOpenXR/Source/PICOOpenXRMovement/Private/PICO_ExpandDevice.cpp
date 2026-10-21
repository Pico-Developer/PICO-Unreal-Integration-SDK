// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_ExpandDevice.h"
#include "Engine/Engine.h"
#include "IXRTrackingSystem.h"
#include "OpenXRCore.h"
#include "IOpenXRHMDModule.h"

FExpandDevicePICO::FExpandDevicePICO()
{
}

void FExpandDevicePICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_PICO_EXPAND_DEVICE_EXTENSION_NAME);
}

const void* FExpandDevicePICO::OnGetSystem(XrInstance InInstance, const void* InNext)
{
	bSupportExpandDeviceEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_EXPAND_DEVICE_EXTENSION_NAME);
	if (bSupportExpandDeviceEXT)
	{
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumerateExpandDevicePICO", (PFN_xrVoidFunction*)&xrEnumerateExpandDevicePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrSetExpandDeviceMotorVibratePICO", (PFN_xrVoidFunction*)&xrSetExpandDeviceMotorVibratePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetExpandDeviceBatteryStatePICO", (PFN_xrVoidFunction*)&xrGetExpandDeviceBatteryStatePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrSetExpandDeviceCustomDataCapabilityPICO", (PFN_xrVoidFunction*)&xrSetExpandDeviceCustomDataCapabilityPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrSetExpandDeviceCustomDataPICO", (PFN_xrVoidFunction*)&xrSetExpandDeviceCustomDataPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetExpandDeviceCustomDataPICO", (PFN_xrVoidFunction*)&xrGetExpandDeviceCustomDataPICO));
	}
	return InNext;
}

void FExpandDevicePICO::PostCreateSession(XrSession InSession)
{
	Session = InSession;
}

bool FExpandDevicePICO::EnumerateExpandDevice(TArray<int64>& IDs)
{
	if (Session && bSupportExpandDeviceEXT)
	{
		uint32 count = 0;
		if (XR_SUCCEEDED(xrEnumerateExpandDevicePICO(Session, 0, &count, nullptr)) && count > 0)
		{
			IDs.SetNum(count);
			TArray<XrExpandDeviceIdPICO> ExpandDeviceIds;
			ExpandDeviceIds.SetNum(count);
			if (XR_SUCCEEDED(xrEnumerateExpandDevicePICO(Session, count, &count, ExpandDeviceIds.GetData())))
			{
				for (int i = 0; i < ExpandDeviceIds.Num(); i++)
				{
					IDs[i] = (int64)ExpandDeviceIds[i];
				}
				return true;
			}
		}
	}
	return false;
}

bool FExpandDevicePICO::SetExpandDeviceMotorVibrate(int64 ID, int64 Duration, int32 Frequency, float Amp)
{
	if (Session && bSupportExpandDeviceEXT)
	{
		XrExpandDeviceMotorVibratePICO ExpandDeviceMotorVibrate = { XR_TYPE_EXPAND_DEVICE_MOTOR_VIBRATE_PICO };
		ExpandDeviceMotorVibrate.amplitude = Amp;
		ExpandDeviceMotorVibrate.frequency = Frequency;
		ExpandDeviceMotorVibrate.duration = Duration;
		if (XR_SUCCEEDED(xrSetExpandDeviceMotorVibratePICO(Session, XrExpandDeviceIdPICO(ID), &ExpandDeviceMotorVibrate)))
		{
			return true;
		}
	}
	return false;
}

bool FExpandDevicePICO::GetExpandDeviceBatteryState(int64 ID, float& BatteryLevel, EChargingStatePICO& ChargingState)
{
	if (Session && bSupportExpandDeviceEXT)
	{
		XrExpandDeviceBatteryStatePICO ExpandDeviceBatteryState = { XR_TYPE_EXPAND_DEVICE_BATTERY_STATE_PICO };
		if (XR_SUCCEEDED(xrGetExpandDeviceBatteryStatePICO(Session, XrExpandDeviceIdPICO(ID), &ExpandDeviceBatteryState)))
		{
			BatteryLevel = ExpandDeviceBatteryState.batteryLevel;
			ChargingState = (EChargingStatePICO)ExpandDeviceBatteryState.chargingState;
			return true;
		}
	}
	return false;
}

bool FExpandDevicePICO::SetExpandDeviceCustomDataCapability(bool Enable)
{
	if (Session && bSupportExpandDeviceEXT)
	{
		if (XR_SUCCEEDED(xrSetExpandDeviceCustomDataCapabilityPICO(Session, Enable)))
		{
            bCustomDataCapability = Enable;
			return true;
		}
	}
    bCustomDataCapability = false;
	return false;
}

bool FExpandDevicePICO::SetExpandDeviceCustomData(const TArray<FExpandDeviceDataPICO>& Datas)
{
	if (Session && bSupportExpandDeviceEXT && bCustomDataCapability)
	{
		TArray<XrExpandDeviceCustomDataPICO> ExpandDeviceCustomData;
		ExpandDeviceCustomData.Init({ XR_TYPE_EXPAND_DEVICE_CUSTOM_DATA_PICO }, Datas.Num());
		for (int i = 0; i < Datas.Num(); i++)
		{
			ExpandDeviceCustomData[i].deviceId = Datas[i].ID;
			FMemory::Memcpy(ExpandDeviceCustomData[i].data, Datas[i].Data.GetData(), XR_MAX_EXPAND_DEVICE_CUSTOM_DATA_SIZE_PICO);
		}

		if (XR_SUCCEEDED(xrSetExpandDeviceCustomDataPICO(Session, ExpandDeviceCustomData.Num(), ExpandDeviceCustomData.GetData())))
		{
			return true;
		}
	}
	return false;
}

bool FExpandDevicePICO::GetExpandDeviceCustomData(TArray<FExpandDeviceDataPICO>& Datas)
{
	if (Session && bSupportExpandDeviceEXT && bCustomDataCapability)
	{
		uint32 count = 0;
		if (XR_SUCCEEDED(xrGetExpandDeviceCustomDataPICO(Session, 0, &count, nullptr)) && count > 0)
		{
			TArray<XrExpandDeviceCustomDataPICO> ExpandDeviceCustomData;
			ExpandDeviceCustomData.Init({ XR_TYPE_EXPAND_DEVICE_CUSTOM_DATA_PICO }, count);
			if (XR_SUCCEEDED(xrGetExpandDeviceCustomDataPICO(Session, count, &count, ExpandDeviceCustomData.GetData())))
			{
				Datas.Reset(count);
				for (uint32 i = 0; i < count; i++)
				{
					FExpandDeviceDataPICO Data;
					Data.ID = ExpandDeviceCustomData[i].deviceId;
					FMemory::Memcpy(Data.Data.GetData(), ExpandDeviceCustomData[i].data, XR_MAX_EXPAND_DEVICE_CUSTOM_DATA_SIZE_PICO);
					Datas.Add(Data);
				}
				return true;
			}
		}
	}
	return false;
}
