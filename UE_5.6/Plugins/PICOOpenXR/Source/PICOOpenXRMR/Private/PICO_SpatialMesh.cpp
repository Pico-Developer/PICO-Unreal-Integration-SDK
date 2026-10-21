// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.


#include "PICO_SpatialMesh.h"
#include "PICO_MRModule.h"
#include "IOpenXRHMDModule.h"

#include "IXRTrackingSystem.h"
#include "OpenXRCore.h"
#include "Engine/Engine.h"

FSpatialMeshExtensionPICO::FSpatialMeshExtensionPICO(): CurrentLod(ESpatialMeshLodPICO::High)
{
}

FSpatialMeshExtensionPICO* FSpatialMeshExtensionPICO::GetInstance()
{
	return &FPICOOpenXRMRModule::Get().GetMRModuleExtension().GetSpatialMeshPICO();
}

void FSpatialMeshExtensionPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_PICO_SPATIAL_MESH_EXTENSION_NAME);
}

void FSpatialMeshExtensionPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	if (FSpatialSensingExtensionPICO::GetInstance()->IsSupportsSpatialSensing())
	{
		bsupportsSpatialMeshEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_SPATIAL_MESH_EXTENSION_NAME);
		UE_LOG(LogMRPICO, Verbose, TEXT("bsupportsSpatialSensingEXT:%d"), bsupportsSpatialMeshEXT)
		if (bsupportsSpatialMeshEXT)
		{
			XrSystemSpatialMeshPropertiesPICO SpatialMeshPropertiesPICO = { (XrStructureType)XR_TYPE_SYSTEM_SPATIAL_MESH_PROPERTIES_PICO };
			XrSystemProperties SPSpatialMesh{ XR_TYPE_SYSTEM_PROPERTIES,&SpatialMeshPropertiesPICO };
			XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &SPSpatialMesh));
			bsupportsSpatialMesh = SpatialMeshPropertiesPICO.supportsSpatialMesh == XR_TRUE;
			UE_LOG(LogMRPICO,Verbose, TEXT("bsupportsSpatialMesh:%d"), bsupportsSpatialMesh)
		}
	}
}

void FSpatialMeshExtensionPICO::PostCreateSession(XrSession InSession)
{
	FMixedRealityPICO::PostCreateSession(InSession);
	const UPICOOpenXRRuntimeSettings* Settings = GetDefault<UPICOOpenXRRuntimeSettings>();

	if (Settings->bEnableMesh)
	{
		FSenseDataProviderCreateInfoMeshPICO cFPICOSenseDataProviderCreateInfoMesh = {};
		cFPICOSenseDataProviderCreateInfoMesh.Lod = Settings->MeshLod;
		if (Settings->bSemanticsAlignWithTriangle)
		{
			cFPICOSenseDataProviderCreateInfoMesh.ConfigArray.Add(ESpatialMeshConfigPICO::Spatial_Mesh_Config_Semantic);
		}
		else if (Settings->bSemanticsAlignWithVertex)
		{
			cFPICOSenseDataProviderCreateInfoMesh.ConfigArray.Add(ESpatialMeshConfigPICO::Spatial_Mesh_Config_Semantic);
			cFPICOSenseDataProviderCreateInfoMesh.ConfigArray.Add(ESpatialMeshConfigPICO::Spatial_Mesh_Config_Semantic_Align_With_Vertex);
		}
		EResultPICO OutResult=EResultPICO::XR_Error_Unknown_PICO;
		if(CreateProvider(cFPICOSenseDataProviderCreateInfoMesh,OutResult))
		{
			UE_LOG(LogMRPICO, Log, TEXT("OnBeginSession CreateSpatialMeshProvider Success!!"));
		}
		else
		{
			UE_LOG(LogMRPICO, Error, TEXT("OnBeginSession CreateSpatialMeshProvider failed OutResult:%d"),OutResult);
		}
	}
}

void FSpatialMeshExtensionPICO::OnDestroySession(XrSession InSession)
{
	FMixedRealityPICO::OnDestroySession(InSession);
	ClearMeshProviderBuffer();

}

