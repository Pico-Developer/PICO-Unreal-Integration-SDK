// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Delegates/DelegateCombinations.h"
#include "Misc/EnumRange.h"
#include "openxr/openxr.h"

#include "PICOOpenXRRuntimeSettings.h"
#include "PICO_MRTypes.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogMRPICO, Log, All);

#define PXR_SUCCESS(Result) (uint8(Result) == 0)
#define PXR_FAILURE(Result) (uint8(Result) != 0)
#define PXR_UUID_SIZE 16

UENUM(BlueprintType)
enum class EMRStatePICO:uint8
{
	Initialized = 0,
	Running = 1,
	Stopped = 2,
};

UENUM(BlueprintType)
enum class EAnchorStatePICO : uint8
{
	Invalid  = 0,
	Saved = 1,
};


UENUM(BlueprintType)
enum class EVSTDisplayStatusPICO:uint8
{
	Display_Disabled = 0 ,
	Display_Enabling,
	Display_Enabled,
	Display_Disabling
};

UENUM(BlueprintType)
enum class EResultPICO : uint8
{
	XR_Success = 0,
	XR_TimeoutExpired,

	XR_Spatial_Mesh_Data_No_Update,
	XR_Scene_Capture_Process_Successful,
	XR_Error_Spatial_Mesh_Volumes_Update_Failed,
	
	XR_Error_ValidationFailure,
	XR_Error_RuntimeFailure,
	XR_Error_OutOfMemory,
	XR_Error_APIVersionUnsupported,
	XR_Error_InitializationFailed,
	XR_Error_FunctionUnsupported,
	XR_Error_FeatureUnsupported,
	XR_Error_LimitReached,
	XR_Error_SizeInsufficient,
	XR_Error_HandleInvalid,
	XR_Error_Pose_Invalid_PICO,
	XR_Error_SpatialLocalizationRunning_PICO,
	XR_Error_SpatialLocalizationNotRunning_PICO,
	XR_Error_SpatialMapCreated_PICO,
	XR_Error_SpatialMapNotCreated_PICO,
	XR_Error_SpatialSensingServiceUnavailable_PICO,
	XR_Error_ComponentNotSupported_PICO,
	XR_Error_ComponentConflict_PICO,
	XR_Error_ComponentNotAdded_PICO,
	XR_Error_ComponentAdded_PICO,
	XR_Error_AnchorEntityNotFound_PICO,
	XR_Error_Tracking_State_Invalid_PICO,
	XR_Error_Space_Locating_PICO,
	XR_Error_Anchor_Sharing_Network_Timeout_PICO,
	XR_Error_Anchor_Sharing_Authentication_Failure_PICO,
	XR_Error_Anchor_Sharing_Network_Failure_PICO,
	XR_Error_Anchor_Sharing_Localization_Fail_PICO,
	XR_Error_Anchor_Sharing_Map_Insufficient_PICO,

	XR_Error_Permission_Insufficient_PICO,
	XR_Error_Unknown_PICO
};

UENUM(BlueprintType)
enum class ESpatialTrackingStatePICO : uint8
{
	SpatialTrackingState_Invalid	= 0,
	SpatialTrackingState_Valid		= 1,
	SpatialTrackingState_Limited	= 2
};

UENUM(BlueprintType)
enum class ESpatialTrackingStateMessagePICO : uint8
{
	SpatialTrackingStateMessage_Unknown			= 0,
	SpatialTrackingStateMessage_InternalError	= 1,
};

UENUM(BlueprintType)
enum class EAnchorComponentTypeFlagPICO : uint8
{
	ComponentFlag_None = 0,
	ComponentFlag_Pose = 1,
	ComponentFlag_Persistence = 2,
	ComponentFlag_SceneLabel = 3,
	ComponentFlag_Plane = 4,
	ComponentFlag_Volume = 5
};

