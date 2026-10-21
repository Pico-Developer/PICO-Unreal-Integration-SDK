// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "PICOSpatial/Public/PICOSpatialFunctionLibrary.h"
#include "AssetRegistry/IAssetRegistry.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPICOSpatialEditor, Log, All);

/**
 * PICO Spatial Editor Module
 *
 * This module provides editor-time functionality for the PICO Spatial plugin, including:
 * - Asset cooking and exporting (GLB, USD, textures)
 * - Dependency tracking and world dependency management
 * - Bundle tool integration for asset packaging
 * - Material conversion to USD shader graphs
 * - Niagara system conversion to PICO spatial format
 *
 * The module extends the LevelEditor toolbar with a "Cook PICO Spatial Content" button
 * that processes all worlds containing an APICOSpatialContainer-derived spatial container and exports their dependencies.
 */
class FPICOSpatialEditorModule : public IModuleInterface
{
public:
	// ============================================================================
	// IModuleInterface Implementation
	// ============================================================================

	/**
	 * Called when the module is loaded during editor startup.
	 * Initializes paths, registers settings, configures packaging settings,
	 * and registers menu extensions.
	 */
	virtual void StartupModule() override;

	/**
	 * Called when the module is unloaded during editor shutdown.
	 * Unregisters settings and menu extensions.
	 */
	virtual void ShutdownModule() override;

private:
	// ============================================================================
	// Core Cook Pipeline
	// ============================================================================

	/**
	 * Executes the main content cooking pipeline.
	 *
	 * Pipeline steps:
	 * 1. Initialize settings and validate bundle tool
	 * 2. Clean previous build artifacts
	 * 3. Scan asset registry and find target worlds
	 * 4. Export assets from target worlds
	 * 5. Handle user cancellation
	 * 6. Execute bundle tool (optional)
	 * 7. Save world dependencies DataTable
	 *
	 * This is the entry point for the cooking process, triggered by the toolbar button
	 * or programmatically after asset registry has been refreshed.
	 */
	void ExecuteCookContent();

	/**
	 * Registers menu extensions to add the "Cook PICO Spatial Content" button
	 * to the LevelEditor toolbar.
	 *
	 * The button saves dirty packages, scans the asset registry, and executes
	 * the cook content pipeline after ensuring all dependencies are loaded.
	 */
	void RegisterMenuExtensions();

	// ============================================================================
	// Asset Export Functions
	// ============================================================================

	/**
	 * Exports non-texture assets (meshes, materials) to GLB or USD format.
	 *
	 * @param Assets                              Map of assets to export with their types
	 * @param ExportedAssets                      Set tracking already exported assets (prevents duplicates)
	 * @param FailedAssets                        Set tracking assets that already failed export in this cook
	 * @param OutGlbMaterials                     Output set of materials exported as GLB
	 * @param OutConvertedMaterialToUsdPath       Output map of materials converted to USD with their file paths
	 * @param OutWorldDependency                  Output dependency record for the current world
	 * @param ProgressTask                        Progress bar to update during export
	 * @param bExportUsdShaderGraphMaterial       Whether to export materials as USD shader graphs
	 * @param OutFailedAssetExports               Output list of assets that failed to export but should not stop the cook
	 * @param bOutCancelled                       Output flag indicating if user cancelled the operation
	 * @return true if export completed successfully or with non-fatal asset failures, false when cancelled
	 */
	bool ExportNonTextureAssets(
		const TMap<UObject*, EAssetTypePICO>& Assets,
		TSet<UObject*>& ExportedAssets,
		TSet<UObject*>& FailedAssets,
		TSet<UObject*>& OutGlbMaterials,
		TMap<const UMaterialInterface*, FString>& OutConvertedMaterialToUsdPath,
		FWorldDependenciesPICO& OutWorldDependency,
		FScopedSlowTask& ProgressTask,
		bool bExportUsdShaderGraphMaterial,
		TArray<FString>& OutFailedAssetExports,
		bool& bOutCancelled);

	/**
	 * Exports texture assets with smart filtering, preferring PNG and falling back to EXR when needed.
	 *
	 * Textures embedded in GLB materials are skipped unless they're also
	 * referenced by non-material assets (e.g., UI, particles).
	 *
	 * @param TexturesInMaterial                  Multi-map of textures referenced by materials
	 * @param TexturesNotInMaterial               Map of textures referenced by non-material assets
	 * @param GlbMaterials                        Set of materials exported as GLB
	 * @param ExportedAssets                      Set tracking already exported assets
	 * @param FailedAssets                        Set tracking textures that already failed export in this cook
	 * @param OutWorldDependency                  Output dependency record for the current world
	 * @param ProgressTask                        Progress bar to update during export
	 * @param OutFailedAssetExports               Output list of textures that failed to export but should not stop the cook
	 * @param bOutCancelled                       Output flag indicating if user cancelled the operation
	 * @return true if export completed successfully or with non-fatal texture failures, false when cancelled
	 */
	bool ExportTextureAssets(
		const TMultiMap<UObject*, UObject*>& TexturesInMaterial,
		const TMap<UObject*, UObject*>& TexturesNotInMaterial,
		const TSet<UObject*>& GlbMaterials,
		TSet<UObject*>& ExportedAssets,
		TSet<UObject*>& FailedAssets,
		TSet<UObject*>& OutExportedTexturesThisCall,
		TMap<UObject*, FString>& OutFailedTextureExportsThisCall,
		FWorldDependenciesPICO& OutWorldDependency,
		FScopedSlowTask& ProgressTask,
		TArray<FString>& OutFailedAssetExports,
		bool& bOutCancelled);

