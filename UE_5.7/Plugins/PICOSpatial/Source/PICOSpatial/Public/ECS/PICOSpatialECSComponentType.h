#pragma once

#include "CoreMinimal.h"
#include "PICOSpatialECSComponentType.generated.h"

UENUM(BlueprintType)
enum class EPICOSpatialECSComponentType : uint8
{
	Unknown,
	Transform,
	Text,
	VideoPlayer,
	Model,
	GaussianSplatting,
};

