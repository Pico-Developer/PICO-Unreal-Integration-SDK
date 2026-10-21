// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_SpatialAnchor.h"
#include "PICO_MRModule.h"

#include "IOpenXRHMDModule.h"
#include "OpenXRCore.h"
#include "IXRTrackingSystem.h"
#include "GameFramework/Actor.h"


FSpatialAnchorExtensionPICO::FSpatialAnchorExtensionPICO()
{
}

FSpatialAnchorExtensionPICO* FSpatialAnchorExtensionPICO::GetInstance()
{
	return &FPICOOpenXRMRModule::Get().GetMRModuleExtension().GetSpatialAnchorPICO();
}

void FSpatialAnchorExtensionPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_PICO_SPATIAL_ANCHOR_EXTENSION_NAME);
}

void FSpatialAnchorExtensionPICO::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_PICO_SPATIAL_ANCHOR_SHARING_EXTENSION_NAME);
}

void FSpatialAnchorExtensionPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	if (FSpatialSensingExtensionPICO::GetInstance()->IsSupportsSpatialSensing())
	{
		bSupportsSpatialAnchorEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_SPATIAL_ANCHOR_EXTENSION_NAME);
		UE_LOG(LogMRPICO,Verbose, TEXT("bSupportsSpatialAnchorEXT:%d"), bSupportsSpatialAnchorEXT)

		if (bSupportsSpatialAnchorEXT)
		{
			XrSystemSpatialAnchorPropertiesPICO SpatialAnchorPropertiesPICO = { (XrStructureType)XR_TYPE_SYSTEM_SPATIAL_ANCHOR_PROPERTIES_PICO };
			XrSystemProperties SPSpatialAnchor{ XR_TYPE_SYSTEM_PROPERTIES,&SpatialAnchorPropertiesPICO };
			XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &SPSpatialAnchor));
			bSupportsSpatialAnchor = SpatialAnchorPropertiesPICO.supportsSpatialAnchor == XR_TRUE;
			UE_LOG(LogHMD,Verbose, TEXT("bSupportsSpatialAnchor:%d"), bSupportsSpatialAnchor)

			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateSpatialAnchorAsyncPICO", (PFN_xrVoidFunction*)&xrCreateSpatialAnchorAsyncPICO));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateSpatialAnchorCompletePICO", (PFN_xrVoidFunction*)&xrCreateSpatialAnchorCompletePICO));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrPersistSpatialAnchorAsyncPICO", (PFN_xrVoidFunction*)&xrPersistSpatialAnchorAsyncPICO));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrPersistSpatialAnchorCompletePICO", (PFN_xrVoidFunction*)&xrPersistSpatialAnchorCompletePICO));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrUnpersistSpatialAnchorAsyncPICO", (PFN_xrVoidFunction*)&xrUnpersistSpatialAnchorAsyncPICO));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrUnpersistSpatialAnchorCompletePICO", (PFN_xrVoidFunction*)&xrUnpersistSpatialAnchorCompletePICO));
		}


		bSupportsSpatialAnchorSharingEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_SPATIAL_ANCHOR_SHARING_EXTENSION_NAME);
		UE_LOG(LogMRPICO,Verbose, TEXT("bSupportsSpatialAnchorSharingEXT:%d"), bSupportsSpatialAnchorSharingEXT)

		if (bSupportsSpatialAnchorSharingEXT)
		{
			XrSystemSpatialAnchorSharingPropertiesPICO SpatialAnchorSharingPropertiesPICO = { (XrStructureType)XR_TYPE_SYSTEM_SPATIAL_ANCHOR_SHARING_PROPERTIES_PICO };
			XrSystemProperties SPSpatialAnchor{ XR_TYPE_SYSTEM_PROPERTIES,&SpatialAnchorSharingPropertiesPICO };
			XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &SPSpatialAnchor));
			bSupportsSpatialAnchorSharing = SpatialAnchorSharingPropertiesPICO.supportsSpatialAnchorSharing == XR_TRUE;
			UE_LOG(LogMRPICO,Log, TEXT("bSupportsSpatialAnchorSharing:%d"), bSupportsSpatialAnchorSharing)

			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrShareSpatialAnchorAsyncPICO", (PFN_xrVoidFunction*)&xrShareSpatialAnchorAsyncPICO));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrShareSpatialAnchorCompletePICO", (PFN_xrVoidFunction*)&xrShareSpatialAnchorCompletePICO));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDownloadSharedSpatialAnchorAsyncPICO", (PFN_xrVoidFunction*)&xrDownloadSharedSpatialAnchorAsyncPICO));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDownloadSharedSpatialAnchorCompletePICO", (PFN_xrVoidFunction*)&xrDownloadSharedSpatialAnchorCompletePICO));
		}
		
	}
}

