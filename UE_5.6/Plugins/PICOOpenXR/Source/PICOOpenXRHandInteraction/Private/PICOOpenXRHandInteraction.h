// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/IInputInterface.h"
#include "HeadMountedDisplayTypes.h"
#include "XRMotionControllerBase.h"
#include "InputCoreTypes.h"
#include "IInputDevice.h"

#include "IOpenXRExtensionPlugin.h"
#include "OpenXRCore.h"

class UInputAction;
class UInputTrigger;
class UInputModifier;

UENUM(BlueprintType)
enum class EHandController : uint8
{
	LeftGrip,
	LeftAim,
	LeftPinch,
	LeftPoke,
	RightAim,
	RightGrip,
	RightPinch,
	RightPoke
};

class FPICOOpenXRHandInteraction :
	public IOpenXRExtensionPlugin,
	public IInputDevice,
	public FXRMotionControllerBase
{
public:
	struct FHandInteraction
	{
		XrActionSet		HandInteractionActionSet;
		FOpenXRPath		SubPath;
		XrAction		PoseAction;
		int32			DeviceId;

		FHandInteraction(XrActionSet InActionSet, FOpenXRPath InRolePath, const char* InName);

		void AddTrackedDevices(class IOpenXRHMD* HMD);
		void GetSuggestedBindings(EHandController Type, TArray<XrActionSuggestedBinding>& OutSuggestedBindings);
	};

	struct FHandInteractionAction
	{
		XrActionSet		Set;
		XrActionType	Type;
		FName			Name;
		XrAction		Handle;

		// Enhanced Input
		TObjectPtr<const UInputAction> Object;
		TMultiMap<XrPath, TObjectPtr<UInputTrigger>> Triggers;
		TMultiMap<XrPath, TObjectPtr<UInputModifier>> Modifiers;

		FHandInteractionAction(XrActionSet InActionSet,
			XrActionType InActionType,
			const FName& InName,
			const TObjectPtr<const UInputAction>& InObject,
			const TArray<XrPath>& SubactionPaths,
			IOpenXRHMD* OpenXRHMD = nullptr);
	};

public:
	FPICOOpenXRHandInteraction(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler);
	virtual ~FPICOOpenXRHandInteraction() override;

	/** IOpenXRExtensionPlugin */
	virtual FString GetDisplayName() override
	{
		return FString(TEXT("PICOOpenXRHandInteraction"));
	}
	virtual bool GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual void PostCreateInstance(XrInstance InInstance) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual void OnDestroySession(XrSession InSession) override;
	virtual void AttachActionSets(TSet<XrActionSet>& OutActionSets) override;
	virtual void GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets) override;

	/** IMotionController interface */
	virtual bool GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const override;
	virtual bool GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityAsAxisAndLength, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const override;
	virtual bool GetControllerOrientationAndPositionForTime(const int32 ControllerIndex, const FName MotionSource, FTimespan Time, bool& OutTimeWasUsed, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityAsAxisAndLength, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const override;
	virtual ETrackingStatus GetControllerTrackingStatus(const int32 ControllerIndex, const FName MotionSource) const override;
	virtual FName GetMotionControllerDeviceTypeName() const override;
	virtual void EnumerateSources(TArray<FMotionControllerSource>& SourcesOut) const override;

	/** IInputDevice interface */
	virtual void Tick(float DeltaTime) override;
	virtual void SendControllerEvents() override;
	virtual void SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override;
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) override;
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values) override;
	virtual bool SupportsForceFeedback(int32 ControllerId) override { return false; }
	virtual bool IsGamepadAttached() const override;
	virtual bool SetEnhancedInputUserSettings(TObjectPtr<class UEnhancedInputUserSettings> InputSettings) override;
	virtual bool AttachInputMappingContexts(const TSet<TObjectPtr<UInputMappingContext>>& MappingContexts) override;

private:
	void AddKeys();

	XrAction GetActionForMotionSource(FName MotionSource) const;
	int32 GetDeviceIDForMotionSource(FName MotionSource) const;
	XrPath GetRolePathForMotionSource(FName MotionSource) const;

	bool bActionsAttached = false;
	bool bSupportHandInteractionLeftRight = false;

	class IXRTrackingSystem* XRTrackingSystem = nullptr;
	class IOpenXRHMD* OpenXRHMD = nullptr;
	XrInstance Instance = XR_NULL_HANDLE;

	TSharedPtr<FGenericApplicationMessageHandler> MessageHandler;
	int32 DeviceIndex;

	XrActionSet HandInteractionActionSet;
	TMap<EHandController, FHandInteraction> HandInteractions;

	TMap<FName, EHandController> MotionSourceToEControllerHandMap;

	TMap<TStrongObjectPtr<const UInputMappingContext>, uint32> InputMappingContextToPriorityMap;
	TArray<FHandInteractionAction> EnhancedActions;

	TArray<XrPath> SubactionPaths;
};
