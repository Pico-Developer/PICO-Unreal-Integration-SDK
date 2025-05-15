// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved.

#pragma once
#include "CoreMinimal.h"
#include "SampleBuffer.h"
#include <AudioDevice.h>

#include "Sound/SampleBufferIO.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/DataTable.h"
#include "PXR_InputFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class EPICOXRHandTrackingConfidence : uint8
{
	Low,
	High
};

UENUM(BlueprintType)
enum class EPICOXRActiveInputDevice: uint8
{
	NoneActive,
	ControllerActive,
	HandTrackingActive,
};

UENUM(BlueprintType)
enum class EPICOXRHandFinger :uint8
{
	None,
	Index,
	Middle,
	Ring,
	Pinky
};

UENUM(BlueprintType)
enum class EPICOXRHandJoint : uint8
{
	Palm,
	Wrist,
	ThumbMetacarpal,
	ThumbProximal,
	ThumbDistal,
	ThumbTip,
	IndexMetacarpal,
	IndexProximal,
	IndexIntermediate,
	IndexDistal,
	IndexTip,
	MiddleMetacarpal,
	MiddleProximal,
	MiddleIntermediate,
	MiddleDistal,
	MiddleTip,
	RingMetacarpal,
	RingProximal,
	RingIntermediate,
	RingDistal,
	RingTip,
	LittleMetacarpal,
	LittleProximal,
	LittleIntermediate,
	LittleDistal,
	LittleTip
};

constexpr int32 EHandJointCount = static_cast<int32>(EPICOXRHandJoint::LittleTip) + 1;

UENUM(BlueprintType)
enum class EPICOXRHandType : uint8
{
	None,
	HandLeft,
	HandRight,
};

UENUM(BlueprintType)
enum class EPICOXRHandedness :uint8
{
	LeftHand,
	RightHand,
};

UENUM(BlueprintType)
enum class EPICOXRControllerType :uint8
{
	LeftHand UMETA(DisplayName = "PICO LeftHand"),
	RightHand UMETA(DisplayName = "PICO RightHand"),
	G2Hand UMETA(DisplayName = "PICO G2 Hand"),
};

UENUM(BlueprintType)
enum class EPICOXRArmModelJoint : uint8
{
	Controller,
	Wrist,
	Elbow,
	Shoulder
};

UENUM(BlueprintType)
enum class EPICOXRGazeBehavior : uint8
{
	Never,
	DuringMotion,
	Always
};

UENUM(BlueprintType)
enum class EPICOXRControllerDeviceType : uint8
{
	UnKnown = 0 UMETA(DisplayName = "Unknown Controller"),
	G2 = 3 UMETA(DisplayName = "PICO G2"),
	Neo2 = 4 UMETA(DisplayName = "PICO Neo2"),
	Neo3 = 5 UMETA(DisplayName = "PICO Neo3"),
	PICO_4 = 6 UMETA(DisplayName = "PICO 4"),
	G3 = 7 UMETA(DisplayName = "PICO G3"),
};

UENUM(BlueprintType)
enum class EPICOXRVibrateController : uint8
{
	No = 0,
	Left = 1,
	Right = 2,
	LeftAndRight = 3
};

UENUM(BlueprintType)
enum class EPICOXRChannelFlip : uint8
{
	No = 0,
	Yes = 1
};

UENUM(BlueprintType)
enum class EPICOXRCacheConfig : uint8
{
	CacheAndVibrate,
	CacheNoVibrate
};

UENUM(BlueprintType)
enum class EBodyActionList: uint8
{
	PxrNoneAction,
	PxrTouchGround,
	PxrKeepStatic
};