void FSpatialAnchorExtensionPICO::PostCreateSession(XrSession InSession)
{
	FMixedRealityPICO::PostCreateSession(InSession);
	const UPICOOpenXRRuntimeSettings* Settings = GetDefault<UPICOOpenXRRuntimeSettings>();

	if (Settings->bEnableAnchor)
	{
		FSenseDataProviderCreateInfoAnchorPICO cFPICOSenseDataProviderCreateInfoAnchor = {};
		EResultPICO OutResult =EResultPICO::XR_Error_Unknown_PICO;
		if(CreateProvider(cFPICOSenseDataProviderCreateInfoAnchor,OutResult))
		{
			UE_LOG(LogMRPICO, Log, TEXT("OnBeginSession CreateSpatialAnchorProvider Success!!"));
		}
		else
		{
			UE_LOG(LogMRPICO, Error, TEXT("OnBeginSession CreateSpatialAnchorProvider failed OutResult:%d"),OutResult);
		}
	}
}

bool FSpatialAnchorExtensionPICO::CreateSpatialAnchorAsync(const FPICOPollFutureDelegate& Delegate, const FTransform& InAnchorEntityTransform, EResultPICO& OutResult)
{
	bool bResult=false;
	if(bSupportsSpatialAnchor)
	{
		if (!HasValidTrackingContext() || xrCreateSpatialAnchorAsyncPICO == nullptr)
		{
			OutResult = EResultPICO::XR_Error_ValidationFailure;
			return false;
		}

		XrFutureEXT xrFutureEXT;
		XrSpatialAnchorCreateInfoPICO cSpatialAnchorCreateInfoBD = {};
		cSpatialAnchorCreateInfoBD.type = XR_TYPE_SPATIAL_ANCHOR_CREATE_INFO_PICO;
		FTransform TrackingToWorld=XRTrackingSystem->GetTrackingToWorldTransform();

		const FQuat TrackingOrientation = TrackingToWorld.Inverse().TransformRotation(InAnchorEntityTransform.Rotator().Quaternion());
		const FVector TrackingPosition = TrackingToWorld.Inverse().TransformPosition(InAnchorEntityTransform.GetLocation());

		FTransform UnrealPose = FTransform(TrackingOrientation, TrackingPosition);
		float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();

		XrPosef AnchorPose = ToXrPose(UnrealPose, WorldToMetersScale);
		cSpatialAnchorCreateInfoBD.space = TrackingSpace;
		cSpatialAnchorCreateInfoBD.pose = AnchorPose;
		cSpatialAnchorCreateInfoBD.time = CurrentDisplayTime;

		XrResult xrResult = xrCreateSpatialAnchorAsyncPICO(ProviderHandle, &cSpatialAnchorCreateInfoBD, &xrFutureEXT);
		bResult = XR_SUCCEEDED(xrResult);

		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
		if (bResult)
		{
			bResult = FSpatialSensingExtensionPICO::GetInstance()->AddPollFutureRequirement(xrFutureEXT, Delegate);
		}
	}

	return bResult;
}

