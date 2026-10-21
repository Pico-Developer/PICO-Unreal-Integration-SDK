// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PICO_CameraTypes.h"
#include "PICO_CameraFunctionLibrary.generated.h"

/**
 * Camera Device Handle - represents a logical connection to a camera
 */
USTRUCT(BlueprintType)
struct FPICOCameraDeviceHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	int64 Handle;

	FPICOCameraDeviceHandle() : Handle(-1) {}
	explicit FPICOCameraDeviceHandle(int64 InHandle) : Handle(InHandle) {}

	bool IsValid() const { return Handle >= 0; }
};

/**
 * Camera Capture Session Handle - represents a configured capture stream
 */
USTRUCT(BlueprintType)
struct FPICOCameraCaptureSessionHandle
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PICO|Camera")
	int64 Handle;

	FPICOCameraCaptureSessionHandle() : Handle(-1) {}
	explicit FPICOCameraCaptureSessionHandle(int64 InHandle) : Handle(InHandle) {}

	bool IsValid() const { return Handle >= 0; }
};

/**
 * Async operation result delegates
 */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPICOCameraDeviceCreatedDelegate, bool, bSuccess, FPICOCameraDeviceHandle, DeviceHandle);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FPICOCameraCaptureSessionCreatedDelegate, bool, bSuccess, FPICOCameraCaptureSessionHandle, SessionHandle);

/**
 * PICO Camera Function Library
 * Provides Blueprint access to PICO camera features
 */
