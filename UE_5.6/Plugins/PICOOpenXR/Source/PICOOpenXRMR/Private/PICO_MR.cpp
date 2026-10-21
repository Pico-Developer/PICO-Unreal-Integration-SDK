// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.


#include "PICO_MR.h"
#include "PICO_MRModule.h"
#include "OpenXRCore.h"
#include "IOpenXRHMDModule.h"

#include "IXRTrackingSystem.h"
#include "PICO_LightEstimation.h"
#include "Engine/Engine.h"

#include "PICO_MRFunctionLibrary.h"
#include "PICO_MRTextures.h"
#include "PICO_MRTypes.h"
#include "PICO_SceneCapture.h"
#include "PICO_SpatialAnchor.h"
#include "PICO_SpatialMesh.h"
#include "PICO_SpatialPlane.h"


FSpatialSensingExtensionPICO::FSpatialSensingExtensionPICO(): CurrentDisplayTime(0), GlobalUUIDCount(0)
{
}

FSpatialSensingExtensionPICO* FSpatialSensingExtensionPICO::GetInstance()
{
	return &FPICOOpenXRMRModule::Get().GetMRModuleExtension().GetSpatialSensingPICO();
}

void FSpatialSensingExtensionPICO::OnEvent(XrSession InSession, const XrEventDataBaseHeader* InHeader)
{
	const XrEventDataBuffer* EventDataBuffer = reinterpret_cast<const XrEventDataBuffer*>(InHeader);

	if (EventDataBuffer == nullptr)
	{
		return;
	}
	
	PollEvent(EventDataBuffer);
}

void FSpatialSensingExtensionPICO::GetRequiredExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	OutExtensions.Add(XR_PICO_SPATIAL_SENSING_EXTENSION_NAME);
	OutExtensions.Add(XR_EXT_FUTURE_EXTENSION_NAME);
}

void FSpatialSensingExtensionPICO::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	bSupportsSpatialSensingEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_PICO_SPATIAL_SENSING_EXTENSION_NAME);
	Instance =InInstance;
	UE_LOG(LogMRPICO, Verbose, TEXT("bSupportsSpatialSensingEXT:%d"), bSupportsSpatialSensingEXT)

	if (bSupportsSpatialSensingEXT)
	{
		XrSystemSpatialSensingPropertiesPICO SpatialSensingPropertiesPICO = { (XrStructureType)XR_TYPE_SYSTEM_SPATIAL_SENSING_PROPERTIES_PICO };
		XrSystemProperties SPSpatialSensing{ XR_TYPE_SYSTEM_PROPERTIES,&SpatialSensingPropertiesPICO };
		XR_ENSURE(xrGetSystemProperties(InInstance, InSystem, &SPSpatialSensing));
		bSupportsSpatialSensing = SpatialSensingPropertiesPICO.supportsSpatialSensing == XR_TRUE;
		UE_LOG(LogMRPICO, VeryVerbose, TEXT("bSupportsSpatialSensing:%d"), bSupportsSpatialSensing)

		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateSenseDataProviderPICO", (PFN_xrVoidFunction*)&xrCreateSenseDataProviderPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrStartSenseDataProviderAsyncPICO", (PFN_xrVoidFunction*)&xrStartSenseDataProviderAsyncPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrStartSenseDataProviderCompletePICO", (PFN_xrVoidFunction*)&xrStartSenseDataProviderCompletePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetSenseDataProviderStatePICO", (PFN_xrVoidFunction*)&xrGetSenseDataProviderStatePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrQuerySenseDataAsyncPICO", (PFN_xrVoidFunction*)&xrQuerySenseDataAsyncPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrQuerySenseDataCompletePICO", (PFN_xrVoidFunction*)&xrQuerySenseDataCompletePICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrStopSenseDataProviderPICO", (PFN_xrVoidFunction*)&xrStopSenseDataProviderPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroySenseDataProviderPICO", (PFN_xrVoidFunction*)&xrDestroySenseDataProviderPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroySenseDataSnapshotPICO", (PFN_xrVoidFunction*)&xrDestroySenseDataSnapshotPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetQueriedSenseDataPICO", (PFN_xrVoidFunction*)&xrGetQueriedSenseDataPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetSpatialEntityUuidPICO", (PFN_xrVoidFunction*)&xrGetSpatialEntityUuidPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetSpatialEntityComponentDataPICO", (PFN_xrVoidFunction*)&xrGetSpatialEntityComponentDataPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrEnumerateSpatialEntityComponentTypesPICO", (PFN_xrVoidFunction*)&xrEnumerateSpatialEntityComponentTypesPICO));

		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrPollFutureEXT", (PFN_xrVoidFunction*)&xrPollFutureEXT));

		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrRetrieveSpatialEntityAnchorPICO", (PFN_xrVoidFunction*)&xrRetrieveSpatialEntityAnchorPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyAnchorPICO", (PFN_xrVoidFunction*)&xrDestroyAnchorPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGetAnchorUuidPICO", (PFN_xrVoidFunction*)&xrGetAnchorUuidPICO));
		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrLocateAnchorPICO", (PFN_xrVoidFunction*)&xrLocateAnchorPICO));

		XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrReleaseEnvironmentTextureImagePICO", (PFN_xrVoidFunction*)&xrReleaseEnvironmentTextureImagePICO));

	}
}

void FSpatialSensingExtensionPICO::PostCreateSession(XrSession InSession)
{
	Session = InSession;
	static FName SystemName(TEXT("OpenXR"));
	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
	{
		XRTrackingSystem = GEngine->XRSystem.Get();
	}
	
	FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddRaw(this, &FSpatialSensingExtensionPICO::ApplicationResumeDelegate);
}

void FSpatialSensingExtensionPICO::OnDestroySession(XrSession InSession)
{
	if (InSession != Session)
	{
		return;
	}

	FCoreDelegates::ApplicationHasEnteredForegroundDelegate.RemoveAll(this);
	Session = XR_NULL_HANDLE;
	TrackingSpace = XR_NULL_HANDLE;
	CurrentDisplayTime = 0;
	XRTrackingSystem = nullptr;
}

