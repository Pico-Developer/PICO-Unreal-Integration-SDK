// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICO_MRTypes.h"
#include "PICO_SpatialMeshComponent.h"
#include "PICO_SpatialMeshActor.generated.h"

UCLASS(BlueprintType,DisplayName="Spatial Mesh Actor PICO")
class ASpatialMeshActorPICO : public AActor
{
	GENERATED_BODY()

public:
	ASpatialMeshActorPICO(const FObjectInitializer& ObjectInitializer);
	
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type Reason) override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit",meta=(ExposeOnSpawn = true))
	bool bDrawOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit",meta=(ExposeOnSpawn = true))
	UMaterialInterface* SpatialMeshMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit",meta=(ExposeOnSpawn = true))
	TEnumAsByte<ECollisionEnabled::Type> CollisionType=ECollisionEnabled::Type::QueryAndPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit",meta=(ExposeOnSpawn = true))
	bool bSpatialMeshVisible=true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit",meta=(ExposeOnSpawn = true))
	TMap<ESemanticLabelPICO,FColor> SemanticToColors;
	
	UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
	bool StartDraw();

	UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
	bool PauseDraw();

	UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
	void SetMeshVisibility(bool visibility);
	
	UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
	bool ClearMesh();

	UFUNCTION(BlueprintPure, Category = "PICO|XR Toolkit")
	int32 GetMeshNum();

private:
	UFUNCTION()
	void HandleRequestSpatialMeshContentsEvent(EResultPICO Result, const TArray<FSpatialMeshInfoPICO>& MeshInfos);
	UFUNCTION()
	void HandleMeshDataUpdatedEvent();
	FColor GetColorBySceneLabel(ESemanticLabelPICO SceneLabel);

	bool UpdateMeshByMeshInfo(USpatialMeshComponentPICO* SpatialMesh, const FSpatialMeshInfoPICO& MeshInfo);

protected:
	UPROPERTY(Transient)
	TMap<FSpatialUUIDPICO, USpatialMeshComponentPICO*> EntityToMeshMap;
	TQueue<TArray<FSpatialMeshInfoPICO>> MeshInfoQueue;
	int32 NumDrawCalls=0;
	int32 DrawnPrimitives=0;
};