UENUM(BlueprintType)
enum class EPxrControllerKeyMap : uint8
{
	PXR_CONTROLLER_KEY_HOME = 0,
	PXR_CONTROLLER_KEY_AX = 1,
	PXR_CONTROLLER_KEY_BY = 2,
	PXR_CONTROLLER_KEY_BACK = 3,
	PXR_CONTROLLER_KEY_TRIGGER = 4,
	PXR_CONTROLLER_KEY_VOL_UP = 5,
	PXR_CONTROLLER_KEY_VOL_DOWN = 6,
	PXR_CONTROLLER_KEY_ROCKER = 7,
	PXR_CONTROLLER_KEY_GRIP = 8,
	PXR_CONTROLLER_KEY_TOUCHPAD = 9,
	PXR_CONTROLLER_KEY_LASTONE = 127,

	PXR_CONTROLLER_TOUCH_AX = 128,
	PXR_CONTROLLER_TOUCH_BY = 129,
	PXR_CONTROLLER_TOUCH_ROCKER = 130,
	PXR_CONTROLLER_TOUCH_TRIGGER = 131,
	PXR_CONTROLLER_TOUCH_THUMB = 132,
	PXR_CONTROLLER_TOUCH_LASTONE = 255
};

UENUM(BlueprintType)
enum class EPxrBodyTrackerRole : uint8
{
	PxrPelvis = 0 UMETA(DisplayName = "Pelvis"),
	Pxr_LEFT_HIP = 1 UMETA(DisplayName = "LEFT_HIP"),
	Pxr_RIGHT_HIP = 2 UMETA(DisplayName = "RIGHT_HIP"),
	Pxr_SPINE1 = 3 UMETA(DisplayName = "SPINE1"),
	Pxr_LEFT_KNEE = 4 UMETA(DisplayName = "LEFT_KNEE"),
	Pxr_RIGHT_KNEE = 5 UMETA(DisplayName = "RIGHT_KNEE"),
	Pxr_SPINE2 = 6 UMETA(DisplayName = "SPINE2"),
	Pxr_LEFT_ANKLE = 7 UMETA(DisplayName = "LEFT_ANKLE"),
	Pxr_RIGHT_ANKLE = 8 UMETA(DisplayName = "RIGHT_ANKLE"),
	Pxr_SPINE3 = 9 UMETA(DisplayName = "SPINE3"),
	Pxr_LEFT_FOOT = 10 UMETA(DisplayName = "LEFT_FOOT"),
	Pxr_RIGHT_FOOT = 11 UMETA(DisplayName = "RIGHT_FOOT"),
	Pxr_NECK = 12 UMETA(DisplayName = "NECK"),
	Pxr_LEFT_COLLAR = 13 UMETA(DisplayName = "LEFT_COLLAR"),
	Pxr_RIGHT_COLLAR = 14 UMETA(DisplayName = "RIGHT_COLLAR"),
	Pxr_HEAD = 15 UMETA(DisplayName = "HEAD"),
	Pxr_LEFT_SHOULDER = 16 UMETA(DisplayName = "LEFT_SHOULDER"),
	Pxr_RIGHT_SHOULDER = 17 UMETA(DisplayName = "RIGHT_SHOULDER"),
	Pxr_LEFT_ELBOW = 18 UMETA(DisplayName = "LEFT_ELBOW"),
	Pxr_RIGHT_ELBOW = 19 UMETA(DisplayName = "RIGHT_ELBOW"),
	Pxr_LEFT_WRIST = 20 UMETA(DisplayName = "LEFT_WRIST"),
	Pxr_RIGHT_WRIST = 21 UMETA(DisplayName = "RIGHT_WRIST"),
	Pxr_LEFT_HAND = 22 UMETA(DisplayName = "LEFT_HAND"),
	Pxr_RIGHT_HAND = 23 UMETA(DisplayName = "RIGHT_HAND"),
	Pxr_ROLE_COUNT = 24 UMETA(DisplayName = "Pxr_ROLE_COUNT")
};

UENUM(BlueprintType)
enum class EPxrBodyActionList : uint8
{
	NoneAction = 0,
	TouchGround = 1,
	KeepStatic = 2,
	TouchGroundAndKeepStatic = 3
};