void FSpatialSensingExtensionPICO::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace InTrackingSpace)
{
	CurrentDisplayTime = DisplayTime;
	TrackingSpace = InTrackingSpace;

	PXR_PollFuture();
}

void FSpatialSensingExtensionPICO::ApplicationResumeDelegate()
{
	UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->ResumeDelegate.Broadcast();
}


bool FSpatialSensingExtensionPICO::AddPollFutureRequirement(const XrFutureEXT& FutureHandle, const FPICOPollFutureDelegate& Delegate)
{
	FFutureMessagePICO cFutureMessage;
	cFutureMessage.MessageHandle = FutureHandle;
	cFutureMessage.Uuid = GetUUID();
	FutureToDelegateMap.Add(cFutureMessage.Uuid, Delegate);

	return FutureQueueForProviders.Enqueue(cFutureMessage);
}

bool FSpatialSensingExtensionPICO::StartSenseDataProviderComplete(const XrFutureEXT& FutureHandle, FSenseDataProviderStartCompletionPICO& Completion, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		XrSenseDataProviderStartCompletionPICO SenseDataProviderStartCompletionBD = {};
		SenseDataProviderStartCompletionBD.type = XR_TYPE_SENSE_DATA_PROVIDER_START_COMPLETION_PICO;
		XrResult xrResult = xrStartSenseDataProviderCompletePICO(Session, FutureHandle, &SenseDataProviderStartCompletionBD);
		bResult = XR_SUCCEEDED(xrResult);
		Completion.FutureResult = CastToPICOResult(SenseDataProviderStartCompletionBD.futureResult);
		OutResult = CastToPICOResult(xrResult);
	}

	return bResult;
}

bool FSpatialSensingExtensionPICO::QuerySenseDataComplete(const XrSenseDataProviderPICO& ProviderHandle, const XrFutureEXT& FutureHandle, FSenseDataQueryCompletionPICO& Completion, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		XrSenseDataQueryCompletionPICO cPxrSenseDataQueryCompletionBD = {};
		cPxrSenseDataQueryCompletionBD.type = XR_TYPE_SENSE_DATA_QUERY_COMPLETION_PICO;
		XrResult xrResult = xrQuerySenseDataCompletePICO(ProviderHandle, FutureHandle, &cPxrSenseDataQueryCompletionBD);
		bResult = XR_SUCCEEDED(xrResult);
		Completion.SnapShotHandle = cPxrSenseDataQueryCompletionBD.snapshot;
		Completion.FutureResult = CastToPICOResult(cPxrSenseDataQueryCompletionBD.futureResult);

		OutResult = CastToPICOResult(xrResult);
	}

	return bResult;
}

bool FSpatialSensingExtensionPICO::GetQueriedSenseData(const XrSenseDataProviderPICO& ProviderHandle, const XrSenseDataSnapshotPICO& QueryResultHandle, FQueriedSenseDataPICO& QueriedSenseData, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		XrQueriedSenseDataGetInfoPICO cQueriedSenseDataGetInfoPICO = {};
		cQueriedSenseDataGetInfoPICO.type = XR_TYPE_QUERIED_SENSE_DATA_GET_INFO_PICO;
		cQueriedSenseDataGetInfoPICO.snapshot = QueryResultHandle;

		XrQueriedSenseDataPICO cPxrQueriedSenseDataBD = {};
		cPxrQueriedSenseDataBD.type = XR_TYPE_QUERIED_SENSE_DATA_PICO;
		
		XrResult xrResult = xrGetQueriedSenseDataPICO(ProviderHandle, &cQueriedSenseDataGetInfoPICO, &cPxrQueriedSenseDataBD);
		bResult = XR_SUCCEEDED(xrResult);

		if (bResult)
		{
			if (cPxrQueriedSenseDataBD.spatialEntityCountOutput == 0)
			{
				return bResult;
			}
			cPxrQueriedSenseDataBD.spatialEntityCapacityInput = cPxrQueriedSenseDataBD.spatialEntityCountOutput;
			QueriedSenseData.QueriedSpatialEntityInfos.SetNum(cPxrQueriedSenseDataBD.spatialEntityCapacityInput);

			cPxrQueriedSenseDataBD.spatialEntities = QueriedSenseData.QueriedSpatialEntityInfos.GetData();
			xrResult = xrGetQueriedSenseDataPICO(ProviderHandle, &cQueriedSenseDataGetInfoPICO, &cPxrQueriedSenseDataBD);

			bResult = XR_SUCCEEDED(xrResult);
		}

		OutResult = CastToPICOResult(xrResult);
	}

	return bResult;
}

bool FSpatialSensingExtensionPICO::DestroySenseDataQueryResult(const XrSenseDataSnapshotPICO& QueryResultHandle, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		FRWScopeLock Lock(DestroyLock, SLT_Write);
		XrResult xrResult = xrDestroySenseDataSnapshotPICO(QueryResultHandle);
		bResult = XR_SUCCEEDED(xrResult);
		OutResult = CastToPICOResult(xrResult);
	}
	return bResult;
}

void FSpatialSensingExtensionPICO::PXR_PollFuture()
{
	if (bSupportsSpatialSensing)
	{
		FFutureMessagePICO CurrentMessage;

		uint64 LeftMessageCount = 0;
		bool hasFoundFrameBarrier = false;

		while (FutureQueueForProviders.Peek(CurrentMessage))
		{
			if (CurrentMessage.IsFrameBarrier)
			{
				CurrentMessage.IsFrameBarrier = false;
				FutureQueueForProviders.Pop();
				FutureQueueForProviders.Enqueue(CurrentMessage);
				break;
			}

			FutureQueueForProviders.Pop();
			XrFuturePollInfoEXT FuturePollInfoEXT = {};
			FuturePollInfoEXT.type = XR_TYPE_FUTURE_POLL_INFO_EXT;
			FuturePollInfoEXT.future = CurrentMessage.MessageHandle;

			XrFuturePollResultEXT cPollResultEXT = {};
			cPollResultEXT.type = XR_TYPE_FUTURE_POLL_RESULT_EXT;

			XrResult Result = xrPollFutureEXT(Instance, &FuturePollInfoEXT, &cPollResultEXT);

			const bool bResult = XR_SUCCEEDED(Result);
			if (!bResult)
			{
				UE_LOG(LogMRPICO, Error, TEXT("Provider PollFuture failed at:%p"), FuturePollInfoEXT.future);
			}

			if (cPollResultEXT.state == XR_FUTURE_STATE_READY_EXT
				&& FutureToDelegateMap.Contains(CurrentMessage.Uuid))
			{
				FutureToDelegateMap[CurrentMessage.Uuid].ExecuteIfBound(CurrentMessage.MessageHandle);
				FutureToDelegateMap.Remove(CurrentMessage.Uuid);
			}
			else
			{
				LeftMessageCount++;
				if (!hasFoundFrameBarrier)
				{
					CurrentMessage.IsFrameBarrier = true;
					hasFoundFrameBarrier = true;
				}
				FutureQueueForProviders.Enqueue(CurrentMessage);
			}
		}
	}
}

