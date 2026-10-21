// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_MRAsyncActions.h"

#include "PICOOpenXRRuntimeSettings.h"
#include "PICO_LightEstimation.h"
#include "PICO_MR.h"
#include "PICO_SceneCapture.h"
#include "PICO_SpatialAnchor.h"
#include "PICO_SpatialMesh.h"
#include "PICO_SpatialPlane.h"
#include "Async/Async.h"
#include "Engine/World.h"
#include "Engine/TextureRenderTargetCube.h"

void URequestSceneCapturesPICO_AsyncAction::Activate()
{
	EResultPICO OutResult = EResultPICO::XR_Error_RuntimeFailure;
	
	const bool bStarted = FSceneCaptureExtensionPICO::GetInstance()->RequestSpatialSceneInfos
	(
		SceneLoadInfo,
		FPICOPollFutureDelegate::CreateUObject(this, &URequestSceneCapturesPICO_AsyncAction::HandleRequestSpatialSceneComplete),
		OutResult
	);
	
	if (!bStarted)
	{
		OnFailure.Broadcast(OutResult);
	}
}
URequestSceneCapturesPICO_AsyncAction* URequestSceneCapturesPICO_AsyncAction::RequestSceneCapturesPICO_Async(const FSceneLoadInfoPICO& InSceneLoadInfo)
{
	URequestSceneCapturesPICO_AsyncAction* Action = NewObject<URequestSceneCapturesPICO_AsyncAction>();
	Action->SceneLoadInfo = InSceneLoadInfo;
	Action->RegisterWithGameInstance(GWorld);

	return Action;
}

void URequestSceneCapturesPICO_AsyncAction::HandleRequestSpatialSceneComplete(const XrFutureEXT& FutureHandle)
{
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;
	TArray<FMRSceneInfoPICO> SceneLoadInfos;
	
	if (FSceneCaptureExtensionPICO::GetInstance()->GetSpatialSceneInfos(FutureHandle, SceneLoadInfos, OutResult))
	{
		OnSuccess.Broadcast(OutResult, SceneLoadInfos);
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}
	
	SetReadyToDestroy();
}