	// ============================================================================
	// Material Conversion
	// ============================================================================

	/**
	 * Recursively converts a material and its parent chain to USD shader graph format.
	 *
	 * For MaterialInstanceConstants, recursively converts the parent material first,
	 * then creates a USD material instance that references the parent.
	 *
	 * @param ConvertedMaterial                   Map tracking already converted materials (cache)
	 * @param Material                            Material to convert
	 * @return true if conversion succeeded or material was already converted, false on error
	 */
	bool ConvertMaterialToUSDRecursive(
		TMap<const UMaterialInterface*, FString>& ConvertedMaterial,
		const UMaterialInterface* Material);

	// ============================================================================
	// Dependency Tracking
	// ============================================================================

	/**
	 * Recursively collects all dependency assets for a given object.
	 *
	 * This function traverses the asset dependency graph starting from the given object,
	 * categorizing assets by type and tracking texture usage context (material vs non-material).
	 *
	 * Special handling:
	 * - Skips UWorld assets to avoid circular dependencies
	 * - Tracks Texture2D separately when bExportUsdShaderGraphMaterial is enabled
	 * - Distinguishes between textures referenced by materials vs other assets
	 * - Converts each Niagara system at most once per cook while still traversing its dependencies
	 *
	 * @param Object                              Object to collect dependencies for
	 * @param DependencyAssetsNoTexture           Output map of non-texture dependencies
	 * @param Searched                            Set of already searched package names (prevents cycles)
	 * @param ConvertedNiagaraSystems             Set of Niagara systems already converted in this cook
	 * @param FailedNiagaraSystems                Set of Niagara systems that already failed conversion in this cook
	 * @param Outer                               Parent object (for context tracking)
	 * @param OuterType                           Type of the outer asset
	 * @param bExportUsdShaderGraphMaterial       Whether to separately track Texture2D assets
	 * @param InMaterialTextures                  Output map of textures referenced by materials
	 * @param NotInMaterialTextures               Output map of textures referenced by non-material assets
	 */
	void GetAllDependencyAssets(
		UObject* Object,
		TMap<UObject*, EAssetTypePICO>& DependencyAssetsNoTexture,
		TSet<FName>& Searched,
		TSet<FName>& ConvertedNiagaraSystems,
		TSet<FName>& FailedNiagaraSystems,
		UObject* Outer,
		EAssetTypePICO OuterType,
		bool bExportUsdShaderGraphMaterial,
		TMultiMap<UObject*/*Texture*/, UObject*/*Outer*/>& InMaterialTextures,
		TMap<UObject*/*Texture*/, UObject*/*Outer*/>& NotInMaterialTextures);

	/**
	 * Recursively traverses the package dependency graph to determine whether a package
	 * references any package from a target set.
	 *
	 * This is used to determine which worlds depend on APICOSpatialContainer-derived
	 * spatial containers, including references that flow through child worlds,
	 * level instances, or other intermediate world packages.
	 *
	 * @param AssetRegistry                       Asset registry to query dependencies
	 * @param PackageName                         Package name to start searching from
	 * @param TargetPackageNames                  Package names that should count as a match
	 * @param SearchedPackages                    Set tracking already searched packages (prevents cycles)
	 * @return true if any package in the dependency graph matches TargetPackageNames
	 */
	bool DoesPackageDependencyGraphContainAnyPackages(
		IAssetRegistry& AssetRegistry,
		FName PackageName,
		const TSet<FName>& TargetPackageNames,
		TSet<FName>& SearchedPackages);

	// ============================================================================
	// Bundle Tool Integration
	// ============================================================================

	/**
	 * Executes the PICO bundle tool to pack exported assets into a bundle file.
	 *
	 * Platform-specific handling:
	 * - Windows: Validates .exe extension
	 * - macOS/Linux: Sets executable permissions using chmod
	 *
	 * Process monitoring:
	 * - Real-time log output capture
	 * - Progress bar with timeout protection (10 minutes)
	 * - User cancellation support
	 *
	 * Post-processing:
	 * - Verifies output bundle file creation
	 * - Optionally cleans temporary assets (if bClearAssetsAfterBundleConvertation is enabled)
	 *
	 * @param bOutUserCancelled Set to true when execution stops because the user cancelled it
	 * @return true if bundle tool executed successfully, false on error, timeout, or user cancellation
	 */
	bool ExecuteBundleTool(bool& bOutUserCancelled);

	// ============================================================================
	// Cleanup Functions
	// ============================================================================

	/**
	 * Resets generated PICO assets and dependency metadata while preserving UserCustomAssets.
	 *
	 * Steps:
	 * 1. Deletes generated files while preserving UserCustomAssets
	 * 2. Clears world dependencies asset
	 *
	 * This ensures a clean slate before each cook operation.
	 */
	void ResetPICOAssetsAndDependencies();

	/**
	 * Deletes generated bundle files if they exist.
	 *
	 * Called at the start of the cook process to remove outdated bundle files
	 * before generating new ones.
	 */
	void DeletePicoBundleFile();

	// ============================================================================
	// Member Variables
	// ============================================================================

	/** Absolute path to the active non-uasset export root (Content/AssetsPICO or Build/Android/assets/AssetsPICO). */
	FString PICOAssetsPath;

	/** Absolute path to the active UserCustomAssets directory under the current non-uasset export root. */
	FString UserCustomAssetsPath;

	/** Absolute path to generated pico_spatial.bundle under Build/Android/assets. */
	FString BundleFilePath;
};