bool FSpatialSensingExtensionPICO::GetSpatialEntityLocation(const XrSenseDataSnapshotPICO& SnapshotHandle, const XrSpatialEntityIdPICO& EntityHandle, FTransform& Transform, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		if (TrackingSpace == XR_NULL_HANDLE || !HasValidTrackingSystem())
		{
			OutResult = EResultPICO::XR_Error_ValidationFailure;
			return false;
		}

		XrSpatialEntityLocationGetInfoPICO cComponentInfoGetInfo = {};
		cComponentInfoGetInfo.type = XR_TYPE_SPATIAL_ENTITY_LOCATION_GET_INFO_PICO;
		cComponentInfoGetInfo.componentType = XR_SPATIAL_ENTITY_COMPONENT_TYPE_LOCATION_PICO;
		cComponentInfoGetInfo.entity = EntityHandle;
		cComponentInfoGetInfo.baseSpace = TrackingSpace;

		XrSpatialEntityLocationDataPICO cComponentInfo = {};
		cComponentInfo.type = XR_TYPE_SPATIAL_ENTITY_LOCATION_DATA_PICO;

		XrResult xrResult = xrGetSpatialEntityComponentDataPICO(SnapshotHandle
																, reinterpret_cast<XrSpatialEntityComponentGetInfoBaseHeaderPICO*>(&cComponentInfoGetInfo)
																, reinterpret_cast<XrSpatialEntityComponentDataBaseHeaderPICO*>(&cComponentInfo));
		bResult = XR_SUCCEEDED(xrResult);
		if (bResult)
		{
			float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();
			FTransform TrackingToWorld = XRTrackingSystem->GetTrackingToWorldTransform();
			FTransform UnrealPose = ToFTransform(cComponentInfo.location.pose, WorldToMetersScale);

			Transform.SetLocation(TrackingToWorld.TransformPosition(UnrealPose.GetLocation()));
			Transform.SetRotation(TrackingToWorld.TransformRotation(UnrealPose.GetRotation()));
		}

		OutResult = CastToPICOResult(xrResult);
	}
	return bResult;
}

bool FSpatialSensingExtensionPICO::GetSpatialEntitySemantic(const XrSenseDataSnapshotPICO& SnapshotHandle, const XrSpatialEntityIdPICO& EntityHandle, TArray<ESemanticLabelPICO>& Semantics, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		XrSpatialEntitySemanticGetInfoPICO cComponentInfoGetInfo = {};
		cComponentInfoGetInfo.type = XR_TYPE_SPATIAL_ENTITY_SEMANTIC_GET_INFO_PICO;
		cComponentInfoGetInfo.componentType = XR_SPATIAL_ENTITY_COMPONENT_TYPE_SEMANTIC_PICO;
		cComponentInfoGetInfo.entity = EntityHandle;
		XrSpatialEntitySemanticDataPICO cComponentInfo;
		cComponentInfo.type = XR_TYPE_SPATIAL_ENTITY_SEMANTIC_DATA_PICO;
		cComponentInfo.semanticLabels = nullptr;
		cComponentInfo.semanticCapacityInput = 0;

		XrResult xrResult = xrGetSpatialEntityComponentDataPICO(SnapshotHandle
																, reinterpret_cast<XrSpatialEntityComponentGetInfoBaseHeaderPICO*>(&cComponentInfoGetInfo)
																, reinterpret_cast<XrSpatialEntityComponentDataBaseHeaderPICO*>(&cComponentInfo));
		bResult = XR_SUCCEEDED(xrResult);

		if (bResult)
		{
			cComponentInfo.semanticCapacityInput = cComponentInfo.semanticCountOutput;
			UE_LOG(LogMRPICO,VeryVerbose,TEXT("FSpatialSensingExtensionPICO::semanticCountOutput:%d"),cComponentInfo.semanticCountOutput);
			TArray<XrSemanticLabelPICO> TempSemantics;
			TempSemantics.SetNum(cComponentInfo.semanticCapacityInput);
			cComponentInfo.semanticLabels = TempSemantics.GetData();

			xrResult = xrGetSpatialEntityComponentDataPICO(SnapshotHandle
														   , reinterpret_cast<XrSpatialEntityComponentGetInfoBaseHeaderPICO*>(&cComponentInfoGetInfo)
														   , reinterpret_cast<XrSpatialEntityComponentDataBaseHeaderPICO*>(&cComponentInfo));
			bResult = XR_SUCCEEDED(xrResult);

			if (bResult)
			{
				for (auto Semantic : TempSemantics)
				{
					UE_LOG(LogMRPICO,VeryVerbose,TEXT("Semantic:%d"),Semantic);
					Semantics.Add(static_cast<ESemanticLabelPICO>(Semantic));
				}
			}
		}

		OutResult = CastToPICOResult(xrResult);
	}
	return bResult;
}

