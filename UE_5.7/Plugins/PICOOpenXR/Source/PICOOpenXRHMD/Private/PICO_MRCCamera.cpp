// Fill out your copyright notice in the Description page of Project Settings.


#include "PICO_MRCCamera.h"
#include "PICO_HMDModule.h"
#include "Components/SceneCaptureComponent2D.h"
#include "TextureResource.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"

AMRCCameraPICO::AMRCCameraPICO(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, HMDPICO(nullptr)
	, Width(0)
	, Height(0)
	, Fov(0.0f)
	, FlipFlop(true)
{
	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> BGRef(TEXT("/Script/Engine.TextureRenderTarget2D'/PICOOpenXR/Textures/RT_BG.RT_BG'"));
	BackgroundRT = BGRef.Object;
	check(BackgroundRT != nullptr);
	BackgroundRT->RenderTargetFormat = RTF_RGBA8_SRGB;
	BackgroundRT->bAutoGenerateMips = false;

	static ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D> FGRef(TEXT("/Script/Engine.TextureRenderTarget2D'/PICOOpenXR/Textures/RT_FG.RT_FG'"));
	ForegroundRT = FGRef.Object;
	check(ForegroundRT != nullptr);
	ForegroundRT->RenderTargetFormat = RTF_RGBA8_SRGB;
	ForegroundRT->bAutoGenerateMips = false;
	
	GetCaptureComponent2D()->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	GetCaptureComponent2D()->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bAllowTickBeforeBeginPlay = false;

	UStaticMesh* StaticMeshAsset = LoadObject<UStaticMesh>(NULL, TEXT("/Script/Engine.StaticMesh'/Engine/EditorMeshes/MatineeCam_SM.MatineeCam_SM'"));

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

	if (StaticMeshComponent && StaticMeshAsset)
	{
		AddOwnedComponent(StaticMeshComponent);
		StaticMeshComponent->SetupAttachment(RootComponent);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		StaticMeshComponent->SetStaticMesh(StaticMeshAsset);
		StaticMeshComponent->SetRelativeScale3D(FVector(0.3, 0.3, 0.3));
		StaticMeshComponent->SetOwnerNoSee(true);
	}
}

void AMRCCameraPICO::BeginPlay()
{
	Super::BeginPlay();

	if ((HMDPICO = &FPICOOpenXRHMDModule::Get().GetXRPlugin()) == nullptr)
	{
		SetActorTickEnabled(false);
		return;
	}

	static FName SystemName(TEXT("OpenXR"));
	if (GEngine->XRSystem.IsValid() && (GEngine->XRSystem->GetSystemName() == SystemName))
	{
		XRTrackingSystem = GEngine->XRSystem.Get();	
	}
	else
	{
		SetActorTickEnabled(false);
		return;
	}

	if (!HMDPICO->bSupportMRCExtension || !HMDPICO->GetExternalCameraInfo(Width, Height, Fov) || !Width || !Height || !Fov)
	{
		SetActorTickEnabled(false);
		return;
	}
	else
	{
		BackgroundRT->ResizeTarget(Width, Height);
		ForegroundRT->ResizeTarget(Width, Height);

		InitializeRTRenderResource(BackgroundRT);
		InitializeRTRenderResource(ForegroundRT);

		HMDPICO->CreateMRCLayer(BackgroundRT, ForegroundRT);

		GetCaptureComponent2D()->FOVAngle = Fov * ((float)Width / (float)Height);

		DisableForegroundLayer = HMDPICO->bIsMRCForegroundLayerDisabled;
	}
}

void AMRCCameraPICO::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (HMDPICO)
	{
		HMDPICO->DestroyMRCLayer();
	}
}

void MakeProjectionMatrix(float YMultiplier, float FOV, float FarClipPlane, FMatrix& ProjectionMatrix)
{
	if (FarClipPlane < GNearClippingPlane)
	{
		FarClipPlane = GNearClippingPlane;
	}

	if ((int32)ERHIZBuffer::IsInverted)
	{
		ProjectionMatrix = FReversedZPerspectiveMatrix(
			FOV,
			FOV,
			1.0f,
			YMultiplier,
			GNearClippingPlane,
			FarClipPlane
		);
	}
	else
	{
		ProjectionMatrix = FPerspectiveMatrix(
			FOV,
			FOV,
			1.0f,
			YMultiplier,
			GNearClippingPlane,
			FarClipPlane
		);
	}
}

