// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IStereoLayersFlagsSupplier.h"
/**
 * 
 */
class FPICOXRStereoLayersFlagsSupplier: public IStereoLayersFlagsSupplier
{
public:
	FPICOXRStereoLayersFlagsSupplier();

	virtual ~FPICOXRStereoLayersFlagsSupplier()
	{
		if (Instance.IsValid())
		{
			Instance.Reset();
		}
	};

	PICOXRHMD_API static TSharedPtr<FPICOXRStereoLayersFlagsSupplier> Get();
	
	virtual void EnumerateFlags(TSet<FName>& OutFlags) override
    	{
    		OutFlags.Append(SupportedFilters);
    	}

	PICOXRHMD_API int GetPxrLayerFlags(uint32 DescFlags);

private:

	TMap<uint32, uint32> UnrealFlagToPxrFlagMap;
	TArray<FName> SupportedFilters;

	static TSharedPtr<FPICOXRStereoLayersFlagsSupplier> Instance;

};
