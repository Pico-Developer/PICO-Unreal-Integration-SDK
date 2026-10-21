// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "PICO_MRTypes.h"
#include "PICO_AnchorComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PICO_MRFunctionLibrary.generated.h"


// Delegates
//MR
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRAnchorDataUpdatedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRMeshDataUpdatedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRPlaneDataUpdatedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRSceneCaptureDataUpdatedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRLightEstimationDataUpdatedDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRAnchorServiceStateUpdatedDelegate,EMRStatePICO,State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRMeshScanningStateUpdatedDelegate,EMRStatePICO,State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRSceneCaptureServiceStateUpdatedDelegate,EMRStatePICO,State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRPlaneScanningStateUpdatedDelegate,EMRStatePICO,State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRLightEstimationServiceStateUpdatedDelegate,EMRStatePICO,State);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPXRVSTDisplayChangedDelegate,EVSTDisplayStatusPICO,Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPXRResume);

UCLASS(BlueprintType)
class PICOOPENXRMR_API UMRDelegateManagerPICO : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FPXRAnchorDataUpdatedDelegate AnchorDataUpdatedDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FPXRMeshDataUpdatedDelegate MeshDataUpdatedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRPlaneDataUpdatedDelegate PlaneDataUpdatedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRSceneCaptureDataUpdatedDelegate SceneCaptureDataUpdatedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRLightEstimationDataUpdatedDelegate LightEstimationDataUpdatedDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FPXRAnchorServiceStateUpdatedDelegate SpatialAnchorServiceStateUpdatedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRMeshScanningStateUpdatedDelegate MeshScanningStateUpdatedDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FPXRSceneCaptureServiceStateUpdatedDelegate SceneCaptureServiceStateUpdatedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRPlaneScanningStateUpdatedDelegate PlaneScanningStateUpdatedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRLightEstimationServiceStateUpdatedDelegate LightEstimationServiceStateUpdatedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRVSTDisplayChangedDelegate VSTDisplayChangedDelegate;

	UPROPERTY(BlueprintAssignable)
	FPXRResume ResumeDelegate;
};



