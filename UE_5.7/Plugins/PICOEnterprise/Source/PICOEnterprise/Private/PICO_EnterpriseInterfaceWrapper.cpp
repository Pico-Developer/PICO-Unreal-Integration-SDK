// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_EnterpriseInterfaceWrapper.h"

#if PLATFORM_ANDROID
#include <dlfcn.h>
#include <jni.h>
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#include <android_native_app_glue.h>
extern struct android_app* GNativeAndroidApp;
#endif

DEFINE_LOG_CATEGORY_STATIC(LogPXRBWrapper, Log, All);

FInterfaceWrapper FInterfaceWrapper::Wrapper;

void* LoadEntryPoint(void* Handle, const char* EntryPointName)
{
#if PLATFORM_ANDROID
	void* ptr = dlsym(Handle, EntryPointName);
	if (ptr)
	{
		UE_LOG(LogPXRBWrapper, Display, TEXT("Load libpxr_xrsdk_native.so [%s] Success"), ANSI_TO_TCHAR(EntryPointName));
	}
	else
	{
		UE_LOG(LogPXRBWrapper, Display, TEXT("Load libpxr_xrsdk_native.so [%s] Failed"), ANSI_TO_TCHAR(EntryPointName));
	}
	return ptr;
#endif
	UE_LOG(LogPXRBWrapper, Display, TEXT("Load libpxr_xrsdk_native.so [%s] Failed"), ANSI_TO_TCHAR(EntryPointName));
	return nullptr;
}

bool FInterfaceWrapper::Initialize()
{
	UE_LOG(LogPXRBWrapper, Display, TEXT("FInterfaceWrapper Begin Init"));
	if (Initialized)
	{
		UE_LOG(LogPXRBWrapper, Display, TEXT("FInterfaceWrapper Already Init"));
		return true;
	}

	void* Handle = nullptr;
#if PLATFORM_ANDROID
	Handle = FPlatformProcess::GetDllHandle(TEXT("libpxr_xrsdk_native.so"));
#endif
	if (Handle == nullptr)
	{
		UE_LOG(LogPXRBWrapper, Display, TEXT("GetDllHandle Failed"));
	}
	else
	{
		PE_GetHeadTrackingConfidence_Interface = reinterpret_cast<getHeadTrackingConfidence*>(LoadEntryPoint(Handle, "getHeadTrackingConfidence"));
		PE_OpenVSTCamera_Interface = reinterpret_cast<openVSTCamera*>(LoadEntryPoint(Handle, "openVSTCamera"));
		PE_CloseVSTCamera_Interface = reinterpret_cast<closeVSTCamera*>(LoadEntryPoint(Handle, "closeVSTCamera"));
		PE_AcquireVSTCameraFrame_Interface = reinterpret_cast<acquireVSTCameraFrame*>(LoadEntryPoint(Handle, "acquireVSTCameraFrame"));
		PE_AcquireVSTCameraFrameAntiDistortion_Interface = reinterpret_cast<acquireVSTCameraFrameAntiDistortion*>(LoadEntryPoint(Handle, "acquireVSTCameraFrameAntiDistortion"));
		PE_GetCameraParameters_Interface = reinterpret_cast<getCameraParameters*>(LoadEntryPoint(Handle, "getCameraParameters"));
		UE_LOG(LogPXRBWrapper, Display, TEXT("FInterfaceWrapper Init Success"));
	}

	Handle = nullptr;
#if PLATFORM_ANDROID
	Handle = FPlatformProcess::GetDllHandle(TEXT("libCameraRenderingPlugin.so"));
#endif
	if (Handle == nullptr)
	{
		UE_LOG(LogPXRBWrapper, Display, TEXT("GetDllHandle Failed: libCameraRenderingPlugin.so"));
	}
	else
	{
#if PLATFORM_ANDROID
        pe_SetJNIEnv = reinterpret_cast<setUEEnv*>(LoadEntryPoint(Handle, "setUEEnv"));
#endif
		pe_CloseCamera = reinterpret_cast<closeCamera*>(LoadEntryPoint(Handle, "closeCamera"));
		pe_OpenCameraAsync = reinterpret_cast<openCameraAsync*>(LoadEntryPoint(Handle, "openCameraAsync"));
		pe_SetConfigureDefault = reinterpret_cast<setConfigureDefault*>(LoadEntryPoint(Handle, "setConfigureDefault"));
		pe_SetConfigure = reinterpret_cast<setConfigure*>(LoadEntryPoint(Handle, "setConfigure"));
		pe_SetConfigureMap = reinterpret_cast<setConfigureMap*>(LoadEntryPoint(Handle, "setConfigureMap"));
		pe_StartPerformance = reinterpret_cast<startPerformance*>(LoadEntryPoint(Handle, "startPerformance"));
		pe_StartPreview = reinterpret_cast<startPreview*>(LoadEntryPoint(Handle, "startPreview"));
		pe_SetCameraFrameBuffer = reinterpret_cast<setCameraFrameBuffer*>(LoadEntryPoint(Handle, "setCameraFrameBuffer"));
		pe_SetCapturelibCallBack = reinterpret_cast<setCapturelibCallBack*>(LoadEntryPoint(Handle, "setCapturelibCallBack"));
		pe_GetCameraExtrinsics = reinterpret_cast<getCameraExtrinsics*>(LoadEntryPoint(Handle, "getCameraExtrinsics"));
		pe_GetCameraIntrinsics = reinterpret_cast<getCameraIntrinsics*>(LoadEntryPoint(Handle, "getCameraIntrinsics"));
		pe_GetCameraParametersNew = reinterpret_cast<getCameraParametersNew*>(LoadEntryPoint(Handle, "getCameraParametersNew"));

		check(pe_CloseCamera != nullptr);
		check(pe_OpenCameraAsync != nullptr);
		check(pe_SetConfigureDefault != nullptr);
		check(pe_SetConfigure != nullptr);
		check(pe_SetConfigureMap != nullptr);
		check(pe_StartPerformance != nullptr);
		check(pe_StartPreview != nullptr);
		check(pe_SetCameraFrameBuffer != nullptr);
		check(pe_SetCapturelibCallBack != nullptr);
		check(pe_GetCameraExtrinsics != nullptr);
		check(pe_GetCameraIntrinsics != nullptr);
		check(pe_GetCameraParametersNew != nullptr);

		UE_LOG(LogPXRBWrapper, Display, TEXT("FInterfaceWrapper Camera API Init Success"));
        
#if PLATFORM_ANDROID
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        check(pe_SetJNIEnv != nullptr);
        pe_SetJNIEnv(GNativeAndroidApp->activity->vm, Env, &FJavaWrapper::GameActivityClassID, &FJavaWrapper::GameActivityThis);
    }
#endif
	}

	Initialized = true;
	return true;
}

void FInterfaceWrapper::Destroy()
{
	Initialized = false;
	Reset();
}
