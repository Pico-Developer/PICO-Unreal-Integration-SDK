// Fill out your copyright notice in the Description page of Project Settings.


#include "PXR_SceneCapturesGenerator.h"
#include "JsonObjectConverter.h"
#include "PXR_EventManager.h"
#include "PXR_MRAsyncActions.h"
#include "PXR_MRFunctionLibrary.h"
#include "Algo/Transform.h"
#include "Engine/StaticMeshActor.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#if WITH_EDITOR
#include "FileHelpers.h"
#endif

constexpr float WALL_WIDTH = 1.0f;
// Sets default values
APICOXRSceneCapturesGenerator::APICOXRSceneCapturesGenerator():
	bEnableProceduralMeshForFloor(true),
	ProceduralMeshMaterialForFloor(nullptr),
	ProceduralMeshUVAdjustmentForFloor(),
	bEnableProceduralMeshCollisionForFloor(false),
	bEnableProceduralMeshForCeiling(false),
	ProceduralMeshMaterialForCeiling(nullptr),
	ProceduralMeshUVAdjustmentForCeiling(),
	bEnableProceduralMeshCollisionForCeiling(false)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GenerateMaps =
	{
		{EPICOSemanticLabel::Wall, {}},
		{EPICOSemanticLabel::Door, {}},
		{EPICOSemanticLabel::Window, {}},
		{EPICOSemanticLabel::Opening, {}},
		{EPICOSemanticLabel::Table, {}},
		{EPICOSemanticLabel::Sofa, {}},
		{EPICOSemanticLabel::Chair, {}},
		{EPICOSemanticLabel::Human, {}},
		{EPICOSemanticLabel::Curtain, {}},
		{EPICOSemanticLabel::Cabinet, {}},
		{EPICOSemanticLabel::Bed, {}},
		{EPICOSemanticLabel::Screen, {}},
		{EPICOSemanticLabel::VirtualWall, {}},
	};
}

// Called when the game starts or when spawned
void APICOXRSceneCapturesGenerator::BeginPlay()
{
	Super::BeginPlay();
	for (const auto& Scene:GenerateMaps)
	{
		if (Scene.Value.Actor)
		{
			SceneLoadInfo.SemanticFilter.Add(Scene.Key);
		}
	}
	if (bEnableProceduralMeshForFloor)
	{
		SceneLoadInfo.SemanticFilter.Add(EPICOSemanticLabel::Floor);
	}

	if (bEnableProceduralMeshForCeiling)
	{
		SceneLoadInfo.SemanticFilter.Add(EPICOSemanticLabel::Ceiling);
	}
}

// Called every frame
void APICOXRSceneCapturesGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APICOXRSceneCapturesGenerator::LoadSceneDataAsync(const FPXRLoadSceneDataEventDelegate& OnLoadFinished)
{
	SceneDataLoadDelegate = OnLoadFinished;
	HandleSceneDataUpdatedEvent();
}

void APICOXRSceneCapturesGenerator::EnableAutoLoadingSceneData(bool InAutoLoadingEnabled)
{
	if (InAutoLoadingEnabled)
	{
		if (!UPICOXRHMDFunctionLibrary::PXR_GetEventManager()->SceneCaptureDataUpdatedDelegate.Contains(this, GET_FUNCTION_NAME_CHECKED(APICOXRSceneCapturesGenerator, HandleSceneDataUpdatedEvent)))
		{
			UPICOXRHMDFunctionLibrary::PXR_GetEventManager()->SceneCaptureDataUpdatedDelegate.AddDynamic(this, &APICOXRSceneCapturesGenerator::HandleSceneDataUpdatedEvent);
		}
	}
	else
	{
		if (UPICOXRHMDFunctionLibrary::PXR_GetEventManager()->SceneCaptureDataUpdatedDelegate.Contains(this, GET_FUNCTION_NAME_CHECKED(APICOXRSceneCapturesGenerator, HandleSceneDataUpdatedEvent)))
		{
			UPICOXRHMDFunctionLibrary::PXR_GetEventManager()->SceneCaptureDataUpdatedDelegate.RemoveDynamic(this, &APICOXRSceneCapturesGenerator::HandleSceneDataUpdatedEvent);
		}
	}
}

FVector APICOXRSceneCapturesGenerator::ConvertUnityPositionToUE(const FVector& InPosition, float WorldToMetersScale)
{
	return FVector(InPosition.Z, InPosition.X, InPosition.Y) * WorldToMetersScale;
}

FQuat APICOXRSceneCapturesGenerator::ConvertUnityRotationToUE(const FQuat& InRotation)
{
	return FQuat(InRotation.Z, InRotation.X, InRotation.Y, InRotation.W);
}

