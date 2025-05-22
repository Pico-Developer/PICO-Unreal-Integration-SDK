// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_MotionTrackingModule.h"
#include "PXR_HMDModule.h"

#define LOCTEXT_NAMESPACE "PICOXRMotionTracking"

FPICOXRMotionTrackingModule::FPICOXRMotionTrackingModule()
{
}

void FPICOXRMotionTrackingModule::StartupModule()
{
}

void FPICOXRMotionTrackingModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FPICOXRMotionTrackingModule, PICOXRMotionTracking)

#undef LOCTEXT_NAMESPACE