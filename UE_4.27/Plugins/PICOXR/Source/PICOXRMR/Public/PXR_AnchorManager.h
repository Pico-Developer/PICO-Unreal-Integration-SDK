// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "PXR_PluginWrapper.h"
#include "PXR_HMDTypes.h"
#include "PXR_HMD.h"
#include "PXR_MRTypes.h"
#include "PXR_AnchorComponent.h"

DECLARE_DELEGATE_TwoParams(FPICOCreateAnchorEntityDelegate, EPICOResult::Type, UPICOAnchorComponent*);
DECLARE_DELEGATE_OneParam(FPICODestroyAnchorEntityDelegate, EPICOResult::Type);
DECLARE_DELEGATE_TwoParams(FPICOPersistAnchorEntityDelegate, EPICOResult::Type, const TArray<UPICOAnchorComponent*>&);
DECLARE_DELEGATE_TwoParams(FPICOUnpersistAnchorEntityDelegate, EPICOResult::Type, const TArray<UPICOAnchorComponent*>&);
DECLARE_DELEGATE_OneParam(FPICOClearAnchorEntityDelegate, EPICOResult::Type);
DECLARE_DELEGATE_TwoParams(FPICOLoadAnchorEntityDelegate, EPICOResult::Type, const TArray<FAnchorLoadResult>&);
DECLARE_DELEGATE_TwoParams(FPICOStartSpatialSceneCaptureDelegate, EPICOResult::Type, EPICOSpatialSceneCaptureStatus);

DECLARE_MULTICAST_DELEGATE_TwoParams(FPICOSpatialTrackingStateUpdateDelegate, EPICOSpatialTrackingState, EPICOSpatialTrackingStateMessage);


class PICOXRMR_API FPICOAnchorManager
{
public:
	static FPICOAnchorManager* GetInstance();

	void Initialize(FPICOXRHMD* InPICOXRHMD);
	void Shutdown();

public:
	void PollEvent(PxrEventDataBuffer* EventData);

	FPICOSpatialTrackingStateUpdateDelegate& OnSpatialTrackingStateUpdate()
	{
		return SpatialTrackingStateUpdateDelegate;
	}

	bool CreateAnchorEntity(AActor* BindingActor, const FTransform& AnchorEntityTransfrom, float Timeout, const FPICOCreateAnchorEntityDelegate& Delegate);
	bool DestroyAnchorEntity(AActor* BoundActor, const FPICODestroyAnchorEntityDelegate& Delegate);
	bool PersistAnchorEntity(const TArray<AActor*>& BoundActors, EPICOPersistLocation PersistLocation, const FPICOPersistAnchorEntityDelegate& Delegate);
	bool UnpersistAnchorEntity(const TArray<AActor*>& BoundActors, EPICOPersistLocation PersistLocation, const FPICOUnpersistAnchorEntityDelegate& Delegate);
	bool ClearAnchorEntity(EPICOPersistLocation PersistLocation, const FPICOClearAnchorEntityDelegate& Delegate);
	bool LoadAnchorEntity(const FPICOAnchorLoadInfo& LoadInfo, const FPICOLoadAnchorEntityDelegate& Delegate);
	bool StartSpatialSceneCapture(const FPICOStartSpatialSceneCaptureDelegate& Delegate);

	bool GetAnchorEntityUUID(AActor* BoundActor, FPICOAnchorUUID& OutAnchorUUID);
	bool GetAnchorComponentFlags(AActor* BoundActor, TArray<TEnumAsByte<EPICOAnchorComponentTypeFlag::Type>>& OutAnchorComponentFlags);
	bool GetAnchorSceneLabel(AActor* BoundActor, EPICOAnchorSceneLabel& OutAnchorSceneLabel);
	bool GetAnchorPlaneBoundaryInfo(AActor* BoundActor, FPICOAnchorPlaneBoundaryInfo& OutAnchorPlaneBoundaryInfo);
	bool GetAnchorPlanePolygonInfo(AActor* BoundActor, FPICOAnchorPlanePolygonInfo& OutAnchorPlanePolygonInfo);
	bool GetAnchorVolumeInfo(AActor* BoundActor, FPICOAnchorVolumeInfo& OutAnchorVolumeInfo);

	bool GetAnchorPose(UPICOAnchorComponent* AnchorComponent, FTransform& OutAnchorPose);
	bool UpdateAnchor(UPICOAnchorComponent* AnchorComponent);

private:
	FPICOAnchorManager();
	~FPICOAnchorManager();

	FPICOSpatialTrackingStateUpdateDelegate SpatialTrackingStateUpdateDelegate;

