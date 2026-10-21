// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PICO_HandTrackingFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PICOOPENXRHANDTRACKING_API UHandTrackingFunctionLibraryPICO : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable, Category = "PICO|HandTracking")
	  static bool StartHandTrackingPICO();

	UFUNCTION(BlueprintCallable, Category = "PICO|HandTracking")
	static void StopHandTrackingPICO();

	UFUNCTION(BlueprintCallable, Category = "PICO|HandTracking")
	static bool UpdateHandTrackingDataPICO();

	UFUNCTION(BlueprintCallable, Category = "PICO|HandTracking")
	static bool IsHandTrackingSupportPICO();

	UFUNCTION(BlueprintCallable, Category = "PICO|HandTracking")
	static bool IsHandTrackingRunningPICO();

	UFUNCTION(BlueprintCallable, Category = "PICO|HandTracking")
	static bool GetHandTrackingDataPICO(EControllerHand Hand, TArray<FVector>& OutPositions, TArray<FQuat>& OutRotations, TArray<float>& OutRadii, TArray<FVector>& LinearVelocity, TArray<FVector>& AngularVelocity, float& Scale);

	UFUNCTION(BlueprintCallable, Category = "PICO|HandTracking")
	static bool GetHandTrackingMeshScalePICO(EControllerHand Hand, float& Scale);
};