// imu data
USTRUCT(BlueprintType)
struct FPxrBodyTrackingImu
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FBodyTrackingImuData")
	int64 TimeStamp; // time stamp of imu
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FBodyTrackingImuData")
	float temperature; // temperature of imu
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FBodyTrackingImuData")
	FVector GyroData; // gyroscope data, x,y,z
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FBodyTrackingImuData")
	FVector AccData; // Accelerometer data, x,y,z
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FBodyTrackingImuData")
	FVector MagData; // magnetometer data, x,y,z

	FPxrBodyTrackingImu()
		: TimeStamp(0),
		  temperature(0.f),
		  GyroData(FVector::ZeroVector),
		  AccData(FVector::ZeroVector),
		  MagData(FVector::ZeroVector)
	{
	}
};


USTRUCT(BlueprintType)
struct FFitnessBandConnectState
{
	GENERATED_USTRUCT_BODY()
	/** The number of trackers currently connected  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FFitnessBandConnectState")
	uint8 Num;
	/** The Array of trackers ID currently connected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FFitnessBandConnectState")
	TArray<uint8> TrackerIDArray;
	FFitnessBandConnectState()
		: Num(0)
	{
	}
};

USTRUCT(BlueprintType)
struct FPxrBodyTrackingRoleData
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	int64 TimeStamp;
	/** bone name. if bone == NONE_ROLE, this bone is not calculated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite,  Category = "PXR|BodyTrackingData")
	EPxrBodyTrackerRole bone;
	/** bone local transform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	FTransform LocalPose;					
	/** velocity of x,y,z */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	FVector velo;                     
	/** acceleration of x,y,z */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	FVector acce;                     
	/** angular velocity of x,y,z */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	FVector wvelo;                    
	/** angular acceleration of x,y,z */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	FVector wacce;                    // 
	/** multiple actions can be supported at the same time by means of OR BodyActionList */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	EPxrBodyActionList  bodyAction;              

	FPxrBodyTrackingRoleData()
		:TimeStamp(0),
		bone(EPxrBodyTrackerRole::PxrPelvis),
		  LocalPose(FTransform()),
		  velo(FVector::ZeroVector),
		  acce(FVector::ZeroVector),
		  wvelo(FVector::ZeroVector),
		  wacce(FVector::ZeroVector),
		  bodyAction(EPxrBodyActionList::NoneAction)
	{
	}
};

UENUM(BlueprintType)
enum class ESwiftMode: uint8
{
	SWIFT_DEFAULT = 0 UMETA(DisplayName = "Default"),
	SWIFT_FULLBODY = 1 UMETA(DisplayName = "Full Body"),
};


USTRUCT(BlueprintType)
struct FBodyTrackingBoneLength
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	float headLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	float neckLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	float torsoLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	float hipLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	float upperLegLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	float lowerLegLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	float footLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	float shoulderLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	float upperArmLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	float lowerArmLen;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|BodyTrackingData")
	float handLen;
};

USTRUCT(BlueprintType)
struct FAudioClipData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 delaytag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 slot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 buffersize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 sampleRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 channelMask;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 bitrate;

	FAudioClipData()
	:delaytag(0),
	slot(0),
	buffersize(0),
	sampleRate(0),
	channelMask(0),
	bitrate(0)
	{
	}
};

USTRUCT(BlueprintType)
struct FPHFDataContent
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFDataContent")
	FString data;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFDataContent")
	int size =0;
	FPHFDataContent()
	{
		data="";
		size=0;
	}
};

USTRUCT(BlueprintType)
struct PICOXRINPUT_API FPHFData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFData")
	FPHFDataContent PHFDataContent;
	FPHFData()
		:PHFDataContent(FPHFDataContent())
	{
	}
};

USTRUCT(BlueprintType)
struct FPHFJsonFrameData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFDataContent")
		int64 frameseq;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFDataContent")
		int play;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFDataContent")
		int frequency;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFDataContent")
		int loop;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFDataContent")
		float gain;

	FPHFJsonFrameData()
	{
		frameseq=0;
		play=0;
		frequency=0;
		loop=0;
		gain=0;
	}
};

