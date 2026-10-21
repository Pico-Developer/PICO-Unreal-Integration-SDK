// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PICOSpatialSceneEntity.h"

/**
 * Scene capture entity proxy.
 * Represents a scene capture component in the spatial scene.
 * Handles camera perspective, rendering settings, and capture modes for spatial rendering.
 */
class FSceneCaptureEntityProxy : public FSceneEntityProxy
{
public:
	/**
	 * Constructor.
	 * @param InParentEntityProxy Parent entity proxy (can be null for root entities)
	 * @param SceneComponent The scene capture component this proxy represents
	 */
	FSceneCaptureEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	virtual ~FSceneCaptureEntityProxy() override;
	
	/** Updates proxy data from the scene capture component */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	
	/** Initializes the native spatial entity with capture settings */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	
	/** Updates the native entity with current capture proxy data */
	virtual void UpdateNativeEntity() override;

protected:
	/**
	 * Constructs scene capture-specific entity data.
	 * @param SceneComponent The scene component to extract capture data from
	 * @param bInitialize Whether to perform full initialization or partial update
	 */
	void ConstructSceneCaptureEntityData(USceneComponent* SceneComponent, bool bInitialize);

public:
	/** Field of view in degrees */
	float FOV = 90.0;
	
	/** Aspect ratio of the capture */
	float Ratio = 1.0;
	
	/** Near clipping plane distance */
	float Near = 0.0f;
	
	/** Far clipping plane distance */
	float Far = 10000.0f;
	
	/** X resolution of the capture */
	int X = 0;
	
	/** Y resolution of the capture */
	int Y = 0;

	/** Whether scene capture is currently active */
	bool bIsCapturing = false;
	
	/** Whether to show debug visualization plane */
	bool bShowDebugPlane = false;
	
	/** Whether to use anti-aliasing */
	bool bUseAA = false;
	
	/** Whether to capture every frame */
	bool bCaptureEveryFrame = false;
	
	/** Whether to capture when camera moves */
	bool bCaptureOnMove = false;
	
	/** Whether to manually trigger a single frame capture */
	bool bManualCaptureOneFrame = false;

	/** Cached debug plane width in meters so mesh updates only when needed */
	float DebugPlaneWidthMeters = 0.0f;

	/** Cached debug plane height in meters so mesh updates only when needed */
	float DebugPlaneHeightMeters = 0.0f;

#if PLATFORM_ANDROID
	/** Native render target texture */
	spatialpack::TextureResource NativeRT;

        /** Cached debug preview material to avoid rebinding every frame */
        spatialpack::Material DebugPlaneMaterial;

        /** Tracks which native render target resource is currently bound to the debug preview material */
        uint64 DebugPlaneMaterialResourceId = 0;

	/** Intermediate native entity used to rotate the preview plane without affecting the capture camera */
	spatialpack::SceneEntity DebugPlaneOffsetEntity;
#endif
};
