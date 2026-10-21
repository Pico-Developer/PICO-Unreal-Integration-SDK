// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_SceneCapture.h"
#include "PICO_MRModule.h"

#include "IOpenXRHMDModule.h"
#include "OpenXRCore.h"

FSceneCaptureExtensionPICO::FSceneCaptureExtensionPICO()
{
}

FSceneCaptureExtensionPICO* FSceneCaptureExtensionPICO::GetInstance()
{
	return &FPICOOpenXRMRModule::Get().GetMRModuleExtension().GetSceneCapturePICO();
}

void FSceneCaptureExtensionPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_PICO_SCENE_CAPTURE_EXTENSION_NAME);
}

void FSceneCaptureExtensionPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	if (FSpatialSensingExtensionPICO::GetInstance()->IsSupportsSpatialSensing())
	{
		bSupportsSceneCaptureEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_SCENE_CAPTURE_EXTENSION_NAME);
		UE_LOG(LogMRPICO,Verbose, TEXT("bSupportsSceneCaptureEXT:%d"), bSupportsSceneCaptureEXT)

		if (bSupportsSceneCaptureEXT)
		{
			XrSystemSceneCapturePropertiesPICO SceneCapturePropertiesPICO = { (XrStructureType)XR_TYPE_SYSTEM_SCENE_CAPTURE_PROPERTIES_PICO };
			XrSystemProperties SPSceneCapture{ XR_TYPE_SYSTEM_PROPERTIES,&SceneCapturePropertiesPICO };
			XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &SPSceneCapture));
			bSupportsSceneCapture = SceneCapturePropertiesPICO.supportsSceneCapture == XR_TRUE;
			UE_LOG(LogMRPICO,Verbose, TEXT("bSupportsSceneCapture:%d"), bSupportsSceneCapture)

			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrStartSceneCaptureAsyncPICO", (PFN_xrVoidFunction*)&xrStartSceneCaptureAsyncPICO));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrStartSceneCaptureCompletePICO", (PFN_xrVoidFunction*)&xrStartSceneCaptureCompletePICO));
		}
	}
}

void FSceneCaptureExtensionPICO::PostCreateSession(XrSession InSession)
{
	FMixedRealityPICO::PostCreateSession(InSession);

	const UPICOOpenXRRuntimeSettings* Settings = GetDefault<UPICOOpenXRRuntimeSettings>();

	if (Settings->bEnableSceneCapture)
	{
		FSenseDataProviderCreateInfoSceneCapturePICO cFPICOSenseDataProviderCreateInfoSceneCapture = {};
		EResultPICO OutResult =EResultPICO::XR_Error_Unknown_PICO;
		if(CreateProvider(cFPICOSenseDataProviderCreateInfoSceneCapture,OutResult))
		{
			UE_LOG(LogMRPICO, Log, TEXT("OnBeginSession CreateSceneCaptureProvider Success!!"));
		}
		else
		{
			UE_LOG(LogMRPICO, Error, TEXT("OnBeginSession CreateSceneCaptureProvider failed OutResult:%d"),OutResult);
		}
	}
}

