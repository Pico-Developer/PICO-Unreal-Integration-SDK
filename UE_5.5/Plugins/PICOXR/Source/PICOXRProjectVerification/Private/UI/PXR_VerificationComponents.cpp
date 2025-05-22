// Fill out your copyright notice in the Description page of Project Settings.


#include "PXR_VerificationComponents.h"

#include "DetailColumnSizeData.h"
#include "SPXR_VerificationWidget.h"
#include "SWarningOrErrorBox.h"
#include "Styling/DefaultStyleCache.h"

UPICOXRVerificationSplitterRow::UPICOXRVerificationSplitterRow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPICOXRVerificationSplitterRow::SetVerificationObject(UVerificationObject* InObjectPtr)
{
	if (InObjectPtr)
	{
		VerificationObject = InObjectPtr;
		VerificationWidget->BuildVerificationRow(VerificationObject);
	}
}

void UPICOXRVerificationSplitterRow::SetNameText(const FText& InText)
{
	VerificationWidget->SetNameText(InText);
}

void UPICOXRVerificationSplitterRow::SetDescriptionText(const FText& InText)
{
	VerificationWidget->SetDescriptionText(InText);
}

void UPICOXRVerificationSplitterRow::SetVerificationsPart(EVerificationsStatus InPart)
{
	VerificationWidget->VerificationsPart=InPart;
}

TSharedRef<SWidget> UPICOXRVerificationSplitterRow::RebuildWidget()
{
	VerificationWidget=SNew(SPICOXRProjectVerificationWidget);
	VerificationWidget->RecommendIconBrush=&RecommendBrush;
	return VerificationWidget.ToSharedRef();
}

void UPICOXRVerificationSplitterRow::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	VerificationWidget.Reset();
}

UPICOXRVerificationRestart::UPICOXRVerificationRestart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UPICOXRVerificationRestart::RebuildWidget()
{
	TSharedRef<SWidget> RebuildWidget = SNew(SPICOXRRestartEditorWidget);
	return RebuildWidget;
}

void UPICOXRVerificationRestart::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
}
