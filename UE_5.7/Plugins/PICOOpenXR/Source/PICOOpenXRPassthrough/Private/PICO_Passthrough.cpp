// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICO_Passthrough.h"
#include "PICO_PassthroughModule.h"
#include "PICOOpenXRRuntimeSettings.h"
#include "IXRTrackingSystem.h"
#include "Engine/Engine.h"
#include "OpenXRCore.h"
#include "IOpenXRHMDModule.h"

#if PLATFORM_ANDROID
#include <dlfcn.h> 
#endif //PLATFORM_ANDROID

FPICOOpenXRPassthrough::FPICOOpenXRPassthrough()
	: Session(XR_NULL_HANDLE)
	, bSupportPassthroughEXT(false)
{
}

void FPICOOpenXRPassthrough::Register()
{
	RegisterOpenXRExtensionModularFeature();
}

void FPICOOpenXRPassthrough::Unregister()
{
	UnregisterOpenXRExtensionModularFeature();
}

bool FPICOOpenXRPassthrough::GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions)
{
	if (UPICOOpenXRRuntimeSettings::GetBoolConfigByKey(TEXT("bEnableVST")))
	{
		OutExtensions.Add(XR_FB_PASSTHROUGH_EXTENSION_NAME);
		OutExtensions.Add(XR_FB_TRIANGLE_MESH_EXTENSION_NAME);

		return true;
	}
	return false;
}

void FPICOOpenXRPassthrough::PostGetSystem(XrInstance InInstance, XrSystemId InSystem)
{
	bSupportPassthroughEXT = IOpenXRHMDModule::Get().IsExtensionEnabled(XR_FB_PASSTHROUGH_EXTENSION_NAME) && IOpenXRHMDModule::Get().IsExtensionEnabled(XR_FB_TRIANGLE_MESH_EXTENSION_NAME);
	if (bSupportPassthroughEXT)
	{
		XrSystemPassthroughProperties2FB passthroughSystemProperties{ XR_TYPE_SYSTEM_PASSTHROUGH_PROPERTIES2_FB };
		XrSystemProperties systemProperties{ XR_TYPE_SYSTEM_PROPERTIES, &passthroughSystemProperties };
		xrGetSystemProperties(InInstance, InSystem, &systemProperties);

		bCurrentDeviceSupportedPassthrough = (passthroughSystemProperties.capabilities & XR_PASSTHROUGH_CAPABILITY_BIT_FB) == XR_PASSTHROUGH_CAPABILITY_BIT_FB;
		bPassthroughSupportColor = bCurrentDeviceSupportedPassthrough && (passthroughSystemProperties.capabilities & XR_PASSTHROUGH_CAPABILITY_COLOR_BIT_FB) == XR_PASSTHROUGH_CAPABILITY_COLOR_BIT_FB;
		bPassthroughSupportDepth = bCurrentDeviceSupportedPassthrough && (passthroughSystemProperties.capabilities & XR_PASSTHROUGH_CAPABILITY_LAYER_DEPTH_BIT_FB) == XR_PASSTHROUGH_CAPABILITY_LAYER_DEPTH_BIT_FB;
		UE_LOG(LogPassthroughPICO, Verbose, TEXT("bCurrentDeviceSupportedPassthrough:%d,bPassthroughSupportColor:%d,bPassthroughSupportDepth:%d"), bCurrentDeviceSupportedPassthrough, bPassthroughSupportColor, bPassthroughSupportDepth);
		if (bCurrentDeviceSupportedPassthrough)
		{
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreatePassthroughFB", (PFN_xrVoidFunction*)(&pfnXrCreatePassthroughFBX)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyPassthroughFB", (PFN_xrVoidFunction*)(&pfnXrDestroyPassthroughFBX)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrPassthroughStartFB", (PFN_xrVoidFunction*)(&pfnXrPassthroughStartFBX)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrPassthroughPauseFB", (PFN_xrVoidFunction*)(&pfnXrPassthroughPauseFBX)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreatePassthroughLayerFB", (PFN_xrVoidFunction*)(&pfnXrCreatePassthroughLayerFBX)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyPassthroughLayerFB", (PFN_xrVoidFunction*)(&pfnDestroyPassthroughLayerFBX)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrPassthroughLayerPauseFB", (PFN_xrVoidFunction*)(&pfnXrPassthroughLayerPauseFBX)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrPassthroughLayerResumeFB", (PFN_xrVoidFunction*)(&pfnXrPassthroughLayerResumeFBX)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrPassthroughLayerSetStyleFB", (PFN_xrVoidFunction*)(&pfnXrPassthroughLayerSetStyleFBX)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateGeometryInstanceFB", (PFN_xrVoidFunction*)(&pfnXrCreateGeometryInstanceF)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyGeometryInstanceFB", (PFN_xrVoidFunction*)(&pfnXrDestroyGeometryInstanceFB)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrGeometryInstanceSetTransformFB", (PFN_xrVoidFunction*)(&pfnXrGeometryInstanceSetTransformFB)));

			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrCreateTriangleMeshFB", (PFN_xrVoidFunction*)(&pfnXrCreateTriangleMeshFB)));
			XR_ENSURE(xrGetInstanceProcAddr(InInstance, "xrDestroyTriangleMeshFB", (PFN_xrVoidFunction*)(&pfnXrDestroyTriangleMeshFB)));
		}
	}
}

