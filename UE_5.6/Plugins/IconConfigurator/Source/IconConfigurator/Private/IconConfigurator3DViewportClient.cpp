// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "IconConfigurator3DViewportClient.h"

#include "PreviewScene.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialExpressionTextureSampleParameter2D.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Components/StaticMeshComponent.h"

// Plane spacing in Unreal units
static constexpr float PlaneSpacing = 10.f;

// ---- Helper: create an Unlit translucent material with a texture parameter in code ----
static UMaterial* CreateIconConfiguratorMaterial()
{
	UMaterial* Mat = NewObject<UMaterial>(
		GetTransientPackage(),
		NAME_None,
		RF_Transient);

	Mat->BlendMode         = BLEND_Translucent;
	Mat->SetShadingModel(MSM_Unlit);
	Mat->TwoSided          = true;

	// Create Texture2D parameter node
	UMaterialExpressionTextureSampleParameter2D* TexParam =
		NewObject<UMaterialExpressionTextureSampleParameter2D>(Mat);
	TexParam->ParameterName              = TEXT("BaseColor");
	TexParam->SamplerType                = SAMPLERTYPE_Color;
	TexParam->MaterialExpressionEditorX  = -300;
	TexParam->MaterialExpressionEditorY  = 0;

	// Correct texture orientation: after plane rotation, UV axes are rotated/flipped in camera view.
	// By derivation: the correct sampling coordinates are (1-V_mesh, U_mesh),
	// i.e. negate V and use it as the horizontal axis, use U directly as the vertical axis.

	UMaterialExpressionTextureCoordinate* RawUV =
		NewObject<UMaterialExpressionTextureCoordinate>(Mat);
	RawUV->UTiling = 1.0f;
	RawUV->VTiling = 1.0f;
	RawUV->MaterialExpressionEditorX = -700;
	RawUV->MaterialExpressionEditorY =  50;
	Mat->GetExpressionCollection().AddExpression(RawUV);

	// Extract V component (G channel)
	UMaterialExpressionComponentMask* ExtractV =
		NewObject<UMaterialExpressionComponentMask>(Mat);
	ExtractV->R = false; ExtractV->G = true; ExtractV->B = false; ExtractV->A = false;
	ExtractV->Input.Connect(0, RawUV);
	ExtractV->MaterialExpressionEditorX = -550;
	ExtractV->MaterialExpressionEditorY =   0;
	Mat->GetExpressionCollection().AddExpression(ExtractV);

	// 1 - V
	UMaterialExpressionOneMinus* OneMinusV =
		NewObject<UMaterialExpressionOneMinus>(Mat);
	OneMinusV->Input.Connect(0, ExtractV);
	OneMinusV->MaterialExpressionEditorX = -400;
	OneMinusV->MaterialExpressionEditorY =   0;
	Mat->GetExpressionCollection().AddExpression(OneMinusV);

	// Extract U component (R channel)
	UMaterialExpressionComponentMask* ExtractU =
		NewObject<UMaterialExpressionComponentMask>(Mat);
	ExtractU->R = true; ExtractU->G = false; ExtractU->B = false; ExtractU->A = false;
	ExtractU->Input.Connect(0, RawUV);
	ExtractU->MaterialExpressionEditorX = -550;
	ExtractU->MaterialExpressionEditorY = 150;
	Mat->GetExpressionCollection().AddExpression(ExtractU);

	// 1 - U
	UMaterialExpressionOneMinus* OneMinusU =
		NewObject<UMaterialExpressionOneMinus>(Mat);
	OneMinusU->Input.Connect(0, ExtractU);
	OneMinusU->MaterialExpressionEditorX = -400;
	OneMinusU->MaterialExpressionEditorY = 150;
	Mat->GetExpressionCollection().AddExpression(OneMinusU);

	// Compose (1-V, 1-U) as the final UV
	UMaterialExpressionAppendVector* FinalUV =
		NewObject<UMaterialExpressionAppendVector>(Mat);
	FinalUV->A.Connect(0, OneMinusV); // x = 1-V
	FinalUV->B.Connect(0, OneMinusU); // y = 1-U
	FinalUV->MaterialExpressionEditorX = -220;
	FinalUV->MaterialExpressionEditorY =  50;
	Mat->GetExpressionCollection().AddExpression(FinalUV);

	TexParam->Coordinates.Connect(0, FinalUV);

	// Add to material expression collection
	Mat->GetExpressionCollection().AddExpression(TexParam);

	// UE5.4+ API: property connections are accessed via GetEditorOnlyData()
	// Use auto* to deduce return type, avoiding an explicit include of UMaterialEditorOnlyData.
	if (auto* EditorData = Mat->GetEditorOnlyData())
	{
		// TexParam output indices: 0=RGBA, 1=R, 2=G, 3=B, 4=A
		EditorData->EmissiveColor.Connect(0, TexParam);
		EditorData->Opacity.Connect(4, TexParam);
	}

	Mat->PostEditChange();
	return Mat;
}

