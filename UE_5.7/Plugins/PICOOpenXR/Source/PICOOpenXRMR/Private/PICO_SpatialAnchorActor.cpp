// Fill out your copyright notice in the Description page of Project Settings.


#include "PICO_SpatialAnchorActor.h"

#include "PICO_AnchorComponent.h"
#include "PICO_MRAsyncActions.h"
#include "PICO_MRFunctionLibrary.h"

// Sets default values
ASpatialAnchorActorPICO::ASpatialAnchorActorPICO(): SpatialAnchorComponent(nullptr)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void ASpatialAnchorActorPICO::CreateSpatialAnchorAsync_Internal()
{
	if (SpatialAnchorComponent&&!SpatialAnchorComponent->IsAnchorValid())
	{
		UCreateSpatialAnchorPICO_AsyncAction* CreateSpatialAnchorAction = UCreateSpatialAnchorPICO_AsyncAction::CreateSpatialAnchorPICO_Async(this,this->GetActorTransform());
		CreateSpatialAnchorAction->OnSuccess.AddDynamic(this, &ASpatialAnchorActorPICO::HandleCreateSpatialAnchorEvent);
		CreateSpatialAnchorAction->Activate();
	}
	else
	{
		FSpatialUUIDPICO UUID=FSpatialUUIDPICO();
		SaveDelegate.ExecuteIfBound(UUID,EResultPICO::XR_Error_ValidationFailure);
	}
}

void ASpatialAnchorActorPICO::PersistSpatialAnchorAsync_Internal()
{
	UPersistSpatialAnchorPICO_AsyncAction* PersistSpatialAnchorAction = UPersistSpatialAnchorPICO_AsyncAction::PersistSpatialAnchorPICO_Async(this);
	PersistSpatialAnchorAction->OnSuccess.AddDynamic(this, &ASpatialAnchorActorPICO::HandlePersistSpatialAnchorEvent);
	PersistSpatialAnchorAction->Activate();
}

void ASpatialAnchorActorPICO::UnpersistSpatialAnchorAsync_Internal()
{
	if (SpatialAnchorComponent&&SpatialAnchorComponent->IsAnchorValid())
	{
		UUnpersistSpatialAnchorPICO_AsyncAction* UnpersistSpatialAnchorAction = UUnpersistSpatialAnchorPICO_AsyncAction::UnpersistSpatialAnchorPICO_Async(this);
		UnpersistSpatialAnchorAction->OnSuccess.AddDynamic(this, &ASpatialAnchorActorPICO::HandleUnpersistSpatialAnchorEvent);
		UnpersistSpatialAnchorAction->Activate();
	}
	else
	{
		FSpatialUUIDPICO UUID=FSpatialUUIDPICO();
		DeleteDelegate.ExecuteIfBound(UUID,EResultPICO::XR_Error_ValidationFailure);
	}
}

void ASpatialAnchorActorPICO::SaveSpatialAnchorAsync(const FPXRSpatialAnchorEventDelegate& OnSaveFinished)
{
	SaveDelegate = OnSaveFinished;
	CreateSpatialAnchorAsync_Internal();
}

void ASpatialAnchorActorPICO::DeleteSpatialAnchorAsync(const FPXRSpatialAnchorEventDelegate& OnDeleteFinished)
{
	DeleteDelegate = OnDeleteFinished;
	UnpersistSpatialAnchorAsync_Internal();
}

// Called when the game starts or when spawned
void ASpatialAnchorActorPICO::BeginPlay()
{
	Super::BeginPlay();

	if (!SpatialAnchorComponent)
	{
		SpatialAnchorComponent = Cast<UAnchorComponentPICO>(AddComponentByClass(UAnchorComponentPICO::StaticClass(), false, FTransform::Identity, false));
	}
}

// Called every frame
void ASpatialAnchorActorPICO::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASpatialAnchorActorPICO::HandleCreateSpatialAnchorEvent(EResultPICO Result, const UAnchorComponentPICO* AnchorComponent)
{
	HandleAnchorCreated(Result);

	if (Result==EResultPICO::XR_Success)
	{
		PersistSpatialAnchorAsync_Internal();
	}
	else
	{
		FSpatialUUIDPICO UUID=FSpatialUUIDPICO();
		SaveDelegate.ExecuteIfBound(UUID,EResultPICO::XR_Error_ValidationFailure);
	}
	
}

void ASpatialAnchorActorPICO::HandlePersistSpatialAnchorEvent(EResultPICO Result, const UAnchorComponentPICO* AnchorComponent)
{
	HandleAnchorPersisted(Result);

	if (Result==EResultPICO::XR_Success)
	{
		FSpatialUUIDPICO OutAnchorUUID;
		EResultPICO OutResult;
		UMRFunctionLibraryPICO::GetAnchorEntityUuidByComponentPICO(AnchorComponent,OutAnchorUUID,OutResult);
		SaveDelegate.ExecuteIfBound(OutAnchorUUID,Result);
	}
	else
	{
		FSpatialUUIDPICO UUID=FSpatialUUIDPICO();
		SaveDelegate.ExecuteIfBound(UUID,Result);
	}

}

void ASpatialAnchorActorPICO::HandleUnpersistSpatialAnchorEvent(EResultPICO Result, const UAnchorComponentPICO* AnchorComponent)
{
	HandleAnchorUnpersisted(Result);

	if (Result==EResultPICO::XR_Success)
	{
		FSpatialUUIDPICO OutAnchorUUID;
		EResultPICO OutResult;
		UMRFunctionLibraryPICO::GetAnchorEntityUuidByComponentPICO(AnchorComponent,OutAnchorUUID,OutResult);
		DeleteDelegate.ExecuteIfBound(OutAnchorUUID,Result);
	}
	else
	{
		FSpatialUUIDPICO UUID=FSpatialUUIDPICO();
		DeleteDelegate.ExecuteIfBound(UUID,Result);
	}
}

