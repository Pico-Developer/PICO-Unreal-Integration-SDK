// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved.

#pragma once
#include "ProceduralMeshComponent.h"
#include "IStereoLayers.h"
#include "PXR_HMDRenderBridge.h"
#include "XRSwapChain.h"
#include "GameFramework/PlayerController.h"
#include "PXR_PluginWrapper.h"
#include "Components/StereoLayerComponent.h"
#include "PXR_StereoLayer.generated.h"

class FDelayDeleteLayerManager;
class FPXRGameFrame;

class FPxrLayer : public TSharedFromThis<FPxrLayer, ESPMode::ThreadSafe>
{
public:
	FPxrLayer(uint32 ID, uint32 InPxrLayerId, FDelayDeleteLayerManager* InDelayDeletion);
	~FPxrLayer();

protected:
	uint32 ID;
	uint32 PxrLayerId;
private:
	FDelayDeleteLayerManager* DelayDeletion;
};

typedef TSharedPtr<FPxrLayer, ESPMode::ThreadSafe> FPxrLayerPtr;

class FPICOXRStereoLayer : public TSharedFromThis<FPICOXRStereoLayer, ESPMode::ThreadSafe>
{
public:
	FPICOXRStereoLayer(FPICOXRHMD* InHMDDevice,uint32 InPXRLayerId, const IStereoLayers::FLayerDesc& InLayerDesc);
	FPICOXRStereoLayer(const FPICOXRStereoLayer& InPXRLayer);
	~FPICOXRStereoLayer();

	TSharedPtr<FPICOXRStereoLayer, ESPMode::ThreadSafe> CloneMyself() const;
	void SetPXRLayerDesc(const IStereoLayers::FLayerDesc& InDesc);
	const IStereoLayers::FLayerDesc& GetPXRLayerDesc() const { return LayerDesc; }
	const uint32& GetID()const{return ID;}

	bool IsLayerSupportDepth() { return (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_SUPPORT_DEPTH) != 0; }
	void ManageUnderlayComponent(bool bRatioChanged);
	void CreateUnderlayMesh(TArray<FVector>& VerticePos, TArray<int32>& TriangleIndics, TArray<FVector2D>& TexUV);