UENUM(BlueprintType)
enum class ESpatialSceneDataTypeFlagsPICO : uint8
{
	SpatialSceneDataTypeFlag_None = 0,
	SpatialSceneDataTypeFlag_Unknown = 1,
	SpatialSceneDataTypeFlag_Floor = 2,
	SpatialSceneDataTypeFlag_Ceiling = 3,
	SpatialSceneDataTypeFlag_Wall = 4,
	SpatialSceneDataTypeFlag_Door = 5,
	SpatialSceneDataTypeFlag_Window = 6,
	SpatialSceneDataTypeFlag_Opening = 7,
	SpatialSceneDataTypeFlag_Object = 8,
};

UENUM(BlueprintType)
enum class EAnchorSceneLabelPICO : uint8
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
enum class EPersistLocationPICO : uint8
{
	PersistLocation_Local		= 0,
	PersistLocation_Shared		= 1,
	PersistLocation_Max		= 255 UMETA(Hidden),
};

UENUM(BlueprintType)
enum class ELoadFilterTypePICO : uint8
{
	LoadFilterType_None				= 0 UMETA(Hidden),
	LoadFilterType_UUID				= 1,
	LoadFilterType_SpatialSceneData = 2
};

UENUM(BlueprintType)
enum class ESpatialSceneCaptureStatusPICO : uint8
{
	SpatialSceneCaptureStatus_NotDefined	= 0
};

USTRUCT(BlueprintType)
struct PICOOPENXRMR_API FSpatialHandlePICO
{
	GENERATED_BODY()

	uint64_t Value;

	operator uint64_t() const { return Value; }

	FSpatialHandlePICO() { this->Value = 0; }
	FSpatialHandlePICO(const uint64_t& NewValue) { this->Value = NewValue; }
	FSpatialHandlePICO(const FSpatialHandlePICO& Anchor) { this->Value = Anchor.GetValue(); }

	bool operator==(const FSpatialHandlePICO& Other) const
	{
		return Other.GetValue() == Value;
	}

	bool IsValid() const { return Value > 0; }

	void Reset() { Value = 0; }
	
	uint64_t GetValue() const { return Value; }
	
	FString ToString() const
	{
		return LexToString(Value);
	}
};

USTRUCT(BlueprintType)
struct PICOOPENXRMR_API FFutureMessagePICO
{
	GENERATED_BODY()

	XrFutureEXT MessageHandle;
	bool IsFrameBarrier;

	uint64 Uuid;
	
	FFutureMessagePICO()
	{
		Uuid=0;
		MessageHandle=XR_NULL_HANDLE;
		IsFrameBarrier=false;
	}
};

FORCEINLINE uint32 GetTypeHash(const FSpatialHandlePICO& Anchor)
{
	uint64 Origin = (uint64)Anchor.GetValue();
	uint32 A = Origin & 0xffffffff;
	uint32 B = (Origin >> 32) & 0xffffffff;
	uint32 Hash = HashCombine(A, B);
	return Hash;
}

template<>
struct TStructOpsTypeTraits<FSpatialHandlePICO> : public TStructOpsTypeTraitsBase2<FSpatialHandlePICO>
{
	enum
	{
		WithIdenticalViaEquality = true,
	};
};

inline FString FormatToHexString(uint64 From)
{
	if (From == 0)
	{
		return TEXT("0");
	}

	FString To;

	static FString Hex = TEXT("0123456789abcdef");
	
	uint64 HexNumber = From;
	while (HexNumber)
	{
		int32 Index = HexNumber & 0x0f;
		To += Hex[Index];
		HexNumber >>= 4;
	}

	return To.Reverse();
}

USTRUCT(BlueprintType)
struct PICOOPENXRMR_API FSpatialUUIDPICO
{
	GENERATED_BODY()

	FSpatialUUIDPICO()
	{
		FMemory::Memzero(UUIDArray, PXR_UUID_SIZE);
	}