bool FSpatialAnchorExtensionPICO::PersistSpatialAnchorAsync(AActor* BoundActor, EPersistLocationPICO PersistLocation, const FPICOPollFutureDelegate& Delegate, EResultPICO& OutResult)
{
	bool bResult=false;
	if(bSupportsSpatialAnchor)
	{
		if (!IsAnchorValid(BoundActor))
		{
			return false;
		}

		UAnchorComponentPICO* AnchorComponent = GetAnchorComponent(BoundActor);

		if (!AnchorComponent)
		{
			return false;
		}

		XrFutureEXT xrFutureEXT;

		XrSpatialAnchorPersistInfoPICO SpatialAnchorPersistInfoBD = {};
		SpatialAnchorPersistInfoBD.type = XR_TYPE_SPATIAL_ANCHOR_PERSIST_INFO_PICO;
		SpatialAnchorPersistInfoBD.anchor = reinterpret_cast<XrAnchorPICO>(AnchorComponent->GetAnchorHandle().GetValue());
		SpatialAnchorPersistInfoBD.location = static_cast<XrPersistenceLocationPICO>(PersistLocation);

		XrResult xrResult = xrPersistSpatialAnchorAsyncPICO(ProviderHandle, &SpatialAnchorPersistInfoBD, &xrFutureEXT);
		bResult = XR_SUCCEEDED(xrResult);
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);

		if (bResult)
		{
			bResult = FSpatialSensingExtensionPICO::GetInstance()->AddPollFutureRequirement(xrFutureEXT, Delegate);
		}
	}

	return bResult;
}

bool FSpatialAnchorExtensionPICO::UnpersistSpatialAnchorAsync(AActor* BoundActor, const FPICOPollFutureDelegate& Delegate, EResultPICO& OutResult)
{
	bool bResult=false;
	if(bSupportsSpatialAnchor)
	{
		if (!IsAnchorValid(BoundActor))
		{
			return false;
		}

		UAnchorComponentPICO* AnchorComponent = GetAnchorComponent(BoundActor);

		XrFutureEXT xrFutureEXT;

		XrSpatialAnchorUnpersistInfoPICO SpatialAnchorUnpersistInfoBD = {};
		SpatialAnchorUnpersistInfoBD.type = XR_TYPE_SPATIAL_ANCHOR_UNPERSIST_INFO_PICO;
		SpatialAnchorUnpersistInfoBD.anchor = reinterpret_cast<XrAnchorPICO>(AnchorComponent->GetAnchorHandle().GetValue());
		XrResult xrResult = xrUnpersistSpatialAnchorAsyncPICO(ProviderHandle,&SpatialAnchorUnpersistInfoBD, &xrFutureEXT);

		bResult = XR_SUCCEEDED(xrResult);
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
		if (bResult)
		{
			bResult = FSpatialSensingExtensionPICO::GetInstance()->AddPollFutureRequirement(xrFutureEXT, Delegate);
		}
	}

	return bResult;
}

bool FSpatialAnchorExtensionPICO::ShareSpatialAnchorAsync(AActor* BoundActor, const FPICOPollFutureDelegate& Delegate, EResultPICO& OutResult)
{
	bool bResult=false;
	if (bSupportsSpatialAnchorSharing)
	{
		if (!IsAnchorValid(BoundActor))
		{
			return false;
		}

		UAnchorComponentPICO* AnchorComponent = GetAnchorComponent(BoundActor);
	
		XrFutureEXT xrFutureEXT;
	
		XrSpatialAnchorShareInfoPICO SpatialAnchorShareInfo = {};
		SpatialAnchorShareInfo.type = XR_TYPE_SPATIAL_ANCHOR_SHARE_INFO_PICO;
		SpatialAnchorShareInfo.anchor = reinterpret_cast<XrAnchorPICO>(AnchorComponent->GetAnchorHandle().GetValue());
	
		XrResult xrResult =xrShareSpatialAnchorAsyncPICO(ProviderHandle,&SpatialAnchorShareInfo,&xrFutureEXT);
		bResult = XR_SUCCEEDED(xrResult);

		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
		
		if (bResult)
		{
			bResult = FSpatialSensingExtensionPICO::GetInstance()->AddPollFutureRequirement(xrFutureEXT, Delegate);
		}
	}
	
	return bResult;
}

