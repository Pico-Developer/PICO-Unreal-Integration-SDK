// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetailColumnSizeData.h"
#include "PICO_VerificationObject.h"
#include "SPICO__VerificationWidget.h"
#include "Components/Button.h"
#include "PICO_VerificationComponents.generated.h"


/**
 * 
 */
UCLASS()
class PICOOPENXRPORTAL_API UPICOVerificationSplitterRow : public UContentWidget
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category = UI)
    void SetVerificationObject(UPICOVerificationObject* ObjectPtr);

	UFUNCTION(BlueprintCallable, Category="Widget")
	void SetNameText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category="Widget")
	void SetDescriptionText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category="Widget")
	void SetVerificationsPart(EVerificationsStatusPICO InPart);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Widget")
	FSlateBrush RecommendBrush;
	
	TSharedPtr<STextBlock> NameTextBlock;

	TSharedPtr<SButton> FixButton;

	TSharedPtr<SPICOXRProjectVerificationWidget> VerificationWidget;

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
#if WITH_EDITOR
	virtual TSharedRef<SWidget> RebuildDesignWidget(TSharedRef<SWidget> Content) override { return Content; }
#endif
	
	TSharedPtr<FDetailColumnSizeData> ColumnSizeData{};
	UPROPERTY()
	TObjectPtr<UPICOVerificationObject> VerificationObject;
};


UCLASS()
class PICOOPENXRPORTAL_API UPICOVerificationRestart : public UContentWidget
{
	GENERATED_UCLASS_BODY()

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
#if WITH_EDITOR
	virtual TSharedRef<SWidget> RebuildDesignWidget(TSharedRef<SWidget> Content) override { return Content; }
#endif
};