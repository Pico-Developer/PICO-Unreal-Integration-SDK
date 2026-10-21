// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StereoLayerComponent.h"
#include "ProceduralMeshComponent.h"
#include "PICO_PokeHoleComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup = (PICO), meta = (BlueprintSpawnableComponent, DisplayName = "Poke Hole Component PICO"))
class PICOOPENXRHMD_API UPokeHoleComponentPICO : public UProceduralMeshComponent
{
	GENERATED_BODY()

public:
	UPokeHoleComponentPICO(const FObjectInitializer& ObjectInitializer);

	virtual void OnRegister() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnUnregister() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO")
	FName TargetStereoLayerComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO", Meta = (ToolTip = "Material overrides."))
	TObjectPtr<class UMaterialInterface> OverrideMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO")
	float QuadScale = 0.99;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO")
	float CylinderScale = 0.99;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO")
	float CubemapScale = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO")
	bool bFlipCubeFace = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO")
	float CustomRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO")
	int SphereSegments = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PICO")
	bool bFlipSphereFace = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, NoClear, Instanced, Category = "PICO", DisplayName = "Stereo Layer Shape")
	TObjectPtr<UStereoLayerShape> Shape;

private:
	UPROPERTY()
	TObjectPtr<class UStereoLayerComponent> StereoLayer;

	uint32 bLastQuadPreserveTextureRatio : 1;
	
	void CreatePokeHoleMesh(UStereoLayerComponent* InStereoLayer, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector2D>& UV0);
};

