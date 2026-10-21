// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "PICO_PassthroughFunctionLibrary.generated.h"

#ifdef XR_FB_passthrough
typedef enum {
    NoneStyle = 0,
    MonoToRgba = 1,
    MonoToMono = 2,
    HandsContrast = 3,
    BrightnessContrastSaturation = 4,



    EnumSize = 0x7fffffff
} PassthroughColorMapType;
#endif // XR_FB_passthrough

UCLASS()
class PICOOPENXRPASSTHROUGH_API UPassthroughFunctionLibraryPICO : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	* Check if the current device supports passthrough
    *
	* @param Support    true if the device supports passthrough, false otherwise
	* @param HasColor   true if the device supports color passthrough, false otherwise
	* @param HasDepth   true if the device supports depth passthrough, false otherwise
    *
	* @return		    true if the function call was successful, false otherwise
    */
	UFUNCTION(BlueprintCallable, Category = "PICO|Passthrough")
	static bool GetSupportedPassthroughPICO(bool& Support, bool& HasColor, bool& HasDepth);
};