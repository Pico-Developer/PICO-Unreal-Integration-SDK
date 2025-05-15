// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#include "PXR_AnchorManager.h"
#include "PXR_AnchorComponent.h"
#include "PXR_PluginWrapper.h"
#include "PXR_HMDModule.h"
#include "PXR_HMDPrivate.h"

FPICOAnchorManager::FPICOAnchorManager()
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager Construction");
	HandleOfCreateAnchorEntity = CreateAnchorEntityEventDelegate.AddRaw(this, &FPICOAnchorManager::HandleCreateAnchorEntityEvent);
	HandleOfPersistAnchorEntity = PersistAnchorEntityEventDelegate.AddRaw(this, &FPICOAnchorManager::HandlePersistAnchorEntityEvent);
	HandleOfUnpersistAnchorEntity = UnpersistAnchorEntityEventDelegate.AddRaw(this, &FPICOAnchorManager::HandleUnpersistAnchorEntityEvent);
	HandleOfClearAnchorEntity = ClearAnchorEntityEventDelegate.AddRaw(this, &FPICOAnchorManager::HandleClearAnchorEntityEvent);
	HandleOfLoadAnchorEntity = LoadAnchorEntityEventDelegate.AddRaw(this, &FPICOAnchorManager::HandleLoadAnchorEntityEvent);
	HandleOfStartSpatialSceneCapture = StartSpatialSceneCaptureEventDelegate.AddRaw(this, &FPICOAnchorManager::HandleStartSpatialSceneCaptureEvent);
}

FPICOAnchorManager::~FPICOAnchorManager()
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager Destruction");
	CreateAnchorEntityEventDelegate.Remove(HandleOfCreateAnchorEntity);
	PersistAnchorEntityEventDelegate.Remove(HandleOfPersistAnchorEntity);
	UnpersistAnchorEntityEventDelegate.Remove(HandleOfUnpersistAnchorEntity);
	ClearAnchorEntityEventDelegate.Remove(HandleOfClearAnchorEntity);
	LoadAnchorEntityEventDelegate.Remove(HandleOfLoadAnchorEntity);
	StartSpatialSceneCaptureEventDelegate.Remove(HandleOfStartSpatialSceneCapture);
}

FPICOAnchorManager* FPICOAnchorManager::GetInstance()
{
	static FPICOAnchorManager Instance;
	return &Instance;
}

void FPICOAnchorManager::Initialize(FPICOXRHMD* InPICOXRHMD)
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager::Initialize");
	PICOXRHMD = InPICOXRHMD;
	if (PICOXRHMD)
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::Initialize Bind PollEvent");
		HandleOfPollEvent = PICOXRHMD->OnPollEventDelegate().AddRaw(FPICOAnchorManager::GetInstance(), &FPICOAnchorManager::PollEvent);
	}
}

void FPICOAnchorManager::Shutdown()
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager::Shutdown");
	if (HandleOfPollEvent.IsValid() && PICOXRHMD)
	{
		PICOXRHMD->OnPollEventDelegate().Remove(HandleOfPollEvent);
	}
}

void FPICOAnchorManager::PollEvent(PxrEventDataBuffer* EventData)
{
	PxrStructureType EventType = EventData->type;
	switch (EventType)
	{
		case PXR_TYPE_EVENT_DATA_SPATIAL_TRACKING_STATE_UPDATE:			// SpatialTrackingStateUpdate		Event
		{
			PXR_LOGI(PxrMR, "FPICOAnchorManager::PollEvent PXR_TYPE_EVENT_DATA_SPATIAL_TRACKING_STATE_UPDATE");
			const PxrEventDataSpatialTrackingStateUpdate* TrackingState = reinterpret_cast<const PxrEventDataSpatialTrackingStateUpdate*>(EventData);
			EPICOSpatialTrackingState State = (EPICOSpatialTrackingState)TrackingState->stateInfo.state;
			EPICOSpatialTrackingStateMessage Message = (EPICOSpatialTrackingStateMessage)TrackingState->stateInfo.message;
			SpatialTrackingStateUpdateDelegate.Broadcast(State, Message);
			break;
		}

		case PXR_TYPE_EVENT_DATA_ANCHOR_ENTITY_CREATED:					// CreateAnchorEntity				Event
		{
			PXR_LOGI(PxrMR, "FPICOAnchorManager::PollEvent PXR_TYPE_EVENT_DATA_ANCHOR_ENTITY_CREATED");
			const PxrEventDataAnchorEntityCreated* CreatedInfo = reinterpret_cast<const PxrEventDataAnchorEntityCreated*>(EventData);
			EPICOResult Result = CastToPICOResult(CreatedInfo->result);
			FPICOAnchor AnchorHandle = CreatedInfo->anchorHandle;
			FPICOAnchorUUID AnchorUUID = CreatedInfo->uuid.value;
			CreateAnchorEntityEventDelegate.Broadcast(CreatedInfo->taskId, Result, AnchorHandle, AnchorUUID);
			break;
		}
		
		case PXR_TYPE_EVENT_DATA_ANCHOR_ENTITY_PERSISTED:				// PersistAnchorEntity				Event
		{
			PXR_LOGI(PxrMR, "FPICOAnchorManager::PollEvent PXR_TYPE_EVENT_DATA_ANCHOR_ENTITY_PERSISTED");
			const PxrEventDataAnchorEntityPersisted* PersistedInfo = reinterpret_cast<const PxrEventDataAnchorEntityPersisted*>(EventData);
			EPICOPersistLocation PersistLocation = (EPICOPersistLocation)PersistedInfo->location;
			EPICOResult Result = CastToPICOResult(PersistedInfo->result);
			PersistAnchorEntityEventDelegate.Broadcast(PersistedInfo->taskId, Result, PersistLocation);
			break;
		}
		case PXR_TYPE_EVENT_DATA_ANCHOR_ENTITY_UNPERSISTED:				// UnpersistAnchorEntity			Event
		{
			PXR_LOGI(PxrMR, "FPICOAnchorManager::PollEvent PXR_TYPE_EVENT_DATA_ANCHOR_ENTITY_UNPERSISTED");
			const PxrEventDataAnchorEntityUnpersisted* UnpersistedInfo = reinterpret_cast<const PxrEventDataAnchorEntityUnpersisted*>(EventData);
			EPICOPersistLocation PersistLocation = (EPICOPersistLocation)UnpersistedInfo->location;
			EPICOResult Result = CastToPICOResult(UnpersistedInfo->result);
			UnpersistAnchorEntityEventDelegate.Broadcast(UnpersistedInfo->taskId, Result, PersistLocation);
			break;
		}
		case PXR_TYPE_EVENT_DATA_ANCHOR_ENTITY_CLEARED:					// ClearAnchorEntity				Event
		{
			PXR_LOGI(PxrMR, "FPICOAnchorManager::PollEvent PXR_TYPE_EVENT_DATA_ANCHOR_ENTITY_CLEARED");
			const PxrEventDataAnchorEntityCleared* ClearInfo = reinterpret_cast<const PxrEventDataAnchorEntityCleared*>(EventData);
			EPICOPersistLocation PersistLocation = (EPICOPersistLocation)ClearInfo->location;
			EPICOResult Result = CastToPICOResult(ClearInfo->result);
			ClearAnchorEntityEventDelegate.Broadcast(ClearInfo->taskId, Result, PersistLocation);
			break;
		}
		case PXR_TYPE_EVENT_DATA_ANCHOR_ENTITY_LOADED:					// LoadAnchorEntity					Event
		{
			PXR_LOGI(PxrMR, "FPICOAnchorManager::PollEvent PXR_TYPE_EVENT_DATA_ANCHOR_ENTITY_LOADED");
			const PxrEventDataAnchorEntityLoaded* LoadedInfo = reinterpret_cast<const PxrEventDataAnchorEntityLoaded*>(EventData);
			EPICOPersistLocation PersistLocation = (EPICOPersistLocation)LoadedInfo->location;
			EPICOResult Result = CastToPICOResult(LoadedInfo->result);
			LoadAnchorEntityEventDelegate.Broadcast(LoadedInfo->taskId, Result, LoadedInfo->count, PersistLocation);
			break;
		}
		case PXR_TYPE_EVENT_DATA_SPATIAL_SCENE_CAPTURED:				// SpatialSceneCaptured				Event
		{
			PXR_LOGI(PxrMR, "FPICOAnchorManager::PollEvent PXR_TYPE_EVENT_DATA_SPATIAL_SCENE_CAPTURED");
			const PxrEventDataSpatialSceneCaptured* CapturedInfo = reinterpret_cast<const PxrEventDataSpatialSceneCaptured*>(EventData);
			EPICOResult Result = CastToPICOResult(CapturedInfo->result);
			EPICOSpatialSceneCaptureStatus Status = (EPICOSpatialSceneCaptureStatus)CapturedInfo->status;
			StartSpatialSceneCaptureEventDelegate.Broadcast(CapturedInfo->taskId, Result, Status);
			break;
		}
		default:
		{
			PXR_LOGV(PxrMR, "FPICOAnchorManager::PollEvent default");
			break;
		}
	}
}

