// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_HMD.h"
#include "PICOOpenXRRuntimeSettings.h"
#include "OpenXRCore.h"
#include "IXRTrackingSystem.h"
#include "OpenXRCore.h"
#include "IOpenXRHMDModule.h"
#include "OpenXRHMD_Swapchain.h"
#include "OpenXRHMD_RenderBridge.h"
#include "PixelShaderUtils.h"
#include "Shader.h"
#include "ClearQuad.h"
#include "ScreenRendering.h"
#include "PICO_Shaders.h"
#include "HDRHelper.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "TextureResource.h"
#include "GameFramework/Actor.h"
#include "PICO_MRCCamera.h"
#include "Runtime/Launch/Resources/Version.h"
#include "PICO_DynamicResolutionState.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "openxr_pico/private/ext_battery_state_display.h"

#include "openxr/openxr.h"

#ifndef XR_EXT_RENDER_MODEL_EXTENSION_NAME
#define XR_EXT_RENDER_MODEL_EXTENSION_NAME "XR_EXT_render_model"
#endif

#ifndef XR_EXT_INTERACTION_RENDER_MODEL_EXTENSION_NAME
#define XR_EXT_INTERACTION_RENDER_MODEL_EXTENSION_NAME "XR_EXT_interaction_render_model"
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidPlatformMisc.h"
#include <dlfcn.h> 
#endif //PLATFORM_ANDROID

// Static instance
FHMDPICO* FHMDPICO::PICOInstance = nullptr;

static TAutoConsoleVariable<int32> CVarPICOEnableSuperResolution(
	TEXT("r.Mobile.PICO.EnableSuperResolution"),
	0,
	TEXT("0: Disable SuperResolution (Default)\n")
	TEXT("1: Enable SuperResolution on supported platforms\n"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarPICOSharpeningSetting(
	TEXT("r.Mobile.PICO.SharpeningSetting"),
	0,
	TEXT("0: Disable Sharpening (Default)\n")
	TEXT("1: Enable NormalSharpening on supported platforms\n")
	TEXT("2: Enable QualitySharpening on supported platforms\n"),
	ECVF_Default);

static TAutoConsoleVariable<int32> CVarPICOSharpeningEnhanceMode(
	TEXT("r.Mobile.PICO.SharpeningEnhanceMode"),
	0,
	TEXT("0: Disable Sharpening EnhanceMode (Default)\n")
	TEXT("1: Enable Fixed Foveated on supported platforms\n")
	TEXT("2: Enable Adaptive on supported platforms\n")
	TEXT("3: Enable FixedFoveated and Adaptive on supported platforms\n"),
	ECVF_Default);

static TAutoConsoleVariable<float> CVarPICODynamicResolutionPixelDensity(
	TEXT("r.PICO.DynamicResolution.PixelDensity"),
	0,
	TEXT("0 Static Pixel Density corresponding to Pixel Density 1.0 (default)\n")
	TEXT(">0 Manual Pixel Density Override\n"),
	ECVF_Default);
	
#if PLATFORM_ANDROID
static PFN_xrGetInstanceProcAddr GPICOChainedGetInstanceProcAddr = nullptr;
static PFN_xrEnumerateSwapchainFormats GPICOChainedEnumerateSwapchainFormats = nullptr;

static bool IsPICOBGRASRGBSwapchainFormat(int64 Format)
{
	// Vulkan VkFormat values returned by xrEnumerateSwapchainFormats on Android.
	constexpr int64 VkFormatB8G8R8A8Srgb = 50;

	return Format == VkFormatB8G8R8A8Srgb;
}

static XrResult XRAPI_CALL PICOLayerEnumerateSwapchainFormats(XrSession Session, uint32_t FormatCapacityInput, uint32_t* FormatCountOutput, int64_t* Formats)
{
	if (GPICOChainedEnumerateSwapchainFormats == nullptr || FormatCountOutput == nullptr || (FormatCapacityInput > 0 && Formats == nullptr))
	{
		return GPICOChainedEnumerateSwapchainFormats != nullptr
			? GPICOChainedEnumerateSwapchainFormats(Session, FormatCapacityInput, FormatCountOutput, Formats)
			: XR_ERROR_FUNCTION_UNSUPPORTED;
	}

	uint32_t RuntimeFormatCount = 0;
	XrResult Result = GPICOChainedEnumerateSwapchainFormats(Session, 0, &RuntimeFormatCount, nullptr);
	if (XR_FAILED(Result))
	{
		return Result;
	}

	TArray<int64_t> RuntimeFormats;
	RuntimeFormats.SetNum(RuntimeFormatCount);
	Result = GPICOChainedEnumerateSwapchainFormats(Session, RuntimeFormatCount, &RuntimeFormatCount, RuntimeFormats.GetData());
	if (XR_FAILED(Result))
	{
		return Result;
	}
	RuntimeFormats.SetNum(RuntimeFormatCount, EAllowShrinking::No);

	uint32_t FilteredFormatCount = 0;
	for (int64_t Format : RuntimeFormats)
	{
		if (!IsPICOBGRASRGBSwapchainFormat(Format))
		{
			if (Formats != nullptr && FilteredFormatCount < FormatCapacityInput)
			{
				Formats[FilteredFormatCount] = Format;
			}
			++FilteredFormatCount;
		}
	}

	*FormatCountOutput = FilteredFormatCount;
	return FormatCapacityInput == 0 || FormatCapacityInput >= FilteredFormatCount ? XR_SUCCESS : XR_ERROR_SIZE_INSUFFICIENT;
}

static XrResult XRAPI_CALL PICOLayerGetInstanceProcAddr(XrInstance Instance, const char* Name, PFN_xrVoidFunction* Function)
{
	if (GPICOChainedGetInstanceProcAddr == nullptr)
	{
		return XR_ERROR_FUNCTION_UNSUPPORTED;
	}

	const XrResult Result = GPICOChainedGetInstanceProcAddr(Instance, Name, Function);
	if (XR_SUCCEEDED(Result) && Function != nullptr && *Function != nullptr && Name != nullptr && FCStringAnsi::Strcmp(Name, "xrEnumerateSwapchainFormats") == 0)
	{
		GPICOChainedEnumerateSwapchainFormats = reinterpret_cast<PFN_xrEnumerateSwapchainFormats>(*Function);
		*Function = reinterpret_cast<PFN_xrVoidFunction>(PICOLayerEnumerateSwapchainFormats);
	}

	return Result;
}
#endif // PLATFORM_ANDROID

FHMDPICO::FHMDPICO()
	: LoaderHandle(nullptr)
	, Instance(XR_NULL_HANDLE)
	, System(XR_NULL_SYSTEM_ID)
	, Session(XR_NULL_HANDLE)
	, bSupportLocalFloorLevelEXT(false)
	, bSupportDisplayRefreshRate(false)
	, CurrentDisplayRefreshRate(0)
	, bContentProtectEnabled(false)
	, CurrentDisplayTime(0)
	, IsSupportsUserPresence(false)
	, WornState(EHMDWornState::Type::Unknown)
	, bSupportMRCExtension(false)
	, bIsMRCRunning(false)
	, MRCDebugMode(FMRCDebugModePICO())
	, bSupportedCameraImage(false)
	, NextDeviceHandle(1)
	, NextSessionHandle(1)
	, bSupportedFBCompositionLayerSettingsExt(false)
	, bSupportedPICOCompositionLayerSettingsExt(false)
	, ProjectionLayerSettings({ XR_TYPE_LAYER_SETTINGS_PICO })
{
	FHMDPICO::PICOInstance = this;
	SupportedDisplayRefreshRates.Empty();
}

FHMDPICO::~FHMDPICO()
{
	if (FHMDPICO::PICOInstance == this)
	{
		FHMDPICO::PICOInstance = nullptr;
	}
}

void FHMDPICO::Register()
{
	RegisterOpenXRExtensionModularFeature();
	OnWorldTickStartDelegateHandle = FWorldDelegates::OnWorldTickStart.AddRaw(this, &FHMDPICO::OnWorldTickStart);
}

void FHMDPICO::Unregister()
{
	UnregisterOpenXRExtensionModularFeature();
#if PLATFORM_ANDROID
	GPICOChainedGetInstanceProcAddr = nullptr;
	GPICOChainedEnumerateSwapchainFormats = nullptr;
#endif // PLATFORM_ANDROID

	if (LoaderHandle)
	{
		FPlatformProcess::FreeDllHandle(LoaderHandle);
		LoaderHandle = nullptr;
	}

	if (OnWorldTickStartDelegateHandle.IsValid())
	{
		FWorldDelegates::OnWorldTickStart.Remove(OnWorldTickStartDelegateHandle);
		OnWorldTickStartDelegateHandle.Reset();
	}
}

bool FHMDPICO::GetCustomLoader(PFN_xrGetInstanceProcAddr* OutGetProcAddr)
{
#if PLATFORM_ANDROID
	if (!FAndroidMisc::GetDeviceMake().ToLower().Contains("pico"))
	{
		return false;
	}

	// clear errors
	dlerror();

	LoaderHandle = FPlatformProcess::GetDllHandle(TEXT("libopenxr_loader_pico.so"));
	if (LoaderHandle == nullptr)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Unable to load libopenxr_loader_pico.so, error %s"), ANSI_TO_TCHAR(dlerror()));
		return false;
	}

	// clear errors
	dlerror();

	PFN_xrGetInstanceProcAddr xrGetInstanceProcAddrPtr = (PFN_xrGetInstanceProcAddr)FPlatformProcess::GetDllExport(LoaderHandle, TEXT("xrGetInstanceProcAddr"));
	if (xrGetInstanceProcAddrPtr == nullptr)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Unable to load OpenXR xrGetInstanceProcAddr, error %s"), ANSI_TO_TCHAR(dlerror()));
		return false;
	}
	*OutGetProcAddr = xrGetInstanceProcAddrPtr;

	extern struct android_app* GNativeAndroidApp;
	PFN_xrInitializeLoaderKHR xrInitializeLoaderKHR;
	xrGetInstanceProcAddrPtr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", (PFN_xrVoidFunction*)&xrInitializeLoaderKHR);
	if (xrInitializeLoaderKHR == nullptr)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Unable to load OpenXR xrInitializeLoaderKHR"));
		return false;
	}
	XrLoaderInitInfoAndroidKHR LoaderInitializeInfoAndroid;
	LoaderInitializeInfoAndroid.type = XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR;
	LoaderInitializeInfoAndroid.next = NULL;
	LoaderInitializeInfoAndroid.applicationVM = GNativeAndroidApp->activity->vm;
	LoaderInitializeInfoAndroid.applicationContext = GNativeAndroidApp->activity->clazz;
	XR_ENSURE(xrInitializeLoaderKHR((XrLoaderInitInfoBaseHeaderKHR*)&LoaderInitializeInfoAndroid));

	//Used to determine whether the pico runtime loads successfully
	{
		PFN_xrEnumerateInstanceExtensionProperties xrEnumerateInstanceExtensionPropertiesPtr;
		xrGetInstanceProcAddrPtr(XR_NULL_HANDLE, "xrEnumerateInstanceExtensionProperties", (PFN_xrVoidFunction*)&xrEnumerateInstanceExtensionPropertiesPtr);
		if (xrEnumerateInstanceExtensionPropertiesPtr == nullptr)
		{
			UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Unable to load OpenXR xrEnumerateInstanceExtensionProperties!"));
			return false;
		}

		uint32 ExtensionsCount = 0;
		if (XR_FAILED(xrEnumerateInstanceExtensionPropertiesPtr(nullptr, 0, &ExtensionsCount, nullptr)))
		{
			UE_LOG(LogPICOOpenXRHMD, Error, TEXT("xrEnumerateInstanceExtensionPropertiesPtr Failed!"));
			return false;
		}
	}

	UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Loaded PICO OpenXR Loader"));
	return true;
#endif //PLATFORM_ANDROID
	return false;
}

bool FHMDPICO::InsertOpenXRAPILayer(PFN_xrGetInstanceProcAddr& InOutGetProcAddr)
{
#if PLATFORM_ANDROID
	if (!FAndroidMisc::GetDeviceMake().ToLower().Contains("pico"))
	{
		return false;
	}

	if (InOutGetProcAddr == nullptr)
	{
		return false;
	}

	GPICOChainedGetInstanceProcAddr = InOutGetProcAddr;
	InOutGetProcAddr = PICOLayerGetInstanceProcAddr;
	UE_LOG(LogPICOOpenXRHMD, Log, TEXT("PICO OpenXR API layer enabled: filtering VK_FORMAT_B8G8R8A8_SRGB swapchain format."));
	return true;
#else
	return false;
#endif // PLATFORM_ANDROID
}

bool FHMDPICO::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_FB_DISPLAY_REFRESH_RATE_EXTENSION_NAME);
	OutExtensions.Add(XR_EXT_LOCAL_FLOOR_EXTENSION_NAME);
	OutExtensions.Add(XR_FB_COMPOSITION_LAYER_SECURE_CONTENT_EXTENSION_NAME);
	OutExtensions.Add(XR_EXT_PERFORMANCE_SETTINGS_EXTENSION_NAME);
	OutExtensions.Add(XR_EXT_USER_PRESENCE_EXTENSION_NAME);
	OutExtensions.Add(XR_EXT_INTERACTION_PROFILE_BATTERY_STATE_DISPLAY_EXTENSION_NAME);
	OutExtensions.Add(XR_EXT_RENDER_MODEL_EXTENSION_NAME);
	OutExtensions.Add(XR_EXT_INTERACTION_RENDER_MODEL_EXTENSION_NAME);
	OutExtensions.Add(XR_PICO_VIRTUAL_BOUNDARY_EXTENSION_NAME);
	OutExtensions.Add(XR_PICO_EXTERNAL_CAMERA_EXTENSION_NAME);
	OutExtensions.Add(XR_PICO_LAYER_SETTINGS_EXTENSION_NAME);
#if UE_VERSION_NEWER_THAN(5, 5, 0)
	OutExtensions.Add(XR_PICO_ADAPTIVE_RESOLUTION_EXTENSION_NAME);
#endif
	OutExtensions.Add(XR_PICO_LAYER_COLOR_MATRIX_EXTENSION_NAME);
	OutExtensions.Add(XR_EXT_FUTURE_EXTENSION_NAME);  // Required by XR_PICO_camera_image
	OutExtensions.Add(XR_PICO_CAMERA_IMAGE_EXTENSION_NAME);

	OutExtensions.Add(XR_FB_COMPOSITION_LAYER_SETTINGS_EXTENSION_NAME);
	
	return true;
}

void FHMDPICO::PostCreateInstance(XrInstance InInstance)
{
	Instance = InInstance;
	XrInstanceProperties InstanceProps = { XR_TYPE_INSTANCE_PROPERTIES, nullptr };
	XR_ENSURE(xrGetInstanceProperties(InInstance, &InstanceProps));
	InstanceProps.runtimeName[XR_MAX_RUNTIME_NAME_SIZE - 1] = 0; // Ensure the name is null terminated.
	FString RuntimeName = FString(InstanceProps.runtimeName);
	UE_LOG(LogPICOOpenXRHMD, Log, TEXT("PICO OpenXR PostCreateInstance RuntimeName:%s"), *RuntimeName);
	// Normalize runtime name by removing whitespace before matching.
	FString RuntimeNameNoWhitespace;
	RuntimeNameNoWhitespace.Reserve(RuntimeName.Len());
	for (TCHAR C : RuntimeName)
	{
		if (!FChar::IsWhitespace(C))
		{
			RuntimeNameNoWhitespace.AppendChar(C);
		}
	}

	bIsPicoRuntime = RuntimeNameNoWhitespace.Contains(TEXT("pico"), ESearchCase::IgnoreCase);
	if (bIsPicoRuntime)
	{
		UPICOOpenXRRuntimeSettings::ApplyCVarSettingsFromProjectSettings();

		if (IConsoleVariable* VRSFormatCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.Vulkan.VRSFormat")))
		{
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("PICO VRS format current cvar value: %d"), VRSFormatCVar->GetInt());
		}
	}

	bSupportBatteryStateDisplayEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_EXT_INTERACTION_PROFILE_BATTERY_STATE_DISPLAY_EXTENSION_NAME);
	if (bSupportBatteryStateDisplayEXT)
	{
		bHasLeftHandUserPath = XR_SUCCEEDED(xrStringToPath(InInstance, "/user/hand/left", &LeftHandUserPath)) && LeftHandUserPath != XR_NULL_PATH;
		bHasRightHandUserPath = XR_SUCCEEDED(xrStringToPath(InInstance, "/user/hand/right", &RightHandUserPath)) && RightHandUserPath != XR_NULL_PATH;
		bHasHeadUserPath = XR_SUCCEEDED(xrStringToPath(InInstance, "/user/head", &HeadUserPath)) && HeadUserPath != XR_NULL_PATH;
	}
}

void FHMDPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	System = InSystem;
	bSupportDisplayRefreshRate = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_FB_DISPLAY_REFRESH_RATE_EXTENSION_NAME);

	if (bSupportDisplayRefreshRate)
	{
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumerateDisplayRefreshRatesFB", (PFN_xrVoidFunction*)&xrEnumerateDisplayRefreshRatesFB));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetDisplayRefreshRateFB", (PFN_xrVoidFunction*)&xrGetDisplayRefreshRateFB));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrRequestDisplayRefreshRateFB", (PFN_xrVoidFunction*)&xrRequestDisplayRefreshRateFB));
	}
	
	bSupportLocalFloorLevelEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_EXT_LOCAL_FLOOR_EXTENSION_NAME);
	bSupportPerformanceSettingsEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_EXT_PERFORMANCE_SETTINGS_EXTENSION_NAME);
	if (bSupportPerformanceSettingsEXT)
	{
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrPerfSettingsSetPerformanceLevelEXT", (PFN_xrVoidFunction*)&xrPerfSettingsSetPerformanceLevelEXT));
	}

	if (IOpenXRHMDModule::Get().IsExtensionEnabled(XR_EXT_USER_PRESENCE_EXTENSION_NAME))
	{
		XrSystemUserPresencePropertiesEXT SystemUserPresenceProperties = { XR_TYPE_SYSTEM_USER_PRESENCE_PROPERTIES_EXT };
		XrSystemProperties systemProperties = { XR_TYPE_SYSTEM_PROPERTIES, &SystemUserPresenceProperties };
		XR_ENSURE(xrGetSystemProperties(InInstance, System, &systemProperties));
		IsSupportsUserPresence = SystemUserPresenceProperties.supportsUserPresence == XR_TRUE;
	}

	if (IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_VIRTUAL_BOUNDARY_EXTENSION_NAME))
	{
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetVirtualBoundaryModePICO", (PFN_xrVoidFunction*)&xrGetVirtualBoundaryModePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetVirtualBoundaryStatusPICO", (PFN_xrVoidFunction*)&xrGetVirtualBoundaryStatusPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrSetVirtualBoundaryEnablePICO", (PFN_xrVoidFunction*)&xrSetVirtualBoundaryEnablePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrSetVirtualBoundaryVisiblePICO", (PFN_xrVoidFunction*)&xrSetVirtualBoundaryVisiblePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrSetVirtualBoundarySeeThroughVisiblePICO", (PFN_xrVoidFunction*)&xrSetVirtualBoundarySeeThroughVisiblePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetVirtualBoundaryTriggerPICO", (PFN_xrVoidFunction*)&xrGetVirtualBoundaryTriggerPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetVirtualBoundaryGeometryPICO", (PFN_xrVoidFunction*)&xrGetVirtualBoundaryGeometryPICO));

		bSupportedVirtualBoundary = true;
	}

	bSupportMRCExtension = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_EXTERNAL_CAMERA_EXTENSION_NAME);
	if (bSupportMRCExtension && !MRCDebugMode.EnableExtension)
	{
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetExternalCameraInfoPICO", (PFN_xrVoidFunction*)&xrGetExternalCameraInfoPICO));
	}

	bSupportedPICOCompositionLayerSettingsExt = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_LAYER_SETTINGS_EXTENSION_NAME);
	
	bSupportAdaptiveResolution = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_ADAPTIVE_RESOLUTION_EXTENSION_NAME);
	if (bSupportAdaptiveResolution)
	{
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrUpdateAdaptiveResolutionPICO", (PFN_xrVoidFunction*)&xrUpdateAdaptiveResolutionPICO));
	}
	
	bSupportColorMatrixExtension = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_LAYER_COLOR_MATRIX_EXTENSION_NAME);
	bSupportedFBCompositionLayerSettingsExt = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_FB_COMPOSITION_LAYER_SETTINGS_EXTENSION_NAME);

	// Load XR_EXT_future extension for async operations
	bSupportedFutureEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_EXT_FUTURE_EXTENSION_NAME);
	if (bSupportedFutureEXT)
	{
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrPollFutureEXT", (PFN_xrVoidFunction*)&xrPollFutureEXT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCancelFutureEXT", (PFN_xrVoidFunction*)&xrCancelFutureEXT));
		UE_LOG(LogPICOOpenXRHMD, Verbose, TEXT("XR_EXT_future extension loaded successfully"));
	}

	bSupportedCameraImage = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_CAMERA_IMAGE_EXTENSION_NAME);
	if (bSupportedCameraImage)
	{
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumerateAvailableCamerasPICO", (PFN_xrVoidFunction*)&xrEnumerateAvailableCamerasPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumerateCameraPropertyTypesPICO", (PFN_xrVoidFunction*)&xrEnumerateCameraPropertyTypesPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetCameraPropertiesPICO", (PFN_xrVoidFunction*)&xrGetCameraPropertiesPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumerateCameraCapabilityTypesPICO", (PFN_xrVoidFunction*)&xrEnumerateCameraCapabilityTypesPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetCameraSupportedCapabilitiesPICO", (PFN_xrVoidFunction*)&xrGetCameraSupportedCapabilitiesPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateCameraDeviceAsyncPICO", (PFN_xrVoidFunction*)&xrCreateCameraDeviceAsyncPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateCameraDeviceCompletePICO", (PFN_xrVoidFunction*)&xrCreateCameraDeviceCompletePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyCameraDevicePICO", (PFN_xrVoidFunction*)&xrDestroyCameraDevicePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateCameraCaptureSessionAsyncPICO", (PFN_xrVoidFunction*)&xrCreateCameraCaptureSessionAsyncPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateCameraCaptureSessionCompletePICO", (PFN_xrVoidFunction*)&xrCreateCameraCaptureSessionCompletePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyCameraCaptureSessionPICO", (PFN_xrVoidFunction*)&xrDestroyCameraCaptureSessionPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetCameraIntrinsicsPICO", (PFN_xrVoidFunction*)&xrGetCameraIntrinsicsPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetCameraExtrinsicsPICO", (PFN_xrVoidFunction*)&xrGetCameraExtrinsicsPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrBeginCameraCapturePICO", (PFN_xrVoidFunction*)&xrBeginCameraCapturePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEndCameraCapturePICO", (PFN_xrVoidFunction*)&xrEndCameraCapturePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrAcquireCameraImagePICO", (PFN_xrVoidFunction*)&xrAcquireCameraImagePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetCameraImageDataPICO", (PFN_xrVoidFunction*)&xrGetCameraImageDataPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrReleaseCameraImagePICO", (PFN_xrVoidFunction*)&xrReleaseCameraImagePICO));
	}

	bSupportedRenderModelEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_EXT_RENDER_MODEL_EXTENSION_NAME);
	bSupportedInteractionRenderModelEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_EXT_INTERACTION_RENDER_MODEL_EXTENSION_NAME);
	if (bSupportedRenderModelEXT && bSupportedInteractionRenderModelEXT)
	{
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumerateInteractionRenderModelIdsEXT", (PFN_xrVoidFunction*)&xrEnumerateInteractionRenderModelIdsEXT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateRenderModelEXT", (PFN_xrVoidFunction*)&xrCreateRenderModelEXT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyRenderModelEXT", (PFN_xrVoidFunction*)&xrDestroyRenderModelEXT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetRenderModelPropertiesEXT", (PFN_xrVoidFunction*)&xrGetRenderModelPropertiesEXT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateRenderModelAssetEXT", (PFN_xrVoidFunction*)&xrCreateRenderModelAssetEXT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyRenderModelAssetEXT", (PFN_xrVoidFunction*)&xrDestroyRenderModelAssetEXT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetRenderModelAssetDataEXT", (PFN_xrVoidFunction*)&xrGetRenderModelAssetDataEXT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumerateRenderModelSubactionPathsEXT", (PFN_xrVoidFunction*)&xrEnumerateRenderModelSubactionPathsEXT));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateRenderModelSpaceEXT", (PFN_xrVoidFunction*)&xrCreateRenderModelSpaceEXT));
	}
}

const void* FHMDPICO::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	static FName SystemName(TEXT("OpenXR"));
	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
	{
		OpenXRHMD = (FOpenXRHMD*)GEngine->XRSystem.Get();
	}

	UPICOOpenXRRuntimeSettings* Settings = GetMutableDefault<UPICOOpenXRRuntimeSettings>();
	if (Settings)
	{
		check(Settings != nullptr);
		if (Settings->DisableRHIThread)
		{
#if PLATFORM_ANDROID
			GPendingRHIThreadMode = ERHIThreadMode::None;
#endif //PLATFORM_ANDROID
		}
		
		EnableContentProtect(Settings->bContentProtectEXT);
	}

	return InNext;
}

float ConvertDisplayRefreshRate(EDisplayRefreshRatePICO Rate)
{
	switch (Rate)
	{
	case EDisplayRefreshRatePICO::Default:
		return 0.0f;
		break;
	case EDisplayRefreshRatePICO::Rate72:
		return 72.0f;
		break;
	case EDisplayRefreshRatePICO::Rate90:
		return 90.0f;
		break;
	case EDisplayRefreshRatePICO::Rate120:
		return 120.0f;
		break;
	}
	return 0.0f;
}

void FHMDPICO::PostCreateSession(XrSession InSession)
{
	FReadScopeLock Lock(SessionHandleMutex);
	Session = InSession;
	UPICOOpenXRRuntimeSettings* Settings = GetMutableDefault<UPICOOpenXRRuntimeSettings>();

	if (Settings && Settings->bDynamicResolution && bSupportAdaptiveResolution)
	{
		bDynamicResolution = true;
		MinimumResolutionScale = Settings->MinimumDynamicResolutionScale;
		CurrentAdaptiveResolutionSetting = Settings->AdaptiveResolutionSetting;
		GEngine->ChangeDynamicResolutionStateAtNextFrame(MakeShareable(new FDynamicResolutionStatePICO(this)));
	}

	if (bSupportDisplayRefreshRate)
	{
		uint32 DisplayRefreshRateCountOutput = 0;
		XR_ENSURE(xrEnumerateDisplayRefreshRatesFB(InSession, 0, &DisplayRefreshRateCountOutput, nullptr));
		if (DisplayRefreshRateCountOutput > 0)
		{
			SupportedDisplayRefreshRates.SetNum(DisplayRefreshRateCountOutput);
			XR_ENSURE(xrEnumerateDisplayRefreshRatesFB(InSession, (uint32)SupportedDisplayRefreshRates.Num(), &DisplayRefreshRateCountOutput, SupportedDisplayRefreshRates.GetData()));
			for (int i = 0; i < SupportedDisplayRefreshRates.Num(); i++)
			{
				UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Supported DisplayRefreshRates[%d]:%f"), i, SupportedDisplayRefreshRates[i]);
			}
		}

		XR_ENSURE(xrGetDisplayRefreshRateFB(InSession, &CurrentDisplayRefreshRate));
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Get current default DisplayRefreshRate:%f"), CurrentDisplayRefreshRate);

		if (Settings)
		{
			float RequestRate = ConvertDisplayRefreshRate(Settings->DisplayRefreshRate);
			SetDisplayRefreshRate(RequestRate);
		}
	}

	if (ViewTrackingSpace == XR_NULL_HANDLE)
	{
		XrReferenceSpaceCreateInfo SpaceInfo;
		SpaceInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
		SpaceInfo.next = nullptr;
		SpaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
		SpaceInfo.poseInReferenceSpace = ToXrPose(FTransform::Identity);
		XR_ENSURE(xrCreateReferenceSpace(InSession, &SpaceInfo, &ViewTrackingSpace));
	}

	if (bSupportMRCExtension && MRCSpace == XR_NULL_HANDLE)
	{
		XrMrcSpaceCreateInfoPICO MRCSpaceCreateInfoBd = { XR_TYPE_MRC_SPACE_CREATE_INFO_PICO };
		XrReferenceSpaceCreateInfo SpaceInfo = {};
		SpaceInfo.type = XR_TYPE_REFERENCE_SPACE_CREATE_INFO;
		SpaceInfo.next = &MRCSpaceCreateInfoBd;
		SpaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
		SpaceInfo.poseInReferenceSpace = ToXrPose(FTransform::Identity);
		XR_ENSURE(xrCreateReferenceSpace(InSession, &SpaceInfo, &MRCSpace));
	}

	CurrentSessionState = XR_SESSION_STATE_UNKNOWN;
}

bool FHMDPICO::GetRenderModelGLBBytes(EControllerHand Hand, TArray<uint8>& OutGlbBytes, FString& OutError)
{
	XrSession LocalSession = XR_NULL_HANDLE;
	XrInstance LocalInstance = XR_NULL_HANDLE;
	{
		FReadScopeLock Lock(SessionHandleMutex);
		LocalSession = Session;
		LocalInstance = Instance;
	}

	if (LocalSession == XR_NULL_HANDLE)
	{
		OutError = TEXT("Session is null");
		return false;
	}

	if (CurrentSessionState != XR_SESSION_STATE_SYNCHRONIZED && CurrentSessionState != XR_SESSION_STATE_VISIBLE && CurrentSessionState != XR_SESSION_STATE_FOCUSED)
	{
		OutError = FString::Printf(TEXT("Session state not ready: %d"), (int32)CurrentSessionState);
		return false;
	}

	if (!bSupportedRenderModelEXT || !bSupportedInteractionRenderModelEXT)
	{
		OutError = TEXT("XR_EXT_render_model or XR_EXT_interaction_render_model not enabled");
		return false;
	}

	if (!xrEnumerateInteractionRenderModelIdsEXT || !xrCreateRenderModelEXT || !xrDestroyRenderModelEXT || !xrGetRenderModelPropertiesEXT || !xrCreateRenderModelAssetEXT || !xrDestroyRenderModelAssetEXT || !xrGetRenderModelAssetDataEXT)
	{
		OutError = TEXT("Render model function pointers missing");
		return false;
	}

	if (Hand != EControllerHand::Left && Hand != EControllerHand::Right)
	{
		OutError = TEXT("Hand must be Left or Right");
		return false;
	}

	XrPath DesiredHandPath = XR_NULL_PATH;
	if (Hand == EControllerHand::Left || Hand == EControllerHand::Right)
	{
		if (LocalInstance != XR_NULL_HANDLE)
		{
			if (Hand == EControllerHand::Left && !bHasLeftHandUserPath)
			{
				bHasLeftHandUserPath = XR_SUCCEEDED(xrStringToPath(LocalInstance, "/user/hand/left", &LeftHandUserPath)) && LeftHandUserPath != XR_NULL_PATH;
			}
			if (Hand == EControllerHand::Right && !bHasRightHandUserPath)
			{
				bHasRightHandUserPath = XR_SUCCEEDED(xrStringToPath(LocalInstance, "/user/hand/right", &RightHandUserPath)) && RightHandUserPath != XR_NULL_PATH;
			}
		}

		DesiredHandPath = (Hand == EControllerHand::Left) ? LeftHandUserPath : RightHandUserPath;
		if (DesiredHandPath == XR_NULL_PATH)
		{
			OutError = TEXT("Requested hand user path is not available");
			return false;
		}
	}

	uint32 RenderModelIdCount = 0;
	XrInteractionRenderModelIdsEnumerateInfoEXT EnumInfo = { XR_TYPE_INTERACTION_RENDER_MODEL_IDS_ENUMERATE_INFO_EXT };
	const XrResult EnumRes0 = xrEnumerateInteractionRenderModelIdsEXT(LocalSession, &EnumInfo, 0, &RenderModelIdCount, nullptr);
	if (!XR_SUCCEEDED(EnumRes0))
	{
		OutError = FString::Printf(TEXT("xrEnumerateInteractionRenderModelIdsEXT(count) failed: %d"), (int32)EnumRes0);
		return false;
	}
	if (RenderModelIdCount == 0)
	{
		OutError = TEXT("No render model ids available");
		return false;
	}

	TArray<XrRenderModelIdEXT> RenderModelIds;
	RenderModelIds.SetNum(RenderModelIdCount);
	const XrResult EnumRes1 = xrEnumerateInteractionRenderModelIdsEXT(LocalSession, &EnumInfo, (uint32)RenderModelIds.Num(), &RenderModelIdCount, RenderModelIds.GetData());
	if (!XR_SUCCEEDED(EnumRes1))
	{
		OutError = FString::Printf(TEXT("xrEnumerateInteractionRenderModelIdsEXT(fill) failed: %d"), (int32)EnumRes1);
		return false;
	}
	if (RenderModelIdCount == 0)
	{
		OutError = TEXT("No render model ids returned");
		return false;
	}

	static const char* const GltfExtensions[] = { "KHR_materials_specular", "KHR_materials_ior" };
	XrRenderModelEXT RenderModel = XR_NULL_HANDLE;
	bool bMatched = false;
	for (XrRenderModelIdEXT RenderModelId : RenderModelIds)
	{
		if (RenderModelId == XR_NULL_RENDER_MODEL_ID_EXT)
		{
			continue;
		}

		XrRenderModelCreateInfoEXT CreateInfo = { XR_TYPE_RENDER_MODEL_CREATE_INFO_EXT };
		CreateInfo.renderModelId = RenderModelId;
		CreateInfo.gltfExtensionCount = UE_ARRAY_COUNT(GltfExtensions);
		CreateInfo.gltfExtensions = GltfExtensions;

		const XrResult CreateRMRes = xrCreateRenderModelEXT(LocalSession, &CreateInfo, &RenderModel);
		if (!XR_SUCCEEDED(CreateRMRes) || RenderModel == XR_NULL_HANDLE)
		{
			continue;
		}

		if (DesiredHandPath != XR_NULL_PATH && xrEnumerateRenderModelSubactionPathsEXT)
		{
			uint32 PathCount = 0;
			XrInteractionRenderModelSubactionPathInfoEXT Info = { XR_TYPE_INTERACTION_RENDER_MODEL_SUBACTION_PATH_INFO_EXT };
			const XrResult PathRes0 = xrEnumerateRenderModelSubactionPathsEXT(RenderModel, &Info, 0, &PathCount, nullptr);
			if (XR_SUCCEEDED(PathRes0) && PathCount > 0)
			{
				TArray<XrPath> Paths;
				Paths.SetNum(PathCount);
				const XrResult PathRes1 = xrEnumerateRenderModelSubactionPathsEXT(RenderModel, &Info, (uint32)Paths.Num(), &PathCount, Paths.GetData());
				if (XR_SUCCEEDED(PathRes1))
				{
					for (XrPath P : Paths)
					{
						if (P == DesiredHandPath)
						{
							bMatched = true;
							break;
						}
					}
				}
			}
		}
		else
		{
			bMatched = true;
		}

		if (bMatched)
		{
			break;
		}

		xrDestroyRenderModelEXT(RenderModel);
		RenderModel = XR_NULL_HANDLE;
	}

	if (!bMatched || RenderModel == XR_NULL_HANDLE)
	{
		OutError = TEXT("No matching render model id found");
		return false;
	}

	XrRenderModelAssetEXT Asset = XR_NULL_HANDLE;
	bool bSuccess = false;

	XrRenderModelPropertiesGetInfoEXT PropsGet = { XR_TYPE_RENDER_MODEL_PROPERTIES_GET_INFO_EXT };
	XrRenderModelPropertiesEXT Props = { XR_TYPE_RENDER_MODEL_PROPERTIES_EXT };
	const XrResult PropsRes = xrGetRenderModelPropertiesEXT(RenderModel, &PropsGet, &Props);
	if (XR_SUCCEEDED(PropsRes))
	{
		XrRenderModelAssetCreateInfoEXT AssetCreate = { XR_TYPE_RENDER_MODEL_ASSET_CREATE_INFO_EXT };
		AssetCreate.cacheId = Props.cacheId;
		const XrResult CreateAssetRes = xrCreateRenderModelAssetEXT(LocalSession, &AssetCreate, &Asset);
		if (XR_SUCCEEDED(CreateAssetRes) && Asset != XR_NULL_HANDLE)
		{
			XrRenderModelAssetDataGetInfoEXT DataGet = { XR_TYPE_RENDER_MODEL_ASSET_DATA_GET_INFO_EXT };
			XrRenderModelAssetDataEXT Data = { XR_TYPE_RENDER_MODEL_ASSET_DATA_EXT };
			Data.bufferCapacityInput = 0;
			Data.buffer = nullptr;

			const XrResult GetSizeRes = xrGetRenderModelAssetDataEXT(Asset, &DataGet, &Data);
			if (XR_SUCCEEDED(GetSizeRes) && Data.bufferCountOutput > 0)
			{
				OutGlbBytes.SetNumUninitialized((int32)Data.bufferCountOutput);
				Data.bufferCapacityInput = (uint32)OutGlbBytes.Num();
				Data.buffer = OutGlbBytes.GetData();

				const XrResult GetDataRes = xrGetRenderModelAssetDataEXT(Asset, &DataGet, &Data);
				if (XR_SUCCEEDED(GetDataRes) && Data.bufferCountOutput <= (uint32)OutGlbBytes.Num())
				{
					bSuccess = true;
				}
				else
				{
					OutError = FString::Printf(TEXT("xrGetRenderModelAssetDataEXT(data) failed: %d"), (int32)GetDataRes);
				}
			}
			else
			{
				OutError = FString::Printf(TEXT("xrGetRenderModelAssetDataEXT(size) failed: %d"), (int32)GetSizeRes);
			}
		}
		else
		{
			OutError = FString::Printf(TEXT("xrCreateRenderModelAssetEXT failed: %d"), (int32)CreateAssetRes);
		}
	}
	else
	{
		OutError = FString::Printf(TEXT("xrGetRenderModelPropertiesEXT failed: %d"), (int32)PropsRes);
	}

	if (Asset != XR_NULL_HANDLE)
	{
		xrDestroyRenderModelAssetEXT(Asset);
	}
	xrDestroyRenderModelEXT(RenderModel);

	return bSuccess;
}

