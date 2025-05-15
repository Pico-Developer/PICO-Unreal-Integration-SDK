// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once

#include "PXR_MotionTrackingTypes.h"
#include "PXR_MotionTrackingFunctionLibrary.generated.h"

UCLASS()
class  PICOXRMOTIONTRACKING_API UPICOXRMotionTrackingFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "PXR|FaceTracking")
		static bool GetFaceTrackingSupported(bool& Supported, TArray<EPXRFaceTrackingMode>& SupportedModes);

	UFUNCTION(BlueprintCallable, Category = "PXR|FaceTracking")
		static bool GetFaceTrackingState(bool& IsTracking, FPXRFaceTrackingState& TrackingState);

	UFUNCTION(BlueprintCallable, Category = "PXR|FaceTracking")
		static bool StartFaceTracking(const FPXRFaceTrackingStartInfo& StartInfo);

	UFUNCTION(BlueprintCallable, Category = "PXR|FaceTracking")
		static bool StopFaceTracking(const FPXRFaceTrackingStopInfo& StopInfo);

	UFUNCTION(BlueprintCallable, Category = "PXR|FaceTracking")
		static bool GetFaceTrackingData(const FPXRFaceTrackingDataGetInfo& GetInfo, FPXRFaceTrackingData& OutFaceTrackingData);

	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool GetEyeTrackingSupported(bool& Supported, TArray<EPXREyeTrackingMode>& SupportedModes);

	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool GetEyeTrackingState(bool& IsTracking, FPXREyeTrackingState& TrackingState);

	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool StartEyeTracking(const FPXREyeTrackingStartInfo& StartInfo);

	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool StopEyeTracking(const FPXREyeTrackingStopInfo& StopInfo);

	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking", meta = (WorldToMetersScale = "100.0"))
		static bool GetEyeTrackingData(float WorldToMetersScale, const FPXREyeTrackingDataGetInfo& GetInfo, FPXREyeTrackingData& OutEyeTrackingData);

	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool GetEyeOpenness(float& LeftEyeOpenness, float& RightEyeOpenness);

	UFUNCTION(BlueprintCallable, Category = "PXR|EyeTracking")
		static bool GetEyePupilInfo(FPXREyePupilInfo& EyePupilInfo);

};