bool FSpatialSensingExtensionPICO::GetSpatialEntityBoundary3D(const XrSenseDataSnapshotPICO& SnapshotHandle, const XrSpatialEntityIdPICO& EntityHandle, FBoundingBox3DPICO& Box, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		if (!HasValidTrackingSystem())
		{
			OutResult = EResultPICO::XR_Error_ValidationFailure;
			return false;
		}

		XrSpatialEntityBoundingBox3DGetInfoPICO cComponentInfoGetInfo = {};
		cComponentInfoGetInfo.type = XR_TYPE_SPATIAL_ENTITY_BOUNDING_BOX_3D_GET_INFO_PICO;
		cComponentInfoGetInfo.componentType = XR_SPATIAL_ENTITY_COMPONENT_TYPE_BOUNDING_BOX_3D_PICO;
		cComponentInfoGetInfo.entity = EntityHandle;
		XrSpatialEntityBoundingBox3DDataPICO cComponentInfo = {};
		cComponentInfo.type = XR_TYPE_SPATIAL_ENTITY_BOUNDING_BOX_3D_DATA_PICO;
		XrResult xrResult = xrGetSpatialEntityComponentDataPICO(SnapshotHandle
																, reinterpret_cast<XrSpatialEntityComponentGetInfoBaseHeaderPICO*>(&cComponentInfoGetInfo)
																, reinterpret_cast<XrSpatialEntityComponentDataBaseHeaderPICO*>(&cComponentInfo));
		bResult = XR_SUCCEEDED(xrResult);

		if (bResult)
		{
			float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();

			Box.Center.SetLocation(FVector(cComponentInfo.boundingBox3D.center.position.x,
										   cComponentInfo.boundingBox3D.center.position.y,
										   cComponentInfo.boundingBox3D.center.position.z) * WorldToMetersScale);

			Box.Center.SetRotation(FQuat(cComponentInfo.boundingBox3D.center.orientation.x,
										 cComponentInfo.boundingBox3D.center.orientation.y,
										 cComponentInfo.boundingBox3D.center.orientation.z,
										 cComponentInfo.boundingBox3D.center.orientation.w));

			Box.Extent.Width = cComponentInfo.boundingBox3D.extents.width * WorldToMetersScale;
			Box.Extent.Height = cComponentInfo.boundingBox3D.extents.height * WorldToMetersScale;
			Box.Extent.Depth = cComponentInfo.boundingBox3D.extents.depth * WorldToMetersScale;
		}

		OutResult = CastToPICOResult(xrResult);
	}
	return bResult;
}

bool FSpatialSensingExtensionPICO::GetSpatialEntityBoundary2D(const XrSenseDataSnapshotPICO& SnapshotHandle, const XrSpatialEntityIdPICO& EntityHandle, FBoundingBox2DPICO& Box, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		if (!HasValidTrackingSystem())
		{
			OutResult = EResultPICO::XR_Error_ValidationFailure;
			return false;
		}

		XrSpatialEntityBoundingBox2DGetInfoPICO cComponentInfoGetInfo = {};
		cComponentInfoGetInfo.type = XR_TYPE_SPATIAL_ENTITY_BOUNDING_BOX_2D_GET_INFO_PICO;
		cComponentInfoGetInfo.componentType = XR_SPATIAL_ENTITY_COMPONENT_TYPE_BOUNDING_BOX_2D_PICO;
		cComponentInfoGetInfo.entity = EntityHandle;
		XrSpatialEntityBoundingBox2DDataPICO cComponentInfo = {};
		cComponentInfo.type = XR_TYPE_SPATIAL_ENTITY_BOUNDING_BOX_2D_DATA_PICO;
		XrResult xrResult = xrGetSpatialEntityComponentDataPICO(SnapshotHandle
																, reinterpret_cast<XrSpatialEntityComponentGetInfoBaseHeaderPICO*>(&cComponentInfoGetInfo)
																, reinterpret_cast<XrSpatialEntityComponentDataBaseHeaderPICO*>(&cComponentInfo));
		bResult = XR_SUCCEEDED(xrResult);

		if (bResult)
		{
			float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();

			Box.Center = FVector(0, cComponentInfo.boundingBox2D.offset.x, cComponentInfo.boundingBox2D.offset.y) * WorldToMetersScale;
			Box.Extent.Width = cComponentInfo.boundingBox2D.extent.width * WorldToMetersScale;
			Box.Extent.Height = cComponentInfo.boundingBox2D.extent.height * WorldToMetersScale;
		}

		OutResult = CastToPICOResult(xrResult);
	}
	return bResult;
}

bool FSpatialSensingExtensionPICO::GetSpatialEntityPolygon(const XrSenseDataSnapshotPICO& SnapshotHandle, const XrSpatialEntityIdPICO& EntityHandle, TArray<FVector>& Vertices, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		if (!HasValidTrackingSystem())
		{
			OutResult = EResultPICO::XR_Error_ValidationFailure;
			return false;
		}

		XrSpatialEntityPolygonGetInfoPICO cComponentInfoGetInfo = {};
		cComponentInfoGetInfo.type = XR_TYPE_SPATIAL_ENTITY_POLYGON_GET_INFO_PICO;
		cComponentInfoGetInfo.componentType = XR_SPATIAL_ENTITY_COMPONENT_TYPE_POLYGON_PICO;
		cComponentInfoGetInfo.entity = EntityHandle;
		XrSpatialEntityPolygonDataPICO cComponentInfo = {};
		cComponentInfo.type = XR_TYPE_SPATIAL_ENTITY_POLYGON_DATA_PICO;
		XrResult xrResult = xrGetSpatialEntityComponentDataPICO(SnapshotHandle
																, reinterpret_cast<XrSpatialEntityComponentGetInfoBaseHeaderPICO*>(&cComponentInfoGetInfo)
																, reinterpret_cast<XrSpatialEntityComponentDataBaseHeaderPICO*>(&cComponentInfo));
		bResult = XR_SUCCEEDED(xrResult);

		if (bResult)
		{
			cComponentInfo.polygonCapacityInput = cComponentInfo.polygonCountOutput;

			TArray<XrVector2f> TempVertices;
			TempVertices.SetNum(cComponentInfo.polygonCapacityInput);
			cComponentInfo.polygonVertices = TempVertices.GetData();

			xrResult = xrGetSpatialEntityComponentDataPICO(SnapshotHandle
														   , reinterpret_cast<XrSpatialEntityComponentGetInfoBaseHeaderPICO*>(&cComponentInfoGetInfo)
														   , reinterpret_cast<XrSpatialEntityComponentDataBaseHeaderPICO*>(&cComponentInfo));
			bResult = XR_SUCCEEDED(xrResult);
			float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();


			if (bResult)
			{
				Vertices.Empty(TempVertices.Num());
				Algo::Transform(TempVertices, Vertices, [this,WorldToMetersScale](const auto& Vertex) { return FVector(0.f, Vertex.x, Vertex.y) * WorldToMetersScale; });
			}
		}

		OutResult = CastToPICOResult(xrResult);
	}
	return bResult;
}