	DECLARE_MULTICAST_DELEGATE_FourParams(FPICOCreateAnchorEntityEventDelegate, uint64_t, EPICOResult::Type, const FPICOAnchor&, const FPICOAnchorUUID&);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FPICOPersistAnchorEntityEventDelegate, uint64_t, EPICOResult::Type, EPICOPersistLocation);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FPICOUnpersistAnchorEntityEventDelegate, uint64_t, EPICOResult::Type, EPICOPersistLocation);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FPICOClearAnchorEntityEventDelegate, uint64_t, EPICOResult::Type, EPICOPersistLocation);
	DECLARE_MULTICAST_DELEGATE_FourParams(FPICOLoadAnchorEntityEventDelegate, uint64_t, EPICOResult::Type, uint32_t, EPICOPersistLocation);
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FPICOStartSpatialSceneCaptureEventDelegate, uint64_t, EPICOResult::Type, EPICOSpatialSceneCaptureStatus);

	FPICOCreateAnchorEntityEventDelegate CreateAnchorEntityEventDelegate;
	FPICOPersistAnchorEntityEventDelegate PersistAnchorEntityEventDelegate;
	FPICOUnpersistAnchorEntityEventDelegate UnpersistAnchorEntityEventDelegate;
	FPICOClearAnchorEntityEventDelegate ClearAnchorEntityEventDelegate;
	FPICOLoadAnchorEntityEventDelegate LoadAnchorEntityEventDelegate;
	FPICOStartSpatialSceneCaptureEventDelegate StartSpatialSceneCaptureEventDelegate;

	FDelegateHandle HandleOfCreateAnchorEntity;
	FDelegateHandle HandleOfPersistAnchorEntity;
	FDelegateHandle HandleOfUnpersistAnchorEntity;
	FDelegateHandle HandleOfClearAnchorEntity;
	FDelegateHandle HandleOfLoadAnchorEntity;
	FDelegateHandle HandleOfStartSpatialSceneCapture;

	void HandleCreateAnchorEntityEvent(uint64_t AsyncTaskId, EPICOResult::Type Result, const FPICOAnchor& AnchorHandle, const FPICOAnchorUUID& AnchorUUID);
	void HandlePersistAnchorEntityEvent(uint64_t AsyncTaskId, EPICOResult::Type Result, EPICOPersistLocation PersistLocation);
	void HandleUnpersistAnchorEntityEvent(uint64_t AsyncTaskId, EPICOResult::Type Result, EPICOPersistLocation PersistLocation);
	void HandleClearAnchorEntityEvent(uint64_t AsyncTaskId, EPICOResult::Type Result, EPICOPersistLocation PersistLocation);
	void HandleLoadAnchorEntityEvent(uint64_t AsyncTaskId, EPICOResult::Type Result, uint32_t AnchorCount, EPICOPersistLocation PersistLocation);
	void HandleStartSpatialSceneCaptureEvent(uint64_t AsyncTaskId, EPICOResult::Type Result, EPICOSpatialSceneCaptureStatus SpatialSceneCaptureStatus);

	bool IsAnchorValid(AActor* BoundActor);
	bool IsAnchorValid(UPICOAnchorComponent* AnchorComponent);
	UPICOAnchorComponent* GetAnchorComponent(AActor* BoundActor);

private:
	FPICOXRHMD* PICOXRHMD;

	struct FAnchorCreateInfo
	{
		uint64_t AsyncTaskId;
		FPICOCreateAnchorEntityDelegate Delegate;
		UPICOAnchorComponent* AnchorComponent;
	};

	struct FAnchorPersistInfo
	{
		uint64_t AsyncTaskId;
		FPICOPersistAnchorEntityDelegate Delegate;
		TArray<UPICOAnchorComponent*> AnchorComponents;
	};

	struct FAnchorUnpersistInfo
	{
		uint64_t AsyncTaskId;
		FPICOUnpersistAnchorEntityDelegate Delegate;
		TArray<UPICOAnchorComponent*> AnchorComponents;
	};

	struct FAnchorClearInfo
	{
		uint64_t AsyncTaskId;
		FPICOClearAnchorEntityDelegate Delegate;
	};

	struct FAnchorLoadInfo
	{
		uint64_t AsyncTaskId;
		FPICOLoadAnchorEntityDelegate Delegate;
	};

	struct FStartSpatialSceneCaptureInfo
	{
		uint64_t AsyncTaskId;
		FPICOStartSpatialSceneCaptureDelegate Delegate;
	};

	TMap<uint64_t, FAnchorCreateInfo> CreateAnchorBindings;
	TMap<uint64_t, FAnchorPersistInfo> PersistAnchorsBindings;
	TMap<uint64_t, FAnchorUnpersistInfo> UnpersistAnchorsBindings;
	TMap<uint64_t, FAnchorClearInfo> ClearAnchorsBindings;
	TMap<uint64_t, FAnchorLoadInfo> LoadAnchorsBindings;
	TMap<uint64_t, FStartSpatialSceneCaptureInfo> StartSpatialSceneCaptureBindings;

	FDelegateHandle HandleOfPollEvent;
};