bool FSpatialMeshExtensionPICO::CreateProvider(const FSenseDataProviderCreateInfoBasePICO& createInfo,EResultPICO& OutResult)
{
	check(createInfo.Type == EProviderTypePICO::Pico_Provider_Mesh);
	bool bResult = false;
	if (bsupportsSpatialMesh)
	{
		if (Session == XR_NULL_HANDLE)
		{
			OutResult = EResultPICO::XR_Error_ValidationFailure;
			return false;
		}

		if (IsHandleValid())
		{
			UE_LOG(LogMRPICO, Error, TEXT("CreateProvider failed already create"));
			OutResult=EResultPICO::XR_Error_LimitReached;
			return false;
		}
		Type = EProviderTypePICO::Pico_Provider_Mesh;
		const FSenseDataProviderCreateInfoMeshPICO& MeshCreateInfo = static_cast<const FSenseDataProviderCreateInfoMeshPICO&>(createInfo);
		XrSenseDataProviderCreateInfoSpatialMeshPICO cSpatialMeshProviderCreateInfoBD = {};
		cSpatialMeshProviderCreateInfoBD.type = XR_TYPE_SENSE_DATA_PROVIDER_CREATE_INFO_SPATIAL_MESH_PICO;

		cSpatialMeshProviderCreateInfoBD.lod = static_cast<XrSpatialMeshLodPICO>(MeshCreateInfo.Lod);
		CurrentLod = MeshCreateInfo.Lod;
		for (auto Config : MeshCreateInfo.ConfigArray)
		{
			cSpatialMeshProviderCreateInfoBD.configFlags |= static_cast<uint64_t>(Config);
		}

		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrCreateSenseDataProviderPICO(Session, reinterpret_cast<XrSenseDataProviderCreateInfoBaseHeaderPICO*>(&cSpatialMeshProviderCreateInfoBD), &ProviderHandle);
		bResult = XR_SUCCEEDED(xrResult);

		ProviderHandle = bResult ? ProviderHandle:XR_NULL_HANDLE;
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
	}

	return bResult;
}

bool FSpatialMeshExtensionPICO::GetSpatialTriangleMeshInfos(const XrFutureEXT& FutureHandle,
                                                            TArray<FSpatialMeshInfoPICO>& MeshInfos, EResultPICO& OutResult)
{
	if (bsupportsSpatialMesh)
	{
		FScopeLock Lock(&CriticalSection);
		FSenseDataQueryCompletionPICO SenseDataQueryCompletion;
		if (!FSpatialSensingExtensionPICO::GetInstance()->QuerySenseDataComplete(ProviderHandle,FutureHandle, SenseDataQueryCompletion,OutResult))
		{
			return false;
		}

		if (PXR_FAILURE(SenseDataQueryCompletion.FutureResult))
		{
			OutResult = SenseDataQueryCompletion.FutureResult;
			return false;
		}

		if (SenseDataQueryCompletion.SnapShotHandle==XR_NULL_HANDLE)
		{
			OutResult =EResultPICO::XR_Error_HandleInvalid;
			return false;
		}

		
		FQueriedSenseDataPICO QueriedSenseData;
		if (!FSpatialSensingExtensionPICO::GetInstance()->GetQueriedSenseData(ProviderHandle,SenseDataQueryCompletion.SnapShotHandle, QueriedSenseData,OutResult))
		{
			return false;
		}
		
		TSet<FSpatialUUIDPICO> CurrentUUIDSet;
		TSet<FSpatialUUIDPICO> RemovedUUIDSet;

		MeshInfos.Empty();
		CurrentUUIDSet.Empty();

		UE_LOG(LogMRPICO, VeryVerbose, TEXT("FSpatialMeshExtensionPICO::GetSpatialTriangleMeshInfos:%d"),QueriedSenseData.QueriedSpatialEntityInfos.Num());
		int32 CountAfterDiff = 0;
		for (auto EntityInfo : QueriedSenseData.QueriedSpatialEntityInfos)
		{
			if (!EntityInfo.entity)
			{
				continue;
			}

			CurrentUUIDSet.Add(EntityInfo.uuid.data);
			CountAfterDiff++;
			TArray<uint16> Triangles;
			FSpatialMeshInfoPICO cFPICOMRMeshInfo;
			cFPICOMRMeshInfo.UUID = EntityInfo.uuid.data;
			cFPICOMRMeshInfo.UpdateTime = static_cast<int64>(EntityInfo.lastUpdateTime);

			if (IsContainsInLastUpdate(EntityInfo.uuid.data))
			{
				if (GetLastUpdateTimeByUUID(EntityInfo.uuid.data) >= EntityInfo.lastUpdateTime)
				{
					CachedUUIDToMRMeshInfoMap[EntityInfo.uuid.data].State = ESpatialMeshStatePICO::Stable;
					continue;
				}
				cFPICOMRMeshInfo.State = ESpatialMeshStatePICO::Updated;
			}
			else
			{
				cFPICOMRMeshInfo.State = ESpatialMeshStatePICO::Added;
			}

			if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityLocation(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, cFPICOMRMeshInfo.MeshPose,OutResult))
			{
				UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityLocation Failed OutResult:%d"),OutResult);
				continue;
			}

			const UPICOOpenXRRuntimeSettings* Settings = GetDefault<UPICOOpenXRRuntimeSettings>();
			if (Settings->bSemanticsAlignWithTriangle || Settings->bSemanticsAlignWithVertex)
			{
				if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntitySemantic(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, cFPICOMRMeshInfo.Semantics,OutResult))
				{
					UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntitySemantic Failed OutResult:%d"),OutResult);
					continue;
				}
			}

			if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityTriangleMesh(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, cFPICOMRMeshInfo.Vertices, Triangles,OutResult))
			{
				UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityTriangleMesh Failed OutResult:%d"),OutResult);
				continue;
			}

			if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityBoundary3D(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, cFPICOMRMeshInfo.BoundingBox,OutResult))
			{
				UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityBoundary3D Failed OutResult:%d"),OutResult);
				continue;
			}

			cFPICOMRMeshInfo.Indices.SetNum(Triangles.Num());
			cFPICOMRMeshInfo.Indices = static_cast<TArray<int32>>(Triangles);

			CachedUUIDToMRMeshInfoMap.Emplace(EntityInfo.uuid.data, MoveTemp(cFPICOMRMeshInfo));
		}

		for (auto MRMeshInfo : CachedUUIDToMRMeshInfoMap)
		{
			if (!CurrentUUIDSet.Contains(MRMeshInfo.Key))
			{
				CachedUUIDToMRMeshInfoMap[MRMeshInfo.Key].State = ESpatialMeshStatePICO::Removed;
				RemovedUUIDSet.Add(MRMeshInfo.Key);
			}
		}

		CachedUUIDToMRMeshInfoMap.GenerateValueArray(MeshInfos);

		for (auto RemovedUUID : RemovedUUIDSet)
		{
			CachedUUIDToMRMeshInfoMap.Remove(RemovedUUID);
		}

		if (!FSpatialSensingExtensionPICO::GetInstance()->DestroySenseDataQueryResult(SenseDataQueryCompletion.SnapShotHandle,OutResult))
		{
			UE_LOG(LogMRPICO, Error,TEXT("DestroySenseDataQueryResult Failed!"));
			return false;
		}

		return true;
	}
	return false;
}

