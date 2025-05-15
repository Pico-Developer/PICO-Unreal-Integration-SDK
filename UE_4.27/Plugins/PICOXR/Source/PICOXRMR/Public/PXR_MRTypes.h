// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "PXR_PluginWrapper.h"
#include "PXR_Log.h"
#include "PXR_HMDPrivate.h"

#include "PXR_MRTypes.generated.h"

DEFINE_LOG_CATEGORY_STATIC(PxrMR, Log, All);

#define PXR_SUCCESS(Result) ((Result) >= 0)
#define PXR_FAILURE(Result) ((Result) < 0)

UENUM(BlueprintType)
namespace EPICOResult
{
	enum Type
	{
		PXR_Success									= 0,
		PXR_TimeoutExpired							= 1,

		PXR_Error_ValidationFailure					= -1,
		PXR_Error_RuntimeFailure					= -2,
		PXR_Error_OutOfMemory						= -3,
		PXR_Error_APIVersionUnsupported				= -4,
		PXR_Error_InitializationFailed				= -6,
		PXR_Error_FunctionUnsupported				= -7,
		PXR_Error_FeatureUnsupported				= -8,
		PXR_Error_LimitReached						= -10,
		PXR_Error_SizeInsufficient					= -11,
		PXR_Error_HandleInvalid						= -12,

		PXR_Error_SpatialLocalizationRunning		= -1000,
		PXR_Error_SpatialLocalizationNotRunning		= -1001,
		PXR_Error_SpatialMapCreated					= -1002,
		PXR_Error_SpatialMapNotCreated				= -1003,
		PXR_Error_ComponentNotSupported				= -501,
		PXR_Error_ComponentConflict					= -502,
		PXR_Error_ComponentNotAdded					= -503,
		PXR_Error_ComponentAdded					= -504,
		PXR_Error_AnchorEntityNotFound              = -505
	};
}

UENUM(BlueprintType)
enum class EPICOSpatialTrackingState : uint8
{
	SpatialTrackingState_Invalid	= 0,
	SpatialTrackingState_Valid		= 1,
	SpatialTrackingState_Limited	= 2
};

UENUM(BlueprintType)
enum class EPICOSpatialTrackingStateMessage : uint8
{
	SpatialTrackingStateMessage_Unknown			= 0,
	SpatialTrackingStateMessage_InternalError	= 1,
};

UENUM(BlueprintType)
namespace EPICOAnchorComponentTypeFlag
{
	enum Type
	{
		ComponentFlag_None			= 0,
		ComponentFlag_Pose          = 0x00000001,
		ComponentFlag_Persistence   = 0x00000002,
		ComponentFlag_SceneLabel    = 0x00000004,
		ComponentFlag_Plane		    = 0x00000008,
		ComponentFlag_Volume		= 0x00000010,
	};
}

UENUM(BlueprintType)
namespace EPICOSpatialSceneDataTypeFlags
{
	enum Type
	{
		SpatialSceneDataTypeFlag_None		= 0,
		SpatialSceneDataTypeFlag_Unknown	= 0x00000001,
		SpatialSceneDataTypeFlag_Floor		= 0x00000002,
		SpatialSceneDataTypeFlag_Ceiling	= 0x00000004,
		SpatialSceneDataTypeFlag_Wall		= 0x00000008,
		SpatialSceneDataTypeFlag_Door		= 0x00000010,
		SpatialSceneDataTypeFlag_Window		= 0x00000020,
		SpatialSceneDataTypeFlag_Opening	= 0x00000040,
		SpatialSceneDataTypeFlag_Object		= 0x00000080,
	};
}

UENUM(BlueprintType)
enum class EPICOAnchorSceneLabel : uint8
{
	SceneLabel_Unknown		= 0,
	SceneLabel_Floor		= 1,
	SceneLabel_Ceiling		= 2,
	SceneLabel_Wall			= 3,
	SceneLabel_Door			= 4,
	SceneLabel_Window		= 5,
	SceneLabel_Opening		= 6,
	SceneLabel_Table		= 7,
	SceneLabel_Sofa			= 8,
};

