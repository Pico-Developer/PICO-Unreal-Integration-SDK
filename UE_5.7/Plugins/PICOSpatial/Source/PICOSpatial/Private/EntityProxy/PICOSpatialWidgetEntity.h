// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PICOSpatialMeshEntity.h"

#if PLATFORM_ANDROID
#include "IVulkanDynamicRHI.h"
#include "IOpenGLDynamicRHI.h"
#endif

/**
 * Widget entity proxy.
 * Represents a UMG widget component rendered in the spatial scene.
 * Handles widget rendering to texture and synchronization with native spatial entities.
 */
class FWidgetEntityProxy : public FMeshEntityProxy
{
public:
	/**
	 * Constructor.
	 * @param InParentEntityProxy Parent entity proxy (can be null for root entities)
	 * @param SceneComponent The widget component this proxy represents
	 */
	FWidgetEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	
	virtual ~FWidgetEntityProxy() {}

	/** Updates proxy data from the widget component */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	
	/** Initializes the native spatial entity with widget data */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	
	/** Updates the native entity with current widget proxy data */
	virtual void UpdateNativeEntity() override;

protected:
	/**
	 * Constructs widget-specific entity data.
	 * @param SceneComponent The scene component to extract widget data from
	 * @param bInitialize Whether to perform full initialization or partial update
	 */
	void ConstructWidgetEntityData(USceneComponent* SceneComponent, bool bInitialize);

public:
	/** Size of the widget in 2D space */
	FVector2D Size;
	
	/** Render target for the widget */
	UTextureRenderTarget2D*	WidgetRenderTarget = nullptr;

#if PLATFORM_ANDROID
	/** Native mesh resource for widget plane */
	spatialpack::MeshResource WidgetMesh;
	
	/** Native material for widget rendering */
	spatialpack::Material WidgetMaterial;
	
	/** Render texture buffer queue for double buffering */
	spatialpack::RenderTextureBufferQueue mRenderTextureBufferQueue;
	
	/** Current render texture resource */
	spatialpack::TextureResource renderTexture;
	
	/** Map of render texture resources to RHI textures */
	TMap<spatialpack::ResourceId, FTextureRHIRef> RenderTexturesRHI;
	
	/** Currently active rendering texture */
	spatialpack::TextureResource CurrentRenderingTexture;
#endif

	/** GPU fence for tracking copy completion */
	FGPUFenceRHIRef CopyCompleteFence;
	
	/** Whether a copy operation is in progress */
	bool bCopyInProgress = false;

private:
	/** Whether the widget requires manual updates */
	bool bManuallyUpdateWidget = false;
};
