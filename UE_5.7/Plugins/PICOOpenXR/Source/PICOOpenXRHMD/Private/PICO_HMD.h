// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IOpenXRExtensionPlugin.h"
#include "OpenXRHMD.h"
#include "PICO_HMDFunctionLibrary.h"

class IXRTrackingSystem;

class FHMDPICO : public IOpenXRExtensionPlugin
{
public:
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnXRSessionStateChangedPICO, int32);

	FHMDPICO();
	virtual ~FHMDPICO() override;

	void Register();
	void Unregister();

	// Static accessor for Blueprint library
	static FHMDPICO* GetPICOInstance() { return PICOInstance; }

	/** IOpenXRExtensionPlugin implementation */
	virtual FString GetDisplayName() override
	{
		return FString(TEXT("HMDPICO"));
	}

	virtual bool GetCustomLoader(PFN_xrGetInstanceProcAddr* OutGetProcAddr) override;
	virtual bool InsertOpenXRAPILayer(PFN_xrGetInstanceProcAddr& InOutGetProcAddr) override;
	virtual bool GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual void PostCreateInstance(XrInstance InInstance) override;
	virtual void PostGetSystem(XrInstance InInstance, XrSystemId InSystem) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual void PostCreateSession(XrSession InSession) override;
	virtual void OnDestroySession(XrSession InSession) override;
	virtual bool UseCustomReferenceSpaceType(XrReferenceSpaceType& OutReferenceSpaceType) override;
	virtual bool GetSpectatorScreenController(FHeadMountedDisplayBase* InHMDBase, TUniquePtr<FDefaultSpectatorScreenController>& OutSpectatorScreenController) override;
	virtual void OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader) override;
	virtual const void* OnEndProjectionLayer_RHIThread(XrSession InSession, int32 InLayerIndex, const void* InNext, XrCompositionLayerFlags& OutFlags) override;
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace) override;
	virtual void OnBeginRendering_GameThread(XrSession InSession, FSceneViewFamily& InViewFamily, TArrayView<const uint32> VisibleLayers) override;
	virtual void OnBeginRendering_RenderThread(XrSession InSession, FRDGBuilder& GraphBuilder) override;
	virtual void UpdateCompositionLayers_RHIThread(XrSession InSession, TArray<XrCompositionLayerBaseHeader*>& Headers) override;

	FOnXRSessionStateChangedPICO& OnXRSessionStateChanged() { return OnXRSessionStateChangedDelegate; }
	int32 GetCurrentSessionStateInt() const { return (int32)CurrentSessionState; }

	bool GetSupportedDisplayRefreshRates(TArray<float>& Rates);
	bool GetCurrentDisplayRefreshRate(float& Rate, bool DoubleCheck);
	bool SetDisplayRefreshRate(float Rate);

	void EnableContentProtect(bool Enable);

	bool SetPerformanceLevel(int domain, int level);
	bool GetDevicePoseForTime(const EControllerHand Hand, bool UseDefaultTime, FTimespan Timespan, bool& OutTimeWasUsed, FRotator& Orientation, FVector& Position, bool& bProvidedLinearVelocity, FVector& LinearVelocity, bool& bProvidedAngularVelocity, FVector& AngularVelocityRadPerSec, bool& bProvidedLinearAcceleration, FVector& LinearAcceleration, float InWorldToMetersScale);
	EHMDWornState::Type GetHMDWornState(bool& ResultValid);
	bool GetFieldOfView(float& OutHFOVInDegrees, float& OutVFOVInDegrees);
	bool GetInterpupillaryDistance(float& IPD);
	void SetBaseRotationAndBaseOffset(FRotator Rotation, FVector BaseOffset, EOrientPositionSelector::Type Options);
	void GetBaseRotationAndBaseOffset(FRotator& OutRotation, FVector& OutBaseOffset);
    FTimespan GetDisplayTime();

	bool IsStationaryBoundaryMode(bool& bIsStationary);
	bool GetVirtualBoundaryStatus(bool& bIsReady, bool& bIsEnable, bool& bIsVisible);
	bool SetVirtualBoundaryEnable(bool bEnable);
	bool SetVirtualBoundaryVisible(bool bVisible);
	bool SetVirtualBoundarySeeThroughVisible(bool bVisible);
	bool BoundaryintersectPointOrNode(bool bPoint, EControllerHand Node, FVector Point, EBoundaryTypePICO BoundaryType, bool& Valid, bool& IsTriggering, float& ClosestDistance, FVector& ClosestPoint, FVector& ClosestPointNormal, float InWorldToMetersScale = 100);
	bool GetBoundaryGeometry(EBoundaryTypePICO BoundaryType, bool& Valid, TArray<FVector>& Points, float InWorldToMetersScale);

	// Camera Image Extension API
	bool IsCameraImageExtensionSupported() const;
	bool EnumerateAvailableCameras(TArray<uint64>& OutCameraIDs);
	bool GetCameraProperties(uint64 CameraID, TArray<uint8>& OutPropertiesData);
	bool GetCameraSupportedCapabilities(uint64 CameraID, TArray<uint8>& OutCapabilitiesData);
	bool CreateCameraDeviceAsync(uint64 CameraID, int64 Handle, uint64& OutFuture);
	bool CompleteCameraDeviceCreation(int64 Handle, uint64 Future);
	bool DestroyCameraDevice(int64 Handle);
	bool CreateCaptureSessionAsync(int64 DeviceHandle, const TArray<uint8>& ConfigsData, int64 SessionHandle, uint64& OutFuture);
	bool CompleteCaptureSessionCreation(int64 SessionHandle, uint64 Future);
	bool DestroyCaptureSession(int64 SessionHandle);
	bool GetCameraIntrinsics(int64 SessionHandle, FVector2D& OutFocalLength, FVector2D& OutPrincipalPoint, FVector2D& OutFOV);
	bool GetCameraExtrinsics(int64 SessionHandle, FTransform& OutPose);
	bool BeginCameraCapture(int64 SessionHandle);
	bool EndCameraCapture(int64 SessionHandle);
	bool AcquireCameraImage(int64 SessionHandle, int64 LastCaptureTime, int64& OutCaptureTime, uint64& OutImageID, bool& bNewImage);
	bool GetCameraImageData(int64 SessionHandle, uint64 ImageID, TArray<uint8>& OutBuffer, int32& OutWidth, int32& OutHeight, int32& OutStride);
	bool ReleaseCameraImage(int64 SessionHandle, uint64 ImageID);

	bool GetAdaptivePixelDensity(EAdaptiveResolutionSettingPICO Setting, float& PixelDensity);
	FIntPoint GetDefaultRenderTargetSize();
	void GetCurrentRenderTargetSize(uint32& InOutSizeX, uint32& InOutSizeY);
	bool SetProjectionLayerColorMatrix3x3f(bool Enable, FVector3f ColumnA, FVector3f ColumnB, FVector3f ColumnC);
	bool GetViewportSize(FIntPoint& Size);
	bool GetBatteryStateDisplay(const EControllerHand Hand, bool& OutValid, float& OutBatteryLevel, bool& OutCharging, bool& OutPluggedIn, bool& OutNoBattery);
	bool GetRenderModelGLBBytes(EControllerHand Hand, TArray<uint8>& OutGlbBytes, FString& OutError);
	bool GetRenderModelWorldTransform(EControllerHand Hand, FTransform& OutWorldTransform, FString& OutError);

	void CopyTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture* DstTexture, FRHITexture* SrcTexture, FIntRect DstRect = FIntRect(), FIntRect SrcRect = FIntRect(), bool bAlphaPremultiply = false, bool bNoAlpha = false, bool bClearGreen = false, bool bInvertX = false, bool bInvertY = false, bool bInvertAlpha = false) const;
	
	bool								 bDynamicResolution = false;
	float 							     CurrentDynamicPixelDensity = 1.0f;
	float							     MinimumResolutionScale = 0.6f;
	EAdaptiveResolutionSettingPICO 	     CurrentAdaptiveResolutionSetting = EAdaptiveResolutionSettingPICO::Balanced;

