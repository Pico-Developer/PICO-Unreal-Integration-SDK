// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Forward declarations
class UNiagaraGraph;
class UNiagaraScript;
class UPICOSpatialNiagaraAsset;
class UNiagaraSystem;
struct FNiagaraEmitterPICO;

/**
 * Namespace containing shared Niagara-to-PICO conversion utilities.
 */
namespace NiagaraToPICO
{
	// ========== Tag Constants ==========
	// These are defined in PICOSpatialEditor.cpp
	
	extern const FString LoopDurationTag;
	extern const FString LoopDelayTag;
	extern const FString LoopCountTag;
	
	// ========== Conversion Functions ==========
	
	/**
	 * Converts a Niagara graph to a Niagara asset
	 */
	bool NiagaraGraphToNiagaraAsset(const UNiagaraGraph* NodeGraph, UPICOSpatialNiagaraAsset* NiagaraAsset);
	
	/**
	 * Converts a Niagara graph to a Niagara emitter
	 */
	bool NiagaraGraphToNiagaraEmitter(const UNiagaraGraph* NodeGraph, FNiagaraEmitterPICO& Emitter);
	
	/**
	 * Converts Niagara scripts to a Niagara asset
	 */
	bool NiagaraScriptsToNiagaraAsset(const TArray<UNiagaraScript*>& Scripts, FNiagaraEmitterPICO& Emitter);
	
	// ========== Utility Functions ==========

	/**
	 * Warms Niagara editor state before conversion and optionally saves the source asset.
	 */
	bool PrepareNiagaraSystemForConversion(UNiagaraSystem* NiagaraSystem);
	
	/**
	 * Gets the item name from a long path name
	 */
	FString GetItemName(const FString& LongName);
	
	/**
	 * Gets the module name from a long path name
	 */
	FString GetModuleName(const FString& LongName);
	
	/**
	 * Extracts the initialize particle module name from a stat ID string
	 */
	FString ExtractInitializeParticle(const FString& Input);
	
	/**
	 * Filters trailing numeric characters from a string
	 */
	FString FilterTrailingNumbers(const FString& Input);
	
	/**
	 * Splits a string by dot character
	 */
	bool SplitStringByDot(const FString& InputString, FString& OutFirstPart, FString& OutSecondPart);
}
