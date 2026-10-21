#pragma once

#include "Misc/Paths.h"

inline FString GetPICONiagaraContentAssetsRootPath()
{
	// Niagara-derived .uasset content must always live under Content/AssetsPICO
	// so runtime can keep loading /Game/AssetsPICO regardless of APK-assets mode.
	return FPaths::ProjectContentDir() / TEXT("AssetsPICO");
}