private:
	void*								 LoaderHandle;
	FRWLock								 SessionHandleMutex;
	XrInstance							 Instance;
	XrSystemId							 System;
	XrSession							 Session;
	XrPath								 CommonInteractionProfile;
	// Cached in PostCreateInstance(). Used to gate runtime-specific behavior on the render thread.
	bool								 bIsPicoRuntime = false;

	bool								 bSupportLocalFloorLevelEXT;

	bool								 bSupportDisplayRefreshRate;
	float								 CurrentDisplayRefreshRate;
	TArray<float>						 SupportedDisplayRefreshRates;
	PFN_xrEnumerateDisplayRefreshRatesFB xrEnumerateDisplayRefreshRatesFB = nullptr;
	PFN_xrGetDisplayRefreshRateFB		 xrGetDisplayRefreshRateFB = nullptr;
	PFN_xrRequestDisplayRefreshRateFB	 xrRequestDisplayRefreshRateFB = nullptr;

	bool								 bContentProtectEnabled;
	XrCompositionLayerSecureContentFB	 ContentProtect;
	bool								 bSupportPerformanceSettingsEXT;
	PFN_xrPerfSettingsSetPerformanceLevelEXT xrPerfSettingsSetPerformanceLevelEXT = nullptr;
	bool								 bSupportBatteryStateDisplayEXT = false;
	bool								 bHasLeftHandUserPath = false;
	bool								 bHasRightHandUserPath = false;
	bool								 bHasHeadUserPath = false;
	XrPath								 LeftHandUserPath = XR_NULL_PATH;
	XrPath								 RightHandUserPath = XR_NULL_PATH;
	XrPath								 HeadUserPath = XR_NULL_PATH;

	XrTime                               CurrentDisplayTime;
	class FOpenXRHMD*				     OpenXRHMD = nullptr;
	bool                                 IsSupportsUserPresence;
	EHMDWornState::Type     			 WornState;

	bool								 bSupportedVirtualBoundary = false;
	PFN_xrGetVirtualBoundaryModePICO     xrGetVirtualBoundaryModePICO = nullptr;
	PFN_xrGetVirtualBoundaryStatusPICO   xrGetVirtualBoundaryStatusPICO = nullptr;
	PFN_xrSetVirtualBoundaryEnablePICO	 xrSetVirtualBoundaryEnablePICO = nullptr;
	PFN_xrSetVirtualBoundaryVisiblePICO	 xrSetVirtualBoundaryVisiblePICO = nullptr;
	PFN_xrSetVirtualBoundarySeeThroughVisiblePICO xrSetVirtualBoundarySeeThroughVisiblePICO = nullptr;
	PFN_xrGetVirtualBoundaryTriggerPICO	 xrGetVirtualBoundaryTriggerPICO = nullptr;
	PFN_xrGetVirtualBoundaryGeometryPICO xrGetVirtualBoundaryGeometryPICO = nullptr;

	XrSpace								 CurrentBaseSpace = XR_NULL_HANDLE;

	FIntRect GetViewportSize(const FOpenXRLayer::FPerEyeTextureData& EyeData, const IStereoLayers::FLayerDesc& Desc);
	FVector2D GetQuadSize(const FOpenXRLayer::FPerEyeTextureData& EyeData, const IStereoLayers::FLayerDesc& Desc);

	void OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaTime);
	FDelegateHandle OnWorldTickStartDelegateHandle;

	/*************************** MRC Begin ***************************/
