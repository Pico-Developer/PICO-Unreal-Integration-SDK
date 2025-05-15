// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved.

#include "PXR_InputFunctionLibrary.h"
#include "PXR_HMDPrivate.h"
#include "PXR_Utils.h"
#include "PXR_Input.h"
#include "PXR_Log.h"
#include "UObject/ConstructorHelpers.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"


FPICOXRInput* GetPICOXRInput()
{
    TArray<IMotionController*> MotionControllers = IModularFeatures::Get().GetModularFeatureImplementations<IMotionController>(IMotionController::GetModularFeatureName());
    for (auto MotionController : MotionControllers)
    {
        if (MotionController != nullptr && MotionController->GetMotionControllerDeviceTypeName() == FName(TEXT("PICOXRInput")))
        {
            return static_cast<FPICOXRInput*>(MotionController);
        }
    }
    return nullptr;
}

UDataTable* UPICOXRInputFunctionLibrary::PHFDataTable = nullptr;

UPICOXRInputFunctionLibrary::UPICOXRInputFunctionLibrary()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> DT(TEXT("DataTable'/PICOXR/Blueprint/PHFDataTable.PHFDataTable'"));
	PHFDataTable = DT.Object;
#if !PLATFORM_ANDROID
	if (PHFDataTable)
	{
		TArray<FString> PHFFiles;
		FString PHFFilesDir = FPaths::ProjectPluginsDir() / FString::Printf(TEXT("PICOXR/Content/Phf/"));
		IFileManager::Get().FindFiles(PHFFiles, *PHFFilesDir);

		PHFDataTable->EmptyTable();
		FString CSVString = "Name,PHFData\n";
		for (auto item : PHFFiles)
		{
			FString Content;
			FFileHelper::LoadFileToString(Content, *(FPaths::Combine(PHFFilesDir, item)));
			item.RemoveFromEnd(".phf");
			FPHFData Data;
			Data.PHFDataContent.data = Content;
			Data.PHFDataContent.size = Content.Len();
			PHFDataTable->AddRow(*item, Data);
		}
	}
#endif
}

IPXR_HandTracker* GetHandTracker()
{
    TArray<IPXR_HandTracker*> HandTrackers = IModularFeatures::Get().GetModularFeatureImplementations<IPXR_HandTracker>(IPXR_HandTracker::GetModularFeatureName());
    for (auto HandTracker : HandTrackers)
    {
        if (HandTracker != nullptr && HandTracker->GetHandTrackerDeviceTypeName() == FName(TEXT("PICOHandTracking")))
        {
            return HandTracker;
        }
    }
    return nullptr;
}

