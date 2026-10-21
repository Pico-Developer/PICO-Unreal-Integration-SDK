// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "PICO_MR.h"

// Forward declaration
class FPICOOpenXRMRModule;

class FSceneCaptureExtensionPICO :public FMixedRealityPICO
{
public:
	FSceneCaptureExtensionPICO();
	virtual ~FSceneCaptureExtensionPICO() override {}

	static FSceneCaptureExtensionPICO* GetInstance();
	
	virtual bool CreateProvider(const FSenseDataProviderCreateInfoBasePICO& CreateInfo,EResultPICO& OutResult) override;

	void GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions);
	void PostGetSystem(XrInstance InInstance, XrSystemId InSystem);
	virtual void PostCreateSession(XrSession InSession) override;
	
	bool RequestSpatialSceneInfos(const FSceneLoadInfoPICO& LoadInfo, const FPICOPollFutureDelegate& Delegate,EResultPICO& OutResult);
	bool GetSpatialSceneInfos(const XrFutureEXT& FutureHandle,TArray<FMRSceneInfoPICO>& SceneLoadInfos,EResultPICO& OutResult);
	bool StartSceneCaptureAsync(const FPICOPollFutureDelegate& Delegate,EResultPICO& OutResult);
	bool StartSceneCaptureComplete(const XrFutureEXT& FutureHandle,FSceneCaptureStartCompletionPICO& cPICOSceneCaptureStartCompletion,EResultPICO& OutResult);

	void ClearComponentBuffer();

	bool GetSpatialSceneBoundingBox3D(const FSpatialUUIDPICO& UUID, FBoundingBox3DPICO& Box);
	bool GetSpatialSceneBoundingBox2D(const FSpatialUUIDPICO& UUID, FBoundingBox2DPICO& Box2D);
	bool GetSpatialSceneBoundingPolygon(const FSpatialUUIDPICO& UUID, TArray<FVector>& Polygon);
	
private:
	// XR_PICO_scene_capture
	PFN_xrStartSceneCaptureAsyncPICO			xrStartSceneCaptureAsyncPICO		= nullptr;
	PFN_xrStartSceneCaptureCompletePICO			xrStartSceneCaptureCompletePICO		= nullptr;
	
	TMap<FSpatialUUIDPICO, TSharedRef<FBoundingBox3DPICO>> EntityToBoundingBox3DMap;
	TMap<FSpatialUUIDPICO, TSharedRef<FBoundingBox2DPICO>> EntityToBoundingBox2DMap;
	TMap<FSpatialUUIDPICO,TSharedRef<TArray<FVector>>> EntityToPolygonMap;

	bool bSupportsSceneCaptureEXT = false;
	bool bSupportsSceneCapture = false;

};