bool FSpatialSensingExtensionPICO::GetSpatialEntityTriangleMesh(const XrSenseDataSnapshotPICO& SnapshotHandle, const XrSpatialEntityIdPICO& EntityHandle, TArray<FVector>& Vertices, TArray<uint16>& Triangles, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		if (!HasValidTrackingSystem())
		{
			OutResult = EResultPICO::XR_Error_ValidationFailure;
			return false;
		}

		XrSpatialEntityTriangleMeshGetInfoPICO cComponentInfoGetInfo = {};
		cComponentInfoGetInfo.type = XR_TYPE_SPATIAL_ENTITY_TRIANGLE_MESH_GET_INFO_PICO;
		cComponentInfoGetInfo.componentType = XR_SPATIAL_ENTITY_COMPONENT_TYPE_TRIANGLE_MESH_PICO;
		cComponentInfoGetInfo.entity = EntityHandle;
		XrSpatialEntityTriangleMeshDataPICO cComponentInfo = {};
		cComponentInfo.type = XR_TYPE_SPATIAL_ENTITY_TRIANGLE_MESH_DATA_PICO;
		XrResult xrResult = xrGetSpatialEntityComponentDataPICO(SnapshotHandle
																, reinterpret_cast<XrSpatialEntityComponentGetInfoBaseHeaderPICO*>(&cComponentInfoGetInfo)
																, reinterpret_cast<XrSpatialEntityComponentDataBaseHeaderPICO*>(&cComponentInfo));
		bResult = XR_SUCCEEDED(xrResult);

		if (bResult)
		{
			cComponentInfo.indexCapacityInput = cComponentInfo.indexCountOutput;
			cComponentInfo.vertexCapacityInput = cComponentInfo.vertexCountOutput;

			Triangles.SetNum(cComponentInfo.indexCapacityInput);
			cComponentInfo.indices = Triangles.GetData();

			TArray<XrVector3f> TempVertices;
			TempVertices.SetNum(cComponentInfo.vertexCapacityInput);
			cComponentInfo.vertices = TempVertices.GetData();

			xrResult = xrGetSpatialEntityComponentDataPICO(SnapshotHandle
														   , reinterpret_cast<XrSpatialEntityComponentGetInfoBaseHeaderPICO*>(&cComponentInfoGetInfo)
														   , reinterpret_cast<XrSpatialEntityComponentDataBaseHeaderPICO*>(&cComponentInfo));
			bResult = XR_SUCCEEDED(xrResult);

			if (bResult)
			{
				float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();

				for (auto Vertice : TempVertices)
				{
					FVector VertexPose = ToFVector(Vertice) * WorldToMetersScale;
					if (VertexPose.ContainsNaN())
					{
						return false;
					}
					Vertices.Add(VertexPose);
				}
			}
		}

		OutResult = CastToPICOResult(xrResult);
	}
	return bResult;
}

bool FSpatialSensingExtensionPICO::GetSpatialEntityPlaneOrientation(const XrSenseDataSnapshotPICO& SnapshotHandle, const XrSpatialEntityIdPICO& EntityHandle, EPlaneOrientationPICO& PlaneOrientationPICO, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		XrSpatialEntityPlaneOrientationGetInfoPICO cComponentInfoGetInfo = {};
		cComponentInfoGetInfo.type = XR_TYPE_SPATIAL_ENTITY_PLANE_ORIENTATION_GET_INFO_PICO;
		cComponentInfoGetInfo.componentType = XR_SPATIAL_ENTITY_COMPONENT_TYPE_PLANE_ORIENTATION_PICO;
		cComponentInfoGetInfo.entity = EntityHandle;
		XrSpatialEntityPlaneOrientationDataPICO cComponentInfo = {};
		cComponentInfo.type = XR_TYPE_SPATIAL_ENTITY_PLANE_ORIENTATION_DATA_PICO;
		XrResult xrResult = xrGetSpatialEntityComponentDataPICO(SnapshotHandle
																, reinterpret_cast<XrSpatialEntityComponentGetInfoBaseHeaderPICO*>(&cComponentInfoGetInfo)
																, reinterpret_cast<XrSpatialEntityComponentDataBaseHeaderPICO*>(&cComponentInfo));
		bResult = XR_SUCCEEDED(xrResult);

		if (bResult)
		{
			PlaneOrientationPICO=static_cast<EPlaneOrientationPICO>(cComponentInfo.orientation);
		}

		OutResult = CastToPICOResult(xrResult);
	}
	return bResult;
}

bool FSpatialSensingExtensionPICO::EnumerateSpatialEntityComponentTypes(const XrSenseDataSnapshotPICO& SnapshotHandle, const XrSpatialEntityIdPICO& EntityHandle, TArray<ESpatialEntityComponentTypePICO>& componentTypes, EResultPICO& OutResult)
{
	bool bResult = false;
	if (bSupportsSpatialSensing)
	{
		uint32_t ComponentTypeCapacityInput = 0;
		uint32_t ComponentTypeCountOutput = 0;
		XrResult xrResult = xrEnumerateSpatialEntityComponentTypesPICO(SnapshotHandle, EntityHandle, ComponentTypeCapacityInput, &ComponentTypeCountOutput, nullptr);
		bResult = XR_SUCCEEDED(xrResult);

		if (bResult && ComponentTypeCountOutput != 0)
		{
			ComponentTypeCapacityInput = ComponentTypeCountOutput;

			TArray<XrSpatialEntityComponentTypePICO> TempComponentTypes;
			TempComponentTypes.SetNum(ComponentTypeCapacityInput);

			xrResult = xrEnumerateSpatialEntityComponentTypesPICO(SnapshotHandle,
																  EntityHandle,
																  ComponentTypeCapacityInput,
																  &ComponentTypeCountOutput,
																  TempComponentTypes.GetData());
			bResult = XR_SUCCEEDED(xrResult);
			for (auto ComponentTypeBD : TempComponentTypes)
			{
				componentTypes.Add(static_cast<ESpatialEntityComponentTypePICO>(ComponentTypeBD));
			}
		}

		OutResult = CastToPICOResult(xrResult);
	}
	return bResult;
}

