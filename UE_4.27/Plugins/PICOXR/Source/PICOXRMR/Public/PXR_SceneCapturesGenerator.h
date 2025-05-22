// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PXR_MRTypes.h"
#include "PXR_SceneCapturesGenerator.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FPXRLoadSceneDataEventDelegate,EPICOResult,Result);

UCLASS(BlueprintType, DisplayName = "PICO XR SceneCaptures Generator")
class APICOXRSceneCapturesGenerator : public AActor
{
    GENERATED_BODY()

public:
    APICOXRSceneCapturesGenerator();

protected:
    // It can be overridden to perform custom initialization logic.
    virtual void BeginPlay() override;

public:
    // DeltaTime is the time passed since the last frame.
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "PICO XR Toolkit")
    void LoadSceneDataAsync(const FPXRLoadSceneDataEventDelegate& OnLoadFinished);
    
    UFUNCTION(BlueprintCallable, Category = "PICO XR Toolkit")
    void EnableAutoLoadingSceneData(bool InAutoLoadingEnabled);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "PICO XR Toolkit|Generate Actors by Semantics", meta = (ExposeOnSpawn = true))
    TMap<EPICOSemanticLabel, FSceneCaptureGeneratorActor> GenerateMaps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO XR Toolkit|ProceduralMesh For Floor", meta = (ExposeOnSpawn = true))
    bool bEnableProceduralMeshForFloor;
    
    // Materials for floor corresponding to Procedural Mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO XR Toolkit|ProceduralMesh For Floor", meta = (ExposeOnSpawn = true))
    UMaterialInterface* ProceduralMeshMaterialForFloor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO XR Toolkit|ProceduralMesh For Floor", meta = (ExposeOnSpawn = true))
    FPICOUVAdjustment ProceduralMeshUVAdjustmentForFloor;
    
    // Procedural Mesh for floor, whether collisions are generated
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO XR Toolkit|ProceduralMesh For Floor", meta = (ExposeOnSpawn = true))
    bool bEnableProceduralMeshCollisionForFloor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO XR Toolkit|ProceduralMesh For Ceiling", meta = (ExposeOnSpawn = true))
    bool bEnableProceduralMeshForCeiling;
    
    // Materials for ceiling corresponding to Procedural Mesh
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO XR Toolkit|ProceduralMesh For Ceiling", meta = (ExposeOnSpawn = true))
    UMaterialInterface* ProceduralMeshMaterialForCeiling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO XR Toolkit|ProceduralMesh For Ceiling", meta = (ExposeOnSpawn = true))
    FPICOUVAdjustment ProceduralMeshUVAdjustmentForCeiling;
    
    // Procedural Mesh for ceiling, whether collisions are generated
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PICO XR Toolkit|ProceduralMesh For Ceiling", meta = (ExposeOnSpawn = true))
    bool bEnableProceduralMeshCollisionForCeiling;
    
    UFUNCTION(BlueprintCallable, Category = "PICO XR Toolkit")
    void SpawnSceneCaptures_Offline(const FPICOMRSceneInfos_Offline& Scene_Offline);
    
    UFUNCTION(BlueprintCallable, Category = "PICO XR Toolkit")
    void SpawnSceneCaptures(const TArray<FPICOMRSceneInfo>& SceneInfos);
    
    UFUNCTION(BlueprintCallable, Category = "PICO XR Toolkit")
    bool LoadOfflineSceneData(FString ImportPath, FPICOMRSceneInfos_Offline& OutSceneInfos);
    
    UFUNCTION(BlueprintCallable, Category = "PICO XR Toolkit")
    void ClearSceneCaptures();
    
    UFUNCTION(BlueprintPure, Category = "PICO XR Toolkit")
    TArray<AActor*> GetGeneratedActors();

private:
    FPXRLoadSceneDataEventDelegate SceneDataLoadDelegate;

    UPROPERTY()
    TArray<AActor*> SceneCaptures;

    FPICOSceneLoadInfo SceneLoadInfo;
    
    UFUNCTION()
    void HandleSceneLoadInfosEvent(EPICOResult Result, const TArray<FPICOMRSceneInfo>& SceneInfos);

    UFUNCTION()
    void HandleSceneDataUpdatedEvent();
    
    void SetScaleBasedOnRotationAndOriginScale(USceneComponent* SceneComponent, const FVector& OriginScale, const FQuat& BaseRotation, const FVector& BaseScale);
    
    FVector ConvertUnityPositionToUE(const FVector& InPosition, float WorldToMetersScale);
    
    FQuat ConvertUnityRotationToUE(const FQuat& InRotation);
    
    bool SpawnAndRescaling2DCapture(EPICOSemanticLabel Label, const FVector& Location, const FRotator& Rotation, const FVector& OriginScale);
    
    bool SpawnAndRescaling3DCapture(EPICOSemanticLabel Label, const FVector& Location, const FRotator& Rotation, const FVector& OriginScale);
    
    bool SpawnPolygonCapture(EPICOSemanticLabel Label, const FTransform& Transform, const TArray<FVector>& Vertices);
    
    static const FString& EnumToString(const EPICOSemanticLabel& SemanticLabel)
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
        static const FString Screen("Screen");
        static const FString VirtualWall("VirtualWall");
        static const FString Stairway("Stairway");

        // Switch case to map the enum value to the corresponding string.
        switch (SemanticLabel)
        {
            case EPICOSemanticLabel::Unknown: return Unknown;
            case EPICOSemanticLabel::Floor: return Floor;
            case EPICOSemanticLabel::Ceiling: return Ceiling;
            case EPICOSemanticLabel::Wall: return Wall;
            case EPICOSemanticLabel::Door: return Door;
            case EPICOSemanticLabel::Window: return Window;
            case EPICOSemanticLabel::Opening: return Opening;
            case EPICOSemanticLabel::Table: return Table;
            case EPICOSemanticLabel::Sofa: return Sofa;
            case EPICOSemanticLabel::Chair: return Chair;
            case EPICOSemanticLabel::Human: return Human;
            case EPICOSemanticLabel::Curtain: return Curtain;
            case EPICOSemanticLabel::Cabinet: return Cabinet;
            case EPICOSemanticLabel::Bed: return Bed;
            case EPICOSemanticLabel::Screen: return Screen;
            case EPICOSemanticLabel::VirtualWall: return VirtualWall;
            case EPICOSemanticLabel::Stairway: return Stairway;
            default: return Unknown;
        }
    }
};