bool FSceneCaptureExtensionPICO::RequestSpatialSceneInfos(const FSceneLoadInfoPICO& LoadInfo, const FPICOPollFutureDelegate& Delegate, EResultPICO& Result)
{
	bool bResult=false;
	
	if (bSupportsSceneCapture)
	{
		XrFutureEXT Handle;
		XrSenseDataQueryInfoPICO cSenseDataQueryInfoBD = {};
		cSenseDataQueryInfoBD.type = XR_TYPE_SENSE_DATA_QUERY_INFO_PICO;

		XrSenseDataFilterSemanticPICO cPxrSpatialEntitySemanticFilter = {};
		cPxrSpatialEntitySemanticFilter.type = XR_TYPE_SENSE_DATA_FILTER_SEMANTIC_PICO;
		cPxrSpatialEntitySemanticFilter.semanticCount = LoadInfo.SemanticFilter.Num();

		TArray<XrSemanticLabelPICO> SemanticLabels;

		for (auto Semantic : LoadInfo.SemanticFilter)
		{
			SemanticLabels.Add(static_cast<XrSemanticLabelPICO>(Semantic));
		}

		SemanticLabels.SetNum(LoadInfo.SemanticFilter.Num());
		cPxrSpatialEntitySemanticFilter.semantics = SemanticLabels.GetData();
	
		cSenseDataQueryInfoBD.filter = reinterpret_cast<XrSenseDataFilterBaseHeaderPICO*>(&cPxrSpatialEntitySemanticFilter);
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

bool FSceneCaptureExtensionPICO::GetSpatialSceneInfos(const XrFutureEXT& FutureHandle, TArray<FMRSceneInfoPICO>& SceneInfos, EResultPICO& OutResult)
{
	if (bSupportsSceneCapture)
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
		
		ClearComponentBuffer();

		for (auto EntityInfo : QueriedSenseData.QueriedSpatialEntityInfos)
		{
			TArray<ESpatialEntityComponentTypePICO> ComponentTypes;
			FMRSceneInfoPICO cFPICOMRSceneInfo = {};
			cFPICOMRSceneInfo.UUID = EntityInfo.uuid.data;

			if (!FSpatialSensingExtensionPICO::GetInstance()->EnumerateSpatialEntityComponentTypes(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, ComponentTypes,OutResult))
			{
				UE_LOG(LogMRPICO, Error, TEXT("EnumerateSpatialEntityComponentTypes Failed OutResult:%d"),OutResult);
				continue;
			}

			for (ESpatialEntityComponentTypePICO ComponentType : ComponentTypes)
			{
				switch (ComponentType)
				{
				case ESpatialEntityComponentTypePICO::Location:
					{
						FTransform cFTransform;

						if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityLocation(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, cFTransform,OutResult))
						{
							UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityLocation Failed OutResult:%d"),OutResult);
							continue;
						}
						
						cFPICOMRSceneInfo.ScenePose = cFTransform;
					}
					break;
				case ESpatialEntityComponentTypePICO::Semantic:
					{
						TArray<ESemanticLabelPICO> Semantics;
						if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntitySemantic(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, Semantics,OutResult))
						{
							UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityLocation Failed OutResult:%d"),OutResult);
							continue;
						}
						
						if (Semantics.Num())
						{
							cFPICOMRSceneInfo.Semantic = Semantics[0];
						}
					}
					break;
				case ESpatialEntityComponentTypePICO::Boundary_2D:
					{
						TSharedRef<FBoundingBox2DPICO> Box = MakeShared<FBoundingBox2DPICO>();
						if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityBoundary2D(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, Box.Get(),OutResult))
						{
							UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityLocation Failed OutResult:%d"),OutResult);
							continue;
						}
						
						cFPICOMRSceneInfo.SceneType = ESceneTypePICO::BoundingBox2D;
						EntityToBoundingBox2DMap.Add(EntityInfo.uuid.data, Box);
					}
					break;
				case ESpatialEntityComponentTypePICO::Polygon:
					{
						TSharedRef<TArray<FVector>> Polygon = MakeShareable(new TArray<FVector>());
						if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityPolygon(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, Polygon.Get(),OutResult))
						{
							UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityPolygon Failed OutResult:%d"),OutResult);
							continue;
						}

						cFPICOMRSceneInfo.SceneType = ESceneTypePICO::BoundingPolygon;
						EntityToPolygonMap.Add(EntityInfo.uuid.data, Polygon);
					}
					break;
				case ESpatialEntityComponentTypePICO::Boundary_3D:
					{
						TSharedRef<FBoundingBox3DPICO> Box = MakeShared<FBoundingBox3DPICO>();
						if (!FSpatialSensingExtensionPICO::GetInstance()->GetSpatialEntityBoundary3D(SenseDataQueryCompletion.SnapShotHandle, EntityInfo.entity, Box.Get(),OutResult))
						{
							UE_LOG(LogMRPICO, Error, TEXT("GetSpatialEntityBoundary3D Failed OutResult:%d"),OutResult);
							continue;
						}
						
						cFPICOMRSceneInfo.SceneType = ESceneTypePICO::BoundingBox3D;
						EntityToBoundingBox3DMap.Add(EntityInfo.uuid.data, Box);
					}
					break;
				default: ;
				}
			}

			SceneInfos.Add(cFPICOMRSceneInfo);
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