	FSpatialUUIDPICO(const uint8_t(&NewUUIDArray)[PXR_UUID_SIZE])
	{
		FMemory::Memcpy(UUIDArray, NewUUIDArray);
	}

	bool operator==(const FSpatialUUIDPICO& UUID) const
	{
		return IsEqual(UUID);
	}

	bool operator!=(const FSpatialUUIDPICO& UUID) const
	{
		return !IsEqual(UUID);
	}

	bool IsValid() const
	{
		static uint8_t ZeroUUID[PXR_UUID_SIZE] = { 0 };
		return FMemory::Memcmp(UUIDArray, ZeroUUID, PXR_UUID_SIZE*sizeof(uint8_t))!= 0;
	}

	bool IsEqual(const FSpatialUUIDPICO& UUID) const
	{
		bool result =FMemory::Memcmp(&UUIDArray, &UUID.UUIDArray, sizeof(UUIDArray)) == 0;
		
		return result;
	}

	FString ToString() const
	{
		return BytesToHex(UUIDArray, PXR_UUID_SIZE);
	}

	friend FArchive& operator<<(FArchive& Ar, FSpatialUUIDPICO& UUID)
	{
		for (int32 Index = 0; Index < PXR_UUID_SIZE; ++Index)
		{
			Ar.Serialize(&UUID.UUIDArray[Index], sizeof(uint8_t));
		}
		return Ar;
	}

	bool Serialize(FArchive& Ar)
	{
		Ar << *this;
		return true;
	}

	uint8_t UUIDArray[PXR_UUID_SIZE];
};

FORCEINLINE uint32 GetTypeHash(const FSpatialUUIDPICO& AnchorUUID)
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
struct TStructOpsTypeTraits<FSpatialUUIDPICO> : public TStructOpsTypeTraitsBase2<FSpatialUUIDPICO>
{
	enum
	{
		WithIdenticalViaEquality = true,
		WithSerializer = true,
	};
};

USTRUCT(BlueprintType)
struct PICOOPENXRMR_API FAnchorPlaneBoundaryInfoPICO
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "PICO|MR")
	FVector Center = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "PICO|MR")
	FVector2D Extent = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct PICOOPENXRMR_API FAnchorPlanePolygonInfoPICO
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "PICO|MR")
	int32 VerticesNum = 0;

	UPROPERTY(BlueprintReadWrite, Category = "PICO|MR")
	TArray<FVector> Vertices;
};

USTRUCT(BlueprintType)
struct PICOOPENXRMR_API FAnchorVolumeInfoPICO
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "PICO|MR")
	FVector Center = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "PICO|MR")
	FVector Extent = FVector::ZeroVector;
};

UENUM(BlueprintType)
enum class ESemanticLabelPICO:uint8
{
	Unknown = 0,
	Floor = 1,
	Ceiling = 2,
	Wall = 3,
	Door = 4,
	Window = 5,
	Opening = 6,
	Table = 7,
	Sofa = 8,
	Chair = 9,
	Human = 10,
	Curtain = 13,
	Cabinet = 14,
	Bed =15,
	Plant =16,
	Screen =17,
	VirtualWall = 18,
	Refrigerator = 19,
	Washing_Machine = 20,
	Air_Conditioner = 21,
	Lamp = 22,
	Wall_Art = 23,
	Stairway =24,
	Count UMETA(Hidden)
};