bool FHMDPICO::GetRenderModelWorldTransform(EControllerHand Hand, FTransform& OutWorldTransform, FString& OutError)
{
	XrSession LocalSession = XR_NULL_HANDLE;
	XrInstance LocalInstance = XR_NULL_HANDLE;
	XrSpace LocalBaseSpace = XR_NULL_HANDLE;
	XrTime LocalDisplayTime = 0;
	{
		FReadScopeLock Lock(SessionHandleMutex);
		LocalSession = Session;
		LocalInstance = Instance;
		LocalBaseSpace = CurrentBaseSpace;
		LocalDisplayTime = CurrentDisplayTime;
	}

	if (LocalSession == XR_NULL_HANDLE)
	{
		OutError = TEXT("Session is null");
		return false;
	}

	if (LocalBaseSpace == XR_NULL_HANDLE)
	{
		OutError = TEXT("Base space is null");
		return false;
	}

	if (CurrentSessionState != XR_SESSION_STATE_SYNCHRONIZED && CurrentSessionState != XR_SESSION_STATE_VISIBLE && CurrentSessionState != XR_SESSION_STATE_FOCUSED)
	{
		OutError = FString::Printf(TEXT("Session state not ready: %d"), (int32)CurrentSessionState);
		return false;
	}

	if (!bSupportedRenderModelEXT || !bSupportedInteractionRenderModelEXT)
	{
		OutError = TEXT("XR_EXT_render_model or XR_EXT_interaction_render_model not enabled");
		return false;
	}

	if (!xrEnumerateInteractionRenderModelIdsEXT || !xrCreateRenderModelEXT || !xrDestroyRenderModelEXT || !xrEnumerateRenderModelSubactionPathsEXT || !xrCreateRenderModelSpaceEXT)
	{
		OutError = TEXT("Render model function pointers missing (space)");
		return false;
	}

	if (Hand != EControllerHand::Left && Hand != EControllerHand::Right)
	{
		OutError = TEXT("Hand must be Left or Right");
		return false;
	}

	XrPath DesiredHandPath = XR_NULL_PATH;
	if (LocalInstance != XR_NULL_HANDLE)
	{
		if (Hand == EControllerHand::Left && !bHasLeftHandUserPath)
		{
			bHasLeftHandUserPath = XR_SUCCEEDED(xrStringToPath(LocalInstance, "/user/hand/left", &LeftHandUserPath)) && LeftHandUserPath != XR_NULL_PATH;
		}
		if (Hand == EControllerHand::Right && !bHasRightHandUserPath)
		{
			bHasRightHandUserPath = XR_SUCCEEDED(xrStringToPath(LocalInstance, "/user/hand/right", &RightHandUserPath)) && RightHandUserPath != XR_NULL_PATH;
		}
	}
	DesiredHandPath = (Hand == EControllerHand::Left) ? LeftHandUserPath : RightHandUserPath;
	if (DesiredHandPath == XR_NULL_PATH)
	{
		OutError = TEXT("Requested hand user path is not available");
		return false;
	}

	XrRenderModelEXT& ModelHandle = (Hand == EControllerHand::Left) ? LeftRenderModelForSpace : RightRenderModelForSpace;
	XrSpace& ModelSpace = (Hand == EControllerHand::Left) ? LeftRenderModelSpace : RightRenderModelSpace;
	{
		FScopeLock Lock(&RenderModelSpaceMutex);
		if (ModelSpace == XR_NULL_HANDLE || ModelHandle == XR_NULL_HANDLE)
		{
			if (ModelSpace != XR_NULL_HANDLE)
			{
				xrDestroySpace(ModelSpace);
				ModelSpace = XR_NULL_HANDLE;
			}
			if (ModelHandle != XR_NULL_HANDLE)
			{
				xrDestroyRenderModelEXT(ModelHandle);
				ModelHandle = XR_NULL_HANDLE;
			}

			uint32 RenderModelIdCount = 0;
			XrInteractionRenderModelIdsEnumerateInfoEXT EnumInfo = { XR_TYPE_INTERACTION_RENDER_MODEL_IDS_ENUMERATE_INFO_EXT };
			const XrResult EnumRes0 = xrEnumerateInteractionRenderModelIdsEXT(LocalSession, &EnumInfo, 0, &RenderModelIdCount, nullptr);
			if (!XR_SUCCEEDED(EnumRes0) || RenderModelIdCount == 0)
			{
				OutError = FString::Printf(TEXT("xrEnumerateInteractionRenderModelIdsEXT(count) failed: %d"), (int32)EnumRes0);
				return false;
			}

			TArray<XrRenderModelIdEXT> RenderModelIds;
			RenderModelIds.SetNum(RenderModelIdCount);
			const XrResult EnumRes1 = xrEnumerateInteractionRenderModelIdsEXT(LocalSession, &EnumInfo, (uint32)RenderModelIds.Num(), &RenderModelIdCount, RenderModelIds.GetData());
			if (!XR_SUCCEEDED(EnumRes1) || RenderModelIdCount == 0)
			{
				OutError = FString::Printf(TEXT("xrEnumerateInteractionRenderModelIdsEXT(fill) failed: %d"), (int32)EnumRes1);
				return false;
			}

			bool bMatched = false;
			for (XrRenderModelIdEXT RenderModelId : RenderModelIds)
			{
				if (RenderModelId == XR_NULL_RENDER_MODEL_ID_EXT)
				{
					continue;
				}
				XrRenderModelCreateInfoEXT CreateInfo = { XR_TYPE_RENDER_MODEL_CREATE_INFO_EXT };
				CreateInfo.renderModelId = RenderModelId;
				CreateInfo.gltfExtensionCount = 0;
				CreateInfo.gltfExtensions = nullptr;

				XrRenderModelEXT TempModel = XR_NULL_HANDLE;
				const XrResult CreateRes = xrCreateRenderModelEXT(LocalSession, &CreateInfo, &TempModel);
				if (!XR_SUCCEEDED(CreateRes) || TempModel == XR_NULL_HANDLE)
				{
					continue;
				}

				uint32 PathCount = 0;
				XrInteractionRenderModelSubactionPathInfoEXT Info = { XR_TYPE_INTERACTION_RENDER_MODEL_SUBACTION_PATH_INFO_EXT };
				const XrResult PathRes0 = xrEnumerateRenderModelSubactionPathsEXT(TempModel, &Info, 0, &PathCount, nullptr);
				if (XR_SUCCEEDED(PathRes0) && PathCount > 0)
				{
					TArray<XrPath> Paths;
					Paths.SetNum(PathCount);
					const XrResult PathRes1 = xrEnumerateRenderModelSubactionPathsEXT(TempModel, &Info, (uint32)Paths.Num(), &PathCount, Paths.GetData());
					if (XR_SUCCEEDED(PathRes1))
					{
						for (XrPath P : Paths)
						{
							if (P == DesiredHandPath)
							{
								bMatched = true;
								break;
							}
						}
					}
				}

				if (!bMatched)
				{
					xrDestroyRenderModelEXT(TempModel);
					continue;
				}

				XrRenderModelSpaceCreateInfoEXT SpaceCreate = { XR_TYPE_RENDER_MODEL_SPACE_CREATE_INFO_EXT };
				SpaceCreate.renderModel = TempModel;
				XrSpace TempSpace = XR_NULL_HANDLE;
				const XrResult SpaceRes = xrCreateRenderModelSpaceEXT(LocalSession, &SpaceCreate, &TempSpace);
				if (!XR_SUCCEEDED(SpaceRes) || TempSpace == XR_NULL_HANDLE)
				{
					xrDestroyRenderModelEXT(TempModel);
					OutError = FString::Printf(TEXT("xrCreateRenderModelSpaceEXT failed: %d"), (int32)SpaceRes);
					return false;
				}

				ModelHandle = TempModel;
				ModelSpace = TempSpace;
				break;
			}

			if (ModelSpace == XR_NULL_HANDLE || ModelHandle == XR_NULL_HANDLE)
			{
				OutError = TEXT("No matching render model space found");
				return false;
			}
		}
	}

	XrSpaceLocation Loc = { XR_TYPE_SPACE_LOCATION };
	const XrResult LocateRes = xrLocateSpace(ModelSpace, LocalBaseSpace, LocalDisplayTime, &Loc);
	if (!XR_SUCCEEDED(LocateRes))
	{
		OutError = FString::Printf(TEXT("xrLocateSpace(renderModelSpace) failed: %d"), (int32)LocateRes);
		return false;
	}

	const bool bHasPos = (Loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) != 0;
	const bool bHasOri = (Loc.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) != 0;
	if (!bHasPos || !bHasOri)
	{
		OutError = TEXT("Render model space pose not valid");
		return false;
	}

	const float WorldToMeters = OpenXRHMD ? OpenXRHMD->GetWorldToMetersScale() : 100.0f;
	const FTransform PoseInTracking = ToFTransform(Loc.pose, WorldToMeters);
	const FTransform TrackingToWorld = OpenXRHMD ? OpenXRHMD->GetTrackingToWorldTransform() : FTransform::Identity;
	OutWorldTransform = PoseInTracking * TrackingToWorld;
	return true;
}

void FHMDPICO::OnDestroySession(XrSession InSession)
{
	// Clean up Camera Image resources
	if (bSupportedCameraImage)
	{
		// Clean up all capture sessions
		TArray<int64> SessionsToDestroy;
		for (auto& Pair : CaptureSessions)
		{
			SessionsToDestroy.Add(Pair.Key);
		}
		
		for (int64 SessionHandle : SessionsToDestroy)
		{
			FCameraCaptureSessionInfo* SessionInfo = CaptureSessions.Find(SessionHandle);
			if (SessionInfo && SessionInfo->bValid)
			{
				// End capture if still capturing
				if (SessionInfo->bCapturing && xrEndCameraCapturePICO)
				{
					xrEndCameraCapturePICO(SessionInfo->Session);
				}
				
				// Release all acquired images
				if (xrReleaseCameraImagePICO)
				{
					for (auto& ImagePair : SessionInfo->AcquiredImages)
					{
						if (ImagePair.Value)
						{
							xrReleaseCameraImagePICO(SessionInfo->Session, ImagePair.Key);
						}
					}
				}
				
				// Destroy session
				if (xrDestroyCameraCaptureSessionPICO)
				{
					xrDestroyCameraCaptureSessionPICO(SessionInfo->Session);
				}
			}
		}
		CaptureSessions.Empty();
		
		// Clean up all camera devices
		TArray<int64> DevicesToDestroy;
		for (auto& Pair : CameraDevices)
		{
			DevicesToDestroy.Add(Pair.Key);
		}
		
		for (int64 DeviceHandle : DevicesToDestroy)
		{
			FCameraDeviceInfo* DeviceInfo = CameraDevices.Find(DeviceHandle);
			if (DeviceInfo && DeviceInfo->bValid && xrDestroyCameraDevicePICO)
			{
				xrDestroyCameraDevicePICO(DeviceInfo->Device);
			}
		}
		CameraDevices.Empty();
		
		PendingDeviceCreation.Empty();
		PendingSessionCreation.Empty();
		
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera Image resources cleaned up"));
	}

	if (ViewTrackingSpace)
	{
		XR_ENSURE(xrDestroySpace(ViewTrackingSpace));
	}
	ViewTrackingSpace = XR_NULL_HANDLE;


	if (MRCSpace)
	{
		XR_ENSURE(xrDestroySpace(MRCSpace));
	}
	MRCSpace = XR_NULL_HANDLE;

	{
		FScopeLock Lock(&RenderModelSpaceMutex);
		if (LeftRenderModelSpace)
		{
			XR_ENSURE(xrDestroySpace(LeftRenderModelSpace));
		}
		LeftRenderModelSpace = XR_NULL_HANDLE;
		if (RightRenderModelSpace)
		{
			XR_ENSURE(xrDestroySpace(RightRenderModelSpace));
		}
		RightRenderModelSpace = XR_NULL_HANDLE;

		if (LeftRenderModelForSpace)
		{
			XR_ENSURE(xrDestroyRenderModelEXT(LeftRenderModelForSpace));
		}
		LeftRenderModelForSpace = XR_NULL_HANDLE;
		if (RightRenderModelForSpace)
		{
			XR_ENSURE(xrDestroyRenderModelEXT(RightRenderModelForSpace));
		}
		RightRenderModelForSpace = XR_NULL_HANDLE;
	}
}

bool FHMDPICO::UseCustomReferenceSpaceType(XrReferenceSpaceType& OutReferenceSpaceType)
{
	if (bSupportLocalFloorLevelEXT)
	{
		UPICOOpenXRRuntimeSettings* Settings = GetMutableDefault<UPICOOpenXRRuntimeSettings>();
		if (Settings && Settings->bLocalFloorLevelEXT)
		{
			OutReferenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL_FLOOR_EXT;
			return true;
		}
	}
	return false;
}

bool FHMDPICO::GetSpectatorScreenController(FHeadMountedDisplayBase* InHMDBase, TUniquePtr<FDefaultSpectatorScreenController>& OutSpectatorScreenController)
{
#if PLATFORM_ANDROID
	OutSpectatorScreenController = nullptr;
	return true;
#else // PLATFORM_ANDROID
	OutSpectatorScreenController = MakeUnique<FDefaultSpectatorScreenController>(InHMDBase);
	return false;
#endif // PLATFORM_ANDROID
}

static const TCHAR* GetSessionStateName(XrSessionState State)
{
	switch (State)
	{
	case XR_SESSION_STATE_IDLE: return TEXT("IDLE");
	case XR_SESSION_STATE_READY: return TEXT("READY");
	case XR_SESSION_STATE_SYNCHRONIZED: return TEXT("SYNCHRONIZED");
	case XR_SESSION_STATE_VISIBLE: return TEXT("VISIBLE");
	case XR_SESSION_STATE_FOCUSED: return TEXT("FOCUSED");
	case XR_SESSION_STATE_STOPPING: return TEXT("STOPPING");
	case XR_SESSION_STATE_LOSS_PENDING: return TEXT("LOSS_PENDING");
	case XR_SESSION_STATE_EXITING: return TEXT("EXITING");
	default: return TEXT("UNKNOWN");
	}
}

void FHMDPICO::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
{
	const XrEventDataBuffer* EventDataBuffer = reinterpret_cast<const XrEventDataBuffer*>(InHeader);

	if (EventDataBuffer == nullptr)
	{
		return;
	}

	switch (EventDataBuffer->type)
	{
	case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
	{
		const XrEventDataSessionStateChanged* SessionStateChanged = reinterpret_cast<const XrEventDataSessionStateChanged*>(EventDataBuffer);
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("XR session state changed: %d (%s)"), (int32)SessionStateChanged->state, GetSessionStateName(SessionStateChanged->state));
		CurrentSessionState = SessionStateChanged->state;
		OnXRSessionStateChangedDelegate.Broadcast((int32)CurrentSessionState);
	}
	break;
	case XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB:
		if (bSupportDisplayRefreshRate)
		{
			const XrEventDataDisplayRefreshRateChangedFB* DisplayRefreshRate = reinterpret_cast<const XrEventDataDisplayRefreshRateChangedFB*>(EventDataBuffer);
			CurrentDisplayRefreshRate = DisplayRefreshRate->toDisplayRefreshRate;
			UHMDFunctionLibraryPICO::GetDelegateManagerPICO()->OnDeviceDisplayRefreshRateChanged.Broadcast(DisplayRefreshRate->fromDisplayRefreshRate, DisplayRefreshRate->toDisplayRefreshRate);
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("DisplayRefreshRate changed from %f to %f."), DisplayRefreshRate->fromDisplayRefreshRate, DisplayRefreshRate->toDisplayRefreshRate);
		}
		break;
	case XR_TYPE_EVENT_DATA_PERF_SETTINGS_EXT:
		if (bSupportPerformanceSettingsEXT)
		{
			const XrEventDataPerfSettingsEXT* PerfSettings = reinterpret_cast<const XrEventDataPerfSettingsEXT*>(EventDataBuffer);
			UHMDFunctionLibraryPICO::GetDelegateManagerPICO()->OnDevicePerformanceSettingsChanged.Broadcast(EPerfSettingsDomainPICO(PerfSettings->domain)
				, EPerfSettingsSubDomainPICO(PerfSettings->subDomain)
				, EPerfSettingsNotificationLevelPICO(PerfSettings->toLevel)
				, EPerfSettingsNotificationLevelPICO(PerfSettings->fromLevel));
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("PerformanceSettings level changed from %d to %d (domain:%d subdomain:%d"), PerfSettings->fromLevel, PerfSettings->toLevel, PerfSettings->domain, PerfSettings->subDomain);
		}
		break;
	case XR_TYPE_EVENT_DATA_USER_PRESENCE_CHANGED_EXT:
		if (IsSupportsUserPresence)
		{
			const XrEventDataUserPresenceChangedEXT* UserPresenceChanged = reinterpret_cast<const XrEventDataUserPresenceChangedEXT*>(EventDataBuffer);
			if (UserPresenceChanged->isUserPresent)
			{
				WornState = EHMDWornState::Type::Worn;
				FCoreDelegates::VRHeadsetPutOnHead.Broadcast();
			}
			else
			{
				WornState = EHMDWornState::Type::NotWorn;
				FCoreDelegates::VRHeadsetRemovedFromHead.Broadcast();
			}
		}
		break;
	case XR_TYPE_EVENT_DATA_MRC_STATUS_CHANGED_PICO:
		if (bSupportMRCExtension)
		{
			const XrEventDataMrcStatusChangedPICO* MRCStatusChanged = reinterpret_cast<const XrEventDataMrcStatusChangedPICO*>(EventDataBuffer);
			bIsMRCRunningStored = bIsMRCRunning = MRCStatusChanged->mrcStatus == 1;
			UHMDFunctionLibraryPICO::GetDelegateManagerPICO()->OnMRCStatusChanged.Broadcast(bIsMRCRunning);
		}
		break;
	default:
		break;
	}
}

