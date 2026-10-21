// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOOpenXRHandInteraction.h"
#include "IPICOOpenXRHandInteractionModule.h"

#include "IXRTrackingSystem.h"
#include "CoreMinimal.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ObjectMacros.h"
#include "Engine/Engine.h"
#include "OpenXRCore.h"
#include "Framework/Application/SlateApplication.h"
#include "IOpenXRExtensionPlugin.h"
#include "Modules/ModuleManager.h"
#include "Features/IModularFeatures.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IOpenXRHMDModule.h"
#include "IOpenXRHMD.h"
#include "IOpenXRExtensionPluginDelegates.h"

#include "GameFramework/InputSettings.h"
#include "EnhancedInputLibrary.h"
#include "EnhancedInputSubsystemInterface.h"
#include "EnhancedInputModule.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "PlayerMappableInputConfig.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "EnhancedInputDeveloperSettings.h"
#include "PICOOpenXRRuntimeSettings.h"

#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Editor.h"
#include "EnhancedInputEditorSubsystem.h"
#endif

#define LOCTEXT_NAMESPACE "PICOOpenXRHandInteraction"

DEFINE_LOG_CATEGORY_STATIC(LogPICOOpenXRHandInteraction, Display, All);

const FKey PICOHI_Left_Pinch_Axis("PICOHI_Left_Pinch_Axis");
const FKey PICOHI_Left_Pinch_Click("PICOHI_Left_Pinch_Click");
const FKey PICOHI_Left_Aim_Axis("PICOHI_Left_Aim_Axis");
const FKey PICOHI_Left_Aim_Click("PICOHI_Left_Aim_Click");
const FKey PICOHI_Left_Grasp_Axis("PICOHI_Left_Grasp_Axis");
const FKey PICOHI_Left_Grasp_Click("PICOHI_Left_Grasp_Click");

const FKey PICOHI_Right_Pinch_Axis("PICOHI_Right_Pinch_Axis");
const FKey PICOHI_Right_Pinch_Click("PICOHI_Right_Pinch_Click");
const FKey PICOHI_Right_Aim_Axis("PICOHI_Right_Aim_Axis");
const FKey PICOHI_Right_Aim_Click("PICOHI_Right_Aim_Click");
const FKey PICOHI_Right_Grasp_Axis("PICOHI_Right_Grasp_Axis");
const FKey PICOHI_Right_Grasp_Click("PICOHI_Right_Grasp_Click");

class FPICOOpenXRHandInteractionModule :
	public IPICOOpenXRHandInteractionModule
{
public:
	FPICOOpenXRHandInteractionModule()
		: InputDevice(nullptr)
	{}

	virtual void StartupModule() override
	{
		IPICOOpenXRHandInteractionModule::StartupModule();

		// HACK: Generic Application might not be instantiated at this point so we create the input device with a
		// dummy message handler. When the Generic Application creates the input device it passes a valid message
		// handler to it which is further on used for all the controller events. This hack fixes issues caused by
		// using a custom input device before the Generic Application has instantiated it. Eg. within BeginPlay()
		//
		// This also fixes the warnings that pop up on the custom input keys when the blueprint loads. Those
		// warnings are caused because Unreal loads the bluerints before the input device has been instantiated
		// and has added its keys, thus leading Unreal to believe that those keys don't exist. This hack causes
		// an earlier instantiation of the input device, and consequently, the custom keys.
		TSharedPtr<FGenericApplicationMessageHandler> DummyMessageHandler(new FGenericApplicationMessageHandler());
		CreateInputDevice(DummyMessageHandler.ToSharedRef());
	}

	virtual void ShutdownModule() override
	{
		IPICOOpenXRHandInteractionModule::ShutdownModule();
	}

	virtual TSharedPtr<class IInputDevice> CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) override
	{
		if (!InputDevice.IsValid())
		{
			TSharedPtr<FPICOOpenXRHandInteraction> ViveTrackingInputDevice(new FPICOOpenXRHandInteraction(InMessageHandler));
			InputDevice = ViveTrackingInputDevice;

			return InputDevice;
		}
		else
		{
			InputDevice.Get()->SetMessageHandler(InMessageHandler);
			return InputDevice;
		}
		return nullptr;
	}

	virtual TSharedPtr<IInputDevice> GetInputDevice() override
	{
		if (!InputDevice.IsValid())
		{
			CreateInputDevice(FSlateApplication::Get().GetPlatformApplication()->GetMessageHandler());
		}
		return InputDevice;
	}

