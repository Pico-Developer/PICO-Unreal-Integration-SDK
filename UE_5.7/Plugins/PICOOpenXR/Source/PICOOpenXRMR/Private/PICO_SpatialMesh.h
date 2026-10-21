// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_MR.h"

// Forward declaration
class FPICOOpenXRMRModule;

class FSpatialMeshExtensionPICO : public FMixedRealityPICO
{
public:
	FSpatialMeshExtensionPICO();
	virtual ~FSpatialMeshExtensionPICO() override {}

	void GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions);
	void PostGetSystem(XrInstance InInstance, XrSystemId InSystem);
	virtual void PostCreateSession(XrSession InSession) override;

	virtual void OnDestroySession(XrSession InSession) override;
	
	static FSpatialMeshExtensionPICO* GetInstance();

	virtual bool CreateProvider(const FSenseDataProviderCreateInfoBasePICO& CreateInfo,EResultPICO& OutResult) override;
	
	bool RequestSpatialTriangleMesh(const FPICOPollFutureDelegate& Delegate,EResultPICO& OutResult);
	bool GetSpatialTriangleMeshInfos(const XrFutureEXT& FutureHandle,
		TArray<FSpatialMeshInfoPICO>& MeshInfos,EResultPICO& OutResult);

	ESpatialMeshLodPICO GetCurrentSpatialMeshLod();
	void ClearMeshProviderBuffer();
	
private:
	bool IsContainsInLastUpdate(const FSpatialUUIDPICO& UUID);
	void SetLastUUIDToMRMeshInfoMap(const TMap<FSpatialUUIDPICO,FSpatialMeshInfoPICO>& UUIDToMRMeshInfoMap);
	int64_t GetLastUpdateTimeByUUID(const FSpatialUUIDPICO& UUID);
	
	TMap<FSpatialUUIDPICO, FSpatialMeshInfoPICO> CachedUUIDToMRMeshInfoMap;

	ESpatialMeshLodPICO CurrentLod;
	mutable FCriticalSection CriticalSection;

	bool bsupportsSpatialMeshEXT = false;
	bool bsupportsSpatialMesh = false;
};
