// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PICOSpatialUtility.h"

// Forward declarations
class FPICOSpatial;

/**
 * Wrapper for native spatial entity
 * Manages the lifecycle of a spatialpack::SceneEntity
 */
struct FNativeEntity
{
	FNativeEntity() {}
	~FNativeEntity();
	
	/** Check if the native entity is valid and initialized */
	bool IsValid();
	
#if PLATFORM_ANDROID
	/** The underlying PICO Spatial SDK scene entity */
	spatialpack::SceneEntity entity;
#endif // PLATFORM_ANDROID
};

/**
 * Base proxy class for scene entities in PICO Spatial
 * Bridges Unreal scene components with PICO Spatial SDK entities
 */
class FSceneEntityProxy
{
public:
	/**
	 * Constructor
	 * @param InParentEntityProxy Parent entity proxy in the hierarchy
	 * @param SceneComponent Scene component this proxy represents
	 */
	FSceneEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	
	virtual ~FSceneEntityProxy();

	/**
	 * Updates the proxy with current component data
	 * @param SceneComponent Component to update from
	 */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent);
	
	/**
	 * Initializes the native spatial entity
	 * @param InCurrentScene The spatial scene to add the entity to
	 * @param SpatialContainer The spatial container managing this entity
	 * @return True if initialization succeeded
	 */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer);
	
	/** Updates the native entity with current proxy data */
	virtual void UpdateNativeEntity();

protected:
	/** 
	 * Constructs scene entity data from a USceneComponent
	 * @param SceneComponent Source component to extract data from
	 * @param bInitialize Whether to perform full initialization
	 */
	void ConstructSceneEntityData(USceneComponent* SceneComponent, bool bInitialize);

	/**
	 * Get cached PICOSpatial instance
	 * Caches the singleton reference on first access to avoid repeated Get() calls
	 * @return Reference to the FPICOSpatial singleton
	 */
	FPICOSpatial& GetCachedPICOSpatial();

public:
	// Static data - set once during construction
	
	/** Type of this entity proxy */
	EEntityProxyType Type;
	
	/** The USceneComponent this proxy represents. TWeakObjectPtr is automatically nulled by GC,
	 *  preventing dangling pointer access when the component is destroyed before the proxy. */
	TWeakObjectPtr<USceneComponent> Owner;
	
	/** Display name for the entity */
	FString Name;
	
	/** True if this is the root entity of a spatial container */
	bool bIsSpatialContainerRootEntity;

	// Dynamic data - can change during runtime
	
	/** Parent entity in the hierarchy */
	FSceneEntityProxy* ParentEntityProxy;
	
	/** Native parent entity reference */
	FNativeEntity* ParentEntity;
	
	/** Current transform in world space */
	FTransform Transform;
	
	/** Previously stored transform for dirty checking */
	FTransform StoredTransform;
	
	/** True if transform has changed since last update */
	bool bTransformDirty;
	
	/** Current visibility state */
	bool bVisible;

	// Native spatial entity
	
	/** Thread-safe shared pointer to the native spatial entity */
	TSharedPtr<FNativeEntity, ESPMode::ThreadSafe> NativeEntity;

	/** Broadcast the proxy-created event once the native entity is actually ready. */
	void NotifyProxyCreatedIfNeeded();

private:
	/** Cached pointer to FPICOSpatial singleton to avoid repeated Get() calls */
	FPICOSpatial* CachedPICOSpatialPtr = nullptr;

	/** True once OnComponentProxyCreated has been broadcast for this proxy. */
	bool bHasBroadcastProxyCreated = false;
};