UCLASS()
class PICOOPENXRHMD_API UPICOCameraFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ====================================================================================
	// Camera Enumeration and Properties
	// ====================================================================================

	/**
	 * Enumerate available cameras on the device
	 * @param PropertyFilter Optional filter based on camera properties
	 * @param OutCameraIDs Array of camera IDs that match the filter
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Enumerate Available Cameras PICO", AutoCreateRefTerm = "PropertyFilter"))
	static bool EnumerateAvailableCameras(const FPICOCameraPropertyFilter& PropertyFilter, TArray<int64>& OutCameraIDs);

	/**
	 * Get properties of a specific camera
	 * @param CameraID The camera to query
	 * @param OutFacing Camera facing direction
	 * @param OutPosition Camera position
	 * @param OutCameraType Camera type
	 * @param bOutSupportsFacing Whether the camera supports Facing property
	 * @param bOutSupportsPosition Whether the camera supports Position property
	 * @param bOutSupportsCameraType Whether the camera supports CameraType property
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Get Camera Properties PICO"))
	static bool GetCameraProperties(int64 CameraID, EPICOCameraFacing& OutFacing, EPICOCameraPosition& OutPosition, EPICOCameraType& OutCameraType, bool& bOutSupportsFacing, bool& bOutSupportsPosition, bool& bOutSupportsCameraType);

	// ====================================================================================
	// Camera Capabilities
	// ====================================================================================

	/**
	 * Get supported capability types for a camera
	 * @param CameraID The camera to query
	 * @param OutCapabilityTypes Array of supported capability types
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Get Supported Capability Types PICO"))
	static bool GetSupportedCapabilityTypes(int64 CameraID, TArray<EPICOCameraCapabilityType>& OutCapabilityTypes);

	/**
	 * Get supported image resolutions for a camera
	 * @param CameraID The camera to query
	 * @param OutResolutions Array of supported resolutions
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Get Supported Resolutions PICO"))
 static bool GetSupportedResolutions(int64 CameraID, FPICOCameraSupportedResolutions& OutResolutions);

	/**
	 * Get supported image formats for a camera
	 * @param CameraID The camera to query
	 * @param OutFormats Array of supported formats
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Get Supported Image Formats PICO"))
	static bool GetSupportedImageFormats(int64 CameraID, FPICOCameraSupportedFormats& OutFormats);

	/**
	 * Get supported FPS values for a camera
	 * @param CameraID The camera to query
	 * @param OutFPS Array of supported FPS values
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Get Supported FPS PICO"))
	static bool GetSupportedFPS(int64 CameraID, FPICOCameraSupportedFPS& OutFPS);

	/**
	 * Get supported data transfer types for a camera
	 * @param CameraID The camera to query
	 * @param OutTransferTypes Array of supported data transfer types
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Get Supported Transfer Types PICO"))
	static bool GetSupportedTransferTypes(int64 CameraID, FPICOCameraSupportedTransferTypes& OutTransferTypes);

	/**
	 * Get supported camera models for a camera
	 * @param CameraID The camera to query
	 * @param OutModels Array of supported camera models
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Get Supported Models PICO"))
	static bool GetSupportedModels(int64 CameraID, FPICOCameraSupportedModels& OutModels);

	// ====================================================================================
	// Camera Device Management
	// ====================================================================================

	/**
	 * Create a camera device (asynchronous)
	 * @param CameraID The camera to initialize
	 * @param OnCompleted Delegate called when creation completes
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Create Camera Device Async PICO", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void CreateCameraDeviceAsync(UObject* WorldContextObject, int64 CameraID, FLatentActionInfo LatentInfo, bool& bSuccess, FPICOCameraDeviceHandle& OutDeviceHandle);

	/**
	 * Destroy a camera device
	 * @param DeviceHandle The device to destroy
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Destroy Camera Device PICO"))
	static bool DestroyCameraDevice(FPICOCameraDeviceHandle DeviceHandle);

	// ====================================================================================
	// Camera Capture Session Management
	// ====================================================================================

	/**
	 * Create a camera capture session (asynchronous)
	 * @param DeviceHandle The camera device
	 * @param Config Capture configuration
	 * @param OnCompleted Delegate called when creation completes
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Create Capture Session Async PICO", Latent, LatentInfo = "LatentInfo", WorldContext = "WorldContextObject"))
	static void CreateCaptureSessionAsync(UObject* WorldContextObject, FPICOCameraDeviceHandle DeviceHandle, const FPICOCameraCaptureConfig& Config, FLatentActionInfo LatentInfo, bool& bSuccess, FPICOCameraCaptureSessionHandle& OutSessionHandle);

	/**
	 * Destroy a camera capture session
	 * @param SessionHandle The session to destroy
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Destroy Capture Session PICO"))
	static bool DestroyCaptureSession(FPICOCameraCaptureSessionHandle SessionHandle);

	/**
	 * Get camera intrinsics for a capture session
	 * @param SessionHandle The capture session
	 * @param OutIntrinsics Camera intrinsics data
	 *                      - FocalLength: in pixels (not meters)
	 *                      - PrincipalPoint: in pixels (not meters)
	 *                      - FOV: in degrees
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Get Camera Intrinsics PICO"))
	static bool GetCameraIntrinsics(FPICOCameraCaptureSessionHandle SessionHandle, FPICOCameraIntrinsics& OutIntrinsics);

	/**
	 * Get camera extrinsics for a capture session
	 * @param SessionHandle The capture session
	 * @param OutExtrinsics Camera extrinsics data (Position in cm)
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Get Camera Extrinsics PICO"))
	static bool GetCameraExtrinsics(FPICOCameraCaptureSessionHandle SessionHandle, FPICOCameraExtrinsics& OutExtrinsics);

	// ====================================================================================
	// Image Capture
	// ====================================================================================

	/**
	 * Begin camera capture
	 * @param SessionHandle The capture session
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Begin Camera Capture PICO"))
	static bool BeginCameraCapture(FPICOCameraCaptureSessionHandle SessionHandle);

	/**
	 * End camera capture
	 * @param SessionHandle The capture session
	 * @return true if successful
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "End Camera Capture PICO"))
	static bool EndCameraCapture(FPICOCameraCaptureSessionHandle SessionHandle);

	/**
	 * Acquire the latest camera image
	 * @param SessionHandle The capture session
	 * @param LastCaptureTime Timestamp of last acquired image (0 to get earliest available)
	 * @param OutImageData Image data (if available)
	 * @return true if new image available, false if no update
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Acquire Camera Image PICO"))
	static bool AcquireCameraImage(FPICOCameraCaptureSessionHandle SessionHandle, FTimespan LastCaptureTime, FPICOCameraImageData& OutImageData);

	/**
	 * Create a texture from camera image data
	 * @param ImageData The image data
	 * @return Created texture (nullptr on failure)
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Create Texture From Image Data PICO"))
	static UTexture2D* CreateTextureFromImageData(const FPICOCameraImageData& ImageData);

	/**
	 * Update a render target with camera image data
	 * 
	 * IMPORTANT: The render target must use RGBA8 or RGBA8_SRGB format.
	 * Other formats (e.g., RGB8, R16F, RGBA16F) may cause visual artifacts,
	 * incorrect colors, or crashes due to format mismatch with camera RGBA8888 data.
	 * 
	 * @param RenderTarget The render target to update (must be RGBA8/RGBA8_SRGB format, will auto-resize if dimensions don't match)
	 * @param ImageData The image data to copy (RGBA8888 format from camera)
	 * @return true if successful, false on validation failure
	 * 
	 * @warning If render target format is not RGBA8/RGBA8_SRGB, a warning will be logged and visual issues may occur
	 * @see CreateTextureFromImageData for one-time texture creation
	 */
	UFUNCTION(BlueprintCallable, Category = "PICO|Camera", meta = (DisplayName = "Update Render Target With Image Data PICO"))
	static bool UpdateRenderTargetWithImageData(UTextureRenderTarget2D* RenderTarget, const FPICOCameraImageData& ImageData);

	/**
	 * Check if camera image extension is supported
	 * @return true if supported
	 */
	UFUNCTION(BlueprintPure, Category = "PICO|Camera", meta = (DisplayName = "Is Camera Image Supported PICO"))
	static bool IsCameraImageSupported();

	// ====================================================================================
	// Utility Functions - ToString Conversions
	// ====================================================================================

	/**
	 * Convert camera intrinsics to string for display/debug
	 * @param Intrinsics Camera intrinsics data
	 * @return String representation
	 */
	UFUNCTION(BlueprintPure, Category = "PICO|Camera|Utility", meta = (DisplayName = "To String (Camera Intrinsics PICO)", CompactNodeTitle = "->", BlueprintAutocast))
	static FString Conv_CameraIntrinsicsToString(const FPICOCameraIntrinsics& Intrinsics);

	/**
	 * Convert camera extrinsics to string for display/debug
	 * @param Extrinsics Camera extrinsics data
	 * @return String representation
	 */
	UFUNCTION(BlueprintPure, Category = "PICO|Camera|Utility", meta = (DisplayName = "To String (Camera Extrinsics PICO)", CompactNodeTitle = "->", BlueprintAutocast))
	static FString Conv_CameraExtrinsicsToString(const FPICOCameraExtrinsics& Extrinsics);

	/**
	 * Convert camera image data to string for display/debug
	 * @param ImageData Camera image data
	 * @return String representation
	 */
	UFUNCTION(BlueprintPure, Category = "PICO|Camera|Utility", meta = (DisplayName = "To String (Camera Image Data PICO)", CompactNodeTitle = "->", BlueprintAutocast))
	static FString Conv_CameraImageDataToString(const FPICOCameraImageData& ImageData);

	/**
	 * Convert camera capture config to string for display/debug
	 * @param Config Camera capture configuration
	 * @return String representation
	 */
	UFUNCTION(BlueprintPure, Category = "PICO|Camera|Utility", meta = (DisplayName = "To String (Camera Capture Config PICO)", CompactNodeTitle = "->", BlueprintAutocast))
	static FString Conv_CameraCaptureConfigToString(const FPICOCameraCaptureConfig& Config);
};