private:
	TSharedPtr<FPICOOpenXRHandInteraction> InputDevice;
};

IMPLEMENT_MODULE(FPICOOpenXRHandInteractionModule, PICOOpenXRHandInteraction);

FPICOOpenXRHandInteraction::FHandInteraction::FHandInteraction(XrActionSet InActionSet, FOpenXRPath InRolePath, const char* InName)
	: HandInteractionActionSet(InActionSet)
	, SubPath(InRolePath)
	, PoseAction(XR_NULL_HANDLE)
	, DeviceId(-1)
{
	TArray<XrPath> SubPaths;
	SubPaths.Add(SubPath);
	XrActionCreateInfo Info;
	Info.type = XR_TYPE_ACTION_CREATE_INFO;
	Info.next = nullptr;
	Info.countSubactionPaths = SubPaths.Num();
	Info.subactionPaths = SubPaths.GetData();
	Info.actionType = XR_ACTION_TYPE_POSE_INPUT;

	FCStringAnsi::Strncpy(Info.localizedActionName, InName, XR_MAX_ACTION_NAME_SIZE);
	FCStringAnsi::StrncatTruncateDest(Info.localizedActionName, XR_MAX_ACTION_NAME_SIZE, " Pose");
	FilterActionName(Info.localizedActionName, Info.actionName);
	XR_ENSURE(xrCreateAction(HandInteractionActionSet, &Info, &PoseAction));
}

void FPICOOpenXRHandInteraction::FHandInteraction::AddTrackedDevices(IOpenXRHMD* HMD)
{
	if (HMD)
	{
		DeviceId = HMD->AddTrackedDevice(PoseAction, SubPath);
	}
}

void FPICOOpenXRHandInteraction::FHandInteraction::GetSuggestedBindings(EHandController Type, TArray<XrActionSuggestedBinding>& OutSuggestedBindings)
{
	switch (Type)
	{
	case EHandController::LeftGrip:
	case EHandController::RightGrip:
		OutSuggestedBindings.Add(XrActionSuggestedBinding{ PoseAction, SubPath / FString("input/grip/pose") });
		break;
	case EHandController::LeftAim:
	case EHandController::RightAim:
		OutSuggestedBindings.Add(XrActionSuggestedBinding{ PoseAction, SubPath / FString("input/aim/pose") });
		break;
	case EHandController::LeftPinch:
	case EHandController::RightPinch:
		OutSuggestedBindings.Add(XrActionSuggestedBinding{ PoseAction, SubPath / FString("input/pinch_ext/pose") });
		break;
	case EHandController::LeftPoke:
	case EHandController::RightPoke:
		OutSuggestedBindings.Add(XrActionSuggestedBinding{ PoseAction, SubPath / FString("input/poke_ext/pose") });
		break;
	default:
		break;
	}
}

FPICOOpenXRHandInteraction::FPICOOpenXRHandInteraction(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
	: MessageHandler(InMessageHandler)
	, DeviceIndex(0)
	, HandInteractionActionSet(XR_NULL_HANDLE)
	, HandInteractions()
	, InputMappingContextToPriorityMap()
	, EnhancedActions()
	, SubactionPaths()
{
	// Register modular feature manually
	IModularFeatures::Get().RegisterModularFeature(IMotionController::GetModularFeatureName(), static_cast<IMotionController*>(this));
	IModularFeatures::Get().RegisterModularFeature(IOpenXRExtensionPlugin::GetModularFeatureName(), static_cast<IOpenXRExtensionPlugin*>(this));
	AddKeys();

	// We're implicitly requiring that the OpenXRPlugin has been loaded and
	// initialized at this point.
	if (!IOpenXRHMDModule::Get().IsAvailable())
	{
		UE_LOG(LogPICOOpenXRHandInteraction, Error, TEXT("Error - OpenXRHMDPlugin isn't available"));
	}

	MotionSourceToEControllerHandMap.Add(TEXT("LeftAim"), EHandController::LeftAim);
	MotionSourceToEControllerHandMap.Add(TEXT("LeftGrip"), EHandController::LeftGrip);
	MotionSourceToEControllerHandMap.Add(TEXT("LeftPinch"), EHandController::LeftPinch);
	MotionSourceToEControllerHandMap.Add(TEXT("LeftPoke"), EHandController::LeftPoke);
	MotionSourceToEControllerHandMap.Add(TEXT("RightAim"), EHandController::RightAim);
	MotionSourceToEControllerHandMap.Add(TEXT("RightGrip"), EHandController::RightGrip);
	MotionSourceToEControllerHandMap.Add(TEXT("RightPoke"), EHandController::RightPoke);
	MotionSourceToEControllerHandMap.Add(TEXT("RightPinch"), EHandController::RightPinch);
}

FPICOOpenXRHandInteraction::~FPICOOpenXRHandInteraction()
{
	// Unregister modular feature manually
	IModularFeatures::Get().UnregisterModularFeature(IMotionController::GetModularFeatureName(), static_cast<IMotionController*>(this));
	IModularFeatures::Get().UnregisterModularFeature(IOpenXRExtensionPlugin::GetModularFeatureName(), static_cast<IOpenXRExtensionPlugin*>(this));
}

bool FPICOOpenXRHandInteraction::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (UPICOOpenXRRuntimeSettings::GetBoolConfigByKey(TEXT("bIsHandTrackingUsed")) && UPICOOpenXRRuntimeSettings::GetBoolConfigByKey(TEXT("bEnableHandInteractionEXT")))
	{
		OutExtensions.Add(XR_EXT_HAND_INTERACTION_EXTENSION_NAME);
		return true;
	}
	return false;
}