public:
	bool bSupportMRCExtension;
	void CreateMRCLayer(class UTexture* BackgroundRTTexture, class UTexture* ForegroundRTTexture);
	void DestroyMRCLayer();
	bool GetExternalCameraInfo(int32& width, int32& height, float& fov);
	bool GetExternalCameraPose(FTransform& Pose);
	void EnableMRCDebugMode(class UWorld* WorldContext, bool Enable, bool ViewInHMD, bool UseCustomTransform, const FTransform& Pose, bool UseCustomCameraInfo, int Width, int Height, float Fov);
	void DisableMRCForegroundLayer(UObject* WorldContextObject, bool Disable);
	void PauseMRC(bool Pause);
	bool								 bIsMRCForegroundLayerDisabled = false;
private:
	PFN_xrGetExternalCameraInfoPICO		 xrGetExternalCameraInfoPICO = nullptr;
	XrSpace								 ViewTrackingSpace = XR_NULL_HANDLE;
	XrSpace								 MRCSpace = XR_NULL_HANDLE;
	bool								 bIsMRCRunning = false;
	bool								 bIsMRCRunningStored = false;
	bool								 bIsMRCForegroundLayerDisabled_RebderThread = false;
	
	class AMRCCameraPICO*				 MRCSceneCapture2DPICO = nullptr;
	IStereoLayers::FLayerDesc			 MRCLayerDesc_RenderThread;
	TSharedPtr<FOpenXRLayer, ESPMode::ThreadSafe> MRCLayer;
	XrCompositionLayerQuad				 MRCQuadLayerLeft_RenderThread;
	XrCompositionLayerQuad				 MRCQuadLayerLeft_RHIThread;
	XrCompositionLayerQuad				 MRCQuadLayerRight_RenderThread;
	XrCompositionLayerQuad				 MRCQuadLayerRight_RHIThread;

	struct FMRCDebugModePICO
	{
		FMRCDebugModePICO()
			:EnableExtension(false)
			, ViewInHMD(false)
			, UseCustomTransform(false)
			, Pose(FTransform::Identity)
			, UseCustomCameraInfo(false)
			, Width(256)
			, Height(256)
			, Fov(90.0f)
		{}

		bool EnableExtension;
		bool ViewInHMD;
		bool UseCustomTransform;
		FTransform Pose;
		bool UseCustomCameraInfo;
		int Width;
		int Height;
		float Fov;
	};

	FMRCDebugModePICO					 MRCDebugMode;

	/*************************** MRC End ***************************/

	/*************************** Camera Image Extension Start ***************************/
	bool bSupportedCameraImage;
	bool bSupportedFutureEXT;  // XR_EXT_future extension support
	bool bSupportedRenderModelEXT = false;
	bool bSupportedInteractionRenderModelEXT = false;
	XrSessionState						 CurrentSessionState = XR_SESSION_STATE_UNKNOWN;
	FOnXRSessionStateChangedPICO		 OnXRSessionStateChangedDelegate;
	
	// Camera Image Extension Function Pointers
	PFN_xrEnumerateAvailableCamerasPICO xrEnumerateAvailableCamerasPICO = nullptr;
	PFN_xrEnumerateCameraPropertyTypesPICO xrEnumerateCameraPropertyTypesPICO = nullptr;
	PFN_xrGetCameraPropertiesPICO xrGetCameraPropertiesPICO = nullptr;
	PFN_xrEnumerateCameraCapabilityTypesPICO xrEnumerateCameraCapabilityTypesPICO = nullptr;
	PFN_xrGetCameraSupportedCapabilitiesPICO xrGetCameraSupportedCapabilitiesPICO = nullptr;
	PFN_xrCreateCameraDeviceAsyncPICO xrCreateCameraDeviceAsyncPICO = nullptr;
	PFN_xrCreateCameraDeviceCompletePICO xrCreateCameraDeviceCompletePICO = nullptr;
	PFN_xrDestroyCameraDevicePICO xrDestroyCameraDevicePICO = nullptr;
	PFN_xrCreateCameraCaptureSessionAsyncPICO xrCreateCameraCaptureSessionAsyncPICO = nullptr;
	PFN_xrCreateCameraCaptureSessionCompletePICO xrCreateCameraCaptureSessionCompletePICO = nullptr;
	PFN_xrDestroyCameraCaptureSessionPICO xrDestroyCameraCaptureSessionPICO = nullptr;
	PFN_xrGetCameraIntrinsicsPICO xrGetCameraIntrinsicsPICO = nullptr;
	PFN_xrGetCameraExtrinsicsPICO xrGetCameraExtrinsicsPICO = nullptr;
	PFN_xrBeginCameraCapturePICO xrBeginCameraCapturePICO = nullptr;
	PFN_xrEndCameraCapturePICO xrEndCameraCapturePICO = nullptr;
	PFN_xrAcquireCameraImagePICO xrAcquireCameraImagePICO = nullptr;
	PFN_xrGetCameraImageDataPICO xrGetCameraImageDataPICO = nullptr;
	PFN_xrReleaseCameraImagePICO xrReleaseCameraImagePICO = nullptr;
	
	// XR_EXT_future extension function pointers
	PFN_xrPollFutureEXT xrPollFutureEXT = nullptr;
	PFN_xrCancelFutureEXT xrCancelFutureEXT = nullptr;

	PFN_xrEnumerateInteractionRenderModelIdsEXT xrEnumerateInteractionRenderModelIdsEXT = nullptr;
	PFN_xrCreateRenderModelEXT xrCreateRenderModelEXT = nullptr;
	PFN_xrDestroyRenderModelEXT xrDestroyRenderModelEXT = nullptr;
	PFN_xrGetRenderModelPropertiesEXT xrGetRenderModelPropertiesEXT = nullptr;
	PFN_xrCreateRenderModelAssetEXT xrCreateRenderModelAssetEXT = nullptr;
	PFN_xrDestroyRenderModelAssetEXT xrDestroyRenderModelAssetEXT = nullptr;
	PFN_xrGetRenderModelAssetDataEXT xrGetRenderModelAssetDataEXT = nullptr;
	PFN_xrEnumerateRenderModelSubactionPathsEXT xrEnumerateRenderModelSubactionPathsEXT = nullptr;
	PFN_xrCreateRenderModelSpaceEXT xrCreateRenderModelSpaceEXT = nullptr;

	XrRenderModelEXT LeftRenderModelForSpace = XR_NULL_HANDLE;
	XrRenderModelEXT RightRenderModelForSpace = XR_NULL_HANDLE;
	XrSpace LeftRenderModelSpace = XR_NULL_HANDLE;
	XrSpace RightRenderModelSpace = XR_NULL_HANDLE;
	FCriticalSection RenderModelSpaceMutex;
	
	// Camera device and session management
	struct FCameraDeviceInfo
	{
		XrCameraDevicePICO Device;
		XrCameraIdPICO CameraID;
		bool bValid;
		
		FCameraDeviceInfo() : Device(XR_NULL_HANDLE), CameraID(0), bValid(false) {}
	};
	
	struct FCameraCaptureSessionInfo
	{
		XrCameraCaptureSessionPICO Session;
		XrCameraDevicePICO Device;
		bool bCapturing;
		bool bValid;
		TMap<XrCameraImageIdPICO, bool> AcquiredImages;
		
		FCameraCaptureSessionInfo() 
			: Session(XR_NULL_HANDLE)
			, Device(XR_NULL_HANDLE)
			, bCapturing(false)
			, bValid(false) 
		{}
	};
	
	TMap<int64, FCameraDeviceInfo> CameraDevices;
	TMap<int64, FCameraCaptureSessionInfo> CaptureSessions;
	int64 NextDeviceHandle;
	int64 NextSessionHandle;
	
	TMap<XrFutureEXT, int64> PendingDeviceCreation;
	TMap<XrFutureEXT, int64> PendingSessionCreation;
	/*************************** Camera Image Extension End ***************************/

	bool								 bSupportedFBCompositionLayerSettingsExt;
	bool								 bSupportedPICOCompositionLayerSettingsExt;
	XrLayerSettingsPICO					 ProjectionLayerSettings;
	XrCompositionLayerSettingsFB		 ProjectionLayerSettingsFB;

	bool								 bSupportAdaptiveResolution = false;
	PFN_xrUpdateAdaptiveResolutionPICO	 xrUpdateAdaptiveResolutionPICO = nullptr;

	bool								 bSupportColorMatrixExtension = false;
	bool								 bUseColorMatrixExtension = false;
	float ColorMatrix3x3f[9] = {};

	static FHMDPICO* PICOInstance;
};
