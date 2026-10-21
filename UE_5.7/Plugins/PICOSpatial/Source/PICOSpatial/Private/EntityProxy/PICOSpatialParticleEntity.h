// Copyright Epic Games, Inc. All Rights Reserved.
#pragma once

#include "PICOSpatialNiagaraAsset.h"
#include "PICOSpatialPrimitiveEntity.h"

/**
 * Particle entity proxy.
 * Represents a Niagara particle system component in the spatial scene.
 * Handles conversion of Niagara particle properties to Copenhagen particle system format.
 */
class FParticleEntityProxy : public FPrimitiveEntityProxy
{
public:
	/**
	 * Sub-particle entity proxy.
	 * Represents individual particle emitters within a particle system.
	 */
	struct FSubParticleEntityProxy
	{
		// Static data
		
		/** Name of the particle emitter */
		FString Name;

		// Dynamic data
		
		/** Unique identifier for the emitter */
		uint32 Id;
	
		/** Native spatial entity for this sub-emitter */
		TSharedPtr<FNativeEntity, ESPMode::ThreadSafe> SubNativeEntity;

		/** Parent entity containing this sub-emitter */
		TSharedPtr<FNativeEntity, ESPMode::ThreadSafe> ParentEntity;
	};

	/**
	 * Constructor.
	 * @param InParentEntityProxy Parent entity proxy (can be null for root entities)
	 * @param SceneComponent The Niagara component this proxy represents
	 */
	FParticleEntityProxy(FSceneEntityProxy* InParentEntityProxy, USceneComponent* SceneComponent);
	
	virtual ~FParticleEntityProxy() {}

	/** Updates proxy data from the Niagara component */
	virtual void UpdateEntityProxy(USceneComponent* SceneComponent) override;
	
	/** Initializes the native spatial entity with particle system data */
	virtual bool InitializeNativeEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer) override;
	
	/** Updates the native entity with current particle proxy data */
	virtual void UpdateNativeEntity() override;

private:
	/**
	 * Constructs particle-specific entity data.
	 * @param SceneComponent The scene component to extract particle data from
	 * @param bInitialize Whether to perform full initialization or partial update
	 */
	void ConstructParticleEntityData(USceneComponent* SceneComponent, bool bInitialize);
	
	/**
	 * Loads Niagara properties from a PICO Spatial asset.
	 * @param PackagePath Path to the Niagara asset package
	 */
	void LoadNiagaraPropertiesFromAsset(const FString& PackagePath);
	
	/**
	 * Converts Niagara emitter data to Copenhagen particle format.
	 * @param NiagaraEmitterPICO Source Niagara emitter data
	 * @param CopenhagenParticle Target Copenhagen particle data
	 */
	void NiagaraEmitterPICOToCopenhagenParticle(const FNiagaraEmitterPICO& NiagaraEmitterPICO, spatialpack::CopenhagenParticle& CopenhagenParticle);

	// Size handling methods
	
	/** Handles particle size over lifetime curve */
	void HandleSizeOverLifetime(const FNiagaraEmitterPICO& NiagaraEmitterPICO, spatialpack::CopenhagenParticle& CopenhagenParticle, spatialpack::CopenhagenCurveType CurveType);
	
	/** Handles particle initial size */
	void HandleParticleSize(const FNiagaraEmitterPICO& NiagaraEmitterPICO, spatialpack::CopenhagenParticle& CopenhagenParticle);
	
	/** Gets initial size of particles */
	void GetInitializeSizeOfEmitter(const FNiagaraEmitterPICO& NiagaraEmitterPICO, float& InitializeSizeOrScale);
	
	/** Gets update scale factor for particles */
	void GetUpdateScaleFactorOfEmitter(const FNiagaraEmitterPICO& NiagaraEmitterPICO, float& UpdateScaleFactor);
	
	/** Gets random size range for particles */
	void GetInitializeRandomSizeOfEmitter(const FNiagaraEmitterPICO& NiagaraEmitterPICO, float& InitializeMaxSizeOrScale, float& InitializeMinSizeOrScale);
	
	/**
	 * Creates a sub-entity for a particle emitter.
	 * @param InCurrentScene Current spatial scene
	 * @param SpatialContainer Spatial container for the entity
	 * @return Shared pointer to the created sub-entity
	 */
	TSharedPtr<FSubParticleEntityProxy, ESPMode::ThreadSafe> CreateSubEntity(spatialpack::Scene* InCurrentScene, spatialpack::AppForm* SpatialContainer);
	
	/** Loaded Niagara asset */
	TObjectPtr<UPICOSpatialNiagaraAsset> LoadedAssetObjectPtr = nullptr;
	
	/** Array of sub-particle emitter entities */
	TArray<TSharedPtr<FSubParticleEntityProxy, ESPMode::ThreadSafe>> SubEntities;
	
	/** World-space position of the particle system */
	spatialpack::Vector3f ParticlePosition;
};
