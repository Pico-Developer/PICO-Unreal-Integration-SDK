// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "PXR_MRTypes.h"
#include "Components/ActorComponent.h"

#include "PXR_AnchorComponent.generated.h"

UCLASS(Blueprintable)
class PICOXRMR_API UPICOMREventComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UPICOMREventComponent(const FObjectInitializer& ObjectInitializer);
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnSpatialTrackingStateUpdate(EPICOSpatialTrackingState SpatialTrackingState, EPICOSpatialTrackingStateMessage SpatialTrackingStateMessage);

protected:
	FDelegateHandle HandleOfSpatialTrackingStateUpdate;
};

UCLASS(BlueprintType)
class PICOXRMR_API UPICOAnchorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPICOAnchorComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "PXR|MR")
	FPICOAnchor GetAnchorHandle() const { return AnchorHandle; }

	UFUNCTION(BlueprintCallable, Category = "PXR|MR")
	void SetAnchorHandle(FPICOAnchor NewAnchorHandle) { AnchorHandle = NewAnchorHandle; }

	UFUNCTION(BlueprintCallable, Category = "PXR|MR")
	FString GetAnchorHandleString() const { return AnchorHandle.ToString(); }

	UFUNCTION(BlueprintCallable, Category = "PXR|MR")
	bool IsAnchorValid() const { return AnchorHandle.IsValid(); }

	UFUNCTION(BlueprintCallable, Category = "PXR|MR")
	void SetAnchorUUID(FPICOAnchorUUID NewAnchorUUID) { AnchorUUID = NewAnchorUUID; }

	UFUNCTION(BlueprintCallable, Category = "PXR|MR")
	FPICOAnchorUUID GetAnchorUUID() const { return AnchorUUID; }

	UFUNCTION(BlueprintCallable, Category = "PXR|MR")
	FString GetAnchorUUIDString() const {return AnchorUUID.ToString(); }

protected:
	FPICOAnchor AnchorHandle;
	FPICOAnchorUUID AnchorUUID;
};