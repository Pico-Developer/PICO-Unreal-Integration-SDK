// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Delegates/DelegateCombinations.h"
#include "PICO_AnchorComponent.h"

#include "PICO_MRAsyncActions.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOCreateSpatialAnchorActionSuccess, EResultPICO, Result,const UAnchorComponentPICO*, AnchorComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOCreateSpatialAnchorActionFailure, EResultPICO, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOPersistSpatialAnchorActionSuccess, EResultPICO, Result,const UAnchorComponentPICO*, AnchorComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOPersistSpatialAnchorActionFailure, EResultPICO, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOUnpersistSpatialAnchorActionSuccess, EResultPICO, Result,const UAnchorComponentPICO*, AnchorComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOUnpersistSpatialAnchorActionFailure, EResultPICO, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOLoadSpatialAnchorActionSuccess, EResultPICO, Result, const TArray<FAnchorLoadResultPICO>&, AnchorLoadResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOLoadSpatialAnchorActionFailure, EResultPICO, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICOStartSpatialSceneCaptureActionSuccess, EResultPICO, Result, ESpatialSceneCaptureStatusPICO, SpatialSceneCaptureStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOStartSpatialSceneCaptureActionFailure, EResultPICO, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOStartProviderActionResult,EResultPICO, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICOLaunchSceneCaptureAppActionResult,EResultPICO, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICORequestSpatialMeshActionSuccess,EResultPICO, Result, const TArray<FSpatialMeshInfoPICO>&, MeshInfos);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICORequestSpatialMeshActionFailure,EResultPICO, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICORequestSpatialPlaneActionSuccess,EResultPICO, Result, const TArray<FSpatialPlaneInfoPICO>&, PlaneInfos);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICORequestSpatialPlaneActionFailure,EResultPICO, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPICORequestSceneCapturesActionSuccess,EResultPICO, Result, const TArray<FMRSceneInfoPICO>&, SceneInfos);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICORequestSceneCapturesActionFailure,EResultPICO, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPICORequestLightEstimationActionSuccess,EResultPICO, Result,const ULightEstimationTexturePICO*, LightEstimationTexture,const float,SourceCubemapAngle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPICORequestLightEstimationActionFailure,EResultPICO, Result);

//////////////////////////////////////////////////////////////////////////
/// Spatial Anchor
//////////////////////////////////////////////////////////////////////////

