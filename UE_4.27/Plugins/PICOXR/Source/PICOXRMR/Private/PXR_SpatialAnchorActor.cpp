// Fill out your copyright notice in the Description page of Project Settings.


#include "PXR_SpatialAnchorActor.h"

#include "PXR_AnchorComponent.h"
#include "PXR_MRAsyncActions.h"
#include "PXR_MRFunctionLibrary.h"

// Sets default values
APICOXRSpatialAnchorActor::APICOXRSpatialAnchorActor(): SpatialAnchorComponent(nullptr)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void APICOXRSpatialAnchorActor::CreateSpatialAnchorAsync_Internal()
{
	if (SpatialAnchorComponent&&!SpatialAnchorComponent->IsAnchorValid())
	{
		UPICOCreateSpatialAnchor_AsyncAction* CreateSpatialAnchorAction = UPICOCreateSpatialAnchor_AsyncAction::PXR_CreateSpatialAnchor_Async(this,this->GetActorTransform());
		CreateSpatialAnchorAction->OnSuccess.AddDynamic(this, &APICOXRSpatialAnchorActor::HandleCreateSpatialAnchorEvent);
		CreateSpatialAnchorAction->Activate();
	}
	else
	{
		FPICOSpatialUUID UUID=FPICOSpatialUUID();
		SaveDelegate.ExecuteIfBound(UUID,EPICOResult::PXR_Error_ValidationFailure);
	}
}

void APICOXRSpatialAnchorActor::PersistSpatialAnchorAsync_Internal()
{
	UPICOPersistSpatialAnchor_AsyncAction* PersistSpatialAnchorAction = UPICOPersistSpatialAnchor_AsyncAction::PXR_PersistSpatialAnchor_Async(this);
	PersistSpatialAnchorAction->OnSuccess.AddDynamic(this, &APICOXRSpatialAnchorActor::HandlePersistSpatialAnchorEvent);
	PersistSpatialAnchorAction->Activate();
}

void APICOXRSpatialAnchorActor::UnpersistSpatialAnchorAsync_Internal()
{
	if (SpatialAnchorComponent&&SpatialAnchorComponent->IsAnchorValid())
	{
		UPICOUnpersistSpatialAnchor_AsyncAction* UnpersistSpatialAnchorAction = UPICOUnpersistSpatialAnchor_AsyncAction::PXR_UnpersistSpatialAnchor_Async(this);
		UnpersistSpatialAnchorAction->OnSuccess.AddDynamic(this, &APICOXRSpatialAnchorActor::HandleUnpersistSpatialAnchorEvent);
		UnpersistSpatialAnchorAction->Activate();
	}
	else
	{
		FPICOSpatialUUID UUID=FPICOSpatialUUID();
		DeleteDelegate.ExecuteIfBound(UUID,EPICOResult::PXR_Error_ValidationFailure);
	}
}

void APICOXRSpatialAnchorActor::SaveSpatialAnchorAsync(const FPXRSpatialAnchorEventDelegate& OnSaveFinished)
{
	SaveDelegate = OnSaveFinished;
	CreateSpatialAnchorAsync_Internal();
}

void APICOXRSpatialAnchorActor::DeleteSpatialAnchorAsync(const FPXRSpatialAnchorEventDelegate& OnDeleteFinished)
{
	DeleteDelegate = OnDeleteFinished;
	UnpersistSpatialAnchorAsync_Internal();
}

// Called when the game starts or when spawned
void APICOXRSpatialAnchorActor::BeginPlay()
{
	Super::BeginPlay();

	if (!SpatialAnchorComponent)
	{
		SpatialAnchorComponent = Cast<UPICOAnchorComponent>(AddComponentByClass(UPICOAnchorComponent::StaticClass(), false, FTransform::Identity, false));
	}
}

// Called every frame
void APICOXRSpatialAnchorActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APICOXRSpatialAnchorActor::HandleCreateSpatialAnchorEvent(EPICOResult Result,const UPICOAnchorComponent* AnchorComponent)
{
	HandleAnchorCreated(Result);

	if (Result==EPICOResult::PXR_Success)
	{
		PersistSpatialAnchorAsync_Internal();
	}
	else
	{
		FPICOSpatialUUID UUID=FPICOSpatialUUID();
		SaveDelegate.ExecuteIfBound(UUID,Result);
	}
	
}

void APICOXRSpatialAnchorActor::HandlePersistSpatialAnchorEvent(EPICOResult Result,const UPICOAnchorComponent* AnchorComponent)
{
	HandleAnchorPersisted(Result);

	if (Result==EPICOResult::PXR_Success)
	{
		FPICOSpatialUUID OutAnchorUUID;
		EPICOResult OutResult;
		UPICOXRMRFunctionLibrary::PXR_GetAnchorEntityUuidByComponent(AnchorComponent,OutAnchorUUID,OutResult);
		SaveDelegate.ExecuteIfBound(OutAnchorUUID,Result);
	}
	else
	{
		FPICOSpatialUUID UUID=FPICOSpatialUUID();
		SaveDelegate.ExecuteIfBound(UUID,Result);
	}

}

void APICOXRSpatialAnchorActor::HandleUnpersistSpatialAnchorEvent(EPICOResult Result,const UPICOAnchorComponent* AnchorComponent)
{
	HandleAnchorUnpersisted(Result);

	if (Result==EPICOResult::PXR_Success)
	{
		FPICOSpatialUUID OutAnchorUUID;
		EPICOResult OutResult;
		UPICOXRMRFunctionLibrary::PXR_GetAnchorEntityUuidByComponent(AnchorComponent,OutAnchorUUID,OutResult);
		DeleteDelegate.ExecuteIfBound(OutAnchorUUID,Result);
	}
	else
	{
		FPICOSpatialUUID UUID=FPICOSpatialUUID();

		DeleteDelegate.ExecuteIfBound(UUID,Result);
	}
}