bool UPICOXRInputFunctionLibrary::PXR_GetControllerPower(EPICOXRControllerType ControllerType, int32& Power)
{
#if PLATFORM_ANDROID
    FPICOXRInput* PICOXRInputInstence = GetPICOXRInput();
    if (PICOXRInputInstence)
    {
        Power = PICOXRInputInstence->UPxr_GetControllerPower((uint8_t)ControllerType);
        return true;
    }
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::PXR_GetControllerConnectionState(EPICOXRControllerType ControllerType, bool& Status)
{
#if PLATFORM_ANDROID
	FPICOXRInput* Input =  GetPICOXRInput();
	if (Input)
	{
		if (ControllerType ==EPICOXRControllerType::LeftHand || ControllerType == EPICOXRControllerType::RightHand)
		{
			Status = Input->UPxr_GetControllerConnectState(static_cast<int32>(ControllerType));
		}
		else if (ControllerType == EPICOXRControllerType::G2Hand)
		{
			Status = Input->UPxr_GetControllerConnectState(0);
		}
		else
		{
			return false;
		}
		return true;
	}  
#endif
    
#if PLATFORM_WINDOWS
	Status = true;
	return true;
#endif
	return false;
}

bool UPICOXRInputFunctionLibrary::PXR_GetMainControllerHandle( EPICOXRHandedness& Handedness)
{
    bool Ret = false;
#if PLATFORM_ANDROID
	FPICOXRInput* Input =  GetPICOXRInput();
	if (Input)
	{
		int32 MainHandle = 0;
		Ret = Input->UPxr_GetControllerMainInputHandle(MainHandle);
		Handedness = MainHandle == 0? EPICOXRHandedness::LeftHand: EPICOXRHandedness::RightHand;
		return Ret;
	}
#endif
    
    return Ret;
}

bool UPICOXRInputFunctionLibrary::PXR_SetMainControllerHandle(EPICOXRHandedness Handedness)
{
#if PLATFORM_ANDROID
	FPICOXRInput *PxrInput = GetPICOXRInput();
	if (PxrInput)
	{
		PxrInput->UPxr_SetControllerMainInputHandle(int32(Handedness));
		return true;
	}
#endif
	return false;
}

bool UPICOXRInputFunctionLibrary::PXR_GetControllerAngularVelocity(EPICOXRControllerType ControllerType,FVector& AngularVelocity)
{
#if PLATFORM_ANDROID
    PxrControllerTracking Tracking;
    uint32_t Hand;

    if (ControllerType == EPICOXRControllerType::LeftHand) 
    {
        Hand = EPICOXRControllerHandness::LeftController;
    } 
    else 
    {
        Hand = EPICOXRControllerHandness::RightController;
    }

    float HeadSensorData[7] = {0};
    FPICOXRHMDModule::GetPluginWrapper().GetControllerTrackingState(Hand, 0, HeadSensorData, &Tracking);

    float Data[3] = {0.0f};
    Data[0] = Tracking.localControllerPose.angularVelocity.x;
    Data[1] = Tracking.localControllerPose.angularVelocity.y;
    Data[2] = Tracking.localControllerPose.angularVelocity.z;

    PXR_LOGD(PxrUnrealFunctionLibrary,"Pxr_GetControllerAngularVelocityState %f,%f,%f",Data[0],Data[1],Data[2]);
    AngularVelocity = FVector(Data[0],Data[1],Data[2]);
    AngularVelocity = FVector(-AngularVelocity.Z , AngularVelocity.X , AngularVelocity.Y);
    return true;
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::PXR_GetControllerAcceleration(EPICOXRControllerType ControllerType,FVector& Acceleration)
{
#if PLATFORM_ANDROID
    PxrControllerTracking Tracking;
    uint32_t Hand;

    if (ControllerType == EPICOXRControllerType::LeftHand) {
        Hand = EPICOXRControllerHandness::LeftController;
    } else {
        Hand = EPICOXRControllerHandness::RightController;
    }
	float HeadSensorData[7] = { 0 };
    FPICOXRHMDModule::GetPluginWrapper().GetControllerTrackingState(Hand, 0, HeadSensorData, &Tracking);

    float Data[3] = {0.0f};
    Data[0] = Tracking.localControllerPose.angularAcceleration.x;
    Data[1] = Tracking.localControllerPose.angularAcceleration.y;
    Data[2] = Tracking.localControllerPose.angularAcceleration.z;

    PXR_LOGD(PxrUnrealFunctionLibrary,"Pxr_GetControllerAccelerationState %f,%f,%f",Data[0],Data[1],Data[2]);
    Acceleration = FVector(Data[0],Data[1],Data[2]);
    Acceleration = FVector(-Acceleration.Z , Acceleration.X , Acceleration.Y);
    return true;
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::PXR_GetControllerLinearVelocity(EPICOXRControllerType ControllerType,
    FVector& LinearVelocity)
{
#if PLATFORM_ANDROID
    PxrControllerTracking Tracking;
    uint32_t Hand;

    if (ControllerType == EPICOXRControllerType::LeftHand) {
        Hand = EPICOXRControllerHandness::LeftController;
    } else {
        Hand = EPICOXRControllerHandness::RightController;
    }
	float HeadSensorData[7] = { 0 };
    FPICOXRHMDModule::GetPluginWrapper().GetControllerTrackingState(Hand, 0, HeadSensorData, &Tracking);

    float Data[3] = {0.0f};
    Data[0] = Tracking.localControllerPose.linearVelocity.x;
    Data[1] = Tracking.localControllerPose.linearVelocity.y;
    Data[2] = Tracking.localControllerPose.linearVelocity.z;

    PXR_LOGD(PxrUnrealFunctionLibrary,"Pxr_GetControllerLinearVelocityState %f,%f,%f",Data[0],Data[1],Data[2]);
    LinearVelocity = FVector(Data[0],Data[1],Data[2]);
    LinearVelocity = FVector(-LinearVelocity.Z , LinearVelocity.X , LinearVelocity.Y);
    return true;
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::PXR_VibrateController(EPICOXRControllerType ControllerType,float Strength, int Time)
{
#if PLATFORM_ANDROID
    if (FPICOXRHMDModule::GetPluginWrapper().SetControllerVibration((uint32_t)ControllerType, Strength, Time) == 0)
    {
        return true;
    }
#endif
    return false;
}

void UPICOXRInputFunctionLibrary::PXR_GetControllerDeviceType(EPICOXRControllerDeviceType& OutControllerType)
{
	int32 ControllerType = 0;
#if PLATFORM_ANDROID
	PxrControllerCapability Cap;
    FPICOXRHMDModule::GetPluginWrapper().GetControllerCapabilities(PXR_CONTROLLER_LEFT, &Cap);
	ControllerType = Cap.type;
#endif
#if PLATFORM_WINDOWS
	ControllerType = 6;
#endif
	OutControllerType = static_cast<EPICOXRControllerDeviceType>(ControllerType);
}

int UPICOXRInputFunctionLibrary::PXR_ResetController(int Device)
{
	int result = -1;
#if PLATFORM_ANDROID
	result = FPICOXRHMDModule::GetPluginWrapper().ResetController(Device);
#endif
	return result;
}

int UPICOXRInputFunctionLibrary::PXR_SetArmModelParameters(EPICOXRGazeBehavior GazeType, EPICOXRArmModelJoint ArmJoint, float ElbowHeight, float ElbowDepth, float PointerTiltAngle)
{
	float pxrElbowHeight = FMath::Clamp(ElbowHeight, 0.0f, 20.0f) / 100.0f;
	float pxrElbowDepth = FMath::Clamp(ElbowDepth, 0.0f, 20.0f) / 100.0f;
	float pxrPointerTiltAngle = FMath::Clamp(PointerTiltAngle, 0.0f, 30.0f);
	int result = -1;
#if PLATFORM_ANDROID
	PxrGazeType pxrgazetype = PxrGazeType((uint8)GazeType);
	PxrArmmodelType pxrarmjoint = PxrArmmodelType((uint8)ArmJoint);
	result = FPICOXRHMDModule::GetPluginWrapper().SetArmModelParameters(pxrgazetype, pxrarmjoint, pxrElbowHeight, pxrElbowDepth, pxrPointerTiltAngle);
#endif
	return result;
}

int UPICOXRInputFunctionLibrary::PXR_GetHandedness(EPICOXRHandedness &Handedness)
{
	int result = -1;
#if PLATFORM_ANDROID
	int handness;
	result = FPICOXRHMDModule::GetPluginWrapper().GetControllerHandness(&handness);
	Handedness = handness == 0 ? EPICOXRHandedness::RightHand : EPICOXRHandedness::LeftHand;
#endif
	return result;
}

bool UPICOXRInputFunctionLibrary::PXR_GetControllerEnableHomekey()
{
#if PLATFORM_ANDROID
	FPICOXRInput* PxrInput = GetPICOXRInput();
	if (PxrInput)
	{
		return PxrInput->UPxr_GetControllerEnableHomeKey();
	}
#endif
   
    return false;
}

void UPICOXRInputFunctionLibrary::PXR_SetControllerOriginOffset(EPICOXRControllerType Controller, FVector Offset)
{
#if PLATFORM_ANDROID
	if (Controller == EPICOXRControllerType::LeftHand)
	{
		FPICOXRInput::OriginOffsetL = Offset;
	}
	else if (Controller == EPICOXRControllerType::RightHand)
	{
		FPICOXRInput::OriginOffsetR = Offset;
	}
#endif
}

bool UPICOXRInputFunctionLibrary::PXR_GetControllerPredictedLocationAndRotation(EControllerHand DeviceHand, float PredictedTime, FVector& OutLocation, FRotator& OutRotation)
{
	bool Result = false;
#if PLATFORM_ANDROID
	FVector PredictedLocation; FRotator PredictedRotation;
	PredictedLocation = FVector::ZeroVector;
	PredictedRotation = FRotator::ZeroRotator;
	FPICOXRInput* PxrInput = GetPICOXRInput();
	if (PxrInput)
	{
		Result = PxrInput->GetPredictedLocationAndRotation(DeviceHand, PredictedTime, PredictedRotation, PredictedLocation);
	}
	OutLocation = PredictedLocation;
	OutRotation = PredictedRotation;
#endif
   
    return Result;
}
int UPICOXRInputFunctionLibrary::PXR_StartControllerVCMotor(FString file, EPICOXRVibrateController slot)
{
    //0-Left And Right 1-Left 2-Right 3-Left And Right
    //1-Reversal 0-No Reversal
    int state = 0;
#if PLATFORM_ANDROID
	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
        state = FPICOXRHMDModule::GetPluginWrapper().StartControllerVCMotor(TCHAR_TO_UTF8(*file), static_cast<int>(slot));
    }
#endif
    return state;
}

int UPICOXRInputFunctionLibrary::PXR_SetControllerAmp(float mode) {
    int state = 0;
#if PLATFORM_ANDROID
   	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
   	{
        state = FPICOXRHMDModule::GetPluginWrapper().SetControllerAmp(mode);
    }
#endif
    return state;
}

int UPICOXRInputFunctionLibrary::PXR_StopControllerVCMotor(int SourceId) {
    int state = 0;
#if PLATFORM_ANDROID
    if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
        state = FPICOXRHMDModule::GetPluginWrapper().StopControllerVCMotor(SourceId);
    }
#endif
    return state;
}

int UPICOXRInputFunctionLibrary::PXR_SetControllerVibrationEvent(int deviceID, int frequency, float strength, int time) {
    int state = 0;
#if PLATFORM_ANDROID
    if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000305))
    {
        PXR_LOGD(PxrUnreal, "PXR_SetControllerVibrationEvent :%d", deviceID);
        state = FPICOXRHMDModule::GetPluginWrapper().SetControllerVibrationEvent((uint32)deviceID, frequency, strength, time);
    }
#endif
    return state;
}


