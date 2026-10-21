// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_MovementExtension.h"
#include "IOpenXRHMDModule.h"
#include "PICOOpenXRRuntimeSettings.h"
#include <type_traits>
#include <utility>

namespace
{
	template <typename T>
	struct THasGetOptionalExtensions
	{
		template <typename U>
		static auto Test(int) -> decltype(std::declval<U&>().GetOptionalExtensions(std::declval<TArray<const ANSICHAR*>&>()), std::true_type());

		template <typename>
		static std::false_type Test(...);

		static constexpr bool Value = decltype(Test<T>(0))::value;
	};

	template <typename T>
	typename std::enable_if<THasGetOptionalExtensions<T>::Value, void>::type AppendOptionalExtensions(T& ExtensionPlugin, TArray<const ANSICHAR*>& OutExtensions)
	{
		ExtensionPlugin.GetOptionalExtensions(OutExtensions);
	}

	template <typename T>
	typename std::enable_if<!THasGetOptionalExtensions<T>::Value, void>::type AppendOptionalExtensionsNoOp(T&, TArray<const ANSICHAR*>&)
	{
		// Submodules without GetOptionalExtensions() intentionally contribute no optional extensions.
	}

	template <typename T>
	typename std::enable_if<!THasGetOptionalExtensions<T>::Value, void>::type AppendOptionalExtensions(T& ExtensionPlugin, TArray<const ANSICHAR*>& OutExtensions)
	{
		AppendOptionalExtensionsNoOp(ExtensionPlugin, OutExtensions);
	}

	bool IsSettingEnabled(const TCHAR* ConfigKey)
	{
		return UPICOOpenXRRuntimeSettings::GetBoolConfigByKey(ConfigKey);
	}

	bool AreExtensionsAvailable(const TArray<const ANSICHAR*>& Extensions)
	{
		if (!IOpenXRHMDModule::IsAvailable())
		{
			return true;
		}

		const IOpenXRHMDModule& OpenXRHMDModule = IOpenXRHMDModule::Get();
		for (const ANSICHAR* Extension : Extensions)
		{
			if (!OpenXRHMDModule.IsExtensionAvailable(UTF8_TO_TCHAR(Extension)))
			{
				return false;
			}
		}

		return true;
	}

	bool HasAnyEnabledSubmodule(bool bEnableFaceTracking, bool bEnableBodyTracking, bool bEnableMotionTracking, bool bEnableEyeTracking, bool bEnableExpandDevices)
	{
		return bEnableFaceTracking || bEnableBodyTracking || bEnableMotionTracking || bEnableEyeTracking || bEnableExpandDevices;
	}

	bool HasAnyActiveSubmodule(bool bFaceTrackingActive, bool bBodyTrackingActive, bool bMotionTrackingActive, bool bEyeTrackingActive, bool bExpandDeviceActive)
	{
		return bFaceTrackingActive || bBodyTrackingActive || bMotionTrackingActive || bEyeTrackingActive || bExpandDeviceActive;
	}
}

FMovementExtensionPICO::FMovementExtensionPICO()
{
}

template <typename T>
void FMovementExtensionPICO::TryActivateSubmodule(bool bIsEnabled, T& ExtensionPlugin, bool& bIsActive, TArray<const ANSICHAR*>& OutExtensions)
{
	if (!bIsEnabled)
	{
		return;
	}

	TArray<const ANSICHAR*> RequiredExtensions;
	ExtensionPlugin.GetRequiredExtensions(RequiredExtensions);

	if (!AreExtensionsAvailable(RequiredExtensions))
	{
		return;
	}

	OutExtensions.Append(RequiredExtensions);
	bIsActive = true;
}

template <typename T>
void FMovementExtensionPICO::AppendActiveSubmoduleOptionalExtensions(bool bIsActive, T& ExtensionPlugin, TArray<const ANSICHAR*>& OutExtensions)
{
	if (bIsActive)
	{
		AppendOptionalExtensions(ExtensionPlugin, OutExtensions);
	}
}

void FMovementExtensionPICO::Register()
{
	RegisterOpenXRExtensionModularFeature();
}

void FMovementExtensionPICO::Unregister()
{
	UnregisterOpenXRExtensionModularFeature();
}

