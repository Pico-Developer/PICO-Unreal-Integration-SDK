// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_MRTypes.h"
#include "PICO_SpatialMeshComponent.h"
#include "PICO_SpatialPlaneComponent.h"
#include "PICO_SpatialPlaneActor.generated.h"

UCLASS(BlueprintType,DisplayName="Spatial Plane Actor PICO")
class ASpatialPlaneActorPICO : public AActor
{
	GENERATED_BODY()

public:
	ASpatialPlaneActorPICO(const FObjectInitializer& ObjectInitializer);
	
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	virtual void Tick(float DeltaTime) override;

	/** Determines AI to be spawned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Spatial Plane Actor")
	TSubclassOf<class USpatialPlaneComponentPICO>  PlaneComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Plane Actor",meta=(ExposeOnSpawn = true))
	bool bDrawOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Plane Actor",meta=(ExposeOnSpawn = true))
	UMaterialInterface* SpatialPlaneMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Plane Actor",meta=(ExposeOnSpawn = true))
	TEnumAsByte<ECollisionEnabled::Type> CollisionType=ECollisionEnabled::Type::QueryAndPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Plane Actor",meta=(ExposeOnSpawn = true))
	bool bSpatialPlaneVisible=true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spatial Plane Actor",meta=(ExposeOnSpawn = true))
	TMap<ESemanticLabelPICO,FColor> SemanticToColors;
	
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
	void HandleRequestSpatialPlaneContentsEvent(EResultPICO Result, const TArray<FSpatialPlaneInfoPICO>& MeshInfos);
	UFUNCTION()
	void HandlePlaneDataUpdatedEvent();
	FColor GetColorBySceneLabel(ESemanticLabelPICO SceneLabel);

	bool UpdatePlaneByPlaneInfo(USpatialPlaneComponentPICO* SpatialMesh, const FSpatialPlaneInfoPICO& PlaneInfo);

protected:
	UPROPERTY(Transient)
	TMap<FSpatialUUIDPICO, USpatialPlaneComponentPICO*> EntityToPlaneMap;
	
	TQueue<TArray<FSpatialPlaneInfoPICO>> PlaneInfoQueue;
};