const void* FPICOOpenXRPassthrough::OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext)
{
	static FName SystemName(TEXT("OpenXR"));
	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
	{
		XRTrackingSystem = GEngine->XRSystem.Get();
	}

	return InNext;
}

void FPICOOpenXRPassthrough::PostCreateSession(XrSession InSession)
{
	Session = InSession;
}

void FPICOOpenXRPassthrough::UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace)
{
	BaseSpace = TrackingSpace;
	TheDisplayTime = DisplayTime;

	if (IsInGameThread())
	{
		ENQUEUE_RENDER_COMMAND(UpdatePassthroughLayers)(
			[this, PassthroughLayers = passthroughCompLayers](FRHICommandListImmediate&)
			{
				passthroughCompLayers_RenderThread.Empty(PassthroughLayers.Num());
				for (auto Pair : PassthroughLayers)
				{
					passthroughCompLayers_RenderThread.Emplace(Pair.Value);
				}
			});
	}
}

void FPICOOpenXRPassthrough::OnBeginRendering_GameThread(XrSession InSession, FSceneViewFamily& InViewFamily, TArrayView<const uint32> VisibleLayers)
{
	bIsAR = (XRTrackingSystem->GetXRSystemFlags() & EXRSystemFlags::IsAR) != 0;
	if (IConsoleVariable* OpenXREnvironmentBlendMode = IConsoleManager::Get().FindConsoleVariable(TEXT("xr.OpenXREnvironmentBlendMode")))
	{
		if (OpenXREnvironmentBlendMode->GetInt() > 1)//BlendModeOverride and not opaque
		{
			bIsAR = true;
		}
	}
}