bool FPICOAnchorManager::CreateAnchorEntity(AActor* BindingActor, const FTransform& AnchorEntityTransform, float Timeout, const FPICOCreateAnchorEntityDelegate& Delegate, EPICOResult& OutResult)
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager::CreateAnchorEntity");
	if (!IsValid(BindingActor) || !IsValid(BindingActor->GetWorld()))
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::CreateAnchorEntity Actor is Invalid[%s]", IsValid(BindingActor) ? *BindingActor->GetName() : TEXT("nullptr"));
		OutResult = EPICOResult::PXR_Error_ValidationFailure;
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = Cast<UPICOAnchorComponent>(BindingActor->GetComponentByClass(UPICOAnchorComponent::StaticClass()));
	if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::CreateAnchorEntity AnchorComponent and AnchorHandle is Already Valid[%llu]", (uint64)AnchorComponent->GetAnchorHandle().GetValue());
		OutResult = EPICOResult::PXR_Error_ValidationFailure;
		return false;
	}
	if (!AnchorComponent)
	{
		AnchorComponent = Cast<UPICOAnchorComponent>(BindingActor->AddComponentByClass(UPICOAnchorComponent::StaticClass(), false, FTransform::Identity, false));
	}

	float WorldToMetersScale = BindingActor->GetWorld()->GetWorldSettings()->WorldToMeters;
	FTransform TrackingToWorld = PICOXRHMD->GetTrackingToWorldTransform();
	const FQuat TrackingOrientation = TrackingToWorld.Inverse().TransformRotation(AnchorEntityTransform.Rotator().Quaternion());
	const FVector TrackingPosition = TrackingToWorld.Inverse().TransformPosition(AnchorEntityTransform.GetLocation());
	FPose UnrealPose = FPose(TrackingOrientation, TrackingPosition);

	PxrPosef AnchorPose;
	ConvertPose_Private(UnrealPose, AnchorPose, PICOXRHMD->GetBaseOrientation(), PICOXRHMD->GetBaseOffsetInMeters(), WorldToMetersScale);

	PxrAnchorEntityCreateInfo EntityInfo;
	FPICOXRHMDModule::GetPluginWrapper().GetTrackingOrigin(&EntityInfo.pxrTrackingOrigin);
	EntityInfo.timeMs = static_cast<double>(Timeout);
	EntityInfo.pose = AnchorPose;

	PXR_LOGI(PxrMR, "FPICOAnchorManager::CreateAnchorEntity Params: TrackingOrigin[%d], Timeout[%f], Pose[%s]", (int32)EntityInfo.pxrTrackingOrigin, Timeout, *AnchorEntityTransform.ToString());

	uint64_t AsyncTaskId = 0;
	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().CreateAnchorEntity(&EntityInfo, &AsyncTaskId));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::CreateAnchorEntity Call PxrAPI Result[%d], TaskID[%llu]", (int32)Result, (uint64)AsyncTaskId);
	if (PXR_FAILURE(Result))
	{
		OutResult = Result;
		return false;
	}

	FAnchorCreateInfo TaskInfo;
	TaskInfo.AsyncTaskId = AsyncTaskId;
	TaskInfo.Delegate = Delegate;
	TaskInfo.AnchorComponent = AnchorComponent;
	CreateAnchorBindings.Add(AsyncTaskId, TaskInfo);
	return true;
}

bool FPICOAnchorManager::DestroyAnchorEntity(AActor* BoundActor, const FPICODestroyAnchorEntityDelegate& Delegate, EPICOResult& OutResult)
{
	if (!IsAnchorValid(BoundActor))
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::DestroyAnchorEntity Actor is Invalid");
		OutResult = EPICOResult::PXR_Error_ValidationFailure;
		return false;
	}
	UPICOAnchorComponent* AnchorComponent = GetAnchorComponent(BoundActor);

	PxrAnchorEntityDestroyInfo EntityInfo;
	EntityInfo.anchor = AnchorComponent->GetAnchorHandle().GetValue();

	PXR_LOGI(PxrMR, "FPICOAnchorManager::DestroyAnchorEntity Params: Handle[%llu]", (uint64)AnchorComponent->GetAnchorHandle().GetValue());

	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().DestroyAnchorEntity(&EntityInfo));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::DestroyAnchorEntity Call Complete: Result[%d]", (int32)Result);

	if (PXR_SUCCESS(Result))
	{
		AnchorComponent->SetAnchorHandle(0);
		AnchorComponent->SetAnchorUUID(FPICOAnchorUUID());
	}
	else
	{
		OutResult = Result;
	}

	Delegate.ExecuteIfBound(Result);
	return PXR_SUCCESS(Result);
}

