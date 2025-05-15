// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#include "PXR_AsyncAnchorAction.h"
#include "PXR_AnchorManager.h"

//////////////////////////////////////////////////////////////////////////
/// Create Anchor Entity
//////////////////////////////////////////////////////////////////////////
void UPICOCreateAnchorEntity_AsyncAction::Activate()
{
	if (!IsValid(BindingActor))
	{
		OnFailure.Broadcast(EPICOResult::PXR_Error_ValidationFailure);
		return;
	}

	EPICOResult Result = EPICOResult::PXR_Error_ValidationFailure;
	bool bStarted = FPICOAnchorManager::GetInstance()->CreateAnchorEntity(
		BindingActor,
		AnchorEntityTransform,
		Timeout,
		FPICOCreateAnchorEntityDelegate::CreateUObject(this, &UPICOCreateAnchorEntity_AsyncAction::HandleCreateAnchorEntityComplete),
		Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

UPICOCreateAnchorEntity_AsyncAction* UPICOCreateAnchorEntity_AsyncAction::PXR_CreateAnchorEntity_Async(AActor* InBindingActor, const FTransform& InAnchorEntityTransform, float InTimeout)
{
	UPICOCreateAnchorEntity_AsyncAction* Action = NewObject<UPICOCreateAnchorEntity_AsyncAction>();
	Action->BindingActor = InBindingActor;
	Action->AnchorEntityTransform = InAnchorEntityTransform;
	Action->Timeout = InTimeout;

	if (IsValid(InBindingActor))
	{
		Action->RegisterWithGameInstance(InBindingActor->GetWorld());
	}
	else
	{
		Action->RegisterWithGameInstance(GWorld);
	}

	return Action;
}

void UPICOCreateAnchorEntity_AsyncAction::HandleCreateAnchorEntityComplete(EPICOResult Result, UPICOAnchorComponent* AnchorEntityComponent)
{
	if (PXR_SUCCESS(Result))
	{
		OnSuccess.Broadcast(Result, AnchorEntityComponent);
	}
	else
	{
		OnFailure.Broadcast(Result);
	}

	SetReadyToDestroy();
}


//////////////////////////////////////////////////////////////////////////
/// Destroy Anchor Entity
//////////////////////////////////////////////////////////////////////////
void UPICODestroyAnchorEntityAction::Activate()
{
	if (!IsValid(BoundActor))
	{
		OnFailure.Broadcast(EPICOResult::PXR_Error_ValidationFailure);
		return;
	}

	EPICOResult Result = EPICOResult::PXR_Error_ValidationFailure;
	bool bStarted = FPICOAnchorManager::GetInstance()->DestroyAnchorEntity(
		BoundActor,
		FPICODestroyAnchorEntityDelegate::CreateUObject(this, &UPICODestroyAnchorEntityAction::HandleDestroyAnchorEntityComplete),
		Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

UPICODestroyAnchorEntityAction* UPICODestroyAnchorEntityAction::PXR_DestroyAnchorEntity(AActor* InBoundActor)
{
	UPICODestroyAnchorEntityAction* Action = NewObject<UPICODestroyAnchorEntityAction>();
	Action->BoundActor = InBoundActor;

	if (IsValid(InBoundActor))
	{
		Action->RegisterWithGameInstance(InBoundActor->GetWorld());
	}
	else
	{
		Action->RegisterWithGameInstance(GWorld);
	}

	return Action;
}

void UPICODestroyAnchorEntityAction::HandleDestroyAnchorEntityComplete(EPICOResult Result)
{
	if (PXR_SUCCESS(Result))
	{
		OnSuccess.Broadcast(Result);
	}
	else
	{
		OnFailure.Broadcast(Result);
	}

	SetReadyToDestroy();
}


//////////////////////////////////////////////////////////////////////////
/// Persist Anchor Entity
//////////////////////////////////////////////////////////////////////////
void UPICOPersistAnchorEntity_AsyncAction::Activate()
{
	for (AActor* BoundActor : BoundActors)
	{
		if (!IsValid(BoundActor))
		{
			OnFailure.Broadcast(EPICOResult::PXR_Error_ValidationFailure);
			return;
		}
	}

	EPICOResult Result = EPICOResult::PXR_Error_ValidationFailure;
	bool bStarted = FPICOAnchorManager::GetInstance()->PersistAnchorEntity(
		BoundActors,
		PersistLocation,
		FPICOPersistAnchorEntityDelegate::CreateUObject(this, &UPICOPersistAnchorEntity_AsyncAction::HandlePersistAnchorEntityComplete),
		Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

UPICOPersistAnchorEntity_AsyncAction* UPICOPersistAnchorEntity_AsyncAction::PXR_PersistAnchorEntity_Async(const TArray<AActor*>& InBoundActors, EPICOPersistLocation InPersistLocation)
{
	UPICOPersistAnchorEntity_AsyncAction* Action = NewObject<UPICOPersistAnchorEntity_AsyncAction>();
	Action->BoundActors = InBoundActors;
	Action->PersistLocation = InPersistLocation;

	if (InBoundActors.Num() > 0 && IsValid(InBoundActors[0]))
	{
		Action->RegisterWithGameInstance(InBoundActors[0]->GetWorld());
	}
	else
	{
		Action->RegisterWithGameInstance(GWorld);
	}

	return Action;
}

void UPICOPersistAnchorEntity_AsyncAction::HandlePersistAnchorEntityComplete(EPICOResult Result, const TArray<UPICOAnchorComponent*>& AnchorList)
{
	if (PXR_SUCCESS(Result))
	{
		OnSuccess.Broadcast(Result, AnchorList);
	}
	else
	{
		OnFailure.Broadcast(Result);
	}

	SetReadyToDestroy();
}


//////////////////////////////////////////////////////////////////////////
/// Unpersist Anchor Entity
//////////////////////////////////////////////////////////////////////////
void UPICOUnpersistAnchorEntity_AsyncAction::Activate()
{
	for (AActor* BoundActor : BoundActors)
	{
		if (!IsValid(BoundActor))
		{
			OnFailure.Broadcast(EPICOResult::PXR_Error_ValidationFailure);
			return;
		}
	}

	EPICOResult Result = EPICOResult::PXR_Error_ValidationFailure;
	bool bStarted = FPICOAnchorManager::GetInstance()->UnpersistAnchorEntity(
		BoundActors,
		PersistLocation,
		FPICOUnpersistAnchorEntityDelegate::CreateUObject(this, &UPICOUnpersistAnchorEntity_AsyncAction::HandleUnpersistAnchorEntityComplete),
		Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

UPICOUnpersistAnchorEntity_AsyncAction* UPICOUnpersistAnchorEntity_AsyncAction::PXR_UnpersistAnchorEntity_Async(const TArray<AActor*>& InBoundActors, EPICOPersistLocation InPersistLocation)
{
	UPICOUnpersistAnchorEntity_AsyncAction* Action = NewObject<UPICOUnpersistAnchorEntity_AsyncAction>();
	Action->BoundActors = InBoundActors;
	Action->PersistLocation = InPersistLocation;

	if (InBoundActors.Num() > 0 && IsValid(InBoundActors[0]))
	{
		Action->RegisterWithGameInstance(InBoundActors[0]->GetWorld());
	}
	else
	{
		Action->RegisterWithGameInstance(GWorld);
	}

	return Action;
}

void UPICOUnpersistAnchorEntity_AsyncAction::HandleUnpersistAnchorEntityComplete(EPICOResult Result, const TArray<UPICOAnchorComponent*>& AnchorList)
{
	if (PXR_SUCCESS(Result))
	{
		OnSuccess.Broadcast(Result, AnchorList);
	}
	else
	{
		OnFailure.Broadcast(Result);
	}

	SetReadyToDestroy();
}


//////////////////////////////////////////////////////////////////////////
/// Clear Anchor Entity
//////////////////////////////////////////////////////////////////////////
void UPICOClearAnchorEntity_AsyncAction::Activate()
{
	EPICOResult Result = EPICOResult::PXR_Error_ValidationFailure;
	bool bStarted = FPICOAnchorManager::GetInstance()->ClearAnchorEntity(
		PersistLocation,
		FPICOClearAnchorEntityDelegate::CreateUObject(this, &UPICOClearAnchorEntity_AsyncAction::HandleClearAnchorEntityComplete),
		Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

UPICOClearAnchorEntity_AsyncAction* UPICOClearAnchorEntity_AsyncAction::PXR_ClearAnchorEntity_Async(EPICOPersistLocation InPersistLocation)
{
	UPICOClearAnchorEntity_AsyncAction* Action = NewObject<UPICOClearAnchorEntity_AsyncAction>();
	Action->PersistLocation = InPersistLocation;
	Action->RegisterWithGameInstance(GWorld);
	return Action;
}

void UPICOClearAnchorEntity_AsyncAction::HandleClearAnchorEntityComplete(EPICOResult Result)
{
	if (PXR_SUCCESS(Result))
	{
		OnSuccess.Broadcast(Result);
	}
	else
	{
		OnFailure.Broadcast(Result);
	}

	SetReadyToDestroy();
}


//////////////////////////////////////////////////////////////////////////
/// Load Anchor Entity
//////////////////////////////////////////////////////////////////////////
void UPICOLoadAnchorEntity_AsyncAction::Activate()
{
	EPICOResult Result = EPICOResult::PXR_Error_ValidationFailure;
	bool bStarted = FPICOAnchorManager::GetInstance()->LoadAnchorEntity(
		LoadInfo,
		FPICOLoadAnchorEntityDelegate::CreateUObject(this, &UPICOLoadAnchorEntity_AsyncAction::HandleLoadAnchorEntityComplete),
		Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

UPICOLoadAnchorEntity_AsyncAction* UPICOLoadAnchorEntity_AsyncAction::PXR_LoadAnchorEntity_Async(const FPICOAnchorLoadInfo& InLoadInfo)
{
	UPICOLoadAnchorEntity_AsyncAction* Action = NewObject<UPICOLoadAnchorEntity_AsyncAction>();
	Action->LoadInfo = InLoadInfo;
	Action->RegisterWithGameInstance(GWorld);
	return Action;
}

void UPICOLoadAnchorEntity_AsyncAction::HandleLoadAnchorEntityComplete(EPICOResult Result, const TArray<FAnchorLoadResult>& AnchorLoadResults)
{
	if (PXR_SUCCESS(Result))
	{
		OnSuccess.Broadcast(Result, AnchorLoadResults);
	}
	else
	{
		OnFailure.Broadcast(Result);
	}

	SetReadyToDestroy();
}


//////////////////////////////////////////////////////////////////////////
/// Start Spatial Scene Capture
//////////////////////////////////////////////////////////////////////////
void UPICOStartSpatialSceneCapture_AsyncAction::Activate()
{
	EPICOResult Result = EPICOResult::PXR_Error_ValidationFailure;
	bool bStarted = FPICOAnchorManager::GetInstance()->StartSpatialSceneCapture(
		FPICOStartSpatialSceneCaptureDelegate::CreateUObject(this, &UPICOStartSpatialSceneCapture_AsyncAction::HandleStartSpatialSceneCaptureComplete),
		Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

UPICOStartSpatialSceneCapture_AsyncAction* UPICOStartSpatialSceneCapture_AsyncAction::PXR_StartSpatialSceneCapture_Async()
{
	UPICOStartSpatialSceneCapture_AsyncAction* Action = NewObject<UPICOStartSpatialSceneCapture_AsyncAction>();
	Action->RegisterWithGameInstance(GWorld);
	return Action;
}

void UPICOStartSpatialSceneCapture_AsyncAction::HandleStartSpatialSceneCaptureComplete(EPICOResult Result, EPICOSpatialSceneCaptureStatus SpatialSceneCaptureStatus)
{
	if (PXR_SUCCESS(Result))
	{
		OnSuccess.Broadcast(Result, SpatialSceneCaptureStatus);
	}
	else
	{
		OnFailure.Broadcast(Result);
	}

	SetReadyToDestroy();
}
