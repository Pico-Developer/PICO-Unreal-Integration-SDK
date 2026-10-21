// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkSourceFactory.h"
#include "PICO_LiveLinkSourceFactory.generated.h"

/**
 * 
 */
UCLASS()
class UPICOLiveLinkSourceFactory : public ULiveLinkSourceFactory
{
public:
	GENERATED_BODY()
	
	virtual FText GetSourceDisplayName() const override;
	virtual FText GetSourceTooltip() const override;

	virtual EMenuType GetMenuType() const override;
	virtual TSharedPtr<ILiveLinkSource> CreateSource(const FString& ConnectionString) const override;

	TSharedPtr<class SLiveLinkPICOXRMovementSourceEditor> ActiveSourceEditor;
};