const void* FHMDPICO::OnEndProjectionLayer_RHIThread(XrSession InSession, int32 InLayerIndex, const void* InNext, XrCompositionLayerFlags& OutFlags)
{
	if (bIsPicoRuntime)
	{
		OutFlags |= XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
		OutFlags |= XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT;
	}
	if (bContentProtectEnabled)
	{
		ContentProtect = { XR_TYPE_COMPOSITION_LAYER_SECURE_CONTENT_FB };
		ContentProtect.next = const_cast<void*>(InNext);
		ContentProtect.flags = XR_COMPOSITION_LAYER_SECURE_CONTENT_REPLACE_LAYER_BIT_FB;
		InNext = &ContentProtect;
	}

	static const auto CVarPICOSharpening = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.Mobile.PICO.SharpeningSetting"));
	const ESharpeningTypePICO SharpeningType = static_cast<ESharpeningTypePICO>(CVarPICOSharpening->GetValueOnAnyThread());

	if (bSupportedFBCompositionLayerSettingsExt)
	{
		bool bAttachFBCompositionLayerSettings = false;
		ProjectionLayerSettingsFB = { XR_TYPE_COMPOSITION_LAYER_SETTINGS_FB };
		switch (SharpeningType)
		{
		case ESharpeningTypePICO::NormalSharpening:
		{
			ProjectionLayerSettingsFB.layerFlags |= XR_COMPOSITION_LAYER_SETTINGS_NORMAL_SHARPENING_BIT_FB;
			bAttachFBCompositionLayerSettings = true;
		}
		break;
		case ESharpeningTypePICO::QualitySharpening:
		{
			ProjectionLayerSettingsFB.layerFlags |= XR_COMPOSITION_LAYER_SETTINGS_QUALITY_SHARPENING_BIT_FB;
			bAttachFBCompositionLayerSettings = true;
		}
		break;
		default:;
		}

		if (bAttachFBCompositionLayerSettings)
		{
			ProjectionLayerSettingsFB.next = const_cast<void*>(InNext);
			InNext = &ProjectionLayerSettingsFB;
		}
	}

	if (bSupportedPICOCompositionLayerSettingsExt)
	{
		bool bAttachPICOCompositionLayerSettings = false;
		ProjectionLayerSettings = { XR_TYPE_LAYER_SETTINGS_PICO };

		static const auto CVarPICOSuperResolution = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.Mobile.PICO.EnableSuperResolution"));
		if (CVarPICOSuperResolution->GetValueOnAnyThread() == 1)
		{
			ProjectionLayerSettings.layerFlags |= XR_LAYER_SETTINGS_SUPER_RESOLUTION_BIT_PICO;
			bAttachPICOCompositionLayerSettings = true;
		}

		static const auto CVarPICOSharpeningEnhance = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.Mobile.PICO.SharpeningEnhanceMode"));
		const ESharpeningEnhanceModePICO EnhanceMode = static_cast<ESharpeningEnhanceModePICO>(CVarPICOSharpeningEnhance->GetValueOnAnyThread());

		switch (SharpeningType)
		{
		case ESharpeningTypePICO::NormalSharpening:
		{
			switch (EnhanceMode)
			{
			case ESharpeningEnhanceModePICO::FixedFoveated:
			{
				ProjectionLayerSettings.layerFlags |= XR_LAYER_SETTINGS_FIXED_FOVEATED_NORMAL_SHARPENING_BIT_PICO;
				bAttachPICOCompositionLayerSettings = true;
			}
			break;
			case ESharpeningEnhanceModePICO::Adaptive:
			{
				ProjectionLayerSettings.layerFlags |= XR_LAYER_SETTINGS_SELF_ADAPTIVE_NORMAL_SHARPENING_BIT_PICO;
				bAttachPICOCompositionLayerSettings = true;
			}
			break;
			case ESharpeningEnhanceModePICO::Both:
			{
				ProjectionLayerSettings.layerFlags |= XR_LAYER_SETTINGS_FIXED_FOVEATED_NORMAL_SHARPENING_BIT_PICO;
				ProjectionLayerSettings.layerFlags |= XR_LAYER_SETTINGS_SELF_ADAPTIVE_NORMAL_SHARPENING_BIT_PICO;
				bAttachPICOCompositionLayerSettings = true;
			}
			break;
			default:;
			}
		}
		break;
		case ESharpeningTypePICO::QualitySharpening:
		{
			switch (EnhanceMode)
			{
			case ESharpeningEnhanceModePICO::FixedFoveated:
			{
				ProjectionLayerSettings.layerFlags |= XR_LAYER_SETTINGS_FIXED_FOVEATED_QUALITY_SHARPENING_BIT_PICO;
				bAttachPICOCompositionLayerSettings = true;
			}
			break;
			case ESharpeningEnhanceModePICO::Adaptive:
			{
				ProjectionLayerSettings.layerFlags |= XR_LAYER_SETTINGS_SELF_ADAPTIVE_QUALITY_SHARPENING_BIT_PICO;
				bAttachPICOCompositionLayerSettings = true;
			}
			break;
			case ESharpeningEnhanceModePICO::Both:
			{
				ProjectionLayerSettings.layerFlags |= XR_LAYER_SETTINGS_FIXED_FOVEATED_QUALITY_SHARPENING_BIT_PICO;
				ProjectionLayerSettings.layerFlags |= XR_LAYER_SETTINGS_SELF_ADAPTIVE_QUALITY_SHARPENING_BIT_PICO;
				bAttachPICOCompositionLayerSettings = true;
			}
			break;
			default:;
			}
		}
		break;
		default:;
		}

		if (bAttachPICOCompositionLayerSettings)
		{
			ProjectionLayerSettings.next = const_cast<void*>(InNext);
			InNext = &ProjectionLayerSettings;
		}
	}

	if (bSupportColorMatrixExtension && bUseColorMatrixExtension)
	{
		XrLayerColorMatrixPICO LayerColorMatrix = { XR_TYPE_LAYER_COLOR_MATRIX_PICO };
		FMemory::Memcpy(LayerColorMatrix.matrix.m, ColorMatrix3x3f, 9);
		LayerColorMatrix.next = const_cast<void*>(InNext);
		InNext = &LayerColorMatrix;
	}

	return InNext;
}

void FHMDPICO::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
	CurrentDisplayTime = DisplayTime;
	CurrentBaseSpace = TrackingSpace;
}

void FHMDPICO::OnBeginRendering_GameThread(XrSession InSession, FSceneViewFamily& InViewFamily, TArrayView<const uint32> VisibleLayers)
{
	if (bDynamicResolution)
	{
		float NewPixelDensity = 1.0;
		float PixelDensity = GetAdaptivePixelDensity(CurrentAdaptiveResolutionSetting, NewPixelDensity) ? NewPixelDensity : 1.0f;
		
		static const auto CVarPICODynamicPixelDensity = IConsoleManager::Get().FindTConsoleVariableDataFloat(TEXT("r.PICO.DynamicResolution.PixelDensity"));
		const float PixelDensityCVarOverride = CVarPICODynamicPixelDensity != nullptr ? CVarPICODynamicPixelDensity->GetValueOnAnyThread() : 0.0f;
		if (PixelDensityCVarOverride > 0.0f)
		{
			PixelDensity = PixelDensityCVarOverride;
		}

		CurrentDynamicPixelDensity = FMath::Clamp(PixelDensity, MinimumResolutionScale, 1.0f);
	}

	ENQUEUE_RENDER_COMMAND(UpdateMRCState)(
		[this, IsMRCForegroundLayerDisabled = bIsMRCForegroundLayerDisabled](FRHICommandListImmediate&)
		{
			bIsMRCForegroundLayerDisabled_RebderThread = IsMRCForegroundLayerDisabled;
		});
}

FIntRect FHMDPICO::GetViewportSize(const FOpenXRLayer::FPerEyeTextureData& EyeData, const IStereoLayers::FLayerDesc& Desc)
{
	FBox2D Viewport(EyeData.SwapchainSize * Desc.UVRect.Min, EyeData.SwapchainSize * Desc.UVRect.Max);
	return FIntRect(Viewport.Min.IntPoint(), Viewport.Max.IntPoint());
}

FVector2D FHMDPICO::GetQuadSize(const FOpenXRLayer::FPerEyeTextureData& EyeData, const IStereoLayers::FLayerDesc& Desc)
{
	if (Desc.Flags & IStereoLayers::LAYER_FLAG_QUAD_PRESERVE_TEX_RATIO)
	{
		float AspectRatio = EyeData.SwapchainSize.Y / EyeData.SwapchainSize.X;
		return FVector2D(Desc.QuadSize.X, Desc.QuadSize.X * AspectRatio);
	}
	return Desc.QuadSize;
}

void FHMDPICO::OnWorldTickStart(UWorld* InWorld, ELevelTick TickType, float DeltaTime)
{
	if (bSupportMRCExtension && IsInGameThread())
	{
		if (bIsMRCRunning && !MRCSceneCapture2DPICO)
		{
			FWorldContext& Context = GEngine->GetWorldContextFromWorldChecked(InWorld);
			if (((Context.WorldType == EWorldType::PIE) || (Context.WorldType == EWorldType::Game)) && (Context.World() != NULL))
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				SpawnInfo.bNoFail = true;
				SpawnInfo.ObjectFlags = RF_Transient;
				MRCSceneCapture2DPICO = InWorld->SpawnActor<AMRCCameraPICO>(SpawnInfo);
				MRCSceneCapture2DPICO->SetActorEnableCollision(false);
			}
		}
		else if (!bIsMRCRunning && MRCSceneCapture2DPICO)
		{
			MRCSceneCapture2DPICO->Destroy();
			MRCSceneCapture2DPICO = nullptr;
		}
	}
}

void FHMDPICO::OnBeginRendering_RenderThread(XrSession InSession, FRDGBuilder& GraphBuilder)
{
	if (OpenXRHMD == nullptr || InSession == XR_NULL_HANDLE)
	{
		return;
	}

	FXRRenderBridge* RenderBridge = OpenXRHMD->GetActiveRenderBridge_GameThread(OpenXRHMD->ShouldUseSeparateRenderTarget());
	FOpenXRRenderBridge* Bridge = static_cast<FOpenXRRenderBridge*>(RenderBridge);
	uint8 UnusedActualFormat = 0;
	ETextureCreateFlags Flags = TexCreate_Dynamic | TexCreate_SRGB | TexCreate_ShaderResource | TexCreate_RenderTargetable;
	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	if (bSupportMRCExtension && bIsMRCRunning && !MRCLayer.IsValid())
	{
		if (Bridge && MRCLayerDesc_RenderThread.TextureObj.IsValid() && MRCLayerDesc_RenderThread.LeftTextureObj.IsValid())
		{
			MRCLayer = MakeShareable(new FOpenXRLayer(MRCLayerDesc_RenderThread));

			{
				FRHITexture* Texture = MRCLayer->Desc.TextureObj->GetResource()->TextureRHI;
				FXRSwapChainPtr SwapChain = Bridge->CreateSwapchain(InSession,
					IStereoRenderTargetManager::GetStereoLayerPixelFormat(),
					UnusedActualFormat,
					Texture->GetSizeX(),
					Texture->GetSizeY(),
#ifdef PICO_CUSTOM_ENGINE
					1,
#endif
					1,
					Texture->GetNumMips(),
					Texture->GetNumSamples(),
					Texture->GetFlags() | Flags,
					Texture->GetClearBinding());
				MRCLayer->RightEye.Swapchain = SwapChain;
				MRCLayer->RightEye.SwapchainSize = Texture->GetSizeXY();
				MRCLayer->RightEye.bStaticSwapchain = false;
				MRCLayer->RightEye.bUpdateTexture = true;
				MRCLayer->RightEye.Texture = Texture;
			}

			{
				FRHITexture* Texture = MRCLayer->Desc.LeftTextureObj->GetResource()->TextureRHI;
				FXRSwapChainPtr SwapChain = Bridge->CreateSwapchain(InSession,
					IStereoRenderTargetManager::GetStereoLayerPixelFormat(),
					UnusedActualFormat,
					Texture->GetSizeX(),
					Texture->GetSizeY(),
#ifdef PICO_CUSTOM_ENGINE
					1,
#endif
					1,
					Texture->GetNumMips(),
					Texture->GetNumSamples(),
					Texture->GetFlags() | Flags,
					Texture->GetClearBinding());
				MRCLayer->LeftEye.Swapchain = SwapChain;
				MRCLayer->LeftEye.SwapchainSize = Texture->GetSizeXY();
				MRCLayer->LeftEye.bStaticSwapchain = false;
				MRCLayer->LeftEye.bUpdateTexture = true;
				MRCLayer->LeftEye.Texture = Texture;
			}

			const bool bNoAlpha = MRCLayer->Desc.Flags & IStereoLayers::LAYER_FLAG_TEX_NO_ALPHA_CHANNEL;
			const bool bIsStereo = MRCLayer->Desc.LeftTextureObj.IsValid();
			const FTransform PositionTransform = FTransform::Identity;
			float WorldToMeters = OpenXRHMD->GetWorldToMetersScale();

			XrCompositionLayerQuad MRCQuadLayer = { XR_TYPE_COMPOSITION_LAYER_QUAD, nullptr };
			MRCQuadLayer.layerFlags = bNoAlpha ? 0 : XR_COMPOSITION_LAYER_UNPREMULTIPLIED_ALPHA_BIT | XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
			MRCQuadLayer.layerFlags |= XR_COMPOSITION_LAYER_MRC_COMPOSITION_BIT_PICO;
			MRCQuadLayer.space = ViewTrackingSpace;
			MRCQuadLayer.subImage.imageArrayIndex = 0;
			MRCQuadLayer.pose = ToXrPose(MRCLayer->Desc.Transform * PositionTransform, WorldToMeters);

			const FVector2D LayerComponentScaler(MRCLayer->Desc.Transform.GetScale3D().Y, MRCLayer->Desc.Transform.GetScale3D().Z);

			if (MRCLayer->RightEye.Swapchain.IsValid())
			{
				MRCQuadLayer.eyeVisibility = bIsStereo ? XR_EYE_VISIBILITY_RIGHT : XR_EYE_VISIBILITY_BOTH;

				MRCQuadLayer.subImage.imageRect = ToXrRect(GetViewportSize(MRCLayer->RightEye, MRCLayer->Desc));
				MRCQuadLayer.subImage.swapchain = static_cast<FOpenXRSwapchain*>(MRCLayer->RightEye.Swapchain.Get())->GetHandle();
				MRCQuadLayer.size = ToXrExtent2D(GetQuadSize(MRCLayer->RightEye, MRCLayer->Desc) * LayerComponentScaler, WorldToMeters);
				MRCQuadLayerRight_RenderThread = MRCQuadLayer;
			}

			if (MRCLayer->LeftEye.Swapchain.IsValid())
			{
				MRCQuadLayer.eyeVisibility = XR_EYE_VISIBILITY_LEFT;
				MRCQuadLayer.subImage.imageRect = ToXrRect(GetViewportSize(MRCLayer->LeftEye, MRCLayer->Desc));
				MRCQuadLayer.subImage.swapchain = static_cast<FOpenXRSwapchain*>(MRCLayer->LeftEye.Swapchain.Get())->GetHandle();
				MRCQuadLayer.size = ToXrExtent2D(GetQuadSize(MRCLayer->LeftEye, MRCLayer->Desc) * LayerComponentScaler, WorldToMeters);
				MRCQuadLayerLeft_RenderThread = MRCQuadLayer;
			}
		}
		else
		{
			MRCLayer.Reset();
		}
	}
	else if(MRCLayer.IsValid() && (!MRCLayerDesc_RenderThread.TextureObj.IsValid() || !MRCLayerDesc_RenderThread.LeftTextureObj.IsValid()))
	{
		MRCLayer.Reset();
	}

	if (MRCLayer.IsValid())
	{
		static const int64 OPENXR_SWAPCHAIN_WAIT_TIMEOUT = 100000000ll;        // 100ms in nanoseconds.
		bool bInvertX = MRCDebugMode.ViewInHMD ? true : false;
		bool bInvertY = MRCDebugMode.ViewInHMD ? false : true;

		if (MRCLayer->RightEye.Swapchain.IsValid())
		{
			FRHITexture* const RightDstTexture = MRCLayer->RightEye.Swapchain->GetTexture2DArray() ? MRCLayer->RightEye.Swapchain->GetTexture2DArray() : MRCLayer->RightEye.Swapchain->GetTexture2D();

			//Right SwapChain
			const FXRSwapChainPtr& RightDstSwapChain = MRCLayer->RightEye.Swapchain;
			RHICmdList.EnqueueLambda([RightDstSwapChain](FRHICommandListImmediate& InRHICmdList)
				{
					RightDstSwapChain->IncrementSwapChainIndex_RHIThread();
					RightDstSwapChain->WaitCurrentImage_RHIThread(OPENXR_SWAPCHAIN_WAIT_TIMEOUT);
				});

			if (bIsMRCForegroundLayerDisabled_RebderThread)
			{
				FRHIRenderPassInfo RenderPassInfo(RightDstTexture, ERenderTargetActions::DontLoad_Store);
				TransitionRenderPassTargets(RHICmdList, RenderPassInfo);
				RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("ClearRT"));
				DrawClearQuad(RHICmdList, FLinearColor::Green);
				RHICmdList.EndRenderPass();
			}
			else
			{
				FRHITexture* RightSrcTexture = MRCLayer->Desc.TextureObj->GetResource()->TextureRHI;
				const FIntRect RightDstRect(FIntPoint(0, 0), MRCLayer->RightEye.SwapchainSize.IntPoint());
				CopyTexture_RenderThread(RHICmdList, RightDstTexture, RightSrcTexture, RightDstRect, FIntRect(), true, true, true, bInvertX, bInvertY);
			}

			RHICmdList.EnqueueLambda([RightDstSwapChain](FRHICommandListImmediate& InRHICmdList)
				{
					RightDstSwapChain->ReleaseCurrentImage_RHIThread(nullptr);
				});
		}

		if (MRCLayer->LeftEye.Swapchain.IsValid())
		{
			//Left SwapChain
			const FXRSwapChainPtr& LeftDstSwapChain = MRCLayer->LeftEye.Swapchain;
			RHICmdList.EnqueueLambda([LeftDstSwapChain](FRHICommandListImmediate& InRHICmdList)
				{
					LeftDstSwapChain->IncrementSwapChainIndex_RHIThread();
					LeftDstSwapChain->WaitCurrentImage_RHIThread(OPENXR_SWAPCHAIN_WAIT_TIMEOUT);
				});

			FRHITexture* LeftSrcTexture = MRCLayer->Desc.LeftTextureObj->GetResource()->TextureRHI;
			const FIntRect LeftDstRect(FIntPoint(0, 0), MRCLayer->LeftEye.SwapchainSize.IntPoint());
			FRHITexture* const LeftDstTexture = MRCLayer->LeftEye.Swapchain->GetTexture2DArray() ? MRCLayer->LeftEye.Swapchain->GetTexture2DArray() : MRCLayer->LeftEye.Swapchain->GetTexture2D();
			CopyTexture_RenderThread(RHICmdList, LeftDstTexture, LeftSrcTexture, LeftDstRect, FIntRect(), true, true, true/*BG*/, bInvertX, bInvertY);

			RHICmdList.EnqueueLambda([LeftDstSwapChain](FRHICommandListImmediate& InRHICmdList)
				{
					LeftDstSwapChain->ReleaseCurrentImage_RHIThread(nullptr);
				});
		}

		if (MRCDebugMode.ViewInHMD)
		{
			MRCQuadLayerLeft_RenderThread.layerFlags &= ~XR_COMPOSITION_LAYER_MRC_COMPOSITION_BIT_PICO;
			MRCQuadLayerRight_RenderThread.layerFlags &= ~XR_COMPOSITION_LAYER_MRC_COMPOSITION_BIT_PICO;
		}
		else
		{
			MRCQuadLayerLeft_RenderThread.layerFlags |= XR_COMPOSITION_LAYER_MRC_COMPOSITION_BIT_PICO;
			MRCQuadLayerRight_RenderThread.layerFlags |= XR_COMPOSITION_LAYER_MRC_COMPOSITION_BIT_PICO;
		}

		GraphBuilder.RHICmdList.EnqueueLambda([this, MRCLayerLeft_RenderThread = MRCQuadLayerLeft_RenderThread, MRCLayerRight_RenderThread = MRCQuadLayerRight_RenderThread](FRHICommandListImmediate&)
			{
				MRCQuadLayerLeft_RHIThread = MRCLayerLeft_RenderThread;
				MRCQuadLayerRight_RHIThread = MRCLayerRight_RenderThread;
			});
	}
}