Audio::FSampleBuffer tSampleBuffer;


int UPICOXRInputFunctionLibrary::PXR_StartVibrateBySharem(USoundWave* InSoundWave, EPICOXRVibrateController slot, EPICOXRChannelFlip slotConfig,int& SourceId) {
    int state = 0;
    if (InSoundWave == nullptr) {
        PXR_LOGI(PxrUnreal, "PXR_StartVibrateBySharem : InSoundWave is nullptr");
        return state;
    }
#if PLATFORM_ANDROID
        if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
        {
        int32 delaytag;
        int32 buffersize;
        int32 sampleRate;
        int32 channelMask;
        int32 bitrate = 16;
        float OutSampleDuration;
        uint8* data = nullptr;
        PxrVibrate_config parameter;
        FString TempString;
        int Value = 1024;
        if (GConfig->GetString(FPlatformProperties::GetRuntimeSettingsClassName(), TEXT("AudioCallbackBufferFrameSize"), TempString, GEngineIni))
        {
            Value = FCString::Atoi(*TempString);
            PXR_LOGI(PxrUnreal, "AudioCallbackBufferFrameSize = %d", Value);
        }

        Audio::FSoundWavePCMLoader SoundWaveLoader;
        if (InSoundWave)
        {
            TFunction<void(const USoundWave* SoundWave, const Audio::FSampleBuffer& SampleBuffer)> OnLoaded
                = [](const USoundWave* SoundWave, const Audio::FSampleBuffer& SampleBuffer)
            {
                tSampleBuffer = SampleBuffer;
            };
            SoundWaveLoader.LoadSoundWave(InSoundWave, MoveTemp(OnLoaded), true);
        }
        data = InSoundWave->RawPCMData;
        buffersize = tSampleBuffer.GetNumSamples() * tSampleBuffer.GetNumChannels();
        //OutNumFrames = tSampleBuffer.GetNumFrames();
        channelMask = tSampleBuffer.GetNumChannels();
        sampleRate = tSampleBuffer.GetSampleRate();
        OutSampleDuration = tSampleBuffer.GetSampleDuration();
        parameter.slot = static_cast<int>(slot);
        parameter.buffersize = static_cast<uint64>(buffersize);
        parameter.sampleRate = sampleRate;
        parameter.channelCounts = channelMask;
        parameter.bitrate = bitrate;
        parameter.reversal = static_cast<int>(slotConfig);
        parameter.isCache = 0;
        state = FPICOXRHMDModule::GetPluginWrapper().StartVibrateBySharemU(data, &parameter, &SourceId);
    }
#endif
    return state;
}
int UPICOXRInputFunctionLibrary::PXR_SaveVibrateByCache(USoundWave* InSoundWave, EPICOXRVibrateController slot, EPICOXRChannelFlip slotConfig, EPICOXRCacheConfig enableV, int& SourceId) {
    int state = 0;
    if (InSoundWave == nullptr) {
        PXR_LOGI(PxrUnreal, "PXR_SaveVibrateByCache : InSoundWave is nullptr");
        return state;
    }
#if PLATFORM_ANDROID
       if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
       {
        int32 delaytag;
        int32 buffersize;
        int32 sampleRate;
        int32 channelMask;
        int32 bitrate = 16;
        float OutSampleDuration;
        uint8* data = nullptr;
        PxrVibrate_config parameter;
        FString TempString;
        int Value = 1024;
        if (GConfig->GetString(FPlatformProperties::GetRuntimeSettingsClassName(), TEXT("AudioCallbackBufferFrameSize"), TempString, GEngineIni))
        {
            Value = FCString::Atoi(*TempString);
            PXR_LOGI(PxrUnreal, "AudioCallbackBufferFrameSize = %d", Value);
        }

        Audio::FSoundWavePCMLoader SoundWaveLoader;
        if (InSoundWave)
        {
            TFunction<void(const USoundWave* SoundWave, const Audio::FSampleBuffer& SampleBuffer)> OnLoaded
                = [](const USoundWave* SoundWave, const Audio::FSampleBuffer& SampleBuffer)
            {
                tSampleBuffer = SampleBuffer;
            };
            SoundWaveLoader.LoadSoundWave(InSoundWave, MoveTemp(OnLoaded), true);
        }
        data = InSoundWave->RawPCMData;
        PXR_LOGI(PxrUnreal, "AudioCallbackBufferFrameSize = %d", data[0]);
        buffersize = tSampleBuffer.GetNumSamples() * tSampleBuffer.GetNumChannels();
        //OutNumFrames = tSampleBuffer.GetNumFrames();
        channelMask = tSampleBuffer.GetNumChannels();
        sampleRate = tSampleBuffer.GetSampleRate();
        OutSampleDuration = tSampleBuffer.GetSampleDuration();
        parameter.slot = static_cast<int>(slot);
        parameter.buffersize = static_cast<uint64>(buffersize);
        parameter.sampleRate = sampleRate;
        parameter.channelCounts = channelMask;
        parameter.bitrate = bitrate;
        parameter.reversal = static_cast<int>(slotConfig);
        int isVibrate = 0;
        if (enableV == EPICOXRCacheConfig::CacheAndVibrate)
        {
            isVibrate = 1;
        }
        else if(enableV == EPICOXRCacheConfig::CacheNoVibrate)
        {
            isVibrate = 2;
        }
        parameter.isCache = isVibrate;
        state = FPICOXRHMDModule::GetPluginWrapper().StartVibrateBySharemU(data, &parameter, &SourceId);
    }
#endif
    return state;
}