bool FSpatialAnchorExtensionPICO::DownloadSharedSpatialAnchorsAsync(const FAnchorLoadInfoPICO& LoadInfo, const FPICOPollFutureDelegate& Delegate, TSet<XrFutureEXT>& HandleSet, EResultPICO& OutResult)
{
	bool bResult=false;
	if (bSupportsSpatialAnchorSharing)
	{
		if (LoadInfo.UUIDFilter.Num() > 0)
		{
			for (auto Uuid : LoadInfo.UUIDFilter)
			{
				XrSharedSpatialAnchorDownloadInfoPICO cSenseDataQueryInfoBD = {};
				cSenseDataQueryInfoBD.type = XR_TYPE_SHARED_SPATIAL_ANCHOR_DOWNLOAD_INFO_PICO;

				UE_LOG(LogMRPICO, Log,TEXT("SpatialAnchorShareInfo.uuid:%s"), *Uuid.ToString());
				FMemory::Memcpy(cSenseDataQueryInfoBD.uuid.data, Uuid.UUIDArray);
				XrFutureEXT xrFutureEXT;
				XrResult xrResult =xrDownloadSharedSpatialAnchorAsyncPICO(ProviderHandle,&cSenseDataQueryInfoBD,&xrFutureEXT);
				bResult = XR_SUCCEEDED(xrResult);

				OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);

				if (bResult)
				{
					bResult = FSpatialSensingExtensionPICO::GetInstance()->AddPollFutureRequirement(xrFutureEXT, Delegate);
					HandleSet.Add(xrFutureEXT);
				}
			}
		}
	}
	
	return bResult;
}

bool FSpatialAnchorExtensionPICO::CreateSpatialAnchorComplete(const XrFutureEXT& FutureHandle, FSpatialAnchorCreateCompletionPICO& completion,EResultPICO& OutResult)
{
	bool bResult=false;
	if(bSupportsSpatialAnchor)
	{
		XrSpatialAnchorCreateCompletionPICO SpatialAnchorCreateCompletionBD = {};
		SpatialAnchorCreateCompletionBD.type = XR_TYPE_SPATIAL_ANCHOR_CREATE_COMPLETION_PICO;
		XrResult xrResult = xrCreateSpatialAnchorCompletePICO(ProviderHandle, FutureHandle, &SpatialAnchorCreateCompletionBD);

		completion.AnchorHandle = reinterpret_cast<uint64_t>(SpatialAnchorCreateCompletionBD.anchor);
		completion.UUID = SpatialAnchorCreateCompletionBD.uuid.data;
		bResult = XR_SUCCEEDED(xrResult);
		completion.FutureResult = FSpatialSensingExtensionPICO::CastToPICOResult(SpatialAnchorCreateCompletionBD.futureResult);
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);

	}
	return bResult;
}

bool FSpatialAnchorExtensionPICO::PersistSpatialAnchorComplete(const XrFutureEXT& FutureHandle, FSpatialAnchorPersistCompletionPICO& completion,EResultPICO& OutResult)
{
	bool bResult=false;
	if(bSupportsSpatialAnchor)
	{
		XrSpatialAnchorPersistCompletionPICO SpatialAnchorPersistCompletionBD = {};
		SpatialAnchorPersistCompletionBD.type = XR_TYPE_SPATIAL_ANCHOR_PERSIST_COMPLETION_PICO;
		XrResult xrResult = xrPersistSpatialAnchorCompletePICO(ProviderHandle, FutureHandle,&SpatialAnchorPersistCompletionBD);
		bResult = XR_SUCCEEDED(xrResult);

		completion.AnchorHandle = reinterpret_cast<uint64_t>(SpatialAnchorPersistCompletionBD.anchor);
		completion.UUID = SpatialAnchorPersistCompletionBD.uuid.data;

		completion.FutureResult = FSpatialSensingExtensionPICO::CastToPICOResult(SpatialAnchorPersistCompletionBD.futureResult);
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);

	}
	return bResult;
}

