// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_CameraFunctionLibrary.h"
#include "PICO_HMD.h"
#include "IOpenXRHMDModule.h"
#include "Engine/Engine.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "LatentActions.h"
#include "RenderingThread.h"
#include "RenderUtils.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "TextureResource.h"
#include "RHI.h"
#include "RHICommandList.h"

// Helper function to get FHMDPICO instance
static FHMDPICO* GetPICOHMD()
{
	return FHMDPICO::GetPICOInstance();
}

// Helper function to skip capability data we don't need
static void SkipCapabilityData(FMemoryReader& Reader, int32 CapabilityType)
{
	switch (CapabilityType)
	{
	case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_RESOLUTION_PICO: // 1010033008
	{
		uint32 Count;
		Reader << Count;
		for (uint32 i = 0; i < Count; i++)
		{
			int32 Width, Height;
			Reader << Width << Height;
		}
		break;
	}
	case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_FORMAT_PICO:       // 1010033012
	case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_DATA_TRANSFER_TYPE_PICO: // 1010033010
	case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_CAMERA_MODEL_PICO:       // 1010033014
	case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_FPS_PICO:          // 1010033016
	{
		uint32 Count;
		Reader << Count;
		for (uint32 i = 0; i < Count; i++)
		{
			int32 Value;
			Reader << Value;
		}
		break;
	}
	default:
		break;
	}
}

bool UPICOCameraFunctionLibrary::IsCameraImageSupported()
{
	FHMDPICO* HMD = GetPICOHMD();
	return HMD && HMD->IsCameraImageExtensionSupported();
}

bool UPICOCameraFunctionLibrary::EnumerateAvailableCameras(const FPICOCameraPropertyFilter& PropertyFilter, TArray<int64>& OutCameraIDs)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD)
	{
		return false;
	}

	TArray<uint64> AllCameraIDs;
	if (!HMD->EnumerateAvailableCameras(AllCameraIDs))
	{
		return false;
	}

	OutCameraIDs.Empty();

	// If no filters are active, return all cameras
	if (!PropertyFilter.bFilterByFacing && !PropertyFilter.bFilterByPosition && !PropertyFilter.bFilterByCameraType)
	{
		OutCameraIDs.SetNum(AllCameraIDs.Num());
		for (int32 i = 0; i < AllCameraIDs.Num(); i++)
		{
			OutCameraIDs[i] = static_cast<int64>(AllCameraIDs[i]);
		}
		return true;
	}

	// Apply filters
	for (uint64 CameraID : AllCameraIDs)
	{
		TArray<uint8> PropertiesData;
		if (!HMD->GetCameraProperties(CameraID, PropertiesData))
		{
			continue; // Skip cameras we can't query
		}

		// Deserialize properties
		FMemoryReader Reader(PropertiesData);
		uint32 PropertyCount;
		Reader << PropertyCount;

		EPICOCameraFacing CameraFacing = EPICOCameraFacing::World;
		EPICOCameraPosition CameraPosition = EPICOCameraPosition::Unspecified;
		EPICOCameraType CameraType = EPICOCameraType::PassthroughColor;
		bool bHasFacing = false;
		bool bHasPosition = false;
		bool bHasCameraType = false;

		// Read all properties
		for (uint32 i = 0; i < PropertyCount; i++)
		{
			int32 PropertyType;
			Reader << PropertyType;

			switch (PropertyType)
			{
			case XR_TYPE_CAMERA_PROPERTY_FACING_PICO: // 1010033003
			{
				int32 FacingValue;
				Reader << FacingValue;
				CameraFacing = static_cast<EPICOCameraFacing>(FacingValue);
				bHasFacing = true;
				break;
			}
			case XR_TYPE_CAMERA_PROPERTY_POSITION_PICO: // 1010033004
			{
				int32 PositionValue;
				Reader << PositionValue;
				CameraPosition = static_cast<EPICOCameraPosition>(PositionValue);
				bHasPosition = true;
				break;
			}
			case XR_TYPE_CAMERA_PROPERTY_CAMERA_TYPE_PICO: // 1010033005
			{
				int32 TypeValue;
				Reader << TypeValue;
				CameraType = static_cast<EPICOCameraType>(TypeValue);
				bHasCameraType = true;
				break;
			}
			default:
			{
				int32 DummyValue;
				Reader << DummyValue;
				break;
			}
			}
		}

		// Apply filters
		bool bPassedFilter = true;

		if (PropertyFilter.bFilterByFacing && bHasFacing)
		{
			if (CameraFacing != PropertyFilter.Facing)
			{
				bPassedFilter = false;
			}
		}

		if (PropertyFilter.bFilterByPosition && bHasPosition)
		{
			if (CameraPosition != PropertyFilter.Position)
			{
				bPassedFilter = false;
			}
		}

		if (PropertyFilter.bFilterByCameraType && bHasCameraType)
		{
			if (CameraType != PropertyFilter.CameraType)
			{
				bPassedFilter = false;
			}
		}

		// Add camera if it passed all filters
		if (bPassedFilter)
		{
			OutCameraIDs.Add(static_cast<int64>(CameraID));
		}
	}

	return true;
}

