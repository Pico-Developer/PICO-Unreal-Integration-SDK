// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "MRMeshComponent.h"
#include "PXR_ProceduralMeshComponent.generated.h"

UCLASS(ClassGroup = "PICO")
class UPICOProceduralMeshComponent : public UMRMeshComponent
{
	GENERATED_BODY()
	
public:
	void SetUpdateTime(uint64 Time);
	int64 GetUpdateTime() const;
	
private:
	uint64 LastUpdateTime = 0;
};