/* UAsyncTask_LaunchAnchorService
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API ULaunchSpatialAnchorServicePICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;


	/// <summary>
	/// Launch Spatial Anchor service.
	/// </summary>
	/// <returns>
	/// Result: Returns 0 for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// </returns>
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static ULaunchSpatialAnchorServicePICO_AsyncAction* LaunchSpatialAnchorServicePICO_Async();

	UPROPERTY(BlueprintAssignable)
	FPICOStartProviderActionResult OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOStartProviderActionResult OnFailure;

private:
	void HandleLaunchSpatialAnchorServiceComplete(const XrFutureEXT& FutureHandle);
};

/* UAsyncTask_CreateAnchorEntity
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API UCreateSpatialAnchorPICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	/// <summary>
	/// Creates an anchor entity in the app's memory.
	/// </summary>
	/// <param name="BoundActor">(In) Sets the actor of the anchor entity.</param>
	/// <param name="InSpatialAnchorTransform">(In) Sets the position, rotation, and scale of the anchor entity.</param>
	/// <returns>
	/// - Result: Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// - AnchorComponent: Associated with an Actor. Each component updates the anchor entity's pose to the Actor during each Actor Tick. 
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UCreateSpatialAnchorPICO_AsyncAction* CreateSpatialAnchorPICO_Async(AActor* BoundActor, const FTransform& InSpatialAnchorTransform);

	UPROPERTY(BlueprintAssignable)
	FPICOCreateSpatialAnchorActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOCreateSpatialAnchorActionFailure OnFailure;

	UPROPERTY(Transient)
	AActor* BoundActor;

	FTransform SpatialAnchorTransform;

private:
	void HandleCreateSpatialAnchorComplete(const XrFutureEXT& FutureHandle);

};

/* UAsyncTask_PersistAnchorEntity
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API UPersistSpatialAnchorPICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;
	/// <summary>
	/// Make specified anchor entities persistent, which means saving achor entities to specified location. You can choose local storage location or cloud storage location.
	/// </summary>
	/// <param name="BoundActor">Specifies the bound Actors of the to-be-persisted anchor entities.</param>
	/// <param name="PersistLocation">The location that the anchor entities are saved to:
	/// - Persist Location Local: device's local storage.
	/// - Persist Location Shared: cloud storage.
	/// </param>
	/// <returns>
	/// - Result: Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// - AnchorComponent: Associated with an Actor. Each component updates the anchor entity's pose to the Actor during each Actor Tick. 
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UPersistSpatialAnchorPICO_AsyncAction* PersistSpatialAnchorPICO_Async(AActor* BoundActor, EPersistLocationPICO PersistLocation = EPersistLocationPICO::PersistLocation_Local);

	UPROPERTY(BlueprintAssignable)
	FPICOPersistSpatialAnchorActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOPersistSpatialAnchorActionFailure OnFailure;

	UPROPERTY()
	AActor* BoundActor;

	EPersistLocationPICO PersistLocation;

private:
	void HandlePersistSpatialAnchorComplete(const XrFutureEXT& FutureHandle);
	void HandleShareSpatialAnchorComplete(const XrFutureEXT& FutureHandle);

};

/* UAsyncTask_UnpersistAnchorEntity
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API UUnpersistSpatialAnchorPICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	/// <summary>
	/// Unpersists specified anchor entities, which means deleting anchor entities from the location where they are saved. Currently, it only supports deleting anchor entities saved in the device's local storage.
	/// </summary>
	/// <param name="BoundActor">Specifies the bound Actors of the to-be-unpersisted anchor entities.</param>
	/// <param name="PersistLocation">Specifies the location where the anchor entities are saved:
	/// - Persist Location Local: device's local storage.
	/// - Persist Location Shared: not supported.
	/// </param>
	/// <returns>
	/// - Result: Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// - AnchorComponent: Associated with an Actor. Each component updates the anchor entity's pose to the Actor during each Actor Tick. 
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UUnpersistSpatialAnchorPICO_AsyncAction* UnpersistSpatialAnchorPICO_Async(AActor* BoundActor, EPersistLocationPICO PersistLocation = EPersistLocationPICO::PersistLocation_Local);

	UPROPERTY(BlueprintAssignable)
	FPICOUnpersistSpatialAnchorActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOUnpersistSpatialAnchorActionFailure OnFailure;

	UPROPERTY()
	AActor* BoundActor;

	EPersistLocationPICO PersistLocation;

private:
	void HandleUnpersistSpatialAnchorComplete(const XrFutureEXT& FutureHandle);

};

/* UAsyncTask_LoadAnchorEntity
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API URequestSpatialAnchorsPICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;


	/// <summary>
	/// Loads anchor entities from the device's local storage or cloud storage.
	/// </summary>
	/// <param name="LoadInfo">Specifying which anchor entities to load.
	/// - Persist Location:
	///   	- Persist Location Local: device's local storage
	/// 	- Persist Location Shared: cloud storage
	/// </param>
	/// <returns>
	/// - Result: Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// - AnchorComponent: Associated with an Actor. Each component updates the anchor entity's pose to the Actor during each Actor Tick. 
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static URequestSpatialAnchorsPICO_AsyncAction* LoadSpatialAnchorPICO_Async(const FAnchorLoadInfoPICO& LoadInfo);

	UPROPERTY(BlueprintAssignable)
	FPICOLoadSpatialAnchorActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOLoadSpatialAnchorActionFailure OnFailure;

	FAnchorLoadInfoPICO LoadInfo;

private:
	void HandleLoadSpatialAnchorsComplete(const XrFutureEXT& FutureHandle);
	void HandleDownloadSharedAnchorsComplete(const XrFutureEXT& FutureHandle);

	TSet<XrFutureEXT> FutureHandleSet;
	TArray<FAnchorLoadResultPICO> SharedAnchorLoadResults;
};

//////////////////////////////////////////////////////////////////////////
/// Spatial Mesh
//////////////////////////////////////////////////////////////////////////
/* UAsyncTask_LaunchMRMesh
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API ULaunchSpatialMeshScanningPICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	/// <summary>
	/// Launches Spatial Mesh scanning.
	/// </summary>
	/// <returns>
	/// Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static ULaunchSpatialMeshScanningPICO_AsyncAction* LaunchSpatialMeshScanningPICO_Async();

	UPROPERTY(BlueprintAssignable)
	FPICOStartProviderActionResult OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOStartProviderActionResult OnFailure;

private:
	void HandleLaunchMeshScanningComplete(const XrFutureEXT& FutureHandle);
};

/* UAsyncTask_RequestSpatialMesh
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API URequestSpatialMeshPICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	/// <summary>
	/// Requests Spatial Mesh Infos.
	/// </summary>
	/// <returns>
	/// - Result: Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// - MeshInfos:An array of SpatialMeshInfo, contains Mesh UUID, pose, vertices, indices and so on.
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static URequestSpatialMeshPICO_AsyncAction* RequestSpatialMeshInfosPICO_Async();

	UPROPERTY(BlueprintAssignable)
	FPICORequestSpatialMeshActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICORequestSpatialMeshActionFailure OnFailure;

private:
	void HandleRequestSpatialMeshComplete(const XrFutureEXT& FutureHandle);
};
//////////////////////////////////////////////////////////////////////////
/// Spatial Plane
//////////////////////////////////////////////////////////////////////////
/* UAsyncTask_LaunchMRPlane
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API ULaunchSpatialPlaneScanningPICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	/// <summary>
	/// Launches Spatial Plane scanning.
	/// </summary>
	/// <returns>
	/// Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static ULaunchSpatialPlaneScanningPICO_AsyncAction* LaunchSpatialPlaneScanningPICO_Async();

	UPROPERTY(BlueprintAssignable)
	FPICOStartProviderActionResult OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOStartProviderActionResult OnFailure;

private:
	void HandleLaunchPlaneScanningComplete(const XrFutureEXT& FutureHandle);
};

/* UAsyncTask_RequestSpatialPlane
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API URequestSpatialPlanePICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	/// <summary>
	/// Requests Spatial Plane Infos.
	/// </summary>
	/// <returns>
	/// - Result: Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// - PlaneInfos:An array of SpatialPlaneInfo, contains Plane UUID, pose, vertices, indices and so on.
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static URequestSpatialPlanePICO_AsyncAction* RequestSpatialPlaneInfosPICO_Async();

	UPROPERTY(BlueprintAssignable)
	FPICORequestSpatialPlaneActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICORequestSpatialPlaneActionFailure OnFailure;

private:
	void HandleRequestSpatialPlaneComplete(const XrFutureEXT& FutureHandle);
};
//////////////////////////////////////////////////////////////////////////
/// Scene Capture
//////////////////////////////////////////////////////////////////////////
/* UAsyncTask_LaunchSceneCaptureService
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API ULaunchSceneCaptureServicePICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	/// <summary>
	/// Launches SceneCapture service.
	/// </summary>
	/// <returns>
	/// Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static ULaunchSceneCaptureServicePICO_AsyncAction* LaunchSceneCaptureServicePICO_Async();

	UPROPERTY(BlueprintAssignable)
	FPICOStartProviderActionResult OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOStartProviderActionResult OnFailure;

private:
	void HandleLaunchSceneCaptureServiceComplete(const XrFutureEXT& FutureHandle);
};

/* UAsyncTask_LaunchSceneCaptureApp
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API ULaunchSceneCaptureAppPICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	/// <summary>
	/// Launches SceneCapture App.
	/// </summary>
	/// <returns>
	/// Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static ULaunchSceneCaptureAppPICO_AsyncAction* LaunchSceneCaptureAppPICO_Async();

	UPROPERTY(BlueprintAssignable)
	FPICOLaunchSceneCaptureAppActionResult OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOLaunchSceneCaptureAppActionResult OnFailure;
	
private:
	void HandleLaunchSceneCaptureAppComplete(const XrFutureEXT& FutureHandle);
};

/* UAsyncTask_RequestSpatialSceneInfo
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API URequestSceneCapturesPICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	/// <summary>
	/// Request SceneCaptures.
	/// </summary>
	/// <returns>
	/// - Result: Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes only in the result.
	/// - SceneloadInfos: An array of SceneInfo, contains UUID, pose, Semantic, SceneType and so on.
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static URequestSceneCapturesPICO_AsyncAction* RequestSceneCapturesPICO_Async(const FSceneLoadInfoPICO& SceneLoadInfo);

	UPROPERTY(BlueprintAssignable)
	FPICORequestSceneCapturesActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICORequestSceneCapturesActionFailure OnFailure;

	FSceneLoadInfoPICO SceneLoadInfo;

private:
	void HandleRequestSpatialSceneComplete(const XrFutureEXT& FutureHandle);
};

//////////////////////////////////////////////////////////////////////////
/// Light Estimatione
//////////////////////////////////////////////////////////////////////////
/* UAsyncTask_LaunchSceneCaptureService
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API ULaunchLightEstimationServicePICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;

	/// <summary>
	/// Launches SceneCapture service.
	/// </summary>
	/// <returns>
	/// Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes.
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static ULaunchLightEstimationServicePICO_AsyncAction* LaunchLightEstimationServicePICO_Async();

	UPROPERTY(BlueprintAssignable)
	FPICOStartProviderActionResult OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICOStartProviderActionResult OnFailure;

private:
	void HandleLaunchLightEstimationServiceComplete(const XrFutureEXT& FutureHandle);
};

/* UAsyncTask_RequestLightEstimationInfo
 *****************************************************************************/
UCLASS()
class PICOOPENXRMR_API URequestLightEstimationPICO_AsyncAction : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	virtual void Activate() override;
	/// <summary>
	/// Request SceneCaptures.
	/// </summary>
	/// <returns>
	/// - Result: Returns `0` for success and other values for failure. For failure reasons, refer to the EPICOResult enum. Currently, the returned error message result is unique, and you temporarily can not identify the error causes only in the result.
	/// - SceneloadInfos: An array of SceneInfo, contains UUID, pose, Semantic, SceneType and so on.
	/// </returns>	
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static URequestLightEstimationPICO_AsyncAction* RequestLightEstimationPICO_Async();

	UPROPERTY(BlueprintAssignable)
	FPICORequestLightEstimationActionSuccess OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FPICORequestLightEstimationActionFailure OnFailure;

private:
	/** The cube map of the reflected environment */
	UPROPERTY()
	ULightEstimationTexturePICO* EnvironmentCaptureTexture;

	float SourceCubemapAngle;

	void HandleRequestLightEstimationComplete(const XrFutureEXT& FutureHandle);
};