bool UPICOCameraFunctionLibrary::GetCameraProperties(int64 CameraID, EPICOCameraFacing& OutFacing, EPICOCameraPosition& OutPosition, EPICOCameraType& OutCameraType, bool& bOutSupportsFacing, bool& bOutSupportsPosition, bool& bOutSupportsCameraType)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD)
	{
		return false;
	}

	// Get serialized properties data from HMD
	TArray<uint8> PropertiesData;
	if (!HMD->GetCameraProperties(static_cast<uint64>(CameraID), PropertiesData))
	{
		return false;
	}

	// Deserialize the data
	FMemoryReader Reader(PropertiesData);
	
	uint32 PropertyCount;
	Reader << PropertyCount;

	// Initialize support flags to false
	bOutSupportsFacing = false;
	bOutSupportsPosition = false;
	bOutSupportsCameraType = false;

	// Initialize output values to defaults
	OutFacing = EPICOCameraFacing::World;
	OutPosition = EPICOCameraPosition::Unspecified;
	OutCameraType = EPICOCameraType::PassthroughColor;

	// Read each property
	for (uint32 i = 0; i < PropertyCount; i++)
	{
		int32 PropertyType;
		Reader << PropertyType;

		switch (PropertyType)
		{
		case XR_TYPE_CAMERA_PROPERTY_FACING_PICO: // 1010033003
		{
			int32 FacingValue;
			Reader << FacingValue;
			OutFacing = static_cast<EPICOCameraFacing>(FacingValue);
			bOutSupportsFacing = true;
			break;
		}
		case XR_TYPE_CAMERA_PROPERTY_POSITION_PICO: // 1010033004
		{
			int32 PositionValue;
			Reader << PositionValue;
			OutPosition = static_cast<EPICOCameraPosition>(PositionValue);
			bOutSupportsPosition = true;
			break;
		}
		case XR_TYPE_CAMERA_PROPERTY_CAMERA_TYPE_PICO: // 1010033005
		{
			int32 TypeValue;
			Reader << TypeValue;
			OutCameraType = static_cast<EPICOCameraType>(TypeValue);
			bOutSupportsCameraType = true;
			break;
		}
		default:
		{
			// Skip unknown property value
			int32 DummyValue;
			Reader << DummyValue;
			break;
		}
		}
	}

	return true;
}

bool UPICOCameraFunctionLibrary::GetSupportedCapabilityTypes(int64 CameraID, TArray<EPICOCameraCapabilityType>& OutCapabilityTypes)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD)
	{
		return false;
	}

	// Get capabilities data which includes all capability types
	TArray<uint8> CapabilitiesData;
	if (!HMD->GetCameraSupportedCapabilities(static_cast<uint64>(CameraID), CapabilitiesData))
	{
		return false;
	}

	FMemoryReader Reader(CapabilitiesData);
	
	uint32 CapabilityCount;
	Reader << CapabilityCount;

	OutCapabilityTypes.Empty();
	
	for (uint32 i = 0; i < CapabilityCount; i++)
	{
		int32 CapabilityType;
		Reader << CapabilityType;

		// Map XrStructureType to EPICOCameraCapabilityType
		switch (CapabilityType)
		{
		case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_RESOLUTION_PICO: // 1010033008
			OutCapabilityTypes.Add(EPICOCameraCapabilityType::ImageResolution);
			// Skip resolution data
			{
				uint32 ResCount;
				Reader << ResCount;
				for (uint32 j = 0; j < ResCount; j++)
				{
					int32 Width, Height;
					Reader << Width << Height;
				}
			}
			break;
		case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_DATA_TRANSFER_TYPE_PICO: // 1010033010
			OutCapabilityTypes.Add(EPICOCameraCapabilityType::DataTransferType);
			// Skip transfer type data
			{
				uint32 TypeCount;
				Reader << TypeCount;
				for (uint32 j = 0; j < TypeCount; j++)
				{
					int32 TransferType;
					Reader << TransferType;
				}
			}
			break;
		case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_FORMAT_PICO: // 1010033012
			OutCapabilityTypes.Add(EPICOCameraCapabilityType::ImageFormat);
			// Skip format data
			{
				uint32 FormatCount;
				Reader << FormatCount;
				for (uint32 j = 0; j < FormatCount; j++)
				{
					int32 Format;
					Reader << Format;
				}
			}
			break;
		case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_CAMERA_MODEL_PICO: // 1010033014
			OutCapabilityTypes.Add(EPICOCameraCapabilityType::CameraModel);
			// Skip model data
			{
				uint32 ModelCount;
				Reader << ModelCount;
				for (uint32 j = 0; j < ModelCount; j++)
				{
					int32 Model;
					Reader << Model;
				}
			}
			break;
		case XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_FPS_PICO: // 1010033016
			OutCapabilityTypes.Add(EPICOCameraCapabilityType::ImageFPS);
			// Skip FPS data
			{
				uint32 FPSCount;
				Reader << FPSCount;
				for (uint32 j = 0; j < FPSCount; j++)
				{
					int32 FPS;
					Reader << FPS;
				}
			}
			break;
		default:
			break;
		}
	}

	return true;
}

