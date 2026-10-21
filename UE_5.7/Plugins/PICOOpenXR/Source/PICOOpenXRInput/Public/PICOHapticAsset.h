// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PICOHapticAsset.generated.h"

USTRUCT(BlueprintType)
struct FPICOHapticFrameData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Haptic")
	int32 FrameSeq = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Haptic")
	int32 Play = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Haptic")
	float Frequency = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Haptic")
	int32 Loop = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Haptic")
	float Gain = 0.0f;
};

/**
 * PICO Haptic Asset that stores PHF (PICO Haptic Format) data
 */
UCLASS(BlueprintType)
class PICOOPENXRINPUT_API UPICOHapticAsset : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Haptic")
	FString PHFVersion;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Haptic")
	int32 FrameDuration = 20;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Haptic")
	TArray<FPICOHapticFrameData> PatternData_L;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO Haptic")
	TArray<FPICOHapticFrameData> PatternData_R;

	UFUNCTION(BlueprintCallable, Category = "PICO Haptic")
	float GetTotalDuration() const
	{
		int32 MaxFrames = FMath::Max(PatternData_L.Num(), PatternData_R.Num());
		return (MaxFrames * FrameDuration) / 1000.0f;
	}

	// Get frame data for specific hand at specific time
	UFUNCTION(BlueprintCallable, Category = "PICO Haptic")
	bool GetFrameDataAtTime(float TimeSeconds, bool bLeftHand, FPICOHapticFrameData& OutFrameData) const
	{
		int32 FrameIndex = FMath::FloorToInt(TimeSeconds * 1000.0f / FrameDuration);
		const TArray<FPICOHapticFrameData>& PatternData = bLeftHand ? PatternData_L : PatternData_R;

		if (FrameIndex >= 0 && FrameIndex < PatternData.Num())
		{
			OutFrameData = PatternData[FrameIndex];
			return true;
		}
		return false;
	}
};