// ----------------------------------------------------------------

FIconConfigurator3DViewportClient::FIconConfigurator3DViewportClient(FPreviewScene* InPreviewScene)
	: FEditorViewportClient(nullptr, InPreviewScene)
	, PreviewScenePtr(InPreviewScene)
{
	SetViewMode(VMI_Unlit);
	bSetListenerPosition = false;
	SetRealtime(true);

	EngineShowFlags.SetGrid(false);
	EngineShowFlags.SetBillboardSprites(false);
	EngineShowFlags.SetBounds(false);

	SetViewportType(LVT_Perspective);

	SetupPlanes();
	ResetCamera();
}

FIconConfigurator3DViewportClient::~FIconConfigurator3DViewportClient()
{
}

void FIconConfigurator3DViewportClient::SetupPlanes()
{
	UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(
		nullptr,
		TEXT("/Engine/BasicShapes/Plane.Plane"),
		nullptr, LOAD_None, nullptr);

	if (!PlaneMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("IconPreviewTool: Failed to load Plane mesh."));
		return;
	}

	// All planes share the same base material; create separate MID instances
	UMaterial* BaseMat = CreateIconConfiguratorMaterial();

	PlaneMaterials.SetNum(3);
	PlaneComponents.SetNum(3);

	for (int32 i = 0; i < 3; ++i)
	{
		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(
			GetTransientPackage(),
			*FString::Printf(TEXT("IconPlane_%d"), i),
			RF_Transient);

		Comp->SetStaticMesh(PlaneMesh);
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Comp->SetCastShadow(false);

		// Create dynamic material instance
		UMaterialInstanceDynamic* MID = nullptr;
		if (BaseMat)
		{
			MID = UMaterialInstanceDynamic::Create(BaseMat, GetTransientPackage());
			Comp->SetMaterial(0, MID);
		}
		PlaneMaterials[i] = MID;
		PlaneComponents[i] = Comp;

		// Register with Identity transform first
		PreviewScenePtr->AddComponent(Comp, FTransform::Identity);

		// Set final pose with SetRelativeTransform after registration.
		// Camera is at +X looking toward -X; planes are layered along X (depth direction):
		//   i=0 (background): X = -PlaneSpacing  ← farthest from camera (server layer[0]=background)
		//   i=1 (mid):        X = 0
		//   i=2 (foreground): X = +PlaneSpacing  ← closest to camera (server layer[2]=foreground)
		// FRotator(90,0,0) rotates the plane from horizontal XY to vertical YZ, normal pointing +X (toward camera)
		const float XOffset = static_cast<float>(i - 1) * PlaneSpacing;
		Comp->SetRelativeTransform(FTransform(
			FQuat(FRotator(90.f, 0.f, 0.f)),
			FVector(XOffset, 0.f, 0.f),
			FVector(0.5f, 0.5f, 0.5f)));

		// Initially hidden; shown after textures are provided
		Comp->SetVisibility(false);
	}
}

void FIconConfigurator3DViewportClient::ResetCamera()
{
	// Yaw=145° places the camera at an oblique angle so the 3 X-axis planes show obvious depth separation in the viewport
	CameraRotation  = FRotator(-20.f, 235.f, 0.f);
	CameraDistance  = 110.f;
	CameraTarget    = FVector::ZeroVector;
	Invalidate();
}