bool UPICOCameraFunctionLibrary::GetSupportedResolutions(int64 CameraID, FPICOCameraSupportedResolutions& OutResolutions)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD)
	{
		return false;
	}

	TArray<uint8> CapabilitiesData;
	if (!HMD->GetCameraSupportedCapabilities(static_cast<uint64>(CameraID), CapabilitiesData))
	{
		return false;
	}

	FMemoryReader Reader(CapabilitiesData);
	
	uint32 CapabilityCount;
	Reader << CapabilityCount;

	OutResolutions.Resolutions.Empty();
	
	for (uint32 i = 0; i < CapabilityCount; i++)
	{
		int32 CapabilityType;
		Reader << CapabilityType;

		if (CapabilityType == XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_RESOLUTION_PICO) // 1010033008
		{
			uint32 ResCount;
			Reader << ResCount;
			
			for (uint32 j = 0; j < ResCount; j++)
			{
				int32 Width, Height;
				Reader << Width << Height;
				OutResolutions.Resolutions.Add(FIntPoint(Width, Height));
			}
			return true;
		}
		else
		{
			// Skip other capability types
			SkipCapabilityData(Reader, CapabilityType);
		}
	}

	return false;
}

bool UPICOCameraFunctionLibrary::GetSupportedImageFormats(int64 CameraID, FPICOCameraSupportedFormats& OutFormats)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD)
	{
		return false;
	}

	TArray<uint8> CapabilitiesData;
	if (!HMD->GetCameraSupportedCapabilities(static_cast<uint64>(CameraID), CapabilitiesData))
	{
		return false;
	}

	FMemoryReader Reader(CapabilitiesData);
	
	uint32 CapabilityCount;
	Reader << CapabilityCount;

	OutFormats.Formats.Empty();
	
	for (uint32 i = 0; i < CapabilityCount; i++)
	{
		int32 CapabilityType;
		Reader << CapabilityType;

		if (CapabilityType == XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_FORMAT_PICO) // 1010033012
		{
			uint32 FormatCount;
			Reader << FormatCount;
			
			for (uint32 j = 0; j < FormatCount; j++)
			{
				int32 Format;
				Reader << Format;
				OutFormats.Formats.Add(static_cast<EPICOCameraImageFormat>(Format));
			}
			return true;
		}
		else
		{
			SkipCapabilityData(Reader, CapabilityType);
		}
	}

	return false;
}

bool UPICOCameraFunctionLibrary::GetSupportedFPS(int64 CameraID, FPICOCameraSupportedFPS& OutFPS)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD)
	{
		return false;
	}

	TArray<uint8> CapabilitiesData;
	if (!HMD->GetCameraSupportedCapabilities(static_cast<uint64>(CameraID), CapabilitiesData))
	{
		return false;
	}

	FMemoryReader Reader(CapabilitiesData);
	
	uint32 CapabilityCount;
	Reader << CapabilityCount;

	OutFPS.FPSValues.Empty();
	
for (uint32 i = 0; i < CapabilityCount; i++)
	{
		int32 CapabilityType;
		Reader << CapabilityType;

		if (CapabilityType == XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_IMAGE_FPS_PICO) // 1010033016
		{
			uint32 FPSCount;
			Reader << FPSCount;
			
			for (uint32 j = 0; j < FPSCount; j++)
			{
				int32 FPS;
				Reader << FPS;
				OutFPS.FPSValues.Add(static_cast<EPICOCameraImageFPS>(FPS));
			}
			return true;
		}
		else
		{
			SkipCapabilityData(Reader, CapabilityType);
		}
	}

	return false;
}

bool UPICOCameraFunctionLibrary::GetSupportedTransferTypes(int64 CameraID, FPICOCameraSupportedTransferTypes& OutTransferTypes)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD)
	{
		return false;
	}

	TArray<uint8> CapabilitiesData;
	if (!HMD->GetCameraSupportedCapabilities(static_cast<uint64>(CameraID), CapabilitiesData))
	{
		return false;
	}

	FMemoryReader Reader(CapabilitiesData);
	
	uint32 CapabilityCount;
	Reader << CapabilityCount;

	OutTransferTypes.TransferTypes.Empty();
	
	for (uint32 i = 0; i < CapabilityCount; i++)
	{
		int32 CapabilityType;
		Reader << CapabilityType;

		if (CapabilityType == XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_DATA_TRANSFER_TYPE_PICO) // 1010033010
		{
			uint32 TypeCount;
			Reader << TypeCount;
			
			for (uint32 j = 0; j < TypeCount; j++)
			{
				int32 TransferType;
				Reader << TransferType;
				OutTransferTypes.TransferTypes.Add(static_cast<EPICOCameraDataTransferType>(TransferType));
			}
			return true;
		}
		else
		{
			SkipCapabilityData(Reader, CapabilityType);
		}
	}

	return false;
}