int UPICOXRInputFunctionLibrary::PXR_StartVibrateByCache(int SourceId) {
    int state = 0;
#if PLATFORM_ANDROID
    if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
        return FPICOXRHMDModule::GetPluginWrapper().StartVibrateByCache(SourceId);
    }
#endif
    return 0;
}

int UPICOXRInputFunctionLibrary::PXR_ClearVibrateByCache(int SourceId) {
    int state = 0;
#if PLATFORM_ANDROID
    if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
        return FPICOXRHMDModule::GetPluginWrapper().ClearVibrateByCache(SourceId);
    }
#endif
    return 0;
}

FQuat UPICOXRInputFunctionLibrary::GetBoneRotation(EPICOXRHandType SkeletonType, EPICOXRHandJoint Key)
{
#if PLATFORM_ANDROID
    IPXR_HandTracker* HandTracker=GetHandTracker();
    if (HandTracker)
    {
        return HandTracker->GetBoneRotation(SkeletonType,Key);
    }
#endif
    return FQuat();
}

FVector UPICOXRInputFunctionLibrary::GetBoneLocation(EPICOXRHandType DeviceHand, EPICOXRHandJoint Key)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->GetBoneLocation(DeviceHand,Key);
	}
#endif
	return FVector();
}

float UPICOXRInputFunctionLibrary::GetBoneRadii(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->GetBoneRadii(DeviceHand,Key);
	}
#endif
    return 0.f;
}

bool UPICOXRInputFunctionLibrary::IsBoneOrientationValid(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->IsBoneOrientationValid(DeviceHand,Key);
	}
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::IsBonePositionValid(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->IsBonePositionValid(DeviceHand,Key);
	}
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::IsBoneOrientationTracked(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->IsBoneOrientationTracked(DeviceHand,Key);
	}
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::IsBonePositionTracked(const EPICOXRHandType DeviceHand, const EPICOXRHandJoint Key)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->IsBonePositionTracked(DeviceHand,Key);
	}
#endif
    return false;
}

FTransform UPICOXRInputFunctionLibrary::GetHandRootPose(EPICOXRHandType SkeletonType)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->GetHandRootPose(SkeletonType);
	}
#endif
    return FTransform();
}

FTransform UPICOXRInputFunctionLibrary::GetPointerPose(const EPICOXRHandType DeviceHand)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->GetPointerPose(DeviceHand);
	}
#endif
    return FTransform();
}

bool UPICOXRInputFunctionLibrary::IsHandTracked(const EPICOXRHandType DeviceHand)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->IsHandTracked(DeviceHand);
	}
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::IsAimValid(const EPICOXRHandType DeviceHand)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->IsAimValid(DeviceHand);
	}
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::IsAimRayTouchedValid(const EPICOXRHandType DeviceHand)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->IsAimRayTouchedValid(DeviceHand);
	}
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::IsSystemGestureInProgress(const EPICOXRHandType DeviceHand)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->IsSystemGestureInProgress(DeviceHand);
	}
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::IsDominantHand(const EPICOXRHandType DeviceHand)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->IsDominantHand(DeviceHand);
	}
#endif
    return false;
}

bool UPICOXRInputFunctionLibrary::IsMenuPressed(const EPICOXRHandType DeviceHand)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->IsMenuPressed(DeviceHand);
	}
#endif
    return false;
}

EPICOXRHandTrackingConfidence UPICOXRInputFunctionLibrary::GetTrackingConfidence(const EPICOXRHandType DeviceHand)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->GetTrackingConfidence(DeviceHand);
	}
#endif
    return EPICOXRHandTrackingConfidence::Low;
}

float UPICOXRInputFunctionLibrary::GetHandScale(const EPICOXRHandType DeviceHand)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->GetHandScale(DeviceHand);
	}
#endif
    return 1.f;
}

float UPICOXRInputFunctionLibrary::GetClickStrength(const EPICOXRHandType DeviceHand)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->GetClickStrength(DeviceHand);
	}
#endif
    return 0.f;
}

bool UPICOXRInputFunctionLibrary::IsHandTrackingEnabled()
{
#if PLATFORM_ANDROID
	const UPICOXRSettings* Settings = GetDefault<UPICOXRSettings>();
	IPXR_HandTracker* HandTracker=GetHandTracker();

	if (Settings&&HandTracker&&Settings->HandTrackingSupport != EPICOXRHandTrackingSupport::ControllersOnly)
	{
		return GetHandTracker()->IsHandTrackingStateValid();
	}
#endif
    return false;
}

