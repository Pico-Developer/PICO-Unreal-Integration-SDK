// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "openxr/openxr.h"
#include "PICO_MovementFunctionLibrary.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PICOOpenXRMovement, Log, All);

UENUM(BlueprintType)
enum class EBodyJointPICO : uint8
{
	Pelvis = 0,
	LeftHip = 1,
	RightHip = 2,
	Spine1 = 3,
	LeftKnee = 4,
	RightKnee = 5,
	Spine2 = 6,
	LeftAnkle = 7,
	RightAnkle = 8,
	Spine3 = 9,
	LeftFoot = 10,
	RightFoot = 11,
	Neck = 12,
	LeftCollar = 13,
	RightCollar = 14,
	Head = 15,
	LeftShoulder = 16,
	RightShoulder = 17,
	LeftElbow = 18,
	RightElbow = 19,
	LeftWrist = 20,
	RightWrist = 21,
	LeftHand = 22,
	RightHand = 23
};

UENUM(BlueprintType)
enum class EBodyTrackingModePICO : uint8
{
	Default = 0		   UMETA(Hidden),
	LowLatency = 1,
	HighAccuracy = 2
};

USTRUCT(BlueprintType)
struct FBodyTrackingBoneLengthPICO
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
	float HeadLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
	float NeckLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
	float TorsoLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
	float HipLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
	float UpperLegLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
	float LowerLegLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
	float FootLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
	float ShoulderLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
	float UpperArmLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
	float LowerArmLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|BodyTracking")
	float HandLen;

	FBodyTrackingBoneLengthPICO()
		: HeadLen(0.0f)
		, NeckLen(0.0f)
		, TorsoLen(0.0f)
		, HipLen(0.0f)
		, UpperLegLen(0.0f)
		, LowerLegLen(0.0f)
		, FootLen(0.0f)
		, ShoulderLen(0.0f)
		, UpperArmLen(0.0f)
		, LowerArmLen(0.0f)
		, HandLen(0.0f)
	{
	}
};

USTRUCT(BlueprintType)
struct FBodyJointDataPICO
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	EBodyJointPICO Joint;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	bool bIsValid;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	FRotator Orientation;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	FVector Position;

	FBodyJointDataPICO()
		: Joint(EBodyJointPICO::Pelvis)
		, bIsValid(false)
		, Orientation(FRotator::ZeroRotator)
		, Position(FVector::ZeroVector)
	{
	}
};

USTRUCT(BlueprintType)
struct FBodyJointPostureFlagPICO
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	bool bStomp;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	bool bStatic;

	FBodyJointPostureFlagPICO()
		: bStomp(false)
		, bStatic(false)
	{
	}
};

USTRUCT(BlueprintType)
struct FBodyJointVelocityPICO
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	bool bIsLinearValid;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	FVector LinearVel;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	bool bIsAngularValid;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	FVector AngularVel;

	FBodyJointVelocityPICO()
		: bIsLinearValid(false)
		, LinearVel(FVector::ZeroVector)
		, bIsAngularValid(false)
		, AngularVel(FVector::ZeroVector)
	{
	}
};

USTRUCT(BlueprintType)
struct FBodyJointAccelerationPICO
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	bool bIsLinearValid;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	FVector LinearAcc;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	bool bIsAngularValid;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	FVector AngularAcc;

	FBodyJointAccelerationPICO()
		: bIsLinearValid(false)
		, LinearAcc(FVector::ZeroVector)
		, bIsAngularValid(false)
		, AngularAcc(FVector::ZeroVector)
	{
	}
};


USTRUCT(BlueprintType)
struct FBodyStatePICO
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	bool IsActive;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	TArray<FBodyJointDataPICO> BaseJointsData;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	TArray<FBodyJointPostureFlagPICO> PostureFlags;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	TArray<FBodyJointVelocityPICO> Velocities;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|BodyTracking")
	TArray<FBodyJointAccelerationPICO> Accelerations;

	FBodyStatePICO()
		: IsActive(false)
		, BaseJointsData()
		, PostureFlags()
		, Velocities()
		, Accelerations()
	{
	}
};

UENUM(BlueprintType)
enum class EBodyTrackingStatusPICO : uint8
{
	Invalid = 0,
	Valid = 1,
	Limited = 2
};

UENUM(BlueprintType)
enum class EBodyTrackingErrorCodePICO : uint8
{
	NoError = 0,
	TrackerNotCalibrated = 1,
	TrackerNumNotEnough = 2,
	TrackerStateNotSatisfied = 3,
	TrackerPersistentInvisibility = 4,
	TrackerDataError = 5,
	UserChange = 6,
	TrackerPoseError = 7,
};

