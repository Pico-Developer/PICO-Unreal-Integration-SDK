// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "PICO_CameraTypes.generated.h"

/** Camera facing direction */
UENUM(BlueprintType)
enum class EPICOCameraFacing : uint8
{
	None = 0 UMETA(Hidden),
	World = 1 UMETA(DisplayName = "World-Facing"),
};

/** Camera position on the device */
UENUM(BlueprintType)
enum class EPICOCameraPosition : uint8
{
	None = 0 UMETA(Hidden),
	Unspecified = 1 UMETA(DisplayName = "Unspecified"),
	Left = 2 UMETA(DisplayName = "Left"),
	Right = 3 UMETA(DisplayName = "Right"),
};

/** Camera type */
UENUM(BlueprintType)
enum class EPICOCameraType : uint8
{
	None = 0 UMETA(Hidden),
	PassthroughColor = 1 UMETA(DisplayName = "Passthrough Color"),
};

/** Camera capability type */
UENUM(BlueprintType)
enum class EPICOCameraCapabilityType : uint8
{
	None = 0 UMETA(Hidden),
	ImageResolution = 1 UMETA(DisplayName = "Image Resolution"),
	ImageFormat = 2 UMETA(DisplayName = "Image Format"),
	DataTransferType = 3 UMETA(DisplayName = "Data Transfer Type"),
	CameraModel = 4 UMETA(DisplayName = "Camera Model"),
	ImageFPS = 5 UMETA(DisplayName = "Image FPS"),
};

/** Data transfer type */
UENUM(BlueprintType)
enum class EPICOCameraDataTransferType : uint8
{
	None = 0 UMETA(Hidden),
	RawBuffer = 1 UMETA(DisplayName = "Raw CPU Buffer"),
};

/** Image format */
UENUM(BlueprintType)
enum class EPICOCameraImageFormat : uint8
{
	None = 0 UMETA(Hidden),
	RGBA_8888 = 1 UMETA(DisplayName = "RGBA8888"),
};

/** Camera model for distortion correction */
UENUM(BlueprintType)
enum class EPICOCameraModel : uint8
{
	None = 0 UMETA(Hidden),
	Pinhole = 1 UMETA(DisplayName = "Pinhole"),
};

/** Image frame rate */
UENUM(BlueprintType)
enum class EPICOCameraImageFPS : uint8
{
	None = 0 UMETA(Hidden),
	FPS_30 = 1 UMETA(DisplayName = "30 FPS"),
	FPS_60 = 2 UMETA(DisplayName = "60 FPS"),
};

/** Camera property type */
UENUM(BlueprintType)
enum class EPICOCameraPropertyType : uint8
{
	None = 0 UMETA(Hidden),
	Facing = 1 UMETA(DisplayName = "Facing Direction"),
	Position = 2 UMETA(DisplayName = "Position"),
	CameraType = 3 UMETA(DisplayName = "Camera Type"),
};

/** Camera intrinsics data */
USTRUCT(BlueprintType)
struct FPICOCameraIntrinsics
{
	GENERATED_BODY()

	/** Focal length in pixels (fx, fy) */
	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	FVector2D FocalLength;

	/** Principal point in pixels (cx, cy) */
	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	FVector2D PrincipalPoint;

	/** Field of view in degrees */
	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	FVector2D FOV;

	FPICOCameraIntrinsics()
		: FocalLength(FVector2D::ZeroVector)
		, PrincipalPoint(FVector2D::ZeroVector)
		, FOV(FVector2D::ZeroVector)
	{
	}

	// Regular C++ method (not exposed to Blueprint)
	FString ToString() const
	{
		return FString::Printf(TEXT("FocalLength: (%f, %f), PrincipalPoint: (%f, %f), FOV: (%f, %f)"),
			FocalLength.X, FocalLength.Y,
			PrincipalPoint.X, PrincipalPoint.Y,
			FOV.X, FOV.Y);
	}
};

/** Camera extrinsics data */
USTRUCT(BlueprintType)
struct FPICOCameraExtrinsics
{
	GENERATED_BODY()

	/** Camera position (in cm) and orientation relative to HMD */
	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	FTransform Pose;

	FPICOCameraExtrinsics()
		: Pose(FTransform::Identity)
	{
	}