EPICOXRHandType UPICOXRInputFunctionLibrary::GetDominantHand()
{
	EPICOXRHandType DominantHand = EPICOXRHandType::None;
#if PLATFORM_ANDROID
	if (IsDominantHand(EPICOXRHandType::HandLeft))
	{
		DominantHand = EPICOXRHandType::HandLeft;
	}
	else if (IsDominantHand(EPICOXRHandType::HandRight))
	{
		DominantHand = EPICOXRHandType::HandRight;
	}
#endif
   
    return DominantHand;
}

bool UPICOXRInputFunctionLibrary::GetFingerIsPinching(const EPICOXRHandType DeviceHand, EPICOXRHandFinger Finger)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->GetFingerIsPinching(DeviceHand,Finger);
	}
#endif
   
    return false;
}

float UPICOXRInputFunctionLibrary::GetFingerPinchStrength(const EPICOXRHandType DeviceHand, EPICOXRHandFinger Finger)
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->GetFingerPinchStrength(DeviceHand,Finger);
	}
#endif
   
    return 0.f;
}

EPICOXRActiveInputDevice UPICOXRInputFunctionLibrary::GetActiveInputDevice()
{
#if PLATFORM_ANDROID
	IPXR_HandTracker* HandTracker=GetHandTracker();
	if (HandTracker)
	{
		return HandTracker->GetActiveInputDevice();
	}
#endif
   
    return EPICOXRActiveInputDevice::NoneActive;
}


int UPICOXRInputFunctionLibrary::PXR_StartVibrateByPHF(FName DataName,EPICOXRVibrateController slot, EPICOXRChannelFlip slotConfig,float ampValue ,int& SourceID)
{
	FString ContextString = TEXT("UPICOXRInputFunctionLibrary::PXR_StartVibrateByPHF");
	SourceID = 0;
	UDataTable* Table = PHFDataTable;
	if (Table)
	{
		FPHFData* PHF_ST = Table->FindRow<FPHFData>(DataName, ContextString);
		if (PHF_ST)
		{
#if PLATFORM_ANDROID
			if (PHF_ST->PHFDataContent.size != 0)
			{
				if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
				{
					int result = 0;
					PxrVibrate_info vibrateInfo;
					vibrateInfo.slot = (uint32)(static_cast<int>(slot));
					vibrateInfo.reversal = (uint32)(static_cast<int>(slotConfig));
					vibrateInfo.amp = ampValue;
					int ID;
					result = FPICOXRHMDModule::GetPluginWrapper().StartVibrateByPHF(TCHAR_TO_UTF8(*(PHF_ST->PHFDataContent.data)), PHF_ST->PHFDataContent.size, &ID, &vibrateInfo);
					SourceID = ID;
					return result;
				}
			}
#endif
		}
	}
	return 0;
}

int UPICOXRInputFunctionLibrary::PXR_PauseVibrate(int SourceID)
{
#if PLATFORM_ANDROID
	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
	{
		return FPICOXRHMDModule::GetPluginWrapper().PauseVibrate(SourceID);
	}
#endif
	return 0;
}

int UPICOXRInputFunctionLibrary::PXR_ResumeVibrate(int SourceID)
{
#if PLATFORM_ANDROID
	int SdkVersion = 0;
	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
	{
		return FPICOXRHMDModule::GetPluginWrapper().ResumeVibrate(SourceID);
	}
#endif
	return 0;
}

void UPICOXRInputFunctionLibrary::PXR_GetVibrateDelayTime(FString &data) {
    int length = 10;
    data = nullptr;
#if PLATFORM_ANDROID
    if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000305))
    {
        data = FPICOXRHMDModule::GetPluginWrapper().GetVibrateDelayTime(&length);
    }
#endif
}
int UPICOXRInputFunctionLibrary::PXR_SetControllerEnableKey(bool isEnable, EPxrControllerKeyMap Key) {
#if PLATFORM_ANDROID
    PxrControllerKeyMap PxrKey = static_cast<PxrControllerKeyMap>(Key);
    return FPICOXRHMDModule::GetPluginWrapper().SetControllerEnableKey(isEnable, PxrKey);
#endif
    return 0;
}

int UPICOXRInputFunctionLibrary::PXR_UpdateVibrateParams(int SourceID, EPICOXRVibrateController slot, EPICOXRChannelFlip slotConfig, float AmpValue) {
#if PLATFORM_ANDROID
    if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
        PxrVibrate_info vibrateInfo;
        vibrateInfo.slot = (uint32)(static_cast<int>(slot));
        vibrateInfo.reversal = (uint32)(static_cast<int>(slotConfig));
        vibrateInfo.amp = AmpValue;
        return FPICOXRHMDModule::GetPluginWrapper().UpdateVibrateParams(SourceID, &vibrateInfo);
    }
#endif
    return 0;
}

int UPICOXRInputFunctionLibrary::PXR_CreateHapticStream(FString PHFVersion, int FrameDurationMs, int Slot, int Reversal, float Amp, float Speed, int& SourceID)
{
	int result = 0;
#if PLATFORM_ANDROID
	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x200030A))
	{
		PxrVibrate_info info;
		info.amp = Amp;
		info.reversal = Reversal;
		info.slot = Slot;
		int32_t id = 0;
		result = FPICOXRHMDModule::GetPluginWrapper().CreateHapticStream(TCHAR_TO_UTF8(*PHFVersion), FrameDurationMs, &info, Speed, &id);
        SourceID = id;
	}
#endif
	return result;
}

int UPICOXRInputFunctionLibrary::PXR_RemovePHFHaptic(int SourceID)
{
	int result = 0;
#if PLATFORM_ANDROID
	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x200030A))
	{
		result = FPICOXRHMDModule::GetPluginWrapper().RemovePHFHaptic(SourceID);
	}
#endif
	return result;
}

int UPICOXRInputFunctionLibrary::PXR_StartPHFHaptic(int SourceID)
{
	int result = 0;
#if PLATFORM_ANDROID
	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x200030A))
	{
		result = FPICOXRHMDModule::GetPluginWrapper().StartPHFHaptic(SourceID);
	}
#endif
	return result;
}

int UPICOXRInputFunctionLibrary::PXR_StopPHFHaptic(int SourceID)
{
	int result = 0;
#if PLATFORM_ANDROID
	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x200030A))
	{
		result = FPICOXRHMDModule::GetPluginWrapper().StopPHFHaptic(SourceID);
	}
