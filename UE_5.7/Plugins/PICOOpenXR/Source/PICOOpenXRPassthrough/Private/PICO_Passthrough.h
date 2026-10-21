// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_PassthroughLayer.h"
#include "IOpenXRExtensionPlugin.h"

class FPICOOpenXRPassthrough : public IOpenXRExtensionPlugin
{
public:
	FPICOOpenXRPassthrough();
	virtual ~FPICOOpenXRPassthrough() override {}

	void Register();
	void Unregister();

	/** IOpenXRExtensionPlugin implementation */
	virtual FString GetDisplayName() override
	{
		return FString(TEXT("PICOOpenXRPassthrough"));
	}

	virtual bool GetOptionalExtensions(TArray<const ANSICHAR*>& OutExtensions) override;
	virtual const void* OnCreateSession(XrInstance InInstance, XrSystemId InSystem, const void* InNext) override;
	virtual void PostGetSystem(XrInstance InInstance, XrSystemId InSystem) override;
	virtual void PostCreateSession(XrSession InSession) override;
	virtual void UpdateDeviceLocations(XrSession InSession, XrTime DisplayTime, XrSpace TrackingSpace) override;
	virtual void OnBeginRendering_GameThread(XrSession InSession, FSceneViewFamily& InViewFamily, TArrayView<const uint32> VisibleLayers) override;
	virtual void OnBeginRendering_RenderThread(XrSession InSession, FRDGBuilder& GraphBuilder) override;
	virtual void OnDestroySession(XrSession InSession) override;

	virtual void UpdateCompositionLayers_RHIThread(XrSession InSession, TArray<XrCompositionLayerBaseHeader*>& Headers) override;

	bool GetSupportedPassthrough(bool& Support, bool& HasColor, bool& HasDepth);

	void UpdateInsightPassthrough();
	void ShutdownInsightPassthrough();
	bool IsInsightPassthroughSupportedOpenXR(bool* supported);
	bool InitializeInsightPassthroughOpenXR();
	bool ShutdownInsightPassthroughOpenXR();
	bool GetInsightPassthroughInitializedOpenXR();
	uint32 CreatePassthroughLayer(const IStereoLayers::FLayerDesc& InLayerDesc);
	bool DestroyPassthroughLayer(uint32 Id);
	bool PausePassthroughLayer(uint32 Id);
	bool ResumePassthroughLayer(uint32 ID);
	void SetLayerDesc(uint32 LayerId, const IStereoLayers::FLayerDesc& InLayerDesc);
	void UpdatePassthroughStyle_RenderThread(XrPassthroughLayerFB Layer, const FVSTEdgeStyleParametersPICO& EdgeStyleParameters);

	FORCEINLINE XrColor4f ToXrColorf(const FLinearColor LinearColor)
	{
		return XrColor4f{ LinearColor.R, LinearColor.G , LinearColor.B, LinearColor.A };
	}

	void AddPassthroughMesh_RenderThread(XrPassthroughLayerFB Layer, const TArray<FVector>& Vertices, const TArray<int32>& Triangles, FTransform Transformation, XrTriangleMeshFB* OutMeshHandle, XrGeometryInstanceFB* OutInstanceHandle);
	void UpdatePassthroughMeshTransform_RenderThread(XrGeometryInstanceFB InstanceHandle, FTransform Transformation);
	void RemovePassthroughMesh_RenderThread(XrTriangleMeshFB MeshHandle, XrGeometryInstanceFB InstanceHandle);
protected:

	struct FPassthroughLayer
	{
		XrPassthroughFB PassthroughFeature;
		XrCompositionLayerPassthroughFB PassthroughCompLayer;
		uint32 Id;
		IStereoLayers::FLayerDesc Desc;

		bool IsBackgroundLayer() const;
		bool IsOverlayLayer() const;
		bool PassthroughSupportsDepth() const;
	};

	struct FPassthroughMesh
	{
		FPassthroughMesh(XrTriangleMeshFB MeshHandle, XrGeometryInstanceFB InstanceHandle)
			: MeshHandle(MeshHandle)
			, InstanceHandle(InstanceHandle)
		{
		}
		XrTriangleMeshFB MeshHandle;
		XrGeometryInstanceFB InstanceHandle;
	};

private:
	XrSession Session;
	XrSpace BaseSpace;
	XrTime TheDisplayTime;

	TAtomic<bool>						 bIsAR = false;

	class IXRTrackingSystem* XRTrackingSystem = nullptr;

	bool bSupportPassthroughEXT;
	bool bCurrentDeviceSupportedPassthrough;
	bool bPassthroughSupportColor;
	bool bPassthroughSupportDepth;
	bool bPassthroughShowing;
	PFN_xrCreatePassthroughFB pfnXrCreatePassthroughFBX = nullptr;
	PFN_xrDestroyPassthroughFB pfnXrDestroyPassthroughFBX = nullptr;
	PFN_xrPassthroughStartFB pfnXrPassthroughStartFBX = nullptr;
	PFN_xrPassthroughPauseFB pfnXrPassthroughPauseFBX = nullptr;
	PFN_xrCreatePassthroughLayerFB pfnXrCreatePassthroughLayerFBX = nullptr;
	PFN_xrDestroyPassthroughLayerFB pfnDestroyPassthroughLayerFBX = nullptr;
	PFN_xrPassthroughLayerPauseFB pfnXrPassthroughLayerPauseFBX = nullptr;
	PFN_xrPassthroughLayerResumeFB pfnXrPassthroughLayerResumeFBX = nullptr;
	PFN_xrPassthroughLayerSetStyleFB pfnXrPassthroughLayerSetStyleFBX = nullptr;
	PFN_xrCreateGeometryInstanceFB pfnXrCreateGeometryInstanceF = nullptr;
	PFN_xrDestroyGeometryInstanceFB pfnXrDestroyGeometryInstanceFB = nullptr;
	PFN_xrGeometryInstanceSetTransformFB pfnXrGeometryInstanceSetTransformFB = nullptr;
	XrPassthroughFB passthroughFeature = XR_NULL_HANDLE;

	PFN_xrCreateTriangleMeshFB pfnXrCreateTriangleMeshFB = nullptr;
	PFN_xrDestroyTriangleMeshFB pfnXrDestroyTriangleMeshFB = nullptr;

	enum class FInsightInitStatus
	{
		NotInitialized,
		Initialized,
		Failed,
	};

	FInsightInitStatus InsightInitStatus;

	uint32 NextLayerIndex = 0;
	TMap<uint32, FPassthroughLayer> passthroughCompLayers;

	TArray<FPassthroughLayer> passthroughCompLayers_RenderThread;
	TArray<FPassthroughLayer> passthroughCompLayers_RHIThread;

	typedef TSharedPtr<TMap<FString, FPassthroughMesh>, ESPMode::ThreadSafe> FCustomShapeGeometryMapPtr;
	FCustomShapeGeometryMapPtr CustomShapeGeometryMap;
};