bool FSceneCaptureExtensionPICO::StartSceneCaptureAsync(const FPICOPollFutureDelegate& Delegate, EResultPICO& Result)
{
	bool bResult=false;
	if (bSupportsSceneCapture)
	{
		XrFutureEXT FutureHandle;
		XrSceneCaptureStartInfoPICO cXrSceneCaptureStartInfoPICO = {};
		cXrSceneCaptureStartInfoPICO.type = XR_TYPE_SCENE_CAPTURE_START_INFO_PICO;
		XrResult xrResult = xrStartSceneCaptureAsyncPICO(ProviderHandle, &cXrSceneCaptureStartInfoPICO, &FutureHandle);
		bResult = XR_SUCCEEDED(xrResult);
		Result = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);

		if (bResult)
		{
			bResult = FSpatialSensingExtensionPICO::GetInstance()->AddPollFutureRequirement(FutureHandle, Delegate);
		}
	}

	return bResult;
}

void FSceneCaptureExtensionPICO::ClearComponentBuffer()
{
	EntityToBoundingBox3DMap.Empty();
	EntityToBoundingBox2DMap.Empty();
	EntityToPolygonMap.Empty();
}

bool FSceneCaptureExtensionPICO::GetSpatialSceneBoundingBox3D(const FSpatialUUIDPICO& UUID, FBoundingBox3DPICO& Box3D)
{
	if (EntityToBoundingBox3DMap.Contains(UUID))
	{
		Box3D = EntityToBoundingBox3DMap.Find(UUID)->Get();
		return true;
	}
	return false;
}

bool FSceneCaptureExtensionPICO::GetSpatialSceneBoundingBox2D(const FSpatialUUIDPICO& UUID, FBoundingBox2DPICO& Box2D)
{
	if (EntityToBoundingBox2DMap.Contains(UUID))
	{
		Box2D = EntityToBoundingBox2DMap.Find(UUID)->Get();
		return true;
	}
	return false;
}

bool FSceneCaptureExtensionPICO::GetSpatialSceneBoundingPolygon(const FSpatialUUIDPICO& UUID, TArray<FVector>& Polygon)
{
	if (EntityToPolygonMap.Contains(UUID))
	{
		Polygon = EntityToPolygonMap.Find(UUID)->Get();
		return true;
	}
	return false;
}

bool FSceneCaptureExtensionPICO::StartSceneCaptureComplete(const XrFutureEXT& FutureHandle, FSceneCaptureStartCompletionPICO& completion,EResultPICO& OutResult)
{
	bool bResult=false;
	if (bSupportsSceneCapture)
	{
		XrSceneCaptureStartCompletionPICO cPxrSceneCaptureStartCompletionBD = {};
		cPxrSceneCaptureStartCompletionBD.type = XR_TYPE_SCENE_CAPTURE_START_COMPLETION_PICO;
		XrResult xrResult = xrStartSceneCaptureCompletePICO(ProviderHandle, FutureHandle, &cPxrSceneCaptureStartCompletionBD);
		completion.FutureResult = FSpatialSensingExtensionPICO::CastToPICOResult(cPxrSceneCaptureStartCompletionBD.futureResult);

		bResult = XR_SUCCEEDED(xrResult);
		OutResult =FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
		return bResult;
	}
	
	return bResult;
}

bool FSceneCaptureExtensionPICO::CreateProvider(const FSenseDataProviderCreateInfoBasePICO& createInfo,EResultPICO& OutResult)
{
	check(createInfo.Type == EProviderTypePICO::Pico_Provider_Scene_Capture);
	bool bResult=false;
	if (bSupportsSceneCapture)
	{
		if (Session == XR_NULL_HANDLE)
		{
			OutResult = EResultPICO::XR_Error_ValidationFailure;
			return false;
		}

		Type = EProviderTypePICO::Pico_Provider_Scene_Capture;
		if (IsHandleValid())
		{
			UE_LOG(LogMRPICO, Error, TEXT("CreateProvider failed already create"));
			OutResult=EResultPICO::XR_Error_LimitReached;
			return false;
		}
		XrSenseDataProviderCreateInfoSceneCapturePICO cSceneCaptureProviderCreateInfoBD = {};
		cSceneCaptureProviderCreateInfoBD.type = XR_TYPE_SENSE_DATA_PROVIDER_CREATE_INFO_SCENE_CAPTURE_PICO;

		XrResult xrResult =FSpatialSensingExtensionPICO::GetInstance()->xrCreateSenseDataProviderPICO(Session, reinterpret_cast<XrSenseDataProviderCreateInfoBaseHeaderPICO*>(&cSceneCaptureProviderCreateInfoBD), &ProviderHandle);
		bResult = XR_SUCCEEDED(xrResult);
		ProviderHandle = bResult ? ProviderHandle:XR_NULL_HANDLE;
		
		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);

	}

	return bResult;
}

