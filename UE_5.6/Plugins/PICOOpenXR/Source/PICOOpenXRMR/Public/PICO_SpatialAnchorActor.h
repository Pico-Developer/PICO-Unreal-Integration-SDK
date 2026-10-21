// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PICO_MRTypes.h"
#include "GameFramework/Actor.h"
#include "PICO_SpatialAnchorActor.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FPXRSpatialAnchorEventDelegate,const FSpatialUUIDPICO&,UUID,EResultPICO, Result);

UCLASS(BlueprintType,DisplayName="Spatial Anchor Actor PICO")
class ASpatialAnchorActorPICO : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpatialAnchorActorPICO();
	
	UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
	void SaveSpatialAnchorAsync(const FPXRSpatialAnchorEventDelegate& OnSaveFinished);

	UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
	void DeleteSpatialAnchorAsync(const FPXRSpatialAnchorEventDelegate& OnDeleteFinished);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void HandleAnchorCreated(EResultPICO Result){};

	virtual void HandleAnchorPersisted (EResultPICO Result){};
	
	virtual void HandleAnchorUnpersisted(EResultPICO Result){};

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	FPXRSpatialAnchorEventDelegate SaveDelegate;
	FPXRSpatialAnchorEventDelegate DeleteDelegate;

	UPROPERTY()
	UAnchorComponentPICO* SpatialAnchorComponent;

	void CreateSpatialAnchorAsync_Internal();
	
	void PersistSpatialAnchorAsync_Internal();

	void UnpersistSpatialAnchorAsync_Internal();
	
	UFUNCTION()
	void HandleCreateSpatialAnchorEvent(EResultPICO Result, const UAnchorComponentPICO* AnchorComponent);

	UFUNCTION()
	void HandlePersistSpatialAnchorEvent(EResultPICO Result, const UAnchorComponentPICO* AnchorComponent);

	UFUNCTION()
	void HandleUnpersistSpatialAnchorEvent(EResultPICO Result, const UAnchorComponentPICO* AnchorComponent);
};