void APICOXRSceneCapturesGenerator::SpawnSceneCaptures_Offline(const FPICOMRSceneInfos_Offline& Scene_Offline)
{
	float WorldToMetersScale = 100.0f;
	if (GWorld != nullptr)
	{
		WorldToMetersScale=GWorld->GetWorldSettings()->WorldToMeters;
	}

	for (auto MRScene : Scene_Offline.OutSceneInfos)
	{
		FVector Location=ConvertUnityPositionToUE(MRScene.Position,WorldToMetersScale);
		FQuat Quat=ConvertUnityRotationToUE(MRScene.Rotation);
		FRotator Rotation=FRotator(Quat);
		
		FTransform Transform=FTransform(Rotation,Location);

		TArray<FVector> Vertices;
		if (MRScene.PolygonVertices.Num())
		{
			Algo::Transform(MRScene.PolygonVertices, Vertices, [WorldToMetersScale](const auto& Vertex) { return FVector(-Vertex.Y, Vertex.X,0) * WorldToMetersScale; });
		}
		
		switch (MRScene.SemanticLabel) {
		case EPICOSemanticLabel::Unknown:
			break;
		case EPICOSemanticLabel::Floor:
		case EPICOSemanticLabel::Ceiling:
			{
				SpawnPolygonCapture(MRScene.SemanticLabel,Transform,Vertices);
			}
			break;
		case EPICOSemanticLabel::Wall:
		case EPICOSemanticLabel::Door:
		case EPICOSemanticLabel::Window:
		case EPICOSemanticLabel::Opening:
		case EPICOSemanticLabel::VirtualWall:
			{
				FVector OriginScale=FVector(MRScene.Box2DInfo.Extent.Y*WorldToMetersScale,MRScene.Box2DInfo.Extent.X*WorldToMetersScale,WALL_WIDTH);
				SpawnAndRescaling2DCapture(MRScene.SemanticLabel,Location,Rotation,OriginScale);
			}
			break;
		case EPICOSemanticLabel::Table:
		case EPICOSemanticLabel::Sofa:
		case EPICOSemanticLabel::Chair:
		case EPICOSemanticLabel::Human:
		case EPICOSemanticLabel::Curtain:
		case EPICOSemanticLabel::Cabinet:
		case EPICOSemanticLabel::Bed:
		case EPICOSemanticLabel::Stairway:
		case EPICOSemanticLabel::Screen:
			{
				FVector OriginScale=ConvertUnityPositionToUE(MRScene.Box3DInfo.Extent,WorldToMetersScale);
				SpawnAndRescaling3DCapture(MRScene.SemanticLabel,Location,Rotation,OriginScale);
			}
			break;
		default:
			break;
		}
	}
}

bool APICOXRSceneCapturesGenerator::SpawnAndRescaling2DCapture(EPICOSemanticLabel Label, const FVector& Location, const FRotator& Rotation,const FVector& OriginScale)
{
	if (GenerateMaps.Contains(Label))
	{
		AActor* Box2DActor = GetWorld()->SpawnActor(GenerateMaps[Label].Actor);
		if (Box2DActor != nullptr)
		{
			Box2DActor->Tags.AddUnique(FName(EnumToString(Label)));
			SceneCaptures.Add(Box2DActor);
			auto Root = Box2DActor->GetRootComponent();
			Root->SetMobility(EComponentMobility::Movable);
			Box2DActor->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			if (GenerateMaps[Label].ScalingMode == ESceneCaptureScalingMode::Stretch)
			{
				const auto Bounds = Box2DActor->CalculateComponentsBoundingBoxInLocalSpace(true);
				const FVector ActorSize = Bounds.GetSize();
				FVector FinalScale = OriginScale / ActorSize;
				Box2DActor->SetActorScale3D(FinalScale);
			}

			Box2DActor->SetActorLocation(Location);
			Box2DActor->SetActorRotation(Rotation);

			return true;
		}
	}
	
	return false;
}