bool FPICOAnchorManager::PersistAnchorEntity(const TArray<AActor*>& BoundActors, EPICOPersistLocation PersistLocation, const FPICOPersistAnchorEntityDelegate& Delegate, EPICOResult& OutResult)
{
	TArray<uint64_t> AnchorHandles;
	TArray<UPICOAnchorComponent*> AnchorComponents;
	for (AActor* BoundActor : BoundActors)
	{
		if (!IsAnchorValid(BoundActor))
		{
			PXR_LOGI(PxrMR, "FPICOAnchorManager::PersistAnchorEntity Actor is Invalid: Actor[%s]", IsValid(BoundActor) ? *BoundActor->GetName() : TEXT("nullptr"));
			OutResult = EPICOResult::PXR_Error_ValidationFailure;
			return false;
		}

		UPICOAnchorComponent* AnchorComponent = GetAnchorComponent(BoundActor);
		AnchorHandles.Add(AnchorComponent->GetAnchorHandle().GetValue());
		AnchorComponents.Add(AnchorComponent);
	}

	uint64_t AsyncTaskId = 0;
	PxrAnchorEntityPersistInfo PersistInfo;
	PersistInfo.anchorList.anchors = AnchorHandles.GetData();
	PersistInfo.anchorList.count = AnchorHandles.Num();
	PersistInfo.location = (PxrPersistLocation)PersistLocation;

	PXR_LOGI(PxrMR, "FPICOAnchorManager::PersistAnchorEntity Params: ActorNum[%d], HandleNum[%d], Location[%d]", BoundActors.Num(), AnchorHandles.Num(), (int32)PersistLocation);

	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().PersistAnchorEntity(&PersistInfo, &AsyncTaskId));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::PersistAnchorEntity Call PxrAPI Result[%d]: TaskID[%llu]", (int32)Result, (uint64)AsyncTaskId);
	if (PXR_FAILURE(Result))
	{
		OutResult = Result;
		return false;
	}

	FAnchorPersistInfo TaskInfo;
	TaskInfo.AsyncTaskId = AsyncTaskId;
	TaskInfo.Delegate = Delegate;
	TaskInfo.AnchorComponents = AnchorComponents;
	PersistAnchorsBindings.Add(AsyncTaskId, TaskInfo);
	return true;
}

bool FPICOAnchorManager::UnpersistAnchorEntity(const TArray<AActor*>& BoundActors, EPICOPersistLocation PersistLocation, const FPICOUnpersistAnchorEntityDelegate& Delegate, EPICOResult& OutResult)
{
	TArray<uint64_t> AnchorHandles;
	TArray<UPICOAnchorComponent*> AnchorComponents;
	for (AActor* BoundActor : BoundActors)
	{
		if (!IsAnchorValid(BoundActor))
		{
			PXR_LOGI(PxrMR, "FPICOAnchorManager::UnpersistAnchorEntity Actor is Invalid: Actor[%s]", IsValid(BoundActor) ? *BoundActor->GetName() : TEXT("nullptr"));
			OutResult = EPICOResult::PXR_Error_ValidationFailure;
			return false;
		}

		UPICOAnchorComponent* AnchorComponent = GetAnchorComponent(BoundActor);
		AnchorHandles.Add(AnchorComponent->GetAnchorHandle().GetValue());
		AnchorComponents.Add(AnchorComponent);
	}

	uint64_t AsyncTaskId = 0;
	PxrAnchorEntityUnpersistInfo UnpersistInfo;
	UnpersistInfo.anchorList.anchors = AnchorHandles.GetData();
	UnpersistInfo.anchorList.count = AnchorHandles.Num();
	UnpersistInfo.location = (PxrPersistLocation)PersistLocation;

	PXR_LOGI(PxrMR, "FPICOAnchorManager::UnpersistAnchorEntity Params: ActorNum[%d], HandleNum[%d], Location[%d]", BoundActors.Num(), AnchorHandles.Num(), (int32)PersistLocation);

	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().UnpersistAnchorEntity(&UnpersistInfo, &AsyncTaskId));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::UnpersistAnchorEntity Call PxrAPI Result[%d]: TaskID[%llu]", (int32)Result, (uint64)AsyncTaskId);
	if (PXR_FAILURE(Result))
	{
		OutResult = Result;
		return false;
	}

	FAnchorUnpersistInfo TaskInfo;
	TaskInfo.AsyncTaskId = AsyncTaskId;
	TaskInfo.Delegate = Delegate;
	TaskInfo.AnchorComponents = AnchorComponents;
	UnpersistAnchorsBindings.Add(AsyncTaskId, TaskInfo);
	return true;
}

bool FPICOAnchorManager::ClearAnchorEntity(EPICOPersistLocation PersistLocation, const FPICOClearAnchorEntityDelegate& Delegate, EPICOResult& OutResult)
{
	uint64_t AsyncTaskId = 0;
	PxrAnchorEntityClearInfo ClearInfo;
	ClearInfo.location = (PxrPersistLocation)PersistLocation;

	PXR_LOGI(PxrMR, "FPICOAnchorManager::ClearAnchorEntity Params: Location[%d]", (int32)PersistLocation);

	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().ClearPersistedAnchorEntity(&ClearInfo, &AsyncTaskId));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::ClearAnchorEntity Call PxrAPI Result[%d]: TaskID[%llu]", (int32)Result, (uint64)AsyncTaskId);
	if (PXR_FAILURE(Result))
	{
		OutResult = Result;
		return false;
	}

	FAnchorClearInfo TaskInfo;
	TaskInfo.AsyncTaskId = AsyncTaskId;
	TaskInfo.Delegate = Delegate;
	ClearAnchorsBindings.Add(AsyncTaskId, TaskInfo);
	return true;
}