void FSpatialSensingExtensionPICO::PollEvent(const XrEventDataBuffer* EventData)
{
	if (bSupportsSpatialSensing)
	{
		UE_LOG(LogMRPICO, VeryVerbose, TEXT("PollEvent EventData:%d"), EventData->type);
		switch (EventData->type)
		{
		case XR_TYPE_EVENT_DATA_SENSE_DATA_UPDATED_PICO:
			{
				const XrEventDataSenseDataUpdatedPICO* DataSenseDataState = reinterpret_cast<const XrEventDataSenseDataUpdatedPICO*>(EventData);

				UE_LOG(LogMRPICO, VeryVerbose, TEXT("DataSenseDataState->provider:%p"), DataSenseDataState->provider);

				switch (GetProviderTypeByHandle(DataSenseDataState->provider))
				{
				case EProviderTypePICO::Pico_Provider_Anchor:
					{
						UE_LOG(LogMRPICO, VeryVerbose, TEXT("Pico_Provider_Anchor DataUpdated"));
						UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->AnchorDataUpdatedDelegate.Broadcast();
					}
					break;
				case EProviderTypePICO::Pico_Provider_Mesh:
					{
						UE_LOG(LogMRPICO, VeryVerbose, TEXT("Pico_Provider_Mesh DataUpdated"));
						UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->MeshDataUpdatedDelegate.Broadcast();
					}
					break;
				case EProviderTypePICO::Pico_Provider_Plane:
					{
						UE_LOG(LogMRPICO, VeryVerbose, TEXT("Pico_Provider_Plane DataUpdated"));
						UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->PlaneDataUpdatedDelegate.Broadcast();
					}
					break;
				case EProviderTypePICO::Pico_Provider_Scene_Capture:
					{
						UE_LOG(LogMRPICO, VeryVerbose, TEXT("Pico_Provider_Scene_Capture DataUpdated"));
						UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->SceneCaptureDataUpdatedDelegate.Broadcast();
					}
					break;
				case EProviderTypePICO::Pico_Provider_Light_Estimate:
					{
						UE_LOG(LogMRPICO, VeryVerbose, TEXT("Pico_Provider_Light_Estimate DataUpdated"));
						UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->LightEstimationDataUpdatedDelegate.Broadcast();
					}
					break;
				default: ;
				};

				break;
			}
		case XR_TYPE_EVENT_DATA_SENSE_DATA_PROVIDER_STATE_CHANGED_PICO:
			{
				const XrEventDataSenseDataProviderStateChangedPICO* SenseDataProviderState = reinterpret_cast<const XrEventDataSenseDataProviderStateChangedPICO*>(EventData);
				const EMRStatePICO State = static_cast<EMRStatePICO>(SenseDataProviderState->newState);
				UE_LOG(LogMRPICO, VeryVerbose, TEXT("PollEvent State:%d"), State);
				switch (GetProviderTypeByHandle(SenseDataProviderState->provider))
				{
				case EProviderTypePICO::Pico_Provider_Anchor:
					{
						UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->SpatialAnchorServiceStateUpdatedDelegate.Broadcast(State);
					}
					break;
				case EProviderTypePICO::Pico_Provider_Mesh:
					{
						UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->MeshScanningStateUpdatedDelegate.Broadcast(State);
					}
					break;
				case EProviderTypePICO::Pico_Provider_Scene_Capture:
					{
						UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->SceneCaptureServiceStateUpdatedDelegate.Broadcast(State);
					}
					break;
				case EProviderTypePICO::Pico_Provider_Plane:
					{
						UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->PlaneScanningStateUpdatedDelegate.Broadcast(State);
					}
					break;
				case EProviderTypePICO::Pico_Provider_Light_Estimate:
					{
						UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->LightEstimationServiceStateUpdatedDelegate.Broadcast(State);
					}
					break;
				default: ;
				};
				break;
			}
		default:
			{
				break;
			}
		}
	}
}


EProviderTypePICO FSpatialSensingExtensionPICO::GetProviderTypeByHandle(const XrSenseDataProviderPICO& Handle)
{
	if (FSpatialAnchorExtensionPICO::GetInstance()->IsEqualProvider(Handle))
	{
		return EProviderTypePICO::Pico_Provider_Anchor;
	}

	if (FSpatialMeshExtensionPICO::GetInstance()->IsEqualProvider(Handle))
	{
		return EProviderTypePICO::Pico_Provider_Mesh;
	}

	if (FSceneCaptureExtensionPICO::GetInstance()->IsEqualProvider(Handle))
	{
		return EProviderTypePICO::Pico_Provider_Scene_Capture;
	}

	if (FSpatialPlaneExtensionPICO::GetInstance()->IsEqualProvider(Handle))
	{
		return EProviderTypePICO::Pico_Provider_Plane;
	}

	if (FLightEstimationExtensionPICO::GetInstance()->IsEqualProvider(Handle))
	{
		return EProviderTypePICO::Pico_Provider_Light_Estimate;
	}

	return EProviderTypePICO::Pico_Provider_Max;
}


