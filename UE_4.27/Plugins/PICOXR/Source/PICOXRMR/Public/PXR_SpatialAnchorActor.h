// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PXR_MRTypes.h"
#include "GameFramework/Actor.h"
#include "PXR_SpatialAnchorActor.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FPXRSpatialAnchorEventDelegate,const FPICOSpatialUUID&,UUID,EPICOResult, Result);

UCLASS(BlueprintType,DisplayName="PICO XR SpatialAnchor Actor")
class APICOXRSpatialAnchorActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APICOXRSpatialAnchorActor();
	
	UFUNCTION(BlueprintCallable, Category = "PICO XR Toolkit")
	void SaveSpatialAnchorAsync(const FPXRSpatialAnchorEventDelegate& OnSaveFinished);

	UFUNCTION(BlueprintCallable, Category = "PICO XR Toolkit")
	void DeleteSpatialAnchorAsync(const FPXRSpatialAnchorEventDelegate& OnDeleteFinished);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void HandleAnchorCreated(EPICOResult Result){};

	virtual void HandleAnchorPersisted (EPICOResult Result){};
	
	virtual void HandleAnchorUnpersisted(EPICOResult Result){};

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	FPXRSpatialAnchorEventDelegate SaveDelegate;
	FPXRSpatialAnchorEventDelegate DeleteDelegate;

	UPROPERTY()
	UPICOAnchorComponent* SpatialAnchorComponent;

	void CreateSpatialAnchorAsync_Internal();
	
	void PersistSpatialAnchorAsync_Internal();

	void UnpersistSpatialAnchorAsync_Internal();
	
	UFUNCTION()
	void HandleCreateSpatialAnchorEvent(EPICOResult Result, const UPICOAnchorComponent* AnchorComponent);

	UFUNCTION()
	void HandlePersistSpatialAnchorEvent(EPICOResult Result, const UPICOAnchorComponent* AnchorComponent);

	UFUNCTION()
	void HandleUnpersistSpatialAnchorEvent(EPICOResult Result, const UPICOAnchorComponent* AnchorComponent);
};