void FPICOOpenXRHandInteraction::PostCreateInstance(XrInstance InInstance)
{
	Instance = InInstance;
}

const void* FPICOOpenXRHandInteraction::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	UPICOOpenXRRuntimeSettings* Settings = GetMutableDefault<UPICOOpenXRRuntimeSettings>();
	if (Settings)
	{
		bSupportHandInteractionLeftRight = Settings->bIsHandTrackingUsed && Settings->bEnableHandInteractionEXT && Settings->bSupportHandInteractionLeftRight;
	}
	return InNext;
}

void FPICOOpenXRHandInteraction::OnDestroySession(XrSession InSession)
{
	HandInteractions.Reset();
	EnhancedActions.Reset();
	SubactionPaths.Reset();
	bActionsAttached = false;
}

void FPICOOpenXRHandInteraction::AttachActionSets(TSet<XrActionSet>& OutActionSets)
{
	check(Instance != XR_NULL_HANDLE);

	if (GEngine && GEngine->XRSystem.IsValid())
	{
		XRTrackingSystem = GEngine->XRSystem.Get();
		OpenXRHMD = XRTrackingSystem->GetIOpenXRHMD();
	}

	if (HandInteractionActionSet)
	{
		XR_ENSURE(xrDestroyActionSet(HandInteractionActionSet));
	}

	SubactionPaths.Add(FOpenXRPath("/user/hand/left"));
	SubactionPaths.Add(FOpenXRPath("/user/hand/right"));

	XrActionSetCreateInfo CreateInfo = { XR_TYPE_ACTION_SET_CREATE_INFO };
	FCStringAnsi::Strncpy(CreateInfo.actionSetName, "handinteractions", XR_MAX_ACTION_SET_NAME_SIZE);
	FCStringAnsi::Strncpy(CreateInfo.localizedActionSetName,
		"Hand Interactions", XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE);
	XR_ENSURE(xrCreateActionSet(Instance, &CreateInfo, &HandInteractionActionSet));

	HandInteractions.Add(EHandController::LeftAim, FHandInteraction(HandInteractionActionSet, SubactionPaths[0], "LeftAim"));
	HandInteractions.Add(EHandController::LeftGrip, FHandInteraction(HandInteractionActionSet, SubactionPaths[0], "LeftGrip"));
	HandInteractions.Add(EHandController::LeftPinch, FHandInteraction(HandInteractionActionSet, SubactionPaths[0], "LeftPinch"));
	HandInteractions.Add(EHandController::LeftPoke, FHandInteraction(HandInteractionActionSet, SubactionPaths[0], "LeftPoke"));
	HandInteractions.Add(EHandController::RightAim, FHandInteraction(HandInteractionActionSet, SubactionPaths[1], "RightAim"));
	HandInteractions.Add(EHandController::RightGrip, FHandInteraction(HandInteractionActionSet, SubactionPaths[1], "RightGrip"));
	HandInteractions.Add(EHandController::RightPinch, FHandInteraction(HandInteractionActionSet, SubactionPaths[1], "RightPinch"));
	HandInteractions.Add(EHandController::RightPoke, FHandInteraction(HandInteractionActionSet, SubactionPaths[1], "RightPoke"));

	TArray<XrActionSuggestedBinding> Bindings;
	for (TPair<EHandController, FHandInteraction> Tracker : HandInteractions)
	{
		Tracker.Value.GetSuggestedBindings(Tracker.Key, Bindings);
	}

	// Attempt to load the default input config from the OpenXR input settings.
	const UEnhancedInputDeveloperSettings* InputSettings = GetDefault<UEnhancedInputDeveloperSettings>();
	if (InputSettings && InputSettings->bEnableDefaultMappingContexts)
	{
		for (const auto& Context : InputSettings->DefaultMappingContexts)
		{
			if (Context.InputMappingContext)
			{
				TStrongObjectPtr<const UInputMappingContext> Obj(Context.InputMappingContext.LoadSynchronous());
				InputMappingContextToPriorityMap.Add(Obj, Context.Priority);
			}
			else
			{
				UE_LOG(LogHMD, Warning, TEXT("Default Mapping Contexts contains an Input Mapping Context set to \"None\", ignoring while building OpenXR actions."));
			}
		}
	}

	if (!InputMappingContextToPriorityMap.IsEmpty() && !bActionsAttached)
	{
		for (const auto& MappingContext : InputMappingContextToPriorityMap)
		{
			TMap<FName, int32> ActionMap;
			for (const FEnhancedActionKeyMapping& Mapping : MappingContext.Key->GetMappings())
			{
				if (!Mapping.Action)
				{
					continue;
				}

				FString InputKey = Mapping.Key.ToString().ToLower();
				XrPath Path = XR_NULL_PATH;

				FString SubRole = InputKey.Contains("left") ? "left" : "right";
				FString Behavior = InputKey.Contains("axis") ? "value" : "ready_ext";

				if (InputKey.Contains("pinch"))
				{
					Path = FOpenXRPath("/user/hand/" + SubRole + "/input/pinch_ext/" + Behavior);
				}
				else if (InputKey.Contains("aim"))
				{
					Path = FOpenXRPath("/user/hand/" + SubRole + "/input/aim_activate_ext/" + Behavior);
				}
				else if (InputKey.Contains("grasp"))
				{
					Path = FOpenXRPath("/user/hand/" + SubRole + "/input/grasp_ext/" + Behavior);
				}

				if (Path == XR_NULL_PATH)
				{
					continue;
				}

				FName ActionName = Mapping.Action->GetFName();
				int32& ActionIndex = ActionMap.FindOrAdd(ActionName, INDEX_NONE);
				if (ActionIndex == INDEX_NONE)
				{
					XrActionType ActionType;
					switch (Mapping.Action->ValueType)
					{
					case EInputActionValueType::Axis1D:

						ActionType = XrActionType::XR_ACTION_TYPE_FLOAT_INPUT;
						break;
					case EInputActionValueType::Boolean:
						ActionType = XrActionType::XR_ACTION_TYPE_BOOLEAN_INPUT;
						break;
					}
					ActionIndex = EnhancedActions.Emplace(HandInteractionActionSet, ActionType, ActionName, Mapping.Action, SubactionPaths);
				}

				XrPath Key = InputKey.Contains("left") ? SubactionPaths[0] : SubactionPaths[1];
				for (UInputTrigger* Trigger : Mapping.Triggers)
				{
					TObjectPtr<UInputTrigger> Ptr = Trigger;
					EnhancedActions[ActionIndex].Triggers.AddUnique(Key, Ptr);
				}
				for (UInputModifier* Modifier : Mapping.Modifiers)
				{
					TObjectPtr<UInputModifier> Ptr = Modifier;
					EnhancedActions[ActionIndex].Modifiers.AddUnique(Key, Ptr);
				}
				Bindings.Add(XrActionSuggestedBinding{ EnhancedActions[ActionIndex].Handle, FOpenXRPath(Path) });
			}
		}
	}

	XrInteractionProfileSuggestedBinding InteractionProfile = { XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	InteractionProfile.interactionProfile = FOpenXRPath("/interaction_profiles/ext/hand_interaction_ext");
	InteractionProfile.countSuggestedBindings = Bindings.Num();
	InteractionProfile.suggestedBindings = Bindings.GetData();
	XR_ENSURE(xrSuggestInteractionProfileBindings(Instance, &InteractionProfile));

	for (TPair<EHandController, FHandInteraction>& Tracker : HandInteractions)
	{
		Tracker.Value.AddTrackedDevices(OpenXRHMD);
	}

	OutActionSets.Add(HandInteractionActionSet);

	bActionsAttached = true;
}

void FPICOOpenXRHandInteraction::GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets)
{
	if (bActionsAttached)
	{
		XrActiveActionSet ActiveTrackerSet{ HandInteractionActionSet, XR_NULL_PATH };
		OutActiveSets.Add(ActiveTrackerSet);
	}
}