EResultPICO FSpatialSensingExtensionPICO::CastToPICOResult(XrResult Result)
{
	EResultPICO PICOResult = EResultPICO::XR_Error_Unknown_PICO;
	switch (Result)
	{
	case XrResult::XR_SUCCESS: PICOResult = EResultPICO::XR_Success;
		break;
	case XrResult::XR_TIMEOUT_EXPIRED: PICOResult = EResultPICO::XR_TimeoutExpired;
		break;
	case XrResult::XR_ERROR_VALIDATION_FAILURE: PICOResult = EResultPICO::XR_Error_ValidationFailure;
		break;
	case XrResult::XR_ERROR_RUNTIME_FAILURE: PICOResult = EResultPICO::XR_Error_RuntimeFailure;
		break;
	case XrResult::XR_ERROR_OUT_OF_MEMORY: PICOResult = EResultPICO::XR_Error_OutOfMemory;
		break;
	case XrResult::XR_ERROR_API_VERSION_UNSUPPORTED: PICOResult = EResultPICO::XR_Error_APIVersionUnsupported;
		break;
	case XrResult::XR_ERROR_INITIALIZATION_FAILED: PICOResult = EResultPICO::XR_Error_InitializationFailed;
		break;
	case XrResult::XR_ERROR_FUNCTION_UNSUPPORTED: PICOResult = EResultPICO::XR_Error_FunctionUnsupported;
		break;
	case XrResult::XR_ERROR_FEATURE_UNSUPPORTED: PICOResult = EResultPICO::XR_Error_FeatureUnsupported;
		break;
	case XrResult::XR_ERROR_LIMIT_REACHED: PICOResult = EResultPICO::XR_Error_LimitReached;
		break;
	case XrResult::XR_ERROR_SIZE_INSUFFICIENT: PICOResult = EResultPICO::XR_Error_SizeInsufficient;
		break;
	case XrResult::XR_ERROR_HANDLE_INVALID: PICOResult = EResultPICO::XR_Error_HandleInvalid;
		break;
	case XrResult::XR_ERROR_POSE_INVALID: PICOResult = EResultPICO::XR_Error_Pose_Invalid_PICO;
		break;
	case XrResult::XR_ERROR_SPATIAL_ANCHOR_SHARING_NETWORK_TIMEOUT_PICO: PICOResult = EResultPICO::XR_Error_Anchor_Sharing_Network_Timeout_PICO;
		break;
	case XrResult::XR_ERROR_SPATIAL_ANCHOR_SHARING_AUTHENTICATION_FAILURE_PICO: PICOResult = EResultPICO::XR_Error_Anchor_Sharing_Authentication_Failure_PICO;
		break;
	case XrResult::XR_ERROR_SPATIAL_ANCHOR_SHARING_NETWORK_FAILURE_PICO: PICOResult = EResultPICO::XR_Error_Anchor_Sharing_Network_Failure_PICO;
		break;
	case XrResult::XR_ERROR_SPATIAL_ANCHOR_SHARING_LOCALIZATION_FAIL_PICO: PICOResult = EResultPICO::XR_Error_Anchor_Sharing_Localization_Fail_PICO;
		break;
	case XrResult::XR_ERROR_SPATIAL_ANCHOR_SHARING_MAP_INSUFFICIENT_PICO: PICOResult = EResultPICO::XR_Error_Anchor_Sharing_Map_Insufficient_PICO;
		break;
	case XrResult::XR_ERROR_SPATIAL_SENSING_SERVICE_UNAVAILABLE_PICO: PICOResult = EResultPICO::XR_Error_SpatialSensingServiceUnavailable_PICO;
		break;
	case XrResult::XR_ERROR_PERMISSION_INSUFFICIENT: PICOResult = EResultPICO::XR_Error_Permission_Insufficient_PICO;
		break;
	default:
		PICOResult = EResultPICO::XR_Error_Unknown_PICO;
		break;
	}
	return PICOResult;
}

uint64 FSpatialSensingExtensionPICO::GetUUID()
{
	return GlobalUUIDCount++;
}

FMixedRealityPICO::FMixedRealityPICO(): CurrentDisplayTime(0)
{
}

void FMixedRealityPICO::OnDestroySession(XrSession InSession)
{
	if (InSession != Session)
	{
		return;
	}

	if (IsHandleValid())
	{
		EResultPICO xrResult = EResultPICO::XR_Error_Unknown_PICO;
		switch (GetSenseDataProviderState())
		{
		case EMRStatePICO::Initialized:
			DestroyProvider(xrResult);
			break;
		case EMRStatePICO::Running:
			StopProvider(xrResult);
			DestroyProvider(xrResult);
			break;
		case EMRStatePICO::Stopped:
			DestroyProvider(xrResult);
			break;
		default:
			break;
		}
	}

	Session = XR_NULL_HANDLE;
	TrackingSpace = XR_NULL_HANDLE;
	CurrentDisplayTime = 0;
	XRTrackingSystem = nullptr;
}

void FMixedRealityPICO::PostCreateSession(XrSession InSession)
{
	Session = InSession;
	static FName SystemName(TEXT("OpenXR"));
	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
	{
		XRTrackingSystem = GEngine->XRSystem.Get();
	}
}

void FMixedRealityPICO::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace InTrackingSpace)
{
	CurrentDisplayTime = DisplayTime;
	TrackingSpace = InTrackingSpace;
}

bool FMixedRealityPICO::StartProvider(const FPICOPollFutureDelegate& StartSenseDataProviderDelegate, EResultPICO& OutResult)
{
	bool bResult = false;
    if (FSpatialSensingExtensionPICO::GetInstance()->IsSupportsSpatialSensing()
    	&& Session != XR_NULL_HANDLE
    	&& ProviderHandle != XR_NULL_HANDLE)
    {
    	XrSenseDataProviderStartInfoPICO cSenseDataProviderStartInfoPICO = {};
    	cSenseDataProviderStartInfoPICO.type = XR_TYPE_SENSE_DATA_PROVIDER_START_INFO_PICO;
    	cSenseDataProviderStartInfoPICO.provider = ProviderHandle;

    	XrFutureEXT ProviderFutureEXT;
    	XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrStartSenseDataProviderAsyncPICO(Session, &cSenseDataProviderStartInfoPICO, &ProviderFutureEXT);
    	bResult = XR_SUCCEEDED(xrResult);
    	OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
    	if (bResult)
    	{
    		bResult = FSpatialSensingExtensionPICO::GetInstance()->AddPollFutureRequirement(ProviderFutureEXT, StartSenseDataProviderDelegate);
    	}

    	bResult = bResult ? ProviderFutureEXT != XR_NULL_HANDLE : false;
    }
	return bResult;
}


bool FMixedRealityPICO::StopProvider(EResultPICO& OutResult)
{
	if (ProviderHandle == XR_NULL_HANDLE)
	{
		OutResult = EResultPICO::XR_Success;
		return true;
	}

	bool bResult = false;
	if (FSpatialSensingExtensionPICO::GetInstance()->IsSupportsSpatialSensing())
	{
		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrStopSenseDataProviderPICO(ProviderHandle);
		bResult = XR_SUCCEEDED(xrResult);

		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
	}
	return bResult;
}

