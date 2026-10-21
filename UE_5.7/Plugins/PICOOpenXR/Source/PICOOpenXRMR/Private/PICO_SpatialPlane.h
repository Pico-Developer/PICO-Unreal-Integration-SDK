// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_MR.h"

// Forward declaration
class FPICOOpenXRMRModule;

class FSpatialPlaneExtensionPICO : public FMixedRealityPICO
{
public:
	FSpatialPlaneExtensionPICO();
	virtual ~FSpatialPlaneExtensionPICO() override {}

	void GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions);
	void PostGetSystem(XrInstance InInstance, XrSystemId InSystem);
	virtual void PostCreateSession(XrSession InSession) override;

	virtual void OnDestroySession(XrSession InSession) override;
	
	static FSpatialPlaneExtensionPICO* GetInstance();

	virtual bool CreateProvider(const FSenseDataProviderCreateInfoBasePICO& CreateInfo,EResultPICO& OutResult) override;
	
	bool RequestSpatialTrianglePlane(const FPICOPollFutureDelegate& Delegate,EResultPICO& OutResult);
	bool GetSpatialTrianglePlaneInfos(const XrFutureEXT& FutureHandle,
		TArray<FSpatialPlaneInfoPICO>& PlaneInfos,EResultPICO& OutResult);
	
	void ClearPlaneProviderBuffer();
	
private:
	bool IsContainsInLastUpdate(const FSpatialUUIDPICO& UUID);
	void SetLastUUIDToMRPlaneInfoMap(const TMap<FSpatialUUIDPICO,FSpatialPlaneInfoPICO>& UUIDToMRPlaneInfoMap);
	int64_t GetLastUpdateTimeByUUID(const FSpatialUUIDPICO& UUID);
	
	TMap<FSpatialUUIDPICO, FSpatialPlaneInfoPICO> CachedUUIDToMRPlaneInfoMap;

	mutable FCriticalSection CriticalSection;

	bool bsupportsSpatialPlaneEXT = false;
	bool bsupportsSpatialPlane = false;
};
