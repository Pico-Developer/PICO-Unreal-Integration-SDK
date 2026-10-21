// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_FaceTracking.h"
#include "OpenXRCore.h"

#if PLATFORM_ANDROID
#include <dlfcn.h> 
#endif //PLATFORM_ANDROID

FFaceTrackingPICO::FFaceTrackingPICO()
	: Session(XR_NULL_HANDLE)
	, Time(0)
{
}

FFaceTrackingPICO::~FFaceTrackingPICO()
{
	if (FaceTracker != XR_NULL_HANDLE && xrDestroyFaceTrackerBD != nullptr)
	{
		xrDestroyFaceTrackerBD(FaceTracker);
		FaceTracker = XR_NULL_HANDLE;
	}
	FaceTrackerRefCount = 0;
}

bool FFaceTrackingPICO::GetFaceTrackingSupported(TArray<EFaceTrackingModePICO>& Modes)
{
	Modes.Reset();

	if (!bSupportsFaceTracking || !bFaceTrackingFunctionsAvailable)
	{
		return false;
	}

	if (Session == XR_NULL_HANDLE || xrEnumerateFacialSimulationModesBD == nullptr)
	{
		Modes.Add(EFaceTrackingModePICO::Default);
		Modes.Add(EFaceTrackingModePICO::WithAudio);
		Modes.Add(EFaceTrackingModePICO::WithAudioLipsync);
		Modes.Add(EFaceTrackingModePICO::OnlyAudioWithLipsync);
		return true;
	}

	uint32_t ModeCount = 0;
	XrResult Result = xrEnumerateFacialSimulationModesBD(Session, 0, &ModeCount, nullptr);
	if (XR_FAILED(Result) || ModeCount == 0)
	{
		Modes.Add(EFaceTrackingModePICO::Default);
		Modes.Add(EFaceTrackingModePICO::WithAudio);
		Modes.Add(EFaceTrackingModePICO::WithAudioLipsync);
		Modes.Add(EFaceTrackingModePICO::OnlyAudioWithLipsync);
		return true;
	}

	TArray<XrFacialSimulationModeBD> XrModes;
	XrModes.SetNum(ModeCount);
	Result = xrEnumerateFacialSimulationModesBD(Session, ModeCount, &ModeCount, XrModes.GetData());
	if (XR_FAILED(Result))
	{
		Modes.Add(EFaceTrackingModePICO::Default);
		Modes.Add(EFaceTrackingModePICO::WithAudio);
		Modes.Add(EFaceTrackingModePICO::WithAudioLipsync);
		Modes.Add(EFaceTrackingModePICO::OnlyAudioWithLipsync);
		return true;
	}

	for (uint32_t i = 0; i < ModeCount; ++i)
	{
		const uint8 ModeValue = static_cast<uint8>(XrModes[i]);
		if (ModeValue <= static_cast<uint8>(EFaceTrackingModePICO::OnlyAudioWithLipsync))
		{
			Modes.AddUnique(static_cast<EFaceTrackingModePICO>(ModeValue));
		}
	}

	if (Modes.Num() == 0)
	{
		Modes.Add(EFaceTrackingModePICO::Default);
		Modes.Add(EFaceTrackingModePICO::WithAudio);
		Modes.Add(EFaceTrackingModePICO::WithAudioLipsync);
		Modes.Add(EFaceTrackingModePICO::OnlyAudioWithLipsync);
	}

	return true;
}

void FFaceTrackingPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_BD_FACIAL_SIMULATION_EXTENSION_NAME);
}

const void* FFaceTrackingPICO::OnGetSystem(XrInstance InInstance, const void* InNext)
{
	xrEnumerateFacialSimulationModesBD = nullptr;
	xrCreateFaceTrackerBD = nullptr;
	xrDestroyFaceTrackerBD = nullptr;
	xrGetFacialSimulationDataBD = nullptr;
	xrSetFacialSimulationModeBD = nullptr;
	xrGetFacialSimulationModeBD = nullptr;

	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumerateFacialSimulationModesBD", (PFN_xrVoidFunction*)&xrEnumerateFacialSimulationModesBD));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateFaceTrackerBD", (PFN_xrVoidFunction*)&xrCreateFaceTrackerBD));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyFaceTrackerBD", (PFN_xrVoidFunction*)&xrDestroyFaceTrackerBD));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetFacialSimulationDataBD", (PFN_xrVoidFunction*)&xrGetFacialSimulationDataBD));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrSetFacialSimulationModeBD", (PFN_xrVoidFunction*)&xrSetFacialSimulationModeBD));
	XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetFacialSimulationModeBD", (PFN_xrVoidFunction*)&xrGetFacialSimulationModeBD));

	bFaceTrackingFunctionsAvailable =
		xrEnumerateFacialSimulationModesBD != nullptr &&
		xrCreateFaceTrackerBD != nullptr &&
		xrDestroyFaceTrackerBD != nullptr &&
		xrGetFacialSimulationDataBD != nullptr &&
		xrSetFacialSimulationModeBD != nullptr &&
		xrGetFacialSimulationModeBD != nullptr;

	return InNext;
}

void FFaceTrackingPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	XrSystemFacialSimulationPropertiesBD SystemFacialSimulationPropertiesBD = { XR_TYPE_SYSTEM_FACIAL_SIMULATION_PROPERTIES_BD };
	XrSystemProperties systemProperties{ XR_TYPE_SYSTEM_PROPERTIES,&SystemFacialSimulationPropertiesBD };
	XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &systemProperties));
	bSupportsFaceTracking = SystemFacialSimulationPropertiesBD.supportsFaceTracking == XR_TRUE;
}

