// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_Controller.h"
#include "OpenXRCore.h"
#include "PICO_InputModule.h"
#include "IOpenXRHMDModule.h"
#include <openxr_pico/private/ext_haptic_parametric.h>
#include "Haptics/HapticFeedbackEffect_Base.h"
#include "Framework/Application/SlateApplication.h"
#include "Haptics/HapticFeedbackEffect_SoundWave.h"
#include "Haptics/HapticFeedbackEffect_Curve.h"
#include "Haptics/HapticFeedbackEffect_Buffer.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Curves/CurveFloat.h"

#if PLATFORM_ANDROID
#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#endif

#define LOCTEXT_NAMESPACE "FControllerPICO"

// Left
const FKey PICOG3_Left_Menu_Click("PICOG3_Left_Menu_Click");
const FKey PICOG3_Left_Trigger_Click("PICOG3_Left_Trigger_Click");
const FKey PICOG3_Left_Trigger_Axis("PICOG3_Left_Trigger_Axis");
const FKey PICOG3_Left_Thumbstick_2D("PICOG3_Left_Thumbstick_2D");
const FKey PICOG3_Left_Thumbstick_X("PICOG3_Left_Thumbstick_X");
const FKey PICOG3_Left_Thumbstick_Y("PICOG3_Left_Thumbstick_Y");
const FKey PICOG3_Left_Thumbstick_Click("PICOG3_Left_Thumbstick_Click");
//Right
const FKey PICOG3_Right_Menu_Click("PICOG3_Right_Menu_Click");
const FKey PICOG3_Right_Trigger_Click("PICOG3_Right_Trigger_Click");
const FKey PICOG3_Right_Trigger_Axis("PICOG3_Right_Trigger_Axis");
const FKey PICOG3_Right_Thumbstick_2D("PICOG3_Right_Thumbstick_2D");
const FKey PICOG3_Right_Thumbstick_X("PICOG3_Right_Thumbstick_X");
const FKey PICOG3_Right_Thumbstick_Y("PICOG3_Right_Thumbstick_Y");
const FKey PICOG3_Right_Thumbstick_Click("PICOG3_Right_Thumbstick_Click");

// Left
const FKey PICONeo3_Left_X_Click("PICONeo3_Left_X_Click");
const FKey PICONeo3_Left_Y_Click("PICONeo3_Left_Y_Click");
const FKey PICONeo3_Left_X_Touch("PICONeo3_Left_X_Touch");
const FKey PICONeo3_Left_Y_Touch("PICONeo3_Left_Y_Touch");
const FKey PICONeo3_Left_Menu_Click("PICONeo3_Left_Menu_Click");
const FKey PICONeo3_Left_Grip_Click("PICONeo3_Left_Grip_Click");
const FKey PICONeo3_Left_Grip_Axis("PICONeo3_Left_Grip_Axis");
const FKey PICONeo3_Left_Trigger_Click("PICONeo3_Left_Trigger_Click");
const FKey PICONeo3_Left_Trigger_Axis("PICONeo3_Left_Trigger_Axis");
const FKey PICONeo3_Left_Trigger_Touch("PICONeo3_Left_Trigger_Touch");
const FKey PICONeo3_Left_Thumbstick_2D("PICONeo3_Left_Thumbstick_2D");
const FKey PICONeo3_Left_Thumbstick_X("PICONeo3_Left_Thumbstick_X");
const FKey PICONeo3_Left_Thumbstick_Y("PICONeo3_Left_Thumbstick_Y");
const FKey PICONeo3_Left_Thumbstick_Click("PICONeo3_Left_Thumbstick_Click");
const FKey PICONeo3_Left_Thumbstick_Touch("PICONeo3_Left_Thumbstick_Touch");
//Right
const FKey PICONeo3_Right_A_Click("PICONeo3_Right_A_Click");
const FKey PICONeo3_Right_B_Click("PICONeo3_Right_B_Click");
const FKey PICONeo3_Right_A_Touch("PICONeo3_Right_A_Touch");
const FKey PICONeo3_Right_B_Touch("PICONeo3_Right_B_Touch");
const FKey PICONeo3_Right_Menu_Click("PICONeo3_Right_Menu_Click");
const FKey PICONeo3_Right_Grip_Click("PICONeo3_Right_Grip_Click");
const FKey PICONeo3_Right_Grip_Axis("PICONeo3_Right_Grip_Axis");
const FKey PICONeo3_Right_Trigger_Click("PICONeo3_Right_Trigger_Click");
const FKey PICONeo3_Right_Trigger_Axis("PICONeo3_Right_Trigger_Axis");
const FKey PICONeo3_Right_Trigger_Touch("PICONeo3_Right_Trigger_Touch");
const FKey PICONeo3_Right_Thumbstick_2D("PICONeo3_Right_Thumbstick_2D");
const FKey PICONeo3_Right_Thumbstick_X("PICONeo3_Right_Thumbstick_X");
const FKey PICONeo3_Right_Thumbstick_Y("PICONeo3_Right_Thumbstick_Y");
const FKey PICONeo3_Right_Thumbstick_Click("PICONeo3_Right_Thumbstick_Click");
const FKey PICONeo3_Right_Thumbstick_Touch("PICONeo3_Right_Thumbstick_Touch");

// Left
const FKey PICO4_Left_X_Click("PICO4_Left_X_Click");
const FKey PICO4_Left_Y_Click("PICO4_Left_Y_Click");
const FKey PICO4_Left_X_Touch("PICO4_Left_X_Touch");
const FKey PICO4_Left_Y_Touch("PICO4_Left_Y_Touch");
const FKey PICO4_Left_Menu_Click("PICO4_Left_Menu_Click");
const FKey PICO4_Left_Grip_Click("PICO4_Left_Grip_Click");
const FKey PICO4_Left_Grip_Axis("PICO4_Left_Grip_Axis");
const FKey PICO4_Left_Trigger_Click("PICO4_Left_Trigger_Click");
const FKey PICO4_Left_Trigger_Axis("PICO4_Left_Trigger_Axis");
const FKey PICO4_Left_Trigger_Touch("PICO4_Left_Trigger_Touch");
const FKey PICO4_Left_Thumbstick_2D("PICO4_Left_Thumbstick_2D");
const FKey PICO4_Left_Thumbstick_X("PICO4_Left_Thumbstick_X");
const FKey PICO4_Left_Thumbstick_Y("PICO4_Left_Thumbstick_Y");
const FKey PICO4_Left_Thumbstick_Click("PICO4_Left_Thumbstick_Click");
const FKey PICO4_Left_Thumbstick_Touch("PICO4_Left_Thumbstick_Touch");
//Right
const FKey PICO4_Right_A_Click("PICO4_Right_A_Click");
const FKey PICO4_Right_B_Click("PICO4_Right_B_Click");
const FKey PICO4_Right_A_Touch("PICO4_Right_A_Touch");
const FKey PICO4_Right_B_Touch("PICO4_Right_B_Touch");
const FKey PICO4_Right_Grip_Click("PICO4_Right_Grip_Click");
const FKey PICO4_Right_Grip_Axis("PICO4_Right_Grip_Axis");
const FKey PICO4_Right_Trigger_Click("PICO4_Right_Trigger_Click");
const FKey PICO4_Right_Trigger_Axis("PICO4_Right_Trigger_Axis");
const FKey PICO4_Right_Trigger_Touch("PICO4_Right_Trigger_Touch");
const FKey PICO4_Right_Thumbstick_2D("PICO4_Right_Thumbstick_2D");
const FKey PICO4_Right_Thumbstick_X("PICO4_Right_Thumbstick_X");
const FKey PICO4_Right_Thumbstick_Y("PICO4_Right_Thumbstick_Y");
const FKey PICO4_Right_Thumbstick_Click("PICO4_Right_Thumbstick_Click");
const FKey PICO4_Right_Thumbstick_Touch("PICO4_Right_Thumbstick_Touch");

// Left
const FKey PICOUltra_Left_X_Click("PICOUltra_Left_X_Click");
const FKey PICOUltra_Left_Y_Click("PICOUltra_Left_Y_Click");
const FKey PICOUltra_Left_X_Touch("PICOUltra_Left_X_Touch");
const FKey PICOUltra_Left_Y_Touch("PICOUltra_Left_Y_Touch");
const FKey PICOUltra_Left_Menu_Click("PICOUltra_Left_Menu_Click");
const FKey PICOUltra_Left_Grip_Click("PICOUltra_Left_Grip_Click");
const FKey PICOUltra_Left_Grip_Axis("PICOUltra_Left_Grip_Axis");
const FKey PICOUltra_Left_Trigger_Click("PICOUltra_Left_Trigger_Click");
const FKey PICOUltra_Left_Trigger_Axis("PICOUltra_Left_Trigger_Axis");
const FKey PICOUltra_Left_Trigger_Touch("PICOUltra_Left_Trigger_Touch");
const FKey PICOUltra_Left_Thumbstick_2D("PICOUltra_Left_Thumbstick_2D");
const FKey PICOUltra_Left_Thumbstick_X("PICOUltra_Left_Thumbstick_X");
const FKey PICOUltra_Left_Thumbstick_Y("PICOUltra_Left_Thumbstick_Y");
const FKey PICOUltra_Left_Thumbstick_Click("PICOUltra_Left_Thumbstick_Click");
const FKey PICOUltra_Left_Thumbstick_Touch("PICOUltra_Left_Thumbstick_Touch");
//Right
const FKey PICOUltra_Right_A_Click("PICOUltra_Right_A_Click");
const FKey PICOUltra_Right_B_Click("PICOUltra_Right_B_Click");
const FKey PICOUltra_Right_A_Touch("PICOUltra_Right_A_Touch");
const FKey PICOUltra_Right_B_Touch("PICOUltra_Right_B_Touch");
const FKey PICOUltra_Right_Grip_Click("PICOUltra_Right_Grip_Click");
const FKey PICOUltra_Right_Grip_Axis("PICOUltra_Right_Grip_Axis");
const FKey PICOUltra_Right_Trigger_Click("PICOUltra_Right_Trigger_Click");
const FKey PICOUltra_Right_Trigger_Axis("PICOUltra_Right_Trigger_Axis");
const FKey PICOUltra_Right_Trigger_Touch("PICOUltra_Right_Trigger_Touch");
const FKey PICOUltra_Right_Thumbstick_2D("PICOUltra_Right_Thumbstick_2D");
const FKey PICOUltra_Right_Thumbstick_X("PICOUltra_Right_Thumbstick_X");
const FKey PICOUltra_Right_Thumbstick_Y("PICOUltra_Right_Thumbstick_Y");
const FKey PICOUltra_Right_Thumbstick_Click("PICOUltra_Right_Thumbstick_Click");
const FKey PICOUltra_Right_Thumbstick_Touch("PICOUltra_Right_Thumbstick_Touch");

FControllerPICO::FControllerPICO()
{
}