void AMRCCameraPICO::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FTransform MRCCameraPose = FTransform::Identity;
	if (HMDPICO->GetExternalCameraPose(MRCCameraPose))
	{
		SetActorTransform(MRCCameraPose);
	}
	else
	{
		if (GetCaptureComponent2D()->IsVisible())
		{
			GetCaptureComponent2D()->SetVisibility(false);
		}
		return;
	}

	if (DisableForegroundLayer)
	{
		if (GetCaptureComponent2D()->TextureTarget != BackgroundRT)
		{
			UpdateCamMatrixAndDepth(false);
			GetCaptureComponent2D()->TextureTarget = BackgroundRT;
		}
	}
	else
	{
		if (FlipFlop)
		{
			UpdateCamMatrixAndDepth(false);
			GetCaptureComponent2D()->TextureTarget = BackgroundRT;
			FlipFlop = false;
		}
		else
		{
			UpdateCamMatrixAndDepth(true);
			GetCaptureComponent2D()->TextureTarget = ForegroundRT;
			FlipFlop = true;
		}
	}

	if (!GetCaptureComponent2D()->IsVisible())
	{
		GetCaptureComponent2D()->SetVisibility(true);
	}
}

void AMRCCameraPICO::InitializeRTRenderResource(UTextureRenderTarget2D* RT)
{
	FTextureRenderTargetResource* RenderTargetResource = RT->GameThread_GetRenderTargetResource();

	{
		ENQUEUE_RENDER_COMMAND(FlushDeferredResourceUpdateCommand)(
			[RenderTargetResource](FRHICommandListImmediate& RHICmdList)
			{
				RenderTargetResource->FlushDeferredResourceUpdate(RHICmdList);
			});
		FlushRenderingCommands();

		RT->UpdateResourceImmediate(false);

		ENQUEUE_RENDER_COMMAND(ResetSceneTextureExtentHistory)(
			[RenderTargetResource](FRHICommandListImmediate& RHICmdList)
			{
				RenderTargetResource->ResetSceneTextureExtentsHistory();
			});
		FlushRenderingCommands();
	}
}

void AMRCCameraPICO::UpdateCamMatrixAndDepth(bool bIsForeground)
{
	FTransform HMDPose = FTransform::Identity;
	if (XRTrackingSystem)
	{
		FQuat HMDOrientation = FQuat::Identity;
		FVector HMDPosition = FVector::ZeroVector;
		FTransform TrackingToWorld = XRTrackingSystem->GetTrackingToWorldTransform();
		XRTrackingSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, HMDOrientation, HMDPosition);
		HMDPose = FTransform(HMDOrientation, HMDPosition) * TrackingToWorld;
	}

	FVector HeadToCamera = HMDPose.GetLocation() - GetActorLocation();
	float Distance = FVector::DotProduct(GetActorForwardVector().GetSafeNormal2D(), HeadToCamera);
	float ForegroundMaxDistance = FMath::Max(Distance, GMinClipZ);

	if (bIsForeground)
	{
		GetCaptureComponent2D()->MaxViewDistanceOverride = ForegroundMaxDistance;
	}
	else
	{
		GetCaptureComponent2D()->MaxViewDistanceOverride = -1;
	}

	if (bIsForeground)
	{
		float YMultiplier = (float)Width / (float)Height;
		float FOV = GetCaptureComponent2D()->FOVAngle * (float)PI / 360.0f;
		GetCaptureComponent2D()->bUseCustomProjectionMatrix = true;
		MakeProjectionMatrix(YMultiplier, FOV, ForegroundMaxDistance, GetCaptureComponent2D()->CustomProjectionMatrix);
	}
	else
	{
		GetCaptureComponent2D()->bUseCustomProjectionMatrix = false;
		GetCaptureComponent2D()->CustomProjectionMatrix.SetIdentity();
	}
}
