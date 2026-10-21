// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.


#include "PICO_SpatialPlane.h"
#include "PICO_MRModule.h"
#include "IOpenXRHMDModule.h"

#include "IXRTrackingSystem.h"
#include "OpenXRCore.h"
#include "Engine/Engine.h"

FSpatialPlaneExtensionPICO::FSpatialPlaneExtensionPICO()
{
}

FSpatialPlaneExtensionPICO* FSpatialPlaneExtensionPICO::GetInstance()
{
	return &FPICOOpenXRMRModule::Get().GetMRModuleExtension().GetSpatialPlanePICO();
}

void FSpatialPlaneExtensionPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_PICO_SPATIAL_PLANE_EXTENSION_NAME);
}

void FSpatialPlaneExtensionPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	if (FSpatialSensingExtensionPICO::GetInstance()->IsSupportsSpatialSensing())
	{
		bsupportsSpatialPlaneEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_SPATIAL_PLANE_EXTENSION_NAME);
		UE_LOG(LogMRPICO, Verbose, TEXT("bsupportsSpatialSensingEXT:%d"), bsupportsSpatialPlaneEXT)
		if (bsupportsSpatialPlaneEXT)
		{
			XrSystemSpatialPlanePropertiesPICO SpatialPlanePropertiesPICO = { (XrStructureType)XR_TYPE_SYSTEM_SPATIAL_PLANE_PROPERTIES_PICO };
			XrSystemProperties SPSpatialPlane{ XR_TYPE_SYSTEM_PROPERTIES,&SpatialPlanePropertiesPICO };
			XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &SPSpatialPlane));
			bsupportsSpatialPlane = SpatialPlanePropertiesPICO.supportsSpatialPlane == XR_TRUE;
			UE_LOG(LogMRPICO,Log, TEXT("bsupportsSpatialPlane:%d"), bsupportsSpatialPlane)
		}
	}
}

void FSpatialPlaneExtensionPICO::PostCreateSession(XrSession InSession)
{
	FMixedRealityPICO::PostCreateSession(InSession);
	const UPICOOpenXRRuntimeSettings* Settings = GetDefault<UPICOOpenXRRuntimeSettings>();

	if (Settings->bEnablePlane)
	{
		FSenseDataProviderCreateInfoPlanePICO cFPICOSenseDataProviderCreateInfoPlane = {};
		
		EResultPICO OutResult=EResultPICO::XR_Error_Unknown_PICO;
		if(CreateProvider(cFPICOSenseDataProviderCreateInfoPlane,OutResult))
		{
			UE_LOG(LogMRPICO, Log, TEXT("OnBeginSession CreateSpatialPlaneProvider Success!!"));
		}
		else
		{
			UE_LOG(LogMRPICO, Error, TEXT("OnBeginSession CreateSpatialPlaneProvider failed OutResult:%d"),OutResult);
		}
	}
}

void FSpatialPlaneExtensionPICO::OnDestroySession(XrSession InSession)
{
	FMixedRealityPICO::OnDestroySession(InSession);
	ClearPlaneProviderBuffer();
}

bool FSpatialPlaneExtensionPICO::CreateProvider(const FSenseDataProviderCreateInfoBasePICO& createInfo,EResultPICO& OutResult)
{
	check(createInfo.Type == EProviderTypePICO::Pico_Provider_Plane);
	bool bResult = false;
	if (bsupportsSpatialPlane)
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
		Type = EProviderTypePICO::Pico_Provider_Plane;
		XrSenseDataProviderCreateInfoSpatialPlanePICO cSpatialPlaneProviderCreateInfoBD = {};
		cSpatialPlaneProviderCreateInfoBD.type = XR_TYPE_SENSE_DATA_PROVIDER_CREATE_INFO_SPATIAL_PLANE_PICO;
		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrCreateSenseDataProviderPICO(Session, reinterpret_cast<XrSenseDataProviderCreateInfoBaseHeaderPICO*>(&cSpatialPlaneProviderCreateInfoBD), &ProviderHandle);
		bResult = XR_SUCCEEDED(xrResult);

		ProviderHandle = bResult ? ProviderHandle:XR_NULL_HANDLE;
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
	}

	return bResult;
}