bool FSpatialAnchorExtensionPICO::UnpersistSpatialAnchorComplete(const XrFutureEXT& FutureHandle, FSpatialAnchorUnpersistCompletionPICO& completion,EResultPICO& OutResult)
{
	bool bResult=false;
	if(bSupportsSpatialAnchor)
	{
		XrSpatialAnchorUnpersistCompletionPICO SpatialAnchorUnpersistCompletionBD = {};
		SpatialAnchorUnpersistCompletionBD.type = XR_TYPE_SPATIAL_ANCHOR_UNPERSIST_COMPLETION_PICO;
		XrResult xrResult = xrUnpersistSpatialAnchorCompletePICO(ProviderHandle, FutureHandle, &SpatialAnchorUnpersistCompletionBD);
		bResult = XR_SUCCEEDED(xrResult);

		completion.AnchorHandle = reinterpret_cast<uint64_t>(SpatialAnchorUnpersistCompletionBD.anchor);
		completion.UUID = SpatialAnchorUnpersistCompletionBD.uuid.data;

		completion.FutureResult = FSpatialSensingExtensionPICO::CastToPICOResult(SpatialAnchorUnpersistCompletionBD.futureResult);
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);

	}
	return bResult;
}

bool FSpatialAnchorExtensionPICO::ShareSpatialAnchorComplete(const XrFutureEXT& FutureHandle, FSpatialAnchorShareCompletionPICO& completion,EResultPICO& OutResult)
{
	bool bResult=false;
	if(bSupportsSpatialAnchorSharing)
	{
		XrSpatialAnchorShareCompletionPICO SpatialAnchorShareCompletion = {};
		SpatialAnchorShareCompletion.type = XR_TYPE_SPATIAL_ANCHOR_SHARE_COMPLETION_PICO;
		XrResult xrResult = xrShareSpatialAnchorCompletePICO(ProviderHandle,FutureHandle, &SpatialAnchorShareCompletion);
		UE_LOG(LogMRPICO, Log,TEXT("ShareSpatialAnchorComplete:%d"), SpatialAnchorShareCompletion.futureResult);
		completion.FutureResult = FSpatialSensingExtensionPICO::CastToPICOResult(SpatialAnchorShareCompletion.futureResult);
		bResult = XR_SUCCEEDED(xrResult);
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
	}

	return bResult;
}

bool FSpatialAnchorExtensionPICO::DownloadSharedSpatialAnchorsComplete(const XrFutureEXT& FutureHandle, FSharedSpatialAnchorDownloadCompletionPICO& completion,EResultPICO& OutResult)
{
	bool bResult=false;
	if(bSupportsSpatialAnchorSharing)
	{
		XrSharedSpatialAnchorDownloadCompletionPICO SharedSpatialAnchorDownloadCompletion = {};
		SharedSpatialAnchorDownloadCompletion.type = XR_TYPE_SHARED_SPATIAL_ANCHOR_DOWNLOAD_COMPLETION_PICO;
		XrResult xrResult = xrDownloadSharedSpatialAnchorCompletePICO(ProviderHandle,FutureHandle, &SharedSpatialAnchorDownloadCompletion);
		UE_LOG(LogMRPICO, Log,TEXT("DownloadSharedSpatialAnchorsComplete:%d"), SharedSpatialAnchorDownloadCompletion.futureResult);
		completion.FutureResult = FSpatialSensingExtensionPICO::CastToPICOResult(SharedSpatialAnchorDownloadCompletion.futureResult);
		bResult = XR_SUCCEEDED(xrResult);
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
	}

	return bResult;
}