bool FMixedRealityPICO::DestroyProvider(EResultPICO& OutResult)
{
	if (ProviderHandle == XR_NULL_HANDLE)
	{
		OutResult = EResultPICO::XR_Success;
		return true;
	}

	bool bResult = false;
	if (FSpatialSensingExtensionPICO::GetInstance()->IsSupportsSpatialSensing())
	{
		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrDestroySenseDataProviderPICO(ProviderHandle);
		bResult = XR_SUCCEEDED(xrResult);

		if (bResult)
		{
			ProviderHandle = XR_NULL_HANDLE;
		}

		OutResult = FSpatialSensingExtensionPICO::CastToPICOResult(xrResult);
	}
	return bResult;
}

EProviderTypePICO FMixedRealityPICO::GetProviderType()
{
	return Type;
}

EMRStatePICO FMixedRealityPICO::GetSenseDataProviderState() const
{
	EMRStatePICO SenseDataProviderState = EMRStatePICO::Stopped;
	if (FSpatialSensingExtensionPICO::GetInstance()->IsSupportsSpatialSensing() && ProviderHandle != XR_NULL_HANDLE)
	{
		XrSenseDataProviderStatePICO cPxrSenseDataProviderStateBD = {};
		XrResult xrResult = FSpatialSensingExtensionPICO::GetInstance()->xrGetSenseDataProviderStatePICO(ProviderHandle, &cPxrSenseDataProviderStateBD);
		SenseDataProviderState = XR_SUCCEEDED(xrResult) ? static_cast<EMRStatePICO>(cPxrSenseDataProviderStateBD) : EMRStatePICO::Stopped;
	}
	
	return SenseDataProviderState;
}

void FSpatialSensingExtensionPICO::InitializeCachedLightEstimateInfoPico()
{
#if PLATFORM_ANDROID
	CachedLightEstimationTextureVulkan= {
		.type = XR_TYPE_LIGHT_ESTIMATION_INFO_ENVIRONMENT_TEXTURE_VULKAN_PICO,
		.next = nullptr
	};

	CachedLightEstimateInfoPico.type = XR_TYPE_SPATIAL_ENTITY_LIGHT_ESTIMATION_DATA_PICO;
	CachedLightEstimateInfoPico.next = &CachedLightEstimationTextureVulkan;
	CachedLightEstimateInfoPico.state =XR_LIGHT_ESTIMATION_DATA_STATE_INVALID_PICO;
#endif
}

void FSpatialSensingExtensionPICO::UpdateCachedLightEstimateInfoPico(
	const XrSpatialEntityLightEstimationDataPICO& EntityLightEstimationInfo)
{
	CachedLightEstimateInfoPico = EntityLightEstimationInfo;
}

bool FSpatialSensingExtensionPICO::GetSpatialEntityLightEstimateData(const XrSenseDataSnapshotPICO& SnapshotHandle,
                                                                     const XrSpatialEntityIdPICO& EntityHandle,const FIntPoint& Size,ULightEstimationTexturePICO* OutEnvironmentTexture,EResultPICO& OutResult)
{
	if (OutEnvironmentTexture)
	{
		if (CachedLightEstimateInfoPico.state==XR_LIGHT_ESTIMATION_DATA_STATE_VALID_PICO)
		{
#if PLATFORM_ANDROID
			XrResult xrReleaseResult =xrReleaseEnvironmentTextureImagePICO(Session,reinterpret_cast<XrLightEstimationInfoBaseHeaderPICO*>(&CachedLightEstimationTextureVulkan));
			 UE_LOG(LogMRPICO, Verbose, TEXT("xrReleaseEnvironmentTextureImagePICO xrReleaseResult:%d"),xrReleaseResult);
			 bool bResult = XR_SUCCEEDED(xrReleaseResult);
			 if (!bResult)
			 {
			 	UE_LOG(LogMRPICO, Error, TEXT("ReleaseEnvironmentTextureImage failed"));
			 }
#endif
		}

		InitializeCachedLightEstimateInfoPico();
	}
	else
	{
		UE_LOG(LogMRPICO, Error, TEXT("OutEnvironmentTexture null!"));
		return false;
	}
	
	
	XrSpatialEntityLightEstimationGetInfoPICO xrLightEstimateGetInfoPico{
		XR_TYPE_SPATIAL_ENTITY_LIGHT_ESTIMATION_GET_INFO_PICO,
		nullptr,
		EntityHandle,
		XR_SPATIAL_ENTITY_COMPONENT_TYPE_LIGHT_ESTIMATION_PICO,
	};

	XrSpatialEntityLightEstimationDataPICO TempLightEstimateInfoPico = GetCachedLightEstimateInfoPico();

	
	 XrResult xrResult =xrGetSpatialEntityComponentDataPICO(SnapshotHandle
	 													 ,reinterpret_cast<XrSpatialEntityComponentGetInfoBaseHeaderPICO*>(&xrLightEstimateGetInfoPico)
	 													 , reinterpret_cast<XrSpatialEntityComponentDataBaseHeaderPICO*>(&TempLightEstimateInfoPico));


	if (TempLightEstimateInfoPico.state==XR_LIGHT_ESTIMATION_DATA_STATE_INVALID_PICO)
	{
		UE_LOG(LogMRPICO, Error, TEXT("XR_LIGHT_ESTIMATION_DATA_STATE_INVALID_PICO!"));
		return false;
	}
	
	bool bResult = XR_SUCCEEDED(xrResult);

	UE_LOG(LogMRPICO, Verbose, TEXT("LightEstimationTexture xrGetSpatialEntityComponentDataPICO result: %d"), bResult);
	UpdateCachedLightEstimateInfoPico(TempLightEstimateInfoPico);
	if (bResult)
	{
#if PLATFORM_ANDROID
		bResult =OutEnvironmentTexture->UpdateVulkanTextureByLightEstimateInfo(Size,GetCachedLightEstimationTextureVulkan());
		if (!bResult)
		{
			UE_LOG(LogMRPICO, Error, TEXT("UpdateCachedLightEstimateInfoPico failed!"));
		}
#endif
	}
	
	OutResult=CastToPICOResult(xrResult);

	return bResult;
}
