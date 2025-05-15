// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once
#include "CoreMinimal.h"
#include "PXR_MRTypes.h"
#include "PXR_AnchorComponent.h"
#include "PXR_MRFunctionLibrary.generated.h"

UCLASS()
class PICOXRMR_API UPICOXRMRFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/// <summary>
	/// Gets the universally unique identifier (UUID) of an anchor entity.
	/// </summary>
	/// <param name="BoundActor">The the bound actor of the anchor entity to get UUID for.</param>
	/// <param name="OutAnchorUUID">Returns the UUID of the anchor entity.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorEntityUuid(AActor* BoundActor, FPICOAnchorUUID& OutAnchorUUID);

	/// <summary>
	/// Gets the components supported by an anchor entity.
	/// </summary>
	/// <param name="BoundActor">Specifies the actor of the anchor entity to get supported components for.</param>
	/// <param name="OutAnchorComponentFlags">Returns the flags of the supported components. For details, see [EPICOAnchorComponentTypeFlag](https://developer-cn.pico-interactive.com/en/document/unreal/space-calibration/#185193e6) enum.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorComponentFlags(AActor* BoundActor, TArray<EPICOAnchorComponentTypeFlag>& OutAnchorComponentFlags);

	/// <summary> Gets the scene label of an anchor entity. </summary>
	/// <param name="BoundActor">Specifies the actor of the anchor entity.</param>
	/// <param name="OutAnchorSceneLabel">Returns the anchor entity's scene label. For details, see [EPICOAnchorSceneLabel](https://developer-cn.pico-interactive.com/en/document/unreal/space-calibration/#8f95bfea) enum.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorSceneLabel(AActor* BoundActor, EPICOAnchorSceneLabel& OutAnchorSceneLabel);

	/// <summary>
	/// Gets the information about the boundary (rectangle) for an anchor entity.
	/// Before calling this method, you need to load anchor entities and get the anchor entity load result first. The result contains the actors and UUIDs of anchor entities loaded.
	/// </summary>
	/// <param name="BoundActor">Specifies the actor of the anchor entity. </param>
	/// <param name="OutAnchorPlaneBoundaryInfo">
	/// * `Center`: Returns the offset of the boundary's position relative to the anchor entity's position.
	/// * `Extent`: Returns the width and height of the boundary.
	/// </param>
	/// <returns>Bool:
	/// <ul>
	/// <li> `true` - success</li>
	/// <li> `false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorPlaneBoundaryInfo(AActor* BoundActor, FPICOAnchorPlaneBoundaryInfo& OutAnchorPlaneBoundaryInfo);

	/// <summary>
	/// Gets the information about the polygon (irregular plane) for an anchor entity.
	/// Before calling this method, you need to load anchor entities and get the anchor entity load result first.
	/// </summary>
	/// <param name="BoundActor">Specifies the actor of the anchor entity. </param>
	/// <param name="OutAnchorPlanePolygonInfo">
	/// * `Vertices Num`: The number of vertices.
	/// * `Vertices`: The positions of the polygon's vertices on the X, Y, and Z axis.
	/// </param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorPlanePolygonInfo(AActor* BoundActor, FPICOAnchorPlanePolygonInfo& OutAnchorPlanePolygonInfo);

	/// <summary>
	/// Gets the information about the volume for an anchor entity.
	/// Before calling this method, you need to load anchor entities and get the anchor entity load result first.
	/// </summary>
	/// <param name="BoundActor">Specifies the actor of the anchor entity.</param>
	/// <param name="OutAnchorVolumeInfo">
	/// * `Center`: The offset of the volume's position relative to the anchor entity's position.
	/// * `Extent`: The length, width, and height of the volume.
	/// </param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorVolumeInfo(AActor* BoundActor, FPICOAnchorVolumeInfo& OutAnchorVolumeInfo);

	/// <summary>
	/// Gets the pose of a component's anchor entity.
	/// </summary>
	/// <param name="BoundComponent">Specifies the component for which you want to get anchor entity pose information.</param>
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
	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorPoseByComponent(UPICOAnchorComponent* BoundComponent, FTransform& OutTransform);

	/// <summary>
	/// Gets the pose of an actor's anchor entity.
	/// </summary>
	/// <param name="BoundActor">Specifies the actor for which you want to get anchor entity pose information.</param>
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
	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_GetAnchorPoseByActor(AActor* BoundActor, FTransform& OutTransform);

	/// @brief  Spawns an actor from the specified anchor load result.
	/// @param WorldContext The world context in which the actor will be spawned.
	/// @param LoadResult The anchor load result containing information for spawning, including the anchor's handle, UUID, and location.
	/// @param ActorClass The class of the actor to be spawned.
	/// @return The spawned actor, or nullptr if spawning fails.
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRMR", meta = (WorldContext = "WorldContext", UnsafeDuringActorConstruction = "true"))
	static AActor* PXR_SpawnActorFromLoadResult(UObject* WorldContext, const FAnchorLoadResult& LoadResult, UClass* ActorClass);

	/// @brief Checks if an actor's anchor is valid.
	/// @param BoundActor Specifies the actor for which you want to check.
	/// @return True if the anchor is valid, false otherwise.
	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_IsAnchorValidForActor(AActor* BoundActor);
	
	/// @brief Checks if a component's anchor is valid.
	/// @param BoundComponent Specifies the component for which you want to check.
	/// @return True if the anchor is valid, false otherwise.
	UFUNCTION(BlueprintPure, Category = "PXR|PXRMR")
	static bool PXR_IsAnchorValidForComponent(UActorComponent* BoundComponent);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (Anchor Handle)", CompactNodeTitle = "->", BlueprintAutocast), Category = "PXR|PXRMR")
	static FString PXR_FromAnchorToString(const FPICOAnchor& Anchor);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "ToString (Anchor UUID)", CompactNodeTitle = "->", BlueprintAutocast), Category = "PXR|PXRMR")
	static FString PXR_FromUUIDToString(const FPICOAnchorUUID& AnchorUUID);
};