bool FSpatialAnchorExtensionPICO::LoadAnchorEntityAsync(const FAnchorLoadInfoPICO& LoadInfo, const FPICOPollFutureDelegate& Delegate, EResultPICO& OutResult)
{
	bool bResult=false;
	if(bSupportsSpatialAnchor)
	{
		XrFutureEXT FutureEXTHandle;
		XrSenseDataQueryInfoPICO cSenseDataQueryInfoBD = {};
		cSenseDataQueryInfoBD.type = XR_TYPE_SENSE_DATA_QUERY_INFO_PICO;

		XrSenseDataFilterUuidPICO cPxrSpatialEntityUuidFilterBD = {};
		cPxrSpatialEntityUuidFilterBD.type = XR_TYPE_SENSE_DATA_FILTER_UUID_PICO;
		cPxrSpatialEntityUuidFilterBD.uuidCount = LoadInfo.UUIDFilter.Num();
		TArray<XrUuidEXT> TempPxrUuidArray;
		TempPxrUuidArray.SetNum(cPxrSpatialEntityUuidFilterBD.uuidCount);

		if (cPxrSpatialEntityUuidFilterBD.uuidCount)
		{
			if (cPxrSpatialEntityUuidFilterBD.uuidCount > 0)
			{
				cPxrSpatialEntityUuidFilterBD.uuids = TempPxrUuidArray.GetData();
				for (uint32_t Index = 0; Index < cPxrSpatialEntityUuidFilterBD.uuidCount; ++Index)
				{
					FMemory::Memcpy(cPxrSpatialEntityUuidFilterBD.uuids[Index].data, LoadInfo.UUIDFilter[Index].UUIDArray);
				}
			}
			cSenseDataQueryInfoBD.filter = reinterpret_cast<XrSenseDataFilterBaseHeaderPICO*>(&cPxrSpatialEntityUuidFilterBD);
		}
		else
		{
			cSenseDataQueryInfoBD.filter = nullptr;
		}

		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrQuerySenseDataAsyncPICO(ProviderHandle,&cSenseDataQueryInfoBD,&FutureEXTHandle);
		bResult = XR_SUCCEEDED(xrResult);
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
		if (bResult)
		{
			bResult = FSpatialSensingExtensionPICO::GetInstance()->AddPollFutureRequirement(FutureEXTHandle, Delegate);
		}
	}
	return bResult;
}