bool UPICOCameraFunctionLibrary::GetSupportedModels(int64 CameraID, FPICOCameraSupportedModels& OutModels)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD)
	{
		return false;
	}

	TArray<uint8> CapabilitiesData;
	if (!HMD->GetCameraSupportedCapabilities(static_cast<uint64>(CameraID), CapabilitiesData))
	{
		return false;
	}

	FMemoryReader Reader(CapabilitiesData);
	
	uint32 CapabilityCount;
	Reader << CapabilityCount;

	OutModels.Models.Empty();
	
	for (uint32 i = 0; i < CapabilityCount; i++)
	{
		int32 CapabilityType;
		Reader << CapabilityType;

		if (CapabilityType == XR_TYPE_CAMERA_SUPPORTED_CAPABILITY_CAMERA_MODEL_PICO) // 1010033014
		{
			uint32 ModelCount;
			Reader << ModelCount;
			
			for (uint32 j = 0; j < ModelCount; j++)
			{
				int32 Model;
				Reader << Model;
				OutModels.Models.Add(static_cast<EPICOCameraModel>(Model));
			}
			return true;
		}
		else
		{
			SkipCapabilityData(Reader, CapabilityType);
		}
	}

	return false;
}

// Immediate completion action for validation failures
class FPICOCameraImmediateCompleteAction : public FPendingLatentAction
{
public:
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
	bool* bSuccessPtr;
	bool bSuccessValue;

	FPICOCameraImmediateCompleteAction(const FLatentActionInfo& LatentInfo, bool& bSuccess, bool bInSuccessValue)
		: ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
		, bSuccessPtr(&bSuccess)
		, bSuccessValue(bInSuccessValue)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		*bSuccessPtr = bSuccessValue;
		Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
	}
};

// Latent action for async camera device creation
class FPICOCameraDeviceCreateAction : public FPendingLatentAction
{
public:
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
	bool* bSuccessPtr;
	FPICOCameraDeviceHandle* OutHandlePtr;
	int64 DeviceHandle;
	uint64 Future;
	FHMDPICO* HMD;
	int32 PollCount;
	int32 LastLogFrame;

	FPICOCameraDeviceCreateAction(const FLatentActionInfo& LatentInfo, bool& bSuccess, FPICOCameraDeviceHandle& OutHandle, 
		int64 InDeviceHandle, uint64 InFuture, FHMDPICO* InHMD)
		: ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
		, bSuccessPtr(&bSuccess)
		, OutHandlePtr(&OutHandle)
		, DeviceHandle(InDeviceHandle)
		, Future(InFuture)
		, HMD(InHMD)
		, PollCount(0)
		, LastLogFrame(0)
	{
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("FPICOCameraDeviceCreateAction started: DeviceHandle=%lld, Future=%llu"), 
			DeviceHandle, Future);
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		PollCount++;
		
		// Log progress every 60 frames (~1 second at 60fps)
		if (PollCount - LastLogFrame >= 60)
		{
			UE_LOG(LogPICOOpenXRHMD, Verbose, TEXT("Camera device creation polling: Frame=%d, DeviceHandle=%lld"), 
				PollCount, DeviceHandle);
			LastLogFrame = PollCount;
		}
		
		// Try to complete the operation (now uses xrPollFutureEXT internally)
		bool bSuccess = HMD && HMD->CompleteCameraDeviceCreation(DeviceHandle, Future);
		
		// Timeout after 300 frames (~5 seconds at 60fps)
		if (PollCount > 300)
		{
			UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Camera device creation timeout after %d frames, DeviceHandle=%lld"), 
				PollCount, DeviceHandle);
			*bSuccessPtr = false;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			return;
		}
		
		// Check if completed
		if (bSuccess)
		{
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Camera device creation completed after %d frames, DeviceHandle=%lld"), 
				PollCount, DeviceHandle);
			*bSuccessPtr = true;
			*OutHandlePtr = FPICOCameraDeviceHandle(DeviceHandle);
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};

