// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Delegates/DelegateCombinations.h"
#include "PXR_HMDTypes.h"
#include "PXR_AnchorComponent.h"

#include "PXR_AsyncAnchorAction.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOCreateAnchorEntityActionSuccess, EPICOResult::Type, Result, UPICOAnchorComponent*, AnchorEntityComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOCreateAnchorEntityActionFailure, EPICOResult::Type, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICODestroyAnchorEntityActionSuccess, EPICOResult::Type, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICODestroyAnchorEntityActionFailure, EPICOResult::Type, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOPersistAnchorEntityActionSuccess, EPICOResult::Type, Result, const TArray<UPICOAnchorComponent*>&, AnchorList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOPersistAnchorEntityActionFailure, EPICOResult::Type, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOUnpersistAnchorEntityActionSuccess, EPICOResult::Type, Result, const TArray<UPICOAnchorComponent*>&, AnchorList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOUnpersistAnchorEntityActionFailure, EPICOResult::Type, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOClearAnchorEntityActionSuccess, EPICOResult::Type, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOClearAnchorEntityActionFailure, EPICOResult::Type, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOLoadAnchorEntityActionSuccess, EPICOResult::Type, Result, const TArray<FAnchorLoadResult>&, AnchorLoadResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOLoadAnchorEntityActionFailure, EPICOResult::Type, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOStartSpatialSceneCaptureActionSuccess, EPICOResult::Type, Result, EPICOSpatialSceneCaptureStatus, SpatialSceneCaptureStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOStartSpatialSceneCaptureActionFailure, EPICOResult::Type, Result);

//////////////////////////////////////////////////////////////////////////
/// Create Anchor Entity
//////////////////////////////////////////////////////////////////////////
UCLASS()
class PICOXRMR_API UPICOCreateAnchorEntity_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOCreateAnchorEntity_AsyncAction* PXR_CreateAnchorEntity_Async(AActor* InBindingActor, const FTransform& InAnchorEntityTransfrom, float InTimeout);

	UPROPERTY(BlueprintAssignable)
	FPICOCreateAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOCreateAnchorEntityActionFailure OnFailure;

	AActor* BindingActor;

	FTransform AnchorEntityTransform;

	float Timeout;

private:
	void HandleCreateAnchorEntityComplete(EPICOResult::Type Result, UPICOAnchorComponent* AnchorEntityComponent);
};

//////////////////////////////////////////////////////////////////////////
/// Destroy Anchor Entity
//////////////////////////////////////////////////////////////////////////
UCLASS()
class PICOXRMR_API UPICODestroyAnchorEntityAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICODestroyAnchorEntityAction* PXR_DestroyAnchorEntity(AActor* InBoundActor);

	UPROPERTY(BlueprintAssignable)
	FPICODestroyAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICODestroyAnchorEntityActionFailure OnFailure;

	AActor* BoundActor;

private:
	void HandleDestroyAnchorEntityComplete(EPICOResult::Type Result);
};

//////////////////////////////////////////////////////////////////////////
/// Persist Anchor Entity
//////////////////////////////////////////////////////////////////////////
UCLASS()
class PICOXRMR_API UPICOPersistAnchorEntity_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOPersistAnchorEntity_AsyncAction* PXR_PersistAnchorEntity_Async(const TArray<AActor*>& InBoundActors, EPICOPersistLocation InPersistLocation = EPICOPersistLocation::PersistLocation_Local);

	UPROPERTY(BlueprintAssignable)
	FPICOPersistAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOPersistAnchorEntityActionFailure OnFailure;

	TArray<AActor*> BoundActors;

	EPICOPersistLocation PersistLocation;

private:
	void HandlePersistAnchorEntityComplete(EPICOResult::Type Result, const TArray<UPICOAnchorComponent*>& AnchorList);
};

//////////////////////////////////////////////////////////////////////////
/// Unpersist Anchor Entity
//////////////////////////////////////////////////////////////////////////
UCLASS()
class PICOXRMR_API UPICOUnpersistAnchorEntity_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOUnpersistAnchorEntity_AsyncAction* PXR_UnpersistAnchorEntity_Async(const TArray<AActor*>& InBoundActors, EPICOPersistLocation InPersistLocation = EPICOPersistLocation::PersistLocation_Local);

	UPROPERTY(BlueprintAssignable)
	FPICOUnpersistAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOUnpersistAnchorEntityActionFailure OnFailure;

	TArray<AActor*> BoundActors;

	EPICOPersistLocation PersistLocation;

private:
	void HandleUnpersistAnchorEntityComplete(EPICOResult::Type Result, const TArray<UPICOAnchorComponent*>& AnchorList);
};

//////////////////////////////////////////////////////////////////////////
/// Clear Anchor Entity
//////////////////////////////////////////////////////////////////////////
UCLASS()
class PICOXRMR_API UPICOClearAnchorEntity_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOClearAnchorEntity_AsyncAction* PXR_ClearAnchorEntity_Async(EPICOPersistLocation InPersistLocation = EPICOPersistLocation::PersistLocation_Local);

	UPROPERTY(BlueprintAssignable)
	FPICOClearAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOClearAnchorEntityActionFailure OnFailure;

	EPICOPersistLocation PersistLocation;

private:
	void HandleClearAnchorEntityComplete(EPICOResult::Type Result);
};

//////////////////////////////////////////////////////////////////////////
/// Load Anchor Entity
//////////////////////////////////////////////////////////////////////////
UCLASS()
class PICOXRMR_API UPICOLoadAnchorEntity_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOLoadAnchorEntity_AsyncAction* PXR_LoadAnchorEntity_Async(const FPICOAnchorLoadInfo& InLoadInfo);

	UPROPERTY(BlueprintAssignable)
	FPICOLoadAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOLoadAnchorEntityActionFailure OnFailure;

	FPICOAnchorLoadInfo LoadInfo;

private:
	void HandleLoadAnchorEntityComplete(EPICOResult::Type Result, const TArray<FAnchorLoadResult>& AnchorLoadResults);
};

//////////////////////////////////////////////////////////////////////////
/// Start Spatial Scene Capture
//////////////////////////////////////////////////////////////////////////
UCLASS()
class PICOXRMR_API UPICOStartSpatialSceneCapture_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOStartSpatialSceneCapture_AsyncAction* PXR_StartSpatialSceneCapture_Async();

	UPROPERTY(BlueprintAssignable)
	FPICOStartSpatialSceneCaptureActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOStartSpatialSceneCaptureActionFailure OnFailure;

private:
	void HandleStartSpatialSceneCaptureComplete(EPICOResult::Type Result, EPICOSpatialSceneCaptureStatus SpatialSceneCaptureStatus);
};