ENUM_RANGE_BY_VALUES(ESemanticLabelPICO, 
	ESemanticLabelPICO::Unknown, 
	ESemanticLabelPICO::Floor,
	ESemanticLabelPICO::Ceiling,
	ESemanticLabelPICO::Wall,
	ESemanticLabelPICO::Door,
	ESemanticLabelPICO::Window,
	ESemanticLabelPICO::Opening,
	ESemanticLabelPICO::Table,
	ESemanticLabelPICO::Sofa,
	ESemanticLabelPICO::Chair,
	ESemanticLabelPICO::Human,
	ESemanticLabelPICO::Curtain,
	ESemanticLabelPICO::Cabinet,
	ESemanticLabelPICO::Bed,
	ESemanticLabelPICO::Plant,
	ESemanticLabelPICO::Screen,
	ESemanticLabelPICO::VirtualWall,
	ESemanticLabelPICO::Refrigerator,
	ESemanticLabelPICO::Washing_Machine,
	ESemanticLabelPICO::Air_Conditioner,
	ESemanticLabelPICO::Lamp,
	ESemanticLabelPICO::Wall_Art,
	ESemanticLabelPICO::Stairway);

UENUM(BlueprintType)
enum class EPlaneOrientationPICO:uint8
{
	HorizontalUpward = 0  UMETA(DisplayName="HorizontalUpward"), 
	HorizontalDownward = 1 UMETA(DisplayName="HorizontalDownward"), 
	Vertical = 2 UMETA(DisplayName="Vertical"), 
	Arbitrary = 3 UMETA(DisplayName="Arbitrary"), 
	Count UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ESceneCaptureScalingModePICO: uint8
{
	// Use Default Scale of Actor.
	NoScaling,
	// Stretch each axis by the SceneInfo.
	Stretch,
};

USTRUCT(BlueprintType)
struct PICOOPENXRMR_API FSceneCaptureGeneratorActorPICO
{
	GENERATED_BODY()

