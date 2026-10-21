// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IOpenXRExtensionPlugin.h"
#include "PICO_MRTextures.h"
#include "PICO_MRTypes.h"
#include "openxr/pico_light_estimation.h"

// Forward declaration
class FPICOOpenXRMRModule;
DECLARE_DELEGATE_OneParam(FPICOPollFutureDelegate, const XrFutureEXT&);

class FSpatialSensingExtensionPICO
{
public:
	FSpatialSensingExtensionPICO();
	virtual ~FSpatialSensingExtensionPICO() {}

	static FSpatialSensingExtensionPICO* GetInstance();

	static EResultPICO CastToPICOResult(XrResult Result);

	virtual void OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader);
	virtual void GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions);
	virtual void PostGetSystem(XrInstance InInstance, XrSystemId InSystem);
	virtual void PostCreateSession(XrSession InSession);
	virtual void OnDestroySession(XrSession InSession);
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace);
	
	virtual bool StartSenseDataProviderComplete(const XrFutureEXT& FutureHandle, FSenseDataProviderStartCompletionPICO& Completion,EResultPICO& OutResult);
	virtual bool QuerySenseDataComplete(const XrSenseDataProviderPICO& ProviderHandle,const XrFutureEXT& FutureHandle, FSenseDataQueryCompletionPICO& Completion,EResultPICO& OutResult);
	virtual bool GetQueriedSenseData(const XrSenseDataProviderPICO& ProviderHandle,const XrSenseDataSnapshotPICO& QueryResultHandle, FQueriedSenseDataPICO& QueriedSenseData,EResultPICO& OutResult);
	virtual bool DestroySenseDataQueryResult(const XrSenseDataSnapshotPICO& QueryResultHandle,EResultPICO& OutResult);
	virtual bool GetSpatialEntityLocation(const XrSenseDataSnapshotPICO& SnapshotHandle,const XrSpatialEntityIdPICO& EntityHandle, FTransform& Transform,EResultPICO& OutResult);
	virtual bool GetSpatialEntitySemantic(const XrSenseDataSnapshotPICO& SnapshotHandle,const XrSpatialEntityIdPICO& EntityHandle, TArray<ESemanticLabelPICO>& Semantics,EResultPICO& OutResult);
	virtual bool GetSpatialEntityBoundary3D(const XrSenseDataSnapshotPICO& SnapshotHandle,const XrSpatialEntityIdPICO& EntityHandle, FBoundingBox3DPICO& Box,EResultPICO& OutResult);
	virtual bool GetSpatialEntityBoundary2D(const XrSenseDataSnapshotPICO& SnapshotHandle,const XrSpatialEntityIdPICO& EntityHandle, FBoundingBox2DPICO& Box,EResultPICO& OutResult);
	virtual bool GetSpatialEntityPolygon(const XrSenseDataSnapshotPICO& SnapshotHandle,const XrSpatialEntityIdPICO& EntityHandle, TArray<FVector>& Vertices,EResultPICO& OutResult);
	virtual bool GetSpatialEntityTriangleMesh(const XrSenseDataSnapshotPICO& SnapshotHandle,const XrSpatialEntityIdPICO& EntityHandle, TArray<FVector>& Vertices, TArray<uint16>& Triangles,EResultPICO& OutResult);
	virtual bool GetSpatialEntityLightEstimateData(const XrSenseDataSnapshotPICO& SnapshotHandle,const XrSpatialEntityIdPICO& EntityHandle,const FIntPoint& Size,ULightEstimationTexturePICO* OutEnvironmentTexture,EResultPICO& OutResult);
	virtual bool GetSpatialEntityPlaneOrientation(const XrSenseDataSnapshotPICO& SnapshotHandle,const XrSpatialEntityIdPICO& EntityHandle,EPlaneOrientationPICO& PlaneOrientationPICO,EResultPICO& OutResult);

	
	virtual bool EnumerateSpatialEntityComponentTypes(const XrSenseDataSnapshotPICO& SnapshotHandle,const XrSpatialEntityIdPICO& EntityHandle, TArray<ESpatialEntityComponentTypePICO>& componentTypes,EResultPICO& OutResult);
	virtual bool AddPollFutureRequirement(const XrFutureEXT& FutureHandle, const FPICOPollFutureDelegate& Delegate);

	bool IsSupportsSpatialSensing() const {return bSupportsSpatialSensing; }
	bool IsSupportsSpatialSensingEXT() const {return bSupportsSpatialSensingEXT; }

	// XR_PICO_spatial_sensing
	PFN_xrCreateSenseDataProviderPICO			xrCreateSenseDataProviderPICO		 = nullptr;
	PFN_xrStartSenseDataProviderAsyncPICO		xrStartSenseDataProviderAsyncPICO	 = nullptr;
	PFN_xrStartSenseDataProviderCompletePICO	xrStartSenseDataProviderCompletePICO = nullptr;
	PFN_xrGetSenseDataProviderStatePICO			xrGetSenseDataProviderStatePICO		 = nullptr;
	PFN_xrQuerySenseDataAsyncPICO			    xrQuerySenseDataAsyncPICO			 = nullptr;
	PFN_xrQuerySenseDataCompletePICO			xrQuerySenseDataCompletePICO		 = nullptr;
	PFN_xrStopSenseDataProviderPICO				xrStopSenseDataProviderPICO			 = nullptr;
	PFN_xrDestroySenseDataProviderPICO			xrDestroySenseDataProviderPICO		 = nullptr;
	PFN_xrDestroySenseDataSnapshotPICO          xrDestroySenseDataSnapshotPICO		 = nullptr;
	PFN_xrGetQueriedSenseDataPICO			    xrGetQueriedSenseDataPICO			 = nullptr;
	PFN_xrPollFutureEXT							xrPollFutureEXT						 = nullptr;
	PFN_xrGetSpatialEntityUuidPICO				xrGetSpatialEntityUuidPICO			 = nullptr;
	PFN_xrGetSpatialEntityComponentDataPICO     xrGetSpatialEntityComponentDataPICO	 = nullptr;
	PFN_xrEnumerateSpatialEntityComponentTypesPICO xrEnumerateSpatialEntityComponentTypesPICO = nullptr;

	PFN_xrRetrieveSpatialEntityAnchorPICO		xrRetrieveSpatialEntityAnchorPICO	= nullptr;
	PFN_xrDestroyAnchorPICO						xrDestroyAnchorPICO					= nullptr;
	PFN_xrGetAnchorUuidPICO						xrGetAnchorUuidPICO					= nullptr;
	PFN_xrLocateAnchorPICO						xrLocateAnchorPICO					= nullptr;

	PFN_xrReleaseEnvironmentTextureImagePICO xrReleaseEnvironmentTextureImagePICO = nullptr;