UENUM(BlueprintType)
enum class EPICOPersistLocation : uint8
{
	PersistLocation_None		= 0  UMETA(Hidden),
	PersistLocation_Local		= 1,
	PersistLocation_Remote		= 2,
};

UENUM(BlueprintType)
enum class EPICOLoadFilterType : uint8
{
	LoadFilterType_None				= 0,
	LoadFilterType_UUID				= 1,
	LoadFilterType_ComponentType	= 2,
	LoadFilterType_SpatialSceneData = 3
};

UENUM(BlueprintType)
enum class EPICOSpatialSceneCaptureStatus : uint8
{
	SpatialSceneCaptureStatus_NotDefined	= 0
};

USTRUCT(BlueprintType)
struct PICOXRMR_API FPICOAnchor
{
	GENERATED_BODY()

	uint64_t Value;

	operator uint64_t() const { return Value; }

	FPICOAnchor() { this->Value = 0; }
	FPICOAnchor(const uint64_t& NewValue) { this->Value = NewValue; }
	FPICOAnchor(const FPICOAnchor& Anchor) { this->Value = Anchor.GetValue(); }

	bool operator==(const FPICOAnchor& Other) const
	{
		return Other.GetValue() == Value;
	}

	bool IsValid() const { return Value > 0; }

	uint64_t GetValue() const { return Value; }

	FString ToString() const
	{
		return LexToString((uint64)Value);
	}
};

FORCEINLINE uint32 GetTypeHash(const FPICOAnchor& Anchor)
{
	uint64 Origin = (uint64)Anchor.GetValue();
	uint32 A = Origin & 0xffffffff;
	uint32 B = (Origin >> 32) & 0xffffffff;
	uint32 Hash = HashCombine(A, B);
	return Hash;
}

template<>
struct TStructOpsTypeTraits<FPICOAnchor> : public TStructOpsTypeTraitsBase2<FPICOAnchor>
{
	enum
	{
		WithIdenticalViaEquality = true,
	};
};

USTRUCT(BlueprintType)
struct PICOXRMR_API FPICOAnchorUUID
{
	GENERATED_BODY()

	FPICOAnchorUUID()
	{
		FMemory::Memzero(UUIDArray, PXR_UUID_SIZE);
	}

	FPICOAnchorUUID(const uint64_t(&NewUUIDArray)[PXR_UUID_SIZE])
	{
		FMemory::Memcpy(UUIDArray, NewUUIDArray);
	}

	bool operator==(const FPICOAnchorUUID& UUID) const
	{
		return IsEqual(UUID);
	}

	bool operator!=(const FPICOAnchorUUID& UUID) const
	{
		return !IsEqual(UUID);
	}

	bool IsValid() const
	{
		static uint64_t ZeroUUID[PXR_UUID_SIZE] = { 0 };
		return FMemory::Memcmp(UUIDArray, ZeroUUID, PXR_UUID_SIZE) != 0;
	}

	bool IsEqual(const FPICOAnchorUUID& UUID) const
	{
		return FMemory::Memcmp(UUIDArray, UUID.UUIDArray, PXR_UUID_SIZE) == 0;
	}

	FString ToString() const
	{
		FString Result = FormatToHexString(UUIDArray[0]);
		for (int32 Index = 1; Index < PXR_UUID_SIZE; ++Index)
		{
			Result += TEXT("-");
			Result += FormatToHexString((uint64)UUIDArray[Index]);
		}
		return Result;
	}

	friend FArchive& operator<<(FArchive& Ar, FPICOAnchorUUID& UUID)
	{
		for (int32 Index = 0; Index < PXR_UUID_SIZE; ++Index)
		{
			Ar.Serialize(&UUID.UUIDArray[Index], sizeof(uint64_t));
		}
		return Ar;
	}

	bool Serialize(FArchive& Ar)
	{
		Ar << *this;
		return true;
	}

	uint64_t UUIDArray[PXR_UUID_SIZE];
};

