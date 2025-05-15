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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOCreateAnchorEntityActionSuccess, EPICOResult, Result, UPICOAnchorComponent*, AnchorEntityComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOCreateAnchorEntityActionFailure, EPICOResult, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICODestroyAnchorEntityActionSuccess, EPICOResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICODestroyAnchorEntityActionFailure, EPICOResult, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOPersistAnchorEntityActionSuccess, EPICOResult, Result, const TArray<UPICOAnchorComponent*>&, AnchorList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOPersistAnchorEntityActionFailure, EPICOResult, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOUnpersistAnchorEntityActionSuccess, EPICOResult, Result, const TArray<UPICOAnchorComponent*>&, AnchorList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOUnpersistAnchorEntityActionFailure, EPICOResult, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOClearAnchorEntityActionSuccess, EPICOResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOClearAnchorEntityActionFailure, EPICOResult, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOLoadAnchorEntityActionSuccess, EPICOResult, Result, const TArray<FAnchorLoadResult>&, AnchorLoadResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOLoadAnchorEntityActionFailure, EPICOResult, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOStartSpatialSceneCaptureActionSuccess, EPICOResult, Result, EPICOSpatialSceneCaptureStatus, SpatialSceneCaptureStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOStartSpatialSceneCaptureActionFailure, EPICOResult, Result);

//////////////////////////////////////////////////////////////////////////
/// Create Anchor Entity
//////////////////////////////////////////////////////////////////////////

/// The interfaces below for managing anchors must be used in event graph and cannot be used in code. 

UCLASS()
class PICOXRMR_API UPICOCreateAnchorEntity_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;
	/// @brief Creates an anchor entity in the app's memory.
	/// @param InBindingActor (In) Sets the actor of the anchor entity.
	/// @param InAnchorEntityTransform (In) Sets the position, rotation, and scale of the anchor entity.
	/// @param InTimeout (In) Sets the maximum time allowed for creating an anchor entity to finish.
	/// @return
	/// - Result: Returns 0 for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// - AnchorEntityComponent: Associated with an Actor. Each component owns a Handle and a UUID, and updates the anchor entity's pose to the Actor during each Actor Tick. 
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOCreateAnchorEntity_AsyncAction* PXR_CreateAnchorEntity_Async(AActor* InBindingActor, const FTransform& InAnchorEntityTransform, float InTimeout);

	UPROPERTY(BlueprintAssignable)
	FPICOCreateAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOCreateAnchorEntityActionFailure OnFailure;

	AActor* BindingActor;

	FTransform AnchorEntityTransform;

	float Timeout;

private:
	void HandleCreateAnchorEntityComplete(EPICOResult Result, UPICOAnchorComponent* AnchorEntityComponent);
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

	/// @brief Destroys an anchor entity in the app's memory.
	/// @param InBoundActor Specifies the bound Actor of the anchor entity.
	/// @return  Returns 0 for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICODestroyAnchorEntityAction* PXR_DestroyAnchorEntity(AActor* InBoundActor);

	UPROPERTY(BlueprintAssignable)
	FPICODestroyAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICODestroyAnchorEntityActionFailure OnFailure;

	AActor* BoundActor;

private:
	void HandleDestroyAnchorEntityComplete(EPICOResult Result);
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

	/// @brief Persists specified anchor entities, which means saving anchor entities to a specified location. Currently, only supports saving anchor entities to the device's local storage.
	/// @param InBoundActors Specifies the bound Actors of the to-be-persisted anchor entities.
	/// @param InPersistLocation The location that the anchor entities are saved to:
	/// - Persist Location Local: device's local storage
	/// - Persist Location Remote: not supported
	/// @return
	/// - Result: Returns 0 for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// - AnchorList:  An array of anchors.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOPersistAnchorEntity_AsyncAction* PXR_PersistAnchorEntity_Async(const TArray<AActor*>& InBoundActors, EPICOPersistLocation InPersistLocation = EPICOPersistLocation::PersistLocation_Local);

	UPROPERTY(BlueprintAssignable)
	FPICOPersistAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOPersistAnchorEntityActionFailure OnFailure;

	TArray<AActor*> BoundActors;

	EPICOPersistLocation PersistLocation;

