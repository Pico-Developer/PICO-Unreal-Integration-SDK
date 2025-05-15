// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once

#pragma warning(disable : 5208)

#define PVRP_EXPORT typedef
#include "PXR_EnterpriseInterface.h"
#undef PVRP_EXPORT

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

	getHeadTrackingConfidence* PXR_GetHeadTrackingConfidence_Interface;
	openVSTCamera* PXR_OpenVSTCamera_Interface;
	closeVSTCamera* PXR_CloseVSTCamera_Interface;
	acquireVSTCameraFrame* PXR_AcquireVSTCameraFrame_Interface;
	acquireVSTCameraFrameAntiDistortion* PXR_AcquireVSTCameraFrameAntiDistortion_Interface;
	getCameraParameters* PXR_GetCameraParameters_Interface;

public:
	static FInterfaceWrapper* GetInstance() { return &Wrapper; }

	bool Initialize();
	void Destroy();

public:
	int32 PXR_GetHeadTrackingConfidence()
	{
		return PXR_GetHeadTrackingConfidence_Interface ? PXR_GetHeadTrackingConfidence_Interface() : 0;
	}

	bool PXR_OpenVSTCamera()
	{
		return PXR_OpenVSTCamera_Interface ? (PXR_OpenVSTCamera_Interface() == 0) : false;
	}

	bool PXR_CloseVSTCamera()
	{
		return PXR_CloseVSTCamera_Interface ? (PXR_CloseVSTCamera_Interface() == 0) : false;
	}

	bool PXR_AcquireVSTCameraFrame(frame_item_ext_t& out_frame)
	{
		return PXR_AcquireVSTCameraFrame_Interface ? (PXR_AcquireVSTCameraFrame_Interface(out_frame) == 0) : false;
	}

	bool PXR_AcquireVSTCameraFrameAntiDistortion(const char* token, int32_t width, int32_t height, frame_item_ext_t& frame)
	{
		return PXR_AcquireVSTCameraFrameAntiDistortion_Interface ? (PXR_AcquireVSTCameraFrameAntiDistortion_Interface(token, width, height, frame) == 0) : false;
	}

	bool PXR_GetCameraParameters(const char* token, rgb_camera_params& params)
	{
		return PXR_GetCameraParameters_Interface ? (PXR_GetCameraParameters_Interface(token, params) == 0) : false;
	}
};