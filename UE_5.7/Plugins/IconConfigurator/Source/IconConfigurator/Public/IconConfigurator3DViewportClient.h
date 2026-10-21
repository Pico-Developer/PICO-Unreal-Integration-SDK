// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "IconLayerData.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class FPreviewScene;

/**
 * Custom ViewportClient: manages the scene and interaction (rotate/zoom/pan) for 3 mesh planes
 */
class FIconConfigurator3DViewportClient : public FEditorViewportClient
{
public:
	explicit FIconConfigurator3DViewportClient(FPreviewScene* InPreviewScene);
	virtual ~FIconConfigurator3DViewportClient() override;

	// FEditorViewportClient interface
	virtual void Tick(float DeltaTime) override;
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual bool InputAxis(const FInputKeyEventArgs& Args) override;
	virtual void Draw(FViewport* InViewport, FCanvas* Canvas) override;

	/** Refresh the 3 plane materials with new textures */
	void UpdateLayerTextures(const TArray<FIconLayerData>& Layers);

	/** Reset camera to default position */
	void ResetCamera();

private:
	void SetupPlanes();

	FPreviewScene* PreviewScenePtr = nullptr;

	// Dynamic material instances for 3 planes (index 0=back, 1=mid, 2=front)
	TArray<UMaterialInstanceDynamic*> PlaneMaterials;

	// StaticMeshComponents for 3 planes
	TArray<UStaticMeshComponent*> PlaneComponents;

	// Camera rotation and distance (orbit camera)
	FRotator CameraRotation = FRotator(-20.f, 180.f, 0.f);
	float    CameraDistance = 120.f;
	FVector  CameraTarget   = FVector::ZeroVector;

	// Mouse button states
	bool bIsRotating = false;
	bool bIsPanning  = false;
};