bool FPICOOpenXRHandInteraction::GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, float WorldToMetersScale) const
{
	if (!bActionsAttached || XRTrackingSystem == nullptr)
	{
		return false;
	}

	if (MotionSource == "AnyHand")
	{
		return GetControllerOrientationAndPosition(ControllerIndex, "LeftGrip", OutOrientation, OutPosition, WorldToMetersScale)
			|| GetControllerOrientationAndPosition(ControllerIndex, "RightGrip", OutOrientation, OutPosition, WorldToMetersScale);
	}
	else if (bSupportHandInteractionLeftRight && MotionSource == "Left")
	{
		return GetControllerOrientationAndPosition(ControllerIndex, "LeftGrip", OutOrientation, OutPosition, WorldToMetersScale);
	}
	else if (bSupportHandInteractionLeftRight && MotionSource == "Right")
	{
		return GetControllerOrientationAndPosition(ControllerIndex, "RightGrip", OutOrientation, OutPosition, WorldToMetersScale);
	}

	if (GetControllerTrackingStatus(ControllerIndex, MotionSource) == ETrackingStatus::Tracked)
	{
		FQuat Orientation;
		bool Success = XRTrackingSystem->GetCurrentPose(GetDeviceIDForMotionSource(MotionSource), Orientation, OutPosition);
		OutOrientation = FRotator(Orientation);
		return Success;
	}
	return false;
}