	const FXRSwapChainPtr& GetSwapChain() const { return SwapChain; }
	const FXRSwapChainPtr& GetLeftSwapChain() const { return LeftSwapChain; }
	const FXRSwapChainPtr& GetFoveationSwapChain() const { return FoveationSwapChain; }
#ifdef PICO_CUSTOM_ENGINE
	const FXRSwapChainPtr& GetMotionVectorSwapChain() const { return MotionVectorSwapChain; }
	const FXRSwapChainPtr& GetMotionVectorDepthSwapChain() const { return MotionVectorDepthSwapChain; }
#endif
	void IncrementSwapChainIndex_RHIThread(FPICOXRRenderBridge* RenderBridge);
	const void SubmitLayer_RHIThread(const FGameSettings* Settings, const FPXRGameFrame* Frame);
	int32 GetShapeType();
	void SetEyeLayerDesc(uint32 SizeX, uint32 SizeY, uint32 ArraySize, uint32 NumMips, uint32 NumSamples, FString RHIString,bool EnableSubSampled);
    void PXRLayersCopy_RenderThread(FPICOXRRenderBridge* RenderBridge, FRHICommandListImmediate& RHICmdList);
	void MarkTextureForUpdate(bool bUpdate = true) { bTextureNeedUpdate = bUpdate; }
	bool InitPXRLayer_RenderThread(const FGameSettings* Settings, FPICOXRRenderBridge* CustomPresent, FDelayDeleteLayerManager* DelayDeletion, FRHICommandListImmediate& RHICmdList, const FPICOXRStereoLayer* InLayer = nullptr);
	bool IfCanReuseLayers(const FPICOXRStereoLayer* InLayer) const;
	void ReleaseResources_RHIThread();
	bool IsVisible() { return (LayerDesc.Flags & IStereoLayers::LAYER_FLAG_HIDDEN) == 0; }
	void DestroyUnderlayMesh();
	bool bSplashLayer;
	bool bSplashBlackProjectionLayer;
	bool bMRCLayer;
	bool bNeedsTexSrgbCreate;
	void SetTrackingMode(PxrTrackingModeFlags mode) { TrackingMode = mode; }

#ifdef PICO_CUSTOM_ENGINE
	bool bEnableEyeTrackingFoveationRendering;
#endif

protected:
	FVector GetLayerLocation() const { return LayerDesc.Transform.GetLocation(); };
	FQuat GetLayerOrientation() const { return LayerDesc.Transform.Rotator().Quaternion(); };
	FVector GetLayerScale() const { return LayerDesc.Transform.GetScale3D(); };
	FPICOXRHMD* HMDDevice;
	uint32 ID;
	uint32 PxrLayerID;
	static uint32 PxrLayerIDCounter;
	IStereoLayers::FLayerDesc LayerDesc;
	FXRSwapChainPtr SwapChain;
	FXRSwapChainPtr LeftSwapChain;
	FXRSwapChainPtr FoveationSwapChain;
#ifdef PICO_CUSTOM_ENGINE
	FXRSwapChainPtr MotionVectorSwapChain;
	FXRSwapChainPtr MotionVectorDepthSwapChain;
#endif
    bool bTextureNeedUpdate;
	UProceduralMeshComponent* UnderlayMeshComponent;
	AActor* UnderlayActor;
	FPxrLayerPtr PxrLayer;
	PxrLayerParam PxrLayerCreateParam;
	PxrTrackingModeFlags TrackingMode;
};

typedef TSharedPtr<FPICOXRStereoLayer, ESPMode::ThreadSafe> FPICOLayerPtr;

struct FPICOLayerPtr_SortByPriority
{
	FORCEINLINE bool operator()(const FPICOLayerPtr&A,const FPICOLayerPtr&B)const
	{
		int32 nPriorityA = A->GetPXRLayerDesc().Priority;
		int32 nPriorityB = B->GetPXRLayerDesc().Priority;
		if (nPriorityA < nPriorityB)
		{
			return true;
		}
		else if (nPriorityA > nPriorityB)
		{
			return false;
		}
		else
		{
			return A->GetID() < B->GetID();
		}
	}
};

struct FPICOLayerPtr_SortById
{
	FORCEINLINE bool operator()(const FPICOLayerPtr& A, const FPICOLayerPtr& B) const
	{
		return A->GetID() < B->GetID();
	}
};

struct FLayerPtr_CompareByAll
{
	FORCEINLINE bool operator()(const FPICOLayerPtr& A, const FPICOLayerPtr& B) const
	{
		int32 OrderA = (A->GetID() == 0) ? 0 : A->IsLayerSupportDepth() ? -1 : 1;
		int32 OrderB = (B->GetID() == 0) ? 0 : B->IsLayerSupportDepth() ? -1 : 1;

		if (OrderA != OrderB)
		{
			return OrderA < OrderB;
		}

		const IStereoLayers::FLayerDesc& DescA = A->GetPXRLayerDesc();
		const IStereoLayers::FLayerDesc& DescB = B->GetPXRLayerDesc();

		bool bFaceLockedA = (DescA.PositionType == IStereoLayers::ELayerType::FaceLocked);
		bool bFaceLockedB = (DescB.PositionType == IStereoLayers::ELayerType::FaceLocked);

		if (bFaceLockedA != bFaceLockedB)
		{
			return bFaceLockedB;
		}

		if (DescA.Priority != DescB.Priority)
		{
			return DescA.Priority < DescB.Priority;
		}

		return A->GetID() < B->GetID();
	}
};

