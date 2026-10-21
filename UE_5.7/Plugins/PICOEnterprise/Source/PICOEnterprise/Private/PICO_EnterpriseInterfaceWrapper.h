// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#pragma warning(disable : 5208)

#define PVRP_EXPORT typedef
#include "PXR_EnterpriseInterface.h"
#undef PVRP_EXPORT
#include "CameraAPI.h"

struct FInterfaceWrapper
{
private:
	FInterfaceWrapper()
	{
		Reset();
	}

	void Reset()
	{
		memset(this, 0, sizeof(FInterfaceWrapper));
	}

	bool Initialized = false;

	static FInterfaceWrapper Wrapper;

	getHeadTrackingConfidence* PE_GetHeadTrackingConfidence_Interface;
	openVSTCamera* PE_OpenVSTCamera_Interface;
	closeVSTCamera* PE_CloseVSTCamera_Interface;
	acquireVSTCameraFrame* PE_AcquireVSTCameraFrame_Interface;
	acquireVSTCameraFrameAntiDistortion* PE_AcquireVSTCameraFrameAntiDistortion_Interface;
	getCameraParameters* PE_GetCameraParameters_Interface;

public:
#if PLATFORM_ANDROID
    setUEEnv* pe_SetJNIEnv;
#endif
	closeCamera* pe_CloseCamera;
	openCameraAsync* pe_OpenCameraAsync;
	setConfigureDefault* pe_SetConfigureDefault;
	setConfigure* pe_SetConfigure;
	setConfigureMap* pe_SetConfigureMap;
	startPerformance* pe_StartPerformance;
	startPreview* pe_StartPreview;
	setCameraFrameBuffer* pe_SetCameraFrameBuffer;
	setCapturelibCallBack* pe_SetCapturelibCallBack;
	getCameraExtrinsics* pe_GetCameraExtrinsics;
	getCameraIntrinsics* pe_GetCameraIntrinsics;
	getCameraParametersNew* pe_GetCameraParametersNew;

public:
	static FInterfaceWrapper* GetInstance() { return &Wrapper; }

	bool Initialize();
	void Destroy();

public:
	int32 PE_GetHeadTrackingConfidence()
	{
		return PE_GetHeadTrackingConfidence_Interface ? PE_GetHeadTrackingConfidence_Interface() : 0;
	}

	bool PE_OpenVSTCamera()
	{
		return PE_OpenVSTCamera_Interface ? (PE_OpenVSTCamera_Interface() == 0) : false;
	}

	bool PE_CloseVSTCamera()
	{
		return PE_CloseVSTCamera_Interface ? (PE_CloseVSTCamera_Interface() == 0) : false;
	}

	bool PE_AcquireVSTCameraFrame(frame_item_ext_t& out_frame)
	{
		return PE_AcquireVSTCameraFrame_Interface ? (PE_AcquireVSTCameraFrame_Interface(out_frame) == 0) : false;
	}

	bool PE_AcquireVSTCameraFrameAntiDistortion(const char* token, int32_t width, int32_t height, frame_item_ext_t& frame)
	{
		return PE_AcquireVSTCameraFrameAntiDistortion_Interface ? (PE_AcquireVSTCameraFrameAntiDistortion_Interface(token, width, height, frame) == 0) : false;
	}

	bool PE_GetCameraParameters(const char* token, rgb_camera_params& params)
	{
		return PE_GetCameraParameters_Interface ? (PE_GetCameraParameters_Interface(token, params) == 0) : false;
	}
};