bool FMovementExtensionPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	bFaceTrackingActive = false;
	bBodyTrackingActive = false;
	bMotionTrackingActive = false;
	bEyeTrackingActive = false;
	bExpandDeviceActive = false;

	const bool bEnableFaceTracking = IsSettingEnabled(TEXT("bEnableFaceTracking"));
	const bool bEnableBodyTracking = IsSettingEnabled(TEXT("bEnableBodyTracking"));
	const bool bEnableMotionTracking = IsSettingEnabled(TEXT("EnableMotionTrackingEXT"));
	const bool bEnableEyeTracking = IsSettingEnabled(TEXT("bEyeTrackingEnabled"));
	const bool bEnableExpandDevices = IsSettingEnabled(TEXT("EnableExpandDevicesEXT"));

	if (!HasAnyEnabledSubmodule(bEnableFaceTracking, bEnableBodyTracking, bEnableMotionTracking, bEnableEyeTracking, bEnableExpandDevices))
	{
		return false;
	}

	TryActivateSubmodule(bEnableFaceTracking, FaceTrackingPICO, bFaceTrackingActive, OutExtensions);
	TryActivateSubmodule(bEnableBodyTracking, BodyTrackingPICO, bBodyTrackingActive, OutExtensions);
	TryActivateSubmodule(bEnableMotionTracking, MotionTrackingPICO, bMotionTrackingActive, OutExtensions);
	TryActivateSubmodule(bEnableEyeTracking, EyeTrackingPICO, bEyeTrackingActive, OutExtensions);
	TryActivateSubmodule(bEnableExpandDevices, ExpandDevicePICO, bExpandDeviceActive, OutExtensions);

	if (!HasAnyActiveSubmodule(bFaceTrackingActive, bBodyTrackingActive, bMotionTrackingActive, bEyeTrackingActive, bExpandDeviceActive))
	{
		return false;
	}

	return true;
}

bool FMovementExtensionPICO::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	AppendActiveSubmoduleOptionalExtensions(bFaceTrackingActive, FaceTrackingPICO, OutExtensions);
	AppendActiveSubmoduleOptionalExtensions(bBodyTrackingActive, BodyTrackingPICO, OutExtensions);
	AppendActiveSubmoduleOptionalExtensions(bMotionTrackingActive, MotionTrackingPICO, OutExtensions);
	AppendActiveSubmoduleOptionalExtensions(bEyeTrackingActive, EyeTrackingPICO, OutExtensions);
	AppendActiveSubmoduleOptionalExtensions(bExpandDeviceActive, ExpandDevicePICO, OutExtensions);

	return true;
}

const void* FMovementExtensionPICO::OnGetSystem(XrInstance InInstance, const void* InNext)
{
	const void* Next = InNext;
	if (bFaceTrackingActive)
	{
		Next = FaceTrackingPICO.OnGetSystem(InInstance, Next);
	}
	if (bBodyTrackingActive)
	{
		Next = BodyTrackingPICO.OnGetSystem(InInstance, Next);
	}
	if (bMotionTrackingActive)
	{
		Next = MotionTrackingPICO.OnGetSystem(InInstance, Next);
	}
	if (bExpandDeviceActive)
	{
		Next = ExpandDevicePICO.OnGetSystem(InInstance, Next);
	}
	return Next;
}

void FMovementExtensionPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	if (bFaceTrackingActive)
	{
		FaceTrackingPICO.PostGetSystem(InInstance, InSystem);
	}
	if (bBodyTrackingActive)
	{
		BodyTrackingPICO.PostGetSystem(InInstance, InSystem);
	}
	if (bEyeTrackingActive)
	{
		EyeTrackingPICO.PostGetSystem(InInstance, InSystem);
	}
}

void FMovementExtensionPICO::PostCreateSession(XrSession InSession)
{
	if (bFaceTrackingActive)
	{
		FaceTrackingPICO.PostCreateSession(InSession);
	}
	if (bBodyTrackingActive)
	{
		BodyTrackingPICO.PostCreateSession(InSession);
	}
	if (bMotionTrackingActive)
	{
		MotionTrackingPICO.PostCreateSession(InSession);
	}
	if (bEyeTrackingActive)
	{
		EyeTrackingPICO.PostCreateSession(InSession);
	}
	if (bExpandDeviceActive)
	{
		ExpandDevicePICO.PostCreateSession(InSession);
	}
}

void FMovementExtensionPICO::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
{
	if (bMotionTrackingActive)
	{
		MotionTrackingPICO.OnEvent(InSession, InHeader);
	}
}

const void* FMovementExtensionPICO::OnBeginSession(XrSession InSession, const void* InNext)
{
	const void* Next = InNext;
	if (bEyeTrackingActive)
	{
		Next = EyeTrackingPICO.OnBeginSession(InSession, Next);
	}
	return Next;
}

void FMovementExtensionPICO::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
	if (bFaceTrackingActive)
	{
		FaceTrackingPICO.UpdateDeviceLocations(InSession, DisplayTime, TrackingSpace);
	}
	if (bBodyTrackingActive)
	{
		BodyTrackingPICO.UpdateDeviceLocations(InSession, DisplayTime, TrackingSpace);
	}
	if (bMotionTrackingActive)
	{
		MotionTrackingPICO.UpdateDeviceLocations(InSession, DisplayTime, TrackingSpace);
	}
	if (bEyeTrackingActive)
	{
		EyeTrackingPICO.UpdateDeviceLocations(InSession, DisplayTime, TrackingSpace);
	}
}