UENUM(BlueprintType)
enum class EChargingStatePICO : uint8
{
	Uncharged = 0,
	TrickleCharging = 1,
	Charging = 2,
	ChargingCompleted = 3,
};

UENUM(BlueprintType)
enum class EFaceTrackingModePICO : uint8
{
	Default = 0,
	WithAudio = 1,
	WithAudioLipsync = 2,
	OnlyAudioWithLipsync = 3,
};

UENUM(BlueprintType)
enum class EFaceBlendShapePICO : uint8
{
	// Brow blend shapes (mapped from XrFaceExpressionBD, cleaned names)
	BrowDropLeft = 0,
	BrowDropRight = 1,
	BrowInnerUp = 2,
	BrowOuterUpLeft = 3,
	BrowOuterUpRight = 4,

	// Left eye blend shapes
	EyeBlinkLeft = 5,
	EyeLookDownLeft = 6,
	EyeLookInLeft = 7,
	EyeLookOutLeft = 8,
	EyeLookUpLeft = 9,
	EyeSquintLeft = 10,
	EyeWideLeft = 11,

	// Right eye blend shapes
	EyeBlinkRight = 12,
	EyeLookDownRight = 13,
	EyeLookInRight = 14,
	EyeLookOutRight = 15,
	EyeLookUpRight = 16,
	EyeSquintRight = 17,
	EyeWideRight = 18,

	// Nose blend shapes
	NoseSneerLeft = 19,
	NoseSneerRight = 20,

	// Cheek blend shapes
	CheekPuff = 21,
	CheekSquintLeft = 22,
	CheekSquintRight = 23,

	// Mouth blend shapes
	MouthClose = 24,
	MouthFunnel = 25,
	MouthPucker = 26,
	MouthLeft = 27,
	MouthRight = 28,
	MouthSmileLeft = 29,
	MouthSmileRight = 30,
	MouthFrownLeft = 31,
	MouthFrownRight = 32,
	MouthDimpleLeft = 33,
	MouthDimpleRight = 34,
	MouthStretchLeft = 35,
	MouthStretchRight = 36,
	MouthRollLower = 37,
	MouthRollUpper = 38,
	MouthShrugLower = 39,
	MouthShrugUpper = 40,
	MouthPressLeft = 41,
	MouthPressRight = 42,
	MouthLowerDropLeft = 43,
	MouthLowerDropRight = 44,
	MouthUpperUpLeft = 45,
	MouthUpperUpRight = 46,

	// Jaw blend shapes
	JawForward = 47,
	JawLeft = 48,
	JawRight = 49,
	JawOpen = 50,

	// Tongue
	TongueOut = 51,

	// LIPSYNC (mapped from XrLipExpressionBD, cleaned names)
	PP = 52,
	CH = 53,
	Lo = 54,
	O = 55,
	I = 56,
	Lu = 57,
	RR = 58,
	XX = 59,
	Laa = 60,
	Li = 61,
	FF = 62,
	U = 63,
	TH = 64,
	Lkk = 65,
	SS = 66,
	Le = 67,
	DD = 68,
	E = 69,
	Lnn = 70,
	Sil = 71,

	COUNT = 72,
};

USTRUCT(BlueprintType)
struct FFaceStatePICO
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "PICO|FaceTracking")
	int64 SampleTime;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|FaceTracking")
	bool IsUpperFaceDataValid;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|FaceTracking")
	bool IsLowerFaceDataValid;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|FaceTracking")
	TArray<float> FaceExpressionWeights;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|FaceTracking")
	TArray<float> LipsyncExpressionWeights;

	FFaceStatePICO()
		: SampleTime(0)
		, IsUpperFaceDataValid(false)
		, IsLowerFaceDataValid(false)
	{
		FaceExpressionWeights.SetNumZeroed(XR_FACE_EXPRESSION_COUNT_BD);
		LipsyncExpressionWeights.SetNumZeroed(XR_LIP_EXPRESSION_COUNT_BD);
	}
};

USTRUCT(BlueprintType)
struct FEyeDataPICO
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category = "PICO|EyeTracking")
	float Openness;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|EyeTracking")
	bool bOpennessValid;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|EyeTracking")
	bool bPoseValid;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|EyeTracking")
	FVector Location;

	UPROPERTY(BlueprintReadOnly, Category = "PICO|EyeTracking")
	FRotator Rotation;

	FEyeDataPICO()
		: Openness(0.0f)
		, bOpennessValid(false)
		, bPoseValid(false)
		, Location(FVector::ZeroVector)
		, Rotation(FRotator::ZeroRotator)
	{
	}
};