void FPICOOpenXRPassthrough::OnBeginRendering_RenderThread(XrSession InSession, FRDGBuilder& GraphBuilder)
{
	if (InsightInitStatus == FInsightInitStatus::Initialized)
	{
		for (const auto& layer : passthroughCompLayers_RenderThread)
		{
			if (layer.Desc.HasShape<FFullScreenLayerShapePICO>())
			{
				const FFullScreenLayerShapePICO& FullScreenLayerProps = layer.Desc.GetShape<FFullScreenLayerShapePICO>();
				UpdatePassthroughStyle_RenderThread(layer.PassthroughCompLayer.layerHandle, FullScreenLayerProps.EdgeStyleParameters);
			}
			else if (layer.Desc.HasShape<FCustomShapeLayerShapePICO>())
			{
				const FCustomShapeLayerShapePICO& CustomShapeLayerProps = layer.Desc.GetShape<FCustomShapeLayerShapePICO>();
				UpdatePassthroughStyle_RenderThread(layer.PassthroughCompLayer.layerHandle, CustomShapeLayerProps.EdgeStyleParameters);
			}

			if (!CustomShapeGeometryMap)
			{
				CustomShapeGeometryMap = MakeShared<TMap<FString, FPassthroughMesh>, ESPMode::ThreadSafe>();
			}

			if (layer.Desc.HasShape<FCustomShapeLayerShapePICO>() && BaseSpace != XR_NULL_HANDLE)
			{
				const FCustomShapeLayerShapePICO& CustomShapeLayerProps = layer.Desc.GetShape<FCustomShapeLayerShapePICO>();
				const TArray<FCustomShapeGeometryDescPICO>& UserGeometryList = CustomShapeLayerProps.UserGeometryList;
				TSet<FString> UsedSet;

				for (const FCustomShapeGeometryDescPICO& GeometryDesc : UserGeometryList)
				{
					const FString MeshName = GeometryDesc.MeshName;
					UsedSet.Add(MeshName);

					FPassthroughMesh* LayerPassthroughMesh = CustomShapeGeometryMap->Find(MeshName);
					if (!LayerPassthroughMesh)
					{
						FPICOPassthroughMeshRef GeomPassthroughMesh = GeometryDesc.PassthroughMesh;
						if (GeomPassthroughMesh)
						{
							XrTriangleMeshFB MeshHandle = 0;
							XrGeometryInstanceFB InstanceHandle = 0;

							AddPassthroughMesh_RenderThread(layer.PassthroughCompLayer.layerHandle, GeomPassthroughMesh->GetVertices(), GeomPassthroughMesh->GetTriangles(), GeometryDesc.Transform, &MeshHandle, &InstanceHandle);
							CustomShapeGeometryMap->Add(MeshName, FPassthroughMesh(MeshHandle, InstanceHandle));
						}
					}
					else if (GeometryDesc.bUpdateTransform)
					{
						UpdatePassthroughMeshTransform_RenderThread(LayerPassthroughMesh->InstanceHandle, GeometryDesc.Transform);
					}
				}

				TArray<FString> ItemsToRemove;
				for (auto& Entry : *CustomShapeGeometryMap)
				{
					if (!UsedSet.Contains(Entry.Key))
					{
						ItemsToRemove.Add(Entry.Key);
					}
				}

				for (FString Entry : ItemsToRemove)
				{
					FPassthroughMesh* PassthroughMesh = CustomShapeGeometryMap->Find(Entry);
					if (PassthroughMesh)
					{
						RemovePassthroughMesh_RenderThread(PassthroughMesh->MeshHandle, PassthroughMesh->InstanceHandle);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("PassthroughMesh: %s doesn't exist."), *Entry);
						return;
					}

					CustomShapeGeometryMap->Remove(Entry);
				}
			}
		}

		FRHICommandListImmediate& RHICmdList = FRHICommandListImmediate::Get();
		RHICmdList.EnqueueLambda([this, passthroughLayers_RenderThread = passthroughCompLayers_RenderThread](FRHICommandListImmediate&)
			{
				passthroughCompLayers_RHIThread = passthroughLayers_RenderThread;
			});
	}
}

void FPICOOpenXRPassthrough::UpdatePassthroughStyle_RenderThread(XrPassthroughLayerFB LayerHandle, const FVSTEdgeStyleParametersPICO& EdgeStyleParameters)
{
	XrPassthroughStyleFB Style = { XR_TYPE_PASSTHROUGH_STYLE_FB };
	Style.edgeColor = XrColor4f{ 0 , 0 , 0 , 0 };
	Style.textureOpacityFactor = EdgeStyleParameters.TextureOpacityFactor;

	if (EdgeStyleParameters.bEnableEdgeColor)
	{
		Style.edgeColor = ToXrColorf(EdgeStyleParameters.EdgeColor);
	}

	XrPassthroughColorMapMonoToRgbaFB MonoToRgba = { XR_TYPE_PASSTHROUGH_COLOR_MAP_MONO_TO_RGBA_FB };
	XrPassthroughColorMapMonoToMonoFB MonoToMono = { XR_TYPE_PASSTHROUGH_COLOR_MAP_MONO_TO_MONO_FB };
	XrPassthroughBrightnessContrastSaturationFB BrightnessContrastSaturation = { XR_TYPE_PASSTHROUGH_BRIGHTNESS_CONTRAST_SATURATION_FB };

	if (EdgeStyleParameters.bEnableColorMap)
	{
		switch (EdgeStyleParameters.ColorMapType)
		{
		case PassthroughColorMapType::NoneStyle:
			break;
		case PassthroughColorMapType::MonoToRgba:
			FMemory::Memcpy(MonoToRgba.textureColorMap, EdgeStyleParameters.ColorMapData.GetData(), EdgeStyleParameters.ColorMapData.Num() * sizeof(uint8));
			Style.next = &MonoToRgba;
			break;
		case PassthroughColorMapType::MonoToMono:
			FMemory::Memcpy(MonoToMono.textureColorMap, EdgeStyleParameters.ColorMapData.GetData(), EdgeStyleParameters.ColorMapData.Num() * sizeof(uint8));
			Style.next = &MonoToMono;
			break;
		case PassthroughColorMapType::HandsContrast:
			break;
		case PassthroughColorMapType::BrightnessContrastSaturation:
			BrightnessContrastSaturation.brightness = EdgeStyleParameters.Brightness * 100.0f;
			BrightnessContrastSaturation.contrast = EdgeStyleParameters.Contrast;
			BrightnessContrastSaturation.saturation = EdgeStyleParameters.Saturation;

			Style.next = &BrightnessContrastSaturation;
			break;
		default:
			break;
		}
	}

	XrResult result = pfnXrPassthroughLayerSetStyleFBX(LayerHandle, &Style);
	if (XR_FAILED(result))
	{
		UE_LOG(LogPassthroughPICO, Warning, TEXT("Failed setting passthrough style"));
		return;
	}
}

