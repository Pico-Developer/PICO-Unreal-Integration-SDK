// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "PICO_AnchorComponent.h"
#include "PICO_MR.h"

// Forward declaration
class FPICOOpenXRMRModule;

class FSpatialAnchorExtensionPICO : public FMixedRealityPICO
{
public:
	FSpatialAnchorExtensionPICO();
	virtual ~FSpatialAnchorExtensionPICO() override {}
	
	void GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions);
	void GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions);
	void PostGetSystem(XrInstance InInstance, XrSystemId InSystem);
	virtual void PostCreateSession(XrSession InSession) override;
	
	static FSpatialAnchorExtensionPICO* GetInstance();

	virtual bool CreateProvider(const FSenseDataProviderCreateInfoBasePICO& CreateInfo,EResultPICO& OutResult) override;
	
	bool CreateSpatialAnchorAsync(const FPICOPollFutureDelegate& Delegate,const FTransform& InAnchorEntityTransform,EResultPICO& OutResult);
	bool PersistSpatialAnchorAsync(AActor* BoundActor, EPersistLocationPICO PersistLocation, const FPICOPollFutureDelegate& Delegate,EResultPICO& OutResult);
	bool UnpersistSpatialAnchorAsync(AActor* BoundActor,const FPICOPollFutureDelegate& Delegate,EResultPICO& OutResult);
	bool ShareSpatialAnchorAsync(AActor* BoundActor,const FPICOPollFutureDelegate& Delegate,EResultPICO& OutResult);
	bool LoadAnchorEntityAsync(const FAnchorLoadInfoPICO& LoadInfo, const FPICOPollFutureDelegate& Delegate,EResultPICO& OutResult);
	bool DownloadSharedSpatialAnchorsAsync(const FAnchorLoadInfoPICO& LoadInfo,const FPICOPollFutureDelegate& Delegate,TSet<XrFutureEXT>& HandleSet,EResultPICO& OutResult);
	
	bool CreateSpatialAnchorComplete(const XrFutureEXT& FutureHandle, FSpatialAnchorCreateCompletionPICO& completion,EResultPICO& OutResult);
	bool PersistSpatialAnchorComplete(const XrFutureEXT& FutureHandle, FSpatialAnchorPersistCompletionPICO& completion,EResultPICO& OutResult);
	bool UnpersistSpatialAnchorComplete(const XrFutureEXT& FutureHandle, FSpatialAnchorUnpersistCompletionPICO& completion,EResultPICO& OutResult);
	bool ShareSpatialAnchorComplete(const XrFutureEXT& FutureHandle, FSpatialAnchorShareCompletionPICO& completion,EResultPICO& OutResult);
	bool DownloadSharedSpatialAnchorsComplete(const XrFutureEXT& FutureHandle, FSharedSpatialAnchorDownloadCompletionPICO& completion,EResultPICO& OutResult);

	bool GetAnchorLoadResults(const XrFutureEXT& FutureHandle,TArray<FAnchorLoadResultPICO>& LoadResult,EResultPICO& OutResult);

	bool DestroyAnchorByHandle(const FSpatialHandlePICO& AnchorHandle,EResultPICO& OutResult);
	bool GetAnchorEntityUUID(const UAnchorComponentPICO* AnchorComponent, FSpatialUUIDPICO& OutAnchorUUID,EResultPICO& OutResult);
	bool GetAnchorPose(UAnchorComponentPICO* AnchorComponent, FTransform& OutAnchorPose,EResultPICO& OutResult);
	bool UpdateAnchor(UAnchorComponentPICO* AnchorComponent);

private:
	// XR_PICO_spatial_anchor
	PFN_xrCreateSpatialAnchorAsyncPICO          xrCreateSpatialAnchorAsyncPICO		 = nullptr;
	PFN_xrCreateSpatialAnchorCompletePICO       xrCreateSpatialAnchorCompletePICO	 = nullptr;
	PFN_xrPersistSpatialAnchorAsyncPICO			xrPersistSpatialAnchorAsyncPICO		 = nullptr;
	PFN_xrPersistSpatialAnchorCompletePICO		xrPersistSpatialAnchorCompletePICO	 = nullptr;
	PFN_xrUnpersistSpatialAnchorAsyncPICO		xrUnpersistSpatialAnchorAsyncPICO	 = nullptr;
	PFN_xrUnpersistSpatialAnchorCompletePICO	xrUnpersistSpatialAnchorCompletePICO = nullptr;

	// XR_PICO_spatial_anchor_sharing
	PFN_xrShareSpatialAnchorAsyncPICO            xrShareSpatialAnchorAsyncPICO		 = nullptr;
	PFN_xrShareSpatialAnchorCompletePICO            xrShareSpatialAnchorCompletePICO		 = nullptr;
	PFN_xrDownloadSharedSpatialAnchorAsyncPICO    xrDownloadSharedSpatialAnchorAsyncPICO	 = nullptr;
	PFN_xrDownloadSharedSpatialAnchorCompletePICO    xrDownloadSharedSpatialAnchorCompletePICO	 = nullptr;
	

	
	bool IsAnchorValid(AActor* BoundActor);
	bool IsAnchorValid(const UAnchorComponentPICO* AnchorComponent);
	
	UAnchorComponentPICO* GetAnchorComponent(AActor* BoundActor);

	TMap<FSpatialUUIDPICO,FSpatialHandlePICO> UUIDToAnchorHandleMap;

	bool bSupportsSpatialAnchorEXT = false;
	bool bSupportsSpatialAnchor = false;

	bool bSupportsSpatialAnchorSharingEXT = false;
	bool bSupportsSpatialAnchorSharing = false;

};