private:
	void HandlePersistAnchorEntityComplete(EPICOResult Result, const TArray<UPICOAnchorComponent*>& AnchorList);
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

	/// @brief Unpersists specified anchor entities, which means deleting anchor entities from the location where they are saved. Currently, only supports deleting anchor entities saved in the device's local storage.
	/// @param InBoundActors Specifies the bound Actors of the to-be-unpersisted anchor entities.
	/// @param InPersistLocation Specifies the location where the anchor entities are saved:
	/// - Persist Location Local: device's local storage
	/// - Persist Location Remote: not supported
	/// @return
	/// - Result:  Returns 0 for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// - AnchorList:  An array of anchors.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOUnpersistAnchorEntity_AsyncAction* PXR_UnpersistAnchorEntity_Async(const TArray<AActor*>& InBoundActors, EPICOPersistLocation InPersistLocation = EPICOPersistLocation::PersistLocation_Local);

	UPROPERTY(BlueprintAssignable)
	FPICOUnpersistAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOUnpersistAnchorEntityActionFailure OnFailure;

	TArray<AActor*> BoundActors;

	EPICOPersistLocation PersistLocation;

private:
	void HandleUnpersistAnchorEntityComplete(EPICOResult Result, const TArray<UPICOAnchorComponent*>& AnchorList);
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

	/// @brief Clears all anchor entities saved in a specified location. Currently, only supports deleting all anchor entities saved in the device's local storage.
	/// @param InPersistLocation Specifies the location where the to-be-cleared anchor entities are saved.
	/// - Persist Location Local: device's local storage
	/// - Persist Location Remote: not supported
	/// @return Result:  Returns 0 for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOClearAnchorEntity_AsyncAction* PXR_ClearAnchorEntity_Async(EPICOPersistLocation InPersistLocation = EPICOPersistLocation::PersistLocation_Local);

	UPROPERTY(BlueprintAssignable)
	FPICOClearAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOClearAnchorEntityActionFailure OnFailure;

	EPICOPersistLocation PersistLocation;

private:
	void HandleClearAnchorEntityComplete(EPICOResult Result);
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

	/// @brief Loads anchor entities from the device's local storage.
	/// @param InLoadInfo Specifying which anchor entities to load.
	/// - Load Max Count: Specifies the maximum number of anchor entities. The maximum value is 1024.
	/// - Timeout: Sets the maximum time allowed for creating an anchor entity to finish. Currently, this parameter is invalid.
	/// - Persist Location:
	/// 	- Persist Location Local: device's local storage
	/// 	- Persist Location Remote: not supported
	/// - Include Filter Type:
	/// 	- Load Filter Type None: All anchor entities will be loaded.
	/// 	- Load Filter Type UUID: Only anchor entities in your app can be loaded.
	/// 	- Load Filter Type Spatial Scene Data: Only anchor entities in the Room Capture app can be loaded.
	/// - Exclude Filter Type: Currently, this parameter is invalid.
	/// @return  Returns 0 for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOLoadAnchorEntity_AsyncAction* PXR_LoadAnchorEntity_Async(const FPICOAnchorLoadInfo& InLoadInfo);

	UPROPERTY(BlueprintAssignable)
	FPICOLoadAnchorEntityActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOLoadAnchorEntityActionFailure OnFailure;

	FPICOAnchorLoadInfo LoadInfo;

private:
	void HandleLoadAnchorEntityComplete(EPICOResult Result, const TArray<FAnchorLoadResult>& AnchorLoadResults);
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

	/// @brief Launches the Room Capture app to capture the scene.
	/// @return 
	/// - Result:  Returns 0 for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// - SpatialSceneCaptureStatus:  Reserved parameter.
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPICOStartSpatialSceneCapture_AsyncAction* PXR_StartSpatialSceneCapture_Async();

	UPROPERTY(BlueprintAssignable)
	FPICOStartSpatialSceneCaptureActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOStartSpatialSceneCaptureActionFailure OnFailure;

private:
	void HandleStartSpatialSceneCaptureComplete(EPICOResult Result, EPICOSpatialSceneCaptureStatus SpatialSceneCaptureStatus);
};