void FPICOOpenXRPassthrough::AddPassthroughMesh_RenderThread(XrPassthroughLayerFB Layer, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, FTransform Transformation, XrTriangleMeshFB* OutMeshHandle, XrGeometryInstanceFB* OutInstanceHandle)
{
	check(IsInRenderingThread());

	XrTriangleMeshFB MeshHandle = 0;
	XrGeometryInstanceFB InstanceHandle = 0;

	XrTriangleMeshCreateInfoFB MeshCreateInfo = { XR_TYPE_TRIANGLE_MESH_CREATE_INFO_FB };
	MeshCreateInfo.vertexCount = Vertices.Num();

	float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();

	TArray<XrVector3f> v;
	v.Reserve(Vertices.Num());
	for (int i = 0; i < Vertices.Num(); i++)
	{
		v.Add(ToXrVector(Vertices[i], WorldToMetersScale));
	}
	MeshCreateInfo.vertexBuffer = v.GetData();

	MeshCreateInfo.triangleCount = Triangles.Num() / 3;
	MeshCreateInfo.indexBuffer = (uint32_t*)Triangles.GetData();
	MeshCreateInfo.windingOrder = XR_WINDING_ORDER_CW_FB;

	if (XR_FAILED(pfnXrCreateTriangleMeshFB(Session, &MeshCreateInfo, &MeshHandle)))
	{
		UE_LOG(LogPassthroughPICO, Error, TEXT("Failed creating passthrough mesh surface."));
		return;
	}
	UE_LOG(LogPassthroughPICO, Log, TEXT("Succeed creating passthrough mesh surface."));

	FTransform TransformTrackingSpace = Transformation * XRTrackingSystem->GetTrackingToWorldTransform().Inverse();

	XrGeometryInstanceCreateInfoFB CreateInfo = { XR_TYPE_GEOMETRY_INSTANCE_CREATE_INFO_FB };
	CreateInfo.layer = Layer;
	CreateInfo.mesh = MeshHandle;
	CreateInfo.pose = ToXrPose(TransformTrackingSpace, WorldToMetersScale);
	CreateInfo.scale = ToXrVector(TransformTrackingSpace.GetScale3D());
	CreateInfo.baseSpace = BaseSpace;
	if (XR_FAILED(pfnXrCreateGeometryInstanceF(Session, &CreateInfo, &InstanceHandle)))
	{
		UE_LOG(LogPassthroughPICO, Error, TEXT("Failed adding passthrough mesh surface to scene."));
		return;
	}

	UE_LOG(LogPassthroughPICO, Log, TEXT("Succeed adding passthrough mesh surface to scene."));

	*OutMeshHandle = MeshHandle;
	*OutInstanceHandle = InstanceHandle;
}

