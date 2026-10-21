// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_InputFunctionLibrary.h"
#include "PICO_InputModule.h"
#include "PICO_HMDFunctionLibrary.h"
#include "PICOHapticAsset.h"
#include "Haptics/HapticFeedbackEffect_Curve.h"

bool UInputFunctionLibraryPICO::GetControllerBatteryLevelPICO(const EControllerHand Hand, float& Level)
{
	Level = 0.0f;
	bool bValid = false;
	bool bCharging = false;
	bool bPluggedIn = false;
	bool bNoBattery = false;

	// Prefer the newer EXT-based battery query first so legacy applications using this
	// controller-only API can still receive controller battery data on newer devices.
	// Fall back to the original controller battery query for older devices that only
	// support the legacy path.
	if (UHMDFunctionLibraryPICO::GetBatteryStateDisplayPICO(Hand, bValid, Level, bCharging, bPluggedIn, bNoBattery))
	{
		return bValid;
	}

	return FPICOOpenXRInputModule::Get().GetController().GetControllerBatteryLevel(Hand, Level);
}

void UInputFunctionLibraryPICO::PlayHapticEffectPICO(UHapticFeedbackEffect_Base* HapticEffect, EControllerHand Hand, float Scale, bool bLoop, bool bUseTestCurve, int32 TestCurveNumKeys, float TestCurveDuration, int32 MaxPointsPerFrame)
{
	FPICOOpenXRInputModule::Get().GetController().PlayHapticEffect(HapticEffect, Hand, Scale, bLoop, bUseTestCurve, TestCurveNumKeys, TestCurveDuration, false, MaxPointsPerFrame);
}

void UInputFunctionLibraryPICO::StopHapticEffectPICO(EControllerHand Hand)
{
	FPICOOpenXRInputModule::Get().GetController().StopHapticEffect(Hand);
}

void UInputFunctionLibraryPICO::PlayPHFHapticAssetPICO(UPICOHapticAsset* PHFAsset, EControllerHand Hand, float Scale, bool bLoop, int32 MaxPointsPerFrame)
{
	if (!PHFAsset)
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("PlayPHFHapticAsset: PHFAsset is null"));
		return;
	}

	const TArray<FPICOHapticFrameData>& PatternData = (Hand == EControllerHand::Left) ? 
		PHFAsset->PatternData_L : PHFAsset->PatternData_R;

	if (PatternData.Num() == 0)
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("PlayPHFHapticAsset: No pattern data for %s hand"),
			Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));
		return;
	}

	UHapticFeedbackEffect_Curve* CurveEffect = NewObject<UHapticFeedbackEffect_Curve>();
	FRichCurve* AmplitudeCurve = CurveEffect->HapticDetails.Amplitude.GetRichCurve();
	FRichCurve* FrequencyCurve = CurveEffect->HapticDetails.Frequency.GetRichCurve();

	AmplitudeCurve->Reset();
	FrequencyCurve->Reset();

	const float FrameDurationSeconds = PHFAsset->FrameDuration / 1000.0f;
	int32 ActiveFrameCount = 0;
	
	float MinFreqHz, MaxFreqHz;
	if (!FPICOOpenXRInputModule::Get().GetController().GetEffectiveFrequencyRange(Hand, true, MinFreqHz, MaxFreqHz))
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("PlayPHFHapticAsset: Failed to get frequency range for %s hand"),
			Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));
		return;
	}
	float FreqRange = MaxFreqHz - MinFreqHz;

	for (int32 i = 0; i < PatternData.Num(); ++i)
	{
		const FPICOHapticFrameData& Frame = PatternData[i];
		float TimeSeconds = (Frame.FrameSeq - 1) * FrameDurationSeconds;
		
		if (Frame.Play > 0)
		{

			AmplitudeCurve->AddKey(TimeSeconds, Frame.Gain * Scale);

			if (Frame.Frequency > 0.0f)
			{
				// Normalize using complete formula: (freq - min) / (max - min)
				float NormalizedFreq = 0.5f;
				if (FreqRange > 0.0f)
				{
					NormalizedFreq = (Frame.Frequency - MinFreqHz) / FreqRange;
					NormalizedFreq = FMath::Clamp(NormalizedFreq, 0.0f, 1.0f);
				}
				FrequencyCurve->AddKey(TimeSeconds, NormalizedFreq);
			}
			else
			{
				FrequencyCurve->AddKey(TimeSeconds, 0.5f);
			}
			
			ActiveFrameCount++;
		}
		else
		{
			AmplitudeCurve->AddKey(TimeSeconds, 0.0f);
			FrequencyCurve->AddKey(TimeSeconds, 0.5f);
		}
	}

	if (ActiveFrameCount == 0)
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("PlayPHFHapticAsset: No active frames (all Play=0) for %s hand"),
			Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));
		return;
	}

	FPICOOpenXRInputModule::Get().GetController().PlayHapticEffect(CurveEffect, Hand, 1.0f, bLoop, false, 0, 0.0f, true, MaxPointsPerFrame);

	UE_LOG(LogPICOOpenXRInput, Log, TEXT("PlayPHFHapticAsset: Playing PHF asset on %s hand (Version: %s, ActiveFrames: %d/%d, Duration: %.2fs, FreqRange: %.1f-%.1f Hz)"),
		Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"),
		*PHFAsset->PHFVersion,
		ActiveFrameCount,
		PatternData.Num(),
		PHFAsset->GetTotalDuration(),
		MinFreqHz,
		MaxFreqHz);
}