bool APICOXRSceneCapturesGenerator::SpawnAndRescaling3DCapture(EPICOSemanticLabel Label, const FVector& Location, const FRotator& Rotation,const FVector& OriginScale)
{
	if (GenerateMaps.Contains(Label))
	{
		AActor* Box3DActor = GetWorld()->SpawnActor(GenerateMaps[Label].Actor);
		if (Box3DActor !=nullptr)
		{
			Box3DActor->Tags.AddUnique(FName(EnumToString(Label)));
			SceneCaptures.Add(Box3DActor);
			auto Root = Box3DActor->GetRootComponent();
			Root->SetMobility(EComponentMobility::Movable);
			Box3DActor->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				
			if (GenerateMaps[Label].ScalingMode==ESceneCaptureScalingMode::Stretch)
			{
				const auto Bounds = Box3DActor->CalculateComponentsBoundingBoxInLocalSpace(true);
				const FVector Box3DActorSize = Bounds.GetSize();
				FVector Scale=OriginScale/Box3DActorSize;
				SetScaleBasedOnRotationAndOriginScale(Root,Scale,FQuat::Identity,FVector::OneVector);
			}
	
			Box3DActor->SetActorLocation(Location);
			Box3DActor->SetActorRotation(Rotation);

			return true;
		}
	}

	return false;
}

bool APICOXRSceneCapturesGenerator::SpawnPolygonCapture(EPICOSemanticLabel Label, const FTransform& Transform, const TArray<FVector>& Vertices)
{
	AActor* Actor = this->GetWorld()->SpawnActor<AActor>();
	if (Actor != nullptr)
	{
		SceneCaptures.Add(Actor);
		Actor->SetOwner(this);
		Actor->Tags.AddUnique(FName(EnumToString(Label)));
		Actor->SetRootComponent(NewObject<USceneComponent>(Actor, TEXT("Root")));
		Actor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
		Actor->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		Actor->SetActorRelativeScale3D(FVector::OneVector);
		Actor->SetActorTransform(Transform);
		
		if(Label == EPICOSemanticLabel::Floor ? UPICOXRMRFunctionLibrary::PXR_CreateSceneBoundingPolygonWithUVAdjustment(Actor,!bEnableProceduralMeshCollisionForFloor,
		false,ProceduralMeshUVAdjustmentForFloor,Transform,Vertices,ProceduralMeshMaterialForFloor)
		:UPICOXRMRFunctionLibrary::PXR_CreateSceneBoundingPolygonWithUVAdjustment(Actor,!bEnableProceduralMeshCollisionForCeiling,
		true,ProceduralMeshUVAdjustmentForCeiling,Transform,Vertices,ProceduralMeshMaterialForCeiling))
		{
			return true;
		}
	}

	return false;
	
}

void APICOXRSceneCapturesGenerator::SpawnSceneCaptures(const TArray<FPICOMRSceneInfo>& SceneInfos)
{
	for (auto SceneInfo : SceneInfos)
	{
		FRotator Rotation=FRotator(SceneInfo.ScenePose.GetRotation());
		FVector Location=SceneInfo.ScenePose.GetLocation();
		
		switch (SceneInfo.SceneType) {
		case EPICOSceneType::BoundingBox2D:
			{
				FPICOBoundingBox2D Box2D;
				UPICOXRMRFunctionLibrary::PXR_GetSceneBoundingBox2D(SceneInfo.UUID,Box2D);
				Location+=Box2D.Center;
				FVector OriginScale=FVector(WALL_WIDTH,Box2D.Extent.Width,Box2D.Extent.Height);
				SpawnAndRescaling2DCapture(SceneInfo.Semantic,Location,Rotation,OriginScale);
			}
			break;
		case EPICOSceneType::BoundingPolygon:
			{
				TArray<FVector> PolygonVertices;
				UPICOXRMRFunctionLibrary::PXR_GetSceneBoundingPolygon(SceneInfo.UUID,PolygonVertices);
				SpawnPolygonCapture(SceneInfo.Semantic,SceneInfo.ScenePose,PolygonVertices);
			}
			break;
		case EPICOSceneType::BoundingBox3D:
			{
				FPICOBoundingBox3D Box3D;
				UPICOXRMRFunctionLibrary::PXR_GetSceneBoundingBox3D(SceneInfo.UUID,Box3D);
				FVector OriginScale=FVector(Box3D.Extent.Depth,Box3D.Extent.Width,Box3D.Extent.Height);
				Location+=Box3D.Center.GetLocation();
				SpawnAndRescaling3DCapture(SceneInfo.Semantic,Location,Rotation,OriginScale);
			}
			break;
		}
	}

	
}