void FPICOOpenXRPassthrough::UpdatePassthroughMeshTransform_RenderThread(XrGeometryInstanceFB InstanceHandle, FTransform Transformation)
{
	check(IsInRenderingThread());

	if (InstanceHandle == XR_NULL_HANDLE || XRTrackingSystem == nullptr)
	{
		return;
	}

	float WorldToMetersScale = XRTrackingSystem->GetWorldToMetersScale();
	FTransform TransformTrackingSpace = Transformation * XRTrackingSystem->GetTrackingToWorldTransform().Inverse();

	XrGeometryInstanceTransformFB  PoseInfo = { XR_TYPE_GEOMETRY_INSTANCE_TRANSFORM_FB };
	PoseInfo.baseSpace = BaseSpace;
	PoseInfo.time = TheDisplayTime;
	PoseInfo.pose = ToXrPose(TransformTrackingSpace, WorldToMetersScale);
	PoseInfo.scale = ToXrVector(TransformTrackingSpace.GetScale3D());

	if (XR_FAILED(pfnXrGeometryInstanceSetTransformFB(InstanceHandle, &PoseInfo)))
	{
		UE_LOG(LogPassthroughPICO, Error, TEXT("Failed updating passthrough mesh surface transform."));
		return;
	}
}

void FPICOOpenXRPassthrough::RemovePassthroughMesh_RenderThread(XrTriangleMeshFB MeshHandle, XrGeometryInstanceFB InstanceHandle)
{
	check(IsInRenderingThread());

	if (XR_FAILED(pfnXrDestroyGeometryInstanceFB(InstanceHandle)))
	{
		UE_LOG(LogPassthroughPICO, Error, TEXT("Failed removing passthrough surface from scene."));
		return;
	}

	if (XR_FAILED(pfnXrDestroyTriangleMeshFB(MeshHandle)))
	{
		UE_LOG(LogPassthroughPICO, Error, TEXT("Failed destroying passthrough surface mesh."));
		return;
	}
}

void FPICOOpenXRPassthrough::OnDestroySession(XrSession InSession)
{
	ShutdownInsightPassthrough();
}

void FPICOOpenXRPassthrough::UpdateCompositionLayers_RHIThread(XrSession InSession, TArray<XrCompositionLayerBaseHeader*>& Headers)
{
	if (bCurrentDeviceSupportedPassthrough && passthroughCompLayers_RHIThread.Num() > 0)
	{
		int InsertOrder = 0;
		for (int i = 0; i < passthroughCompLayers_RHIThread.Num(); i++)
		{
			if (passthroughCompLayers_RHIThread[i].IsBackgroundLayer() || passthroughCompLayers_RHIThread[i].PassthroughSupportsDepth())
			{
				Headers.Insert(reinterpret_cast<XrCompositionLayerBaseHeader*>(&(passthroughCompLayers_RHIThread[i].PassthroughCompLayer)), InsertOrder++);
			}
			else if(passthroughCompLayers_RHIThread[i].IsOverlayLayer())
			{
				Headers.Add(reinterpret_cast<XrCompositionLayerBaseHeader*>(&(passthroughCompLayers_RHIThread[i].PassthroughCompLayer)));
			}
		}
	}
}

bool FPICOOpenXRPassthrough::GetSupportedPassthrough(bool& Support, bool& HasColor, bool& HasDepth)
{
	Support = bCurrentDeviceSupportedPassthrough;
	HasColor = bPassthroughSupportColor;
	HasDepth = bPassthroughSupportDepth;
	return bSupportPassthroughEXT;
}

void FPICOOpenXRPassthrough::UpdateInsightPassthrough()
{
	const bool bShouldEnable = (InsightInitStatus == FInsightInitStatus::NotInitialized) &&
		(GetMutableDefault<UPICOOpenXRRuntimeSettings>()->bEnableVST);

	if (bShouldEnable)
	{
		if (InitializeInsightPassthroughOpenXR())
		{
			UE_LOG(LogPassthroughPICO, Log, TEXT("Passthrough Initialized"));
			InsightInitStatus = FInsightInitStatus::Initialized;
		}
		else
		{
			InsightInitStatus = FInsightInitStatus::Failed;
			UE_LOG(LogPassthroughPICO, Log, TEXT("Passthrough initialization failed"));
		}
	}
	else
	{
		const bool bShouldShutdown = (InsightInitStatus == FInsightInitStatus::Initialized) &&
			(!GetMutableDefault<UPICOOpenXRRuntimeSettings>()->bEnableVST);
		if (bShouldShutdown)
		{
			ShutdownInsightPassthrough();
		}
	}
}