USTRUCT(BlueprintType)
struct PICOXRINPUT_API FPHFJsonData : public FTableRowBase
{
	GENERATED_BODY()
	;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFData")
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFData")
	FString phfVersion;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFData")
	int frameDuration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFData")
	TArray<FPHFJsonFrameData> patternData_L;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|FPHFData")
	TArray<FPHFJsonFrameData> patternData_R;

	FPHFJsonData()
	{
		Name = "FPHFJsonData";
		phfVersion = "V0.0";
		frameDuration = 20;
		patternData_L.Empty();
		patternData_R.Empty();
	}
};

UCLASS()
class PICOXRINPUT_API UPICOXRInputFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UPICOXRInputFunctionLibrary();

	/**
	* Get controller power.
	* @param ControllerType   (In) The controller type.
	* @param Power           (Out) The power of controller.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_GetControllerPower(EPICOXRControllerType ControllerType, int32& Power);

	/**
	* Get controller connect state.
	* @param ControllerType   (In) The controller type.
	* @param Status          (Out) Connect state,true:Connected,false:DisConnected.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_GetControllerConnectionState(EPICOXRControllerType ControllerType, bool& Status);

	/**
	* Get the main controller.
	* @param Handedness     (Out) Main handedness.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_GetMainControllerHandle(EPICOXRHandedness& Handedness);

	/**
	* Set the main controller.
	* @param Handedness     (In) Main handedness.
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static bool PXR_SetMainControllerHandle(EPICOXRHandedness Handedness);

	/**
	* Get controller angular velocity.
	* @param ControllerType     (In) The controller type.
	* @param AngularVelocity   (Out) Controller angular velocity.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_GetControllerAngularVelocity(EPICOXRControllerType ControllerType, FVector& AngularVelocity);

	/**
	* Get controller acceleration.
	* @param ControllerType    (In) The controller type.
	* @param Acceleration     (Out) Controller acceleration.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_GetControllerAcceleration(EPICOXRControllerType ControllerType, FVector& Acceleration);

	/**
	* Get controller linear velocity.
	* @param ControllerType     (In) The controller type.
	* @param LinearVelocity    (Out) Controller linear velocity.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_GetControllerLinearVelocity(EPICOXRControllerType ControllerType, FVector& LinearVelocity);

	/**
	* Vibration the controller.
	* @param ControllerType    (In) The controller type.
	* @param Strength          (In) Vibration strength.
	* @param Time              (In) Vibration time.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_VibrateController(EPICOXRControllerType ControllerType, float Strength, int Time);

	/**
	* Get the controller type.
	* @param ControllerType    (Out) The controller type.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static void PXR_GetControllerDeviceType(EPICOXRControllerDeviceType& ControllerType);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_ResetController(int Device);

	/**
	* Set controller arm model parameter.
	* @param GazeType          (In) The gaze type(Never/DuringMotion/Always).
	* @param ArmJoint          (In) The arm joint(Controller/Wrist/Elbow/Shoulder).
	* @param ElbowHeight       (In) The Height of the elbow(cm).Min=0.0f, Max=20.0f.
	* @param ElbowDepth        (In) The Depth of the elbow(cm).Min=0.0f, Max=20.0f.
	* @param PointerTiltAngle  (In) The Downward tilt or pitch of the laser pointer relative to the controller (degrees).Min=0.0f, Max=30.0f.
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput", meta = (GazeType = "DuringMotion", ArmJoint = "Controller", ElbowHeight = "0.0", ElbowDepth = "0.0", PointerTiltAngle = "0.0"))
	static int PXR_SetArmModelParameters(EPICOXRGazeBehavior GazeType, EPICOXRArmModelJoint ArmJoint, float ElbowHeight, float ElbowDepth, float PointerTiltAngle);

	/**
	* Get handedness.
	* @param  Handedness        (Out) Handedness.
	* @return True if get succeed.
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_GetHandedness(EPICOXRHandedness& Handedness);

	/**
	* Get controller enable homeKey.
	* @return True if home key enable.
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static bool PXR_GetControllerEnableHomekey();

	/**
	* Set controller origin offset.
	* @param Controller      (In) Controller type.
	* @param Offset          (In) Origin offset.   
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static void PXR_SetControllerOriginOffset(EPICOXRControllerType Controller, FVector Offset);

	/**
	* Get controller location and rotation with predicted time.
	* @param DeviceHand      (In) Select the left or right handle.
	* @param PredictedTime   (In) Prediction time (ms).
	* @param OutLocation    (Out) Location with predicted time.
	* @param OutRotation    (Out) Rotation with predicted time.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_GetControllerPredictedLocationAndRotation(EControllerHand DeviceHand, float PredictedTime, FVector& OutLocation, FRotator& OutRotation);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_StartControllerVCMotor(FString file, EPICOXRVibrateController slot);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_SetControllerAmp(float mode);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_StopControllerVCMotor(int clientId);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_SetControllerVibrationEvent(int deviceID, int frequency, float strength, int time);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_StartVibrateBySharem(USoundWave* InSoundWave, EPICOXRVibrateController slot, EPICOXRChannelFlip slotConfig, int& SourceId);

	//UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static void PXR_GetVibrateDelayTime(FString& data);
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_SetControllerEnableKey(bool isEnable, EPxrControllerKeyMap Key);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_SaveVibrateByCache(USoundWave* InSoundWave, EPICOXRVibrateController slot, EPICOXRChannelFlip slotConfig, EPICOXRCacheConfig enableV, int& SourceId);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_StartVibrateByCache(int SourceId);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_ClearVibrateByCache(int SourceId);
	/**
	 * Get the BoneRotation
	 *
	 * @param DeviceHand				(in) The hand to get the Bone Rotation from
	 * @param Key                       (in) The HandJoint to get the Bone Rotation from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static FQuat GetBoneRotation(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key);
	/**
	 * Get the BoneLocation
	 *
	 * @param DeviceHand				(in) The hand to get the Bone Location from
	 * @param Key                       (in) The HandJoint to get the Bone Location from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static FVector GetBoneLocation(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key);
	/**
	 * Get the BoneRadii
	 *
	 * @param DeviceHand				(in) The hand to get the Bone Radii from
	 * @param Key                       (in) The HandJoint to get the Bone Radii from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static float GetBoneRadii(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key);
	/**
	 * Check if the specific HandJoint is Bone Orientation Valid
	 *
	 * @param DeviceHand          (in) The hand to get the Orientation Valid status from
	 * @param Key              (in) The HandJoint to get the Orientation Valid status from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool IsBoneOrientationValid(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key);
	/**
	 * Check if the specific HandJoint is Bone Position Valid
	 *
	 * @param DeviceHand          (in) The hand to get the Position Valid status from
	 * @param Key              (in) The HandJoint to get the Position Valid status from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool IsBonePositionValid(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key);
	/**
	 * Check if the specific HandJoint is Bone Orientation Tracked
	 *
	 * @param DeviceHand          (in) The hand to get the Orientation Tracked status from
	 * @param Key              (in) The HandJoint to get the Orientation Tracked status from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool IsBoneOrientationTracked(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key);
	/**
	 * Check if the specific HandJoint is Bone Position Tracked
	 *
	 * @param DeviceHand          (in) The hand to get the Position Tracked status from
	 * @param Key              (in) The HandJoint to get the Position Tracked status from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool IsBonePositionTracked(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key);
	/**
	 * Get the RootPose
	 *
	 * @param DeviceHand				(in) The hand to get the RootPose from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static FTransform GetHandRootPose(const EPICOXRHandType DeviceHand);
	/**
	 * Get the pointer pose
	 *
	 * @param DeviceHand				(in) The hand to get the pointer pose from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static FTransform GetPointerPose(const EPICOXRHandType DeviceHand);
	/**
	 * Check if the hand is tracked
	 *
	 * @param DeviceHand				(in) The hand to get the hand status from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool IsHandTracked(const EPICOXRHandType DeviceHand);
	/**
	 * Check if the Aim state is valid
	 *
	 * @param DeviceHand				(in) The hand to get the Aim state from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool IsAimValid(const EPICOXRHandType DeviceHand);
	/**
	 * Check if the AimRayTouched is valid
	 *
	 * @param DeviceHand				(in) The hand to get the ray status from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool IsAimRayTouchedValid(const EPICOXRHandType DeviceHand);
	/**
	 * Check if the system gesture is in progress
	 *
	 * @param DeviceHand				(in) The hand to get the SystemGesture status from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool IsSystemGestureInProgress(const EPICOXRHandType DeviceHand);
	/**
	 * Check if the hand is dominant
	 *
	 * @param DeviceHand				(in) The hand to get the hand status from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool IsDominantHand(const EPICOXRHandType DeviceHand);
	/**
	 * Check if the menu pressed
	 *
	 * @param DeviceHand				(in) The hand to get the menu status from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool IsMenuPressed(const EPICOXRHandType DeviceHand);
	/**
	 * Get the tracking confidence of the hand
	 *
	 * @param DeviceHand				(in) The hand to get tracking confidence of
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static EPICOXRHandTrackingConfidence GetTrackingConfidence(const EPICOXRHandType DeviceHand);
	/**
	 * Get the scale of the hand
	 *
	 * @param DeviceHand				(in) The hand to get scale of
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static float GetHandScale(const EPICOXRHandType DeviceHand);
	/**
	 * Get the click strength of the hand
	 *
	 * @param DeviceHand				(in) The hand to get click strength of
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static float GetClickStrength(const EPICOXRHandType DeviceHand);
	/**
	 * Check if hand tracking is enabled currently
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool IsHandTrackingEnabled();
	/**
     * Get the user's dominant hand
     *
    */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static EPICOXRHandType GetDominantHand();
	/**
	 * Check if the specific finger is pinching
	 *
	 * @param DeviceHand          (in) The hand to get the IsPinching status from
	 * @param Finger              (in) The Finger to get the IsPinching status from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static bool GetFingerIsPinching(const EPICOXRHandType DeviceHand, const EPICOXRHandFinger Finger);
	/**
	 * Get finger pinch strength of the specific finger
	 *
	 * @param DeviceHand          (in) The hand to get the IsPinching Strength from
	 * @param Finger              (in) The Finger to get the IsPinching Strength from
	 */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static float GetFingerPinchStrength(const EPICOXRHandType DeviceHand, const EPICOXRHandFinger Finger);
	/**
     * Get the active input device
     */
	UFUNCTION(BlueprintPure, Category = "PXR|PXRHandTracking")
	static EPICOXRActiveInputDevice GetActiveInputDevice();
	static class UDataTable* PHFDataTable;
	/**
	* Start vibrate by data name at "/Plugins/PICOXR/Content/Blueprint/PHFDataTable.uasset".
	* You need put your *.phf files on "/Plugins/PICOXR/Content/Phf/" folder.
	* @param DataName		(In)  DataName on PHFDataTable.
	* @param SourceID       (Out) SourceID can be used to pause or resume vibrate.
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_StartVibrateByPHF(FName DataName, EPICOXRVibrateController slot, EPICOXRChannelFlip slotConfig, float ampValue, int& SourceID);

	/**
	* Pause vibrate.
	* @param SourceID		(In)  SourceID got by PXR_StartVibrateByPHF.
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_PauseVibrate(int SourceID);

	/**
	* Resume vibrate.
	* @param SourceID		(In)  SourceID got by PXR_StartVibrateByPHF.
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_ResumeVibrate(int SourceID);

	/**
	* Update Vibrate Params
	* @param SourceID		(In)  SourceID got by PXR_StartVibrateByPHF.
	* 
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_UpdateVibrateParams(int SourceID, EPICOXRVibrateController slot, EPICOXRChannelFlip slotConfig, float AmpValue);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_CreateHapticStream(FString PHFVersion, int FrameDurationMs, int Slot, int Reversal, float Amp, float Speed, int& SourceID);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_RemovePHFHaptic(int SourceID);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_StartPHFHaptic(int SourceID);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_StopPHFHaptic(int SourceID);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_WriteHapticStream(int SourceID, const FPHFJsonData& frames, int From, int NumFrames);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_GetCurrentFrameSequence(int SourceID, int& FrameSequence);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_SetPHFHapticSpeed(int SourceID, float Speed);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static int PXR_GetPHFHapticSpeed(int SourceID, float& Speed);

	//BodyTracking
	/**
	* SetBodyTrackingStaticCalibState
	* @param calibstate		(In)  Calibration state set to system
	* @return true to indicate that the function call was successful.
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static bool PXR_SetBodyTrackingStaticCalibState(int calibstate);

	/**
	* GetBodyTrackingImuData
	* @param deviceId		(In) Tracking device Id.
	* @param res			(Out) ImuData got by deviceId.
	* @return true to indicate that the function call was successful.
	*/
	//UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static bool PXR_GetBodyTrackingImuData(int deviceId, FPxrBodyTrackingImu& res);

	/**
	* GetBodyTrackingPose
	* @param BodyTrackingData		(Out)The Array of BodyTrackingData.
	* @return true to indicate that the function call was successful.
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static bool PXR_GetBodyTrackingPose(TArray<FPxrBodyTrackingRoleData>& BodyTrackingData);

	/**
	* GetBodyTrackingPoseByRole
	* @param RoleType		(In)  BodyTracker Role,such as ANKLE.
	* @param RoleBodyTrackingData		(Out)  The BodyTrackingData of selected role.
	* @return true to indicate that the function call was successful.
	*/
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRInput")
	static bool PXR_GetBodyTrackingPoseByRole(EPxrBodyTrackerRole RoleType,FPxrBodyTrackingRoleData& RoleBodyTrackingData);

	/**
	* SetBodyTrackingMode
	* @param mode		(In)  BodyTrackingMode.
	* @return true to indicate that the function call was successful.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_SetBodyTrackingMode(int32 Mode);

	/**
	* GetFitnessBandConnectState
	* @param State		(out)  FitnessBandConnectState.
	* @return true to indicate that the function call was successful.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_GetFitnessBandConnectState(FFitnessBandConnectState& State);

	/**
	* GetFitnessBandBattery
	* @param TrackerId		(In)  BodyTrackingDevice Id
	* @param Battery		(Out) battery power 1-5
	* @return true to indicate that the function call was successful.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_GetFitnessBandBattery(int32 TrackerId, int32& Battery);

	/**
	* GetFitnessBandCalibState
	* @param CalibrateState		(Out)  CalibrateState.
	* @return true to indicate that the function call was successful.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_GetFitnessBandCalibState(int32& CalibrateState);

	/**
	* LaunchFitnessBandCalibrationAPP
	* @return true to indicate that the function call was successful.
	*/
	UFUNCTION(BlueprintCallable, Category="PXR|PXRInput")
	static bool PXR_LaunchFitnessBandCalibrationAPP();
	/**
	* Set Swift Tracking Mode
	* @param Mode		(In)An enum representing the Swift Tracking Mode
	* @return true to indicate that the function call was successful.
	*/
	UFUNCTION(BlueprintCallable,Category="PXR|PXRInput")
	static bool PXR_SetSwiftMode(ESwiftMode Mode);
	/**
	* Set BodyTracking BoneLength
	* @param BoneLength		(In)Bone Length,such as head or neck
	* @return true to indicate that the function call was successful.
	*/
	UFUNCTION(BlueprintCallable,Category="PXR|PXRInput")
	static bool PXR_SetBodyTrackingBoneLength(const FBodyTrackingBoneLength& BoneLength);
};
