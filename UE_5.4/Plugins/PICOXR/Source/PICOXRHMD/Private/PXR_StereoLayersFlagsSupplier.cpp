// Fill out your copyright notice in the Description page of Project Settings.
#include "PXR_StereoLayersFlagsSupplier.h"
#include "IStereoLayers.h"
#include "PXR_Plugin_Types.h"

TSharedPtr<FPICOXRStereoLayersFlagsSupplier> FPICOXRStereoLayersFlagsSupplier::Instance = NULL;

TSharedPtr<FPICOXRStereoLayersFlagsSupplier> FPICOXRStereoLayersFlagsSupplier::Get()
{
	if (!Instance.IsValid())
	{
		Instance = MakeShared<FPICOXRStereoLayersFlagsSupplier>();
	}

	return Instance;
}

int FPICOXRStereoLayersFlagsSupplier::GetPxrLayerFlags(uint32 DescFlags)
{
	int LayerFlags = 0;

	for (auto& FlagPair : UnrealFlagToPxrFlagMap)
	{
		if (DescFlags & FlagPair.Key)
		{
			LayerFlags |= FlagPair.Value;
		}
	}

	return LayerFlags;
}

FPICOXRStereoLayersFlagsSupplier::FPICOXRStereoLayersFlagsSupplier()
{
	uint32 Value = IStereoLayers::ELayerFlags::LAYER_FLAG_MAX_VALUE << 1;
	UnrealFlagToPxrFlagMap.Add(Value, PXR_LAYER_FLAG_ENABLE_SUPER_RESOLUTION);
	SupportedFilters.Add(FName("Super Resolution"));

	Value = Value << 1;
	UnrealFlagToPxrFlagMap.Add(Value, PXR_LAYER_FLAG_ENABLE_NORMAL_SUPER_SAMPLING);
	SupportedFilters.Add(FName("Normal SuperSampling"));

	Value = Value << 1;
	UnrealFlagToPxrFlagMap.Add(Value, PXR_LAYER_FLAG_ENABLE_QUALITY_SUPER_SAMPLING);
	SupportedFilters.Add(FName("Quality SuperSampling"));

	Value = Value << 1;
	UnrealFlagToPxrFlagMap.Add(Value, PXR_LAYER_FLAG_ENABLE_FIXED_FOVEATED_SUPER_SAMPLING);
	SupportedFilters.Add(FName("Fixed Foveated SuperSampling"));

	Value = Value << 1;
	UnrealFlagToPxrFlagMap.Add(Value, PXR_LAYER_FLAG_ENABLE_NORMAL_SHARPENING);
	SupportedFilters.Add(FName("Normal Sharpening"));

	Value = Value << 1;
	UnrealFlagToPxrFlagMap.Add(Value, PXR_LAYER_FLAG_ENABLE_QUALITY_SHARPENING);
	SupportedFilters.Add(FName("Quality Sharpening"));

	Value = Value << 1;
	UnrealFlagToPxrFlagMap.Add(Value, PXR_LAYER_FLAG_ENABLE_FIXED_FOVEATED_SHARPENING);
	SupportedFilters.Add(FName("Fixed Foveated Sharpening"));
	
	Value = Value << 1;
	UnrealFlagToPxrFlagMap.Add(Value, PXR_LAYER_FLAG_ENABLE_SELF_ADAPTIVE_SHARPENING);
	SupportedFilters.Add(FName("Self Adaptive Sharpening"));

}