void FHMDPICO::UpdateCompositionLayers_RHIThread(XrSession InSession, TArray<XrCompositionLayerBaseHeader*>& Headers)
{
	if (bSupportMRCExtension && MRCLayer.IsValid())
	{
		Headers.Add(reinterpret_cast<XrCompositionLayerBaseHeader*>(&MRCQuadLayerLeft_RHIThread));
		Headers.Add(reinterpret_cast<XrCompositionLayerBaseHeader*>(&MRCQuadLayerRight_RHIThread));
	}
}

bool FHMDPICO::GetSupportedDisplayRefreshRates(TArray<float>& Rates)
{
	Rates = SupportedDisplayRefreshRates;
	return bSupportDisplayRefreshRate;
}

bool FHMDPICO::GetCurrentDisplayRefreshRate(float& Rate, bool DoubleCheck)
{
	if (bSupportDisplayRefreshRate)
	{
		if (DoubleCheck && Session)
		{
			float DisplayRefreshRate = 0;
			if (XR_SUCCEEDED(xrGetDisplayRefreshRateFB(Session, &DisplayRefreshRate)))
			{
				if (DisplayRefreshRate == CurrentDisplayRefreshRate) // Check XR_TYPE_EVENT_DATA_DISPLAY_REFRESH_RATE_CHANGED_FB
				{
					Rate = DisplayRefreshRate;
					return true;
				}
			}
		}
		else
		{
			Rate = CurrentDisplayRefreshRate;
			return true;
		}
	}
	return false;
}

bool FHMDPICO::SetDisplayRefreshRate(float Rate)
{
	FReadScopeLock Lock(SessionHandleMutex);
	if (bSupportDisplayRefreshRate && Session)
	{
		if (SupportedDisplayRefreshRates.Contains(Rate))
		{
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Requesting DisplayRefreshRate:%f"), Rate);
			XR_ENSURE(xrRequestDisplayRefreshRateFB(Session, Rate));
			return true;
		}
		else
		{
			UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("Requested DisplayRefreshRate:%f is not supported by device!"), Rate);
		}
	}
	return false;
}

void FHMDPICO::EnableContentProtect(bool Enable)
{
	bool ContentProtectSupportedEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_FB_COMPOSITION_LAYER_SECURE_CONTENT_EXTENSION_NAME);
	bContentProtectEnabled = Enable && ContentProtectSupportedEXT;
	UE_LOG(LogPICOOpenXRHMD, Log, TEXT(":EnableContentProtect EXT Supported:%d, Enabled:%d"), ContentProtectSupportedEXT, bContentProtectEnabled);
}

bool FHMDPICO::SetPerformanceLevel(int domain, int level)
{
	if (Session && bSupportPerformanceSettingsEXT)
	{
		return XR_SUCCEEDED(xrPerfSettingsSetPerformanceLevelEXT(Session, XrPerfSettingsDomainEXT(domain), XrPerfSettingsLevelEXT(level)));
	}
	return false;
}

bool FHMDPICO::GetDevicePoseForTime(const EControllerHand Hand, bool UseDefaultTime, FTimespan Timespan, bool& OutTimeWasUsed, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityRadPerSec, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float InWorldToMetersScale)
{
	if (OpenXRHMD == nullptr)
	{
		return false;
	}

	int32 DeviceId = -1;
	if (Hand == EControllerHand::HMD)
	{
		DeviceId = IXRTrackingSystem::HMDDeviceId;
	}
	else
	{
		TArray<int32> Devices;
		if (OpenXRHMD->EnumerateTrackedDevices(Devices, EXRTrackedDeviceType::Controller))
		{
			if (Devices.IsValidIndex((int32)Hand))
			{
				DeviceId = Devices[(int32)Hand];
			}
		}
	}

	if (DeviceId == -1)
	{
		return false;
	}

	XrTime TargetTime = 0;
	if (UseDefaultTime)
	{
		TargetTime = CurrentDisplayTime;
	}
	else
	{
		TargetTime = ToXrTime(Timespan);
	}

	if (TargetTime == 0)
	{
		OutTimeWasUsed = false;
	}
	else
	{
		OutTimeWasUsed = true;
	}

	FQuat Orientation;
	bool Success = OpenXRHMD->GetPoseForTime(DeviceId, TargetTime, OutTimeWasUsed, Orientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityRadPerSec, OutbProvidedLinearAcceleration, OutLinearAcceleration, InWorldToMetersScale);
	OutOrientation = FRotator(Orientation);
	return Success;
}

EHMDWornState::Type FHMDPICO::GetHMDWornState(bool& ResultValid)
{
	ResultValid = IsSupportsUserPresence;
	return WornState;
}

bool FHMDPICO::GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees)
{
	if (OpenXRHMD)
	{
		OpenXRHMD->GetHMDDevice()->GetFieldOfView(OutHFOVInDegrees, OutVFOVInDegrees);
		return true;
	}
	return false;
}

bool FHMDPICO::GetInterpupillaryDistance(float& IPD)
{
	if (OpenXRHMD)
	{
		IPD = OpenXRHMD->GetInterpupillaryDistance() * OpenXRHMD->GetWorldToMetersScale();
		return true;
	}
	return false;
}

void FHMDPICO::SetBaseRotationAndBaseOffset(FRotator Rotation, FVector BaseOffset, EOrientPositionSelector::Type Options)
{
	if (OpenXRHMD)
	{
		if ((Options == EOrientPositionSelector::Orientation) || (Options == EOrientPositionSelector::OrientationAndPosition))
		{
			OpenXRHMD->SetBaseRotation(Rotation);
		}
		if ((Options == EOrientPositionSelector::Position) || (Options == EOrientPositionSelector::OrientationAndPosition))
		{
			OpenXRHMD->SetBasePosition(BaseOffset);
		}
	}
}

void FHMDPICO::GetBaseRotationAndBaseOffset(FRotator& OutRotation, FVector& OutBaseOffset)
{
	if (OpenXRHMD != nullptr)
	{
		OutRotation = OpenXRHMD->GetBaseRotation();
		OutBaseOffset = OpenXRHMD->GetBasePosition();
	}
	else
	{
		OutRotation = FRotator::ZeroRotator;
		OutBaseOffset = FVector::ZeroVector;
	}
}

FTimespan FHMDPICO::GetDisplayTime()
{
	return ToFTimespan(CurrentDisplayTime);
}

bool FHMDPICO::IsStationaryBoundaryMode(bool& bIsStationary)
{
	if (Session && bSupportedVirtualBoundary)
	{
		XrVirtualBoundaryModePICO Mode;
		if (XR_SUCCEEDED(xrGetVirtualBoundaryModePICO(Session, &Mode)))
		{
			bIsStationary = Mode == XR_VIRTUAL_BOUNDARY_MODE_STATIONARY_PICO;
			return true;
		}
	}
	return false;
}

bool FHMDPICO::GetVirtualBoundaryStatus(bool& bIsReady, bool& bIsEnable, bool& bIsVisible)
{
	if (Session && bSupportedVirtualBoundary)
	{
		XrVirtualBoundaryStatusPICO Status = { XR_TYPE_VIRTUAL_BOUNDARY_STATUS_PICO };
		if (XR_SUCCEEDED(xrGetVirtualBoundaryStatusPICO(Session, &Status)))
		{
			bIsReady = Status.isReady == XR_TRUE;
			bIsEnable = Status.isEnabled == XR_TRUE;
			bIsVisible = Status.isVisible == XR_TRUE;
			return true;
		}
	}
	return false;
}

bool FHMDPICO::SetVirtualBoundaryEnable(bool bEnable)
{
	if (Session && bSupportedVirtualBoundary)
	{
		if (XR_SUCCEEDED(xrSetVirtualBoundaryEnablePICO(Session, bEnable)))
		{
			return true;
		}
	}
	return false;
}

bool FHMDPICO::SetVirtualBoundaryVisible(bool bVisible)
{
	if (Session && bSupportedVirtualBoundary)
	{
		if ( XR_SUCCEEDED(xrSetVirtualBoundaryVisiblePICO(Session, bVisible)))
		{
			return true;
		}
	}
	return false;
}

bool FHMDPICO::SetVirtualBoundarySeeThroughVisible(bool bVisible)
{
	if (Session && bSupportedVirtualBoundary)
	{
		if (XR_SUCCEEDED(xrSetVirtualBoundarySeeThroughVisiblePICO(Session, bVisible)))
		{
			return true;
		}
	}
	return false;
}

bool FHMDPICO::BoundaryintersectPointOrNode(bool bPoint, EControllerHand Node, FVector Point, EBoundaryTypePICO BoundaryType, bool& Valid, bool& IsTriggering, float& ClosestDistance, FVector& ClosestPoint, FVector& ClosestPointNormal, float InWorldToMetersScale)
{
	if (Session && bSupportedVirtualBoundary)
	{
		XrVirtualBoundaryInfoPICO VirtualBoundaryInfo = { XR_TYPE_VIRTUAL_BOUNDARY_INFO_PICO };
		VirtualBoundaryInfo.baseSpace = CurrentBaseSpace;
		VirtualBoundaryInfo.edgeType = (XrVirtualBoundaryEdgeTypePICO)BoundaryType;

		XrVirtualBoundaryTriggerPointPICO BoundaryTestPoint = { XR_TYPE_VIRTUAL_BOUNDARY_TRIGGER_POINT_PICO };
		BoundaryTestPoint.point = ToXrVector(Point, InWorldToMetersScale);
		BoundaryTestPoint.boundaryInfo = &VirtualBoundaryInfo;

		XrVirtualBoundaryTriggerNodePICO VirtualBoundaryTriggerNode = { XR_TYPE_VIRTUAL_BOUNDARY_TRIGGER_NODE_PICO };
		VirtualBoundaryTriggerNode.boundaryInfo = &VirtualBoundaryInfo;

		switch (Node)
		{
		case EControllerHand::Left:
			VirtualBoundaryTriggerNode.node = XR_VIRTUAL_BOUNDARY_TRIGGER_NODE_TYPE_LEFT_HAND_PICO;
			break;
		case EControllerHand::Right:
			VirtualBoundaryTriggerNode.node = XR_VIRTUAL_BOUNDARY_TRIGGER_NODE_TYPE_RIGHT_HAND_PICO;
			break;
		case EControllerHand::HMD:
			VirtualBoundaryTriggerNode.node = XR_VIRTUAL_BOUNDARY_TRIGGER_NODE_TYPE_HEAD_PICO;
			break;
		default:
			break;
		}

		const XrVirtualBoundaryTriggerBaseHeaderPICO* BaseHeader = nullptr;
		if (bPoint)
		{	
			BaseHeader = reinterpret_cast<const XrVirtualBoundaryTriggerBaseHeaderPICO*>(&BoundaryTestPoint);
		}
		else
		{

			BaseHeader = reinterpret_cast<const XrVirtualBoundaryTriggerBaseHeaderPICO*>(&VirtualBoundaryTriggerNode);
		}

		XrVirtualBoundaryTriggerPICO VirtualBoundaryTrigger = { XR_TYPE_VIRTUAL_BOUNDARY_TRIGGER_PICO };
		if (XR_SUCCEEDED(xrGetVirtualBoundaryTriggerPICO(Session, BaseHeader, &VirtualBoundaryTrigger)))
		{
			Valid = VirtualBoundaryTrigger.isValid == XR_TRUE;
			ClosestDistance = VirtualBoundaryTrigger.closestDistance * InWorldToMetersScale;
			IsTriggering = VirtualBoundaryTrigger.isTriggering == XR_TRUE;
			ClosestPoint = ToFVector(VirtualBoundaryTrigger.closestPoint, InWorldToMetersScale);
			ClosestPointNormal = ToFVector(VirtualBoundaryTrigger.closestPointNormal);
			return true;
		}
	}
	return false;
}

bool FHMDPICO::GetBoundaryGeometry(EBoundaryTypePICO BoundaryType, bool& Valid, TArray<FVector>& Points, float InWorldToMetersScale)
{
	if (Session && bSupportedVirtualBoundary)
	{
		XrVirtualBoundaryInfoPICO VirtualBoundaryInfo = { XR_TYPE_VIRTUAL_BOUNDARY_INFO_PICO };
		VirtualBoundaryInfo.baseSpace = CurrentBaseSpace;
		VirtualBoundaryInfo.edgeType = (XrVirtualBoundaryEdgeTypePICO)BoundaryType;

		uint32 count = 0;
		xrGetVirtualBoundaryGeometryPICO(Session, &VirtualBoundaryInfo, 0, &count, nullptr);
		if (count > 0)
		{
			TArray<XrVector3f> xrPoints;
			Points.SetNum(count);
			xrPoints.SetNum(count);
			if (XR_SUCCEEDED(xrGetVirtualBoundaryGeometryPICO(Session, &VirtualBoundaryInfo, count, &count, xrPoints.GetData())))
			{
				for (int i = 0; i < xrPoints.Num(); i++)
				{
					Points[i] = ToFVector(xrPoints[i], InWorldToMetersScale);
				}
				return true;
			}
		}
	}
	return false;
}

