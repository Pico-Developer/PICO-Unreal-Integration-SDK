// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPICOSpatialUSD, Log, All);

class UMaterialInterface;

/**
 * Module for PICO Spatial USD functionality.
 * Provides USD and MaterialX export/import capabilities for materials.
 */
class PICOSPATIALUSD_API FPICOSpatialUSDModule : public IModuleInterface
{
public:
	/**
	 * Gets the loaded module instance.
	 * @return Reference to the loaded PICOSpatialUSD module
	 */
	static inline FPICOSpatialUSDModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FPICOSpatialUSDModule>("PICOSpatialUSD");
	}

	/**
	 * Checks if the module is currently loaded.
	 * @return True if the module is loaded, false otherwise
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("PICOSpatialUSD");
	}

	/** Called when the module is loaded */
	virtual void StartupModule() override;

	/** Called when the module is unloaded */
	virtual void ShutdownModule() override;

	static bool ConvertMaterialToUsd(const UMaterialInterface* Material, const FString& OutputUSDPath);

	static bool CopyAndModifyUsdMaterial(const FString& SourceParentUSDPath, const FString& TargetUSDPath, const UMaterialInterface* TargetMaterial);

	static bool CreateMaterialInstance(const FString& ParentUSDPath, const FString& InstanceUSDPath, const UMaterialInterface* Material);
};