	// Regular C++ method (not exposed to Blueprint)
	FString ToString() const
	{
		return FString::Printf(TEXT("Position: (%s), Rotation: (%s)"),
			*Pose.GetLocation().ToString(),
			*Pose.GetRotation().Rotator().ToString());
	}
};

/** Camera image data (raw buffer) */
USTRUCT(BlueprintType)
struct FPICOCameraImageData
{
	GENERATED_BODY()

	/** Image width in pixels */
	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	int32 Width;

	/** Image height in pixels */
	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	int32 Height;

	/** Bytes between consecutive rows */
	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	int32 Stride;

	/** Bytes per pixel */
	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	int32 BytesPerPixel;

	/** Timestamp when image was captured */
	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	FTimespan CaptureTime;

	/** Raw pixel data */
	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	TArray<uint8> Buffer;

	FPICOCameraImageData()
		: Width(0)
		, Height(0)
		, Stride(0)
		, BytesPerPixel(0)
		, CaptureTime(FTimespan::Zero())
	{
	}
};

/** Camera configuration for capture session */
USTRUCT(BlueprintType)
struct FPICOCameraCaptureConfig
{
	GENERATED_BODY()

	/** Desired image resolution */
	UPROPERTY(BlueprintReadWrite, Category = "PICO|Camera")
	FIntPoint Resolution;

	/** Desired image format */
	UPROPERTY(BlueprintReadWrite, Category = "PICO|Camera")
	EPICOCameraImageFormat ImageFormat;

	/** Data transfer method */
	UPROPERTY(BlueprintReadWrite, Category = "PICO|Camera")
	EPICOCameraDataTransferType TransferType;

	/** Camera model */
	UPROPERTY(BlueprintReadWrite, Category = "PICO|Camera")
	EPICOCameraModel CameraModel;

	/** Frame rate */
	UPROPERTY(BlueprintReadWrite, Category = "PICO|Camera")
	EPICOCameraImageFPS FPS;

	FPICOCameraCaptureConfig()
		: Resolution(FIntPoint(1024, 768))
		, ImageFormat(EPICOCameraImageFormat::RGBA_8888)
		, TransferType(EPICOCameraDataTransferType::RawBuffer)
		, CameraModel(EPICOCameraModel::Pinhole)
		, FPS(EPICOCameraImageFPS::FPS_30)
	{
	}
};

/** Supported resolutions */
USTRUCT(BlueprintType)
struct FPICOCameraSupportedResolutions
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	TArray<FIntPoint> Resolutions;
};

/** Supported image formats */
USTRUCT(BlueprintType)
struct FPICOCameraSupportedFormats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	TArray<EPICOCameraImageFormat> Formats;
};

/** Supported FPS values */
USTRUCT(BlueprintType)
struct FPICOCameraSupportedFPS
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	TArray<EPICOCameraImageFPS> FPSValues;
};

/** Supported data transfer types */
USTRUCT(BlueprintType)
struct FPICOCameraSupportedTransferTypes
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	TArray<EPICOCameraDataTransferType> TransferTypes;
};

/** Supported camera models */
USTRUCT(BlueprintType)
struct FPICOCameraSupportedModels
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	TArray<EPICOCameraModel> Models;
};

/** Camera property filter */
USTRUCT(BlueprintType)
struct FPICOCameraPropertyFilter
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "PICO|Camera")
	bool bFilterByFacing;

	UPROPERTY(BlueprintReadWrite, Category = "PICO|Camera")
	EPICOCameraFacing Facing;

	UPROPERTY(BlueprintReadWrite, Category = "PICO|Camera")
	bool bFilterByPosition;

	UPROPERTY(BlueprintReadWrite, Category = "PICO|Camera")
	EPICOCameraPosition Position;

	UPROPERTY(BlueprintReadWrite, Category = "PICO|Camera")
	bool bFilterByCameraType;

	UPROPERTY(BlueprintReadWrite, Category = "PICO|Camera")
	EPICOCameraType CameraType;

	FPICOCameraPropertyFilter()
		: bFilterByFacing(false)
		, Facing(EPICOCameraFacing::World)
		, bFilterByPosition(false)
		, Position(EPICOCameraPosition::Unspecified)
		, bFilterByCameraType(false)
		, CameraType(EPICOCameraType::PassthroughColor)
	{
	}
};
