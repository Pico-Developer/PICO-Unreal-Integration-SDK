// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetailColumnSizeData.h"
#include "PXR_VerificationObject.h"
#include "Widgets/SCompoundWidget.h"

UENUM(BlueprintType)
enum class EVerificationsStatus : uint8
{
	Required,
	Applied
};

/**
 * 
 */
class PICOXRPROJECTVERIFICATION_API SPICOXRProjectVerificationWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPICOXRProjectVerificationWidget)
		{
		}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	
	void SetNameText(FText InText);

	void SetDescriptionText(FText InText);
	
	FSlateBrush* RecommendIconBrush;
	
	TSharedPtr<SHorizontalBox> NameWidget;
	TSharedPtr<STextBlock> NameTextBlock;
	TSharedPtr<STextBlock> DescriptionTextBlock;
	/** Build a container for a rules section */
	TSharedPtr<SVerticalBox> BuildVerificationRow(TObjectPtr<UVerificationObject> InObjectPtr);
	TSharedPtr<FDetailColumnSizeData> ColumnSizeData{};
	EVisibility OnRowVisibility() const;

	TSharedPtr<SWidget> RowWidget;
	
	FReply OnFixClicked(TObjectPtr<UVerificationObject> InObjectPtr);
	
	TObjectPtr<UVerificationObject> ObjectPtr;
	
	EVerificationsStatus VerificationsPart;

	const FSlateBrush* IconBrush;
};

class PICOXRPROJECTVERIFICATION_API SPICOXRRestartEditorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SPICOXRRestartEditorWidget)
	{
	}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	TSharedPtr<SWidget> RestartWidget;
	/** @return Is the "restart required" notice visible? */
	EVisibility HandleRestartEditorNoticeVisibility() const;

	/** Handle the "restart now" button being clicked */
	FReply HandleRestartEditorButtonClicked() const;
};