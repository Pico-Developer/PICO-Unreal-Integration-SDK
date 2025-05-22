// Fill out your copyright notice in the Description page of Project Settings.


#include "SPXR_VerificationWidget.h"
#include "PXR_VerificationSubsystem.h"
#include "SlateOptMacros.h"
#include "SWarningOrErrorBox.h"
#include "PXR_VerificatonFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "ProjectVerification"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SPICOXRProjectVerificationWidget::Construct(const FArguments& InArgs)
{
	
}

void SPICOXRProjectVerificationWidget::SetNameText(FText InText)
{
	NameTextBlock->SetText(InText);
}

void SPICOXRProjectVerificationWidget::SetDescriptionText(FText InText)
{
	DescriptionTextBlock->SetText(InText);
}

/**
 * Build a container for a rules section
 */
TSharedPtr<SVerticalBox> SPICOXRProjectVerificationWidget::BuildVerificationRow(TObjectPtr<UVerificationObject> InObjectPtr)
{
	if (InObjectPtr)
	{
		ObjectPtr = InObjectPtr;
	}
	// RowWidget -> Splitter -> (NameWidget | DescriptionWidget | ButtonWidget)
	const bool bShouldHideApplyButton = InObjectPtr?InObjectPtr->IsVerificationFixed():false;
	
	// Setup the column information for the layout
	ColumnSizeData = MakeShared<FDetailColumnSizeData>();
	ColumnSizeData->SetValueColumnWidth(0.75f);
	ColumnSizeData->SetRightColumnMinWidth(150);
	
	if (ObjectPtr->IsVerificationFixed())
	{
		IconBrush = FAppStyle::Get().GetBrush("Icons.SuccessWithColor");
	}
	else
	{
		switch (ObjectPtr->GetLevel()) {
		case EVerificationLevel::Error:
			{
				IconBrush = FAppStyle::Get().GetBrush("Icons.ErrorWithColor");
			}
			break;
		case EVerificationLevel::Warning:
			{
				IconBrush = FAppStyle::Get().GetBrush("Icons.WarningWithColor");
			}
			break;
		case EVerificationLevel::Recommended:
			{
				if (RecommendIconBrush)
				{
					IconBrush = RecommendIconBrush;
				}
			}
			break;
		case EVerificationLevel::None:
			{
				IconBrush = FAppStyle::Get().GetBrush("Icons.ErrorWithColor");
			}
			break;
		}
	}
	
	// Name widget
	NameWidget = SNew(SHorizontalBox).Clipping(EWidgetClipping::OnDemand);

	if (IconBrush != nullptr)
	{
		NameWidget->AddSlot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(8, 0, 8, 0)
			.AutoWidth()
				[SNew(SImage)
						.Image(IconBrush)];
	};
	NameTextBlock=SNew(STextBlock)
				.Font(FAppStyle::Get().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
				.ColorAndOpacity(FSlateColor::UseStyle())
				.Text(FText::GetEmpty());
	NameWidget->AddSlot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(0, 0, 0, 0)
	[NameTextBlock.ToSharedRef()];

	// Description widget
	const TSharedPtr<SHorizontalBox> DescriptionWidget = SNew(SHorizontalBox)
															 .Clipping(EWidgetClipping::OnDemand);

	DescriptionTextBlock =SNew(STextBlock)
					.Font(FAppStyle::Get().GetFontStyle(TEXT("PropertyWindow.NormalFont")))
					.ColorAndOpacity(FSlateColor::UseStyle())
					.Text(LOCTEXT("FPICOXRProjectVerificationModule","Description"));
	DescriptionWidget->AddSlot()
		.VAlign(VAlign_Center)
		.Padding(6, 0)
			[DescriptionTextBlock.ToSharedRef()];

	// Button widget. There are two buttons slots that are customised based on section
	const TSharedPtr<SHorizontalBox> ButtonWidget = SNew(SHorizontalBox);
	
	ButtonWidget->AddSlot()
		.Padding(FMargin(6, 3, 3, 3))
	[SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.Text(LOCTEXT("FPICOXRProjectVerificationModule", "Fix"))
				.OnClicked(this, &SPICOXRProjectVerificationWidget::OnFixClicked,InObjectPtr)
				.Visibility_Lambda([bShouldHideApplyButton]() -> EVisibility { return bShouldHideApplyButton ? EVisibility::Hidden : EVisibility::Visible; })];

	FMenuBuilder EllipsisMenuBuilder(true, nullptr);

	// Splitter so each row can contain Name | Description | Buttons
	const TSharedPtr<SSplitter> Splitter = SNew(SSplitter)
											   .Style(FAppStyle::Get(), "DetailsView.Splitter")
											   .PhysicalSplitterHandleSize(1.0f)
											   .HitDetectionSplitterHandleSize(5.0f);

	// Add the widgets to the splitter
	Splitter->AddSlot()
		.Value(ColumnSizeData->GetNameColumnWidth())
		.OnSlotResized(ColumnSizeData->GetOnNameColumnResized())
			[NameWidget.ToSharedRef()];

	Splitter->AddSlot()
		.Value(ColumnSizeData->GetValueColumnWidth())
		.OnSlotResized(ColumnSizeData->GetOnValueColumnResized())
			[DescriptionWidget.ToSharedRef()];

	Splitter->AddSlot()
		.Value(ColumnSizeData->GetRightColumnWidth())
		.OnSlotResized(ColumnSizeData->GetOnRightColumnResized())
		.MinSize(ColumnSizeData->GetRightColumnMinWidth())
			[ButtonWidget.ToSharedRef()];

	// Top level container for the whole row
	RowWidget = SNew(SBorder)
				.Visibility(this, &SPICOXRProjectVerificationWidget::OnRowVisibility)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.BorderImage(FAppStyle::Get().GetBrush("DetailsView.CategoryMiddle"))
				.BorderBackgroundColor(FAppStyle::Get().GetSlateColor("Colors.Panel"))
				.Padding(0)
				[Splitter.ToSharedRef()];

	ChildSlot
		[RowWidget.ToSharedRef()];

	return nullptr;
}

EVisibility SPICOXRProjectVerificationWidget::OnRowVisibility() const
{
	
	return EVisibility::Visible;
}


FReply SPICOXRProjectVerificationWidget::OnFixClicked(TObjectPtr<UVerificationObject> InObjectPtr)
{
	if (ObjectPtr)
	{
		ObjectPtr->PerformVerificationFix();
	}
	
	return FReply::Handled();
}

void SPICOXRRestartEditorWidget::Construct(const FArguments& InArgs)
{
	RestartWidget=SNew(SWarningOrErrorBox)
					.Visibility(this, &SPICOXRRestartEditorWidget::HandleRestartEditorNoticeVisibility)
					.MessageStyle(EMessageStyle::Warning)
					.Message(LOCTEXT("FPICOXRProjectVerificationModule", "You must restart Unreal Editor for your changes to take effect."))
					[
						SNew(SButton)
						.OnClicked(this, &SPICOXRRestartEditorWidget::HandleRestartEditorButtonClicked)
						.TextStyle(FAppStyle::Get(), "NormalText")
						.Text(LOCTEXT("FPICOXRProjectVerificationModule", "Restart Now"))
					];
	ChildSlot
		[RestartWidget.ToSharedRef()];
}

EVisibility SPICOXRRestartEditorWidget::HandleRestartEditorNoticeVisibility() const
{
	return UUPICOXRVerificationFunctionLibrary::HasVerificationNeedRestart() ? EVisibility::Visible : EVisibility::Collapsed;
}

FReply SPICOXRRestartEditorWidget::HandleRestartEditorButtonClicked() const
{
	const bool bWarn = false;
	UUPICOXRVerificationFunctionLibrary::SetVerificationNeedRestart(false);
	FUnrealEdMisc::Get().RestartEditor(bWarn);
	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
