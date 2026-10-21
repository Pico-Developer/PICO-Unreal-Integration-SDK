// Fill out your copyright notice in the Description page of Project Settings.


#include "PICO_VerificationComponents.h"
#include "SPICO__VerificationWidget.h"

UPICOVerificationSplitterRow::UPICOVerificationSplitterRow(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPICOVerificationSplitterRow::SetVerificationObject(UPICOVerificationObject* InObjectPtr)
{
	if (InObjectPtr)
	{
		VerificationObject = InObjectPtr;
		VerificationWidget->BuildVerificationRow(VerificationObject);
	}
}

void UPICOVerificationSplitterRow::SetNameText(const FText& InText)
{
	VerificationWidget->SetNameText(InText);
}

void UPICOVerificationSplitterRow::SetDescriptionText(const FText& InText)
{
	VerificationWidget->SetDescriptionText(InText);
}

void UPICOVerificationSplitterRow::SetVerificationsPart(EVerificationsStatusPICO InPart)
{
	VerificationWidget->VerificationsPart=InPart;
}

TSharedRef<SWidget> UPICOVerificationSplitterRow::RebuildWidget()
{
	VerificationWidget=SNew(SPICOXRProjectVerificationWidget);
	VerificationWidget->RecommendIconBrush=&RecommendBrush;
	return VerificationWidget.ToSharedRef();
}

void UPICOVerificationSplitterRow::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	VerificationWidget.Reset();
}

UPICOVerificationRestart::UPICOVerificationRestart(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UPICOVerificationRestart::RebuildWidget()
{
	TSharedRef<SWidget> RebuildWidget = SNew(SPICOXRRestartEditorWidget);
	return RebuildWidget;
}

void UPICOVerificationRestart::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
}