bool FPICOAnchorManager::LoadAnchorEntity(const FPICOAnchorLoadInfo& LoadInfo, const FPICOLoadAnchorEntityDelegate& Delegate, EPICOResult& OutResult)
{
	uint64_t AsyncTaskId = 0;
	PxrAnchorEntityLoadInfo PxrLoadInfo;
	PxrLoadInfo.maxResult = LoadInfo.LoadMaxCount;
	PxrLoadInfo.timeout = static_cast<double>(LoadInfo.Timeout);
	PxrLoadInfo.location = (PxrPersistLocation)LoadInfo.PersistLocation;
	
	PxrAnchorEntityLoadUuidFilter IncludeUUIDFilter, ExcludeUUIDFilter;
	IncludeUUIDFilter.type = ExcludeUUIDFilter.type = PXR_TYPE_ANCHOR_ENTITY_LOAD_UUID_FILTER;
	IncludeUUIDFilter.uuidList = ExcludeUUIDFilter.uuidList = nullptr;

	PxrAnchorEntityLoadSpatialSceneFilter IncludeSpatialSceneFilter, ExcludeSpatialSceneFilter;
	IncludeSpatialSceneFilter.type = ExcludeSpatialSceneFilter.type = PXR_TYPE_ANCHOR_ENTITY_LOAD_SPATIAL_SCENE_FILTER;

	if (LoadInfo.IncludeFilterType == EPICOLoadFilterType::LoadFilterType_UUID)
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::LoadAnchorEntity UUID Include Num[%d]", LoadInfo.IncludeUUIDFilter.Num());

		IncludeUUIDFilter.uuidCount = LoadInfo.IncludeUUIDFilter.Num();
		if (IncludeUUIDFilter.uuidCount > 0)
		{
			IncludeUUIDFilter.uuidList = new PxrUUid[IncludeUUIDFilter.uuidCount];
			for (uint32_t Index = 0; Index < IncludeUUIDFilter.uuidCount; ++Index)
			{
				PXR_LOGI(PxrMR, "FPICOAnchorManager::LoadAnchorEntity UUID Include[%d/%d][%s]", Index, LoadInfo.IncludeUUIDFilter.Num(), *LoadInfo.IncludeUUIDFilter[Index].ToString());
				
				FMemory::Memcpy(IncludeUUIDFilter.uuidList[Index].value, LoadInfo.IncludeUUIDFilter[Index].UUIDArray);
			}
		}
		PxrLoadInfo.include = (const PxrAnchorEntityLoadFilterBaseHeader*)(&IncludeUUIDFilter);
	}
	else if (LoadInfo.IncludeFilterType == EPICOLoadFilterType::LoadFilterType_SpatialSceneData)
	{
		uint64_t IncludeSpatialSceneDataTypeFilter = 0;
		for (int32 Index = 0; Index < LoadInfo.IncludeSpatialSceneDataTypeFilter.Num(); ++Index)
		{
			uint64_t PxrValue = 0;
			uint8 BitValue = (uint8)LoadInfo.IncludeSpatialSceneDataTypeFilter[Index];
			if (BitValue != 0)
			{
				PxrValue = 1ULL << (BitValue - 1);
			}
			PXR_LOGI(PxrMR, "FPICOAnchorManager::LoadAnchorEntity SceneLabel Include [%d/%d] Value[%llu][%llu][%llu]", Index, LoadInfo.IncludeSpatialSceneDataTypeFilter.Num(), (uint64)LoadInfo.IncludeSpatialSceneDataTypeFilter[Index], (uint64)PxrValue, (uint64)IncludeSpatialSceneDataTypeFilter);
			
			IncludeSpatialSceneDataTypeFilter |= PxrValue;
		}
		IncludeSpatialSceneFilter.typeFlags = (PxrSpatialSceneDataTypeFlags)IncludeSpatialSceneDataTypeFilter;
		PxrLoadInfo.include = (const PxrAnchorEntityLoadFilterBaseHeader*)(&IncludeSpatialSceneFilter);
		
		PXR_LOGI(PxrMR, "FPICOAnchorManager::LoadAnchorEntity SceneLabel Include Num[%d] Value[%llu]", LoadInfo.IncludeSpatialSceneDataTypeFilter.Num(), (uint64)IncludeSpatialSceneDataTypeFilter);
	}
	else
	{
		PxrLoadInfo.include = nullptr;
	}

	if (LoadInfo.ExcludeFilterType == EPICOLoadFilterType::LoadFilterType_UUID)
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::LoadAnchorEntity UUID Exclude Num[%d]", LoadInfo.ExcludeUUIDFilter.Num());

		ExcludeUUIDFilter.uuidCount = LoadInfo.ExcludeUUIDFilter.Num();
		if (ExcludeUUIDFilter.uuidCount > 0)
		{
			ExcludeUUIDFilter.uuidList = new PxrUUid[ExcludeUUIDFilter.uuidCount];
			for (uint32_t Index = 0; Index < ExcludeUUIDFilter.uuidCount; ++Index)
			{
				PXR_LOGI(PxrMR, "FPICOAnchorManager::LoadAnchorEntity UUID Exclude[%d/%d][%s]", Index, LoadInfo.ExcludeUUIDFilter.Num(), *LoadInfo.ExcludeUUIDFilter[Index].ToString());

				FMemory::Memcpy(ExcludeUUIDFilter.uuidList[Index].value, LoadInfo.ExcludeUUIDFilter[Index].UUIDArray);
			}
		}
		PxrLoadInfo.exclude = (const PxrAnchorEntityLoadFilterBaseHeader*)(&ExcludeUUIDFilter);
	}
	else if (LoadInfo.ExcludeFilterType == EPICOLoadFilterType::LoadFilterType_SpatialSceneData)
	{
		uint64_t ExcludeSpatialSceneDataTypeFilter = 0;
		for (int32 Index = 0; Index < LoadInfo.ExcludeSpatialSceneDataTypeFilter.Num(); ++Index)
		{
			uint64_t PxrValue = 0;
			uint8 BitValue = (uint8)LoadInfo.ExcludeSpatialSceneDataTypeFilter[Index];
			if (BitValue != 0)
			{
				PxrValue = 1ULL << (BitValue - 1);
			}
			PXR_LOGI(PxrMR, "FPICOAnchorManager::LoadAnchorEntity SceneLabel Exclude[%d/%d] Value[%llu][%llu][%llu]", Index, LoadInfo.ExcludeSpatialSceneDataTypeFilter.Num(), (uint64)LoadInfo.ExcludeSpatialSceneDataTypeFilter[Index], (uint64)PxrValue, (uint64)ExcludeSpatialSceneDataTypeFilter);

			ExcludeSpatialSceneDataTypeFilter |= PxrValue;
		}
		ExcludeSpatialSceneFilter.typeFlags = (PxrSpatialSceneDataTypeFlags)ExcludeSpatialSceneDataTypeFilter;
		PxrLoadInfo.exclude = (const PxrAnchorEntityLoadFilterBaseHeader*)(&ExcludeSpatialSceneFilter);

		PXR_LOGI(PxrMR, "FPICOAnchorManager::LoadAnchorEntity SceneLabel Exclude Num[%d] Value[%llu]", LoadInfo.ExcludeSpatialSceneDataTypeFilter.Num(), (uint64)ExcludeSpatialSceneDataTypeFilter);
	}
	else
	{
		PxrLoadInfo.exclude = nullptr;
	}

	PXR_LOGI(PxrMR, "FPICOAnchorManager::LoadAnchorEntity Params: %s", *LoadInfo.ToString());

	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().LoadAnchorEntity(&PxrLoadInfo, &AsyncTaskId));

	if (IncludeUUIDFilter.uuidList)
	{
		delete[] IncludeUUIDFilter.uuidList;
	}
	if (ExcludeUUIDFilter.uuidList)
	{
		delete[] ExcludeUUIDFilter.uuidList;
	}

	PXR_LOGI(PxrMR, "FPICOAnchorManager::LoadAnchorEntity Call PxrAPI Result[%d]: TaskID[%llu]", (int32)Result, (uint64)AsyncTaskId);
	if (PXR_FAILURE(Result))
	{
		OutResult = Result;
		return false;
	}

	FAnchorLoadInfo TaskInfo;
	TaskInfo.AsyncTaskId = AsyncTaskId;
	TaskInfo.Delegate = Delegate;
	LoadAnchorsBindings.Add(AsyncTaskId, TaskInfo);
	return true;
}

