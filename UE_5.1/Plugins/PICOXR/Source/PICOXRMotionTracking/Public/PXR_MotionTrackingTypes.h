// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "PXR_Plugin_Types.h"
#include "PXR_InputFunctionLibrary.h"
#include "PXR_MotionTrackingTypes.generated.h"

UENUM(BlueprintType)
enum class EPXRTrackingStateCode : uint8
{
	PXR_MT_SUCCESS				 = 0,           // Tracking is successful.
	PXR_MT_FAILURE				 = (uint8)-1,   // Tracking failed.
	PXR_MT_MODE_NONE			 = (uint8)-2,   // Tracking mode not set.
	PXR_MT_DEVICE_NOT_SUPPORT	 = (uint8)-3,   // The PICO device does not support tracking.
	PXR_MT_SERVICE_NEED_START	 = (uint8)-4,   // Tracking service needs to be enabled in the UE editor.
	PXR_MT_ET_PERMISSION_DENIED  = (uint8)-5,   // Eye tracking permission denied.
	PXR_MT_FT_PERMISSION_DENIED  = (uint8)-6,   // Face tracking permission denied.
	PXR_MT_MIC_PERMISSION_DENIED = (uint8)-7,   // Microphone permission denied.
	PXR_MT_SYSTEM_DENIED		 = (uint8)-8,   // System permission denied.
	PXR_MT_UNKNOW_ERROR			 = (uint8)-9    // Unknown error.
};

UENUM(BlueprintType)
enum class EPXRFaceBlendShape : uint8
{
	BrowInnerUp = 3,
	BrowDown_R = 4,
	BrowDown_L = 16,
	BrowOuterUp_L = 30,
	BrowOuterUp_R = 36,
	CheekSquint_L = 29,
	CheekSquint_R = 37,
	CheekPuff = 43,
	EyeLookDown_L = 0,
	EyeLookIn_L = 2,
	EyeLookIn_R = 11,
	EyeLookDown_R = 12,
	EyeSquint_L = 26,
	EyeBlink_L = 28,
	EyeLookUp_L = 31,
	EyeLookUp_R = 35,
	EyeBlink_R = 38,
	EyeSquint_R = 41,
	EyeLookOut_L = 44,
	EyeLookOut_R = 45,
	EyeWide_R = 46,
	EyeWide_L = 47,
	JawOpen = 7,
	JawRight = 15,
	JawForward = 25,
	JawLeft = 32,
	MouthClose = 5,
	MouthLowerDown_R = 6,
	MouthUpperUp_R = 8,
	MouthShrugUpper = 9,
	MouthFunnel = 10,
	MouthRollUpper = 14,
	MouthShrugLower = 17,
	MouthRollLower = 18,
	MouthSmile_L = 19,
	MouthPress_L = 20,
	MouthSmile_R = 21,
	MouthPress_R = 22,
	MouthDimple_R = 23,
	MouthLeft = 24,
	MouthFrown_L = 27,
	MouthStretch_L = 33,
	MouthPucker = 34,
	MouthUpperUp_L = 39,
	MouthFrown_R = 40,
	MouthStretch_R = 42,
	MouthRight = 48,
	MouthDimple_L = 49,
	MouthLowerDown_L = 50,
	NoseSneer_L = 1,
	NoseSneer_R = 13,
	TongueOut = 51,
	PP = 52,
	CH = 53,
	o = 54,
	O = 55,
	I = 56,
	u = 57,
	RR = 58,
	XX = 59,
	aa = 60,
	i = 61,
	FF = 62,
	U = 63,
	TH = 64,
	kk = 65,
	SS = 66,
	e = 67,
	DD = 68,
	E = 69,
	nn = 70,
	sil = 71,
	COUNT = 72
};

UENUM(BlueprintType)
enum class EPXRFaceTrackingMode : uint8
{
	PXR_FTM_NONE		  = (uint8)-1,
	PXR_FTM_FACE		  = 0,
	PXR_FTM_LIPS		  = 1,
	PXR_FTM_FACE_LIPS_VIS = 2,
	PXR_FTM_FACE_LIPS_BS  = 3
};

USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXRFaceTrackingState
{
	GENERATED_BODY()
public:
	FPXRFaceTrackingState();

	UPROPERTY(BlueprintReadOnly, Category = "PXR|FaceTracking")
		EPXRFaceTrackingMode CurrentTrackingMode;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|FaceTracking")
		EPXRTrackingStateCode TrackingStateCode;
};

USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXRFaceTrackingStartInfo
{
	GENERATED_BODY()
public:
	FPXRFaceTrackingStartInfo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FaceTracking")
		EPXRFaceTrackingMode StartMode;
};

USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXRFaceTrackingStopInfo
{
	GENERATED_BODY()
public:
	FPXRFaceTrackingStopInfo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FaceTracking")
		bool Pause;
};

USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXRFaceTrackingDataGetInfo
{
	GENERATED_BODY()
public:
	FPXRFaceTrackingDataGetInfo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FaceTracking")
		int64 DisplayTime;
};

USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXRFaceTrackingData
{
	GENERATED_BODY()
public:
	FPXRFaceTrackingData();

	UPROPERTY(BlueprintReadOnly, Category = "PXR|FaceTracking")
		TArray<float> BlendShapeWeights;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|FaceTracking")
		int64 TimeStamp;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|FaceTracking")
		float LaughingProb;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|FaceTracking")
		bool EyeValid;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|FaceTracking")
		bool FaceValid;
};

UENUM(BlueprintType)
enum class EPICOEye : uint8
{
	Left   = 0,
	Right  = 1,
	Center = 2,
	COUNT  = 3,
};

UENUM(BlueprintType)
enum class EPXREyeTrackingMode : uint8
{
	PXR_ETM_BOTH = 0,
	PXR_ETM_NONE = (uint8)-1,
};

USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXREyeTrackingState
{
	GENERATED_BODY()
public:
	FPXREyeTrackingState();

	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
		EPXREyeTrackingMode CurrentTrackingMode;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
		EPXRTrackingStateCode TrackingStateCode;
};

USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXREyeTrackingStartInfo
{
	GENERATED_BODY()
public:
	FPXREyeTrackingStartInfo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|EyeTracking")
		bool NeedCalibration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|EyeTracking")
		EPXREyeTrackingMode StartMode;
};

USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXREyeTrackingStopInfo
{
	GENERATED_BODY()
public:
	FPXREyeTrackingStopInfo() {}
};

USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXREyeTrackingDataGetInfo
{
	GENERATED_BODY()
public:
	FPXREyeTrackingDataGetInfo();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|EyeTracking")
		int64 DisplayTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|EyeTracking")
		bool QueryPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|EyeTracking")
		bool QueryOrientation;
};



USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXRPerEyeData
{
	GENERATED_BODY()
public:
	FPXRPerEyeData();

	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
		EPICOEye EyeType;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
		FVector Position;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
		FRotator Orientation;
	
	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
		bool bIsPoseValid;
	
	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
		float Openness;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
		bool bIsOpennessValid;
};

USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXREyeTrackingData
{
	GENERATED_BODY()
public:
	FPXREyeTrackingData();

	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
		TArray<FPXRPerEyeData> PerEyeDatas;
};

USTRUCT(BlueprintType)
struct PICOXRMOTIONTRACKING_API FPXREyePupilInfo
{
	GENERATED_BODY()
public:
	FPXREyePupilInfo():
		LeftEyePupilDiameter(0.0f),
		RightEyePupilDiameter(0.0f),
		LeftEyePupilPosition(FVector2D::ZeroVector),
		RightEyePupilPosition(FVector2D::ZeroVector)
	{}

	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
	float LeftEyePupilDiameter;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
	float RightEyePupilDiameter;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
	FVector2D LeftEyePupilPosition;

	UPROPERTY(BlueprintReadOnly, Category = "PXR|EyeTracking")
	FVector2D RightEyePupilPosition;
};
