// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.


#include "PXR_Shaders.h"

IMPLEMENT_SHADER_TYPE(, FPICOCubemapPS, TEXT("/Plugin/PICOXR/Private/PICOShaders.usf"), TEXT("MainForCubemap"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(, FPICOLightCubemapPS, TEXT("/Plugin/PICOXR/Private/PICOShaders.usf"), TEXT("MainForLightCubemap"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(template <>, FScreenPSEnvironmentDepthMinMax<true>, TEXT("/Plugin/PICOXR/Private/ScreenPSEnvironmentDepthMinMax.usf"), TEXT("Main"), SF_Pixel);
IMPLEMENT_SHADER_TYPE(template <>, FScreenPSEnvironmentDepthMinMax<false>, TEXT("/Plugin/PICOXR/Private/ScreenPSEnvironmentDepthMinMax.usf"), TEXT("Main"), SF_Pixel);