bool FPICOAnchorManager::StartSpatialSceneCapture(const FPICOStartSpatialSceneCaptureDelegate& Delegate, EPICOResult& OutResult)
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager::StartSpatialSceneCapture");

	uint64_t AsyncTaskId = 0;
	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().StartSpatialSceneCapture(&AsyncTaskId));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::StartSpatialSceneCapture Call PxrAPI Result[%d]: TaskID[%llu]", (int32)Result, (uint64)AsyncTaskId);
	if (PXR_FAILURE(Result))
	{
		OutResult = Result;
		return false;
	}

	FStartSpatialSceneCaptureInfo TaskInfo;
	TaskInfo.AsyncTaskId = AsyncTaskId;
	TaskInfo.Delegate = Delegate;
	StartSpatialSceneCaptureBindings.Add(AsyncTaskId, TaskInfo);
	return true;
}

bool FPICOAnchorManager::GetAnchorEntityUUID(AActor* BoundActor, FPICOAnchorUUID& OutAnchorUUID)
{
	if (!IsAnchorValid(BoundActor))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = GetAnchorComponent(BoundActor);
	FPICOAnchor AnchorHandle = AnchorComponent->GetAnchorHandle();
	PxrUUid PxrAnchorUUID;
	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().GetAnchorEntityUuid(AnchorHandle.GetValue(), &PxrAnchorUUID));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorEntityUUID Call PxrAPI Result[%d], Handle[%llu]", (int32)Result, (uint64)AnchorHandle.GetValue());
	if (PXR_FAILURE(Result))
	{
		return false;
	}

	OutAnchorUUID = PxrAnchorUUID.value;
	AnchorComponent->SetAnchorUUID(OutAnchorUUID);

	PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorEntityUUID AnchorUUID[%s]", *OutAnchorUUID.ToString());
	return true;
}

bool FPICOAnchorManager::GetAnchorComponentFlags(AActor* BoundActor, TArray<EPICOAnchorComponentTypeFlag>& OutAnchorComponentFlags)
{
	OutAnchorComponentFlags.Empty();

	if (!IsAnchorValid(BoundActor))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = GetAnchorComponent(BoundActor);
	FPICOAnchor AnchorHandle = AnchorComponent->GetAnchorHandle();
	PxrAnchorComponentTypeFlags PxrComponentFlags;
	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().GetAnchorComponentFlags(AnchorHandle.GetValue(), &PxrComponentFlags));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorComponentFlags Call PxrAPI Result[%d], Handle[%llu], FlagsValue[%llu]", (int32)Result, (uint64)AnchorHandle.GetValue(), (uint64)PxrComponentFlags);
	if (PXR_FAILURE(Result))
	{
		return false;
	}

	UEnum* ComponentFlagsEnum = StaticEnum<EPICOAnchorComponentTypeFlag>();
	int32 EnumNum = ComponentFlagsEnum->NumEnums() - 1;
	for (int32 Index = 0; Index < EnumNum; ++Index)
	{
		int64 Value = ComponentFlagsEnum->GetValueByIndex(Index);
		if (Value != 0)
		{
			int64 PxrValue = 1ULL << (Value - 1);
			if (PxrValue & PxrComponentFlags)
			{
				PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorComponentFlags FlagName[%s]", *ComponentFlagsEnum->GetNameStringByIndex(Index));
				OutAnchorComponentFlags.Add((EPICOAnchorComponentTypeFlag)Value);
			}
		}
	}
	return true;
}

bool FPICOAnchorManager::GetAnchorSceneLabel(AActor* BoundActor, EPICOAnchorSceneLabel& OutAnchorSceneLabel)
{
	if (!IsAnchorValid(BoundActor))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = GetAnchorComponent(BoundActor);
	FPICOAnchor AnchorHandle = AnchorComponent->GetAnchorHandle();
	PxrSceneLabel SceneLabel;
	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().GetAnchorSceneLabel(AnchorHandle.GetValue(), &SceneLabel));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorSceneLabel Call PxrAPI Result[%d], Handle[%llu], SceneLabel[%d]", (int32)Result, (uint64)AnchorHandle.GetValue(), (int32)SceneLabel);
	if (PXR_FAILURE(Result))
	{
		return false;
	}

	OutAnchorSceneLabel = (EPICOAnchorSceneLabel)SceneLabel;
	return true;
}

