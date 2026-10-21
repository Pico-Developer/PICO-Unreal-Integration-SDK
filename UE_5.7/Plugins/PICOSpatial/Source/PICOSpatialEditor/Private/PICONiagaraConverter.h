// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Forward declarations
class UNiagaraSystem;
class UNiagaraGraph;
class UNiagaraScript;
class UNiagaraSpriteRendererProperties;
class UNiagaraMeshRendererProperties;
class UPICOSpatialNiagaraAsset;
struct FVersionedNiagaraEmitterData;
struct FNiagaraEmitterPICO;
struct FNiagaraEmitterHandle;

/**
 * Converter class for transforming Niagara particle systems to PICO Spatial format
 * This class handles all aspects of Niagara system conversion including:
 * - System-level properties
 * - Emitter configurations
 * - Renderer properties (Sprite and Mesh)
 * - Script parameters and data interfaces
 */
class FPICONiagaraConverter
{
public:
	/**
	 * Main conversion function - Converts a Niagara System to PICO Spatial Niagara Asset
	 * @param NiagaraSystem The Niagara system to convert
	 * @return True if conversion and save succeed
	 */
	static bool ConvertNiagaraSystem(UNiagaraSystem* NiagaraSystem);

private:
	// ========== System-Level Processing ==========
	
	/**
	 * Processes system-level graph to extract system properties
	 * @param NodeGraph The system's Niagara graph
	 * @param OutAsset The asset to populate with system data
	 */
	static void ProcessSystemGraph(const UNiagaraGraph* NodeGraph, UPICOSpatialNiagaraAsset* OutAsset);

	/**
	 * Processes system update script to extract system parameters
	 * @param SystemScript The system update script
	 * @param OutAsset The asset to populate with system parameters
	 */
	static void ProcessSystemScript(const UNiagaraScript* SystemScript, UPICOSpatialNiagaraAsset* OutAsset);

	// ========== Emitter Processing ==========
	
	/**
	 * Processes a single emitter handle
	 * @param EmitterHandle The emitter handle to process
	 * @param OutAsset The asset to add the processed emitter to
	 */
	static void ProcessEmitter(const FNiagaraEmitterHandle& EmitterHandle, UPICOSpatialNiagaraAsset* OutAsset);

	/**
	 * Processes emitter graph nodes to extract emitter configuration
	 * @param NodeGraph The emitter's Niagara graph
	 * @param OutEmitter The emitter to populate with graph data
	 */
	static void ProcessEmitterGraph(const UNiagaraGraph* NodeGraph, FNiagaraEmitterPICO& OutEmitter);

	/**
	 * Processes emitter event handlers for parent-child particle relationships
	 * @param EmitterData The emitter data containing event handlers
	 * @param EmitterHandle The current emitter handle
	 * @param OutEmitter The emitter to populate
	 * @param OutAsset The asset to update parent-child mapping
	 */
	static void ProcessEmitterEventHandlers(FVersionedNiagaraEmitterData* EmitterData, 
		const FNiagaraEmitterHandle& EmitterHandle,
		FNiagaraEmitterPICO& OutEmitter, 
		UPICOSpatialNiagaraAsset* OutAsset);

	// ========== Renderer Processing ==========
	
	/**
	 * Processes all renderers for an emitter
	 * @param EmitterData The emitter data containing renderers
	 * @param OutEmitter The emitter to populate with renderer info
	 */
	static void ProcessRenderers(FVersionedNiagaraEmitterData* EmitterData, FNiagaraEmitterPICO& OutEmitter);

	/**
	 * Processes a sprite renderer
	 * @param SpriteRenderer The sprite renderer to process
	 * @param OutEmitter The emitter to add sprite renderer data to
	 */
	static void ProcessSpriteRenderer(UNiagaraSpriteRendererProperties* SpriteRenderer, FNiagaraEmitterPICO& OutEmitter);

	/**
	 * Processes a mesh renderer
	 * @param MeshRenderer The mesh renderer to process
	 * @param OutEmitter The emitter to add mesh renderer data to
	 */
	static void ProcessMeshRenderer(UNiagaraMeshRendererProperties* MeshRenderer, FNiagaraEmitterPICO& OutEmitter);

	// ========== Script Processing ==========
	
	/**
	 * Processes emitter scripts to extract parameters
	 * @param EmitterData The emitter data to get scripts from
	 * @param OutEmitter The emitter to populate with script data
	 */
	static void ProcessEmitterScripts(FVersionedNiagaraEmitterData* EmitterData, FNiagaraEmitterPICO& OutEmitter);

	// ========== Asset Creation ==========
	
	/**
	 * Creates and initializes a PICO Spatial Niagara asset
	 * @param NiagaraSystem The source Niagara system
	 * @return The created asset, or nullptr if creation fails
	 */
	static UPICOSpatialNiagaraAsset* CreatePICOAsset(UNiagaraSystem* NiagaraSystem);

	/**
	 * Saves the converted asset to disk
	 * @param Asset The asset to save
	 * @param NiagaraSystem The source Niagara system (for path generation)
	 * @return True if save was successful
	 */
	static bool SaveAsset(UPICOSpatialNiagaraAsset* Asset, UNiagaraSystem* NiagaraSystem);
};