void UPICOCameraFunctionLibrary::CreateCameraDeviceAsync(UObject* WorldContextObject, int64 CameraID, 
	FLatentActionInfo LatentInfo, bool& bSuccess, FPICOCameraDeviceHandle& OutDeviceHandle)
{
	bSuccess = false;
	
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD || !WorldContextObject)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("CreateCameraDeviceAsync: Invalid HMD or WorldContext"));
		// Still trigger completion with failure result
		if (WorldContextObject)
		{
			UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
			if (World)
			{
				FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
				if (LatentActionManager.FindExistingAction<FPICOCameraImmediateCompleteAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
				{
					FPICOCameraImmediateCompleteAction* NewAction = new FPICOCameraImmediateCompleteAction(LatentInfo, bSuccess, false);
					LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
				}
			}
		}
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("CreateCameraDeviceAsync: Failed to get World"));
		// Still trigger completion with failure result
		UWorld* TempWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (TempWorld)
		{
			FPICOCameraImmediateCompleteAction* NewAction = new FPICOCameraImmediateCompleteAction(LatentInfo, bSuccess, false);
			TempWorld->GetLatentActionManager().AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		}
		return;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	if (LatentActionManager.FindExistingAction<FPICOCameraDeviceCreateAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) != nullptr)
	{
		UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("CreateCameraDeviceAsync: Duplicate action found"));
		// Still trigger completion with failure result
		FPICOCameraImmediateCompleteAction* NewAction = new FPICOCameraImmediateCompleteAction(LatentInfo, bSuccess, false);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		return;
	}

	static int64 NextHandle = 1;
	int64 DeviceHandle = NextHandle++;
	uint64 Future;

	if (HMD->CreateCameraDeviceAsync(static_cast<uint64>(CameraID), DeviceHandle, Future))
	{
		FPICOCameraDeviceCreateAction* NewAction = new FPICOCameraDeviceCreateAction(LatentInfo, bSuccess, 
			OutDeviceHandle, DeviceHandle, Future, HMD);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
	}
	else
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("CreateCameraDeviceAsync: Failed to start async operation"));
		// Still trigger completion with failure result
		FPICOCameraImmediateCompleteAction* NewAction = new FPICOCameraImmediateCompleteAction(LatentInfo, bSuccess, false);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
	}
}

bool UPICOCameraFunctionLibrary::DestroyCameraDevice(FPICOCameraDeviceHandle DeviceHandle)
{
	FHMDPICO* HMD = GetPICOHMD();
	return HMD && HMD->DestroyCameraDevice(DeviceHandle.Handle);
}

// Latent action for async capture session creation
class FPICOCaptureSessionCreateAction : public FPendingLatentAction
{
public:
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
	bool* bSuccessPtr;
	FPICOCameraCaptureSessionHandle* OutHandlePtr;
	int64 SessionHandle;
	uint64 Future;
	FHMDPICO* HMD;
	int32 PollCount;
	int32 LastLogFrame;

	FPICOCaptureSessionCreateAction(const FLatentActionInfo& LatentInfo, bool& bSuccess, 
		FPICOCameraCaptureSessionHandle& OutHandle, int64 InSessionHandle, uint64 InFuture, FHMDPICO* InHMD)
		: ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
		, bSuccessPtr(&bSuccess)
		, OutHandlePtr(&OutHandle)
		, SessionHandle(InSessionHandle)
		, Future(InFuture)
		, HMD(InHMD)
		, PollCount(0)
		, LastLogFrame(0)
	{
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("FPICOCaptureSessionCreateAction started: SessionHandle=%lld, Future=%llu"), 
			SessionHandle, Future);
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		PollCount++;
		
		// Log progress every 60 frames (~1 second at 60fps)
		if (PollCount - LastLogFrame >= 60)
		{
			UE_LOG(LogPICOOpenXRHMD, Verbose, TEXT("Capture session creation polling: Frame=%d, SessionHandle=%lld"), 
				PollCount, SessionHandle);
			LastLogFrame = PollCount;
		}
		
		// Try to complete the operation (now uses xrPollFutureEXT internally)
		bool bSuccess = HMD && HMD->CompleteCaptureSessionCreation(SessionHandle, Future);
		
		// Timeout after 300 frames (~5 seconds at 60fps)
		if (PollCount > 300)
		{
			UE_LOG(LogPICOOpenXRHMD, Error, TEXT("Capture session creation timeout after %d frames, SessionHandle=%lld"), 
				PollCount, SessionHandle);
			*bSuccessPtr = false;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			return;
		}
		
		// Check if completed
		if (bSuccess)
		{
			UE_LOG(LogPICOOpenXRHMD, Log, TEXT("Capture session creation completed after %d frames, SessionHandle=%lld"), 
				PollCount, SessionHandle);
			*bSuccessPtr = true;
			*OutHandlePtr = FPICOCameraCaptureSessionHandle(SessionHandle);
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}
};