bool FSpatialPlaneExtensionPICO::GetSpatialTrianglePlaneInfos(const XrFutureEXT& FutureHandle,
                                                            TArray<FSpatialPlaneInfoPICO>& PlaneInfos, EResultPICO& OutResult)
{
	if (bsupportsSpatialPlane)
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

		PlaneInfos.Empty();
		CurrentUUIDSet.Empty();

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
			FSpatialPlaneInfoPICO cFPICOMRPlaneInfo;
			cFPICOMRPlaneInfo.UUID = EntityInfo.uuid.data;
			cFPICOMRPlaneInfo.UpdateTime = static_cast<int64>(EntityInfo.lastUpdateTime);

			if (IsContainsInLastUpdate(EntityInfo.uuid.data))
			{
				if (GetLastUpdateTimeByUUID(EntityInfo.uuid.data) >= EntityInfo.lastUpdateTime)
				{
					CachedUUIDToMRPlaneInfoMap[EntityInfo.uuid.data].State = ESpatialMeshStatePICO::Stable;
					continue;
				}
				cFPICOMRPlaneInfo.State = ESpatialMeshStatePICO::Updated;
			}
			else
			{
				cFPICOMRPlaneInfo.State = ESpatialMeshStatePICO::Added;
			}

			if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityPlaneOrientation(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, cFPICOMRPlaneInfo.PlaneOrientation,OutResult))
			{
				UE_LOG(LogMRPICO, Error, TEXT("GetSpatialPlaneOrientation Failed OutResult:%d"),OutResult);
				continue;
			}
			
			if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityLocation(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, cFPICOMRPlaneInfo.PlanePose,OutResult))
			{
				UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityLocation Failed OutResult:%d"),OutResult);
				continue;
			}
			TArray<ESemanticLabelPICO> Semantics;
			if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntitySemantic(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, Semantics,OutResult))
			{
				UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntitySemantic Failed OutResult:%d"),OutResult);
				continue;
			}
			if (!Semantics.IsEmpty())
			{
				cFPICOMRPlaneInfo.Semantic =Semantics[0];
			}

			if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityTriangleMesh(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, cFPICOMRPlaneInfo.Vertices, Triangles,OutResult))
			{
				UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityTriangleMesh Failed OutResult:%d"),OutResult);
				continue;
			}

			if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityBoundary2D(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, cFPICOMRPlaneInfo.BoundingBox,OutResult))
			{
				UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityBoundary3D Failed OutResult:%d"),OutResult);
				continue;
			}

			cFPICOMRPlaneInfo.Indices.SetNum(Triangles.Num());
			cFPICOMRPlaneInfo.Indices = static_cast<TArray<int32>>(Triangles);

			CachedUUIDToMRPlaneInfoMap.Emplace(EntityInfo.uuid.data, MoveTemp(cFPICOMRPlaneInfo));
		}

		for (auto MRPlaneInfo : CachedUUIDToMRPlaneInfoMap)
		{
			if (!CurrentUUIDSet.Contains(MRPlaneInfo.Key))
			{
				CachedUUIDToMRPlaneInfoMap[MRPlaneInfo.Key].State = ESpatialMeshStatePICO::Removed;
				RemovedUUIDSet.Add(MRPlaneInfo.Key);
			}
		}

		CachedUUIDToMRPlaneInfoMap.GenerateValueArray(PlaneInfos);

		for (auto RemovedUUID : RemovedUUIDSet)
		{
			CachedUUIDToMRPlaneInfoMap.Remove(RemovedUUID);
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

void FSpatialPlaneExtensionPICO::ClearPlaneProviderBuffer()
{
	FScopeLock Lock(&CriticalSection);
	CachedUUIDToMRPlaneInfoMap.Empty();
}

bool FSpatialPlaneExtensionPICO::IsContainsInLastUpdate(const FSpatialUUIDPICO& UUID)
{
	return CachedUUIDToMRPlaneInfoMap.Contains(UUID);
}

void FSpatialPlaneExtensionPICO::SetLastUUIDToMRPlaneInfoMap(const TMap<FSpatialUUIDPICO, FSpatialPlaneInfoPICO>& UUIDToMRPlaneInfoMap)
{
	FScopeLock Lock(&CriticalSection);
	CachedUUIDToMRPlaneInfoMap = UUIDToMRPlaneInfoMap;
}

int64_t FSpatialPlaneExtensionPICO::GetLastUpdateTimeByUUID(const FSpatialUUIDPICO& UUID)
{
	if (IsContainsInLastUpdate(UUID))
	{
		return CachedUUIDToMRPlaneInfoMap[UUID].UpdateTime;
	}

	return 0;
}


bool FSpatialPlaneExtensionPICO::RequestSpatialTrianglePlane(const FPICOPollFutureDelegate& Delegate, EResultPICO& Result)
{
	bool bResult = false;
	if (bsupportsSpatialPlane)
	{
		XrFutureEXT Handle;
		XrSenseDataQueryInfoPICO cSenseDataQueryInfoBD = {};
		cSenseDataQueryInfoBD.type = XR_TYPE_SENSE_DATA_QUERY_INFO_PICO;
		cSenseDataQueryInfoBD.filter = nullptr;
		UE_LOG(LogMRPICO, VeryVerbose, TEXT("RequestSpatialTrianglePlane ProviderHandle:%lld"), reinterpret_cast<uint64_t>(ProviderHandle));

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
