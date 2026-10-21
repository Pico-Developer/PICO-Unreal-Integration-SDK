// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "PICO_InputFunctionLibrary.generated.h"

UCLASS()
class PICOOPENXRINPUT_API UInputFunctionLibraryPICO : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
		
public:
	/**
	* Legacy controller battery query API.
	* This API was originally introduced for left/right controller battery queries only.
	* On newer devices, the implementation will first try the newer EXT-based battery API and
	* fall back to the legacy controller path only when the new API is unavailable.
	* This keeps existing applications using the legacy API working on newer devices.
	*
	* @param Hand		left or right hand
	* @param Level      the current battery level of the controller
	*
	* @return		    true if the function call was successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|Input")
	static bool GetControllerBatteryLevelPICO(const EControllerHand Hand, float& Level);

	/**
	* Play a haptic effect on the controller
	*
	* @param HapticEffect   The haptic effect to play (can be null if using test curve)
	* @param Hand           Left or right hand
	* @param Scale          Intensity scale (0.0 - 1.0)
	* @param bLoop          Whether to loop the effect
	* @param bUseTestCurve  If true, generates a test curve instead of using HapticEffect
	* @param TestCurveNumKeys  Number of keyframes for test curve (500 or less = single-shot, 501+ = streaming)
	* @param TestCurveDuration Duration of test curve in seconds
	* @param MaxPointsPerFrame Maximum points per frame (0 = auto-detect mode [default 490 for streaming], non-zero = force streaming and clamp to 1-490)
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|Input")
	static void PlayHapticEffectPICO(class UHapticFeedbackEffect_Base* HapticEffect, EControllerHand Hand, float Scale = 1.0f, bool bLoop = false, bool bUseTestCurve = false, int32 TestCurveNumKeys = 500, float TestCurveDuration = 5.0f, int32 MaxPointsPerFrame = 0);
	
	UFUNCTION(BlueprintCallable, Category = "PICO|Input")
	static void StopHapticEffectPICO(EControllerHand Hand);

	/**
	* Play a PICO Haptic Format (PHF) asset with full parametric haptic support
	* This function properly handles PHF-specific features like play states and absolute frequency values
	*
	* @param PHFAsset       The PICO Haptic Asset to play
	* @param Hand           Left or right hand
	* @param Scale          Intensity scale multiplier (0.0 - 1.0)
	* @param bLoop          Whether to loop the haptic pattern
	* @param MaxPointsPerFrame Maximum points per frame (0 = auto-detect mode [default 490 for streaming], non-zero = force streaming and clamp to 1-490)
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|Input")
	static void PlayPHFHapticAssetPICO(class UPICOHapticAsset* PHFAsset, EControllerHand Hand, float Scale = 1.0f, bool bLoop = false, int32 MaxPointsPerFrame = 0);
};