#endif
	return result;
}

int UPICOXRInputFunctionLibrary::PXR_WriteHapticStream(int SourceID, const FPHFJsonData& frames, int From, int NumFrames)
{
	int result = 0;
#if PLATFORM_ANDROID
	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x200030A))
	{
		PxrPhf_params ppfpara = {};

        int buffernum = FMath::Min(frames.patternData_L.Num(), frames.patternData_R.Num());
        int rest = buffernum - From;
        int maxtosent = FMath::Min(rest, 25);
        int numtosent = NumFrames > maxtosent ? maxtosent : NumFrames;
        
        for (int32 L = 0; L < numtosent; L++)
        {
			ppfpara.params[L * 2].frameseq = frames.patternData_L[From + L].frameseq;
			ppfpara.params[L * 2].frequency = frames.patternData_L[From + L].frequency;
			ppfpara.params[L * 2].gain = frames.patternData_L[From + L].gain;
			ppfpara.params[L * 2].loop = frames.patternData_L[From + L].loop;
			ppfpara.params[L * 2].play = frames.patternData_L[From + L].play;

        }

        for (int32 R = 0; R < numtosent; R++)
		{
			ppfpara.params[R * 2 + 1].frameseq = frames.patternData_R[From + R].frameseq;
			ppfpara.params[R * 2 + 1].frequency = frames.patternData_R[From + R].frequency;
			ppfpara.params[R * 2 + 1].gain = frames.patternData_R[From + R].gain;
			ppfpara.params[R * 2 + 1].loop = frames.patternData_R[From + R].loop;
			ppfpara.params[R * 2 + 1].play = frames.patternData_R[From + R].play;
		}

		result = FPICOXRHMDModule::GetPluginWrapper().WriteHapticStream(SourceID, &ppfpara, numtosent * 2);
	}
#endif
	return result;
}

int UPICOXRInputFunctionLibrary::PXR_GetCurrentFrameSequence(int SourceID, int& FrameSequence)
{
	int result = 0;
#if PLATFORM_ANDROID
	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x200030A))
	{
		uint64_t frame = 0;
		result = FPICOXRHMDModule::GetPluginWrapper().GetCurrentFrameSequence(SourceID, &frame);
        FrameSequence = frame;
	}
#endif
	return result;
}

int UPICOXRInputFunctionLibrary::PXR_SetPHFHapticSpeed(int SourceID, float Speed)
{
	int result = 0;
#if PLATFORM_ANDROID
	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x200030A))
	{
		result = FPICOXRHMDModule::GetPluginWrapper().SetPHFHapticSpeed(SourceID, Speed);
	}
#endif
	return result;
}

int UPICOXRInputFunctionLibrary::PXR_GetPHFHapticSpeed(int SourceID, float& Speed)
{
	int result = 0;
#if PLATFORM_ANDROID
	if (FPICOXRVersionHelper::IsThisVersionOrGreater(0x200030A))
	{
		result = FPICOXRHMDModule::GetPluginWrapper().GetPHFHapticSpeed(SourceID, &Speed);
	}
#endif
	return result;
}

bool UPICOXRInputFunctionLibrary::PXR_SetBodyTrackingStaticCalibState(int calibstate)
{
    bool bSucceeded=false;
    if(FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
#if PLATFORM_ANDROID
        if(FPICOXRHMDModule::GetPluginWrapper().SetBodyTrackingStaticCalibState(calibstate))
        {
           return bSucceeded;
        }
        bSucceeded=true;
#endif
    }

    return bSucceeded;
}

bool UPICOXRInputFunctionLibrary::PXR_GetBodyTrackingImuData(int deviceId, FPxrBodyTrackingImu& res) {
    bool bSucceeded = false;
    if(FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
#if PLATFORM_ANDROID
        PxrBodyTrackingImu imudata={};
       if(FPICOXRHMDModule::GetPluginWrapper().GetBodyTrackingImuData(deviceId, &imudata))
       {
           return bSucceeded;
       }
        
        bSucceeded = true;
        res.TimeStamp = static_cast<int64>(imudata.TimeStamp);
        res.temperature = static_cast<float>(imudata.temperature);
        res.AccData = FVector(-static_cast<float>(imudata.AccData[2]), static_cast<float>(imudata.AccData[0]), static_cast<float>(imudata.AccData[1]));
        res.GyroData = FVector(-static_cast<float>(imudata.GyroData[2]), static_cast<float>(imudata.GyroData[0]), static_cast<float>(imudata.GyroData[1]));
        res.MagData = FVector(-static_cast<float>(imudata.MagData[2]), static_cast<float>(imudata.MagData[0]), static_cast<float>(imudata.MagData[1]));
#endif
    }
    return bSucceeded;
}