void URequestSpatialMeshPICO_AsyncAction::Activate()
{
	EResultPICO Result = EResultPICO::XR_Error_Unknown_PICO;

	const bool bStarted = FSpatialMeshExtensionPICO::GetInstance()->RequestSpatialTriangleMesh
	(
		FPICOPollFutureDelegate::CreateUObject(this, &URequestSpatialMeshPICO_AsyncAction::HandleRequestSpatialMeshComplete),Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

URequestSpatialMeshPICO_AsyncAction* URequestSpatialMeshPICO_AsyncAction::RequestSpatialMeshInfosPICO_Async()
{
	URequestSpatialMeshPICO_AsyncAction* Action = NewObject<URequestSpatialMeshPICO_AsyncAction>();
	Action->RegisterWithGameInstance(GWorld);

	return Action;
}

void URequestSpatialMeshPICO_AsyncAction::HandleRequestSpatialMeshComplete(const XrFutureEXT& FutureHandle)
{
	AsyncTask(ENamedThreads::GameThread, [FutureHandle,this]()
	{
		TArray<FSpatialMeshInfoPICO> MeshInfos;
		TSet<FSpatialUUIDPICO> NeedDeleteEntitySet;
		EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;
		if (FSpatialMeshExtensionPICO::GetInstance()->GetSpatialTriangleMeshInfos(FutureHandle,MeshInfos, OutResult))
		{
			OnSuccess.Broadcast(OutResult, MeshInfos);
		}
		else
		{
			OnFailure.Broadcast(OutResult);
		}
		SetReadyToDestroy();
	});
}

void ULaunchSpatialMeshScanningPICO_AsyncAction::Activate()
{
	if(!FSpatialMeshExtensionPICO::GetInstance()->IsHandleValid())
	{
		const UPICOOpenXRRuntimeSettings* Settings = GetDefault<UPICOOpenXRRuntimeSettings>();

		if (Settings->bEnableMesh)
		{
			FSenseDataProviderCreateInfoMeshPICO cFPICOSenseDataProviderCreateInfoMesh = {};
			cFPICOSenseDataProviderCreateInfoMesh.Lod = Settings->MeshLod;
			if (Settings->bSemanticsAlignWithTriangle)
			{
				cFPICOSenseDataProviderCreateInfoMesh.ConfigArray.Add(ESpatialMeshConfigPICO::Spatial_Mesh_Config_Semantic);
			}
			else if (Settings->bSemanticsAlignWithVertex)
			{
				cFPICOSenseDataProviderCreateInfoMesh.ConfigArray.Add(ESpatialMeshConfigPICO::Spatial_Mesh_Config_Semantic);
				cFPICOSenseDataProviderCreateInfoMesh.ConfigArray.Add(ESpatialMeshConfigPICO::Spatial_Mesh_Config_Semantic_Align_With_Vertex);
			}
			EResultPICO OutResult=EResultPICO::XR_Error_Unknown_PICO;
			FSpatialMeshExtensionPICO::GetInstance()->CreateProvider(cFPICOSenseDataProviderCreateInfoMesh,OutResult);
		}
	}
	
	EResultPICO Result=EResultPICO::XR_Error_Unknown_PICO;
	const bool bStarted = FSpatialMeshExtensionPICO::GetInstance()->StartProvider
	(
		FPICOPollFutureDelegate::CreateUObject(this,
		&ULaunchSpatialMeshScanningPICO_AsyncAction::HandleLaunchMeshScanningComplete)
		,Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

ULaunchSpatialMeshScanningPICO_AsyncAction* ULaunchSpatialMeshScanningPICO_AsyncAction::LaunchSpatialMeshScanningPICO_Async()
{
	ULaunchSpatialMeshScanningPICO_AsyncAction* Action = NewObject<ULaunchSpatialMeshScanningPICO_AsyncAction>();

	Action->RegisterWithGameInstance(GWorld);

	return Action;
}

void ULaunchSpatialMeshScanningPICO_AsyncAction::HandleLaunchMeshScanningComplete(const XrFutureEXT& FutureHandle)
{
	FSenseDataProviderStartCompletionPICO SenseDataProviderStartCompletion;
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;

	if (FSpatialSensingExtensionPICO::GetInstance()->StartSenseDataProviderComplete(FutureHandle, SenseDataProviderStartCompletion,OutResult))
	{
		if (PXR_SUCCESS(SenseDataProviderStartCompletion.FutureResult))
		{
			OnSuccess.Broadcast(SenseDataProviderStartCompletion.FutureResult);
		}
		else
		{
			OnFailure.Broadcast(SenseDataProviderStartCompletion.FutureResult);
		}
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}
	
	SetReadyToDestroy();
}

URequestSpatialPlanePICO_AsyncAction* URequestSpatialPlanePICO_AsyncAction::RequestSpatialPlaneInfosPICO_Async()
{
	URequestSpatialPlanePICO_AsyncAction* Action = NewObject<URequestSpatialPlanePICO_AsyncAction>();
	Action->RegisterWithGameInstance(GWorld);

	return Action;
}

void URequestSpatialPlanePICO_AsyncAction::HandleRequestSpatialPlaneComplete(const XrFutureEXT& FutureHandle)
{
	AsyncTask(ENamedThreads::GameThread, [FutureHandle,this]()
	{
		TArray<FSpatialPlaneInfoPICO> PlaneInfos;
		TSet<FSpatialUUIDPICO> NeedDeleteEntitySet;
		EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;
		if (FSpatialPlaneExtensionPICO::GetInstance()->GetSpatialTrianglePlaneInfos(FutureHandle,PlaneInfos, OutResult))
		{
			OnSuccess.Broadcast(OutResult, PlaneInfos);
		}
		else
		{
			OnFailure.Broadcast(OutResult);
		}
		SetReadyToDestroy();
	});
}

void ULaunchSpatialPlaneScanningPICO_AsyncAction::Activate()
{
	if(!FSpatialPlaneExtensionPICO::GetInstance()->IsHandleValid())
	{
		const UPICOOpenXRRuntimeSettings* Settings = GetDefault<UPICOOpenXRRuntimeSettings>();

		if (Settings->bEnablePlane)
		{
			FSenseDataProviderCreateInfoPlanePICO cFPICOSenseDataProviderCreateInfoPlane = {};
			
			EResultPICO OutResult=EResultPICO::XR_Error_Unknown_PICO;
			FSpatialPlaneExtensionPICO::GetInstance()->CreateProvider(cFPICOSenseDataProviderCreateInfoPlane,OutResult);
		}
	}
	
	EResultPICO Result=EResultPICO::XR_Error_Unknown_PICO;
	const bool bStarted = FSpatialPlaneExtensionPICO::GetInstance()->StartProvider
	(
		FPICOPollFutureDelegate::CreateUObject(this,
		&ULaunchSpatialPlaneScanningPICO_AsyncAction::HandleLaunchPlaneScanningComplete)
		,Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

ULaunchSpatialPlaneScanningPICO_AsyncAction* ULaunchSpatialPlaneScanningPICO_AsyncAction::LaunchSpatialPlaneScanningPICO_Async()
{
	ULaunchSpatialPlaneScanningPICO_AsyncAction* Action = NewObject<ULaunchSpatialPlaneScanningPICO_AsyncAction>();

	Action->RegisterWithGameInstance(GWorld);

	return Action;
}

void ULaunchSpatialPlaneScanningPICO_AsyncAction::HandleLaunchPlaneScanningComplete(const XrFutureEXT& FutureHandle)
{
	FSenseDataProviderStartCompletionPICO SenseDataProviderStartCompletion;
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;

	if (FSpatialSensingExtensionPICO::GetInstance()->StartSenseDataProviderComplete(FutureHandle, SenseDataProviderStartCompletion,OutResult))
	{
		if (PXR_SUCCESS(SenseDataProviderStartCompletion.FutureResult))
		{
			OnSuccess.Broadcast(SenseDataProviderStartCompletion.FutureResult);
		}
		else
		{
			OnFailure.Broadcast(SenseDataProviderStartCompletion.FutureResult);
		}
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}
	
	SetReadyToDestroy();
}

void URequestSpatialPlanePICO_AsyncAction::Activate()
{
	EResultPICO Result = EResultPICO::XR_Error_Unknown_PICO;

	const bool bStarted = FSpatialPlaneExtensionPICO::GetInstance()->RequestSpatialTrianglePlane
	(
		FPICOPollFutureDelegate::CreateUObject(this, &URequestSpatialPlanePICO_AsyncAction::HandleRequestSpatialPlaneComplete),Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

void ULaunchSpatialAnchorServicePICO_AsyncAction::Activate()
{
	if(!FSpatialAnchorExtensionPICO::GetInstance()->IsHandleValid())
	{
		const UPICOOpenXRRuntimeSettings* Settings = GetDefault<UPICOOpenXRRuntimeSettings>();
		if (Settings->bEnableAnchor)
		{
			FSenseDataProviderCreateInfoAnchorPICO cFPICOSenseDataProviderCreateInfoAnchor = {};
			EResultPICO OutResult=EResultPICO::XR_Error_Unknown_PICO;
			FSpatialAnchorExtensionPICO::GetInstance()->CreateProvider(cFPICOSenseDataProviderCreateInfoAnchor,OutResult);
		}
	}
	EResultPICO Result=EResultPICO::XR_Error_Unknown_PICO;
	const bool bStarted = FSpatialAnchorExtensionPICO::GetInstance()->StartProvider(
	FPICOPollFutureDelegate::CreateUObject(
		this,
		&ULaunchSpatialAnchorServicePICO_AsyncAction::HandleLaunchSpatialAnchorServiceComplete),
	Result
	);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

ULaunchSpatialAnchorServicePICO_AsyncAction* ULaunchSpatialAnchorServicePICO_AsyncAction::LaunchSpatialAnchorServicePICO_Async()
{
	ULaunchSpatialAnchorServicePICO_AsyncAction* Action = NewObject<ULaunchSpatialAnchorServicePICO_AsyncAction>();

	Action->RegisterWithGameInstance(GWorld);

	return Action;
}

void ULaunchSpatialAnchorServicePICO_AsyncAction::HandleLaunchSpatialAnchorServiceComplete(const XrFutureEXT& FutureHandle)
{
	FSenseDataProviderStartCompletionPICO SenseDataProviderStartCompletion;
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;

	if (FSpatialSensingExtensionPICO::GetInstance()->StartSenseDataProviderComplete(FutureHandle, SenseDataProviderStartCompletion,OutResult))
	{
		if (PXR_SUCCESS(SenseDataProviderStartCompletion.FutureResult))
		{
			OnSuccess.Broadcast(SenseDataProviderStartCompletion.FutureResult);
		}
		else
		{
			OnFailure.Broadcast(SenseDataProviderStartCompletion.FutureResult);
		}
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}
	

	SetReadyToDestroy();
}

void ULaunchSceneCaptureServicePICO_AsyncAction::Activate()
{
	if(!FSceneCaptureExtensionPICO::GetInstance()->IsHandleValid())
	{
		const UPICOOpenXRRuntimeSettings* Settings = GetDefault<UPICOOpenXRRuntimeSettings>();
		if (Settings->bEnableSceneCapture)
		{
			FSenseDataProviderCreateInfoSceneCapturePICO cSenseDataProviderCreateInfoSceneCapture = {};
			EResultPICO OutResult=EResultPICO::XR_Error_Unknown_PICO;
			FSceneCaptureExtensionPICO::GetInstance()->CreateProvider(cSenseDataProviderCreateInfoSceneCapture,OutResult);
		}
	}	
	
	EResultPICO Result=EResultPICO::XR_Error_Unknown_PICO;

	const bool bStarted = FSceneCaptureExtensionPICO::GetInstance()->StartProvider(
		FPICOPollFutureDelegate::CreateUObject(this,
		&ULaunchSceneCaptureServicePICO_AsyncAction::HandleLaunchSceneCaptureServiceComplete)
		, Result);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

ULaunchSceneCaptureServicePICO_AsyncAction* ULaunchSceneCaptureServicePICO_AsyncAction::LaunchSceneCaptureServicePICO_Async()
{
	ULaunchSceneCaptureServicePICO_AsyncAction* Action = NewObject<ULaunchSceneCaptureServicePICO_AsyncAction>();

	Action->RegisterWithGameInstance(GWorld);

	return Action;
}

void ULaunchSceneCaptureServicePICO_AsyncAction::HandleLaunchSceneCaptureServiceComplete(const XrFutureEXT& FutureHandle)
{
	FSenseDataProviderStartCompletionPICO SenseDataProviderStartCompletion;
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;

	if (FSpatialSensingExtensionPICO::GetInstance()->StartSenseDataProviderComplete(FutureHandle, SenseDataProviderStartCompletion,OutResult))
	{
		if (PXR_SUCCESS(SenseDataProviderStartCompletion.FutureResult))
		{
			OnSuccess.Broadcast(SenseDataProviderStartCompletion.FutureResult);
		}
		else
		{
			OnFailure.Broadcast(SenseDataProviderStartCompletion.FutureResult);
		}
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}

	SetReadyToDestroy();
}

void ULaunchSceneCaptureAppPICO_AsyncAction::HandleLaunchSceneCaptureAppComplete(const XrFutureEXT& FutureHandle)
{
	FSceneCaptureStartCompletionPICO cPICOSceneCaptureStartCompletion;
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;

	if (FSceneCaptureExtensionPICO::GetInstance()->StartSceneCaptureComplete(FutureHandle, cPICOSceneCaptureStartCompletion,OutResult))
	{
		if (PXR_SUCCESS(cPICOSceneCaptureStartCompletion.FutureResult))
		{
			OnSuccess.Broadcast(cPICOSceneCaptureStartCompletion.FutureResult);
		}
		else
		{
			OnFailure.Broadcast(cPICOSceneCaptureStartCompletion.FutureResult);
		}
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}
	

	SetReadyToDestroy();
}

ULaunchSceneCaptureAppPICO_AsyncAction* ULaunchSceneCaptureAppPICO_AsyncAction::LaunchSceneCaptureAppPICO_Async()
{
	ULaunchSceneCaptureAppPICO_AsyncAction* Action = NewObject<ULaunchSceneCaptureAppPICO_AsyncAction>();

	Action->RegisterWithGameInstance(GWorld);

	return Action;
}

void ULaunchSceneCaptureAppPICO_AsyncAction::Activate()
{
	EResultPICO Result = EResultPICO::XR_Error_Unknown_PICO;
	
	bool bStarted = FSceneCaptureExtensionPICO::GetInstance()->StartSceneCaptureAsync
	(
		FPICOPollFutureDelegate::CreateUObject(this, &ULaunchSceneCaptureAppPICO_AsyncAction::HandleLaunchSceneCaptureAppComplete),
		Result
	);
	

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

//////////////////////////////////////////////////////////////////////////
/// Create Anchor Entity
//////////////////////////////////////////////////////////////////////////
void UCreateSpatialAnchorPICO_AsyncAction::Activate()
{
	if (!IsValid(BoundActor) || !IsValid(BoundActor->GetWorld()))
	{
		OnFailure.Broadcast(EResultPICO::XR_Error_ValidationFailure);
		return;
	}

	EResultPICO Result = EResultPICO::XR_Error_Unknown_PICO;
	bool bStarted = FSpatialAnchorExtensionPICO::GetInstance()->CreateSpatialAnchorAsync
	(
		FPICOPollFutureDelegate::CreateUObject(this, &UCreateSpatialAnchorPICO_AsyncAction::HandleCreateSpatialAnchorComplete)
		, SpatialAnchorTransform
		,Result
	);

	
	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

UCreateSpatialAnchorPICO_AsyncAction* UCreateSpatialAnchorPICO_AsyncAction::CreateSpatialAnchorPICO_Async(AActor* BoundActor, const FTransform& InSpatialAnchorTransform)
{
	UCreateSpatialAnchorPICO_AsyncAction* Action = NewObject<UCreateSpatialAnchorPICO_AsyncAction>();
	Action->BoundActor = BoundActor;
	Action->SpatialAnchorTransform = InSpatialAnchorTransform;

	if (IsValid(BoundActor))
	{
		Action->RegisterWithGameInstance(BoundActor->GetWorld());
	}
	else
	{
		Action->RegisterWithGameInstance(GWorld);
	}

	return Action;
}

void UCreateSpatialAnchorPICO_AsyncAction::HandleCreateSpatialAnchorComplete(const XrFutureEXT& FutureHandle)
{
	FSpatialAnchorCreateCompletionPICO SpatialAnchorCreateCompletion;
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;

	if(FSpatialAnchorExtensionPICO::GetInstance()->CreateSpatialAnchorComplete(FutureHandle, SpatialAnchorCreateCompletion,OutResult))
	{
		UAnchorComponentPICO* AnchorComponent = Cast<UAnchorComponentPICO>(BoundActor->GetComponentByClass(UAnchorComponentPICO::StaticClass()));
		if (!AnchorComponent)
		{
			AnchorComponent = Cast<UAnchorComponentPICO>(BoundActor->AddComponentByClass(UAnchorComponentPICO::StaticClass(), false, FTransform::Identity, false));
		}
		AnchorComponent->SetAnchorHandle(SpatialAnchorCreateCompletion.AnchorHandle);

		if(PXR_SUCCESS(SpatialAnchorCreateCompletion.FutureResult))
		{
			OnSuccess.Broadcast(SpatialAnchorCreateCompletion.FutureResult,AnchorComponent);
		}
		else
		{
			OnFailure.Broadcast(SpatialAnchorCreateCompletion.FutureResult);
		}
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}

	SetReadyToDestroy();
}

//////////////////////////////////////////////////////////////////////////
/// Persist Anchor Entity
//////////////////////////////////////////////////////////////////////////
void UPersistSpatialAnchorPICO_AsyncAction::Activate()
{
	if (!IsValid(BoundActor))
	{
		OnFailure.Broadcast(EResultPICO::XR_Error_ValidationFailure);
		return;
	}
	EResultPICO Result = EResultPICO::XR_Error_Unknown_PICO;

	bool bStarted = false;

	switch (PersistLocation)
	{
	case EPersistLocationPICO::PersistLocation_Local:
		{
			bStarted = FSpatialAnchorExtensionPICO::GetInstance()->PersistSpatialAnchorAsync
			(
				BoundActor,
				PersistLocation,
				FPICOPollFutureDelegate::CreateUObject(this, &UPersistSpatialAnchorPICO_AsyncAction::HandlePersistSpatialAnchorComplete),
				Result
			);
		}
		break;
	case EPersistLocationPICO::PersistLocation_Shared:
		{
			bStarted = FSpatialAnchorExtensionPICO::GetInstance()->ShareSpatialAnchorAsync
			(
				BoundActor,
				FPICOPollFutureDelegate::CreateUObject(this, &UPersistSpatialAnchorPICO_AsyncAction::HandleShareSpatialAnchorComplete),
				Result
			);
		}
		break;
	default: ;
	}

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

UPersistSpatialAnchorPICO_AsyncAction* UPersistSpatialAnchorPICO_AsyncAction::PersistSpatialAnchorPICO_Async(AActor* InBoundActor, EPersistLocationPICO InPersistLocation)
{
	UPersistSpatialAnchorPICO_AsyncAction* Action = NewObject<UPersistSpatialAnchorPICO_AsyncAction>();
	Action->BoundActor = InBoundActor;
	Action->PersistLocation = InPersistLocation;

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

void UPersistSpatialAnchorPICO_AsyncAction::HandlePersistSpatialAnchorComplete(const XrFutureEXT& FutureHandle)
{
	FSpatialAnchorPersistCompletionPICO SpatialAnchorPersistCompletion;
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;

	if (FSpatialAnchorExtensionPICO::GetInstance()->PersistSpatialAnchorComplete(FutureHandle, SpatialAnchorPersistCompletion,OutResult))
	{
		UAnchorComponentPICO* AnchorComponent = Cast<UAnchorComponentPICO>(BoundActor->GetComponentByClass(UAnchorComponentPICO::StaticClass()));
		if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
		{
			if(PXR_SUCCESS(SpatialAnchorPersistCompletion.FutureResult))
			{
				OnSuccess.Broadcast(SpatialAnchorPersistCompletion.FutureResult,AnchorComponent);
			}
			else
			{
				OnFailure.Broadcast(SpatialAnchorPersistCompletion.FutureResult);
			}
		}
		else
		{
			OnFailure.Broadcast(EResultPICO::XR_Error_ValidationFailure);
		}
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}
	
	SetReadyToDestroy();
}

void UPersistSpatialAnchorPICO_AsyncAction::HandleShareSpatialAnchorComplete(const XrFutureEXT& FutureHandle)
{
	FSpatialAnchorShareCompletionPICO SpatialAnchorShareCompletion;
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;

	if (FSpatialAnchorExtensionPICO::GetInstance()->ShareSpatialAnchorComplete(FutureHandle, SpatialAnchorShareCompletion,OutResult))
	{
		UAnchorComponentPICO* AnchorComponent = Cast<UAnchorComponentPICO>(BoundActor->GetComponentByClass(UAnchorComponentPICO::StaticClass()));
		if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
		{
			if(PXR_SUCCESS(SpatialAnchorShareCompletion.FutureResult))
			{
				OnSuccess.Broadcast(SpatialAnchorShareCompletion.FutureResult,AnchorComponent);
			}
			else
			{
				OnFailure.Broadcast(SpatialAnchorShareCompletion.FutureResult);
			}
		}
		else
		{
			OnFailure.Broadcast(EResultPICO::XR_Error_ValidationFailure);
		}
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}
	
	SetReadyToDestroy();
}


//////////////////////////////////////////////////////////////////////////
/// Unpersist Anchor Entity
//////////////////////////////////////////////////////////////////////////
void UUnpersistSpatialAnchorPICO_AsyncAction::Activate()
{
	if (!IsValid(BoundActor))
	{
		OnFailure.Broadcast(EResultPICO::XR_Error_ValidationFailure);
		return;
	}
	bool bStarted=false;
	EResultPICO Result = EResultPICO::XR_Error_Unknown_PICO;
	
	bStarted = FSpatialAnchorExtensionPICO::GetInstance()->UnpersistSpatialAnchorAsync(
		BoundActor,
		FPICOPollFutureDelegate::CreateUObject(this, &UUnpersistSpatialAnchorPICO_AsyncAction::HandleUnpersistSpatialAnchorComplete),
		Result
		);


	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

UUnpersistSpatialAnchorPICO_AsyncAction* UUnpersistSpatialAnchorPICO_AsyncAction::UnpersistSpatialAnchorPICO_Async(AActor* BoundActor, EPersistLocationPICO InPersistLocation)
{
	UUnpersistSpatialAnchorPICO_AsyncAction* Action = NewObject<UUnpersistSpatialAnchorPICO_AsyncAction>();
	Action->BoundActor = BoundActor;
	Action->PersistLocation = InPersistLocation;

	if (IsValid(BoundActor))
	{
		Action->RegisterWithGameInstance(BoundActor->GetWorld());
	}
	else
	{
		Action->RegisterWithGameInstance(GWorld);
	}

	return Action;
}

void UUnpersistSpatialAnchorPICO_AsyncAction::HandleUnpersistSpatialAnchorComplete(const XrFutureEXT& FutureHandle)
{
	FSpatialAnchorUnpersistCompletionPICO SpatialAnchorUnpersistCompletion;
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;

	if(FSpatialAnchorExtensionPICO::GetInstance()->UnpersistSpatialAnchorComplete(FutureHandle, SpatialAnchorUnpersistCompletion,OutResult))
	{
		UAnchorComponentPICO* AnchorComponent = Cast<UAnchorComponentPICO>(BoundActor->GetComponentByClass(UAnchorComponentPICO::StaticClass()));
		if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
		{
			if(PXR_SUCCESS(SpatialAnchorUnpersistCompletion.FutureResult))
			{
				OnSuccess.Broadcast(SpatialAnchorUnpersistCompletion.FutureResult,AnchorComponent);
			}
			else
			{
				OnFailure.Broadcast(SpatialAnchorUnpersistCompletion.FutureResult);
			}
		}
		else
		{
			OnFailure.Broadcast(EResultPICO::XR_Error_ValidationFailure);
		}
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}
	
	SetReadyToDestroy();
}

//////////////////////////////////////////////////////////////////////////
/// Load Anchor Entity
//////////////////////////////////////////////////////////////////////////
void URequestSpatialAnchorsPICO_AsyncAction::Activate()
{
	bool bStarted = false;
	EResultPICO Result = EResultPICO::XR_Error_Unknown_PICO;
	
	FutureHandleSet.Empty();
	switch (LoadInfo.PersistLocation)
	{
	case EPersistLocationPICO::PersistLocation_Local:
		{
			bStarted = FSpatialAnchorExtensionPICO::GetInstance()->LoadAnchorEntityAsync
			(
				LoadInfo,
				FPICOPollFutureDelegate::CreateUObject(this, &URequestSpatialAnchorsPICO_AsyncAction::HandleLoadSpatialAnchorsComplete),
				Result
			);
		}
		break;
	case EPersistLocationPICO::PersistLocation_Shared:
		{
			bStarted = FSpatialAnchorExtensionPICO::GetInstance()->DownloadSharedSpatialAnchorsAsync
			(
				LoadInfo,
				FPICOPollFutureDelegate::CreateUObject(this,&URequestSpatialAnchorsPICO_AsyncAction::HandleDownloadSharedAnchorsComplete),
				FutureHandleSet,
				Result
			);
		}
		break;
	default: ;
	}

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

URequestSpatialAnchorsPICO_AsyncAction* URequestSpatialAnchorsPICO_AsyncAction::LoadSpatialAnchorPICO_Async(const FAnchorLoadInfoPICO& InLoadInfo)
{
	URequestSpatialAnchorsPICO_AsyncAction* Action = NewObject<URequestSpatialAnchorsPICO_AsyncAction>();
	Action->LoadInfo = InLoadInfo;
	Action->RegisterWithGameInstance(GWorld);
	return Action;
}

void URequestSpatialAnchorsPICO_AsyncAction::HandleLoadSpatialAnchorsComplete(const XrFutureEXT& FutureHandle)
{
	EResultPICO LoadResult=EResultPICO::XR_Error_Unknown_PICO;

	TArray<FAnchorLoadResultPICO> AnchorLoadResults;
	
	if (FSpatialAnchorExtensionPICO::GetInstance()->GetAnchorLoadResults(FutureHandle,AnchorLoadResults,LoadResult))
	{
		OnSuccess.Broadcast(LoadResult,AnchorLoadResults);
	}
	else
	{
		OnFailure.Broadcast(LoadResult);
	}

	SetReadyToDestroy();
}

void URequestSpatialAnchorsPICO_AsyncAction::HandleDownloadSharedAnchorsComplete(const XrFutureEXT& FutureHandle)
{
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;
	FSharedSpatialAnchorDownloadCompletionPICO SharedSpatialAnchorDownloadCompletion;
	FutureHandleSet.Remove(FutureHandle);
	if(FSpatialAnchorExtensionPICO::GetInstance()->DownloadSharedSpatialAnchorsComplete(FutureHandle, SharedSpatialAnchorDownloadCompletion,OutResult))
	{
		if(PXR_SUCCESS(SharedSpatialAnchorDownloadCompletion.FutureResult))
		{
			if (!FutureHandleSet.Num())
			{
				if(!FSpatialAnchorExtensionPICO::GetInstance()->LoadAnchorEntityAsync(
				LoadInfo,
				FPICOPollFutureDelegate::CreateUObject(this, &URequestSpatialAnchorsPICO_AsyncAction::HandleLoadSpatialAnchorsComplete),
				OutResult
				))
				{
					OnFailure.Broadcast(OutResult);
				}
			}
		}
		else
		{
			OnFailure.Broadcast(SharedSpatialAnchorDownloadCompletion.FutureResult);
		}
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}

	SetReadyToDestroy();
}

void ULaunchLightEstimationServicePICO_AsyncAction::Activate()
{
	Super::Activate();
	if(!FLightEstimationExtensionPICO::GetInstance()->IsHandleValid())
	{
		const UPICOOpenXRRuntimeSettings* Settings = GetDefault<UPICOOpenXRRuntimeSettings>();
		UE_LOG(LogMRPICO,Log,TEXT("ULaunchLightEstimationService bEnableLightEstimation:%d"),Settings->bEnableLightEstimation);

		if (Settings->bEnableLightEstimation)
		{
			FSenseDataProviderCreateInfoLightEstimationPICO CreateInfoLightEstimation = {};
			CreateInfoLightEstimation.Resolution=Settings->LightEstimationResolution;
			EResultPICO OutResult=EResultPICO::XR_Error_Unknown_PICO;
			FLightEstimationExtensionPICO::GetInstance()->CreateProvider(CreateInfoLightEstimation,OutResult);
		}
	}
	
	EResultPICO Result=EResultPICO::XR_Error_Unknown_PICO;

	const bool bStarted = FLightEstimationExtensionPICO::GetInstance()->StartProvider(
		FPICOPollFutureDelegate::CreateUObject(this,
		&ULaunchLightEstimationServicePICO_AsyncAction::HandleLaunchLightEstimationServiceComplete)
		, Result);

	if (!bStarted)
	{
		OnFailure.Broadcast(Result);
	}
}

ULaunchLightEstimationServicePICO_AsyncAction* ULaunchLightEstimationServicePICO_AsyncAction::LaunchLightEstimationServicePICO_Async()
{
	ULaunchLightEstimationServicePICO_AsyncAction* Action = NewObject<ULaunchLightEstimationServicePICO_AsyncAction>();

	Action->RegisterWithGameInstance(GWorld);

	return Action;
}

void ULaunchLightEstimationServicePICO_AsyncAction::HandleLaunchLightEstimationServiceComplete(
	const XrFutureEXT& FutureHandle)
{
	FSenseDataProviderStartCompletionPICO SenseDataProviderStartCompletion;
	EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;

	if (FSpatialSensingExtensionPICO::GetInstance()->StartSenseDataProviderComplete(FutureHandle, SenseDataProviderStartCompletion,OutResult))
	{
		if (PXR_SUCCESS(SenseDataProviderStartCompletion.FutureResult))
		{
			OnSuccess.Broadcast(SenseDataProviderStartCompletion.FutureResult);
		}
		else
		{
			OnFailure.Broadcast(SenseDataProviderStartCompletion.FutureResult);
		}
	}
	else
	{
		OnFailure.Broadcast(OutResult);
	}

	SetReadyToDestroy();
}

void URequestLightEstimationPICO_AsyncAction::Activate()
{
	EResultPICO OutResult = EResultPICO::XR_Error_RuntimeFailure;

	FIntPoint Size=FLightEstimationExtensionPICO::GetInstance()->GetLightEstimationSize();
	
	EnvironmentCaptureTexture = NewObject<ULightEstimationTexturePICO>(this);

	const bool bStarted = FLightEstimationExtensionPICO::GetInstance()->RequestLightEstimation
	(
		FPICOPollFutureDelegate::CreateUObject(this, &URequestLightEstimationPICO_AsyncAction::HandleRequestLightEstimationComplete),
		OutResult
	);
	
	if (!bStarted)
	{
		OnFailure.Broadcast(OutResult);
	}
}

URequestLightEstimationPICO_AsyncAction* URequestLightEstimationPICO_AsyncAction::RequestLightEstimationPICO_Async()
{
	URequestLightEstimationPICO_AsyncAction* Action = NewObject<URequestLightEstimationPICO_AsyncAction>();
	Action->RegisterWithGameInstance(GWorld);

	return Action;
}

void URequestLightEstimationPICO_AsyncAction::HandleRequestLightEstimationComplete(const XrFutureEXT& FutureHandle)
{
	AsyncTask(ENamedThreads::GameThread, [FutureHandle,this]()
	{
		EResultPICO OutResult = EResultPICO::XR_Error_Unknown_PICO;
		if (FLightEstimationExtensionPICO::GetInstance()->GetLightEstimationInfo(FutureHandle,EnvironmentCaptureTexture,SourceCubemapAngle,OutResult))
		{
			// Create the texture
			if (EnvironmentCaptureTexture)
			{
				OnSuccess.Broadcast(OutResult,EnvironmentCaptureTexture,SourceCubemapAngle);
			}
			else
			{
				OnFailure.Broadcast(OutResult);
			}
		}
		else
		{
			OnFailure.Broadcast(OutResult);
		}
	
		SetReadyToDestroy();
	});
}