void FPICOOpenXRPassthrough::ShutdownInsightPassthrough()
{
	if (InsightInitStatus == FInsightInitStatus::Initialized)
	{
		// it may already be deinitialized.
		if (!GetInsightPassthroughInitializedOpenXR() || ShutdownInsightPassthroughOpenXR())
		{
			UE_LOG(LogPassthroughPICO, Log, TEXT("Passthrough shutdown"));
			InsightInitStatus = FInsightInitStatus::NotInitialized;
		}
		else
		{
			UE_LOG(LogPassthroughPICO, Log, TEXT("Failed to shut down passthrough. It may be still in use."));
		}
	}
}

bool FPICOOpenXRPassthrough::IsInsightPassthroughSupportedOpenXR(bool* supported)
{
	if (supported)
	{
		*supported = bCurrentDeviceSupportedPassthrough;
		return true;
	}
	return false;
}

bool FPICOOpenXRPassthrough::InitializeInsightPassthroughOpenXR()
{
	if (Session != XR_NULL_HANDLE && bCurrentDeviceSupportedPassthrough)
	{
		XrPassthroughCreateInfoFB passthroughCreateInfo = { XR_TYPE_PASSTHROUGH_CREATE_INFO_FB };
		passthroughCreateInfo.flags = XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB;

		XrResult result = pfnXrCreatePassthroughFBX(Session, &passthroughCreateInfo, &passthroughFeature);
		if (XR_FAILED(result))
		{
			UE_LOG(LogPassthroughPICO, Warning, TEXT("Failed to create the passthrough feature."));
			return false;
		}
		return true;
	}
	return false;
}

bool FPICOOpenXRPassthrough::ShutdownInsightPassthroughOpenXR()
{
	if (bCurrentDeviceSupportedPassthrough && passthroughFeature)
	{
		pfnXrDestroyPassthroughFBX(passthroughFeature);
		return true;
	}
	return false;
}

bool FPICOOpenXRPassthrough::GetInsightPassthroughInitializedOpenXR()
{
	if (bCurrentDeviceSupportedPassthrough && passthroughFeature)
	{
		return true;
	}
	return false;
}

uint32 FPICOOpenXRPassthrough::CreatePassthroughLayer(const IStereoLayers::FLayerDesc& InLayerDesc)
{
	uint32 id = 0;

	UpdateInsightPassthrough();

	if (GetInsightPassthroughInitializedOpenXR())
	{
		XrPassthroughLayerFB passthroughLayer = XR_NULL_HANDLE;

		XrPassthroughLayerCreateInfoFB layerCreateInfo = { XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB };
		layerCreateInfo.passthrough = passthroughFeature;
		if (InLayerDesc.HasShape<FFullScreenLayerShapePICO>())
		{
			layerCreateInfo.purpose = XR_PASSTHROUGH_LAYER_PURPOSE_RECONSTRUCTION_FB;
		}
		else if (InLayerDesc.HasShape<FCustomShapeLayerShapePICO>())
		{
			layerCreateInfo.purpose = XR_PASSTHROUGH_LAYER_PURPOSE_PROJECTED_FB;
		}
		else
		{
			UE_LOG(LogPassthroughPICO, Warning, TEXT("No available passthrough layer shape!"));
			return id;
		}
		layerCreateInfo.flags = XR_PASSTHROUGH_IS_RUNNING_AT_CREATION_BIT_FB;

		XrResult result = pfnXrCreatePassthroughLayerFBX(Session, &layerCreateInfo, &passthroughLayer);
		if (XR_FAILED(result))
		{
			UE_LOG(LogPassthroughPICO, Error, TEXT("Failed to create a passthrough layer"));
			return id;
		}

		UE_LOG(LogPassthroughPICO, Log, TEXT("Succeed to create a passthrough layer"));

		id = ++NextLayerIndex;//start from 1

		FPassthroughLayer PassthroughLayer;
		PassthroughLayer.Id = id;
		PassthroughLayer.Desc = InLayerDesc;
		PassthroughLayer.PassthroughFeature = passthroughFeature;
		PassthroughLayer.PassthroughCompLayer = { XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB };
		PassthroughLayer.PassthroughCompLayer.layerHandle = passthroughLayer;
		PassthroughLayer.PassthroughCompLayer.flags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
		PassthroughLayer.PassthroughCompLayer.space = XR_NULL_HANDLE;

		passthroughCompLayers.Add(id, PassthroughLayer);
	}
	return id;
}

