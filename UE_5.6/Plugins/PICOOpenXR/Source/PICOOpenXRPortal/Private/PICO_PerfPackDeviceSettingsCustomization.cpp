// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_PerfPackDeviceSettingsCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "PICO_PerformancePackSettings.h"
#include "PropertyHandle.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "PICOPerfPackDeviceSettingsCustomization"

TSharedRef<IPropertyTypeCustomization> FPICOPerfPackDeviceSettingsCustomization::MakeInstance()
{
	return MakeShared<FPICOPerfPackDeviceSettingsCustomization>();
}

void FPICOPerfPackDeviceSettingsCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	PropertyUtilities = StructCustomizationUtils.GetPropertyUtilities();

	if (const FProperty* Property = StructPropertyHandle->GetProperty())
	{
		ParentPropertyName = Property->GetFName();
	}

	TArray<UObject*> OuterObjects;
	StructPropertyHandle->GetOuterObjects(OuterObjects);
	for (UObject* Obj : OuterObjects)
	{
		if (UPICO_PerformancePackSettings* Settings = Cast<UPICO_PerformancePackSettings>(Obj))
		{
			SettingsObject = Settings;
			break;
		}
	}

	HeaderRow
	.NameContent()
	[
		StructPropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		[
			StructPropertyHandle->CreatePropertyValueWidget()
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([this]()
			{
				if (const UPICO_PerformancePackSettings* Settings = SettingsObject.Get())
				{
					return Settings->IsBasicProfileEnabled(ParentPropertyName) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
				}
				return ECheckBoxState::Unchecked;
			})
			.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState)
			{
				if (UPICO_PerformancePackSettings* Settings = SettingsObject.Get())
				{
					Settings->SetBasicProfileEnabled(ParentPropertyName, NewState == ECheckBoxState::Checked);
				}
				if (PropertyUtilities)
				{
					PropertyUtilities->ForceRefresh();
				}
			})
		]
	];
}

void FPICOPerfPackDeviceSettingsCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle, IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	uint32 NumChildren = 0;
	StructPropertyHandle->GetNumChildren(NumChildren);
	for (uint32 Index = 0; Index < NumChildren; ++Index)
	{
		if (TSharedPtr<IPropertyHandle> Child = StructPropertyHandle->GetChildHandle(Index))
		{
			StructBuilder.AddProperty(Child.ToSharedRef());
		}
	}

	StructBuilder.AddCustomRow(LOCTEXT("ResetToDefault", "Reset to Default"))
	.WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.HAlign(HAlign_Right)
		[
			SNew(SButton)
			.Text(LOCTEXT("ResetToDefaultButton", "Reset to Default"))
			.OnClicked(FOnClicked::CreateSP(this, &FPICOPerfPackDeviceSettingsCustomization::HandleResetClicked))
		]
	];
}

FReply FPICOPerfPackDeviceSettingsCustomization::HandleResetClicked()
{
	UPICO_PerformancePackSettings* Settings = SettingsObject.Get();
	if (!Settings)
	{
		return FReply::Handled();
	}

	const FScopedTransaction Transaction(LOCTEXT("ResetDeviceSettingsTransaction", "Reset Device Settings"));
	Settings->Modify();

	if (ParentPropertyName == TEXT("Swan"))
	{
		Settings->ResetSwanToDefault();
		Settings->ApplySwan();
	}
	else if (ParentPropertyName == TEXT("PICO4"))
	{
		Settings->ResetPICO4ToDefault();
		Settings->ApplyPICO4();
	}
	else if (ParentPropertyName == TEXT("Others"))
	{
		Settings->ResetOthersToDefault();
		Settings->ApplyOthers();
	}

	if (PropertyUtilities)
	{
		PropertyUtilities->ForceRefresh();
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