/** Struct containing parameters to describe how to sample an image. */
USTRUCT(BlueprintType)
struct PICOXRHMD_API FImageParameters
{
	GENERATED_BODY()

public:

	FImageParameters()
		: MinUV(FVector2f(0, 0))
		, MaxUV(FVector2f(1, 1))
	{}

	FImageParameters(FVector2f InMinUV, FVector2f InMaxUV)
		: MinUV(InMinUV)
		, MaxUV(InMaxUV)
	{}

	/** Minimum UV coordinates of Window used for sampling the Texture. */
	UPROPERTY(EditAnywhere, export, DisplayName = "Min UV",Category = "PXR|PXRHMD")
		FVector2f MinUV;

	/** Maximum UV coordinates of Window used for sampling the Texture. */
	UPROPERTY(EditAnywhere, export, DisplayName = "Max UV",Category = "PXR|PXRHMD")
		FVector2f MaxUV;
};

/** Class describing settings for a EAC (equiangular cubemap) layer shape. */
class PICOXRHMD_API FEACLayer : public IStereoLayerShape
{
	STEREO_LAYER_SHAPE_BOILERPLATE(FEACLayer)

public:
	/** Add additional shape parameters here. See "StereoLayerShapes.h" for reference. */

	FEACLayer() {}
	FEACLayer(float InScale, float InOverlapFactor, bool bInUseImageRect, FImageParameters &InLeftParams, FImageParameters &InRightParams) :
		Scale(InScale),
		OverlapFactor(InOverlapFactor),
		bUseImageRect(bInUseImageRect),
		LeftParams(InLeftParams),
		RightParams(InRightParams)
	{}

	float				Scale;
	float				OverlapFactor;
	bool				bUseImageRect;
	FImageParameters	LeftParams;
	FImageParameters	RightParams;
};

/** Class describing settings for a stereo layer shaped as an EAC (equiangular cubemap).
	This is a custom shape supported by the PicoXR Plugin. */
UCLASS(meta = (DisplayName = "EAC Layer"))
class PICOXRHMD_API UStereoLayerShapeEAC : public UStereoLayerShape
{
	GENERATED_BODY()

public:
	UStereoLayerShapeEAC() :
		Scale(10000),
		OverlapFactor(1.0f),
		bUseImageRect(false)
	{}

	/** Size of the cube mesh used for rendering the EAC. Used when "Support Depth" = true. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "EAC Properties", DisplayName = "Mesh World Scale")
		float Scale;

	/** The UV rect sampled for each face will be resized by a factor of: [1/OverlapFactor]. 
	Modifying this value can blend seams at the edges of the cubemap. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "EAC Properties")
		float OverlapFactor;

	/** If true, only the specified UV Rect will be used to sample from the Texture.
	If false, the entire Texture will be sampled in both eyes.
	To specify a custom UV Rect, see: "Left/Right Image Parameters" > "Min UV/Max UV" */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "EAC Properties", DisplayName = "Use UV Rect")
		bool bUseImageRect;

	/** Specifies parameters used for sampling the Texture for the left-eye image.
	These include a positional offset, a rotational offset, and UV Rect. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "EAC Properties", DisplayName = "Left UV Rect")
		FImageParameters LeftParams;

	/** Specifies parameters used for sampling the Texture for the right-eye image.
	These include a positional offset, a rotational offset, and UV Rect. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, export, Category = "EAC Properties", DisplayName = "Right UV Rect")
		FImageParameters RightParams;

	UFUNCTION(BlueprintCallable, Category = "Components|Stereo Layer")
		void SetScale(float InScale);

	UFUNCTION(BlueprintCallable, Category = "Components|Stereo Layer")
		void SetOverlapFactor(float InOverlapFactor);

	virtual void ApplyShape(IStereoLayers::FLayerDesc& LayerDesc) override;

#if WITH_EDITOR
	virtual void DrawShapeVisualization(const class FSceneView* View, class FPrimitiveDrawInterface* PDI) override;
#endif
};