bool APICOXRSceneCapturesGenerator::LoadOfflineSceneData(FString ImportPath, FPICOMRSceneInfos_Offline& OutSceneInfos)
{
	OutSceneInfos.OutSceneInfos.Reset();

#if WITH_EDITOR
	FPaths::NormalizeDirectoryName(ImportPath);
	
	FString JsonString;
	if (FFileHelper::LoadFileToString(JsonString, *ImportPath) )
	{
		
		FString Prefix = TEXT("{\r\n\"OutSceneInfos\":\r\n");
		JsonString = Prefix + JsonString;
		JsonString += "\r\n}";

		TSharedRef< TJsonReader<> > JsonReader = TJsonReaderFactory<>::Create(JsonString);

		TSharedPtr<FJsonObject> JsonComparisonReport;
		if ( !FJsonSerializer::Deserialize(JsonReader, JsonComparisonReport) )
		{
			return false;
		}

		if ( FJsonObjectConverter::JsonObjectToUStruct(JsonComparisonReport.ToSharedRef(), &OutSceneInfos, 0, 0) )
		{
			if (OutSceneInfos.OutSceneInfos.Num())
			{
				UE_LOG(LogHMD, Display, TEXT("JsonObjectToUStruct Success"));
			}
			else
			{
				UE_LOG(LogHMD, Error, TEXT("JsonObjectToUStruct Failed"));
			}
		}
		return true;
	}
#endif
	

	return false;
}

void APICOXRSceneCapturesGenerator::ClearSceneCaptures()
{
	for (auto SceneCapture:SceneCaptures)
	{
		if(SceneCapture)
		{
			SceneCapture->Destroy();
		}
	}
	
	SceneCaptures.Empty();
}

TArray<AActor*> APICOXRSceneCapturesGenerator::GetGeneratedActors()
{
	return SceneCaptures;
}

// Determine the scaling values for the corresponding axes from the axis vectors and the original scaling vectors
FVector GetRotatedScale(const FVector& AxisX, const FVector& AxisY, const FVector& AxisZ, const FVector& OriginScale)
{
	FVector RotatedScale;
	RotatedScale.X = (FMath::Abs(AxisX.X) >= UE_INV_SQRT_2)? OriginScale.X : ((FMath::Abs(AxisX.Y) >= UE_INV_SQRT_2)? OriginScale.Y : OriginScale.Z);
	RotatedScale.Y = (FMath::Abs(AxisY.X) >= UE_INV_SQRT_2)? OriginScale.X : ((FMath::Abs(AxisY.Y) >= UE_INV_SQRT_2)? OriginScale.Y : OriginScale.Z);
	RotatedScale.Z = (FMath::Abs(AxisZ.X) >= UE_INV_SQRT_2)? OriginScale.X : ((FMath::Abs(AxisZ.Y) >= UE_INV_SQRT_2)? OriginScale.Y : OriginScale.Z);
	return RotatedScale;
}

void APICOXRSceneCapturesGenerator::HandleSceneDataUpdatedEvent()
{
	UPICORequestSceneCaptures_AsyncAction* RequestSceneCapturesAction = UPICORequestSceneCaptures_AsyncAction::PXR_RequestSceneCaptures_Async(SceneLoadInfo);
	RequestSceneCapturesAction->OnSuccess.AddDynamic(this, &APICOXRSceneCapturesGenerator::HandleSceneLoadInfosEvent);
	RequestSceneCapturesAction->Activate();
}

void APICOXRSceneCapturesGenerator::HandleSceneLoadInfosEvent(EPICOResult Result, const TArray<FPICOMRSceneInfo>& SceneInfos)
{
	if (Result==EPICOResult::PXR_Success)
	{
		ClearSceneCaptures();
		SpawnSceneCaptures(SceneInfos);
		SceneDataLoadDelegate.ExecuteIfBound(Result);
	}
}

void APICOXRSceneCapturesGenerator::SetScaleBasedOnRotationAndOriginScale(USceneComponent* SceneComponent, const FVector& OriginScale, const FQuat& BaseRotation, const FVector& BaseScale)
{
	if (SceneComponent)
	{
		const auto RelativeRotation = SceneComponent->GetRelativeRotationCache().RotatorToQuat(SceneComponent->GetRelativeRotation());
		const auto Rotation = BaseRotation * RelativeRotation;
		const FVector RotatedXAxis = Rotation.GetAxisX();
		const FVector RotatedYAxis = Rotation.GetAxisY();
		const FVector RotatedZAxis = Rotation.GetAxisZ();

		// Get the rotated scaling vector
		FVector RotatedScale = GetRotatedScale(RotatedXAxis, RotatedYAxis, RotatedZAxis, OriginScale);

		const FVector OldScale = SceneComponent->GetRelativeScale3D();
		const FVector NewScale = BaseScale * RotatedScale * OldScale;
		SceneComponent->SetRelativeScale3D(NewScale);

		const FVector NewBaseScale = BaseScale * (OldScale / NewScale);
		for (auto Child : SceneComponent->GetAttachChildren())
		{
			if (Child)
			{
				SetScaleBasedOnRotationAndOriginScale(Child, OriginScale, Rotation, NewBaseScale);
			}
		}
	}
}