void UPICOCameraFunctionLibrary::CreateCaptureSessionAsync(UObject* WorldContextObject, FPICOCameraDeviceHandle DeviceHandle, 
	const FPICOCameraCaptureConfig& Config, FLatentActionInfo LatentInfo, bool& bSuccess, FPICOCameraCaptureSessionHandle& OutSessionHandle)
{
	bSuccess = false;
	
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD || !WorldContextObject || !DeviceHandle.IsValid())
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("CreateCaptureSessionAsync: Invalid HMD, WorldContext, or DeviceHandle"));
		// Still trigger completion with failure result
		if (WorldContextObject)
		{
			UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
			if (World)
			{
				FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
				if (LatentActionManager.FindExistingAction<FPICOCameraImmediateCompleteAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
				{
					FPICOCameraImmediateCompleteAction* NewAction = new FPICOCameraImmediateCompleteAction(LatentInfo, bSuccess, false);
					LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
				}
			}
		}
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("CreateCaptureSessionAsync: Failed to get World"));
		// Still trigger completion with failure result
		UWorld* TempWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
		if (TempWorld)
		{
			FPICOCameraImmediateCompleteAction* NewAction = new FPICOCameraImmediateCompleteAction(LatentInfo, bSuccess, false);
			TempWorld->GetLatentActionManager().AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		}
		return;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
	if (LatentActionManager.FindExistingAction<FPICOCaptureSessionCreateAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) != nullptr)
	{
		UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("CreateCaptureSessionAsync: Duplicate action found"));
		// Still trigger completion with failure result
		FPICOCameraImmediateCompleteAction* NewAction = new FPICOCameraImmediateCompleteAction(LatentInfo, bSuccess, false);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
		return;
	}

	static int64 NextHandle = 1;
	int64 SessionHandle = NextHandle++;
	uint64 Future;

	// Serialize config
	TArray<uint8> ConfigData;
	FMemoryWriter Writer(ConfigData);
	
	// Write config count (5 configs: Resolution, Format, TransferType, Model, FPS)
	int32 ConfigCount = 5;
	Writer << ConfigCount;
	
	// 1. Resolution - use XR_CAMERA_CAPABILITY_TYPE_IMAGE_RESOLUTION_PICO (1)
	int32 ConfigType = XR_CAMERA_CAPABILITY_TYPE_IMAGE_RESOLUTION_PICO;
	Writer << ConfigType;
	// Copy to non-const variables for serialization
	int32 TempResX = Config.Resolution.X;
	int32 TempResY = Config.Resolution.Y;
	Writer << TempResX;
	Writer << TempResY;
	
	// 2. Image Format - use XR_CAMERA_CAPABILITY_TYPE_IMAGE_FORMAT_PICO (2)
	ConfigType = XR_CAMERA_CAPABILITY_TYPE_IMAGE_FORMAT_PICO;
	Writer << ConfigType;
	int32 FormatValue = static_cast<int32>(Config.ImageFormat);
	Writer << FormatValue;
	
	// 3. Transfer Type - use XR_CAMERA_CAPABILITY_TYPE_DATA_TRANSFER_TYPE_PICO (3)
	ConfigType = XR_CAMERA_CAPABILITY_TYPE_DATA_TRANSFER_TYPE_PICO;
	Writer << ConfigType;
	int32 TransferValue = static_cast<int32>(Config.TransferType);
	Writer << TransferValue;
	
	// 4. Camera Model - use XR_CAMERA_CAPABILITY_TYPE_CAMERA_MODEL_PICO (4)
	ConfigType = XR_CAMERA_CAPABILITY_TYPE_CAMERA_MODEL_PICO;
	Writer << ConfigType;
	int32 ModelValue = static_cast<int32>(Config.CameraModel);
	Writer << ModelValue;
	
	// 5. FPS - use XR_CAMERA_CAPABILITY_TYPE_IMAGE_FPS_PICO (5)
	ConfigType = XR_CAMERA_CAPABILITY_TYPE_IMAGE_FPS_PICO;
	Writer << ConfigType;
	int32 FPSValue = static_cast<int32>(Config.FPS);
	Writer << FPSValue;

	if (HMD->CreateCaptureSessionAsync(DeviceHandle.Handle, ConfigData, SessionHandle, Future))
	{
		FPICOCaptureSessionCreateAction* NewAction = new FPICOCaptureSessionCreateAction(LatentInfo, bSuccess, 
			OutSessionHandle, SessionHandle, Future, HMD);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
	}
	else
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("CreateCaptureSessionAsync: Failed to start async operation"));
		// Still trigger completion with failure result
		FPICOCameraImmediateCompleteAction* NewAction = new FPICOCameraImmediateCompleteAction(LatentInfo, bSuccess, false);
		LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, NewAction);
	}
}

bool UPICOCameraFunctionLibrary::DestroyCaptureSession(FPICOCameraCaptureSessionHandle SessionHandle)
{
	FHMDPICO* HMD = GetPICOHMD();
	return HMD && HMD->DestroyCaptureSession(SessionHandle.Handle);
}

bool UPICOCameraFunctionLibrary::GetCameraIntrinsics(FPICOCameraCaptureSessionHandle SessionHandle, FPICOCameraIntrinsics& OutIntrinsics)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD || !SessionHandle.IsValid())
	{
		return false;
	}

	FVector2D FocalLength, PrincipalPoint, FOV;
	if (HMD->GetCameraIntrinsics(SessionHandle.Handle, FocalLength, PrincipalPoint, FOV))
	{
		OutIntrinsics.FocalLength = FocalLength;
		OutIntrinsics.PrincipalPoint = PrincipalPoint;
		OutIntrinsics.FOV = FOV;
		return true;
	}

	return false;
}