void FControllerPICO::Register()
{
	RegisterOpenXRExtensionModularFeature();

	EKeys::AddMenuCategoryDisplayInfo("PICOG3", LOCTEXT("PICOG3SubCategory", "PICO G3 Controller"), TEXT("GraphEditor.PadEvent_16x"));

	// Left
	EKeys::AddKey(FKeyDetails(PICOG3_Left_Menu_Click, LOCTEXT("PICOG3_Left_Menu_Click", "PICO G3 (L) Menu"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));
	EKeys::AddKey(FKeyDetails(PICOG3_Left_Trigger_Click, LOCTEXT("PICOG3_Left_Trigger_Click", "PICO G3 (L) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));
	EKeys::AddKey(FKeyDetails(PICOG3_Left_Trigger_Axis, LOCTEXT("PICOG3_Left_Trigger_Axis", "PICO G3 (L) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));
	EKeys::AddKey(FKeyDetails(PICOG3_Left_Thumbstick_X, LOCTEXT("PICOG3_Left_Thumbstick_X", "PICO G3 (L) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));
	EKeys::AddKey(FKeyDetails(PICOG3_Left_Thumbstick_Y, LOCTEXT("PICOG3_Left_Thumbstick_Y", "PICO G3 (L) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));
	EKeys::AddPairedKey(FKeyDetails(PICOG3_Left_Thumbstick_2D, LOCTEXT("PICOG3_Left_Thumbstick_2D", "PICO G3 (L) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "PICOG3"), PICOG3_Left_Thumbstick_X, PICOG3_Left_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(PICOG3_Left_Thumbstick_Click, LOCTEXT("PICOG3_Left_Thumbstick_Click", "PICO G3 (L) Thumbstick Button"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));

	// Right
	EKeys::AddKey(FKeyDetails(PICOG3_Right_Menu_Click, LOCTEXT("PICOG3_Right_Menu_Click", "PICO G3 (R) Menu"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));
	EKeys::AddKey(FKeyDetails(PICOG3_Right_Trigger_Click, LOCTEXT("PICOG3_Right_Trigger_Click", "PICO G3 (R) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));
	EKeys::AddKey(FKeyDetails(PICOG3_Right_Trigger_Axis, LOCTEXT("PICOG3_Right_Trigger_Axis", "PICO G3 (R) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));
	EKeys::AddKey(FKeyDetails(PICOG3_Right_Thumbstick_X, LOCTEXT("PICOG3_Right_Thumbstick_X", "PICO G3 (R) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));
	EKeys::AddKey(FKeyDetails(PICOG3_Right_Thumbstick_Y, LOCTEXT("PICOG3_Right_Thumbstick_Y", "PICO G3 (R) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));
	EKeys::AddPairedKey(FKeyDetails(PICOG3_Right_Thumbstick_2D, LOCTEXT("PICOG3_Right_Thumbstick_2D", "PICO G3 (R) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "PICOG3"), PICOG3_Right_Thumbstick_X, PICOG3_Right_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(PICOG3_Right_Thumbstick_Click, LOCTEXT("PICOG3_Right_Thumbstick_Click", "PICO G3 (R) Thumbstick Button"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOG3"));

	EKeys::AddMenuCategoryDisplayInfo("PICONeo3", LOCTEXT("PICONeo3SubCategory", "PICO Neo3 Controller"), TEXT("GraphEditor.PadEvent_16x"));

	// Left
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_X_Click, LOCTEXT("PICONeo3_Left_X_Click", "PICO Neo3 (L) X Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Y_Click, LOCTEXT("PICONeo3_Left_Y_Click", "PICO Neo3 (L) Y Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_X_Touch, LOCTEXT("PICONeo3_Left_X_Touch", "PICO Neo3 (L) X Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Y_Touch, LOCTEXT("PICONeo3_Left_Y_Touch", "PICO Neo3 (L) Y Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Menu_Click, LOCTEXT("PICONeo3_Left_Menu_Click", "PICO Neo3 (L) Menu"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Grip_Click, LOCTEXT("PICONeo3_Left_Grip_Click", "PICO Neo3 (L) Grip"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Grip_Axis, LOCTEXT("PICONeo3_Left_Grip_Axis", "PICO Neo3 (L) Grip Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Trigger_Click, LOCTEXT("PICONeo3_Left_Trigger_Click", "PICO Neo3 (L) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Trigger_Axis, LOCTEXT("PICONeo3_Left_Trigger_Axis", "PICO Neo3 (L) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Trigger_Touch, LOCTEXT("PICONeo3_Left_Trigger_Touch", "PICO Neo3 (L) Trigger Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Thumbstick_X, LOCTEXT("PICONeo3_Left_Thumbstick_X", "PICO Neo3 (L) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Thumbstick_Y, LOCTEXT("PICONeo3_Left_Thumbstick_Y", "PICO Neo3 (L) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddPairedKey(FKeyDetails(PICONeo3_Left_Thumbstick_2D, LOCTEXT("PICONeo3_Left_Thumbstick_2D", "PICO Neo3 (L) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"), PICONeo3_Left_Thumbstick_X, PICONeo3_Left_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Thumbstick_Click, LOCTEXT("PICONeo3_Left_Thumbstick_Click", "PICO Neo3 (L) Thumbstick Button"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Left_Thumbstick_Touch, LOCTEXT("PICONeo3_Left_Thumbstick_Touch", "PICO Neo3 (L) Thumbstick Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));

	// Right
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_A_Click, LOCTEXT("PICONeo3_Right_A_Click", "PICO Neo3 (R) A Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_B_Click, LOCTEXT("PICONeo3_Right_B_Click", "PICO Neo3 (R) B Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_A_Touch, LOCTEXT("PICONeo3_Right_A_Touch", "PICO Neo3 (R) A Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_B_Touch, LOCTEXT("PICONeo3_Right_B_Touch", "PICO Neo3 (R) B Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_Menu_Click, LOCTEXT("PICONeo3_Right_Menu_Click", "PICO Neo3 (R) Menu"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_Grip_Click, LOCTEXT("PICONeo3_Right_Grip_Click", "PICO Neo3 (R) Grip"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_Grip_Axis, LOCTEXT("PICONeo3_Right_Grip_Axis", "PICO Neo3 (R) Grip Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_Trigger_Click, LOCTEXT("PICONeo3_Right_Trigger_Click", "PICO Neo3 (R) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_Trigger_Axis, LOCTEXT("PICONeo3_Right_Trigger_Axis", "PICO Neo3 (R) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_Trigger_Touch, LOCTEXT("PICONeo3_Right_Trigger_Touch", "PICO Neo3 (R) Trigger Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_Thumbstick_X, LOCTEXT("PICONeo3_Right_Thumbstick_X", "PICO Neo3 (R) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_Thumbstick_Y, LOCTEXT("PICONeo3_Right_Thumbstick_Y", "PICO Neo3 (R) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddPairedKey(FKeyDetails(PICONeo3_Right_Thumbstick_2D, LOCTEXT("PICONeo3_Right_Thumbstick_2D", "PICO Neo3 (R) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"), PICONeo3_Right_Thumbstick_X, PICONeo3_Right_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_Thumbstick_Click, LOCTEXT("PICONeo3_Right_Thumbstick_Click", "PICO Neo3 (R) Thumbstick Button"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));
	EKeys::AddKey(FKeyDetails(PICONeo3_Right_Thumbstick_Touch, LOCTEXT("PICONeo3_Right_Thumbstick_Touch", "PICO Neo3 (R) Thumbstick Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICONeo3"));

	EKeys::AddMenuCategoryDisplayInfo("PICO4", LOCTEXT("PICO4SubCategory", "PICO 4 Controller"), TEXT("GraphEditor.PadEvent_16x"));

	// Left
	EKeys::AddKey(FKeyDetails(PICO4_Left_X_Click, LOCTEXT("PICO4_Left_X_Click", "PICO 4 (L) X Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_Y_Click, LOCTEXT("PICO4_Left_Y_Click", "PICO 4 (L) Y Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_X_Touch, LOCTEXT("PICO4_Left_X_Touch", "PICO 4 (L) X Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_Y_Touch, LOCTEXT("PICO4_Left_Y_Touch", "PICO 4 (L) Y Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_Menu_Click, LOCTEXT("PICO4_Left_Menu_Click", "PICO 4 (L) Menu"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_Grip_Click, LOCTEXT("PICO4_Left_Grip_Click", "PICO 4 (L) Grip"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_Grip_Axis, LOCTEXT("PICO4_Left_Grip_Axis", "PICO 4 (L) Grip Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_Trigger_Click, LOCTEXT("PICO4_Left_Trigger_Click", "PICO 4 (L) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_Trigger_Axis, LOCTEXT("PICO4_Left_Trigger_Axis", "PICO 4 (L) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_Trigger_Touch, LOCTEXT("PICO4_Left_Trigger_Touch", "PICO 4 (L) Trigger Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_Thumbstick_X, LOCTEXT("PICO4_Left_Thumbstick_X", "PICO 4 (L) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_Thumbstick_Y, LOCTEXT("PICO4_Left_Thumbstick_Y", "PICO 4 (L) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddPairedKey(FKeyDetails(PICO4_Left_Thumbstick_2D, LOCTEXT("PICO4_Left_Thumbstick_2D", "PICO 4 (L) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "PICO4"), PICO4_Left_Thumbstick_X, PICO4_Left_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(PICO4_Left_Thumbstick_Click, LOCTEXT("PICO4_Left_Thumbstick_Click", "PICO 4 (L) Thumbstick Button"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Left_Thumbstick_Touch, LOCTEXT("PICO4_Left_Thumbstick_Touch", "PICO 4 (L) Thumbstick Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));

	// Right
	EKeys::AddKey(FKeyDetails(PICO4_Right_A_Click, LOCTEXT("PICO4_Right_A_Click", "PICO 4 (R) A Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Right_B_Click, LOCTEXT("PICO4_Right_B_Click", "PICO 4 (R) B Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Right_A_Touch, LOCTEXT("PICO4_Right_A_Touch", "PICO 4 (R) A Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Right_B_Touch, LOCTEXT("PICO4_Right_B_Touch", "PICO 4 (R) B Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Right_Grip_Click, LOCTEXT("PICO4_Right_Grip_Click", "PICO 4 (R) Grip"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Right_Grip_Axis, LOCTEXT("PICO4_Right_Grip_Axis", "PICO 4 (R) Grip Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Right_Trigger_Click, LOCTEXT("PICO4_Right_Trigger_Click", "PICO 4 (R) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Right_Trigger_Axis, LOCTEXT("PICO4_Right_Trigger_Axis", "PICO 4 (R) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Right_Trigger_Touch, LOCTEXT("PICO4_Right_Trigger_Touch", "PICO 4 (R) Trigger Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Right_Thumbstick_X, LOCTEXT("PICO4_Right_Thumbstick_X", "PICO 4 (R) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Right_Thumbstick_Y, LOCTEXT("PICO4_Right_Thumbstick_Y", "PICO 4 (R) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddPairedKey(FKeyDetails(PICO4_Right_Thumbstick_2D, LOCTEXT("PICO4_Right_Thumbstick_2D", "PICO 4 (R) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "PICO4"), PICO4_Right_Thumbstick_X, PICO4_Right_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(PICO4_Right_Thumbstick_Click, LOCTEXT("PICO4_Right_Thumbstick_Click", "PICO 4 (R) Thumbstick Button"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));
	EKeys::AddKey(FKeyDetails(PICO4_Right_Thumbstick_Touch, LOCTEXT("PICO4_Right_Thumbstick_Touch", "PICO 4 (R) Thumbstick Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICO4"));

	EKeys::AddMenuCategoryDisplayInfo("PICOUltra", LOCTEXT("PICOUltraSubCategory", "PICO Ultra Controller"), TEXT("GraphEditor.PadEvent_16x"));

	// Left
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_X_Click, LOCTEXT("PICOUltra_Left_X_Click", "PICO Ultra (L) X Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Y_Click, LOCTEXT("PICOUltra_Left_Y_Click", "PICO Ultra (L) Y Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_X_Touch, LOCTEXT("PICOUltra_Left_X_Touch", "PICO Ultra (L) X Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Y_Touch, LOCTEXT("PICOUltra_Left_Y_Touch", "PICO Ultra (L) Y Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Menu_Click, LOCTEXT("PICOUltra_Left_Menu_Click", "PICO Ultra (L) Menu"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Grip_Click, LOCTEXT("PICOUltra_Left_Grip_Click", "PICO Ultra (L) Grip"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Grip_Axis, LOCTEXT("PICOUltra_Left_Grip_Axis", "PICO Ultra (L) Grip Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Trigger_Click, LOCTEXT("PICOUltra_Left_Trigger_Click", "PICO Ultra (L) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Trigger_Axis, LOCTEXT("PICOUltra_Left_Trigger_Axis", "PICO Ultra (L) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Trigger_Touch, LOCTEXT("PICOUltra_Left_Trigger_Touch", "PICO Ultra (L) Trigger Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Thumbstick_X, LOCTEXT("PICOUltra_Left_Thumbstick_X", "PICO Ultra (L) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Thumbstick_Y, LOCTEXT("PICOUltra_Left_Thumbstick_Y", "PICO Ultra (L) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddPairedKey(FKeyDetails(PICOUltra_Left_Thumbstick_2D, LOCTEXT("PICOUltra_Left_Thumbstick_2D", "PICO Ultra (L) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"), PICOUltra_Left_Thumbstick_X, PICOUltra_Left_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Thumbstick_Click, LOCTEXT("PICOUltra_Left_Thumbstick_Click", "PICO Ultra (L) Thumbstick Button"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Left_Thumbstick_Touch, LOCTEXT("PICOUltra_Left_Thumbstick_Touch", "PICO Ultra (L) Thumbstick Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));

	// Right
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_A_Click, LOCTEXT("PICOUltra_Right_A_Click", "PICO Ultra (R) A Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_B_Click, LOCTEXT("PICOUltra_Right_B_Click", "PICO Ultra (R) B Press"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_A_Touch, LOCTEXT("PICOUltra_Right_A_Touch", "PICO Ultra (R) A Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_B_Touch, LOCTEXT("PICOUltra_Right_B_Touch", "PICO Ultra (R) B Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_Grip_Click, LOCTEXT("PICOUltra_Right_Grip_Click", "PICO Ultra (R) Grip"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_Grip_Axis, LOCTEXT("PICOUltra_Right_Grip_Axis", "PICO Ultra (R) Grip Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_Trigger_Click, LOCTEXT("PICOUltra_Right_Trigger_Click", "PICO Ultra (R) Trigger"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_Trigger_Axis, LOCTEXT("PICOUltra_Right_Trigger_Axis", "PICO Ultra (R) Trigger Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_Trigger_Touch, LOCTEXT("PICOUltra_Right_Trigger_Touch", "PICO Ultra (R) Trigger Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_Thumbstick_X, LOCTEXT("PICOUltra_Right_Thumbstick_X", "PICO Ultra (R) Thumbstick X-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_Thumbstick_Y, LOCTEXT("PICOUltra_Right_Thumbstick_Y", "PICO Ultra (R) Thumbstick Y-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis1D | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddPairedKey(FKeyDetails(PICOUltra_Right_Thumbstick_2D, LOCTEXT("PICOUltra_Right_Thumbstick_2D", "PICO Ultra (R) Thumbstick 2D-Axis"), FKeyDetails::GamepadKey | FKeyDetails::Axis2D | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"), PICOUltra_Right_Thumbstick_X, PICOUltra_Right_Thumbstick_Y);
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_Thumbstick_Click, LOCTEXT("PICOUltra_Right_Thumbstick_Click", "PICO Ultra (R) Thumbstick Button"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
	EKeys::AddKey(FKeyDetails(PICOUltra_Right_Thumbstick_Touch, LOCTEXT("PICOUltra_Right_Thumbstick_Touch", "PICO Ultra (R) Thumbstick Touch"), FKeyDetails::GamepadKey | FKeyDetails::NotBlueprintBindableKey, "PICOUltra"));
}

void FControllerPICO::Unregister()
{
	UnregisterOpenXRExtensionModularFeature();
}

bool FControllerPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_BD_CONTROLLER_INTERACTION_EXTENSION_NAME);
	return true;
}

bool FControllerPICO::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_BD_ULTRA_CONTROLLER_INTERACTION_EXTENSION_NAME);
	OutExtensions.Add(XR_EXT_HAPTIC_PARAMETRIC_EXTENSION_NAME);
	return true;
}

void FControllerPICO::PostCreateInstance(XrInstance InInstance)
{
	Instance = InInstance;
	SubactionPaths.Reset();
	XrPath PathLeft;
	XrResult Result;
	Result = xrStringToPath(Instance, "/user/hand/left", &PathLeft);
	if (XR_SUCCEEDED(Result))
	{
		SubactionPaths.Add(PathLeft);
	}

	XrPath PathRight;
	Result = xrStringToPath(Instance, "/user/hand/right", &PathRight);
	if (XR_SUCCEEDED(Result))
	{
		SubactionPaths.Add(PathRight);
	}

	ActionStateFloats.SetNum(SubactionPaths.Num());

	bParametricHapticsExtensionEnabled = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_EXT_HAPTIC_PARAMETRIC_EXTENSION_NAME);
	if (bParametricHapticsExtensionEnabled)
	{
		Result = xrGetInstanceProcAddr(Instance, "xrHapticParametricGetPropertiesEXT",
			(PFN_xrVoidFunction*)(&xrHapticParametricGetPropertiesEXT));
		if (XR_SUCCEEDED(Result) && xrHapticParametricGetPropertiesEXT != nullptr)
		{
			UE_LOG(LogPICOOpenXRInput, Log, TEXT("XR_EXT_haptic_parametric extension loaded successfully"));
		}
		else
		{
			UE_LOG(LogPICOOpenXRInput, Warning, TEXT("Failed to load xrHapticParametricGetPropertiesEXT function"));
			bParametricHapticsExtensionEnabled = false;
		}
	}
	else
	{
		UE_LOG(LogPICOOpenXRInput, Log, TEXT("XR_EXT_haptic_parametric extension is not enabled"));
	}
}

const void* FControllerPICO::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	// Actions and action sets should only be created once per instance lifetime
	// Only create if they haven't been created yet
	if (ControllerActionSet == XR_NULL_HANDLE)
	{
		XrActionSetCreateInfo ActionSetInfo = { XR_TYPE_ACTION_SET_CREATE_INFO };
		FCStringAnsi::Strncpy(ActionSetInfo.actionSetName, "openxrbatteryactionset", XR_MAX_ACTION_SET_NAME_SIZE);
		FCStringAnsi::Strncpy(ActionSetInfo.localizedActionSetName, "OpenXR Battery Action Set", XR_MAX_LOCALIZED_ACTION_SET_NAME_SIZE);
		XR_ENSURE(xrCreateActionSet(Instance, &ActionSetInfo, &ControllerActionSet));

		XrActionCreateInfo ActionInfo = { XR_TYPE_ACTION_CREATE_INFO };
		ActionInfo.countSubactionPaths = SubactionPaths.Num();
		ActionInfo.subactionPaths = SubactionPaths.GetData();
		FCStringAnsi::Strncpy(ActionInfo.actionName, "openxrbatteryaction", XR_MAX_ACTION_NAME_SIZE);
		FCStringAnsi::Strncpy(ActionInfo.localizedActionName, "OpenXR Battery Action", XR_MAX_LOCALIZED_ACTION_NAME_SIZE);
		ActionInfo.actionType = XR_ACTION_TYPE_FLOAT_INPUT;
		XR_ENSURE(xrCreateAction(ControllerActionSet, &ActionInfo, &ControllerBatteryAction));

		// Only create vibration actions if parametric haptics extension is enabled
		if (bParametricHapticsExtensionEnabled)
		{
			ActionInfo = { XR_TYPE_ACTION_CREATE_INFO };
			ActionInfo.countSubactionPaths = SubactionPaths.Num();
			ActionInfo.subactionPaths = SubactionPaths.GetData();
			FCStringAnsi::Strncpy(ActionInfo.actionName, "openxrleftvibrationaction", XR_MAX_ACTION_NAME_SIZE);
			FCStringAnsi::Strncpy(ActionInfo.localizedActionName, "OpenXR Left Vibration Action", XR_MAX_LOCALIZED_ACTION_NAME_SIZE);
			ActionInfo.actionType = XR_ACTION_TYPE_VIBRATION_OUTPUT;
			XR_ENSURE(xrCreateAction(ControllerActionSet, &ActionInfo, &LeftVibrationAction));

			ActionInfo = { XR_TYPE_ACTION_CREATE_INFO };
			ActionInfo.countSubactionPaths = SubactionPaths.Num();
			ActionInfo.subactionPaths = SubactionPaths.GetData();
			FCStringAnsi::Strncpy(ActionInfo.actionName, "openxrrightvibrationaction", XR_MAX_ACTION_NAME_SIZE);
			FCStringAnsi::Strncpy(ActionInfo.localizedActionName, "OpenXR Right Vibration Action", XR_MAX_LOCALIZED_ACTION_NAME_SIZE);
			ActionInfo.actionType = XR_ACTION_TYPE_VIBRATION_OUTPUT;
			XR_ENSURE(xrCreateAction(ControllerActionSet, &ActionInfo, &RightVibrationAction));

			UE_LOG(LogPICOOpenXRInput, Log, TEXT("Created vibration actions for parametric haptics"));
		}
		else
		{
			UE_LOG(LogPICOOpenXRInput, Log, TEXT("Skipping vibration action creation - parametric haptics extension not enabled"));
		}
	}
	else
	{
		UE_LOG(LogPICOOpenXRInput, Log, TEXT("Action set and actions already exist, skipping creation"));
	}

	return InNext;
}

void FControllerPICO::PostCreateSession(XrSession InSession)
{
	Session = InSession;
}

bool FControllerPICO::QueryParametricHapticsProperties(XrSession InSession)
{
	if (!bParametricHapticsExtensionEnabled || xrHapticParametricGetPropertiesEXT == nullptr)
	{
		return false;
	}
	if (InSession == XR_NULL_HANDLE || LeftVibrationAction == XR_NULL_HANDLE || RightVibrationAction == XR_NULL_HANDLE || SubactionPaths.Num() < 2)
	{
		return false;
	}

	auto QueryForHand = [this, InSession](int32 SubactionIndex, XrAction Action, XrHapticParametricPropertiesEXT& OutProps, bool& OutSupports, const TCHAR* HandName)
	{
		XrHapticActionInfo HapticActionInfo{ XR_TYPE_HAPTIC_ACTION_INFO };
		HapticActionInfo.next = nullptr;
		HapticActionInfo.subactionPath = SubactionPaths[SubactionIndex];
		HapticActionInfo.action = Action;

		OutProps.type = XR_TYPE_HAPTIC_PARAMETRIC_PROPERTIES_EXT;
		OutProps.next = nullptr;

		XrResult Result = xrHapticParametricGetPropertiesEXT(InSession, &HapticActionInfo, &OutProps);
		if (XR_SUCCEEDED(Result))
		{
			UE_LOG(LogPICOOpenXRInput, Log, TEXT("%s hand parametric haptic properties: MinFreq=%.1f Hz, MaxFreq=%.1f Hz, IdealRate=%.3f s, MinFirstFrame=%.3f s"),
				HandName,
				OutProps.minFrequencyHz,
				OutProps.maxFrequencyHz,
				OutProps.idealFrameSubmissionRate / 1e9,
				OutProps.minimumFirstFrameDuration / 1e9);
			OutSupports = true;
			return true;
		}

		if (Result == XR_ERROR_FEATURE_UNSUPPORTED)
		{
			UE_LOG(LogPICOOpenXRInput, Warning, TEXT("Parametric haptics not supported on %s hand controller"), HandName);
		}
		else
		{
			UE_LOG(LogPICOOpenXRInput, Warning, TEXT("Failed to query %s hand parametric haptic properties: %s"), HandName, OpenXRResultToString(Result));
		}

		OutSupports = false;
		return false;
	};

	bool bAny = false;
	bAny |= QueryForHand(0, LeftVibrationAction, LeftHapticProperties, bLeftHandSupportsParametricHaptics, TEXT("Left"));
	bAny |= QueryForHand(1, RightVibrationAction, RightHapticProperties, bRightHandSupportsParametricHaptics, TEXT("Right"));
	return bAny;
}

void FControllerPICO::RefreshParametricHapticsProperties()
{
	if (!bParametricHapticsExtensionEnabled || xrHapticParametricGetPropertiesEXT == nullptr || Session == XR_NULL_HANDLE)
	{
		return;
	}

	QueryParametricHapticsProperties(Session);
}

bool FControllerPICO::GetInteractionProfiles(XrInstance InInstance, TArray<FString>& OutKeyPrefixes, TArray<XrPath>& OutPaths, TArray<bool>& OutHasHaptics)
{
	XrPath InteractionProfile;
	XrResult Result = xrStringToPath(InInstance, "/interaction_profiles/bytedance/pico_g3_controller", &InteractionProfile);
	if (XR_SUCCEEDED(Result))
	{
		OutKeyPrefixes.Add("PICOG3");
		OutPaths.Add(InteractionProfile);
		OutHasHaptics.Add(false);
		AddProfile(InteractionProfile);
	}

	Result = xrStringToPath(InInstance, "/interaction_profiles/bytedance/pico_neo3_controller", &InteractionProfile);
	if (XR_SUCCEEDED(Result))
	{
		OutKeyPrefixes.Add("PICONeo3");
		OutPaths.Add(InteractionProfile);
		OutHasHaptics.Add(true);
		AddProfile(InteractionProfile);
	}

	Result = xrStringToPath(InInstance, "/interaction_profiles/bytedance/pico4_controller", &InteractionProfile);
	if (XR_SUCCEEDED(Result))
	{
		OutKeyPrefixes.Add("PICO4");
		OutPaths.Add(InteractionProfile);
		OutHasHaptics.Add(true);
		AddProfile(InteractionProfile);
	}

	bSupportPICOUltra = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_BD_ULTRA_CONTROLLER_INTERACTION_EXTENSION_NAME);
	if (bSupportPICOUltra)
	{
		Result = xrStringToPath(InInstance, "/interaction_profiles/bytedance/pico_ultra_controller_bd", &InteractionProfile);
		if (XR_SUCCEEDED(Result))
		{
			OutKeyPrefixes.Add("PICOUltra");
			OutPaths.Add(InteractionProfile);
			OutHasHaptics.Add(true);
		}
	}
	else
	{
		Result = xrStringToPath(InInstance, "/interaction_profiles/bytedance/pico4s_controller", &InteractionProfile);
		if (XR_SUCCEEDED(Result))
		{
			OutKeyPrefixes.Add("PICOUltra");
			OutPaths.Add(InteractionProfile);
			OutHasHaptics.Add(true);
			AddProfile(InteractionProfile);
		}
	}

	return true;
}

bool FControllerPICO::GetSuggestedBindings(XrPath InInteractionProfile, TArray<XrActionSuggestedBinding>& OutBindings)
{
	if (Instance == XR_NULL_HANDLE)
	{
		return false;
	}

	if (ControllerBatteryAction != XR_NULL_HANDLE && Profiles.Find(InInteractionProfile) != INDEX_NONE)
	{
		XrPath BatteryLeft;
		XrResult Result;
		Result = xrStringToPath(Instance, "/user/hand/left/input/battery/value", &BatteryLeft);
		if (XR_SUCCEEDED(Result))
		{
			OutBindings.Add(XrActionSuggestedBinding{ ControllerBatteryAction ,BatteryLeft });
		}

		XrPath BatteryRight;
		Result = xrStringToPath(Instance, "/user/hand/right/input/battery/value", &BatteryRight);
		if (XR_SUCCEEDED(Result))
		{
			OutBindings.Add(XrActionSuggestedBinding{ ControllerBatteryAction ,BatteryRight });
		}
	}

	// Only bind haptic paths if parametric haptics extension is enabled
	if (bParametricHapticsExtensionEnabled)
	{
		XrPath HapticLeft;
		XrResult Result = xrStringToPath(Instance, "/user/hand/left/output/haptic", &HapticLeft);
		if (XR_SUCCEEDED(Result))
		{
			OutBindings.Add(XrActionSuggestedBinding{ LeftVibrationAction ,HapticLeft });
		}
		else
		{
			UE_LOG(LogPICOOpenXRInput, Warning, TEXT("Failed to get haptic path for left hand: %s"), OpenXRResultToString(Result));
		}

		XrPath HapticRight;
		Result = xrStringToPath(Instance, "/user/hand/right/output/haptic", &HapticRight);
		if (XR_SUCCEEDED(Result))
		{
			OutBindings.Add(XrActionSuggestedBinding{ RightVibrationAction ,HapticRight });
		}
		else
		{
			UE_LOG(LogPICOOpenXRInput, Warning, TEXT("Failed to get haptic path for right hand: %s"), OpenXRResultToString(Result));
		}
	}
	else
	{
		UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Skipping haptic bindings - parametric haptics extension not enabled"));
	}

	return true;
}

void FControllerPICO::AttachActionSets(TSet<XrActionSet>& OutActionSets)
{
	// Only attach if the action set has been created and bindings suggested
	if (ControllerActionSet != XR_NULL_HANDLE)
	{
		OutActionSets.Add(ControllerActionSet);
	}
}

const void* FControllerPICO::OnBeginSession(XrSession InSession, const void* InNext)
{
	return InNext;
}

void FControllerPICO::OnDestroySession(XrSession InSession)
{
	// Clean up haptic streams (covers active streaming and deferred-loop-wait state)
	if (LeftStreamState.IsValid() || LeftStreamState.LoopRestartTime > 0)
	{
		UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Cleaning up left hand haptic stream on session destroy (was streaming: %d, pending loop restart: %d)"),
			LeftStreamState.bIsStreaming ? 1 : 0, LeftStreamState.LoopRestartTime > 0 ? 1 : 0);
		LeftStreamState.Reset();
	}

	if (RightStreamState.IsValid() || RightStreamState.LoopRestartTime > 0)
	{
		UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Cleaning up right hand haptic stream on session destroy (was streaming: %d, pending loop restart: %d)"),
			RightStreamState.bIsStreaming ? 1 : 0, RightStreamState.LoopRestartTime > 0 ? 1 : 0);
		RightStreamState.Reset();
	}

	LeftSingleShotLoopState.Reset();
	RightSingleShotLoopState.Reset();

	// Destroy actions before destroying action set (correct OpenXR lifecycle order)
	// Note: Actions must be destroyed before the action set they belong to
	if (bParametricHapticsExtensionEnabled)
	{
		if (LeftVibrationAction != XR_NULL_HANDLE)
		{
			XR_ENSURE(xrDestroyAction(LeftVibrationAction));
			LeftVibrationAction = XR_NULL_HANDLE;
			UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Destroyed left vibration action"));
		}

		if (RightVibrationAction != XR_NULL_HANDLE)
		{
			XR_ENSURE(xrDestroyAction(RightVibrationAction));
			RightVibrationAction = XR_NULL_HANDLE;
			UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Destroyed right vibration action"));
		}
	}

	if (ControllerBatteryAction != XR_NULL_HANDLE)
	{
		XR_ENSURE(xrDestroyAction(ControllerBatteryAction));
		ControllerBatteryAction = XR_NULL_HANDLE;
		UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Destroyed battery action"));
	}

	// Destroy action set after all actions are destroyed
	if (ControllerActionSet != XR_NULL_HANDLE)
	{
		XR_ENSURE(xrDestroyActionSet(ControllerActionSet));
		ControllerActionSet = XR_NULL_HANDLE;
		UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Destroyed controller action set"));
	}

	// Reset session handle
	Session = XR_NULL_HANDLE;

	// Reset haptic support flags
	bLeftHandSupportsParametricHaptics = false;
	bRightHandSupportsParametricHaptics = false;
	LastHapticUpdateTimeSeconds = 0.0;
}

void FControllerPICO::GetActiveActionSetsForSync(TArray<XrActiveActionSet>& OutActiveSets)
{
	// Only sync if the action set has been created and attached
	if (ControllerActionSet != XR_NULL_HANDLE)
	{
		OutActiveSets.Add(XrActiveActionSet{ ControllerActionSet, SubactionPaths[0] });
		OutActiveSets.Add(XrActiveActionSet{ ControllerActionSet, SubactionPaths[1] });
	}
}

void FControllerPICO::PostSyncActions(XrSession InSession)
{
	IInputInterface* InputInterface = nullptr;
	if (FSlateApplication::IsInitialized())
	{
		InputInterface = FSlateApplication::Get().GetInputInterface();
	}
	int32 ControllerID = 0;

	// Only query battery state if the action has been created
	if (ControllerBatteryAction != XR_NULL_HANDLE)
	{
		for (int i = 0; i < SubactionPaths.Num(); i++)
		{
			XrActionStateGetInfo GetActionStateInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
			GetActionStateInfo.action = ControllerBatteryAction;
			GetActionStateInfo.subactionPath = SubactionPaths[i];
			ActionStateFloats[i] = { XR_TYPE_ACTION_STATE_FLOAT };
			XR_ENSURE(xrGetActionStateFloat(InSession, &GetActionStateInfo, &ActionStateFloats[i]));
		}
	}

	double CurrentTimeSeconds = FPlatformTime::Seconds();

	if (LastHapticUpdateTimeSeconds > 0.0)
	{
		float DeltaTime = (float)(CurrentTimeSeconds - LastHapticUpdateTimeSeconds);

		XrTime CurrentTime = (XrTime)(CurrentTimeSeconds * 1e9);

		UpdateHapticStream(CurrentTime, DeltaTime);
	}

	LastHapticUpdateTimeSeconds = CurrentTimeSeconds;
}

void FControllerPICO::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
{
	if (!InHeader)
	{
		return;
	}
	if (InHeader->type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED)
	{
		const XrEventDataSessionStateChanged* StateChanged = reinterpret_cast<const XrEventDataSessionStateChanged*>(InHeader);
		if (StateChanged->state == XR_SESSION_STATE_FOCUSED)
		{
			// On PICO runtimes the frequency range and frame timing properties only become valid after focus.
			RefreshParametricHapticsProperties();
		}
	}
}
bool FControllerPICO::GetControllerBatteryLevel(const EControllerHand Hand, float& Level)
{
	Level = 0.0f;
	if (Hand == EControllerHand::Left)
	{
		if (ActionStateFloats[0].isActive == XR_TRUE)
		{
			Level = ActionStateFloats[0].currentState;
			return true;
		}
		return false;
	}
	else if (Hand == EControllerHand::Right)
	{
		if (ActionStateFloats[1].isActive == XR_TRUE)
		{
			Level = ActionStateFloats[1].currentState;
			return true;
		}
		return false;
	}
	return false;
}

void FControllerPICO::AddProfile(XrPath Profile)
{
	Profiles.Add(Profile);
}

bool FControllerPICO::GetEffectiveFrequencyRange(EControllerHand Hand, bool bFromPHF, float& OutMinHz, float& OutMaxHz) const
{
	const XrHapticParametricPropertiesEXT& Props = (Hand == EControllerHand::Left) ? LeftHapticProperties : RightHapticProperties;

	if (Props.minFrequencyHz != XR_FREQUENCY_UNSPECIFIED && Props.minFrequencyHz > 0.0f
		&& Props.maxFrequencyHz != XR_FREQUENCY_UNSPECIFIED && Props.maxFrequencyHz > 0.0f)
	{
		OutMinHz = Props.minFrequencyHz;
		OutMaxHz = Props.maxFrequencyHz;
		return true;
	}
	else if (bFromPHF)
	{
		OutMinHz = XR_HAPTIC_PARAMETRIC_FREQUENCY_MIN_HZ_EXT;
		OutMaxHz = XR_HAPTIC_PARAMETRIC_FREQUENCY_MAX_HZ_EXT;
		return true;
	}
	else
	{
		OutMinHz = XR_FREQUENCY_UNSPECIFIED;
		OutMaxHz = XR_FREQUENCY_UNSPECIFIED;
	}

	return false;
}

/**
 * Create a test haptic curve with specified number of keys for testing streaming/single-shot modes
 * @param NumKeys Number of keyframes to generate (default 500)
 *                - Use 500 or less to test single-shot mode
 *                - Use 501+ to test streaming mode (> XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT)
 * @param Duration Total duration in seconds (default 5.0s)
 * @return Generated test curve with sine wave amplitude pattern
 */
static UHapticFeedbackEffect_Curve* CreateStreamingTestCurve(int32 NumKeys = 500, float Duration = 5.0f)
{
	UHapticFeedbackEffect_Curve* TestCurve = NewObject<UHapticFeedbackEffect_Curve>();

	FRichCurve* AmplitudeCurve = TestCurve->HapticDetails.Amplitude.GetRichCurve();
	FRichCurve* FrequencyCurve = TestCurve->HapticDetails.Frequency.GetRichCurve();

	AmplitudeCurve->Reset();
	FrequencyCurve->Reset();

	// Generate keyframes
	for (int32 i = 0; i < NumKeys; ++i)
	{
		float Time = NumKeys > 1 ? (Duration * i) / (NumKeys - 1) : 0.0f;
		float Amplitude = 0.5f + 0.5f * FMath::Sin(Time * 2.0f * PI);
		float Frequency = 0.7f;

		AmplitudeCurve->AddKey(Time, Amplitude);
		FrequencyCurve->AddKey(Time, Frequency);
	}

	// Log test curve info
	const TCHAR* ModeStr = NumKeys > (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT ? TEXT("STREAMING") : TEXT("SINGLE-SHOT");
	UE_LOG(LogPICOOpenXRInput, Log, TEXT("Created test curve: %d keys, %.2fs duration [%s mode expected]"),
		NumKeys, Duration, ModeStr);

	return TestCurve;
}

/**
 * Convert UHapticFeedbackEffect_Buffer to UHapticFeedbackEffect_Curve
 * @param BufferEffect The buffer-based haptic effect
 * @return Converted curve-based haptic effect
 */
static UHapticFeedbackEffect_Curve* ConvertBufferToCurve(UHapticFeedbackEffect_Buffer* BufferEffect)
{
	if (!BufferEffect || BufferEffect->Amplitudes.Num() == 0 || BufferEffect->SampleRate <= 0)
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("ConvertBufferToCurve: Invalid buffer effect (Amplitudes: %d, SampleRate: %d)"),
			BufferEffect ? BufferEffect->Amplitudes.Num() : 0,
			BufferEffect ? BufferEffect->SampleRate : 0);
		return nullptr;
	}

	UHapticFeedbackEffect_Curve* CurveEffect = NewObject<UHapticFeedbackEffect_Curve>();
	FRichCurve* AmplitudeCurve = CurveEffect->HapticDetails.Amplitude.GetRichCurve();
	FRichCurve* FrequencyCurve = CurveEffect->HapticDetails.Frequency.GetRichCurve();

	AmplitudeCurve->Reset();
	FrequencyCurve->Reset();

	const float SampleInterval = 1.0f / BufferEffect->SampleRate;

	for (int32 i = 0; i < BufferEffect->Amplitudes.Num(); ++i)
	{
		float TimeSeconds = i * SampleInterval;
		float NormalizedAmplitude = BufferEffect->Amplitudes[i] / 255.0f;

		AmplitudeCurve->AddKey(TimeSeconds, NormalizedAmplitude);
		FrequencyCurve->AddKey(TimeSeconds, 0.5f);
	}

	UE_LOG(LogPICOOpenXRInput, Log, TEXT("Converted buffer to curve: %d samples at %d Hz -> %d keys, %.3f s duration"),
		BufferEffect->Amplitudes.Num(),
		BufferEffect->SampleRate,
		AmplitudeCurve->GetNumKeys(),
		BufferEffect->GetDuration());

	return CurveEffect;
}

void FParametricHapticStreamState::Reset()
{
	// Release GC root if we own the effect object
	if (bOwnsEffect && CurveEffect)
	{
		CurveEffect->RemoveFromRoot();
	}
	CurveEffect = nullptr;
	bOwnsEffect = false;
	Scale = 1.0f;
	bLoop = false;
	CurrentAmplitudeIndex = 0;
	CurrentFrequencyIndex = 0;
	LastUpdateTime = 0;
	StreamStartTime = 0;
	bIsStreaming = false;
	LoopRestartTime = 0;
	AllAmplitudeKeys.Empty();
	AllFrequencyKeys.Empty();
	MinFrequencyHz = XR_FREQUENCY_UNSPECIFIED;
	MaxFrequencyHz = XR_FREQUENCY_UNSPECIFIED;
	bFromPHF = false;
	MaxPointsPerFrame = 0;
}

void FControllerPICO::PlayHapticEffect(class UHapticFeedbackEffect_Base* HapticEffect, EControllerHand Hand, float Scale, bool bLoop, bool bUseTestCurve, int32 TestCurveNumKeys, float TestCurveDuration, bool bFromPHF, int32 MaxPointsPerFrame)
{
	if (!bParametricHapticsExtensionEnabled)
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("PlayHapticEffect called but XR_EXT_haptic_parametric extension is not enabled"));
		return;
	}

	if (Session == XR_NULL_HANDLE)
	{
		return;
	}

	bool bHandSupported = false;
	if (Hand == EControllerHand::Left)
	{
		bHandSupported = bLeftHandSupportsParametricHaptics;
	}
	else if (Hand == EControllerHand::Right)
	{
		bHandSupported = bRightHandSupportsParametricHaptics;
	}

	if (!bHandSupported)
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("Parametric haptics not supported on %s hand controller"),
			Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));
		return;
	}

	UHapticFeedbackEffect_Curve* CurveEffect = nullptr;
	bool bOwnsEffect = false;

	if (bUseTestCurve)
	{
		CurveEffect = CreateStreamingTestCurve(TestCurveNumKeys, TestCurveDuration);
		bOwnsEffect = true; // internally created, we own it
	}
	else
	{
		CurveEffect = Cast<UHapticFeedbackEffect_Curve>(HapticEffect);

		if (!CurveEffect)
		{
			UHapticFeedbackEffect_Buffer* BufferEffect = Cast<UHapticFeedbackEffect_Buffer>(HapticEffect);
			if (BufferEffect)
			{
				UE_LOG(LogPICOOpenXRInput, Log, TEXT("Converting buffer haptic effect to curve for %s hand"),
					Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));
				CurveEffect = ConvertBufferToCurve(BufferEffect);
				bOwnsEffect = true; // internally created from buffer conversion, we own it
			}
		}
		else
		{
			// If the object has no persistent owner (i.e. it was created via NewObject without
			// an explicit outer, which defaults to the transient package), we take ownership
			// and must protect it from GC ourselves.
			bOwnsEffect = HapticEffect->IsIn(GetTransientPackage());
		}
	}

	if (!CurveEffect)
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("No valid haptic effect provided (must be Curve or Buffer type)"));
		return;
	}

	PlayHapticEffectInternal(CurveEffect, Hand, Scale, bLoop, bFromPHF, MaxPointsPerFrame, bOwnsEffect);
}

void FControllerPICO::StopHapticEffect(EControllerHand Hand)
{
	if (!bParametricHapticsExtensionEnabled)
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("StopHapticEffect called but XR_EXT_haptic_parametric extension is not enabled"));
		return;
	}

	if (Session == XR_NULL_HANDLE)
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("StopHapticEffect called with null session"));
		return;
	}

	if (Hand == EControllerHand::Left)
	{
		LeftSingleShotLoopState.Reset();
	}
	else if (Hand == EControllerHand::Right)
	{
		RightSingleShotLoopState.Reset();
	}

	// Reset stream state: covers both active streaming (bIsStreaming) and
	// deferred-loop-wait state (LoopRestartTime > 0, bIsStreaming = false).
	// Both cases may have bOwnsEffect=true and need RemoveFromRoot.
	if (Hand == EControllerHand::Left && (LeftStreamState.IsValid() || LeftStreamState.LoopRestartTime > 0))
	{
		UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Resetting left hand stream state (was streaming: %d, pending loop restart: %d)"),
			LeftStreamState.bIsStreaming ? 1 : 0, LeftStreamState.LoopRestartTime > 0 ? 1 : 0);
		LeftStreamState.Reset();
	}
	else if (Hand == EControllerHand::Right && (RightStreamState.IsValid() || RightStreamState.LoopRestartTime > 0))
	{
		UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Resetting right hand stream state (was streaming: %d, pending loop restart: %d)"),
			RightStreamState.bIsStreaming ? 1 : 0, RightStreamState.LoopRestartTime > 0 ? 1 : 0);
		RightStreamState.Reset();
	}

	XrHapticActionInfo HapticActionInfo;
	HapticActionInfo.type = XR_TYPE_HAPTIC_ACTION_INFO;
	HapticActionInfo.next = nullptr;
	HapticActionInfo.subactionPath = XR_NULL_PATH;

	if (Hand == EControllerHand::Left)
	{
		HapticActionInfo.action = LeftVibrationAction;
	}
	else if (Hand == EControllerHand::Right)
	{
		HapticActionInfo.action = RightVibrationAction;
	}
	else
	{
		return;
	}

	XrResult Result = xrStopHapticFeedback(Session, &HapticActionInfo);
	if (XR_FAILED(Result))
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("Failed to stop haptic feedback: %s"), OpenXRResultToString(Result));
	}
}

void FControllerPICO::UpdateHapticStream(XrTime CurrentTime, float DeltaTime)
{
	if (!bParametricHapticsExtensionEnabled)
	{
		return;
	}

	if (!bLeftHandSupportsParametricHaptics && !bRightHandSupportsParametricHaptics)
	{
		return;
	}

	auto UpdateHandStream = [this, CurrentTime](EControllerHand Hand, FParametricHapticStreamState& StreamState,
		const XrHapticParametricPropertiesEXT& HapticProps)
		{
			if (!StreamState.IsValid())
			{
				// Check for deferred loop restart: streaming data fully sent, waiting for hardware to finish playing
				if (StreamState.LoopRestartTime > 0 && CurrentTime >= StreamState.LoopRestartTime)
				{
					// Save loop params before Reset() clears them.
					UHapticFeedbackEffect_Curve* LoopCurveEffect = StreamState.CurveEffect;
					float LoopScale = StreamState.Scale;
					bool bLoopFromPHF = StreamState.bFromPHF;
					int32 LoopMaxPointsPerFrame = StreamState.MaxPointsPerFrame;
					bool bLoopOwnsEffect = StreamState.bOwnsEffect;
					// Bridge: prevent GC in the window between Reset (RemoveFromRoot) and
					// the AddToRoot inside PlayHapticEffectInternal.
					if (bLoopOwnsEffect && LoopCurveEffect) { LoopCurveEffect->AddToRoot(); }
					// PlayHapticEffectInternal calls StopHapticEffect internally.
					// StopHapticEffect now sees LoopRestartTime > 0 and calls Reset()
					// which calls RemoveFromRoot, balancing the bridge root above.
					PlayHapticEffectInternal(LoopCurveEffect, Hand, LoopScale, true, bLoopFromPHF, LoopMaxPointsPerFrame, bLoopOwnsEffect);
					if (bLoopOwnsEffect && LoopCurveEffect) { LoopCurveEffect->RemoveFromRoot(); }
					UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Deferred streaming loop restart for %s hand"),
						Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));
				}
				return;
			}

			// When idealFrameSubmissionRate is 0, send at maximum rate (every frame)
			// Otherwise, respect the specified submission interval
			bool bShouldSendFrame = false;
			if (HapticProps.idealFrameSubmissionRate == 0)
			{
				// Send every frame when rate is 0
				bShouldSendFrame = true;
			}
			else
			{
				// Respect the submission interval
				if (StreamState.LastUpdateTime == 0 ||
					(CurrentTime - StreamState.LastUpdateTime) >= HapticProps.idealFrameSubmissionRate)
				{
					bShouldSendFrame = true;
				}
			}

			if (bShouldSendFrame)
			{
				XrHapticParametricStreamFrameTypeEXT FrameType = XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_INTERMEDIATE_FRAME_EXT;

				TArray<XrHapticParametricPointEXT> AmplitudePoints;
				int32 AmplitudeCount = 0;
				int32 ActualMaxPointsPerFrame = (StreamState.MaxPointsPerFrame > 0) ?
					FMath::Clamp(StreamState.MaxPointsPerFrame, 1, (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT) :
					XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT;

				for (int32 i = StreamState.CurrentAmplitudeIndex;
					i < StreamState.AllAmplitudeKeys.Num() && AmplitudeCount < ActualMaxPointsPerFrame;
					++i, ++AmplitudeCount)
				{
					const FRichCurveKey& Key = StreamState.AllAmplitudeKeys[i];
					XrHapticParametricPointEXT Point;
					Point.time = (XrDuration)(Key.Time * 1e9);
					Point.value = FMath::Clamp(Key.Value * StreamState.Scale, 0.0f, 1.0f);
					AmplitudePoints.Add(Point);
				}

				TArray<XrHapticParametricPointEXT> FrequencyPoints;
				int32 FrequencyCount = 0;

				// Frequency keys must not exceed the last amplitude time in the same frame.
				// When amplitude is empty, fall back to the last amplitude key time if available; otherwise clamp to 0.
				XrDuration MaxAllowedFreqTime = 0;
				if (AmplitudePoints.Num() > 0)
				{
					MaxAllowedFreqTime = AmplitudePoints.Last().time;
				}
				else if (StreamState.AllAmplitudeKeys.Num() > 0)
				{
					MaxAllowedFreqTime = (XrDuration)(StreamState.AllAmplitudeKeys.Last().Time * 1e9);
				}

				for (int32 i = StreamState.CurrentFrequencyIndex;
					i < StreamState.AllFrequencyKeys.Num() && FrequencyCount < ActualMaxPointsPerFrame;
					++i)
				{
					const FRichCurveKey& Key = StreamState.AllFrequencyKeys[i];
					XrDuration KeyTime = (XrDuration)(Key.Time * 1e9);

					if (KeyTime > MaxAllowedFreqTime)
					{
						UE_LOG(LogPICOOpenXRInput, Verbose,
							TEXT("Skipping frequency point at %.3f s (exceeds max allowed %.3f s) in stream frame"),
							KeyTime / 1e9, MaxAllowedFreqTime / 1e9);
						break;
					}

					XrHapticParametricPointEXT Point;
					Point.time = KeyTime;
					Point.value = FMath::Clamp(Key.Value, 0.0f, 1.0f);
					FrequencyPoints.Add(Point);
					FrequencyCount++;
				}

				if (AmplitudePoints.Num() == 0)
				{
					UE_LOG(LogPICOOpenXRInput, Verbose,
						TEXT("No amplitude points for stream frame, ending stream for %s hand"),
						Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));

				// All keys were already sent in the first frame (e.g. MaxPointsPerFrame >= total keys).
				if (StreamState.bLoop)
				{
					// Defer restart until hardware finishes playing the already-submitted first-frame data.
					XrTime TotalDurationNs = StreamState.AllAmplitudeKeys.Num() > 0
						? (XrTime)(StreamState.AllAmplitudeKeys.Last().Time * 1e9) : 0;
					StreamState.LoopRestartTime = StreamState.StreamStartTime + TotalDurationNs;
					StreamState.bIsStreaming = false;
					UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Deferred loop restart (all-in-first-frame) for %s hand in %.3f s"),
						Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"), TotalDurationNs / 1e9);
				}
				else
				{
					// Close the open FIRST_FRAME stream and stop
					XrHapticActionInfo HapticActionInfo;
					HapticActionInfo.type = XR_TYPE_HAPTIC_ACTION_INFO;
					HapticActionInfo.next = nullptr;
					HapticActionInfo.subactionPath = XR_NULL_PATH;
					HapticActionInfo.action = (Hand == EControllerHand::Left) ? LeftVibrationAction : RightVibrationAction;
					xrStopHapticFeedback(Session, &HapticActionInfo);
					StreamState.Reset();
				}
				return;
				}

				if (StreamState.CurrentAmplitudeIndex + AmplitudeCount >= StreamState.AllAmplitudeKeys.Num())
				{
					FrameType = XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_LAST_FRAME_EXT;
				}

				if (SendHapticFrame(Hand, AmplitudePoints, FrequencyPoints, FrameType,
					StreamState.MinFrequencyHz, StreamState.MaxFrequencyHz))
				{
					StreamState.CurrentAmplitudeIndex += AmplitudeCount;
					StreamState.CurrentFrequencyIndex += FrequencyCount;

					StreamState.LastUpdateTime = CurrentTime;

					if (FrameType == XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_LAST_FRAME_EXT)
					{
						if (StreamState.bLoop)
						{
							// Defer loop restart until hardware finishes playing the streamed data.
							// StreamStartTime = when FIRST_FRAME was submitted; total duration = last key time.
							XrTime TotalDurationNs = StreamState.AllAmplitudeKeys.Num() > 0
								? (XrTime)(StreamState.AllAmplitudeKeys.Last().Time * 1e9) : 0;
							StreamState.LoopRestartTime = StreamState.StreamStartTime + TotalDurationNs;
							StreamState.bIsStreaming = false;
							UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Streaming LAST_FRAME sent, deferring loop restart for %s hand in %.3f s"),
								Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"), TotalDurationNs / 1e9);
						}
						else
						{
							StreamState.Reset();
							UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Completed haptic stream for %s hand"),
								Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));
						}
					}
				}
				else
				{
					UE_LOG(LogPICOOpenXRInput, Warning, TEXT("Failed to send haptic frame in stream, stopping and resetting for %s hand"),
						Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));

					// Ensure hardware haptic is stopped when frame send fails
					XrHapticActionInfo HapticActionInfo;
					HapticActionInfo.type = XR_TYPE_HAPTIC_ACTION_INFO;
					HapticActionInfo.next = nullptr;
					HapticActionInfo.subactionPath = XR_NULL_PATH;
					HapticActionInfo.action = (Hand == EControllerHand::Left) ? LeftVibrationAction : RightVibrationAction;
					xrStopHapticFeedback(Session, &HapticActionInfo);

					StreamState.Reset();
				}
			}
		};

	if (bLeftHandSupportsParametricHaptics)
	{
		UpdateHandStream(EControllerHand::Left, LeftStreamState, LeftHapticProperties);
	}

	if (bRightHandSupportsParametricHaptics)
	{
		UpdateHandStream(EControllerHand::Right, RightStreamState, RightHapticProperties);
	}

	auto UpdateHandSingleShotLoop = [this, CurrentTime](EControllerHand Hand, FSingleShotHapticLoopState& LoopState)
		{
			if (!LoopState.IsValid())
			{
				return;
			}

			if (LoopState.StartTime == 0)
			{
				LoopState.StartTime = CurrentTime;
				return;
			}

			if ((CurrentTime - LoopState.StartTime) < LoopState.Duration)
			{
				return;
			}

			XrHapticActionInfo HapticActionInfo;
			HapticActionInfo.type = XR_TYPE_HAPTIC_ACTION_INFO;
			HapticActionInfo.next = nullptr;
			HapticActionInfo.subactionPath = XR_NULL_PATH;
			HapticActionInfo.action = (Hand == EControllerHand::Left) ? LeftVibrationAction : RightVibrationAction;
			xrStopHapticFeedback(Session, &HapticActionInfo);

			if (SendHapticFrame(Hand, LoopState.AmplitudePoints, LoopState.FrequencyPoints,
				XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_NONE_EXT, LoopState.MinFrequencyHz, LoopState.MaxFrequencyHz))
			{
				LoopState.StartTime = CurrentTime;
			}
			else
			{
				LoopState.Reset();
			}
		};

	if (bLeftHandSupportsParametricHaptics)
	{
		UpdateHandSingleShotLoop(EControllerHand::Left, LeftSingleShotLoopState);
	}

	if (bRightHandSupportsParametricHaptics)
	{
		UpdateHandSingleShotLoop(EControllerHand::Right, RightSingleShotLoopState);
	}
}

void FControllerPICO::PlayHapticEffectInternal(UHapticFeedbackEffect_Curve* CurveEffect, EControllerHand Hand, float Scale, bool bLoop, bool bFromPHF, int32 MaxPointsPerFrame, bool bOwnsEffect)
{
	float Duration = CurveEffect->GetDuration();
	if (Duration <= 0.0f)
	{
		return;
	}

	const FRichCurve* AmplitudeCurve = CurveEffect->HapticDetails.Amplitude.GetRichCurveConst();
	const FRichCurve* FrequencyCurve = CurveEffect->HapticDetails.Frequency.GetRichCurveConst();

	if (!AmplitudeCurve || !FrequencyCurve)
	{
		return;
	}

	TArray<FRichCurveKey> AmplitudeKeys;
	for (auto It = AmplitudeCurve->GetKeyIterator(); It; ++It)
	{
		AmplitudeKeys.Add(*It);
	}

	TArray<FRichCurveKey> FrequencyKeys;
	for (auto It = FrequencyCurve->GetKeyIterator(); It; ++It)
	{
		FrequencyKeys.Add(*It);
	}

	const XrHapticParametricPropertiesEXT& HapticProps = (Hand == EControllerHand::Left) ? LeftHapticProperties : RightHapticProperties;

	float MinFreqHz = XR_FREQUENCY_UNSPECIFIED;
	float MaxFreqHz = XR_FREQUENCY_UNSPECIFIED;
	if (!GetEffectiveFrequencyRange(Hand, bFromPHF, MinFreqHz, MaxFreqHz))
	{
		UE_LOG(LogPICOOpenXRInput, Log, TEXT("Device returned invalid frequency range (%.1f-%.1f Hz), using UNSPECIFIED"),
			HapticProps.minFrequencyHz, HapticProps.maxFrequencyHz);
	}

	bool bUseStreaming = AmplitudeKeys.Num() > (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT ||
		FrequencyKeys.Num() > (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT ||
		MaxPointsPerFrame > 0;

	StopHapticEffect(Hand);

	if (MaxPointsPerFrame > 0 && AmplitudeKeys.Num() <= (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT)
	{
		UE_LOG(LogPICOOpenXRInput, Log, TEXT("Forcing streaming mode due to custom MaxPointsPerFrame=%d (total keys: %d)"),
			MaxPointsPerFrame, AmplitudeKeys.Num());
	}

	if (bUseStreaming)
	{
		UE_LOG(LogPICOOpenXRInput, Log, TEXT("Starting streaming haptic playback for %s hand (AmplitudeKeys: %d, FrequencyKeys: %d)"),
			Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"),
			AmplitudeKeys.Num(), FrequencyKeys.Num());

		FParametricHapticStreamState& StreamState = (Hand == EControllerHand::Left) ? LeftStreamState : RightStreamState;

		// Protect the CurveEffect from GC for the duration of streaming.
		// If we own it (created internally via NewObject), add it to the GC root.
		// StreamState.Reset() will call RemoveFromRoot when streaming stops.
		if (bOwnsEffect)
		{
			CurveEffect->AddToRoot();
		}
		StreamState.CurveEffect = CurveEffect;
		StreamState.bOwnsEffect = bOwnsEffect;
		StreamState.Scale = Scale;
		StreamState.bLoop = bLoop;
		StreamState.CurrentAmplitudeIndex = 0;
		StreamState.CurrentFrequencyIndex = 0;
		StreamState.AllAmplitudeKeys = AmplitudeKeys;
		StreamState.AllFrequencyKeys = FrequencyKeys;
		StreamState.MinFrequencyHz = MinFreqHz;
		StreamState.MaxFrequencyHz = MaxFreqHz;
		StreamState.bIsStreaming = true;
		StreamState.StreamStartTime = 0;
		StreamState.LastUpdateTime = 0;
		StreamState.bFromPHF = bFromPHF;
		StreamState.MaxPointsPerFrame = MaxPointsPerFrame;

		TArray<XrHapticParametricPointEXT> FirstAmplitudePoints;
		XrDuration FirstFrameDuration = 0;
		int32 FirstAmplitudeCount = 0;
		int32 ActualMaxPointsPerFrame = (MaxPointsPerFrame > 0) ?
			FMath::Clamp(MaxPointsPerFrame, 1, (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT) :
			XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT;

		if (MaxPointsPerFrame > 0)
		{
			UE_LOG(LogPICOOpenXRInput, Log, TEXT("Using custom MaxPointsPerFrame: %d (requested: %d, max allowed: %d)"),
				ActualMaxPointsPerFrame, MaxPointsPerFrame, (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT);
		}

		// When minimumFirstFrameDuration is 0, send maximum points (up to MaxPointsPerFrame) in first frame
		// Otherwise, respect the minimum duration requirement
		if (HapticProps.minimumFirstFrameDuration == 0)
		{
			// Send max points at maximum rate (respecting MaxPointsPerFrame limit)
			UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("First frame: minimumFirstFrameDuration is 0, sending up to %d points"), ActualMaxPointsPerFrame);
			for (int32 i = 0; i < AmplitudeKeys.Num() && FirstAmplitudeCount < ActualMaxPointsPerFrame; ++i, ++FirstAmplitudeCount)
			{
				const FRichCurveKey& Key = AmplitudeKeys[i];
				XrHapticParametricPointEXT Point;
				Point.time = (XrDuration)(Key.Time * 1e9);
				Point.value = FMath::Clamp(Key.Value * Scale, 0.0f, 1.0f);
				FirstAmplitudePoints.Add(Point);
				FirstFrameDuration = Point.time;
			}
		}
		else
		{
			// MUST satisfy minimum first frame duration requirement (highest priority)
			// Keep adding points until we meet the minimum duration.
			// Note: We still must not exceed XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT, otherwise the runtime may reject the call.
			// DO NOT add fake extension points - abort playback if original data doesn't meet requirement within the point budget.
			UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("First frame: minimumFirstFrameDuration=%.3f s, MaxPointsPerFrame=%d"),
				HapticProps.minimumFirstFrameDuration / 1e9, ActualMaxPointsPerFrame);

			for (int32 i = 0; i < AmplitudeKeys.Num() && FirstAmplitudeCount < (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT; ++i)
			{
				const FRichCurveKey& Key = AmplitudeKeys[i];
				XrHapticParametricPointEXT Point;
				Point.time = (XrDuration)(Key.Time * 1e9);
				Point.value = FMath::Clamp(Key.Value * Scale, 0.0f, 1.0f);
				FirstAmplitudePoints.Add(Point);
				FirstAmplitudeCount++;
				FirstFrameDuration = Point.time;

				// Check if we've met the minimum duration requirement
				if (FirstFrameDuration >= HapticProps.minimumFirstFrameDuration)
				{
					UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("First frame: Met minimum duration requirement at point %d (%.3f s)"),
						i + 1, FirstFrameDuration / 1e9);

					// If we exceeded MaxPointsPerFrame to meet duration requirement, log a warning
					if (FirstAmplitudeCount > ActualMaxPointsPerFrame)
					{
						UE_LOG(LogPICOOpenXRInput, Warning, TEXT("First frame: Exceeded MaxPointsPerFrame (%d points vs %d limit) to satisfy minimumFirstFrameDuration requirement"),
							FirstAmplitudeCount, ActualMaxPointsPerFrame);
					}
					break;
				}
			}

			// If original data doesn't meet minimum duration requirement, abort playback
			if (FirstFrameDuration < HapticProps.minimumFirstFrameDuration)
			{
				UE_LOG(LogPICOOpenXRInput, Error, TEXT("First frame: Original haptic data does not meet minimumFirstFrameDuration requirement (%.3f s vs %.3f s required). Aborting playback."),
					FirstFrameDuration / 1e9, HapticProps.minimumFirstFrameDuration / 1e9);

				// Ensure hardware haptic is stopped
				XrHapticActionInfo HapticActionInfo;
				HapticActionInfo.type = XR_TYPE_HAPTIC_ACTION_INFO;
				HapticActionInfo.next = nullptr;
				HapticActionInfo.subactionPath = XR_NULL_PATH;
				HapticActionInfo.action = (Hand == EControllerHand::Left) ? LeftVibrationAction : RightVibrationAction;
				xrStopHapticFeedback(Session, &HapticActionInfo);

				StreamState.Reset();
				return;
			}
		}

		TArray<XrHapticParametricPointEXT> FirstFrequencyPoints;
		int32 FirstFrequencyCount = 0;
		for (int32 i = 0; i < FrequencyKeys.Num() && FirstFrequencyCount < (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT; ++i)
		{
			const FRichCurveKey& Key = FrequencyKeys[i];
			XrDuration KeyTimeNs = (XrDuration)(Key.Time * 1e9);

			if (KeyTimeNs <= FirstFrameDuration)
			{
				XrHapticParametricPointEXT Point;
				Point.time = KeyTimeNs;
				Point.value = FMath::Clamp(Key.Value, 0.0f, 1.0f);
				FirstFrequencyPoints.Add(Point);
				FirstFrequencyCount++;
			}
			else
			{
				break;
			}
		}

		if (FirstAmplitudePoints.Num() == 0 || FirstAmplitudePoints[0].time != 0)
		{
			if (FirstAmplitudePoints.Num() >= (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT)
			{
				UE_LOG(LogPICOOpenXRInput, Error, TEXT("First frame: Cannot insert required amplitude start point without exceeding XR point limit (%d). Aborting playback."),
					(int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT);
				StreamState.Reset();
				return;
			}

			XrHapticParametricPointEXT StartPoint;
			StartPoint.time = 0;
			StartPoint.value = FirstAmplitudePoints.Num() > 0 ? FirstAmplitudePoints[0].value : 0.5f;
			FirstAmplitudePoints.Insert(StartPoint, 0);
			FirstAmplitudeCount++;
		}

		if (FirstAmplitudePoints.Num() < 2)
		{
			if (FirstAmplitudePoints.Num() >= (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT)
			{
				UE_LOG(LogPICOOpenXRInput, Error, TEXT("First frame: Cannot append required amplitude end point without exceeding XR point limit (%d). Aborting playback."),
					(int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT);
				StreamState.Reset();
				return;
			}

			XrHapticParametricPointEXT EndPoint;
			EndPoint.time = FMath::Max(HapticProps.minimumFirstFrameDuration, (XrDuration)(Duration * 1e9));
			EndPoint.value = FirstAmplitudePoints[0].value;
			FirstAmplitudePoints.Add(EndPoint);
		}

		if (FirstFrequencyPoints.Num() > 0 && FirstFrequencyPoints[0].time != 0)
		{
			if (FirstFrequencyPoints.Num() >= (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT)
			{
				UE_LOG(LogPICOOpenXRInput, Error, TEXT("First frame: Cannot insert required frequency start point without exceeding XR point limit (%d). Aborting playback."),
					(int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT);
				StreamState.Reset();
				return;
			}

			XrHapticParametricPointEXT StartPoint;
			StartPoint.time = 0;
			StartPoint.value = FirstFrequencyPoints[0].value;
			FirstFrequencyPoints.Insert(StartPoint, 0);
		}

		UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("First frame: %d amplitude points, %d frequency points, duration=%.3f s (min required=%.3f s)"),
			FirstAmplitudePoints.Num(), FirstFrequencyPoints.Num(),
			FirstAmplitudePoints.Last().time / 1e9, HapticProps.minimumFirstFrameDuration / 1e9);

		StreamState.StreamStartTime = (XrTime)(FPlatformTime::Seconds() * 1e9);

		if (SendHapticFrame(Hand, FirstAmplitudePoints, FirstFrequencyPoints,
			XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_FIRST_FRAME_EXT, MinFreqHz, MaxFreqHz))
		{
			XrDuration LastIncludedTime = FirstAmplitudePoints.Num() > 0 ? FirstAmplitudePoints.Last().time : 0;
			int32 OriginalKeysIncluded = 0;
			for (int32 i = 0; i < AmplitudeKeys.Num(); ++i)
			{
				if ((XrDuration)(AmplitudeKeys[i].Time * 1e9) <= LastIncludedTime)
				{
					OriginalKeysIncluded++;
				}
				else
				{
					break;
				}
			}
			StreamState.CurrentAmplitudeIndex = OriginalKeysIncluded;

			if (OriginalKeysIncluded >= AmplitudeKeys.Num())
			{
				UE_LOG(LogPICOOpenXRInput, Verbose,
					TEXT("All %d amplitude keys included in first frame for %s hand"),
					AmplitudeKeys.Num(),
					Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));
			}

			StreamState.CurrentFrequencyIndex = FirstFrequencyCount;
			StreamState.LastUpdateTime = StreamState.StreamStartTime;
		}
		else
		{
			UE_LOG(LogPICOOpenXRInput, Warning, TEXT("Failed to send first haptic frame, stopping and resetting stream state for %s hand"),
				Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));

			// Ensure hardware haptic is stopped when first frame fails
			XrHapticActionInfo HapticActionInfo;
			HapticActionInfo.type = XR_TYPE_HAPTIC_ACTION_INFO;
			HapticActionInfo.next = nullptr;
			HapticActionInfo.subactionPath = XR_NULL_PATH;
			HapticActionInfo.action = (Hand == EControllerHand::Left) ? LeftVibrationAction : RightVibrationAction;
			xrStopHapticFeedback(Session, &HapticActionInfo);

			StreamState.Reset();
		}
	}
	else
	{
		UE_LOG(LogPICOOpenXRInput, Verbose, TEXT("Using single-shot haptic playback for %s hand"),
			Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"));

		TArray<XrHapticParametricPointEXT> AmplitudePoints;
		int32 MaxAmplitudePoints = FMath::Min(AmplitudeKeys.Num(), (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT);
		for (int32 i = 0; i < MaxAmplitudePoints; ++i)
		{
			const FRichCurveKey& Key = AmplitudeKeys[i];
			XrHapticParametricPointEXT Point;
			Point.time = (XrDuration)(Key.Time * 1e9);
			Point.value = FMath::Clamp(Key.Value * Scale, 0.0f, 1.0f);
			AmplitudePoints.Add(Point);
		}

		TArray<XrHapticParametricPointEXT> FrequencyPoints;
		int32 MaxFrequencyPoints = FMath::Min(FrequencyKeys.Num(), (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT);
		for (int32 i = 0; i < MaxFrequencyPoints; ++i)
		{
			const FRichCurveKey& Key = FrequencyKeys[i];
			XrHapticParametricPointEXT Point;
			Point.time = (XrDuration)(Key.Time * 1e9);
			Point.value = FMath::Clamp(Key.Value, 0.0f, 1.0f);
			FrequencyPoints.Add(Point);
		}

		if (AmplitudePoints.Num() > 0 && FrequencyPoints.Num() > 0)
		{
			XrDuration LastAmplitudeTime = AmplitudePoints.Last().time;

			for (int32 i = FrequencyPoints.Num() - 1; i >= 0; --i)
			{
				if (FrequencyPoints[i].time > LastAmplitudeTime)
				{
					UE_LOG(LogPICOOpenXRInput, Warning,
						TEXT("Removing frequency point at time %.3f s (exceeds last amplitude point at %.3f s)"),
						FrequencyPoints[i].time / 1e9, LastAmplitudeTime / 1e9);
					FrequencyPoints.RemoveAt(i);
				}
				else
				{
					break;
				}
			}
		}

		if (AmplitudePoints.Num() == 0 || AmplitudePoints[0].time != 0)
		{
			if (AmplitudePoints.Num() >= (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT)
			{
				UE_LOG(LogPICOOpenXRInput, Error, TEXT("Single-shot: Cannot insert required amplitude start point without exceeding XR point limit (%d)."),
					(int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT);
				return;
			}

			XrHapticParametricPointEXT StartPoint;
			StartPoint.time = 0;
			StartPoint.value = AmplitudePoints.Num() > 0 ? AmplitudePoints[0].value : 0.5f;
			AmplitudePoints.Insert(StartPoint, 0);
		}

		if (AmplitudePoints.Num() < 2)
		{
			if (AmplitudePoints.Num() >= (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT)
			{
				UE_LOG(LogPICOOpenXRInput, Error, TEXT("Single-shot: Cannot append required amplitude end point without exceeding XR point limit (%d)."),
					(int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT);
				return;
			}

			XrHapticParametricPointEXT EndPoint;
			// Some runtimes report a non-zero minimumFirstFrameDuration; treat NONE as a single submit
			// and ensure the submitted duration is not shorter than the minimum requirement.
			XrDuration MinDuration = (Hand == EControllerHand::Left) ? LeftHapticProperties.minimumFirstFrameDuration : RightHapticProperties.minimumFirstFrameDuration;
			EndPoint.time = FMath::Max(MinDuration, (XrDuration)(Duration * 1e9));
			EndPoint.value = AmplitudePoints[0].value;
			AmplitudePoints.Add(EndPoint);
		}

		if (FrequencyPoints.Num() > 0 && FrequencyPoints[0].time != 0)
		{
			if (FrequencyPoints.Num() >= (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT)
			{
				UE_LOG(LogPICOOpenXRInput, Error, TEXT("Single-shot: Cannot insert required frequency start point without exceeding XR point limit (%d)."),
					(int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT);
				return;
			}

			XrHapticParametricPointEXT StartPoint;
			StartPoint.time = 0;
			StartPoint.value = FrequencyPoints[0].value;
			FrequencyPoints.Insert(StartPoint, 0);
		}

		SendHapticFrame(Hand, AmplitudePoints, FrequencyPoints,
			XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_NONE_EXT, MinFreqHz, MaxFreqHz);

		if (bLoop)
		{
			FSingleShotHapticLoopState& LoopState = (Hand == EControllerHand::Left) ? LeftSingleShotLoopState : RightSingleShotLoopState;
			LoopState.Reset();
			LoopState.bLoop = true;
			LoopState.bIsActive = true;
			LoopState.StartTime = (XrTime)(FPlatformTime::Seconds() * 1e9);
			LoopState.Duration = AmplitudePoints.Last().time;
			LoopState.MinFrequencyHz = MinFreqHz;
			LoopState.MaxFrequencyHz = MaxFreqHz;
			LoopState.AmplitudePoints = AmplitudePoints;
			LoopState.FrequencyPoints = FrequencyPoints;
		}
	}
}

bool FControllerPICO::SendHapticFrame(EControllerHand Hand, const TArray<XrHapticParametricPointEXT>& AmplitudePoints,
	const TArray<XrHapticParametricPointEXT>& FrequencyPoints, XrHapticParametricStreamFrameTypeEXT FrameType,
	float MinFreqHz, float MaxFreqHz)
{
	if (Session == XR_NULL_HANDLE)
	{
		return false;
	}

	int32 MinRequiredPoints = (FrameType == XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_NONE_EXT ||
		FrameType == XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_FIRST_FRAME_EXT) ? 2 : 1;

	if (AmplitudePoints.Num() < MinRequiredPoints)
	{
		UE_LOG(LogPICOOpenXRInput, Error,
			TEXT("Validation error: Need at least %d amplitude point(s), got %d. Frame type: %d"),
			MinRequiredPoints, AmplitudePoints.Num(), (int32)FrameType);
		return false;
	}

	if (AmplitudePoints.Num() > (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT ||
		FrequencyPoints.Num() > (int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT)
	{
		UE_LOG(LogPICOOpenXRInput, Error,
			TEXT("Validation error: Point count exceeds XR limit (%d). Amp=%d, Freq=%d, Frame type: %d"),
			(int32)XR_HAPTIC_PARAMETRIC_MAX_POINTS_TRANSIENTS_EXT,
			AmplitudePoints.Num(), FrequencyPoints.Num(), (int32)FrameType);
		return false;
	}

	if ((FrameType == XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_NONE_EXT ||
		FrameType == XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_FIRST_FRAME_EXT) &&
		AmplitudePoints[0].time != 0)
	{
		UE_LOG(LogPICOOpenXRInput, Error,
			TEXT("Validation error: First amplitude point must be at time 0 for NONE/FIRST_FRAME, got %.3f s. Frame type: %d"),
			AmplitudePoints[0].time / 1e9, (int32)FrameType);
		return false;
	}

	if (AmplitudePoints.Num() > 0 && FrequencyPoints.Num() > 0)
	{
		XrDuration LastAmpTime = AmplitudePoints.Last().time;
		XrDuration LastFreqTime = FrequencyPoints.Last().time;

		if (LastFreqTime > LastAmpTime)
		{
			UE_LOG(LogPICOOpenXRInput, Error,
				TEXT("Validation error: Last frequency point time (%.3f s) exceeds last amplitude point time (%.3f s). Frame type: %d"),
				LastFreqTime / 1e9, LastAmpTime / 1e9, (int32)FrameType);
			return false;
		}

		if ((FrameType == XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_NONE_EXT ||
			FrameType == XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_FIRST_FRAME_EXT) &&
			FrequencyPoints[0].time != 0)
		{
			UE_LOG(LogPICOOpenXRInput, Error,
				TEXT("Validation error: First frequency point must be at time 0 for NONE/FIRST_FRAME, got %.3f s. Frame type: %d"),
				FrequencyPoints[0].time / 1e9, (int32)FrameType);
			return false;
		}
	}

	UE_LOG(LogPICOOpenXRInput, Verbose,
		TEXT("SendHapticFrame: Hand=%s, FrameType=%d, AmpPoints=%d [%.3f-%.3f s], FreqPoints=%d [%.3f-%.3f s], MinFreq=%.1f, MaxFreq=%.1f"),
		Hand == EControllerHand::Left ? TEXT("L") : TEXT("R"),
		(int32)FrameType,
		AmplitudePoints.Num(),
		AmplitudePoints.Num() > 0 ? AmplitudePoints[0].time / 1e9 : 0.0,
		AmplitudePoints.Num() > 0 ? AmplitudePoints.Last().time / 1e9 : 0.0,
		FrequencyPoints.Num(),
		FrequencyPoints.Num() > 0 ? FrequencyPoints[0].time / 1e9 : 0.0,
		FrequencyPoints.Num() > 0 ? FrequencyPoints.Last().time / 1e9 : 0.0,
		MinFreqHz, MaxFreqHz);

	float ActualMinFreqHz = MinFreqHz;
	float ActualMaxFreqHz = MaxFreqHz;
	if (FrameType == XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_INTERMEDIATE_FRAME_EXT ||
		FrameType == XR_HAPTIC_PARAMETRIC_STREAM_FRAME_TYPE_LAST_FRAME_EXT)
	{
		ActualMinFreqHz = XR_FREQUENCY_UNSPECIFIED;
		ActualMaxFreqHz = XR_FREQUENCY_UNSPECIFIED;
	}

	XrHapticParametricVibrationEXT ParametricVibration;
	ParametricVibration.type = XR_TYPE_HAPTIC_PARAMETRIC_VIBRATION_EXT;
	ParametricVibration.next = nullptr;
	ParametricVibration.amplitudePointCount = AmplitudePoints.Num();
	ParametricVibration.amplitudePoints = AmplitudePoints.GetData();
	ParametricVibration.frequencyPointCount = FrequencyPoints.Num();
	ParametricVibration.frequencyPoints = FrequencyPoints.Num() > 0 ? FrequencyPoints.GetData() : nullptr;
	ParametricVibration.transientCount = 0;
	ParametricVibration.transients = nullptr;
	ParametricVibration.minFrequencyHz = ActualMinFreqHz;
	ParametricVibration.maxFrequencyHz = ActualMaxFreqHz;
	ParametricVibration.streamFrameType = FrameType;

	XrHapticActionInfo HapticActionInfo;
	HapticActionInfo.type = XR_TYPE_HAPTIC_ACTION_INFO;
	HapticActionInfo.next = nullptr;
	HapticActionInfo.subactionPath = XR_NULL_PATH;

	if (Hand == EControllerHand::Left)
	{
		HapticActionInfo.action = LeftVibrationAction;
	}
	else if (Hand == EControllerHand::Right)
	{
		HapticActionInfo.action = RightVibrationAction;
	}
	else
	{
		return false;
	}

	XrResult Result = xrApplyHapticFeedback(Session, &HapticActionInfo, (const XrHapticBaseHeader*)&ParametricVibration);
	if (XR_FAILED(Result))
	{
		UE_LOG(LogPICOOpenXRInput, Warning, TEXT("Failed to send haptic frame (type=%d) for %s hand: %s"),
			(int32)FrameType,
			Hand == EControllerHand::Left ? TEXT("left") : TEXT("right"),
			OpenXRResultToString(Result));
		return false;
	}

	return true;
}
#undef LOCTEXT_NAMESPACE