bool UPICOXRInputFunctionLibrary::PXR_GetBodyTrackingPose(TArray<FPxrBodyTrackingRoleData>& BodyTrackingData)
{
    bool bSucceeded = false;
    if(FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
#if PLATFORM_ANDROID
        double dPredictTime=0;
        if (FPICOXRHMDModule::GetPluginWrapper().GetPredictedDisplayTime(&dPredictTime))
        {
            return false;
        }
        PxrBodyTrackingResult bodydata={};
        PXR_LOGV(PxrUnreal, "PXR_GetBodyTrackingPose: dPredictTime %f",dPredictTime);
        if (FPICOXRHMDModule::GetPluginWrapper().GetBodyTrackingPose(dPredictTime,&bodydata))
        {
            return false;
        }
        for (int i = 0; i < 24; i++) {
            FPxrBodyTrackingRoleData element = FPxrBodyTrackingRoleData();
            element.TimeStamp=static_cast<int64>(bodydata.trackingdata[i].localpose.TimeStamp);
            element.LocalPose.SetLocation(FVector(
                -static_cast<float>(bodydata.trackingdata[i].localpose.PosZ),
                static_cast<float>(bodydata.trackingdata[i].localpose.PosX),
                static_cast<float>(bodydata.trackingdata[i].localpose.PosY)));
            element.LocalPose.SetRotation(FQuat(
            -static_cast<float>(bodydata.trackingdata[i].localpose.RotQz),
            static_cast<float>(bodydata.trackingdata[i].localpose.RotQx),
            static_cast<float>(bodydata.trackingdata[i].localpose.RotQy),
            -static_cast<float>(bodydata.trackingdata[i].localpose.RotQw)));
            element.velo = FVector(-static_cast<float>(bodydata.trackingdata[i].velo[2]), static_cast<float>(bodydata.trackingdata[i].velo[0]), static_cast<float>(bodydata.trackingdata[i].velo[1]));
            element.acce = FVector(-static_cast<float>(bodydata.trackingdata[i].acce[2]), static_cast<float>(bodydata.trackingdata[i].acce[0]), static_cast<float>(bodydata.trackingdata[i].acce[1]));
            element.wvelo = FVector(-static_cast<float>(bodydata.trackingdata[i].wvelo[2]), static_cast<float>(bodydata.trackingdata[i].wvelo[0]), static_cast<float>(bodydata.trackingdata[i].wvelo[1]));
            element.wacce = FVector(-static_cast<float>(bodydata.trackingdata[i].wacce[2]), static_cast<float>(bodydata.trackingdata[i].wacce[0]), static_cast<float>(bodydata.trackingdata[i].wacce[1]));
            int Action=static_cast<int>(bodydata.trackingdata[i].bodyAction);
            if ((Action&PxrBodyActionList::PxrTouchGround)&&(Action&PxrBodyActionList::PxrKeepStatic))
            {
                element.bodyAction=EPxrBodyActionList::TouchGroundAndKeepStatic;
            }
            else if (Action&PxrBodyActionList::PxrTouchGround)
            {
                element.bodyAction = EPxrBodyActionList::TouchGround;
            }
            else if (Action&PxrBodyActionList::PxrKeepStatic)
            {
                element.bodyAction = EPxrBodyActionList::KeepStatic;
            }
            else
            {
                element.bodyAction = EPxrBodyActionList::NoneAction;
            }
            
            BodyTrackingData.Add(element);
            PXR_LOGV(PxrUnreal, "PXR_GetBodyTrackingPose:%f---%f---%f---%f---%f---%f---%f",
                BodyTrackingData[i].LocalPose.GetLocation().X, BodyTrackingData[i].LocalPose.GetLocation().Y, BodyTrackingData[i].LocalPose.GetLocation().Z,
                BodyTrackingData[i].LocalPose.GetRotation().X, BodyTrackingData[i].LocalPose.GetRotation().Y, BodyTrackingData[i].LocalPose.GetRotation().Z,
                BodyTrackingData[i].LocalPose.GetRotation().W);
        }
        bSucceeded=true;
#endif
    }
    return bSucceeded;
}

bool UPICOXRInputFunctionLibrary::PXR_GetBodyTrackingPoseByRole(EPxrBodyTrackerRole RoleType, FPxrBodyTrackingRoleData& RoleBodyTrackingData)
{
    bool bSucceeded=false;
    if(FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
#if PLATFORM_ANDROID
        double dPredictTime=0;
        if(FPICOXRHMDModule::GetPluginWrapper().GetPredictedDisplayTime(&dPredictTime))
        {
            return false;
        }
        PxrBodyTrackingResult bodydata = {};
        PXR_LOGV(PxrUnreal, "GetBodyTrackingPoseByRole: dPredictTime %f",dPredictTime);
        if(FPICOXRHMDModule::GetPluginWrapper().GetBodyTrackingPose(dPredictTime,&bodydata))
        {
            return false;
        }

        int RoleIndex=static_cast<int>(RoleType);
        
        RoleBodyTrackingData.TimeStamp=static_cast<int64>(bodydata.trackingdata[RoleIndex].localpose.TimeStamp);
        RoleBodyTrackingData.LocalPose.SetLocation(FVector(
            -static_cast<float>(bodydata.trackingdata[RoleIndex].localpose.PosZ),
            static_cast<float>(bodydata.trackingdata[RoleIndex].localpose.PosX),
            static_cast<float>(bodydata.trackingdata[RoleIndex].localpose.PosY)));
        RoleBodyTrackingData.LocalPose.SetRotation(FQuat(
        -static_cast<float>(bodydata.trackingdata[RoleIndex].localpose.RotQz),
        static_cast<float>(bodydata.trackingdata[RoleIndex].localpose.RotQx),
        static_cast<float>(bodydata.trackingdata[RoleIndex].localpose.RotQy),
        -static_cast<float>(bodydata.trackingdata[RoleIndex].localpose.RotQw)));
        RoleBodyTrackingData.velo = FVector(-static_cast<float>(bodydata.trackingdata[RoleIndex].velo[2]), static_cast<float>(bodydata.trackingdata[RoleIndex].velo[0]), static_cast<float>(bodydata.trackingdata[RoleIndex].velo[1]));
        RoleBodyTrackingData.acce = FVector(-static_cast<float>(bodydata.trackingdata[RoleIndex].acce[2]), static_cast<float>(bodydata.trackingdata[RoleIndex].acce[0]), static_cast<float>(bodydata.trackingdata[RoleIndex].acce[1]));
        RoleBodyTrackingData.wvelo = FVector(-static_cast<float>(bodydata.trackingdata[RoleIndex].wvelo[2]), static_cast<float>(bodydata.trackingdata[RoleIndex].wvelo[0]), static_cast<float>(bodydata.trackingdata[RoleIndex].wvelo[1]));
        RoleBodyTrackingData.wacce = FVector(-static_cast<float>(bodydata.trackingdata[RoleIndex].wacce[2]), static_cast<float>(bodydata.trackingdata[RoleIndex].wacce[0]), static_cast<float>(bodydata.trackingdata[RoleIndex].wacce[1]));
        int Action=static_cast<int>(bodydata.trackingdata[RoleIndex].bodyAction);
        if ((Action&PxrBodyActionList::PxrTouchGround)&&(Action&PxrBodyActionList::PxrKeepStatic))
        {
            RoleBodyTrackingData.bodyAction=EPxrBodyActionList::TouchGroundAndKeepStatic;
        }
        else if (Action&PxrBodyActionList::PxrTouchGround)
        {
            RoleBodyTrackingData.bodyAction = EPxrBodyActionList::TouchGround;
        }
        else if (Action&PxrBodyActionList::PxrKeepStatic)
        {
            RoleBodyTrackingData.bodyAction = EPxrBodyActionList::KeepStatic;
        }
        else
        {
            RoleBodyTrackingData.bodyAction = EPxrBodyActionList::NoneAction;
        }
        bSucceeded=true;

        PXR_LOGV(PxrUnreal, "PXR_GetBodyTrackingPoseByRole:%f---%f---%f---%f---%f---%f---%f",
              RoleBodyTrackingData.LocalPose.GetLocation().X, RoleBodyTrackingData.LocalPose.GetLocation().Y, RoleBodyTrackingData.LocalPose.GetLocation().Z,
              RoleBodyTrackingData.LocalPose.GetRotation().X, RoleBodyTrackingData.LocalPose.GetRotation().Y, RoleBodyTrackingData.LocalPose.GetRotation().Z,
              RoleBodyTrackingData.LocalPose.GetRotation().W);
#endif
    }
    return bSucceeded;
}