bool UPICOCameraFunctionLibrary::GetCameraExtrinsics(FPICOCameraCaptureSessionHandle SessionHandle, FPICOCameraExtrinsics& OutExtrinsics)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD || !SessionHandle.IsValid())
	{
		return false;
	}

	return HMD->GetCameraExtrinsics(SessionHandle.Handle, OutExtrinsics.Pose);
}

bool UPICOCameraFunctionLibrary::BeginCameraCapture(FPICOCameraCaptureSessionHandle SessionHandle)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("BeginCameraCapture: Failed to get PICO HMD instance"));
		return false;
	}
	
	if (!SessionHandle.IsValid())
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("BeginCameraCapture: Invalid session handle"));
		return false;
	}
	
	bool bResult = HMD->BeginCameraCapture(SessionHandle.Handle);
	
	if (bResult)
	{
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("BeginCameraCapture: Started - SessionHandle=%lld"), SessionHandle.Handle);
	}
	else
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("BeginCameraCapture: Failed - SessionHandle=%lld"), SessionHandle.Handle);
	}
	
	return bResult;
}

bool UPICOCameraFunctionLibrary::EndCameraCapture(FPICOCameraCaptureSessionHandle SessionHandle)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("EndCameraCapture: Failed to get PICO HMD instance"));
		return false;
	}
	
	if (!SessionHandle.IsValid())
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("EndCameraCapture: Invalid session handle"));
		return false;
	}
	
	bool bResult = HMD->EndCameraCapture(SessionHandle.Handle);
	
	if (bResult)
	{
		UE_LOG(LogPICOOpenXRHMD, Log, TEXT("EndCameraCapture: Stopped - SessionHandle=%lld"), SessionHandle.Handle);
	}
	else
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("EndCameraCapture: Failed - SessionHandle=%lld"), SessionHandle.Handle);
	}
	
	return bResult;
}

bool UPICOCameraFunctionLibrary::AcquireCameraImage(FPICOCameraCaptureSessionHandle SessionHandle, 
	FTimespan LastCaptureTime, FPICOCameraImageData& OutImageData)
{
	FHMDPICO* HMD = GetPICOHMD();
	if (!HMD)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("AcquireCameraImage: Failed to get PICO HMD instance"));
		return false;
	}
	
	if (!SessionHandle.IsValid())
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("AcquireCameraImage: Invalid session handle"));
		return false;
	}

	int64 CaptureTime;
	uint64 ImageID;
	bool bNewImage = false;

	if (!HMD->AcquireCameraImage(SessionHandle.Handle, LastCaptureTime.GetTicks(), CaptureTime, ImageID, bNewImage))
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("AcquireCameraImage: Failed to acquire - SessionHandle=%lld"), 
			SessionHandle.Handle);
		return false;
	}

	if (!bNewImage)
	{
		// No new image available - this is normal, don't log
		return false;
	}

	// Get image data
	TArray<uint8> Buffer;
	int32 Width, Height, Stride;
	
	if (HMD->GetCameraImageData(SessionHandle.Handle, ImageID, Buffer, Width, Height, Stride))
	{
		OutImageData.Width = Width;
		OutImageData.Height = Height;
		OutImageData.Stride = Stride;
		OutImageData.BytesPerPixel = 4; // RGBA8888
		OutImageData.CaptureTime = FTimespan(CaptureTime);
		OutImageData.Buffer = MoveTemp(Buffer);

		// Auto-release the image
		if (!HMD->ReleaseCameraImage(SessionHandle.Handle, ImageID))
		{
			UE_LOG(LogPICOOpenXRHMD, Warning, TEXT("AcquireCameraImage: Failed to release image - ImageID=%llu"), ImageID);
		}
		
		return true;
	}

	// GetCameraImageData failed
	UE_LOG(LogPICOOpenXRHMD, Error, TEXT("AcquireCameraImage: Failed to get image data - ImageID=%llu"), ImageID);

	// Release on failure
	HMD->ReleaseCameraImage(SessionHandle.Handle, ImageID);
	
	return false;
}

UTexture2D* UPICOCameraFunctionLibrary::CreateTextureFromImageData(const FPICOCameraImageData& ImageData)
{
	if (ImageData.Width <= 0 || ImageData.Height <= 0 || ImageData.Buffer.Num() == 0)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("CreateTextureFromImageData: Invalid image data - %dx%d, Buffer=%d bytes"),
			ImageData.Width, ImageData.Height, ImageData.Buffer.Num());
		return nullptr;
	}

	UTexture2D* Texture = UTexture2D::CreateTransient(ImageData.Width, ImageData.Height, PF_R8G8B8A8);
	if (!Texture)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("CreateTextureFromImageData: Failed to create transient texture"));
		return nullptr;
	}

	// Copy data to texture
	#if ENGINE_MAJOR_VERSION == 5
	void* TextureData = Texture->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, ImageData.Buffer.GetData(), ImageData.Buffer.Num());
	Texture->GetPlatformData()->Mips[0].BulkData.Unlock();
	Texture->UpdateResource();
	#endif

	return Texture;
}