void FFaceTrackingPICO::PostCreateSession(XrSession InSession)
{
	if (InSession != Session && FaceTracker != XR_NULL_HANDLE)
	{
		FaceTrackerRefCount = 1;
		StopFaceTracking();
	}

	Session = InSession;
	FaceTrackerRefCount = 0;
	FaceTrackingMode = EFaceTrackingModePICO::Default;
}

void FFaceTrackingPICO::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
	if (InSession != Session)
	{
		return;
	}
	Time = DisplayTime;
}

bool FFaceTrackingPICO::StartFaceTracking(EFaceTrackingModePICO Mode)
{
	if (!bSupportsFaceTracking || !bFaceTrackingFunctionsAvailable || Session == XR_NULL_HANDLE)
	{
		return false;
	}

	if (FaceTracker != XR_NULL_HANDLE)
	{
		if (Mode != FaceTrackingMode)
		{
			if (!SetFaceTrackingCurrentMode(Mode))
			{
				return false;
			}
			FaceTrackingMode = Mode;
		}
		++FaceTrackerRefCount;
		return true;
	}

	XrFaceTrackerCreateInfoBD CreateInfo = { XR_TYPE_FACE_TRACKER_CREATE_INFO_BD };
	CreateInfo.mode = (XrFacialSimulationModeBD)Mode;
	if (xrCreateFaceTrackerBD != nullptr && XR_SUCCEEDED(xrCreateFaceTrackerBD(Session, &CreateInfo, &FaceTracker)))
	{
		FaceTrackingMode = Mode;
		FaceTrackerRefCount = 1;
		return true;
	}
	return false;
}

bool FFaceTrackingPICO::StopFaceTracking()
{
	if (FaceTrackerRefCount <= 0)
	{
		FaceTrackerRefCount = 0;
		return true;
	}

	--FaceTrackerRefCount;
	if (FaceTrackerRefCount > 0)
	{
		return true;
	}

	if (FaceTracker == XR_NULL_HANDLE)
	{
		return true;
	}

	if (FaceTracker != XR_NULL_HANDLE)
	{
		if (xrDestroyFaceTrackerBD != nullptr && XR_SUCCEEDED(xrDestroyFaceTrackerBD(FaceTracker)))
		{
			FaceTracker = XR_NULL_HANDLE;
			return true;
		}
	}

	FaceTrackerRefCount = 1;
	return false;
}

bool FFaceTrackingPICO::SetFaceTrackingCurrentMode(EFaceTrackingModePICO Mode)
{
	if (FaceTracker != XR_NULL_HANDLE)
	{
		if (xrSetFacialSimulationModeBD != nullptr && XR_SUCCEEDED(xrSetFacialSimulationModeBD(FaceTracker, (XrFacialSimulationModeBD)Mode)))
		{
			return true;
		}
	}
	return false;
}

bool FFaceTrackingPICO::GetFaceTrackingCurrentMode(EFaceTrackingModePICO& Mode)
{
	if (FaceTracker != XR_NULL_HANDLE)
	{
		XrFacialSimulationModeBD CurrentMode;
		if (xrGetFacialSimulationModeBD != nullptr && XR_SUCCEEDED(xrGetFacialSimulationModeBD(FaceTracker, &CurrentMode)))
		{
			Mode = (EFaceTrackingModePICO)CurrentMode;
			return true;
		}
	}
	return false;
}

bool FFaceTrackingPICO::GetFaceTrackingData(int64 DisplayTime, FFaceStatePICO& outState)
{
	if (FaceTracker != XR_NULL_HANDLE)
	{
		check(outState.FaceExpressionWeights.Num() == XR_FACE_EXPRESSION_COUNT_BD);
		check(outState.LipsyncExpressionWeights.Num() == XR_LIP_EXPRESSION_COUNT_BD);

		XrFacialSimulationDataBD FrameState = { XR_TYPE_FACIAL_SIMULATION_DATA_BD };
		FrameState.faceExpressionWeightCount = XR_FACE_EXPRESSION_COUNT_BD;
		FrameState.faceExpressionWeights = outState.FaceExpressionWeights.GetData();

		XrLipExpressionDataBD LipExpressionData = { XR_TYPE_LIP_EXPRESSION_DATA_BD };
		LipExpressionData.lipsyncExpressionWeightCount = XR_LIP_EXPRESSION_COUNT_BD;
		LipExpressionData.lipsyncExpressionWeights = outState.LipsyncExpressionWeights.GetData();
		FrameState.next = &LipExpressionData;

		XrFacialSimulationDataGetInfoBD GetInfo = { XR_TYPE_FACIAL_SIMULATION_DATA_GET_INFO_BD };
		GetInfo.time = DisplayTime > 0 ? DisplayTime : Time;

		if (xrGetFacialSimulationDataBD != nullptr && XR_SUCCEEDED(xrGetFacialSimulationDataBD(FaceTracker, &GetInfo, &FrameState)))
		{
			outState.SampleTime = FrameState.time;
			outState.IsUpperFaceDataValid = FrameState.isUpperFaceDataValid == XR_TRUE;
			outState.IsLowerFaceDataValid = FrameState.isLowerFaceDataValid == XR_TRUE;
			return true;
		}
	}
	return false;
}
