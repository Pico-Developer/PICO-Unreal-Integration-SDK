// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/RichCurve.h"
#include "IOpenXRExtensionPlugin.h"
#include "PICO_InputFunctionLibrary.h"
#include "IOpenXRExtensionPluginDelegates.h"
#include "Runtime/Launch/Resources/Version.h"
#include "openxr_pico/private/ext_haptic_parametric.h"

// Structure to track streaming haptic playback state
struct FParametricHapticStreamState
{
	class UHapticFeedbackEffect_Curve* CurveEffect = nullptr;
	/** True when CurveEffect was created internally (NewObject) and is AddToRoot'd by us. */
	bool bOwnsEffect = false;
	float Scale = 1.0f;
	bool bLoop = false;
	int32 CurrentAmplitudeIndex = 0;
	int32 CurrentFrequencyIndex = 0;
	XrTime LastUpdateTime = 0;
	XrTime StreamStartTime = 0;
	bool bIsStreaming = false;
	/** Non-zero: streaming data fully sent, waiting until this time to restart loop (nanoseconds). */
	XrTime LoopRestartTime = 0;
	TArray<struct FRichCurveKey> AllAmplitudeKeys;
	TArray<struct FRichCurveKey> AllFrequencyKeys;
	float MinFrequencyHz = XR_FREQUENCY_UNSPECIFIED;
	float MaxFrequencyHz = XR_FREQUENCY_UNSPECIFIED;
	bool bFromPHF = false;
	int32 MaxPointsPerFrame = 0;

	void Reset();

	bool IsValid() const
	{
		return bIsStreaming;
	}
};

struct FSingleShotHapticLoopState
{
	bool bLoop = false;
	bool bIsActive = false;
	XrTime StartTime = 0;
	XrDuration Duration = 0;
	float MinFrequencyHz = XR_FREQUENCY_UNSPECIFIED;
	float MaxFrequencyHz = XR_FREQUENCY_UNSPECIFIED;
	TArray<XrHapticParametricPointEXT> AmplitudePoints;
	TArray<XrHapticParametricPointEXT> FrequencyPoints;

	void Reset()
	{
		bLoop = false;
		bIsActive = false;
		StartTime = 0;
		Duration = 0;
		MinFrequencyHz = XR_FREQUENCY_UNSPECIFIED;
		MaxFrequencyHz = XR_FREQUENCY_UNSPECIFIED;
		AmplitudePoints.Empty();
		FrequencyPoints.Empty();
	}

	bool IsValid() const
	{
		return bIsActive && bLoop && Duration > 0 && AmplitudePoints.Num() >= 2;
	}
};

class FControllerPICO : public IOpenXRExtensionPlugin
{
public:
	FControllerPICO();
	virtual ~FControllerPICO() override {}

	void Register();
	void Unregister();

	/** IOpenXRExtensionPlugin */
	virtual FString GetDisplayName() override
	{
		return FString(TEXT("ControllerPICO"));
	}
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual bool GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual void PostCreateInstance(XrInstance InInstance) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual void PostCreateSession(XrSession InSession) override;
	virtual bool GetInteractionProfiles(XrInstance InInstance, TArray<FString>& OutKeyPrefixes, TArray<XrPath>& OutPaths, TArray<bool>& OutHasHaptics) override;
	virtual bool GetSuggestedBindings(XrPath InInteractionProfile, TArray<XrActionSuggestedBinding>& OutBindings) override;
	virtual void AttachActionSets(TSet<XrActionSet>& OutActionSets) override;
	virtual const void* OnBeginSession(XrSession InSession, const void* InNext) override;
	virtual void OnDestroySession(XrSession InSession) override;
	virtual void GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets) override;
	virtual void PostSyncActions(XrSession InSession) override;
	virtual void OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader) override;

	bool GetControllerBatteryLevel(const EControllerHand Hand, float& Level);
	void AddProfile(XrPath Profile);
	void PlayHapticEffect(class UHapticFeedbackEffect_Base* HapticEffect, EControllerHand Hand, float Scale, bool bLoop, bool bUseTestCurve = false, int32 TestCurveNumKeys = 500, float TestCurveDuration = 5.0f, bool bFromPHF = false, int32 MaxPointsPerFrame = 0);
	void StopHapticEffect(EControllerHand Hand);
	void UpdateHapticStream(XrTime CurrentTime, float DeltaTime);

	bool GetEffectiveFrequencyRange(EControllerHand Hand, bool bFromPHF, float& OutMinHz, float& OutMaxHz) const;

	// On PICO runtimes these properties only become valid after XR_SESSION_STATE_FOCUSED.
	// Refresh them when the session focus state changes so playback uses current values.
	void RefreshParametricHapticsProperties();

private:
	void PlayHapticEffectInternal(class UHapticFeedbackEffect_Curve* CurveEffect, EControllerHand Hand, float Scale, bool bLoop, bool bFromPHF, int32 MaxPointsPerFrame, bool bOwnsEffect = false);
	bool SendHapticFrame(EControllerHand Hand, const TArray<XrHapticParametricPointEXT>& AmplitudePoints,
		const TArray<XrHapticParametricPointEXT>& FrequencyPoints, XrHapticParametricStreamFrameTypeEXT FrameType,
		float MinFreqHz, float MaxFreqHz);

	bool QueryParametricHapticsProperties(XrSession InSession);

	XrSession Session = XR_NULL_HANDLE;
	XrInstance Instance = XR_NULL_HANDLE;
	XrActionsSyncInfo SyncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
	XrAction ControllerBatteryAction = XR_NULL_HANDLE;
	XrActionSet ControllerActionSet = XR_NULL_HANDLE;
	TArray<XrPath> SubactionPaths;
	TArray<XrPath> Profiles;
	TArray<XrActionStateFloat> ActionStateFloats;
	bool bSupportPICOUltra = false;

	TSharedPtr<struct FActiveHapticFeedbackEffect> ActiveHapticEffect_Left;
	TSharedPtr<struct FActiveHapticFeedbackEffect> ActiveHapticEffect_Right;
	XrAction RightVibrationAction = XR_NULL_HANDLE;
	XrAction LeftVibrationAction = XR_NULL_HANDLE;

	// XR_EXT_haptic_parametric support
	bool bParametricHapticsExtensionEnabled = false;
	PFN_xrHapticParametricGetPropertiesEXT xrHapticParametricGetPropertiesEXT = nullptr;
	bool bLeftHandSupportsParametricHaptics = false;
	bool bRightHandSupportsParametricHaptics = false;
	XrHapticParametricPropertiesEXT LeftHapticProperties;
	XrHapticParametricPropertiesEXT RightHapticProperties;
	// Streaming state for each hand
	FParametricHapticStreamState LeftStreamState;
	FParametricHapticStreamState RightStreamState;
	FSingleShotHapticLoopState LeftSingleShotLoopState;
	FSingleShotHapticLoopState RightSingleShotLoopState;

	// Time tracking for haptic stream updates
	double LastHapticUpdateTimeSeconds = 0.0;
};