	/**
	 * The class of actor to spawn.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	TSubclassOf<AActor> Actor;
	
	/**
	 * Set what scaling mode to apply to the actor.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	ESceneCaptureScalingModePICO ScalingMode = ESceneCaptureScalingModePICO::Stretch;
};


USTRUCT(BlueprintType)
struct FUVAdjustmentPICO
{
	GENERATED_BODY()

	/** Offset applied to the UV texture coordinates.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FVector2D Offset = FVector2D::ZeroVector;

	/** Scale applied to the UV texture coordinates.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FVector2D Scale = FVector2D::UnitVector;
	
	/** Angle of rotation in degrees.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	float Rotation;

	FUVAdjustmentPICO()
	{
		Offset = FVector2D::ZeroVector;
		Scale = FVector2D::UnitVector;
		Rotation = 0.0f;
	}

};

USTRUCT(BlueprintType)
struct FMRBox2DInfoPICO_Offline
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FVector2D Offset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FVector2D Extent;

	FMRBox2DInfoPICO_Offline()
	{
		Offset = FVector2D::ZeroVector;
		Extent = FVector2D::ZeroVector;
	}
};

USTRUCT(BlueprintType)
struct FMRBox3DInfoPICO_Offline
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FVector Offset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FVector Extent;

	FMRBox3DInfoPICO_Offline()
	{
		Offset = FVector::ZeroVector;
		Extent = FVector::ZeroVector;
	}
};

USTRUCT(BlueprintType)
struct FMRPolygonVerticesPICO_Offline
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	TArray<FVector2D> PolygonVertices;
};

USTRUCT(BlueprintType)
struct FMRSceneInfoPICO_Offline
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FString Guid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FVector Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FQuat Rotation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	ESemanticLabelPICO SemanticLabel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FMRBox3DInfoPICO_Offline Box3DInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FMRBox2DInfoPICO_Offline Box2DInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	TArray<FVector2D> PolygonVertices;

	FMRSceneInfoPICO_Offline()
	{
		Guid = FString();
		Position = FVector::ZeroVector;
		Rotation = FQuat::Identity;
		SemanticLabel = ESemanticLabelPICO::Unknown;
		Box3DInfo = FMRBox3DInfoPICO_Offline();
		Box2DInfo = FMRBox2DInfoPICO_Offline();
		PolygonVertices.Empty();
	}
};

USTRUCT(BlueprintType)
struct FMRSceneInfosPICO_Offline
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	TArray<FMRSceneInfoPICO_Offline> OutSceneInfos;
};


USTRUCT(BlueprintType)
struct PICOOPENXRMR_API FAnchorLoadInfoPICO
{
	GENERATED_BODY()

	FAnchorLoadInfoPICO() :
		PersistLocation(EPersistLocationPICO::PersistLocation_Local)
	{}

	UPROPERTY(BlueprintReadWrite, Category = "PICO|MR")
	EPersistLocationPICO PersistLocation;

	UPROPERTY(BlueprintReadWrite, Category = "PICO|MR")
	TArray<FSpatialUUIDPICO> UUIDFilter;


	FString ToString() const
	{
		FString Result = FString::Printf(TEXT("Location[%d], UUIDFilter[%d]"),
			(int32)PersistLocation,
			UUIDFilter.Num());

		return Result;
	}
};


USTRUCT(BlueprintType)
struct PICOOPENXRMR_API FSceneLoadInfoPICO
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "PICO|MR")
	TArray<ESemanticLabelPICO> SemanticFilter;

	FString ToString() const
	{
		FString Result = FString::Printf(TEXT("SemanticFilter[%d]"),
			SemanticFilter.Num());

		return Result;
	}
};

USTRUCT(BlueprintType)
struct PICOOPENXRMR_API FAnchorLoadResultPICO
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "PICO|MR")
	FSpatialUUIDPICO AnchorUUID;

	FSpatialHandlePICO AnchorHandle;

	UPROPERTY(BlueprintReadWrite, Category = "PICO|MR")
	EPersistLocationPICO PersistLocation;

	FAnchorLoadResultPICO()
		: AnchorUUID()
		, AnchorHandle()
		, PersistLocation(EPersistLocationPICO::PersistLocation_Local)
	{
	}
};

UENUM(BlueprintType, meta = (Categories = "PICO|MR"))
enum class ESpatialMeshConfigPICO:uint8
{
	Spatial_Mesh_Config_None = 0x00000000,
	Spatial_Mesh_Config_Semantic = 0x00000001,
	Spatial_Mesh_Config_Semantic_Align_With_Vertex = 0x00000002
};

enum class EProviderTypePICO:uint8
{
	Pico_Provider_Unknown=0,
	Pico_Provider_Anchor = 1,
	Pico_Provider_Mesh = 2,
	Pico_Provider_Scene_Capture = 3,
	Pico_Provider_Plane = 4,
	Pico_Provider_Light_Estimate = 5,
	Pico_Provider_Max
};

struct FQueriedSpatialEntityInfoPICO
{
	FSpatialHandlePICO SpatialEntityHandle;
	uint64 Time; // Spatial entity last update time
	FSpatialUUIDPICO UUID;

	FQueriedSpatialEntityInfoPICO(): SpatialEntityHandle(0), Time(0)
	{
	}
};

struct FQueriedSenseDataPICO
{
	TArray<XrSpatialEntityStatePICO> QueriedSpatialEntityInfos;
};

struct FSenseDataQueryCompletionPICO
{
	EResultPICO FutureResult;
	XrSenseDataSnapshotPICO SnapShotHandle;

	FSenseDataQueryCompletionPICO()
	{
		FutureResult = EResultPICO::XR_Error_Unknown_PICO;
		SnapShotHandle=XR_NULL_HANDLE;
	}
};

struct FSpatialAnchorCreateCompletionPICO
{
	EResultPICO FutureResult;
	FSpatialHandlePICO AnchorHandle;
	FSpatialUUIDPICO UUID;

	FSpatialAnchorCreateCompletionPICO()
	{
		FutureResult = EResultPICO::XR_Error_Unknown_PICO;
	}
};

struct FSceneCaptureStartCompletionPICO
{
	EResultPICO FutureResult;

	FSceneCaptureStartCompletionPICO()
	{
		FutureResult = EResultPICO::XR_Error_Unknown_PICO;
	}
};

struct FSpatialAnchorShareCompletionPICO
{
	EResultPICO FutureResult;

	FSpatialAnchorShareCompletionPICO()
	{
		FutureResult = EResultPICO::XR_Error_Unknown_PICO;
	}
};

struct FSpatialAnchorPersistCompletionPICO
{
	EResultPICO FutureResult;
	FSpatialHandlePICO AnchorHandle;
	FSpatialUUIDPICO UUID;

	FSpatialAnchorPersistCompletionPICO()
	{
		FutureResult = EResultPICO::XR_Error_Unknown_PICO;
	}
};

struct FSpatialAnchorUnpersistCompletionPICO
{
	EResultPICO FutureResult;
	FSpatialHandlePICO AnchorHandle;
	FSpatialUUIDPICO UUID;

	FSpatialAnchorUnpersistCompletionPICO()
	{
		FutureResult = EResultPICO::XR_Error_Unknown_PICO;
	}
};


struct FSenseDataProviderStartCompletionPICO
{
	EResultPICO FutureResult;

	FSenseDataProviderStartCompletionPICO()
	{
		FutureResult = EResultPICO::XR_Error_Unknown_PICO;
	}
};

struct FSharedSpatialAnchorDownloadCompletionPICO
{
	EResultPICO FutureResult;

	FSharedSpatialAnchorDownloadCompletionPICO()
	{
		FutureResult = EResultPICO::XR_Error_Unknown_PICO;
	}
};

struct FSenseDataProviderCreateInfoBasePICO
{
	EProviderTypePICO Type;
};

struct FSenseDataProviderCreateInfoAnchorPICO : FSenseDataProviderCreateInfoBasePICO
{
	FSenseDataProviderCreateInfoAnchorPICO()
	{
		Type = EProviderTypePICO::Pico_Provider_Anchor;
	}
};

struct FSenseDataProviderCreateInfoPlanePICO : FSenseDataProviderCreateInfoBasePICO
{
	FSenseDataProviderCreateInfoPlanePICO()
	{
		Type = EProviderTypePICO::Pico_Provider_Plane;
	}
};

struct FSenseDataProviderCreateInfoMeshPICO : FSenseDataProviderCreateInfoBasePICO
{
	ESpatialMeshLodPICO Lod;
	TArray<ESpatialMeshConfigPICO> ConfigArray;

	FSenseDataProviderCreateInfoMeshPICO(): Lod(ESpatialMeshLodPICO::Low)
	{
		Type = EProviderTypePICO::Pico_Provider_Mesh;
	}
};

struct FSenseDataProviderCreateInfoSceneCapturePICO : FSenseDataProviderCreateInfoBasePICO
{
	FSenseDataProviderCreateInfoSceneCapturePICO()
	{
		Type = EProviderTypePICO::Pico_Provider_Scene_Capture;
	}
};

UENUM(NotBlueprintType)
enum class ESpatialEntityComponentTypePICO:uint32
{
	Location = 0,
	Semantic = 1,
	Boundary_2D = 2,
	Polygon = 3,
	Boundary_3D = 4,
	Triangle_Mesh = 5,
	LightEstimation=1010028000,
};

struct FSenseDataProviderCreateInfoLightEstimationPICO : FSenseDataProviderCreateInfoBasePICO
{
	ESpatialLightEstimationResolutionPICO Resolution;

	FSenseDataProviderCreateInfoLightEstimationPICO()
	{
		Type = EProviderTypePICO::Pico_Provider_Light_Estimate;
		Resolution=ESpatialLightEstimationResolutionPICO::TextureResolution_128x128;
	}
};

UENUM(BlueprintType)
enum class ESceneTypePICO:uint8
{
	BoundingBox2D = 0,
	BoundingPolygon = 1,
	BoundingBox3D = 2,
};

struct FMRMeshInfoBlockPICO
{
	TArray<FSpatialMeshInfoPICO> MeshInfos;
};

UENUM(BlueprintType)
enum class ESpatialMeshStatePICO:uint8
{
	Added = 0,
	Stable = 1,
	Updated = 2,
	Removed = 3,
};

USTRUCT(BlueprintType)
struct FExtent3DfPICO
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	float Width;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	float Height;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	float Depth;

	FExtent3DfPICO()
	{
		Width = 0.0f;
		Height = 0.0f;
		Depth = 0.0f;
	}
};

USTRUCT(BlueprintType)
struct FExtent2DfPICO
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	float Width;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	float Height;

	FExtent2DfPICO()
	{
		Width = 0.0f;
		Height = 0.0f;
	}
};

USTRUCT(BlueprintType)
struct FBoundingBox3DPICO
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FTransform Center;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FExtent3DfPICO Extent;

	FBoundingBox3DPICO()
	{
		Center = FTransform();
		Extent = FExtent3DfPICO();
	}
};

USTRUCT(BlueprintType)
struct FBoundingBox2DPICO
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FVector Center;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FExtent2DfPICO Extent;

    FBoundingBox2DPICO()
        : Center(FVector::ZeroVector)
        , Extent()
	{
	}
};

USTRUCT(BlueprintType)
struct FSpatialMeshInfoPICO
{
	GENERATED_BODY()

	/** The UUID of the Mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FSpatialUUIDPICO UUID;

	/** The State of the Mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	ESpatialMeshStatePICO State;

	/** The Transform of the Mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FTransform MeshPose;

	/** The BoundingBox of the Mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FBoundingBox3DPICO BoundingBox;

	/** The Vertices Array of the Mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	TArray<FVector> Vertices;

	/** The Indices Array of the Mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	TArray<int32> Indices;
	
	/** The Semantics Array of the Mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	TArray<ESemanticLabelPICO> Semantics;

	/** The Update Time of the Mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	int64 UpdateTime;

	FSpatialMeshInfoPICO()
	{
		State = ESpatialMeshStatePICO::Stable;
		UpdateTime = 0;
	}
};

USTRUCT(BlueprintType)
struct FSpatialPlaneInfoPICO
{
	GENERATED_BODY()

	/** The UUID of the Plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FSpatialUUIDPICO UUID;

	/** The State of the Plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	ESpatialMeshStatePICO State;

	/** The State of the Plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	EPlaneOrientationPICO PlaneOrientation;

	/** The Transform of the Plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FTransform PlanePose;

	/** The BoundingBox of the Plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FBoundingBox2DPICO BoundingBox;

	/** The Vertices Array of the Plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	TArray<FVector> Vertices;

	/** The Indices Array of the Plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	TArray<int32> Indices;
	
	/** The Semantics Array of the Plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	ESemanticLabelPICO Semantic;

	/** The Update Time of the Plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	int64 UpdateTime;

	FSpatialPlaneInfoPICO()
	{
		PlaneOrientation = EPlaneOrientationPICO::HorizontalUpward;
		State = ESpatialMeshStatePICO::Stable;
		Semantic = ESemanticLabelPICO::Unknown;
		UpdateTime = 0;
	}
};


USTRUCT(BlueprintType)
struct FMRSceneInfoPICO
{
	GENERATED_BODY()

	/** The UUID of the Scene */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FSpatialUUIDPICO UUID;

	/** The Transform of the Scene */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	FTransform ScenePose;
	
	/** The Semantics of the Scene */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	ESemanticLabelPICO Semantic;

	/** The SceneType of the Scene */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|MR")
	ESceneTypePICO SceneType;

	FMRSceneInfoPICO()
		: UUID()
		, ScenePose(FTransform::Identity)
		, Semantic(ESemanticLabelPICO::Unknown)
		, SceneType(ESceneTypePICO::BoundingBox2D)
	{
	}
};