UCLASS()
class PICOOPENXRMR_API UMRFunctionLibraryPICO : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static UMRDelegateManagerPICO* PICODelegateManager;
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static UMRDelegateManagerPICO* GetMRDelegateManagerPICO();
	/// <summary>
	/// Gets the universally unique identifier (UUID) of an anchor entity.
	/// </summary>
	/// <param name="BoundActor">The bound actor of the anchor entity to get UUID for.</param>
	/// <param name="OutAnchorUUID">Returns the UUID of the anchor entity.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static bool GetAnchorEntityUuidPICO(AActor* BoundActor, FSpatialUUIDPICO& OutAnchorUUID,EResultPICO& OutResult);
	
	/// <summary>
	/// Gets the universally unique identifier (UUID) of an anchor entity.
	/// </summary>
	/// <param name="AnchorComponent">Specifies the AnchorComponent for which you want to get anchor entity pose information.</param>
	/// <param name="OutAnchorUUID">Returns the UUID of the anchor entity.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static bool GetAnchorEntityUuidByComponentPICO(const UAnchorComponentPICO* AnchorComponent, FSpatialUUIDPICO& OutAnchorUUID,EResultPICO& OutResult);
	/// <summary>
	/// Gets the information about the Bounding Box 2D for a SceneCapture.
	/// Before calling this method, you need to load SceneCaptures and get the SceneInfos first. The result contains the UUIDs of SceneCaptures loaded.
	/// </summary>
	/// <param name="UUID"> Specifies the uuid of the SceneCapture. </param>
	/// <param name="OutBoundingBox2D">
	/// * `Center`: Returns the offset of the boundary's position relative to the world position.
	/// * `Extent`: Returns the width and height of the boundary.
	/// </param>
	/// <returns>Bool:
	/// <ul>
	/// <li> `true` - success</li>
	/// <li> `false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool GetSceneBoundingBox2DPICO(const FSpatialUUIDPICO& UUID, FBoundingBox2DPICO& OutBoundingBox2D);

	/// <summary>
	/// Gets the information about the polygon (irregular plane) for an anchor entity.
	/// Before calling this method, you need to load SceneCaptures and get the SceneInfos first. The result contains the UUIDs of SceneCaptures loaded.
	/// </summary>
	/// <param name="UUID"> Specifies the uuid of the SceneCapture. </param>
	/// <param name="OutPolygonVertices">
	/// * `Vertices`: The positions of the polygon's vertices on the X, Y, and Z axis.
	/// </param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool GetSceneBoundingPolygonPICO(const FSpatialUUIDPICO& UUID, TArray<FVector>& OutPolygonVertices);
	
	/// <summary>
	/// Gets the information about the Bounding Box 3D for an SceneCapture.
	/// Before calling this method, you need to load SceneCaptures and get the SceneInfos first. The result contains the UUIDs of SceneCaptures loaded.
	/// </summary>
	/// <param name="UUID"> Specifies the uuid of the SceneCapture. </param>
	/// <param name="OutBoundingBox3D">
	/// * `Center`: Returns the offset of the boundary's position relative to the world position.
	/// * `Extent`: Returns the width, height and depth of the boundary.
	/// </param>
	/// <returns>Bool:
	/// <ul>
	/// <li> `true` - success</li>
	/// <li> `false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool GetSceneBoundingBox3DPICO(const FSpatialUUIDPICO& UUID, FBoundingBox3DPICO& OutBoundingBox3D);
	
	/// <summary>
	/// Gets the pose of an actor's anchor entit by Actor.
	/// </summary>
	/// <param name="BoundActor">Specifies the Actor for which you want to get anchor entity pose information.</param>
	/// <param name="OutTransform">
	/// * `Location`: The location of the anchor entity.
	/// * `Rotation`: The rotation of the anchor entity.
	/// * `Scale`: The scale of the anchor entity. `1` by default.
	/// </param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static bool GetAnchorPoseByActorPICO(AActor* BoundActor, FTransform& OutTransform,EResultPICO& OutResult);

	/// <summary>
	/// Gets the pose of an actor's anchor entity by AnchorComponent.
	/// </summary>
	/// <param name="AnchorComponent">Specifies the AnchorComponent for which you want to get anchor entity pose information.</param>
	/// <param name="OutTransform">
	/// * `Location`: The location of the anchor entity.
	/// * `Rotation`: The rotation of the anchor entity.
	/// * `Scale`: The scale of the anchor entity. `1` by default.
	/// </param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static bool GetAnchorPoseByComponentPICO(UAnchorComponentPICO* AnchorComponent, FTransform& OutTransform,EResultPICO& OutResult);

	/// <summary>
	/// Destroy anchor entity by AnchorComponent.
	/// </summary>
	/// <param name="AnchorComponent">Specifies the AnchorComponent for the anchor you want to destroy.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool DestroyAnchorByComponentPICO(UAnchorComponentPICO* AnchorComponent,EResultPICO& OutResult);

	/// <summary>
	/// Destroy anchor entity by Actor.
	/// </summary>
	/// <param name="BoundActor">Specifies the Actor for the anchor you want to destroy.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool DestroyAnchorByActorPICO(AActor* BoundActor,EResultPICO& OutResult);

	/// <summary>
	/// Checks if an Actor's anchor is valid.
	/// </summary>
	/// <param name="BoundActor">Specifies the Actor for the anchor you want to check.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - valid</li>
	/// <li>`false` - invalid</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static bool IsAnchorValidForActorPICO(AActor* BoundActor);

	/// <summary>
	/// Checks if an AnchorComponent's anchor is valid.
	/// </summary>
	/// <param name="BoundActor">Specifies the AnchorComponent for the anchor you want to check.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - valid</li>
	/// <li>`false` - invalid</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static bool IsAnchorValidForComponentPICO(UAnchorComponentPICO* AnchorComponent);

	/// <summary>
	/// Spawns an actor from the specified anchor load result.
	/// </summary>
	/// <param name="WorldContext">The world context in which the actor will be spawned.</param>
	/// <param name="LoadResult">The anchor load result containing information for spawning, including the anchor's handle, UUID, and location.</param>
	/// <param name="ActorClass"> The class of the actor to be spawned.</param>
	/// <returns>The spawned actor, or nullptr if spawning fails.
	/// </returns>	
	UFUNCTION(BlueprintCallable, Category = "PICO|MR", meta = (WorldContext = "WorldContext", UnsafeDuringActorConstruction = "true"))
	static AActor* SpawnActorFromLoadResultPICO(UObject* WorldContext, const FAnchorLoadResultPICO& LoadResult, UClass* ActorClass);

	/// <summary>
	/// From AnchorHandle To FString.
	/// </summary>
	/// <param name="AnchorHandle">Specifies the AnchorHandle for which you want to convert.</param>
	/// <returns>FString of AnchorHandle.
	/// </returns>	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (Anchor Handle)", CompactNodeTitle = "->", BlueprintAutocast), Category = "PICO|MR")
	static FString FromAnchorToStringPICO(const FSpatialHandlePICO& AnchorHandle);

	/// <summary>
	/// From UUID To FString.
	/// </summary>
	/// <param name="AnchorUUID">Specifies the AnchorUUID for which you want to convert.</param>
	/// <returns>FString of UUID.
	/// </returns>	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString", CompactNodeTitle = "->", BlueprintAutocast), Category = "PICO|MR")
	static FString FromUUIDToStringPICO(const FSpatialUUIDPICO& AnchorUUID);

	/// <summary>
	/// From FString To UUID.
	/// </summary>
	/// <param name="AnchorUUIDString">Specifies the AnchorUUIDString for which you want to convert.</param>
	/// <returns>UUID.
	/// </returns>	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "To UUID PICO", CompactNodeTitle = "->", BlueprintAutocast),Category = "PICO|MR")
	static FSpatialUUIDPICO FromStringToUUIDPICO(const FString& AnchorUUIDString);

	/// <summary>
	/// Closes Spatial Mesh scanning.
	/// </summary>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool CloseSpatialMeshScanningPICO(EResultPICO& OutResult);

	/// <summary>
	/// Closes Spatial Plane scanning.
	/// </summary>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool CloseSpatialPlaneScanningPICO(EResultPICO& OutResult);
	
	/// <summary>
	/// Closes Spatial Anchor service.
	/// </summary>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool CloseSpatialAnchorServicePICO(EResultPICO& OutResult);

	/// <summary>
	/// Closes Scene Capture service.
	/// </summary>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool CloseSceneCaptureServicePICO(EResultPICO& OutResult);

	/// <summary>
	/// Closes Spatial Light Estimation service.
	/// </summary>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool CloseLightEstimationServicePICO(EResultPICO& OutResult);
	/// <summary>
	///  Creates Scene Bounding Polygon.
	/// </summary>
	/// <param name="BoundActor">Specifies the actor bound by polygon.</param>
	/// <param name="bNeverCreateCollision">True if never create collision, false otherwise.</param>
	/// <param name=" bFlipPolygons"> True if you want to flip polygon, false otherwise. It is recommend to be ture in Floor type, otherwise the orientation is down, which may affect rendering and collision.</param>	
	/// <param name="UseWireframe">True if you want to use Wireframe, false otherwise.</param>
	/// <param name="Transform">The transform of polygon.</param>
	/// <param name="Vertices">The set of vertices.</param>
	/// <param name="DefaultMeshMaterial"> Default mesh material.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool CreateSceneBoundingPolygonPICO(AActor* BoundActor,bool bNeverCreateCollision,bool bFlipPolygon,bool UseWireframe,const FTransform& Transform,const TArray<FVector>& Vertices,UMaterialInterface* DefaultMeshMaterial);

	/// <summary>
	///  Creates Scene Bounding Polygon.
	/// </summary>
	/// <param name="BoundActor">Specifies the actor bound by polygon.</param>
	/// <param name="bNeverCreateCollision">True if never create collision, false otherwise.</param>
	/// <param name="bFlipPolygon"> True if you want to flip polygon, false otherwise. It is recommend to be ture in Floor type, otherwise the orientation is down, which may affect rendering and collision.</param>	
	/// <param name="UVAdjustment"> Used to adjust the scaling/rotation/offset of the UVs.</param>
	/// <param name="Transform">The transform of polygon.</param>
	/// <param name="Vertices">The set of vertices.</param>
	/// <param name="DefaultMeshMaterial"> Default mesh material.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool CreateSceneBoundingPolygonWithUVAdjustment(AActor* BoundActor,bool bNeverCreateCollision,bool bFlipPolygon,const FUVAdjustmentPICO& UVAdjustment,const FTransform& Transform,const TArray<FVector>& Vertices,UMaterialInterface* DefaultMeshMaterial,bool bUseOfflineData=false);

	/// <summary>
	/// Changes Spatial Mesh's Lod Setting.
	/// </summary>
	/// <param name="SpatialMeshLod">Specifies the Lod for which you want to set.</param>	
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static bool ChangeSpatialMeshLodSettingPICO(ESpatialMeshLodPICO SpatialMeshLod,EResultPICO& OutResult);

	/// <summary>
	/// Gets Spatial Mesh Lod Setting.
	/// </summary>
	/// <returns>Spatial Mesh Lod.
	/// </returns>	
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static ESpatialMeshLodPICO GetSpatialMeshLodSettingPICO();

	/// <summary>
	/// Gets Spatial Mesh scanning state.
	/// </summary>
	/// <returns>State of Spatial Mesh scanning:
    ///	- Initialized: After checking the Spacial Mesh feature in ProjectSettings, the initialization will be completed during the app startup process.
	///	- Running: This state is set after the Spatial Mesh scanning has been successfully started.
	///	- Stopped: This state is set after the Spatial Mesh scanning has been successfully shut down.
    /// </returns>		
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static EMRStatePICO GetSpatialMeshScanningStatePICO();

	/// <summary>
	/// Get Spatial Anchor service state.
	/// </summary>
	/// <returns>State of Spatial Anchor service:
    ///	- Initialized: After checking the Spatial Anchor feature in ProjectSettings, the initialization will be completed during the app startup process.
	///	- Running: This state is set after the Spatial Anchor service has been successfully started.
	///	- Stopped: This state is set after the Spatial Anchor service has been successfully shut down.
    /// </returns>	
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static EMRStatePICO GetSpatialAnchorServiceStatePICO();

	/// <summary>
	/// Get Scene Capture service state.
	/// </summary>
    /// <returns>State of Scene Capture service:
    ///	- Initialized: After checking the Scene Capture feature in ProjectSettings, the initialization will be completed during the app startup process.
	///	- Running: This state is set after the Scene Capture service has been successfully started.
	///	- Stopped: This state is set after the Scene Capture service has been successfully shut down.
    /// </returns>	
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static EMRStatePICO GetSceneCaptureServiceStatePICO();

	/// <summary>
	/// Get Spatial Plane Scanning state.
	/// </summary>
	/// <returns>State of Spatial Plane Scanning:
	///	- Initialized: After checking the Spatial Plane feature in ProjectSettings, the initialization will be completed during the app startup process.
	///	- Running: This state is set after the Spatial Plane Scanning has been successfully started.
	///	- Stopped: This state is set after the Spatial Plane Scanning has been successfully shut down.
	/// </returns>	
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static EMRStatePICO GetSpatialPlaneScanningStatePICO();

	/// <summary>
	/// Get Light Estimation service state.
	/// </summary>
	/// <returns>State of Light Estimation service:
	///	- Initialized: After checking the Light Estimation feature in ProjectSettings, the initialization will be completed during the app startup process.
	///	- Running: This state is set after the Light Estimation service has been successfully started.
	///	- Stopped: This state is set after the Light Estimation service has been successfully shut down.
	/// </returns>	
	UFUNCTION(BlueprintPure, Category = "PICO|MR")
	static EMRStatePICO GetLightEstimationServiceStatePICO();

	/// <summary>
	/// Reset Spatial Mesh Infos State.
	/// </summary>
	UFUNCTION(BlueprintCallable, Category = "PICO|MR")
	static void ResetSpatialMeshInfosStatePICO();
};