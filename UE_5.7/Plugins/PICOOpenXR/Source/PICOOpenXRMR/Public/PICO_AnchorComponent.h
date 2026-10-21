// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_MRTypes.h"
#include "Components/ActorComponent.h"

#include "PICO_AnchorComponent.generated.h"

UCLASS(BlueprintType)
class PICOOPENXRMR_API UAnchorComponentPICO : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnchorComponentPICO(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	FSpatialHandlePICO GetAnchorHandle() const { return AnchorHandle; }

	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	void SetAnchorHandle(FSpatialHandlePICO NewAnchorHandle) { AnchorHandle = NewAnchorHandle; }

	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	FString GetAnchorHandleString() const { return AnchorHandle.ToString(); }

	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	bool IsAnchorValid() const { return AnchorHandle.IsValid(); }

	void ResetAnchorHandle();
protected:
	FSpatialHandlePICO AnchorHandle;
};