bool FPICOAnchorManager::GetAnchorPlaneBoundaryInfo(AActor* BoundActor, FPICOAnchorPlaneBoundaryInfo& OutAnchorPlaneBoundaryInfo)
{
	if (!IsAnchorValid(BoundActor))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = GetAnchorComponent(BoundActor);
	FPICOAnchor AnchorHandle = AnchorComponent->GetAnchorHandle();
	PxrAnchorPlaneBoundaryInfo BoundaryInfo;
	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().GetAnchorPlaneBoundaryInfo(AnchorHandle.GetValue(), &BoundaryInfo));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorPlaneBoundaryInfo Call PxrAPI Result[%d], Handle[%llu]", (int32)Result, (uint64)AnchorHandle.GetValue());
	if (PXR_FAILURE(Result))
	{
		return false;
	}

	float WorldToMetersScale = AnchorComponent->GetWorld()->GetWorldSettings()->WorldToMeters;
	OutAnchorPlaneBoundaryInfo.Center = ToFVector(BoundaryInfo.center) * WorldToMetersScale;
	OutAnchorPlaneBoundaryInfo.Extent.X = BoundaryInfo.extent.height * WorldToMetersScale;
	OutAnchorPlaneBoundaryInfo.Extent.Y = BoundaryInfo.extent.width * WorldToMetersScale;

	PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorPlaneBoundaryInfo Center[%s], Extent[%s]", *OutAnchorPlaneBoundaryInfo.Center.ToString(), *OutAnchorPlaneBoundaryInfo.Extent.ToString());
	return true;
}

bool FPICOAnchorManager::GetAnchorPlanePolygonInfo(AActor* BoundActor, FPICOAnchorPlanePolygonInfo& OutAnchorPlanePolygonInfo)
{
	if (!IsAnchorValid(BoundActor))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = GetAnchorComponent(BoundActor);
	FPICOAnchor AnchorHandle = AnchorComponent->GetAnchorHandle();

	PxrAnchorPlanePolygonInfo PolygonInfo;
	PolygonInfo.polygonSizeCapacityInput = 0;
	PolygonInfo.polygonSizeCountOutput = 0;
	PolygonInfo.polygonVertices = nullptr;
	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().GetAnchorPlanePolygonInfo(AnchorHandle.GetValue(), &PolygonInfo));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorPlanePolygonInfo Call PxrAPI Result[%d], Handle[%llu], VerticesNum[%u]", (int32)Result, (uint64)AnchorHandle.GetValue(), (uint32)PolygonInfo.polygonSizeCountOutput);
	if (PXR_SUCCESS(Result) && PolygonInfo.polygonSizeCountOutput > 0)
	{
		PolygonInfo.polygonSizeCapacityInput = PolygonInfo.polygonSizeCountOutput;
		PxrVector3f* Data = new PxrVector3f[PolygonInfo.polygonSizeCountOutput];
		PolygonInfo.polygonVertices = Data;
		EPICOResult DoubleResult = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().GetAnchorPlanePolygonInfo(AnchorHandle.GetValue(), &PolygonInfo));

		PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorPlanePolygonInfo Double Call PxrAPI Result[%d]", (int32)Result);
		if (PXR_SUCCESS(DoubleResult))
		{
			float WorldToMetersScale = AnchorComponent->GetWorld()->GetWorldSettings()->WorldToMeters;

			OutAnchorPlanePolygonInfo.VerticesNum = PolygonInfo.polygonSizeCountOutput;
			OutAnchorPlanePolygonInfo.Vertices.SetNum(OutAnchorPlanePolygonInfo.VerticesNum);
			for (int32 Index = 0; Index < OutAnchorPlanePolygonInfo.VerticesNum; ++Index)
			{
				OutAnchorPlanePolygonInfo.Vertices[Index] = ToFVector(Data[Index]) * WorldToMetersScale;
				PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorPlanePolygonInfo Vertex[%s] for [%d/%d]", *OutAnchorPlanePolygonInfo.Vertices[Index].ToString(), Index, OutAnchorPlanePolygonInfo.VerticesNum);
			}
			delete[] Data;
			return true;
		}
		delete[] Data;
	}

	return false;
}

bool FPICOAnchorManager::GetAnchorVolumeInfo(AActor* BoundActor, FPICOAnchorVolumeInfo& OutAnchorVolumeInfo)
{
	if (!IsAnchorValid(BoundActor))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = GetAnchorComponent(BoundActor);
	FPICOAnchor AnchorHandle = AnchorComponent->GetAnchorHandle();
	PxrAnchorBoxInfo BoxInfo;
	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().GetAnchorBoxInfo(AnchorHandle.GetValue(), &BoxInfo));

	PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorVolumeInfo Call PxrAPI Result[%d], Handle[%llu]", (int32)Result, (uint64)AnchorHandle.GetValue());
	if (PXR_FAILURE(Result))
	{
		return false;
	}

	float WorldToMetersScale = AnchorComponent->GetWorld()->GetWorldSettings()->WorldToMeters;
	OutAnchorVolumeInfo.Center = ToFVector(BoxInfo.center) * WorldToMetersScale;
	OutAnchorVolumeInfo.Extent.X = BoxInfo.extent.z * WorldToMetersScale;
	OutAnchorVolumeInfo.Extent.Y = BoxInfo.extent.x * WorldToMetersScale;
	OutAnchorVolumeInfo.Extent.Z = BoxInfo.extent.y * WorldToMetersScale;

	PXR_LOGI(PxrMR, "FPICOAnchorManager::GetAnchorVolumeInfo Center[%s], Extent[%s]", *OutAnchorVolumeInfo.Center.ToString(), *OutAnchorVolumeInfo.Extent.ToString());
	return true;
}

bool FPICOAnchorManager::GetAnchorPose(UPICOAnchorComponent* AnchorComponent, FTransform& OutAnchorPose)
{
	if (!IsAnchorValid(AnchorComponent))
	{
		return false;
	}

	FPICOAnchor AnchorHandle = AnchorComponent->GetAnchorHandle();

	PxrTrackingOrigin TrackingOrigin = PxrTrackingOrigin::PXR_EYE_LEVEL;
	FPICOXRHMDModule::GetPluginWrapper().GetTrackingOrigin(&TrackingOrigin);

	PxrPosef AnchorPose;
	EPICOResult Result = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().GetAnchorPose(AnchorHandle.GetValue(), TrackingOrigin, &AnchorPose));

	PXR_LOGV(PxrMR, "FPICOAnchorManager::UpdateAnchor Call PxrAPI Result[%d]", (int32)Result);
	if (PXR_FAILURE(Result))
	{
		return false;
	}

	float WorldToMetersScale = AnchorComponent->GetWorld()->GetWorldSettings()->WorldToMeters;

	FPose UnrealPose;
	ConvertPose_Private(AnchorPose, UnrealPose, PICOXRHMD->GetBaseOrientation(), PICOXRHMD->GetBaseOffsetInMeters(), WorldToMetersScale);

	FTransform TrackingToWorld = PICOXRHMD->GetTrackingToWorldTransform();

	OutAnchorPose.SetLocation(TrackingToWorld.TransformPosition(UnrealPose.Position));
	OutAnchorPose.SetRotation(TrackingToWorld.TransformRotation(UnrealPose.Orientation));
	return true;
}