bool FPICOOpenXRHandInteraction::GetControllerOrientationAndPosition(const int32 ControllerIndex, const FName MotionSource, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityAsAxisAndLength, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const
{
	// FTimespan initializes to 0 and GetControllerOrientationAndPositionForTime with time 0 will return the latest data.
	FTimespan Time;
	bool OutTimeWasUsed = false;
	return GetControllerOrientationAndPositionForTime(ControllerIndex, MotionSource, Time, OutTimeWasUsed, OutOrientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityAsAxisAndLength, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
}

bool FPICOOpenXRHandInteraction::GetControllerOrientationAndPositionForTime(const int32 ControllerIndex, const FName MotionSource, FTimespan Time, bool& OutTimeWasUsed, FRotator& OutOrientation, FVector& OutPosition, bool& OutbProvidedLinearVelocity, FVector& OutLinearVelocity, bool& OutbProvidedAngularVelocity, FVector& OutAngularVelocityAsAxisAndLength, bool& OutbProvidedLinearAcceleration, FVector& OutLinearAcceleration, float WorldToMetersScale) const
{
	if (MotionSource == "AnyHand")
	{
		return GetControllerOrientationAndPositionForTime(ControllerIndex, "LeftGrip", Time, OutTimeWasUsed, OutOrientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityAsAxisAndLength, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale)
			|| GetControllerOrientationAndPositionForTime(ControllerIndex, "RightGrip", Time, OutTimeWasUsed, OutOrientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityAsAxisAndLength, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
	}
	else if (bSupportHandInteractionLeftRight && MotionSource == "Left")
	{
		return GetControllerOrientationAndPositionForTime(ControllerIndex, "LeftGrip", Time, OutTimeWasUsed, OutOrientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityAsAxisAndLength, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
	}
	else if (bSupportHandInteractionLeftRight && MotionSource == "Right")
	{
		return GetControllerOrientationAndPositionForTime(ControllerIndex, "RightGrip", Time, OutTimeWasUsed, OutOrientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityAsAxisAndLength, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
	}

	if (OpenXRHMD && GetControllerTrackingStatus(ControllerIndex, MotionSource) == ETrackingStatus::Tracked)
	{
		FQuat Orientation;
		bool Success = OpenXRHMD->GetPoseForTime(GetDeviceIDForMotionSource(MotionSource), Time, OutTimeWasUsed, Orientation, OutPosition, OutbProvidedLinearVelocity, OutLinearVelocity, OutbProvidedAngularVelocity, OutAngularVelocityAsAxisAndLength, OutbProvidedLinearAcceleration, OutLinearAcceleration, WorldToMetersScale);
		OutOrientation = FRotator(Orientation);
		return Success;
	}
	return false;
}

ETrackingStatus FPICOOpenXRHandInteraction::GetControllerTrackingStatus(const int32 ControllerIndex, const FName MotionSource) const
{
	if (MotionSource == "AnyHand")
	{
		if (GetControllerTrackingStatus(ControllerIndex, "LeftGrip") == ETrackingStatus::Tracked
			|| GetControllerTrackingStatus(ControllerIndex, "RightGrip") == ETrackingStatus::Tracked)
		{
			return ETrackingStatus::Tracked;
		}
		else
		{
			return ETrackingStatus::NotTracked;
		}
	}
	else if (bSupportHandInteractionLeftRight && MotionSource == "Left")
	{
		return GetControllerTrackingStatus(ControllerIndex, "LeftGrip");
	}
	else if (bSupportHandInteractionLeftRight && MotionSource == "Right")
	{
		return GetControllerTrackingStatus(ControllerIndex, "RightGrip");
	}

	if (!bActionsAttached || OpenXRHMD == nullptr)
	{
		return ETrackingStatus::NotTracked;
	}

	XrSession Session = OpenXRHMD->GetSession();
	if (Session == XR_NULL_HANDLE)
	{
		return ETrackingStatus::NotTracked;
	}

	if (!(ControllerIndex == DeviceIndex && MotionSourceToEControllerHandMap.Contains(MotionSource)))
	{
		return ETrackingStatus::NotTracked;
	}

	const EHandController DeviceHand = MotionSourceToEControllerHandMap.FindChecked(MotionSource);
	const FHandInteraction* Tracker = HandInteractions.Find(DeviceHand);

	if (!Tracker)
	{
		return ETrackingStatus::NotTracked;
	}

	XrActionStateGetInfo Info = { XR_TYPE_ACTION_STATE_GET_INFO };
	Info.action = Tracker->PoseAction;
	Info.subactionPath = XR_NULL_PATH;

	XrActionStatePose State = { XR_TYPE_ACTION_STATE_POSE };
	if (!XR_ENSURE(xrGetActionStatePose(Session, &Info, &State)))
	{
		return ETrackingStatus::NotTracked;
	}
	return State.isActive ? ETrackingStatus::Tracked : ETrackingStatus::NotTracked;
}

FName FPICOOpenXRHandInteraction::GetMotionControllerDeviceTypeName() const
{
	const static FName DefaultName(TEXT("PICOOpenXRHandInteraction"));
	return DefaultName;
}

void FPICOOpenXRHandInteraction::EnumerateSources(TArray<FMotionControllerSource>& SourcesOut) const
{
	check(IsInGameThread());

	TArray<FName> Sources;
	MotionSourceToEControllerHandMap.GenerateKeyArray(Sources);
	SourcesOut.Append(Sources);
}

void FPICOOpenXRHandInteraction::Tick(float DeltaTime)
{
}

void FPICOOpenXRHandInteraction::SendControllerEvents()
{
	if (!bActionsAttached || OpenXRHMD == nullptr)
	{
		return;
	}

	if (!OpenXRHMD->IsFocused())
	{
		return;
	}

	XrSession Session = OpenXRHMD->GetSession();
	for (XrPath Subaction : SubactionPaths)
	{
		XrInteractionProfileState Profile;
		Profile.type = XR_TYPE_INTERACTION_PROFILE_STATE;
		Profile.next = nullptr;
		XR_ENSURE(xrGetCurrentInteractionProfile(Session, Subaction, &Profile));
		TPair<XrPath, XrPath> Key(Profile.interactionProfile, Subaction);
		for (FHandInteractionAction& Action : EnhancedActions)
		{
			const UInputAction* InputAction = Action.Object;

			XrActionStateGetInfo GetInfo;
			GetInfo.type = XR_TYPE_ACTION_STATE_GET_INFO;
			GetInfo.next = nullptr;
			GetInfo.subactionPath = Subaction;
			GetInfo.action = Action.Handle;

			FInputActionValue InputValue;
			switch (Action.Type)
			{
			case XR_ACTION_TYPE_BOOLEAN_INPUT:
			{
				XrActionStateBoolean State;
				State.type = XR_TYPE_ACTION_STATE_BOOLEAN;
				State.next = nullptr;
				XrResult Result = xrGetActionStateBoolean(Session, &GetInfo, &State);
				if (XR_SUCCEEDED(Result))
				{
					InputValue = FInputActionValue(State.isActive ? (bool)State.currentState : false);
				}
				else
				{
					continue;
				}
			}
			break;
			case XR_ACTION_TYPE_FLOAT_INPUT:
			{
				XrActionStateFloat State;
				State.type = XR_TYPE_ACTION_STATE_FLOAT;
				State.next = nullptr;
				XrResult Result = xrGetActionStateFloat(Session, &GetInfo, &State);
				if (XR_SUCCEEDED(Result))
				{
					InputValue = FInputActionValue(State.isActive ? State.currentState : 0.0f);
				}
				else
				{
					continue;
				}
			}
			break;
			default:
				// Other action types are currently unsupported.
				continue;
			}

			TArray<TObjectPtr<UInputTrigger>> Triggers;
			Action.Triggers.MultiFind(Subaction, Triggers, false);
			TArray<TObjectPtr<UInputModifier>> Modifiers;
			Action.Modifiers.MultiFind(Subaction, Modifiers, false);

			auto InjectSubsystemInput = [InputAction, InputValue, Triggers, Modifiers](IEnhancedInputSubsystemInterface* Subsystem)
				{
					if (Subsystem)
					{
						Subsystem->InjectInputForAction(InputAction, InputValue, Modifiers, Triggers);
					}
				};

			IEnhancedInputModule::Get().GetLibrary()->ForEachSubsystem(InjectSubsystemInput);

#if WITH_EDITOR
			if (GEditor)
			{
				// UEnhancedInputLibrary::ForEachSubsystem only enumerates runtime subsystems.
				InjectSubsystemInput(GEditor->GetEditorSubsystem<UEnhancedInputEditorSubsystem>());
			}
#endif
		}
	}
}

void FPICOOpenXRHandInteraction::SetMessageHandler(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FPICOOpenXRHandInteraction::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar)
{
	return false;
}

void FPICOOpenXRHandInteraction::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value)
{
}

void FPICOOpenXRHandInteraction::SetChannelValues(int32 ControllerId, const FForceFeedbackValues& values)
{
}

bool FPICOOpenXRHandInteraction::IsGamepadAttached() const
{
	return false;
}

bool FPICOOpenXRHandInteraction::SetEnhancedInputUserSettings(TObjectPtr<class UEnhancedInputUserSettings> InputSettings)
{
	if (bActionsAttached)
	{
		UE_LOG(LogHMD, Error, TEXT("Attempted to attach a set of enhanced user input settings when actions are already attached for the current session."));

		return false;
	}

	const TSet<TObjectPtr<const UInputMappingContext>>& MappingContexts = InputSettings->GetRegisteredInputMappingContexts();

	for (const auto& Context : MappingContexts)
	{
		InputMappingContextToPriorityMap.Add(TStrongObjectPtr<const UInputMappingContext>(Context), 0);
	}
	return true;
}

bool FPICOOpenXRHandInteraction::AttachInputMappingContexts(const TSet<TObjectPtr<UInputMappingContext>>& MappingContexts)
{
	if (bActionsAttached)
	{
		UE_LOG(LogHMD, Error, TEXT("Attempted to attach input mapping contexts when action sets are already attached for the current session."));

		return false;
	}

	for (const auto& Context : MappingContexts)
	{
		InputMappingContextToPriorityMap.Add(TStrongObjectPtr<UInputMappingContext>(Context), 0);
	}
	return true;
}

void FPICOOpenXRHandInteraction::AddKeys()
{
	EKeys::AddMenuCategoryDisplayInfo("PICOHI", LOCTEXT("PICOHISubCategory", "PICO Hand Interaction Controller"), TEXT("GraphEditor.PadEvent_16x"));

	EKeys::AddKey(FKeyDetails(PICOHI_Left_Pinch_Axis, LOCTEXT("PICOHI_Left_Pinch_Axis", "PICO Hand (L) Pinch Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));
	EKeys::AddKey(FKeyDetails(PICOHI_Left_Pinch_Click, LOCTEXT("PICOHI_Left_Pinch_Click", "PICO Hand (L) Pinch Ready"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));
	EKeys::AddKey(FKeyDetails(PICOHI_Left_Aim_Axis, LOCTEXT("PICOHI_Left_Aim_Axis", "PICO Hand (L) Aim Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));
	EKeys::AddKey(FKeyDetails(PICOHI_Left_Aim_Click, LOCTEXT("PICOHI_Left_Aim_Click", "PICO Hand (L) Aim Ready"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));
	EKeys::AddKey(FKeyDetails(PICOHI_Left_Grasp_Axis, LOCTEXT("PICOHI_Left_Grasp_Axis", "PICO Hand (L) Grasp Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));
	EKeys::AddKey(FKeyDetails(PICOHI_Left_Grasp_Click, LOCTEXT("PICOHI_Left_Grasp_Click", "PICO Hand (L) Grasp Ready"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));

	EKeys::AddKey(FKeyDetails(PICOHI_Right_Pinch_Axis, LOCTEXT("PICOHI_Right_Pinch_Axis", "PICO Hand (R) Pinch Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));
	EKeys::AddKey(FKeyDetails(PICOHI_Right_Pinch_Click, LOCTEXT("PICOHI_Right_Pinch_Click", "PICO Hand (R) Pinch Ready"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));
	EKeys::AddKey(FKeyDetails(PICOHI_Right_Aim_Axis, LOCTEXT("PICOHI_Right_Aim_Axis", "PICO Hand (R) Aim Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));
	EKeys::AddKey(FKeyDetails(PICOHI_Right_Aim_Click, LOCTEXT("PICOHI_Right_Aim_Click", "PICO Hand (R) Aim Ready"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));
	EKeys::AddKey(FKeyDetails(PICOHI_Right_Grasp_Axis, LOCTEXT("PICOHI_Right_Grasp_Axis", "PICO Hand (R) Grasp Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));
	EKeys::AddKey(FKeyDetails(PICOHI_Right_Grasp_Click, LOCTEXT("PICOHI_Right_Grasp_Click", "PICO Hand (R) Grasp Ready"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOHI"));
}

XrAction FPICOOpenXRHandInteraction::GetActionForMotionSource(FName MotionSource) const
{
	const EHandController* Hand = MotionSourceToEControllerHandMap.Find(MotionSource);
	if (Hand == nullptr)
	{
		return XR_NULL_HANDLE;
	}
	const FHandInteraction& Tracker = HandInteractions[*Hand];
	return Tracker.PoseAction;
}

int32 FPICOOpenXRHandInteraction::GetDeviceIDForMotionSource(FName MotionSource) const
{
	const FHandInteraction& Tracker = HandInteractions[MotionSourceToEControllerHandMap.FindChecked(MotionSource)];
	return Tracker.DeviceId;
}

XrPath FPICOOpenXRHandInteraction::GetRolePathForMotionSource(FName MotionSource) const
{
	const FHandInteraction& Tracker = HandInteractions[MotionSourceToEControllerHandMap.FindChecked(MotionSource)];
	return Tracker.SubPath;
}

#undef LOCTEXT_NAMESPACE

FPICOOpenXRHandInteraction::FHandInteractionAction::FHandInteractionAction(XrActionSet InActionSet, XrActionType InActionType, const FName& InName, const TObjectPtr<const UInputAction>& InObject, const TArray<XrPath>& InSubactionPaths, IOpenXRHMD* OpenXRHMD)
	: Set(InActionSet)
	, Type(InActionType)
	, Name(InName)
	, Handle(XR_NULL_HANDLE)
	, Object(InObject)
{
	char ActionName[NAME_SIZE];
	Name.GetPlainANSIString(ActionName);

	XrActionCreateInfo Info;
	Info.type = XR_TYPE_ACTION_CREATE_INFO;
	Info.next = nullptr;
	FilterActionName(ActionName, Info.actionName);
	Info.actionType = Type;
	Info.countSubactionPaths = InSubactionPaths.Num();
	Info.subactionPaths = InSubactionPaths.GetData();
	FCStringAnsi::Strncpy(Info.localizedActionName, ActionName, XR_MAX_LOCALIZED_ACTION_NAME_SIZE);

	XR_ENSURE(xrCreateAction(Set, &Info, &Handle));
}