bool FPICOOpenXRPassthrough::DestroyPassthroughLayer(uint32 Id)
{
	FPassthroughLayer* LayerFound = passthroughCompLayers.Find(Id);
	if (LayerFound && (*LayerFound).PassthroughCompLayer.layerHandle != XR_NULL_HANDLE)
	{
		XrResult result = pfnDestroyPassthroughLayerFBX((*LayerFound).PassthroughCompLayer.layerHandle);
		if (XR_FAILED(result))
		{
			UE_LOG(LogPassthroughPICO, Warning, TEXT("Failed to destroy a passthrough layer Id:%u"), Id);
			return false;
		}

		passthroughCompLayers.Remove(Id);
		UE_LOG(LogPassthroughPICO, Log, TEXT("Succeed to destroy a passthrough layer Id:%u"), Id);
		return true;
	}
	return false;
}

bool FPICOOpenXRPassthrough::PausePassthroughLayer(uint32 Id)
{
	FPassthroughLayer* LayerFound = passthroughCompLayers.Find(Id);
	if (LayerFound && (*LayerFound).PassthroughCompLayer.layerHandle != XR_NULL_HANDLE)
	{
		XrResult result = pfnXrPassthroughLayerPauseFBX((*LayerFound).PassthroughCompLayer.layerHandle);
		if (XR_FAILED(result))
		{
			UE_LOG(LogPassthroughPICO, Warning, TEXT("Failed to pause a passthrough layer Id:%u"), Id);
			return false;
		}
		return true;
	}
	return false;
}

bool FPICOOpenXRPassthrough::ResumePassthroughLayer(uint32 ID)
{
	FPassthroughLayer* LayerFound = passthroughCompLayers.Find(ID);
	if (LayerFound && (*LayerFound).PassthroughCompLayer.layerHandle != XR_NULL_HANDLE)
	{
		XrResult result = pfnXrPassthroughLayerResumeFBX((*LayerFound).PassthroughCompLayer.layerHandle);
		if (XR_FAILED(result))
		{
			UE_LOG(LogPassthroughPICO, Warning, TEXT("Failed to resume a passthrough layer Id:%u"), ID);
			return false;
		}
		return true;
	}
	return false;
}

void FPICOOpenXRPassthrough::SetLayerDesc(uint32 LayerId, const IStereoLayers::FLayerDesc& InLayerDesc)
{
	FPassthroughLayer* LayerFound = passthroughCompLayers.Find(LayerId);

	if (LayerFound)
	{
		(*LayerFound).Desc = InLayerDesc;
	}
}

bool FPICOOpenXRPassthrough::FPassthroughLayer::IsBackgroundLayer() const
{
	return (Desc.HasShape<FFullScreenLayerShapePICO>() && (Desc.GetShape<FFullScreenLayerShapePICO>().PassthroughLayerOrder == PassthroughLayerOrder_Underlay))
		|| (Desc.HasShape<FCustomShapeLayerShapePICO>() && (Desc.GetShape<FCustomShapeLayerShapePICO>().PassthroughLayerOrder == PassthroughLayerOrder_Underlay));
}

bool FPICOOpenXRPassthrough::FPassthroughLayer::IsOverlayLayer() const
{
	return (Desc.HasShape<FFullScreenLayerShapePICO>() && (Desc.GetShape<FFullScreenLayerShapePICO>().PassthroughLayerOrder == PassthroughLayerOrder_Overlay))
		|| (Desc.HasShape<FCustomShapeLayerShapePICO>() && (Desc.GetShape<FCustomShapeLayerShapePICO>().PassthroughLayerOrder == PassthroughLayerOrder_Overlay));
}

bool FPICOOpenXRPassthrough::FPassthroughLayer::PassthroughSupportsDepth() const
{
	return ((Desc.Flags & IStereoLayers::LAYER_FLAG_SUPPORT_DEPTH) != 0) && Desc.HasShape<FCustomShapeLayerShapePICO>();
}