bool FSpatialAnchorExtensionPICO::GetAnchorLoadResults(const XrFutureEXT& FutureHandle, TArray<FAnchorLoadResultPICO>& LoadResult, EResultPICO& OutResult)
{
	if(bSupportsSpatialAnchor)
	{
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
		
		for (auto Entity : QueriedSenseData.QueriedSpatialEntityInfos)
		{
			XrSpatialEntityAnchorRetrieveInfoPICO cPxrSpatialEntityAnchorRetrieveInfo = {};
			cPxrSpatialEntityAnchorRetrieveInfo.type = XR_TYPE_SPATIAL_ENTITY_ANCHOR_RETRIEVE_INFO_PICO;
			cPxrSpatialEntityAnchorRetrieveInfo.entity = Entity.entity;
			FAnchorLoadResultPICO cAnchorLoadResult;
			cAnchorLoadResult.PersistLocation = EPersistLocationPICO::PersistLocation_Local;
			cAnchorLoadResult.AnchorUUID = Entity.uuid.data;
			XrAnchorPICO cAnchorPICOHandle;
			XrResult RetrieveResult=FSpatialSensingExtensionPICO::GetInstance()->xrRetrieveSpatialEntityAnchorPICO(SenseDataQueryCompletion.SnapShotHandle, &cPxrSpatialEntityAnchorRetrieveInfo, &cAnchorPICOHandle);
			if (XR_SUCCEEDED(RetrieveResult))
			{
				cAnchorLoadResult.AnchorHandle = reinterpret_cast<uint64_t>(cAnchorPICOHandle);
				LoadResult.Add(cAnchorLoadResult);
			}
			else
			{
				UE_LOG(LogMRPICO, Error, TEXT("xrRetrieveSpatialEntityAnchorPICO Failed! RetrieveResult:%d"),RetrieveResult);
			}
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

bool FSpatialAnchorExtensionPICO::CreateProvider(const FSenseDataProviderCreateInfoBasePICO& createInfo, EResultPICO& OutResult)
{
	check(createInfo.Type == EProviderTypePICO::Pico_Provider_Anchor);
	bool bResult=false;
	if(bSupportsSpatialAnchor)
	{
		if (Session == XR_NULL_HANDLE)
		{
			OutResult = EResultPICO::XR_Error_ValidationFailure;
			return false;
		}

		Type = EProviderTypePICO::Pico_Provider_Anchor;
		if (IsHandleValid())
		{
			UE_LOG(LogMRPICO, Error, TEXT("CreateProvider failed already create"));
			OutResult=EResultPICO::XR_Error_LimitReached;
			return false;
		}
		XrSenseDataProviderCreateInfoSpatialAnchorPICO cSpatialAnchorProviderCreateInfoBD = {};
		cSpatialAnchorProviderCreateInfoBD.type = XR_TYPE_SENSE_DATA_PROVIDER_CREATE_INFO_SPATIAL_ANCHOR_PICO;

		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrCreateSenseDataProviderPICO(Session,reinterpret_cast<XrSenseDataProviderCreateInfoBaseHeaderPICO*>(&cSpatialAnchorProviderCreateInfoBD),&ProviderHandle);
		bResult = XR_SUCCEEDED(xrResult);
		ProviderHandle = bResult ? ProviderHandle:XR_NULL_HANDLE;
		
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
	}
	
	return bResult;
}

bool FSpatialAnchorExtensionPICO::DestroyAnchorByHandle(const FSpatialHandlePICO& AnchorHandle,EResultPICO& OutResult)
{
	bool bResult=false;
	if(bSupportsSpatialAnchor)
	{
		XrAnchorPICO Anchor = reinterpret_cast<XrAnchorPICO>(AnchorHandle.GetValue());
		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrDestroyAnchorPICO(Anchor);
		bResult = XR_SUCCEEDED(xrResult);
		OutResult=FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
	}

	return bResult;
}

bool FSpatialAnchorExtensionPICO::IsAnchorValid(AActor* BoundActor)
{
	if (!BoundActor)
	{
		return false;
	}

	UAnchorComponentPICO* AnchorComponent = Cast<UAnchorComponentPICO>(BoundActor->GetComponentByClass(UAnchorComponentPICO::StaticClass()));
	if (!IsValid(AnchorComponent) || !AnchorComponent->IsAnchorValid())
	{
		return false;
	}
	return true;
}

bool FSpatialAnchorExtensionPICO::IsAnchorValid(const UAnchorComponentPICO* AnchorComponent)
{
	if (!IsValid(AnchorComponent) || !AnchorComponent->IsAnchorValid())
	{
		return false;
	}
	return true;
}


UAnchorComponentPICO* FSpatialAnchorExtensionPICO::GetAnchorComponent(AActor* BoundActor)
{
	if (!IsAnchorValid(BoundActor))
	{
		return nullptr;
	}

	UAnchorComponentPICO* AnchorComponent = Cast<UAnchorComponentPICO>(BoundActor->GetComponentByClass(UAnchorComponentPICO::StaticClass()));
	return AnchorComponent;
}

bool FSpatialAnchorExtensionPICO::GetAnchorEntityUUID(const UAnchorComponentPICO* AnchorComponent, FSpatialUUIDPICO& OutAnchorUUID,EResultPICO& OutResult)
{
	bool bResult=false;
	if (!IsAnchorValid(AnchorComponent))
	{
		return bResult;
	}
	
	if(bSupportsSpatialAnchor)
	{
		FSpatialHandlePICO AnchorHandle = AnchorComponent->GetAnchorHandle();
		XrUuidEXT PxrAnchorUUID;
		XrAnchorPICO anchor = reinterpret_cast<XrAnchorPICO>(AnchorHandle.GetValue());
		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrGetAnchorUuidPICO(anchor, &PxrAnchorUUID);
		bResult = XR_SUCCEEDED(xrResult);
		OutAnchorUUID = PxrAnchorUUID.data;
		OutResult=FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
	}
	
	return bResult;
}

bool FSpatialAnchorExtensionPICO::GetAnchorPose(UAnchorComponentPICO* AnchorComponent, FTransform& OutAnchorPose,EResultPICO& OutResult)
{
	bool bResult=false;
	if(bSupportsSpatialAnchor)
	{
		if (!HasValidTrackingContext() || !IsAnchorValid(AnchorComponent))
		{
			OutResult = EResultPICO::XR_Error_ValidationFailure;
			return false;
		}

		FSpatialHandlePICO AnchorHandle = AnchorComponent->GetAnchorHandle();

		XrSpaceLocation cPxrSpaceLocation = {};
		XrAnchorLocateInfoPICO cPxrAnchorLocateInfoBD = {};
		cPxrAnchorLocateInfoBD.type = XR_TYPE_ANCHOR_LOCATE_INFO_PICO;
		cPxrAnchorLocateInfoBD.baseSpace = TrackingSpace;
		cPxrAnchorLocateInfoBD.time = CurrentDisplayTime;
		XrAnchorPICO anchor = reinterpret_cast<XrAnchorPICO>(AnchorHandle.GetValue());
		UE_LOG(LogMRPICO, VeryVerbose, TEXT("XrAnchorPICO :%llu"), reinterpret_cast<uint64_t>(anchor));
		UE_LOG(LogMRPICO, VeryVerbose, TEXT("TrackingSpace :%llu"), reinterpret_cast<uint64_t>(TrackingSpace));

		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrLocateAnchorPICO(anchor, &cPxrAnchorLocateInfoBD, &cPxrSpaceLocation);

		if (xrResult==XR_ERROR_HANDLE_INVALID)
		{
			AnchorComponent->ResetAnchorHandle();
		}
		
		bResult = XR_SUCCEEDED(xrResult);
		if (bResult)
		{
			float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();
			FTransform TrackingToWorld=XRTrackingSystem->GetTrackingToWorldTransform();

			bResult = (cPxrSpaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)
				&& (cPxrSpaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
				&& (cPxrSpaceLocation.locationFlags & XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT)
				&& (cPxrSpaceLocation.locationFlags & XR_SPACE_LOCATION_POSITION_TRACKED_BIT);
			UE_LOG(LogMRPICO, VeryVerbose, TEXT("WorldToMetersScale :%f"), WorldToMetersScale);
			UE_LOG(LogMRPICO, VeryVerbose, TEXT("cPxrSpaceLocation.pose.x :%f"), cPxrSpaceLocation.pose.position.x);
			UE_LOG(LogMRPICO, VeryVerbose, TEXT("cPxrSpaceLocation.pose.y :%f"), cPxrSpaceLocation.pose.position.y);
			UE_LOG(LogMRPICO, VeryVerbose, TEXT("cPxrSpaceLocation.pose.z :%f"), cPxrSpaceLocation.pose.position.z);

			FTransform UnrealPose = ToFTransform(cPxrSpaceLocation.pose, WorldToMetersScale);
			OutAnchorPose.SetLocation(TrackingToWorld.TransformPosition(UnrealPose.GetLocation()));
			OutAnchorPose.SetRotation(TrackingToWorld.TransformRotation(UnrealPose.GetRotation()));

			UE_LOG(LogMRPICO, VeryVerbose, TEXT("OutAnchorPose :%s"), *OutAnchorPose.ToString());
		}
		
		OutResult=FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
	}
	return bResult;
}

bool FSpatialAnchorExtensionPICO::UpdateAnchor(UAnchorComponentPICO* AnchorComponent)
{
	FTransform AnchorTransform;
	EResultPICO OutResult=EResultPICO::XR_Error_Unknown_PICO;
	if (!GetAnchorPose(AnchorComponent, AnchorTransform,OutResult))
	{
		return false;
	}

	AActor* BoundActor = AnchorComponent->GetOwner();
	BoundActor->SetActorLocationAndRotation(AnchorTransform.GetLocation(), AnchorTransform.GetRotation());
	return true;
}