bool UPICOXRInputFunctionLibrary::PXR_SetBodyTrackingMode(int32 Mode)
{
    bool bSucceeded=false;
    if(FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
#if PLATFORM_ANDROID
        if(FPICOXRHMDModule::GetPluginWrapper().SetBodyTrackingMode(Mode))
        {
            return false;
        }
        bSucceeded=true;
#endif
    }

    return bSucceeded;
}

bool UPICOXRInputFunctionLibrary::PXR_GetFitnessBandConnectState(FFitnessBandConnectState& State)
{
    bool bSucceeded=false;
    if(FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
#if PLATFORM_ANDROID
        PxrFitnessBandConnectState ConnectState= {};
        if (FPICOXRHMDModule::GetPluginWrapper().GetFitnessBandConnectState(&ConnectState))
        {
            return false;
        }
        State.Num=ConnectState.num;
        State.TrackerIDArray.Empty();
        State.TrackerIDArray.Insert(ConnectState.trackerID,ConnectState.num,0);
        bSucceeded=true;
#endif
    }
    return bSucceeded;
}

bool UPICOXRInputFunctionLibrary::PXR_GetFitnessBandBattery(int32 TrackerId, int32& Battery)
{
    bool bSucceeded=false;
    if(FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
#if PLATFORM_ANDROID
        if (FPICOXRHMDModule::GetPluginWrapper().GetFitnessBandBattery(TrackerId,&Battery))
        {
            return false;
        }
        bSucceeded=true;
#endif
    }
    return bSucceeded;
}

bool UPICOXRInputFunctionLibrary::PXR_GetFitnessBandCalibState(int32& CalibrateState)
{
    bool bSucceeded=false;
    if(FPICOXRVersionHelper::IsThisVersionOrGreater(0x2000308))
    {
#if PLATFORM_ANDROID

        if (FPICOXRHMDModule::GetPluginWrapper().GetFitnessBandCalibState(&CalibrateState))
        {
            return false;
        }
        bSucceeded=true;
#endif
    }
    return bSucceeded;
}

bool UPICOXRInputFunctionLibrary::PXR_LaunchFitnessBandCalibrationAPP()
{
    FString APPName="com.pvr.swift";
#if PLATFORM_ANDROID
    if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
    {
        static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "StartApplication", "(Ljava/lang/String;)V", false);
        auto APPNameJava = FJavaHelper::ToJavaString(Env, APPName);
        FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, *APPNameJava);
    }
#endif
    return true;
}

bool UPICOXRInputFunctionLibrary::PXR_SetSwiftMode(ESwiftMode Mode)
{
    bool bSucceeded=false;
    if(FPICOXRVersionHelper::IsThisVersionOrGreater(0x200030C))
    {
#if PLATFORM_ANDROID
        BodyTrackingAlgParam cBodyTrackingAlgParam={};
        cBodyTrackingAlgParam.BodyJointSet=static_cast<int>(Mode);
        if (FPICOXRHMDModule::GetPluginWrapper().SetBodyTrackingAlgParam(BodyTrackingAlgParamType::SWIFT_MODE,&cBodyTrackingAlgParam))
        {
            return false;
        }
        bSucceeded=true;
#endif
    }
    return bSucceeded;
}

bool UPICOXRInputFunctionLibrary::PXR_SetBodyTrackingBoneLength(const FBodyTrackingBoneLength& BoneLength)
{
    bool bSucceeded=false;
    if(FPICOXRVersionHelper::IsThisVersionOrGreater(0x200030C))
    {
#if PLATFORM_ANDROID
        BodyTrackingAlgParam cBodyTrackingAlgParam={};
        cBodyTrackingAlgParam.BodyJointSet=static_cast<int>(ESwiftMode::SWIFT_FULLBODY);
        cBodyTrackingAlgParam.BoneLength.headLen=BoneLength.headLen;
        cBodyTrackingAlgParam.BoneLength.neckLen=BoneLength.neckLen;
        cBodyTrackingAlgParam.BoneLength.torsoLen=BoneLength.torsoLen;
        cBodyTrackingAlgParam.BoneLength.hipLen=BoneLength.hipLen;
        cBodyTrackingAlgParam.BoneLength.upperLegLen=BoneLength.upperLegLen;
        cBodyTrackingAlgParam.BoneLength.lowerLegLen=BoneLength.lowerLegLen;
        cBodyTrackingAlgParam.BoneLength.footLen=BoneLength.footLen;
        cBodyTrackingAlgParam.BoneLength.shoulderLen=BoneLength.shoulderLen;
        cBodyTrackingAlgParam.BoneLength.upperArmLen=BoneLength.upperArmLen;
        cBodyTrackingAlgParam.BoneLength.lowerArmLen=BoneLength.lowerArmLen;
        cBodyTrackingAlgParam.BoneLength.handLen=BoneLength.handLen;
        if (FPICOXRHMDModule::GetPluginWrapper().SetBodyTrackingAlgParam(BodyTrackingAlgParamType::BONE_PARAM,&cBodyTrackingAlgParam))
        {
            return false;
        }
        bSucceeded=true;
#endif
    }
    return bSucceeded;
}