bool UPICOCameraFunctionLibrary::UpdateRenderTargetWithImageData(UTextureRenderTarget2D* RenderTarget, const FPICOCameraImageData& ImageData)
{
	if (!RenderTarget)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("UpdateRenderTargetWithImageData: RenderTarget is null"));
		return false;
	}

	if (ImageData.Width <= 0 || ImageData.Height <= 0 || ImageData.Buffer.Num() == 0)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("UpdateRenderTargetWithImageData: Invalid image data - %dx%d, Buffer=%d bytes"),
			ImageData.Width, ImageData.Height, ImageData.Buffer.Num());
		return false;
	}

	if (RenderTarget->SizeX != ImageData.Width || RenderTarget->SizeY != ImageData.Height || RenderTarget->GetFormat() != EPixelFormat::PF_R8G8B8A8)
	{
		RenderTarget->InitCustomFormat(ImageData.Width, ImageData.Height, EPixelFormat::PF_R8G8B8A8, false);
	}

	// Calculate expected buffer size
	const int32 ExpectedBufferSize = ImageData.Width * ImageData.Height * ImageData.BytesPerPixel;
	if (ImageData.Buffer.Num() < ExpectedBufferSize)
	{
		UE_LOG(LogPICOOpenXRHMD, Error, TEXT("UpdateRenderTargetWithImageData: Buffer too small - Got %d, Expected %d"),
			ImageData.Buffer.Num(), ExpectedBufferSize);
		return false;
	}

	// Update render target on render thread
	// Use shared pointer to avoid copying large buffer (3.1MB for 1024x768)
	// This reduces memory copies from 2 to 0 (zero-copy after initial acquisition)
	TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> BufferPtr = MakeShareable(new TArray<uint8>(ImageData.Buffer));
	const int32 Width = ImageData.Width;
	const int32 Height = ImageData.Height;
	const int32 Stride = ImageData.Stride;

	ENQUEUE_RENDER_COMMAND(UpdateRenderTargetCommand)(
		[RenderTarget, BufferPtr, Width, Height, Stride](FRHICommandListImmediate& RHICmdList)
		{
			FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GetRenderTargetResource();
			if (!RenderTargetResource)
			{
				return;
			}

			FRHITexture* RHITexture = RenderTargetResource->GetRenderTargetTexture();
			if (!RHITexture)
			{
				return;
			}

			// Create update region
			FUpdateTextureRegion2D UpdateRegion(0, 0, 0, 0, Width, Height);

			// Update texture
			RHIUpdateTexture2D(
				RHITexture,
				0, // MipIndex
				UpdateRegion,
				Stride,
				BufferPtr->GetData()
			);
		});

	return true;
}
 
// ====================================================================================
// Utility Functions - ToString Conversions
// ====================================================================================

FString UPICOCameraFunctionLibrary::Conv_CameraIntrinsicsToString(const FPICOCameraIntrinsics& Intrinsics)
{
	return Intrinsics.ToString();
}

FString UPICOCameraFunctionLibrary::Conv_CameraExtrinsicsToString(const FPICOCameraExtrinsics& Extrinsics)
{
	return Extrinsics.ToString();
}

FString UPICOCameraFunctionLibrary::Conv_CameraImageDataToString(const FPICOCameraImageData& ImageData)
{
	return FString::Printf(TEXT("Size: %dx%d, Stride: %d, BPP: %d, BufferSize: %d bytes, Time: %s"),
		ImageData.Width,
		ImageData.Height,
		ImageData.Stride,
		ImageData.BytesPerPixel,
		ImageData.Buffer.Num(),
		*ImageData.CaptureTime.ToString());
}

FString UPICOCameraFunctionLibrary::Conv_CameraCaptureConfigToString(const FPICOCameraCaptureConfig& Config)
{
	FString FormatName;
	switch (Config.ImageFormat)
	{
	case EPICOCameraImageFormat::RGBA_8888: FormatName = TEXT("RGBA8888"); break;
	default: FormatName = TEXT("Unknown"); break;
	}

	FString TransferName;
	switch (Config.TransferType)
	{
	case EPICOCameraDataTransferType::RawBuffer: TransferName = TEXT("Raw Buffer"); break;
	default: TransferName = TEXT("Unknown"); break;
	}

	FString ModelName;
	switch (Config.CameraModel)
	{
	case EPICOCameraModel::Pinhole: ModelName = TEXT("Pinhole"); break;
	default: ModelName = TEXT("Unknown"); break;
	}

	FString FPSName;
	switch (Config.FPS)
	{
	case EPICOCameraImageFPS::FPS_30: FPSName = TEXT("30 FPS"); break;
	case EPICOCameraImageFPS::FPS_60: FPSName = TEXT("60 FPS"); break;
	default: FPSName = TEXT("Unknown"); break;
	}

	return FString::Printf(TEXT("Resolution: %dx%d, Format: %s, Transfer: %s, Model: %s, FPS: %s"),
		Config.Resolution.X,
		Config.Resolution.Y,
		*FormatName,
		*TransferName,
		*ModelName,
		*FPSName);
}