USTRUCT(BlueprintType)
struct FExpandDeviceDataPICO
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|ExpandDevice")
	int64 ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|ExpandDevice")
	TArray<uint8> Data;

	FExpandDeviceDataPICO()
		:ID(0)
	{
		Data.SetNum(XR_MAX_EXPAND_DEVICE_CUSTOM_DATA_SIZE_PICO);
	}
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnRequestMotionTrackerCompletePICO, const TArray<int64>&, MotionTrackerIDs, bool, Result);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnMotionTrackerConnectionStateChangedPICO, int64, MotionTrackerID, bool, IsConnected);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnDataMotionTrackerPowerKeyStateChangedPICO, int64, MotionTrackerID, bool, IsLongClick);

/**
 * 
 */
UCLASS()
class PICOOPENXRMOVEMENT_API UMovementFunctionLibraryPICO : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	* Get body tracking data
	*
	* @param outBodyState	body tracking data
	* @param QueryAcc		query acceleration data
	* @param QueryVel		query velocity data
	* @param QueryPostureFlag	query posture flag data
	*
	* @return		    true if the function call was successful, false otherwise
	*/
	UFUNCTION(BlueprintPure, Category = "PICO|BodyTracking")
	static bool TryGetBodyStatePICO(FBodyStatePICO& outBodyState, float WorldToMeters = 100.0f, bool QueryAcc = false, bool QueryVel = false, bool QueryPostureFlag = false);

	/**
	* @return	true if the body tracking is enabled, false otherwise
	*/
	UFUNCTION(BlueprintPure, Category = "PICO|BodyTracking")
	static bool IsBodyTrackingEnabledPICO();

	/**
	* @return	true if the body tracking is supported, false otherwise
	*/
	UFUNCTION(BlueprintPure, Category = "PICO|BodyTracking")
	static bool IsBodyTrackingSupportedPICO();

	/**
	* Start body tracking
	*
	* @param Mode	body tracking mode, LowLatency or HighAccuracy mode
	* @param UseBoneLength	use custom bone length
	* @param BoneLength		bone length data if UseBoneLength is true
	*
	* @return		    true if the function call was successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|BodyTracking", meta = (Mode = "HighAccuracy"))
	static bool StartBodyTrackingPICO(EBodyTrackingModePICO Mode);

	UFUNCTION(BlueprintCallable, Category = "PICO|BodyTracking")
	static bool StopBodyTrackingPICO();

	/**
	* This function will start the body tracking calibration app to let the user calibrate the body tracking system.
	* 
	* @return		    true if the function call was successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|BodyTracking")
	static bool StartBodyTrackingCalibAppPICO();

	/**
	* Get body tracking running status
	*
	* @return		    true if the function call was successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|BodyTracking")
	static bool GetBodyTrackingStatePICO(EBodyTrackingStatusPICO& Status, EBodyTrackingErrorCodePICO& Error);

	static FOnRequestMotionTrackerCompletePICO OnRequestMotionTrackerCompletePICO;
	/**
	* This function will let your motion tracker run as stand alone motion tracker device instead of BodyTracking. OnRequestMotionTrackerCompletePICO returens the motion tracker device ID.
	* 
	* @param DeviceCount	the motion tracker device count to be used as stand alone motion tracker.
	* If the tracker count is more than your tracker count connected, it will start the calibration app to request the matching count trackers.
	* @param OnRequestMotionTrackerCompletePICO	the callback function to be called when the request is completed.
	* @return				true if the function call was successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|MotionTracker")
	static bool RequestMotionTrackerDevicePICO(int DeviceCount, const FOnRequestMotionTrackerCompletePICO& OnRequestMotionTrackerCompletePICO);
	
	UFUNCTION(BlueprintCallable, Category = "PICO|MotionTracker")
	static bool GetMotionTrackerBatteryStatePICO(int64 ID, float& BatteryLevel, EChargingStatePICO& State);
	
	/**
	* Get the motion tracker data, pose is tracking space.
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|MotionTracker")
	static bool LocateMotionTrackerPICO(int64 ID, FRotator& OutRotation, FVector& OutPosition, FVector& OutLinearVelocity, FVector& OutAngularVelocity, FVector& OutLinearAcceleration, FVector& OutAngularAcceleration, bool GetVelAndAcc = false);

	static FOnMotionTrackerConnectionStateChangedPICO OnMotionTrackerConnectionStateChangedPICO;
	UFUNCTION(BlueprintCallable, Category = "PICO|MotionTracker")
	static void BindOnMotionTrackerConnectionStateChangedPICO(const FOnMotionTrackerConnectionStateChangedPICO& Delegate);

	static FOnDataMotionTrackerPowerKeyStateChangedPICO OnDataMotionTrackerPowerKeyStateChangedPICO;
	UFUNCTION(BlueprintCallable, Category = "PICO|MotionTracker")
	static void BindOnDataMotionTrackerPowerKeyStateChangedPICO(const FOnDataMotionTrackerPowerKeyStateChangedPICO& Delegate);
	
	/**
	* Get all expand device IDs to identify the expand device.
	* 
	* @return	true if the function call was successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|ExpandDevice")
	static bool EnumerateExpandDevicePICO(TArray<int64>& IDs);


	/**
	* Send vibration data to the expand device.
	* 
	* @param ID			the expand device ID
	* @param Duration	duration is the duration of the haptic effect in nanoseconds
	* @param Frequency	frequency is vibration frequency 40-500hz
	* @param Amp		amplitude is the amplitude of the vibration between 0.0 and 1.0
	* 
	* @return	true if the function call was successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|ExpandDevice")
	static bool SetExpandDeviceMotorVibratePICO(int64 ID, int64 Duration, int32 Frequency, float Amp);

	UFUNCTION(BlueprintCallable, Category = "PICO|ExpandDevice")
	static bool GetExpandDeviceBatteryStatePICO(int64 ID, float& BatteryLevel, EChargingStatePICO& ChargingState);

	UFUNCTION(BlueprintCallable, Category = "PICO|ExpandDevice")
	static bool SetExpandDeviceCustomDataCapabilityPICO(bool Enable);

	UFUNCTION(BlueprintCallable, Category = "PICO|ExpandDevice")
	static bool SetExpandDeviceCustomDataPICO(const TArray<FExpandDeviceDataPICO>& Datas);

	UFUNCTION(BlueprintCallable, Category = "PICO|ExpandDevice")
	static bool GetExpandDeviceCustomDataPICO(TArray<FExpandDeviceDataPICO>& Datas);

	UFUNCTION(BlueprintCallable, Category = "PICO|FaceTracking")
	static bool GetFaceTrackingSupportedPICO(TArray<EFaceTrackingModePICO>& SupportedModes);

	UFUNCTION(BlueprintCallable, Category = "PICO|FaceTracking")
	static bool StartFaceTrackingPICO(EFaceTrackingModePICO Mode);

	UFUNCTION(BlueprintCallable, Category = "PICO|FaceTracking")
	static bool StopFaceTrackingPICO();

	UFUNCTION(BlueprintCallable, Category = "PICO|FaceTracking")
	static bool SetFaceTrackingCurrentModePICO(EFaceTrackingModePICO Mode);

	UFUNCTION(BlueprintCallable, Category = "PICO|FaceTracking")
	static bool GetFaceTrackingCurrentModePICO(EFaceTrackingModePICO& Mode);

	UFUNCTION(BlueprintCallable, Category = "PICO|FaceTracking")
	static bool GetFaceTrackingDataPICO(int64 DisplayTime, FFaceStatePICO& outState);

	UFUNCTION(BlueprintPure, Category = "PICO|EyeTracking")
	static bool IsEyeTrackerSupportedPICO(bool& Supported);

	UFUNCTION(BlueprintCallable, Category = "PICO|EyeTracking")
	static bool IsEyeTrackingRunningPICO();

	UFUNCTION(BlueprintCallable, Category = "PICO|EyeTracking")
	static bool StartEyeTrackingPICO();

	UFUNCTION(BlueprintCallable, Category = "PICO|EyeTracking")
	static bool StopEyeTrackingPICO();

	/**
	* Get left eye and right eye tracking data, including eye openness, pupil diameter and eye 2d uv. Not included eye gaze data.
	* 
	* @return	true if the function call was successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "PICO|EyeTracking")
	static bool GetEyeTrackingDataPICO(FEyeDataPICO& LeftEye, FEyeDataPICO& RightEye, bool& bDepthValid, float& DepthConfidence, float& Depth, bool QueryGazeData, FEyeTrackerGazeData& OutGazeData);
};