protected:
	EProviderTypePICO GetProviderTypeByHandle(const XrSenseDataProviderPICO& Handle);
	uint64 GetUUID();

private:
	void ApplicationResumeDelegate();
	void PollEvent(const XrEventDataBuffer* EventData);
	void PXR_PollFuture();
	bool HasValidTrackingSystem() const { return XRTrackingSystem != nullptr; }
	void InitializeCachedLightEstimateInfoPico();
	const XrSpatialEntityLightEstimationDataPICO& GetCachedLightEstimateInfoPico()  const  { return CachedLightEstimateInfoPico; }
#if PLATFORM_ANDROID
	const XrLightEstimationInfoEnvironmentTextureVulkanPICO& GetCachedLightEstimationTextureVulkan()  const  { return CachedLightEstimationTextureVulkan; }
#endif
	
	void UpdateCachedLightEstimateInfoPico(const XrSpatialEntityLightEstimationDataPICO& EntityLightEstimationInfo);

	
	TQueue<FFutureMessagePICO> FutureQueueForProviders;
	TMap<uint64_t, FPICOPollFutureDelegate> FutureToDelegateMap;
	
	class IXRTrackingSystem* XRTrackingSystem = nullptr;
	XrTime CurrentDisplayTime;
	XrSession Session = XR_NULL_HANDLE;
	XrInstance Instance = XR_NULL_HANDLE;
	XrSpace TrackingSpace = XR_NULL_HANDLE;
	
	bool bSupportsSpatialSensingEXT = false;
	bool bSupportsSpatialSensing = false;

	uint64 GlobalUUIDCount;
	FRWLock DestroyLock;

	XrSpatialEntityLightEstimationDataPICO CachedLightEstimateInfoPico={};
#if PLATFORM_ANDROID
	XrLightEstimationInfoEnvironmentTextureVulkanPICO CachedLightEstimationTextureVulkan = {};
#endif
};

class FMixedRealityPICO
{
public:
	FMixedRealityPICO();
	virtual ~FMixedRealityPICO() {}

	virtual void OnDestroySession(XrSession InSession);
	virtual void PostCreateSession(XrSession InSession);
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace InTrackingSpace);
	
	virtual bool CreateProvider(const FSenseDataProviderCreateInfoBasePICO& CreateInfo,EResultPICO& OutResult) =0;
	virtual bool StartProvider(const FPICOPollFutureDelegate& StartSenseDataProviderDelegate,EResultPICO& OutResult);
	virtual bool StopProvider(EResultPICO& OutResult);
	virtual bool DestroyProvider(EResultPICO& OutResult);
	virtual EMRStatePICO GetSenseDataProviderState() const;

	bool IsHandleValid() const { return ProviderHandle!=XR_NULL_HANDLE; }
	bool IsInitialized() const { return GetSenseDataProviderState() == EMRStatePICO::Initialized; }
	bool IsRunning() const { return GetSenseDataProviderState() == EMRStatePICO::Running; }
	bool IsEqualProvider(const XrSenseDataProviderPICO& Handle) const { return ProviderHandle==Handle;}
protected:
	virtual EProviderTypePICO GetProviderType();
	bool HasValidTrackingContext() const { return XRTrackingSystem != nullptr && Session != XR_NULL_HANDLE && TrackingSpace != XR_NULL_HANDLE; }
	
	EProviderTypePICO Type=EProviderTypePICO::Pico_Provider_Unknown;
	XrSenseDataProviderPICO ProviderHandle=XR_NULL_HANDLE;
	
	class IXRTrackingSystem* XRTrackingSystem = nullptr;
	XrTime CurrentDisplayTime;
	XrSession Session = XR_NULL_HANDLE;
	XrInstance Instance = XR_NULL_HANDLE;
	XrSpace TrackingSpace = XR_NULL_HANDLE;
};