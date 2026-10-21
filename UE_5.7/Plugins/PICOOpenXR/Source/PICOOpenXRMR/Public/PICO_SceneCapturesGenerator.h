// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PICO_MRTypes.h"
#include "PICO_SceneCapturesGenerator.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FPXRLoadSceneDataEventDelegate,EResultPICO,Result);

UCLASS(BlueprintType, DisplayName = "Scene Captures Generator PICO")
class ASceneCapturesGeneratorPICO : public AActor
{
    GENERATED_BODY()

public:
    ASceneCapturesGeneratorPICO();

protected:
    // It can be overridden to perform custom initialization logic.
    virtual void BeginPlay() override;

public:
    // DeltaTime is the time passed since the last frame.
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
    void LoadSceneDataAsync(const FPXRLoadSceneDataEventDelegate& OnLoadFinished);
    
    UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
    void EnableAutoLoadingSceneData(bool InAutoLoadingEnabled);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "PICO|XR Toolkit|Generate Actors by Semantics", meta = (ExposeOnSpawn = true))
    TMap<ESemanticLabelPICO, FSceneCaptureGeneratorActorPICO> GenerateMaps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit|ProceduralMesh For Floor", meta = (ExposeOnSpawn = true))
    bool bEnableProceduralMeshForFloor;
    
    // Materials for floor corresponding to Procedural Mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit|ProceduralMesh For Floor", meta = (ExposeOnSpawn = true))
    UMaterialInterface* ProceduralMeshMaterialForFloor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit|ProceduralMesh For Floor", meta = (ExposeOnSpawn = true))
    FUVAdjustmentPICO ProceduralMeshUVAdjustmentForFloor;
    
    // Procedural Mesh for floor, whether collisions are generated
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit|ProceduralMesh For Floor", meta = (ExposeOnSpawn = true))
    bool bEnableProceduralMeshCollisionForFloor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit|ProceduralMesh For Ceiling", meta = (ExposeOnSpawn = true))
    bool bEnableProceduralMeshForCeiling;
    
    // Materials for ceiling corresponding to Procedural Mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit|ProceduralMesh For Ceiling", meta = (ExposeOnSpawn = true))
    UMaterialInterface* ProceduralMeshMaterialForCeiling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit|ProceduralMesh For Ceiling", meta = (ExposeOnSpawn = true))
    FUVAdjustmentPICO ProceduralMeshUVAdjustmentForCeiling;
    
    // Procedural Mesh for ceiling, whether collisions are generated
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO|XR Toolkit|ProceduralMesh For Ceiling", meta = (ExposeOnSpawn = true))
    bool bEnableProceduralMeshCollisionForCeiling;
    
    UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
    void SpawnSceneCaptures_Offline(const FMRSceneInfosPICO_Offline& Scene_Offline);
    
    UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
    void SpawnSceneCaptures(const TArray<FMRSceneInfoPICO>& SceneInfos);
    
    UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
    bool LoadOfflineSceneData(FString ImportPath, FMRSceneInfosPICO_Offline& OutSceneInfos);
    
    UFUNCTION(BlueprintCallable, Category = "PICO|XR Toolkit")
    void ClearSceneCaptures();
    
    UFUNCTION(BlueprintPure, Category = "PICO|XR Toolkit")
    TArray<AActor*> GetGeneratedActors();

private:
    FPXRLoadSceneDataEventDelegate SceneDataLoadDelegate;

    UPROPERTY()
    TArray<AActor*> SceneCaptures;

    FSceneLoadInfoPICO SceneLoadInfo;
    
    UFUNCTION()
    void HandleSceneLoadInfosEvent(EResultPICO Result, const TArray<FMRSceneInfoPICO>& SceneInfos);

    UFUNCTION()
    void HandleSceneDataUpdatedEvent();
    
    void SetScaleBasedOnRotationAndOriginScale(USceneComponent* SceneComponent, const FVector& OriginScale, const FQuat& BaseRotation, const FVector& BaseScale);
    
    FVector ConvertUnityPositionToUE(const FVector& InPosition, float WorldToMetersScale);
    
    FQuat ConvertUnityRotationToUE(const FQuat& InRotation);
    
    bool SpawnAndRescaling2DCapture(ESemanticLabelPICO Label, const FVector& Location, const FRotator& Rotation, const FVector& OriginScale);
    
    bool SpawnAndRescaling3DCapture(ESemanticLabelPICO Label, const FVector& Location, const FRotator& Rotation, const FVector& OriginScale);
    
    bool SpawnPolygonCapture(ESemanticLabelPICO Label, const FTransform& Transform, const TArray<FVector>& Vertices,bool bOffline);
    
    static const FString& EnumToString(const ESemanticLabelPICO& SemanticLabel)
    {
        // Static strings representing different semantic labels.
        static const FString Unknown("Unknown");
        static const FString Floor("Floor");
        static const FString Ceiling("Ceiling");
        static const FString Wall("Wall");
        static const FString Door("Door");
        static const FString Window("Window");
        static const FString Opening("Opening");
        static const FString Table("Table");
        static const FString Sofa("Sofa");
        static const FString Chair("Chair");
        static const FString Human("Human");
        static const FString Curtain("Curtain");
        static const FString Cabinet("Cabinet");
        static const FString Bed("Bed");
        static const FString Plant("Plant");
        static const FString Screen("Screen");
        static const FString VirtualWall("VirtualWall");
        static const FString Refrigerator("Refrigerator");
        static const FString Air_Conditioner("Air_Conditioner");
        static const FString Lamp("Lamp");
        static const FString Wall_Art("Wall_Art");
        static const FString Stairway("Stairway");


        // Switch case to map the enum value to the corresponding string.
        switch (SemanticLabel)
        {
            case ESemanticLabelPICO::Unknown: return Unknown;
            case ESemanticLabelPICO::Floor: return Floor;
            case ESemanticLabelPICO::Ceiling: return Ceiling;
            case ESemanticLabelPICO::Wall: return Wall;
            case ESemanticLabelPICO::Door: return Door;
            case ESemanticLabelPICO::Window: return Window;
            case ESemanticLabelPICO::Opening: return Opening;
            case ESemanticLabelPICO::Table: return Table;
            case ESemanticLabelPICO::Sofa: return Sofa;
            case ESemanticLabelPICO::Chair: return Chair;
            case ESemanticLabelPICO::Human: return Human;
            case ESemanticLabelPICO::Curtain: return Curtain;
            case ESemanticLabelPICO::Cabinet: return Cabinet;
            case ESemanticLabelPICO::Bed: return Bed;
            case ESemanticLabelPICO::Plant: return Plant;
            case ESemanticLabelPICO::Refrigerator: return Refrigerator;
            case ESemanticLabelPICO::Air_Conditioner: return Air_Conditioner;
            case ESemanticLabelPICO::Lamp: return Lamp;
            case ESemanticLabelPICO::Wall_Art: return Wall_Art;
            case ESemanticLabelPICO::Screen: return Screen;
            case ESemanticLabelPICO::VirtualWall: return VirtualWall;
            case ESemanticLabelPICO::Stairway: return Stairway;
            default: return Unknown;
        }
    }
};