void FHMDPICO::CopyTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture* DstTexture, FRHITexture* SrcTexture, FIntRect DstRect, FIntRect SrcRect, bool bAlphaPremultiply, bool bNoAlpha, bool bClearGreen, bool bInvertX, bool bInvertY, bool bInvertAlpha) const
{
	ERenderTargetActions RTAction = ERenderTargetActions::Clear_Store;
	ERHIAccess FinalDstAccess = ERHIAccess::SRVMask;

	check(IsInRenderingThread());

	const uint32 ViewportWidth = DstRect.Width();
	const uint32 ViewportHeight = DstRect.Height();
	const FIntPoint TargetSize(ViewportWidth, ViewportHeight);

	const float SrcTextureWidth = SrcTexture->GetSizeX();
	const float SrcTextureHeight = SrcTexture->GetSizeY();
	float U = 0.f, V = 0.f, USize = 1.f, VSize = 1.f;
	if (SrcRect.IsEmpty())
	{
		SrcRect.Min.X = 0;
		SrcRect.Min.Y = 0;
		SrcRect.Max.X = SrcTextureWidth;
		SrcRect.Max.Y = SrcTextureHeight;
	}
	else
	{
		U = SrcRect.Min.X / SrcTextureWidth;
		V = SrcRect.Min.Y / SrcTextureHeight;
		USize = SrcRect.Width() / SrcTextureWidth;
		VSize = SrcRect.Height() / SrcTextureHeight;
	}

	if (bInvertX)
	{
		U = 1.0f - U;
		USize = -USize;
	}

	if (bInvertY)
	{
		V = 1.0f - V;
		VSize = -VSize;
	}

	RHICmdList.Transition(FRHITransitionInfo(DstTexture, ERHIAccess::Unknown, ERHIAccess::RTV));

	FRHITexture* ColorRT = DstTexture->GetTexture2DArray() ? DstTexture->GetTexture2DArray() : DstTexture->GetTexture2D();
	FRHIRenderPassInfo RenderPassInfo(ColorRT, RTAction);
	RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("OpenXRHMD_CopyTexture"));
	{
		if (bClearGreen || bNoAlpha)
		{
			const FIntRect ClearRect(0, 0, DstTexture->GetSizeX(), DstTexture->GetSizeY());
			RHICmdList.SetViewport(ClearRect.Min.X, ClearRect.Min.Y, 0, ClearRect.Max.X, ClearRect.Max.Y, 1.0f);

			if (bClearGreen)
			{
				DrawClearQuad(RHICmdList, FLinearColor::Green);
			}
			else
			{
				// For opaque texture copies, we want to make sure alpha is initialized to 1.0f
				DrawClearQuadAlpha(RHICmdList, 1.0f);
			}
		}

		RHICmdList.SetViewport(DstRect.Min.X, DstRect.Min.Y, 0, DstRect.Max.X, DstRect.Max.Y, 1.0f);

		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

		if (bClearGreen)
		{
			GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_InverseSourceAlpha, BF_SourceAlpha, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI();
		}
		else if (bInvertAlpha)
		{
			GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_Zero, BO_Add, BF_Zero, BF_InverseSourceAlpha >::GetRHI();
		}
		else if (bAlphaPremultiply)
		{
			if (bNoAlpha)
			{
				GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_One, BF_Zero, BO_Add, BF_One, BF_Zero>::GetRHI();
			}
			else
			{
				GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_Zero, BO_Add, BF_One, BF_Zero>::GetRHI();
			}
		}
		else
		{
			if (bNoAlpha)
			{
				GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB>::GetRHI();
			}
			else
			{
				GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
			}
		}

		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;

		FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(OpenXRHMD->GetConfiguredShaderPlatform());

		TShaderMapRef<FScreenVS> VertexShader(ShaderMap);

		TShaderRef<FGlobalShader> PixelShader;
		TShaderRef<FDisplayMappingPixelShader> DisplayMappingPS;
		TShaderRef<FScreenPS> ScreenPS;

		bool bNeedsDisplayMapping = false;
		bool bIsInputLinear = false;
		EDisplayOutputFormat TVDisplayOutputFormat = EDisplayOutputFormat::SDR_sRGB;
		EDisplayColorGamut HMDColorGamut = EDisplayColorGamut::sRGB_D65;
		EDisplayColorGamut TVColorGamut = EDisplayColorGamut::sRGB_D65;

		FOpenXRRenderBridge* RenderBridge = static_cast<FOpenXRRenderBridge*>(OpenXRHMD->GetActiveRenderBridge_GameThread(OpenXRHMD->ShouldUseSeparateRenderTarget()));
		if (FinalDstAccess == ERHIAccess::Present && RenderBridge)
		{
			EDisplayOutputFormat HMDDisplayFormat;
			bool bHMDSupportHDR;
			if (RenderBridge->HDRGetMetaDataForStereo(HMDDisplayFormat, HMDColorGamut, bHMDSupportHDR))
			{
				bool bTVSupportHDR;
				HDRGetMetaData(TVDisplayOutputFormat, TVColorGamut, bTVSupportHDR, FVector2D(0, 0), FVector2D(0, 0), nullptr);
				if (TVDisplayOutputFormat != HMDDisplayFormat || HMDColorGamut != TVColorGamut || bTVSupportHDR != bHMDSupportHDR)
				{
					// shader assumes G 2.2 for input / ST2084/sRGB for output right now
					ensure(HMDDisplayFormat == EDisplayOutputFormat::SDR_ExplicitGammaMapping);
					ensure(TVDisplayOutputFormat == EDisplayOutputFormat::SDR_sRGB || TVDisplayOutputFormat == EDisplayOutputFormat::HDR_ACES_1000nit_ST2084 || TVDisplayOutputFormat == EDisplayOutputFormat::HDR_ACES_2000nit_ST2084);
					bNeedsDisplayMapping = true;
				}
			}

			// In Android Vulkan preview, when the sRGB swapchain texture is sampled, the data is converted to linear and written to the RGBA10A2_UNORM texture.
			// However, D3D interprets integer-valued display formats as containing sRGB data, so we need to convert the linear data back to sRGB.
			if (!IsMobileHDR() && IsMobilePlatform(OpenXRHMD->GetConfiguredShaderPlatform()) && IsSimulatedPlatform(OpenXRHMD->GetConfiguredShaderPlatform()))
			{
				bNeedsDisplayMapping = true;
				TVDisplayOutputFormat = EDisplayOutputFormat::SDR_sRGB;
				bIsInputLinear = true;
			}
		}

		bNeedsDisplayMapping &= IsFeatureLevelSupported(OpenXRHMD->GetConfiguredShaderPlatform(), ERHIFeatureLevel::ES3_1);

		bool bIsArraySource = SrcTexture->GetDesc().IsTextureArray();

		if (bNeedsDisplayMapping)
		{
			FDisplayMappingPixelShader::FPermutationDomain PermutationVector;
			PermutationVector.Set<FDisplayMappingPixelShader::FArraySource>(bIsArraySource);
			PermutationVector.Set<FDisplayMappingPixelShader::FLinearInput>(bIsInputLinear);

			TShaderMapRef<FDisplayMappingPixelShader> DisplayMappingPSRef(ShaderMap, PermutationVector);

			DisplayMappingPS = DisplayMappingPSRef;
			PixelShader = DisplayMappingPSRef;
		}
		else
		{
			if (LIKELY(!bIsArraySource))
			{
				TShaderMapRef<FScreenPS> ScreenPSRef(ShaderMap);
				ScreenPS = ScreenPSRef;
				PixelShader = ScreenPSRef;
			}
			else
			{
				TShaderMapRef<FScreenFromSlice0PS> ScreenPSRef(ShaderMap);
				ScreenPS = ScreenPSRef;
				PixelShader = ScreenPSRef;
			}
		}

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

		RHICmdList.Transition(FRHITransitionInfo(SrcTexture, ERHIAccess::Unknown, ERHIAccess::SRVMask));

		const bool bSameSize = DstRect.Size() == SrcRect.Size();
		if (ScreenPS.IsValid())
		{
			FRHISamplerState* PixelSampler = bSameSize ? TStaticSamplerState<SF_Point>::GetRHI() : TStaticSamplerState<SF_Bilinear>::GetRHI();
			SetShaderParametersLegacyPS(RHICmdList, ScreenPS, PixelSampler, SrcTexture);
		}
		else if (DisplayMappingPS.IsValid())
		{
			SetShaderParametersLegacyPS(RHICmdList, DisplayMappingPS, TVDisplayOutputFormat, TVColorGamut, HMDColorGamut, SrcTexture, bSameSize);
		}

		FModuleManager::GetModulePtr<IRendererModule>("Renderer")->DrawRectangle(
			RHICmdList,
			0, 0,
			ViewportWidth, ViewportHeight,
			U, V,
			USize, VSize,
			TargetSize,
			FIntPoint(1, 1),
			VertexShader,
			EDRF_Default);

	}
	RHICmdList.EndRenderPass();

	RHICmdList.Transition(FRHITransitionInfo(DstTexture, ERHIAccess::RTV, FinalDstAccess));
}

void FHMDPICO::CreateMRCLayer(class UTexture* BackgroundRTTexture, class UTexture* ForegroundRTTexture)
{
	ENQUEUE_RENDER_COMMAND(FCreateMRCLayer)(
		[this, BackgroundRTTexture = BackgroundRTTexture, ForegroundRTTexture = ForegroundRTTexture](FRHICommandListImmediate& RHICmdList)
		{
			if (BackgroundRTTexture && BackgroundRTTexture->GetResource() && ForegroundRTTexture && ForegroundRTTexture->GetResource())
			{
				MRCLayerDesc_RenderThread.PositionType = IStereoLayers::ELayerType::FaceLocked;
				MRCLayerDesc_RenderThread.Transform.SetLocation(FVector(100, 0, 0));
				MRCLayerDesc_RenderThread.TextureObj = ForegroundRTTexture;
				MRCLayerDesc_RenderThread.LeftTextureObj = BackgroundRTTexture;
				MRCLayerDesc_RenderThread.Flags = IStereoLayers::ELayerFlags::LAYER_FLAG_TEX_CONTINUOUS_UPDATE | IStereoLayers::ELayerFlags::LAYER_FLAG_TEX_NO_ALPHA_CHANNEL;
				MRCLayerDesc_RenderThread.QuadSize = FVector2D(100, 100);
			}
		});
}

void FHMDPICO::DestroyMRCLayer()
{
	ENQUEUE_RENDER_COMMAND(FDestroyMRCLayer)(
		[this](FRHICommandListImmediate& RHICmdList)
		{
			MRCLayerDesc_RenderThread.TextureObj.Reset();
			MRCLayerDesc_RenderThread.LeftTextureObj.Reset();
			MRCLayer.Reset();
		});
	MRCSceneCapture2DPICO = nullptr;
}

bool FHMDPICO::GetExternalCameraInfo(int32& width, int32& height, float& fov)
{
	if (MRCDebugMode.UseCustomCameraInfo)
	{
		width = MRCDebugMode.Width;
		height = MRCDebugMode.Height;
		fov = MRCDebugMode.Fov;
		return true;
	}

	if (Session && bSupportMRCExtension && xrGetExternalCameraInfoPICO)
	{
		XrExternalCameraParameterPICO ExternalCameraInfo = { XR_TYPE_EXTERNAL_CAMERA_PARAMETER_PICO };
		if (XR_SUCCEEDED(xrGetExternalCameraInfoPICO(Session, &ExternalCameraInfo)))
		{
			width = ExternalCameraInfo.width;
			height = ExternalCameraInfo.height;
			fov = ExternalCameraInfo.fov;
			return true;
		}
	}
	return false;
}

bool FHMDPICO::GetExternalCameraPose(FTransform& Pose)
{
	if (MRCDebugMode.UseCustomTransform)
	{
		Pose = MRCDebugMode.Pose;
		return true;
	}

	if (bSupportMRCExtension && Session && CurrentBaseSpace && MRCSpace && OpenXRHMD)
	{
		XrSpaceLocation NewLocation = { XR_TYPE_SPACE_LOCATION };
		const XrResult Result = xrLocateSpace(MRCSpace, CurrentBaseSpace, CurrentDisplayTime, &NewLocation);
		if (Result != XR_SUCCESS)
		{
			return false;
		}

		if (!(NewLocation.locationFlags & (XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)))
		{
			return false;
		}

		const FQuat Orientation = ToFQuat(NewLocation.pose.orientation);
		const FVector Position = ToFVector(NewLocation.pose.position, OpenXRHMD->GetWorldToMetersScale());

		FTransform TrackingToWorld = OpenXRHMD->GetTrackingToWorldTransform();
		Pose = FTransform(Orientation, Position) * TrackingToWorld;
		return true;
	}
	return false;
}

void FHMDPICO::EnableMRCDebugMode(class UWorld* WorldContext, bool Enable, bool ViewInHMD, bool UseCustomTransform, const FTransform& Pose, bool UseCustomCameraInfo, int Width, int Height, float Fov)
{
	bSupportMRCExtension = Enable;
	bIsMRCRunning |= Enable;
	bIsMRCRunningStored |= Enable;
	if (Enable)
	{
		MRCDebugMode.EnableExtension = Enable;
		MRCDebugMode.ViewInHMD = ViewInHMD;
		MRCDebugMode.UseCustomTransform = UseCustomTransform;
		MRCDebugMode.Pose = Pose;
		MRCDebugMode.UseCustomCameraInfo = UseCustomCameraInfo;
		MRCDebugMode.Width = Width;
		MRCDebugMode.Height = Height;
		MRCDebugMode.Fov = Fov;
	}
	else
	{
		MRCDebugMode = {};
	}
}

void FHMDPICO::DisableMRCForegroundLayer(UObject* WorldContextObject, bool Disable)
{
	bIsMRCForegroundLayerDisabled = Disable;
	if (MRCSceneCapture2DPICO)
	{
		MRCSceneCapture2DPICO->DisableForegroundLayer = bIsMRCForegroundLayerDisabled;
	}
}

void FHMDPICO::PauseMRC(bool Pause)
{
	if (Pause)
	{
		if (bIsMRCRunning)
		{
			bIsMRCRunning = false;
		}
	}
	else
	{
		if (bIsMRCRunningStored && !bIsMRCRunning)
		{
			bIsMRCRunning = true;
		}
	}
}

bool FHMDPICO::GetAdaptivePixelDensity(EAdaptiveResolutionSettingPICO Setting, float& PixelDensity)
{
	if (OpenXRHMD && bSupportAdaptiveResolution && Session)
	{
		uint32 SizeX, SizeY;
		GetCurrentRenderTargetSize(SizeX, SizeY);
		FIntPoint ViewportSize =
		{
			FMath::CeilToInt(SizeX * CurrentDynamicPixelDensity),
			FMath::CeilToInt(SizeY * CurrentDynamicPixelDensity)
		};
		QuantizeSceneBufferSize(ViewportSize, ViewportSize);

		XrExtent2Di Extent2D = { ViewportSize.X, ViewportSize.Y };
		if (XR_SUCCEEDED(xrUpdateAdaptiveResolutionPICO(Session, (XrAdaptiveResolutionSettingPICO)Setting, Extent2D, &Extent2D)))
		{
			PixelDensity = (float)Extent2D.width / (float)SizeX;
			return true;
		}
	}
	return false;
}

FIntPoint FHMDPICO::GetDefaultRenderTargetSize()
{
	if (OpenXRHMD)
	{
		return OpenXRHMD->GetIdealRenderTargetSize();
	}
	return FIntPoint();
}

void FHMDPICO::GetCurrentRenderTargetSize(uint32& InOutSizeX, uint32& InOutSizeY)
{
	InOutSizeX = InOutSizeY = 0;
	if (OpenXRHMD && GEngine)
	{
		check(GEngine->GameViewport->Viewport);
		OpenXRHMD->CalculateRenderTargetSize(*GEngine->GameViewport->Viewport, InOutSizeX, InOutSizeY);
	}
}

bool FHMDPICO::SetProjectionLayerColorMatrix3x3f(bool Enable, FVector3f ColumnA, FVector3f ColumnB, FVector3f ColumnC)
{
	if (bSupportColorMatrixExtension)
	{
		bUseColorMatrixExtension = Enable;
		if (bUseColorMatrixExtension)
		{
			ColorMatrix3x3f[0] = ColumnA.X;
			ColorMatrix3x3f[1] = ColumnA.Y;
			ColorMatrix3x3f[2] = ColumnA.Z;
			ColorMatrix3x3f[3] = ColumnB.X;
			ColorMatrix3x3f[4] = ColumnB.Y;
			ColorMatrix3x3f[5] = ColumnB.Z;
			ColorMatrix3x3f[6] = ColumnC.X;
			ColorMatrix3x3f[7] = ColumnC.Y;
			ColorMatrix3x3f[8] = ColumnC.Z;
		}
		return true;
	}
	return false;
}

bool FHMDPICO::GetViewportSize(FIntPoint& ViewportSize)
{
	if (OpenXRHMD && bSupportAdaptiveResolution && Session)
	{
		uint32 SizeX, SizeY;
		GetCurrentRenderTargetSize(SizeX, SizeY);
		ViewportSize =
		{
			FMath::CeilToInt(SizeX * CurrentDynamicPixelDensity),
			FMath::CeilToInt(SizeY * CurrentDynamicPixelDensity)
		};
		QuantizeSceneBufferSize(ViewportSize, ViewportSize);
		return true;
	}
	return false;
}

bool FHMDPICO::GetBatteryStateDisplay(const EControllerHand Hand, bool& OutValid, float& OutBatteryLevel, bool& OutCharging, bool& OutPluggedIn, bool& OutNoBattery)
{
	OutValid = false;
	OutBatteryLevel = 0.0f;
	OutCharging = false;
	OutPluggedIn = false;
	OutNoBattery = false;

	if (!Session || !bSupportBatteryStateDisplayEXT)
	{
		return false;
	}

	XrPath TopLevelUserPath = XR_NULL_PATH;
	if (Hand == EControllerHand::Left)
	{
		if (!bHasLeftHandUserPath)
		{
			return false;
		}
		TopLevelUserPath = LeftHandUserPath;
	}
	else if (Hand == EControllerHand::Right)
	{
		if (!bHasRightHandUserPath)
		{
			return false;
		}
		TopLevelUserPath = RightHandUserPath;
	}
	else if (Hand == EControllerHand::HMD)
	{
		if (!bHasHeadUserPath)
		{
			return false;
		}
		TopLevelUserPath = HeadUserPath;
	}
	else
	{
		return false;
	}

	if (TopLevelUserPath == XR_NULL_PATH)
	{
		return false;
	}

	XrBatteryStateDisplayEXT BatteryState{ XR_TYPE_BATTERY_STATE_DISPLAY_EXT };
	XrInteractionProfileState InteractionProfileState{ XR_TYPE_INTERACTION_PROFILE_STATE };
	InteractionProfileState.next = &BatteryState;

	XrResult Result = xrGetCurrentInteractionProfile(Session, TopLevelUserPath, &InteractionProfileState);
	if (XR_FAILED(Result))
	{
		return false;
	}

	OutValid = (BatteryState.stateFlags & XR_BATTERY_STATE_DISPLAY_STATE_VALID_BIT_EXT) != 0;
	OutCharging = (BatteryState.stateFlags & XR_BATTERY_STATE_DISPLAY_STATE_CHARGING_BIT_EXT) != 0;
	OutPluggedIn = (BatteryState.stateFlags & XR_BATTERY_STATE_DISPLAY_STATE_PLUGGED_IN_BIT_EXT) != 0;
	OutNoBattery = (BatteryState.stateFlags & XR_BATTERY_STATE_DISPLAY_STATE_NO_BATTERY_BIT_EXT) != 0;

	if (OutValid)
	{
		OutBatteryLevel = BatteryState.batteryLevel;
	}

	return true;
}

/*************************** Camera Image Extension Implementation Begin ***************************/

bool FHMDPICO::IsCameraImageExtensionSupported() const
{
	return bSupportedCameraImage;
}

bool FHMDPICO::EnumerateAvailableCameras(TArray<uint64>& OutCameraIDs)
{
	if (!bSupportedCameraImage || !xrEnumerateAvailableCamerasPICO)
	{
		UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("Camera Image extension not supported"));
		return false;
	}

	XrAvailableCamerasEnumerateInfoPICO EnumerateInfo{ XR_TYPE_AVAILABLE_CAMERAS_ENUMERATE_INFO_PICO };
	EnumerateInfo.properties = nullptr;
	EnumerateInfo.capabilities = nullptr;

	uint32 Count = 0;
	XrResult Result = xrEnumerateAvailableCamerasPICO(Instance, &EnumerateInfo, 0, &Count, nullptr);
	if (XR_FAILED(Result) || Count == 0)
	{
		UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("Failed to enumerate cameras or no cameras found"));
		return false;
	}

	TArray<XrCameraIdPICO> CameraIds;
	CameraIds.SetNum(Count);
	Result = xrEnumerateAvailableCamerasPICO(Instance, &EnumerateInfo, Count, &Count, CameraIds.GetData());
	
	if (XR_SUCCEEDED(Result))
	{
		OutCameraIDs.SetNum(Count);
		for (uint32 i = 0; i < Count; i++)
		{
			OutCameraIDs[i] = static_cast<uint64>(CameraIds[i]);
		}
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Enumerated %d cameras"), Count);
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to enumerate cameras: %d"), Result);
	return false;
}