bool FPICOAnchorManager::UpdateAnchor(UPICOAnchorComponent* AnchorComponent)
{
	FTransform AnchorTransform;
	if (!GetAnchorPose(AnchorComponent, AnchorTransform))
	{
		return false;
	}
	
	PXR_LOGV(PxrMR, "FPICOAnchorManager::UpdateAnchor Pose[%s]", *AnchorTransform.ToString());

	AActor* BoundActor = AnchorComponent->GetOwner();
	BoundActor->SetActorLocationAndRotation(AnchorTransform.GetLocation(), AnchorTransform.GetRotation());
	return true;
}

void FPICOAnchorManager::HandleCreateAnchorEntityEvent(uint64_t AsyncTaskId, EPICOResult Result, const FPICOAnchor& AnchorHandle, const FPICOAnchorUUID& AnchorUUID)
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleCreateAnchorEntityEvent Params: AsyncTaskId[%llu], Result[%d], AnchorHandle[%llu], AnchorUUID[%s]", (uint64)AsyncTaskId, (int32)Result, (uint64)AnchorHandle.GetValue(), *AnchorUUID.ToString());
	
	FAnchorCreateInfo* TaskInfo = CreateAnchorBindings.Find(AsyncTaskId);
	if (!TaskInfo)
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleCreateAnchorEntityEvent No Task");
		return;
	}

	if (PXR_FAILURE(Result) || !IsValid(TaskInfo->AnchorComponent))
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleCreateAnchorEntityEvent Failed, Maybe AnchorComponent is Invalid");
		TaskInfo->Delegate.ExecuteIfBound(Result, TaskInfo->AnchorComponent);
		PersistAnchorsBindings.Remove(AsyncTaskId);
		return;
	}

	TaskInfo->AnchorComponent->SetAnchorHandle(AnchorHandle);
	TaskInfo->AnchorComponent->SetAnchorUUID(AnchorUUID);
	TaskInfo->Delegate.ExecuteIfBound(Result, TaskInfo->AnchorComponent);
	CreateAnchorBindings.Remove(AsyncTaskId);
}

void FPICOAnchorManager::HandlePersistAnchorEntityEvent(uint64_t AsyncTaskId, EPICOResult Result, EPICOPersistLocation PersistLocation)
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager::HandlePersistAnchorEntityEvent Params: AsyncTaskId[%llu], Result[%d], Location[%d]", (uint64)AsyncTaskId, (int32)Result, (int32)PersistLocation);

	FAnchorPersistInfo* TaskInfo = PersistAnchorsBindings.Find(AsyncTaskId);
	if (!TaskInfo)
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::HandlePersistAnchorEntityEvent No Task");
		return;
	}

	if (PXR_FAILURE(Result))
	{
		TaskInfo->Delegate.ExecuteIfBound(Result, TArray<UPICOAnchorComponent*>());
		PersistAnchorsBindings.Remove(AsyncTaskId);
		return;
	}

	TaskInfo->Delegate.ExecuteIfBound(Result, TaskInfo->AnchorComponents);
	PersistAnchorsBindings.Remove(AsyncTaskId);
}

void FPICOAnchorManager::HandleUnpersistAnchorEntityEvent(uint64_t AsyncTaskId, EPICOResult Result, EPICOPersistLocation PersistLocation)
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleUnpersistAnchorEntityEvent Params: AsyncTaskId[%llu], Result[%d], Location[%d]", (uint64)AsyncTaskId, (int32)Result, (int32)PersistLocation);

	FAnchorUnpersistInfo* TaskInfo = UnpersistAnchorsBindings.Find(AsyncTaskId);
	if (!TaskInfo)
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleUnpersistAnchorEntityEvent No Task");
		return;
	}

	TaskInfo->Delegate.ExecuteIfBound(Result, TaskInfo->AnchorComponents);
	UnpersistAnchorsBindings.Remove(AsyncTaskId);
}

void FPICOAnchorManager::HandleClearAnchorEntityEvent(uint64_t AsyncTaskId, EPICOResult Result, EPICOPersistLocation PersistLocation)
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleClearAnchorEntityEvent Params: AsyncTaskId[%llu], Result[%d], Location[%d]", (uint64)AsyncTaskId, (int32)Result, (int32)PersistLocation);

	FAnchorClearInfo* TaskInfo = ClearAnchorsBindings.Find(AsyncTaskId);
	if (!TaskInfo)
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleClearAnchorEntityEvent No Task");
		return;
	}

	TaskInfo->Delegate.ExecuteIfBound(Result);
	ClearAnchorsBindings.Remove(AsyncTaskId);
}

void FPICOAnchorManager::HandleLoadAnchorEntityEvent(uint64_t AsyncTaskId, EPICOResult Result, uint32_t AnchorCount, EPICOPersistLocation PersistLocation)
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleLoadAnchorEntityEvent Params: AsyncTaskId[%llu], Result[%d], AnchorCount[%u], Location[%d]", (uint64)AsyncTaskId, (int32)Result, (uint32)AnchorCount, (int32)PersistLocation);

	FAnchorLoadInfo* TaskInfo = LoadAnchorsBindings.Find(AsyncTaskId);
	if (!TaskInfo)
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleLoadAnchorEntityEvent No Task");
		return;
	}

	TArray<FAnchorLoadResult> LoadedAnchors;

	if (PXR_FAILURE(Result))
	{
		TaskInfo->Delegate.ExecuteIfBound(Result, LoadedAnchors);
		LoadAnchorsBindings.Remove(AsyncTaskId);
		return;
	}

	if (AnchorCount == 0)
	{
		PXR_LOGW(PxrMR, "FPICOAnchorManager::HandleLoadAnchorEntityEvent AnchorCount == 0");
		TaskInfo->Delegate.ExecuteIfBound(Result, LoadedAnchors);
		LoadAnchorsBindings.Remove(AsyncTaskId);
		return;
	}

	TArray<PxrAnchorEntityLoadResult> PxrLoadedAnchors;
	PxrLoadedAnchors.SetNum(AnchorCount);

	PxrAnchorEntityLoadResults PxrLoadResults;
	PxrLoadResults.resultCapacityInput = AnchorCount;
	PxrLoadResults.resultCountOutput = AnchorCount;
	PxrLoadResults.results = PxrLoadedAnchors.GetData();

	EPICOResult LoadResult = CastToPICOResult(FPICOXRHMDModule::GetPluginWrapper().GetAnchorEntityLoadResults(AsyncTaskId, &PxrLoadResults));
	if (PXR_FAILURE(LoadResult))
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleLoadAnchorEntityEvent LoadResults Failed: LoadResult[%d]", (int32)LoadResult);
		TaskInfo->Delegate.ExecuteIfBound(LoadResult, LoadedAnchors);
		LoadAnchorsBindings.Remove(AsyncTaskId);
		return;
	}

	LoadedAnchors.SetNum(AnchorCount);
	for (uint32_t Index = 0; Index < AnchorCount; ++Index)
	{
		LoadedAnchors[Index].PersistLocation = PersistLocation;
		LoadedAnchors[Index].AnchorHandle = PxrLoadedAnchors[Index].anchor;
		LoadedAnchors[Index].AnchorUUID = PxrLoadedAnchors[Index].uuid.value;
	}
	TaskInfo->Delegate.ExecuteIfBound(LoadResult, LoadedAnchors);
	LoadAnchorsBindings.Remove(AsyncTaskId);
}