FORCEINLINE uint32 GetTypeHash(const FPICOAnchorUUID& AnchorUUID)
{
	uint32 Hash = 0;
	for (int32 Index = 0; Index < PXR_UUID_SIZE; ++Index)
	{
		uint64 Origin = (uint64)AnchorUUID.UUIDArray[Index];
		uint32 A = Origin & 0xffffffff;
		uint32 B = (Origin >> 32) & 0xffffffff;
		Hash = HashCombine(Hash, A);
		Hash = HashCombine(Hash, B);
	}
	return Hash;
}

template<>
struct TStructOpsTypeTraits<FPICOAnchorUUID> : public TStructOpsTypeTraitsBase2<FPICOAnchorUUID>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithSerializer = true,
	};
};

USTRUCT(BlueprintType)
struct PICOXRMR_API FPICOAnchorPlaneBoundaryInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	FVector Center;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	FVector2D Extent;
};

USTRUCT(BlueprintType)
struct PICOXRMR_API FPICOAnchorPlanePolygonInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	int32 VerticesNum;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	TArray<FVector> Vertices;
};

USTRUCT(BlueprintType)
struct PICOXRMR_API FPICOAnchorVolumeInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	FVector Center;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	FVector Extent;
};

USTRUCT(BlueprintType)
struct PICOXRMR_API FPICOAnchorLoadInfo
{
	GENERATED_BODY()

	FPICOAnchorLoadInfo() :
		LoadMaxCount(1024),
		Timeout(0),
		PersistLocation(EPICOPersistLocation::PersistLocation_Local),
		IncludeFilterType(EPICOLoadFilterType::LoadFilterType_None),
		ExcludeFilterType(EPICOLoadFilterType::LoadFilterType_None)
	{}

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	int LoadMaxCount;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	float Timeout;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	EPICOPersistLocation PersistLocation;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	EPICOLoadFilterType IncludeFilterType;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	TArray<FPICOAnchorUUID> IncludeUUIDFilter;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	TArray<TEnumAsByte<EPICOAnchorComponentTypeFlag::Type>> IncludeComponentTypeFilter;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	TArray< TEnumAsByte<EPICOSpatialSceneDataTypeFlags::Type>> IncludeSpatialSceneDataTypeFilter;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	EPICOLoadFilterType ExcludeFilterType;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	TArray<FPICOAnchorUUID> ExcludeUUIDFilter;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	TArray< TEnumAsByte<EPICOAnchorComponentTypeFlag::Type>> ExcludeComponentTypeFilter;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	TArray< TEnumAsByte<EPICOSpatialSceneDataTypeFlags::Type>> ExcludeSpatialSceneDataTypeFilter;

	FString ToString() const
	{
		FString Result = FString::Printf(TEXT("MaxCount[%d], Timeout[%f], Location[%d], IncludeFilterType[%d], IncludeUUIDFilterNum[%d], IncludeComponentTypeFilterNum[%d], IncludeSpatialSceneDataTypeFilterNum[%d], ExcludeFilterType[%d], ExcludeUUIDFilterNum[%d], ExcludeComponentTypeFilterNum[%d], ExcludeSpatialSceneDataTypeFilterNum[%d]"),
			LoadMaxCount, Timeout, (int32)PersistLocation,
			(int32)IncludeFilterType, IncludeUUIDFilter.Num(), IncludeComponentTypeFilter.Num(), IncludeSpatialSceneDataTypeFilter.Num(),
			(int32)ExcludeFilterType, ExcludeUUIDFilter.Num(), ExcludeComponentTypeFilter.Num(), ExcludeSpatialSceneDataTypeFilter.Num());

		return Result;
	}
};

USTRUCT(BlueprintType)
struct PICOXRMR_API FAnchorLoadResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	FPICOAnchor AnchorHandle;
	
	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	FPICOAnchorUUID AnchorUUID;

	UPROPERTY(BlueprintReadWrite, Category = "PXR|MR")
	EPICOPersistLocation PersistLocation;
};