// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetailColumnSizeData.h"
#include "PXR_VerificationObject.h"
#include "SPXR_VerificationWidget.h"
#include "Components/Button.h"
#include "PXR_VerificationComponents.generated.h"


/**
 * 
 */
UCLASS()
class PICOXRPROJECTVERIFICATION_API UPICOXRVerificationSplitterRow : public UContentWidget
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, Category = UI)
    void SetVerificationObject(UVerificationObject* ObjectPtr);

	UFUNCTION(BlueprintCallable, Category="Widget")
	void SetNameText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category="Widget")
	void SetDescriptionText(const FText& InText);

	UFUNCTION(BlueprintCallable, Category="Widget")
	void SetVerificationsPart(EVerificationsStatus InPart);

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
	TObjectPtr<UVerificationObject> VerificationObject;
};


UCLASS()
class PICOXRPROJECTVERIFICATION_API UPICOXRVerificationRestart : public UContentWidget
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