void FIconConfigurator3DViewportClient::UpdateLayerTextures(const TArray<FIconLayerData>& Layers)
{
	const int32 ActiveCount = FMath::Min(Layers.Num(), PlaneMaterials.Num());

	for (int32 i = 0; i < ActiveCount; ++i)
	{
		UMaterialInstanceDynamic* MID = PlaneMaterials[i];
		if (!MID) continue;

		UTexture2D* Tex = Layers[i].PreviewTexture.IsValid()
			? Layers[i].PreviewTexture.Get()
			: (Layers[i].ImportedTexture.IsValid() ? Layers[i].ImportedTexture.Get() : nullptr);

		if (Tex)
		{
			MID->SetTextureParameterValue(TEXT("BaseColor"), Tex);
		}

		if (PlaneComponents[i])
		{
			PlaneComponents[i]->SetVisibility(Tex != nullptr && Layers[i].bVisible);
		}

		MID->SetScalarParameterValue(TEXT("Opacity"), Layers[i].Opacity);
	}

	// Hide planes beyond the active layer count so stale textures from a previous
	// tab (e.g. Manual's 3rd layer) don't bleed into a tab with fewer layers.
	for (int32 i = ActiveCount; i < PlaneComponents.Num(); ++i)
	{
		if (PlaneComponents[i])
		{
			PlaneComponents[i]->SetVisibility(false);
		}
	}

	Invalidate();
}

void FIconConfigurator3DViewportClient::Tick(float DeltaTime)
{
	FEditorViewportClient::Tick(DeltaTime);

	// Orbit camera: offset from Target along -Forward by Distance
	const FVector Forward = CameraRotation.Vector();
	SetViewLocation(CameraTarget - Forward * CameraDistance);
	SetViewRotation(CameraRotation);
}

bool FIconConfigurator3DViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	// Scroll-wheel zoom (scroll fires as key events, not axis events)
	if (EventArgs.Key == EKeys::MouseScrollUp)
	{
		CameraDistance = FMath::Clamp(CameraDistance - 5.f, 20.f, 500.f);
		Invalidate();
		return true;
	}
	if (EventArgs.Key == EKeys::MouseScrollDown)
	{
		CameraDistance = FMath::Clamp(CameraDistance + 5.f, 20.f, 500.f);
		Invalidate();
		return true;
	}
	if (EventArgs.Key == EKeys::LeftMouseButton)
	{
		bIsRotating = (EventArgs.Event == IE_Pressed);
		return true;
	}
	if (EventArgs.Key == EKeys::MiddleMouseButton)
	{
		bIsPanning = (EventArgs.Event == IE_Pressed);
		return true;
	}
	return FEditorViewportClient::InputKey(EventArgs);
}

bool FIconConfigurator3DViewportClient::InputAxis(const FInputKeyEventArgs& Args)
{
	const float Delta = Args.AmountDepressed;

	// Left-button drag to rotate
	if (bIsRotating)
	{
		const float RotSpeed = 0.5f;
		if (Args.Key == EKeys::MouseX)
		{
			CameraRotation.Yaw += Delta * RotSpeed;
			Invalidate();
			return true;
		}
		if (Args.Key == EKeys::MouseY)
		{
			CameraRotation.Pitch = FMath::Clamp(
				CameraRotation.Pitch + Delta * RotSpeed, -89.f, 89.f);
			Invalidate();
			return true;
		}
	}

	// Middle-button drag to pan
	if (bIsPanning)
	{
		const float PanSpeed = 0.15f;
		const FMatrix RotMat = FRotationMatrix(CameraRotation);
		const FVector Right = RotMat.GetScaledAxis(EAxis::Y);
		const FVector Up    = RotMat.GetScaledAxis(EAxis::Z);

		if (Args.Key == EKeys::MouseX)
		{
			CameraTarget += Right * (-Delta * PanSpeed);
			Invalidate();
			return true;
		}
		if (Args.Key == EKeys::MouseY)
		{
			CameraTarget += Up * (Delta * PanSpeed);
			Invalidate();
			return true;
		}
	}

	return FEditorViewportClient::InputAxis(Args);
}

void FIconConfigurator3DViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FEditorViewportClient::Draw(InViewport, Canvas);
}
