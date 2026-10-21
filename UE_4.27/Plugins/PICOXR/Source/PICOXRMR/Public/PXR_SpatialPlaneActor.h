// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "PXR_HMD.h"
#include "PXR_MRTypes.h"
#include "PXR_SpatialPlaneActor.generated.h"

UCLASS(BlueprintType,DisplayName="PICO XR Spatial Plane Actor")
class APICOSpatialPlaneActor : public AActor
{
	GENERATED_BODY()

public:
	APICOSpatialPlaneActor(const FObjectInitializer& ObjectInitializer);
	
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	virtual void Tick(float DeltaTime) override;

	/** Determines AI to be spawned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spatial Plane Actor")
	TSubclassOf<class UPICOSpatialPlaneComponent>  PlaneComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Plane Actor",meta=(ExposeOnSpawn = true))
	bool bDrawOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Plane Actor",meta=(ExposeOnSpawn = true))
	UMaterialInterface* SpatialPlaneMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Plane Actor",meta=(ExposeOnSpawn = true))
	TEnumAsByte<ECollisionEnabled::Type> CollisionType=ECollisionEnabled::Type::QueryAndPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Plane Actor",meta=(ExposeOnSpawn = true))
	bool bSpatialPlaneVisible=true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Plane Actor",meta=(ExposeOnSpawn = true))
	TMap<EPICOSemanticLabel,FColor> SemanticToColors;
	
	UFUNCTION(BlueprintCallable, Category = "PICO|PICO XR Toolkit|Spatial Plane Actor")
	bool StartDraw();

	UFUNCTION(BlueprintCallable, Category = "PICO|PICO XR Toolkit|Spatial Plane Actor")
	bool PauseDraw();

	UFUNCTION(BlueprintCallable, Category = "PICO|PICO XR Toolkit|Spatial Plane Actor")
	void SetPlaneVisibility(bool visibility);
	
	UFUNCTION(BlueprintCallable, Category = "PICO|PICO XR Toolkit|Spatial Plane Actor")
	bool ClearPlane();

	UFUNCTION(BlueprintPure, Category = "PICO|PICO XR Toolkit|Spatial Plane Actor")
	int32 GetPlaneNum();

private:
	UFUNCTION()
	void HandleRequestSpatialPlaneContentsEvent(EPICOResult Result, const TArray<FPICOSpatialPlaneInfo>& MeshInfos);
	UFUNCTION()
	void HandlePlaneDataUpdatedEvent();
	FColor GetColorBySceneLabel(EPICOSemanticLabel SceneLabel);

	bool UpdatePlaneByPlaneInfo(UPICOSpatialPlaneComponent* SpatialMesh, const FPICOSpatialPlaneInfo& PlaneInfo);

protected:
	UPROPERTY(Transient)
	TMap<FPICOSpatialUUID, UPICOSpatialPlaneComponent*> EntityToPlaneMap;
	
	TQueue<TArray<FPICOSpatialPlaneInfo>> PlaneInfoQueue;
};