void FPICOAnchorManager::HandleStartSpatialSceneCaptureEvent(uint64_t AsyncTaskId, EPICOResult Result, EPICOSpatialSceneCaptureStatus SpatialSceneCaptureStatus)
{
	PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleStartSpatialSceneCaptureEvent Params: AsyncTaskId[%llu], Result[%d], bUpdated[%d]", (uint64)AsyncTaskId, (int32)Result, (int32)SpatialSceneCaptureStatus);

	FStartSpatialSceneCaptureInfo* TaskInfo = StartSpatialSceneCaptureBindings.Find(AsyncTaskId);
	if (!TaskInfo)
	{
		PXR_LOGI(PxrMR, "FPICOAnchorManager::HandleStartSpatialSceneCaptureEvent No Task");
		return;
	}

	TaskInfo->Delegate.ExecuteIfBound(Result, SpatialSceneCaptureStatus);
	StartSpatialSceneCaptureBindings.Remove(AsyncTaskId);
}

bool FPICOAnchorManager::IsAnchorValid(AActor* BoundActor)
{
	if (!IsValid(BoundActor) || !IsValid(BoundActor->GetWorld()))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = Cast<UPICOAnchorComponent>(BoundActor->GetComponentByClass(UPICOAnchorComponent::StaticClass()));
	if (!IsValid(AnchorComponent) || !AnchorComponent->IsAnchorValid())
	{
		return false;
	}
	return true;
}

bool FPICOAnchorManager::IsAnchorValid(UPICOAnchorComponent* AnchorComponent)
{
	if (!IsValid(AnchorComponent) || !AnchorComponent->IsAnchorValid())
	{
		return false;
	}
	return true;
}

UPICOAnchorComponent* FPICOAnchorManager::GetAnchorComponent(AActor* BoundActor)
{
	if (!IsValid(BoundActor) || !IsValid(BoundActor->GetWorld()))
	{
		return nullptr;
	}

	UPICOAnchorComponent* AnchorComponent = Cast<UPICOAnchorComponent>(BoundActor->GetComponentByClass(UPICOAnchorComponent::StaticClass()));
	return AnchorComponent;
}

EPICOResult FPICOAnchorManager::CastToPICOResult(PxrResult Result)
{
	EPICOResult PICOResult = EPICOResult::PXR_Error_Unknow;
	switch (Result)
	{
	case PxrResult::PXR_SUCCESS: PICOResult = EPICOResult::PXR_Success; break;
	case PxrResult::PXR_TIMEOUT_EXPIRED: PICOResult = EPICOResult::PXR_TimeoutExpired; break;
	case PxrResult::PXR_ERROR_VALIDATION_FAILURE: PICOResult = EPICOResult::PXR_Error_ValidationFailure; break;
	case PxrResult::PXR_ERROR_RUNTIME_FAILURE: PICOResult = EPICOResult::PXR_Error_RuntimeFailure; break;
	case PxrResult::PXR_ERROR_OUT_OF_MEMORY: PICOResult = EPICOResult::PXR_Error_OutOfMemory; break;
	case PxrResult::PXR_ERROR_API_VERSION_UNSUPPORTED: PICOResult = EPICOResult::PXR_Error_APIVersionUnsupported; break;
	case PxrResult::PXR_ERROR_INITIALIZATION_FAILED: PICOResult = EPICOResult::PXR_Error_InitializationFailed; break;
	case PxrResult::PXR_ERROR_FUNCTION_UNSUPPORTED: PICOResult = EPICOResult::PXR_Error_FunctionUnsupported; break;
	case PxrResult::PXR_ERROR_FEATURE_UNSUPPORTED: PICOResult = EPICOResult::PXR_Error_FeatureUnsupported; break;
	case PxrResult::PXR_ERROR_LIMIT_REACHED: PICOResult = EPICOResult::PXR_Error_LimitReached; break;
	case PxrResult::PXR_ERROR_SIZE_INSUFFICIENT: PICOResult = EPICOResult::PXR_Error_SizeInsufficient; break;
	case PxrResult::PXR_ERROR_HANDLE_INVALID: PICOResult = EPICOResult::PXR_Error_HandleInvalid; break;
	case PxrResult::PXR_ERROR_SPATIAL_LOCALIZATION_RUNNING_BD: PICOResult = EPICOResult::PXR_Error_SpatialLocalizationRunning; break;
	case PxrResult::PXR_ERROR_SPATIAL_LOCALIZATION_NOT_RUNNING_BD: PICOResult = EPICOResult::PXR_Error_SpatialLocalizationNotRunning; break;
	case PxrResult::PXR_ERROR_SPATIAL_MAP_CREATED_BD: PICOResult = EPICOResult::PXR_Error_SpatialMapCreated; break;
	case PxrResult::PXR_ERROR_SPATIAL_MAP_NOT_CREATED_BD: PICOResult = EPICOResult::PXR_Error_SpatialMapNotCreated; break;
	case PxrResult::PXR_ERROR_COMPONENT_NOT_SUPPORTED_BD: PICOResult = EPICOResult::PXR_Error_ComponentNotSupported; break;
	case PxrResult::PXR_ERROR_COMPONENT_CONFLICT: PICOResult = EPICOResult::PXR_Error_ComponentConflict; break;
	case PxrResult::PXR_ERROR_COMPONENT_NOT_ADDED_BD: PICOResult = EPICOResult::PXR_Error_ComponentNotAdded; break;
	case PxrResult::PXR_ERROR_COMPONENT_ADDED_BD: PICOResult = EPICOResult::PXR_Error_ComponentAdded; break;
	case PxrResult::PXR_ERROR_ANCHOR_ENTITY_NOT_FOUND_BD: PICOResult = EPICOResult::PXR_Error_AnchorEntityNotFound; break;
	default:break;
	}
	return PICOResult;
}