bool FHMDPICO::CreateCameraDeviceAsync(uint64 CameraID, int64 Handle, uint64& OutFuture)
{
	if (!bSupportedCameraImage || !xrCreateCameraDeviceAsyncPICO)
	{
		return false;
	}

	XrCameraDeviceCreateInfoPICO CreateInfo{ XR_TYPE_CAMERA_DEVICE_CREATE_INFO_PICO };
	CreateInfo.cameraId = static_cast<XrCameraIdPICO>(CameraID);

	XrFutureEXT Future;
	XrResult Result = xrCreateCameraDeviceAsyncPICO(Instance, &CreateInfo, &Future);
	
	if (XR_SUCCEEDED(Result))
	{
		OutFuture = *reinterpret_cast<uint64*>(&Future);
		PendingDeviceCreation.Add(Future, Handle);
		
		FCameraDeviceInfo& DeviceInfo = CameraDevices.Add(Handle);
		DeviceInfo.CameraID = static_cast<XrCameraIdPICO>(CameraID);
		DeviceInfo.bValid = false;
		
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera device creation started: Handle=%lld, CameraID=%llu"), Handle, CameraID);
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to create camera device async: %d"), Result);
	return false;
}

bool FHMDPICO::CompleteCameraDeviceCreation(int64 Handle, uint64 Future)
{
	if (!bSupportedCameraImage || !xrCreateCameraDeviceCompletePICO)
	{
		return false;
	}

	XrFutureEXT XrFuture;
	*reinterpret_cast<uint64*>(&XrFuture) = Future;

	// If XR_EXT_future is supported, poll the future state first
	if (bSupportedFutureEXT && xrPollFutureEXT)
	{
		XrFuturePollInfoEXT PollInfo{ XR_TYPE_FUTURE_POLL_INFO_EXT };
		PollInfo.future = XrFuture;

		XrFuturePollResultEXT PollResult{ XR_TYPE_FUTURE_POLL_RESULT_EXT };
		XrResult PollStatus = xrPollFutureEXT(Instance, &PollInfo, &PollResult);

		if (XR_FAILED(PollStatus))
		{
			UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to poll future state: %d"), PollStatus);
			return false;
		}

		// Check future state
		switch (PollResult.state)
		{
		case XR_FUTURE_STATE_PENDING_EXT:
			// Still pending, return false but not an error
			UE_LOG(LogPICOOpenXRHMD, VeryVerbose, TEXT("Camera device creation still pending: Handle=%lld"), Handle);
			return false;

		case XR_FUTURE_STATE_READY_EXT:
			// Ready to complete
			UE_LOG(LogPICOOpenXRHMD, Verbose, TEXT("Camera device creation ready: Handle=%lld"), Handle);
			break;

		default:
			UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("Unexpected future state: %d for Handle=%lld"), PollResult.state, Handle);
			return false;
		}
	}

	// Complete the operation
	XrCreateCameraDeviceCompletionPICO Completion{ XR_TYPE_CREATE_CAMERA_DEVICE_COMPLETION_PICO };
	XrResult Result = xrCreateCameraDeviceCompletePICO(Instance, XrFuture, &Completion);
	
	if (XR_SUCCEEDED(Result) && XR_SUCCEEDED(Completion.futureResult))
	{
		FCameraDeviceInfo* DeviceInfo = CameraDevices.Find(Handle);
		if (DeviceInfo)
		{
			DeviceInfo->Device = Completion.device;
			DeviceInfo->bValid = true;
			PendingDeviceCreation.Remove(XrFuture);
			
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera device created successfully: Handle=%lld"), Handle);
			return true;
		}
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to complete camera device creation: %d, FutureResult=%d"), Result, Completion.futureResult);
	return false;
}

bool FHMDPICO::DestroyCameraDevice(int64 Handle)
{
	if (!bSupportedCameraImage || !xrDestroyCameraDevicePICO)
	{
		return false;
	}

	FCameraDeviceInfo* DeviceInfo = CameraDevices.Find(Handle);
	if (!DeviceInfo || !DeviceInfo->bValid)
	{
		return false;
	}

	// Destroy all capture sessions using this device
	TArray<int64> SessionsToDestroy;
	for (auto& Pair : CaptureSessions)
	{
		if (Pair.Value.Device == DeviceInfo->Device)
		{
			SessionsToDestroy.Add(Pair.Key);
		}
	}
	
	for (int64 SessionHandle : SessionsToDestroy)
	{
		DestroyCaptureSession(SessionHandle);
	}

	XrResult Result = xrDestroyCameraDevicePICO(DeviceInfo->Device);
	if (XR_SUCCEEDED(Result))
	{
		CameraDevices.Remove(Handle);
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera device destroyed: Handle=%lld"), Handle);
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to destroy camera device: %d"), Result);
	return false;
}

bool FHMDPICO::CreateCaptureSessionAsync(int64 DeviceHandle, const TArray<uint8>& ConfigsData, int64 SessionHandle, uint64& OutFuture)
{
	if (!bSupportedCameraImage || !xrCreateCameraCaptureSessionAsyncPICO)
	{
		return false;
	}

	FCameraDeviceInfo* DeviceInfo = CameraDevices.Find(DeviceHandle);
	if (!DeviceInfo || !DeviceInfo->bValid)
	{
		return false;
	}

	// Parse ConfigsData and build OpenXR capability structures
	TArray<XrCameraCapabilityBaseHeaderPICO*> ConfigPointers;
	
	// Allocate structures for each capability type
	XrCameraCapabilityImageResolutionPICO ResolutionConfig = { XR_TYPE_CAMERA_CAPABILITY_IMAGE_RESOLUTION_PICO };
	XrCameraCapabilityImageFormatPICO FormatConfig = { XR_TYPE_CAMERA_CAPABILITY_IMAGE_FORMAT_PICO };
	XrCameraCapabilityDataTransferTypePICO TransferConfig = { XR_TYPE_CAMERA_CAPABILITY_DATA_TRANSFER_TYPE_PICO };
	XrCameraCapabilityCameraModelPICO ModelConfig = { XR_TYPE_CAMERA_CAPABILITY_CAMERA_MODEL_PICO };
	XrCameraCapabilityImageFpsPICO FPSConfig = { XR_TYPE_CAMERA_CAPABILITY_IMAGE_FPS_PICO };

	if (ConfigsData.Num() > 0)
	{
		FMemoryReader Reader(ConfigsData);
		
		int32 ConfigCount;
		Reader << ConfigCount;
		
		for (int32 i = 0; i < ConfigCount; i++)
		{
			int32 ConfigType;
			Reader << ConfigType;
			
			switch (ConfigType)
			{
			case XR_CAMERA_CAPABILITY_TYPE_IMAGE_RESOLUTION_PICO: // 1
			{
				int32 Width, Height;
				Reader << Width << Height;
				ResolutionConfig.resolution.width = Width;
				ResolutionConfig.resolution.height = Height;
				ConfigPointers.Add(reinterpret_cast<XrCameraCapabilityBaseHeaderPICO*>(&ResolutionConfig));
				UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Config Resolution: %dx%d"), Width, Height);
				break;
			}
			case XR_CAMERA_CAPABILITY_TYPE_IMAGE_FORMAT_PICO: // 2
			{
				int32 Format;
				Reader << Format;
				FormatConfig.format = static_cast<XrCameraImageFormatPICO>(Format);
				ConfigPointers.Add(reinterpret_cast<XrCameraCapabilityBaseHeaderPICO*>(&FormatConfig));
				UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Config Format: %d"), Format);
				break;
			}
			case XR_CAMERA_CAPABILITY_TYPE_DATA_TRANSFER_TYPE_PICO: // 3
			{
				int32 Transfer;
				Reader << Transfer;
				TransferConfig.transferType = static_cast<XrCameraDataTransferTypePICO>(Transfer);
				ConfigPointers.Add(reinterpret_cast<XrCameraCapabilityBaseHeaderPICO*>(&TransferConfig));
				UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Config TransferType: %d"), Transfer);
				break;
			}
			case XR_CAMERA_CAPABILITY_TYPE_CAMERA_MODEL_PICO: // 4
			{
				int32 Model;
				Reader << Model;
				ModelConfig.model = static_cast<XrCameraModelPICO>(Model);
				ConfigPointers.Add(reinterpret_cast<XrCameraCapabilityBaseHeaderPICO*>(&ModelConfig));
				UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Config Model: %d"), Model);
				break;
			}
			case XR_CAMERA_CAPABILITY_TYPE_IMAGE_FPS_PICO: // 5
			{
				int32 FPS;
				Reader << FPS;
				FPSConfig.fps = static_cast<XrCameraImageFpsPICO>(FPS);
				ConfigPointers.Add(reinterpret_cast<XrCameraCapabilityBaseHeaderPICO*>(&FPSConfig));
				UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Config FPS: %d"), FPS);
				break;
			}
			default:
				UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("Unknown config type: %d"), ConfigType);
				break;
			}
		}
	}

	// Create session with parsed configs
	XrCameraCaptureSessionCreateInfoPICO CreateInfo{ XR_TYPE_CAMERA_CAPTURE_SESSION_CREATE_INFO_PICO };
	CreateInfo.camera = DeviceInfo->Device;
	CreateInfo.configCount = ConfigPointers.Num();
	CreateInfo.configs = ConfigPointers.Num() > 0 ? const_cast<const XrCameraCapabilityBaseHeaderPICO**>(ConfigPointers.GetData()) : nullptr;

	XrFutureEXT Future;
	XrResult Result = xrCreateCameraCaptureSessionAsyncPICO(Session, &CreateInfo, &Future);
	
	if (XR_SUCCEEDED(Result))
	{
		OutFuture = *reinterpret_cast<uint64*>(&Future);
		PendingSessionCreation.Add(Future, SessionHandle);
		
		FCameraCaptureSessionInfo& SessionInfo = CaptureSessions.Add(SessionHandle);
		SessionInfo.Device = DeviceInfo->Device;
		SessionInfo.bValid = false;
		
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Capture session creation started: Handle=%lld, ConfigCount=%d"), 
			SessionHandle, ConfigPointers.Num());
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to create capture session async: %d"), Result);
	return false;
}

bool FHMDPICO::CompleteCaptureSessionCreation(int64 SessionHandle, uint64 Future)
{
	if (!bSupportedCameraImage || !xrCreateCameraCaptureSessionCompletePICO)
	{
		return false;
	}

	XrFutureEXT XrFuture;
	*reinterpret_cast<uint64*>(&XrFuture) = Future;

	// If XR_EXT_future is supported, poll the future state first
	if (bSupportedFutureEXT && xrPollFutureEXT)
	{
		XrFuturePollInfoEXT PollInfo{ XR_TYPE_FUTURE_POLL_INFO_EXT };
		PollInfo.future = XrFuture;

		XrFuturePollResultEXT PollResult{ XR_TYPE_FUTURE_POLL_RESULT_EXT };
		XrResult PollStatus = xrPollFutureEXT(Instance, &PollInfo, &PollResult);

		if (XR_FAILED(PollStatus))
		{
			UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to poll future state: %d"), PollStatus);
			return false;
		}

		// Check future state
		switch (PollResult.state)
		{
		case XR_FUTURE_STATE_PENDING_EXT:
			// Still pending, return false but not an error
			UE_LOG(LogPICOOpenXRHMD, VeryVerbose, TEXT("Capture session creation still pending: Handle=%lld"), SessionHandle);
			return false;

		case XR_FUTURE_STATE_READY_EXT:
			// Ready to complete
			UE_LOG(LogPICOOpenXRHMD, Verbose, TEXT("Capture session creation ready: Handle=%lld"), SessionHandle);
			break;

		default:
			UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("Unexpected future state: %d for SessionHandle=%lld"), PollResult.state, SessionHandle);
			return false;
		}
	}

	// Complete the operation
	XrCreateCameraCaptureSessionCompletionPICO Completion{ XR_TYPE_CREATE_CAMERA_CAPTURE_SESSION_COMPLETION_PICO };
	XrResult Result = xrCreateCameraCaptureSessionCompletePICO(Session, XrFuture, &Completion);
	
	if (XR_SUCCEEDED(Result) && XR_SUCCEEDED(Completion.futureResult))
	{
		FCameraCaptureSessionInfo* SessionInfo = CaptureSessions.Find(SessionHandle);
		if (SessionInfo)
		{
			SessionInfo->Session = Completion.captureSession;
			SessionInfo->bValid = true;
			PendingSessionCreation.Remove(XrFuture);
			
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Capture session created successfully: Handle=%lld"), SessionHandle);
			return true;
		}
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to complete capture session creation: %d, FutureResult=%d"), Result, Completion.futureResult);
	return false;
}

bool FHMDPICO::DestroyCaptureSession(int64 SessionHandle)
{
	if (!bSupportedCameraImage || !xrDestroyCameraCaptureSessionPICO)
	{
		return false;
	}

	FCameraCaptureSessionInfo* SessionInfo = CaptureSessions.Find(SessionHandle);
	if (!SessionInfo || !SessionInfo->bValid)
	{
		return false;
	}

	// End capture if still capturing
	if (SessionInfo->bCapturing)
	{
		EndCameraCapture(SessionHandle);
	}

	// Release all acquired images
	for (auto& ImagePair : SessionInfo->AcquiredImages)
	{
		if (ImagePair.Value) // If not already released
		{
			xrReleaseCameraImagePICO(SessionInfo->Session, ImagePair.Key);
		}
	}
	SessionInfo->AcquiredImages.Empty();

	XrResult Result = xrDestroyCameraCaptureSessionPICO(SessionInfo->Session);
	if (XR_SUCCEEDED(Result))
	{
		CaptureSessions.Remove(SessionHandle);
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Capture session destroyed: Handle=%lld"), SessionHandle);
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to destroy capture session: %d"), Result);
	return false;
}

bool FHMDPICO::GetCameraIntrinsics(int64 SessionHandle, FVector2D& OutFocalLength, FVector2D& OutPrincipalPoint, FVector2D& OutFOV)
{
	if (!bSupportedCameraImage || !xrGetCameraIntrinsicsPICO)
	{
		return false;
	}

	FCameraCaptureSessionInfo* SessionInfo = CaptureSessions.Find(SessionHandle);
	if (!SessionInfo || !SessionInfo->bValid)
	{
		return false;
	}

	XrCameraIntrinsicsPICO Intrinsics{ XR_TYPE_CAMERA_INTRINSICS_PICO };
	XrResult Result = xrGetCameraIntrinsicsPICO(SessionInfo->Session, &Intrinsics);
	
	if (XR_SUCCEEDED(Result))
	{
		// focalLength and principalPoint are in pixels, not meters
		// No need for WorldToMetersScale conversion
		OutFocalLength = FVector2D(Intrinsics.focalLength.x, Intrinsics.focalLength.y);
		OutPrincipalPoint = FVector2D(Intrinsics.principalPoint.x, Intrinsics.principalPoint.y);

		// fov is in degrees
		OutFOV = FVector2D(Intrinsics.fov.x, Intrinsics.fov.y);
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to get camera intrinsics: %d"), Result);
	return false;
}

bool FHMDPICO::GetCameraExtrinsics(int64 SessionHandle, FTransform& OutPose)
{
	if (!bSupportedCameraImage || !xrGetCameraExtrinsicsPICO)
	{
		return false;
	}

	FCameraCaptureSessionInfo* SessionInfo = CaptureSessions.Find(SessionHandle);
	if (!SessionInfo || !SessionInfo->bValid)
	{
		return false;
	}

	XrCameraExtrinsicsPICO Extrinsics{ XR_TYPE_CAMERA_EXTRINSICS_PICO };
	XrResult Result = xrGetCameraExtrinsicsPICO(SessionInfo->Session, &Extrinsics);
	
	if (XR_SUCCEEDED(Result))
	{
		float WorldToMetersScale = OpenXRHMD ? OpenXRHMD->GetWorldToMetersScale() : 100.0f;
		OutPose = ToFTransform(Extrinsics.pose, WorldToMetersScale);
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to get camera extrinsics: %d"), Result);
	return false;
}

bool FHMDPICO::BeginCameraCapture(int64 SessionHandle)
{
	if (!bSupportedCameraImage || !xrBeginCameraCapturePICO)
	{
		return false;
	}

	FCameraCaptureSessionInfo* SessionInfo = CaptureSessions.Find(SessionHandle);
	if (!SessionInfo || !SessionInfo->bValid)
	{
		return false;
	}

	if (SessionInfo->bCapturing)
	{
		UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("Camera capture already started"));
		return false;
	}

	XrCameraCaptureBeginInfoPICO BeginInfo{ XR_TYPE_CAMERA_CAPTURE_BEGIN_INFO_PICO };
	XrResult Result = xrBeginCameraCapturePICO(SessionInfo->Session, &BeginInfo);
	
	if (XR_SUCCEEDED(Result))
	{
		SessionInfo->bCapturing = true;
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera capture started: Handle=%lld"), SessionHandle);
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to begin camera capture: %d"), Result);
	return false;
}

bool FHMDPICO::EndCameraCapture(int64 SessionHandle)
{
	if (!bSupportedCameraImage || !xrEndCameraCapturePICO)
	{
		return false;
	}

	FCameraCaptureSessionInfo* SessionInfo = CaptureSessions.Find(SessionHandle);
	if (!SessionInfo || !SessionInfo->bValid)
	{
		return false;
	}

	if (!SessionInfo->bCapturing)
	{
		UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("Camera capture not started"));
		return false;
	}

	XrResult Result = xrEndCameraCapturePICO(SessionInfo->Session);
	
	if (XR_SUCCEEDED(Result))
	{
		SessionInfo->bCapturing = false;
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera capture ended: Handle=%lld"), SessionHandle);
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to end camera capture: %d"), Result);
	return false;
}

bool FHMDPICO::AcquireCameraImage(int64 SessionHandle, int64 LastCaptureTime, int64& OutCaptureTime, uint64& OutImageID, bool& bNewImage)
{
	if (!bSupportedCameraImage || !xrAcquireCameraImagePICO)
	{
		return false;
	}

	FCameraCaptureSessionInfo* SessionInfo = CaptureSessions.Find(SessionHandle);
	if (!SessionInfo || !SessionInfo->bValid || !SessionInfo->bCapturing)
	{
		return false;
	}

	XrCameraImageAcquireInfoPICO AcquireInfo{ XR_TYPE_CAMERA_IMAGE_ACQUIRE_INFO_PICO };
	AcquireInfo.lastCaptureTime = static_cast<XrTime>(LastCaptureTime);

	XrCameraImagePICO Image{ XR_TYPE_CAMERA_IMAGE_PICO };
	XrResult Result = xrAcquireCameraImagePICO(SessionInfo->Session, &AcquireInfo, &Image);
	
	if (Result == XR_CAMERA_IMAGE_NO_UPDATE_PICO)
	{
		bNewImage = false;
		return true;
	}
	
	if (XR_SUCCEEDED(Result))
	{
		OutCaptureTime = static_cast<int64>(Image.captureTime);
		OutImageID = static_cast<uint64>(Image.imageId);
		SessionInfo->AcquiredImages.Add(Image.imageId, true);
		bNewImage = true;
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to acquire camera image: %d"), Result);
	return false;
}

bool FHMDPICO::GetCameraImageData(int64 SessionHandle, uint64 ImageID, TArray<uint8>& OutBuffer, int32& OutWidth, int32& OutHeight, int32& OutStride)
{
	if (!bSupportedCameraImage || !xrGetCameraImageDataPICO)
	{
		return false;
	}

	FCameraCaptureSessionInfo* SessionInfo = CaptureSessions.Find(SessionHandle);
	if (!SessionInfo || !SessionInfo->bValid)
	{
		return false;
	}

	XrCameraImageDataRawBufferPICO ImageData{ XR_TYPE_CAMERA_IMAGE_DATA_RAW_BUFFER_PICO };
	XrResult Result = xrGetCameraImageDataPICO(SessionInfo->Session, static_cast<XrCameraImageIdPICO>(ImageID), 
		reinterpret_cast<XrCameraImageDataBaseHeaderPICO*>(&ImageData));
	
	if (XR_SUCCEEDED(Result))
	{
		OutWidth = ImageData.width;
		OutHeight = ImageData.height;
		OutStride = ImageData.stride;
		
		OutBuffer.SetNum(ImageData.bufferSize);
		FMemory::Memcpy(OutBuffer.GetData(), ImageData.buffer, ImageData.bufferSize);
		
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to get camera image data: %d"), Result);
	return false;
}

bool FHMDPICO::ReleaseCameraImage(int64 SessionHandle, uint64 ImageID)
{
	if (!bSupportedCameraImage || !xrReleaseCameraImagePICO)
	{
		return false;
	}

	FCameraCaptureSessionInfo* SessionInfo = CaptureSessions.Find(SessionHandle);
	if (!SessionInfo || !SessionInfo->bValid)
	{
		return false;
	}

	XrResult Result = xrReleaseCameraImagePICO(SessionInfo->Session, static_cast<XrCameraImageIdPICO>(ImageID));
	
	if (XR_SUCCEEDED(Result))
	{
		SessionInfo->AcquiredImages.Remove(static_cast<XrCameraImageIdPICO>(ImageID));
		return true;
	}

	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to release camera image: %d"), Result);
	return false;
}

bool FHMDPICO::GetCameraProperties(uint64 CameraID, TArray<uint8>& OutPropertiesData)
{
	if (!bSupportedCameraImage || !xrEnumerateCameraPropertyTypesPICO || !xrGetCameraPropertiesPICO)
	{
		return false;
	}

	// First, enumerate available property types
	uint32 TypeCount = 0;
	XrResult Result = xrEnumerateCameraPropertyTypesPICO(Instance, static_cast<XrCameraIdPICO>(CameraID), 0, &TypeCount, nullptr);
	if (XR_FAILED(Result) || TypeCount == 0)
	{
		UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("Failed to enumerate camera property types or no properties available"));
		return false;
	}

	TArray<XrCameraPropertyTypePICO> PropertyTypes;
	PropertyTypes.SetNum(TypeCount);
	Result = xrEnumerateCameraPropertyTypesPICO(Instance, static_cast<XrCameraIdPICO>(CameraID), TypeCount, &TypeCount, PropertyTypes.GetData());
	if (XR_FAILED(Result))
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to get camera property types: %d"), Result);
		return false;
	}
	else
	{
		for(uint32 i = 0; i < TypeCount; i++)
		{
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu Property Type[%d]: %d"), CameraID, i, static_cast<int32>(PropertyTypes[i]));
		}
	}

	// Build property structures for each type
	TArray<XrCameraPropertyBaseHeaderPICO*> PropertyPointers;
	
	XrCameraPropertyFacingPICO FacingProperty = { XR_TYPE_CAMERA_PROPERTY_FACING_PICO };
	XrCameraPropertyPositionPICO PositionProperty = { XR_TYPE_CAMERA_PROPERTY_POSITION_PICO };
	XrCameraPropertyCameraTypePICO TypeProperty = { XR_TYPE_CAMERA_PROPERTY_CAMERA_TYPE_PICO };

	for (uint32 i = 0; i < TypeCount; i++)
	{
		switch (PropertyTypes[i])
		{
		case XR_CAMERA_PROPERTY_TYPE_FACING_PICO:
			PropertyPointers.Add(reinterpret_cast<XrCameraPropertyBaseHeaderPICO*>(&FacingProperty));
			break;
		case XR_CAMERA_PROPERTY_TYPE_POSITION_PICO:
			PropertyPointers.Add(reinterpret_cast<XrCameraPropertyBaseHeaderPICO*>(&PositionProperty));
			break;
		case XR_CAMERA_PROPERTY_TYPE_CAMERA_TYPE_PICO:
			PropertyPointers.Add(reinterpret_cast<XrCameraPropertyBaseHeaderPICO*>(&TypeProperty));
			break;
		default:
			break;
		}
	}

	// Get properties
	XrCameraPropertiesGetInfoPICO GetInfo = { XR_TYPE_CAMERA_PROPERTIES_GET_INFO_PICO };
	GetInfo.cameraId = static_cast<XrCameraIdPICO>(CameraID);

	XrCameraPropertiesPICO Properties = { XR_TYPE_CAMERA_PROPERTIES_PICO };
	Properties.propertyCount = PropertyPointers.Num();
	Properties.properties = PropertyPointers.GetData();

	Result = xrGetCameraPropertiesPICO(Instance, &GetInfo, &Properties);
	if (XR_FAILED(Result))
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to get camera properties: %d"), Result);
		return false;
	}

	// Serialize properties to output buffer
	// Simple serialization: [count][type1][data1][type2][data2]...
	FMemoryWriter Writer(OutPropertiesData);
	
	Writer << TypeCount;
	
	for (uint32 i = 0; i < Properties.propertyCount; i++)
	{
		XrCameraPropertyBaseHeaderPICO* PropHeader = Properties.properties[i];
		int32 StructType = static_cast<int32>(PropHeader->type);
		Writer << StructType;
		
		switch (PropHeader->type)
		{
		case XR_TYPE_CAMERA_PROPERTY_FACING_PICO:
		{
			XrCameraPropertyFacingPICO* Facing = reinterpret_cast<XrCameraPropertyFacingPICO*>(PropHeader);
			int32 FacingValue = static_cast<int32>(Facing->facing);
			Writer << FacingValue;
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu Facing: %d"), CameraID, FacingValue);
			break;
		}
		case XR_TYPE_CAMERA_PROPERTY_POSITION_PICO:
		{
			XrCameraPropertyPositionPICO* Position = reinterpret_cast<XrCameraPropertyPositionPICO*>(PropHeader);
			int32 PositionValue = static_cast<int32>(Position->position);
			Writer << PositionValue;
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu Position: %d"), CameraID, PositionValue);
			break;
		}
		case XR_TYPE_CAMERA_PROPERTY_CAMERA_TYPE_PICO:
		{
			XrCameraPropertyCameraTypePICO* CamType = reinterpret_cast<XrCameraPropertyCameraTypePICO*>(PropHeader);
			int32 TypeValue = static_cast<int32>(CamType->cameraType);
			Writer << TypeValue;
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu Type: %d"), CameraID, TypeValue);
			break;
		}
		default:
			break;
		}
	}

	return true;
}

bool FHMDPICO::GetCameraSupportedCapabilities(uint64 CameraID, TArray<uint8>& OutCapabilitiesData)
{
	if (!bSupportedCameraImage || !xrEnumerateCameraCapabilityTypesPICO || !xrGetCameraSupportedCapabilitiesPICO)
	{
		return false;
	}

	// First, enumerate available capability types
	uint32 TypeCount = 0;
	XrResult Result = xrEnumerateCameraCapabilityTypesPICO(Instance, static_cast<XrCameraIdPICO>(CameraID), 0, &TypeCount, nullptr);
	if (XR_FAILED(Result) || TypeCount == 0)
	{
		UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("Failed to enumerate camera capability types or no capabilities available"));
		return false;
	}

	TArray<XrCameraCapabilityTypePICO> CapabilityTypes;
	CapabilityTypes.SetNum(TypeCount);
	Result = xrEnumerateCameraCapabilityTypesPICO(Instance, static_cast<XrCameraIdPICO>(CameraID), TypeCount, &TypeCount, CapabilityTypes.GetData());
	if (XR_FAILED(Result))
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to get camera capability types: %d"), Result);
		return false;
	}
	else
	{
		for(uint32 i = 0; i < TypeCount; i++)
		{
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu Capability Type[%d]: %d"), CameraID, i, static_cast<int32>(CapabilityTypes[i]));
		}
	}

	// Build capability structures for each type
	TArray<XrCameraSupportedCapabilityBaseHeaderPICO*> CapabilityPointers;
	
	XrCameraSupportedCapabilityImageResolutionPICO ResolutionCap = { XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_RESOLUTION_PICO };
	XrCameraSupportedCapabilityDataTransferTypePICO TransferCap = { XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_DATA_TRANSFER_TYPE_PICO };
	XrCameraSupportedCapabilityImageFormatPICO FormatCap = { XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_FORMAT_PICO };
	XrCameraSupportedCapabilityCameraModelPICO ModelCap = { XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_CAMERA_MODEL_PICO };
	XrCameraSupportedCapabilityImageFpsPICO FPSCap = { XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_FPS_PICO };

	for (uint32 i = 0; i < TypeCount; i++)
	{
		switch (CapabilityTypes[i])
		{
		case XR_CAMERA_CAPABILITY_TYPE_IMAGE_RESOLUTION_PICO:
			CapabilityPointers.Add(reinterpret_cast<XrCameraSupportedCapabilityBaseHeaderPICO*>(&ResolutionCap));
			break;
		case XR_CAMERA_CAPABILITY_TYPE_DATA_TRANSFER_TYPE_PICO:
			CapabilityPointers.Add(reinterpret_cast<XrCameraSupportedCapabilityBaseHeaderPICO*>(&TransferCap));
			break;
		case XR_CAMERA_CAPABILITY_TYPE_IMAGE_FORMAT_PICO:
			CapabilityPointers.Add(reinterpret_cast<XrCameraSupportedCapabilityBaseHeaderPICO*>(&FormatCap));
			break;
		case XR_CAMERA_CAPABILITY_TYPE_CAMERA_MODEL_PICO:
			CapabilityPointers.Add(reinterpret_cast<XrCameraSupportedCapabilityBaseHeaderPICO*>(&ModelCap));
			break;
		case XR_CAMERA_CAPABILITY_TYPE_IMAGE_FPS_PICO:
			CapabilityPointers.Add(reinterpret_cast<XrCameraSupportedCapabilityBaseHeaderPICO*>(&FPSCap));
			break;
		default:
			break;
		}
	}

	// ====================================================================================
	// FIRST CALL: Query required capacity (Two-Call Idiom Step 1)
	// ====================================================================================
	XrCameraSupportedCapabilitiesGetInfoPICO GetInfo = { XR_TYPE_CAMERA_SUPPORTED_CAPABILITIES_GET_INFO_PICO };
	GetInfo.id = static_cast<XrCameraIdPICO>(CameraID);

	XrCameraSupportedCapabilitiesPICO Capabilities = { XR_TYPE_CAMERA_SUPPORTED_CAPABILITIES_PICO };
	Capabilities.capabilityCount = CapabilityPointers.Num();
	Capabilities.capabilities = CapabilityPointers.GetData();

	Result = xrGetCameraSupportedCapabilitiesPICO(Instance, &GetInfo, &Capabilities);
	if (XR_FAILED(Result))
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to query camera capability sizes: %d"), Result);
		return false;
	}

	UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu required capacities queried successfully"), CameraID);

	// ====================================================================================
	// Allocate arrays based on queried capacity (Two-Call Idiom - Between Step 1 and 2)
	// ====================================================================================
	TArray<XrExtent2Di> Resolutions;
	if (ResolutionCap.resolutionCountOutput > 0)
	{
		Resolutions.SetNum(ResolutionCap.resolutionCountOutput);
		ResolutionCap.resolutionCapacityInput = ResolutionCap.resolutionCountOutput;
		ResolutionCap.resolutions = Resolutions.GetData();
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera  Resolutions required: %d"), ResolutionCap.resolutionCountOutput);
	}
	
	TArray<XrCameraDataTransferTypePICO> TransferTypes;
	if (TransferCap.typeCountOutput > 0)
	{
		TransferTypes.SetNum(TransferCap.typeCountOutput);
		TransferCap.typeCapacityInput = TransferCap.typeCountOutput;
		TransferCap.types = TransferTypes.GetData();
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera  TransferTypes required: %d"), TransferCap.typeCountOutput);
	}
	
	TArray<XrCameraImageFormatPICO> Formats;
	if (FormatCap.formatCountOutput > 0)
	{
		Formats.SetNum(FormatCap.formatCountOutput);
		FormatCap.formatCapacityInput = FormatCap.formatCountOutput;
		FormatCap.formats = Formats.GetData();
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera  Formats required: %d"), FormatCap.formatCountOutput);
	}
	
	TArray<XrCameraModelPICO> Models;
	if (ModelCap.modelCountOutput > 0)
	{
		Models.SetNum(ModelCap.modelCountOutput);
		ModelCap.modelCapacityInput = ModelCap.modelCountOutput;
		ModelCap.models = Models.GetData();
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera  Models required: %d"), ModelCap.modelCountOutput);
	}
	
	TArray<XrCameraImageFpsPICO> FPSValues;
	if (FPSCap.fpsCountOutput > 0)
	{
		FPSValues.SetNum(FPSCap.fpsCountOutput);
		FPSCap.fpsCapacityInput = FPSCap.fpsCountOutput;
		FPSCap.fps = FPSValues.GetData();
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera  FPS required: %d"), FPSCap.fpsCountOutput);
	}

	// ====================================================================================
	// SECOND CALL: Get actual data (Two-Call Idiom Step 2)
	// ====================================================================================
	Result = xrGetCameraSupportedCapabilitiesPICO(Instance, &GetInfo, &Capabilities);
	if (XR_FAILED(Result))
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Failed to get camera supported capabilities: %d"), Result);
		return false;
	}

	UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu capabilities retrieved successfully"), CameraID);

	// Serialize capabilities to output buffer
	// Simple serialization: [count][type1][data1][type2][data2]...
	FMemoryWriter Writer(OutCapabilitiesData);
	
	Writer << TypeCount;
	
	for (uint32 i = 0; i < Capabilities.capabilityCount; i++)
	{
		XrCameraSupportedCapabilityBaseHeaderPICO* CapHeader = Capabilities.capabilities[i];
		int32 StructType = static_cast<int32>(CapHeader->type);
		Writer << StructType;
		
		switch (CapHeader->type)
		{
		case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_RESOLUTION_PICO:
		{
			XrCameraSupportedCapabilityImageResolutionPICO* ResCap = reinterpret_cast<XrCameraSupportedCapabilityImageResolutionPICO*>(CapHeader);
			Writer << ResCap->resolutionCountOutput;
			for (uint32 j = 0; j < ResCap->resolutionCountOutput; j++)
			{
				Writer << ResCap->resolutions[j].width;
				Writer << ResCap->resolutions[j].height;
				UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu Resolution[%d]: %dx%d"), CameraID, j, 
					ResCap->resolutions[j].width, ResCap->resolutions[j].height);
			}
			break;
		}
		case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_DATA_TRANSFER_TYPE_PICO:
		{
			XrCameraSupportedCapabilityDataTransferTypePICO* TransCap = reinterpret_cast<XrCameraSupportedCapabilityDataTransferTypePICO*>(CapHeader);
			Writer << TransCap->typeCountOutput;
			for (uint32 j = 0; j < TransCap->typeCountOutput; j++)
			{
				int32 TypeValue = static_cast<int32>(TransCap->types[j]);
				Writer << TypeValue;
				UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu TransferType[%d]: %d"), CameraID, j, TypeValue);
			}
			break;
		}
		case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_FORMAT_PICO:
		{
			XrCameraSupportedCapabilityImageFormatPICO* FmtCap = reinterpret_cast<XrCameraSupportedCapabilityImageFormatPICO*>(CapHeader);
			Writer << FmtCap->formatCountOutput;
			for (uint32 j = 0; j < FmtCap->formatCountOutput; j++)
			{
				int32 FormatValue = static_cast<int32>(FmtCap->formats[j]);
				Writer << FormatValue;
				UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu Format[%d]: %d"), CameraID, j, FormatValue);
			}
			break;
		}
		case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_CAMERA_MODEL_PICO:
		{
			XrCameraSupportedCapabilityCameraModelPICO* ModCap = reinterpret_cast<XrCameraSupportedCapabilityCameraModelPICO*>(CapHeader);
			Writer << ModCap->modelCountOutput;
			for (uint32 j = 0; j < ModCap->modelCountOutput; j++)
			{
				int32 ModelValue = static_cast<int32>(ModCap->models[j]);
				Writer << ModelValue;
				UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu Model[%d]: %d"), CameraID, j, ModelValue);
			}
			break;
		}
		case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_FPS_PICO:
		{
			XrCameraSupportedCapabilityImageFpsPICO* FpsCap = reinterpret_cast<XrCameraSupportedCapabilityImageFpsPICO*>(CapHeader);
			Writer << FpsCap->fpsCountOutput;
			for (uint32 j = 0; j < FpsCap->fpsCountOutput; j++)
			{
				int32 FPSValue = static_cast<int32>(FpsCap->fps[j]);
				Writer << FPSValue;
				UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera %llu FPS[%d]: %d"), CameraID, j, FPSValue);
			}
			break;
		}
		default:
			break;
		}
	}

	return true;
}

/*************************** Camera Image Extension Implementation End ***************************/