ESpatialMeshLodPICO FSpatialMeshExtensionPICO::GetCurrentSpatialMeshLod()
{
	return CurrentLod;
}

void FSpatialMeshExtensionPICO::ClearMeshProviderBuffer()
{
	FScopeLock Lock(&CriticalSection);
	CachedUUIDToMRMeshInfoMap.Empty();
}

bool FSpatialMeshExtensionPICO::IsContainsInLastUpdate(const FSpatialUUIDPICO& UUID)
{
	return CachedUUIDToMRMeshInfoMap.Contains(UUID);
}

void FSpatialMeshExtensionPICO::SetLastUUIDToMRMeshInfoMap(const TMap<FSpatialUUIDPICO, FSpatialMeshInfoPICO>& UUIDToMRMeshInfoMap)
{
	FScopeLock Lock(&CriticalSection);
	CachedUUIDToMRMeshInfoMap = UUIDToMRMeshInfoMap;
}

int64_t FSpatialMeshExtensionPICO::GetLastUpdateTimeByUUID(const FSpatialUUIDPICO& UUID)
{
	if (IsContainsInLastUpdate(UUID))
	{
		return CachedUUIDToMRMeshInfoMap[UUID].UpdateTime;
	}

	return 0;
}


bool FSpatialMeshExtensionPICO::RequestSpatialTriangleMesh(const FPICOPollFutureDelegate& Delegate, EResultPICO& Result)
{
	bool bResult = false;
	if (bsupportsSpatialMesh)
	{
		XrFutureEXT Handle;
		XrSenseDataQueryInfoPICO cSenseDataQueryInfoBD = {};
		cSenseDataQueryInfoBD.type = XR_TYPE_SENSE_DATA_QUERY_INFO_PICO;
		cSenseDataQueryInfoBD.filter = nullptr;
		UE_LOG(LogMRPICO, VeryVerbose, TEXT("RequestSpatialTriangleMesh ProviderHandle:%lld"), reinterpret_cast<uint64_t>(ProviderHandle));

		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrQuerySenseDataAsyncPICO(ProviderHandle, &cSenseDataQueryInfoBD, &Handle);
		bResult = XR_SUCCEEDED(xrResult);

		Result = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
		if (bResult)
		{
			bResult = FSpatialSensingExtensionPICO::GetInstance()->AddPollFutureRequirement(Handle, Delegate);
		}
	}

	return bResult;
}
