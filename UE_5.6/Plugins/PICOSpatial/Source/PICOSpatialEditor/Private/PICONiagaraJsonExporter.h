// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Forward declarations
class UPICOSpatialNiagaraAsset;

/**
 * Exporter for JSON sidecar files.
 * Saves a simplified JSON representation of UPICOSpatialNiagaraAsset,
 * with resources represented by their paths instead of raw pointers.
 * Controlled by console variable PICO.Niagara.ExportJson.
 */
class PICONiagaraJsonExporter
{
public:
	/**
	 * Main export function: exports JSON only if console variable is enabled.
	 * @param Asset The asset to export
	 * @param UAssetPath Path to the .uasset file, used to generate the JSON path
	 * @return True if export succeeded or was disabled; false only if enabled but failed to export
	 */
	static bool ExportIfEnabled(const UPICOSpatialNiagaraAsset* Asset, const FString& UAssetPath);

	/**
	 * Explicit export, regardless of console variable.
	 * @param Asset The asset to export
	 * @param JsonPath Where to save the JSON file
	 * @return True if export succeeded
	 */
	static bool ExportToFile(const UPICOSpatialNiagaraAsset* Asset, const FString& JsonPath);

private:
	// Returns true if the console variable enables export
	static bool IsExportEnabled();

	// Builds the JSON object from the asset
	static TSharedRef<class FJsonObject> BuildJsonObject(const UPICOSpatialNiagaraAsset* Asset);
};
