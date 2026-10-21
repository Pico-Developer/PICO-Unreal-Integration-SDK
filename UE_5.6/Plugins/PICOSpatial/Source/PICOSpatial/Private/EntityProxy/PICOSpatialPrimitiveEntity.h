// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "PICOSpatialSceneEntity.h"
#include "PhysicsEngine/AggregateGeom.h"

/**
 * Proxy for primitive entities in PICO Spatial
 * Handles entities with collision, physics, and input interactions
 * Base class for mesh-based entities
 */
class FPrimitiveEntityProxy : public FSceneEntityProxy
{
public:
	/**
	 * Constructor
	 * @param ParentEntity Parent entity proxy in the hierarchy
	 * @param SceneComponent Scene component this proxy represents
	 */
	FPrimitiveEntityProxy(FSceneEntityProxy* ParentEntity, USceneComponent* SceneComponent);
	
	virtual ~FPrimitiveEntityProxy();

	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	virtual void UpdateNativeEntity() override;

protected:
	/**
	 * Constructs primitive entity data from a USceneComponent
	 * @param SceneComponent Source component to extract data from
	 * @param bInitialize Whether to perform full initialization
	 */
	void ConstructPrimitiveEntityData(USceneComponent* SceneComponent, bool bInitialize);

public:
	// Static data - collision and rendering properties
	
	/** Aggregate collision geometry */
	FKAggregateGeom Collision;
	
	/** Type of collision tracing to use */
	TEnumAsByte<enum ECollisionTraceFlag> CollisionTraceFlag;
	
	/** Whether to create simple collision shapes */
	bool bCreateSimpleCollision;
	
	/** Whether to create complex per-polygon collision */
	bool bCreateComplexCollision;
	
	/** Whether this entity has dynamic physics simulation */
	bool bDynamic;
	
	/** Whether to cast dynamic shadows */
	bool bCastDynamicShadow;

	/** Original value of UPrimitiveComponent::bRenderInMainPass before any PICO override */
	bool bOriginalRenderInMainPass;

	/** Whether PICO actively changed bRenderInMainPass away from its original value */
	bool bDidOverrideRenderInMainPass;

	// Dynamic data - runtime modifiable properties
	
	/** Whether this entity can receive input events */
	bool bEnableInput;
	
	/** Whether collision is enabled */
	bool bEnableCollision;
	
	/** Whether to automatically handle hover events */
	bool bAutoHover;

	// Spatial resources
	
#if PLATFORM_ANDROID
	/** Array of collision shape resources for the spatial SDK */
	TArray<spatialpack::ShapeResource> ShapeResources;
#endif
};
