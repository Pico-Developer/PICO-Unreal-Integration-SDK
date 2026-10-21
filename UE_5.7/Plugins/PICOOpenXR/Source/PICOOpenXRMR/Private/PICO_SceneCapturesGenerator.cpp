// Fill out your copyright notice in the Description page of Project Settings.


#include "PICO_SceneCapturesGenerator.h"
#include "JsonObjectConverter.h"
#include "PICO_MRAsyncActions.h"
#include "PICO_MRFunctionLibrary.h"
#include "Algo/Transform.h"
#include "Engine/StaticMeshActor.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

constexpr float WALL_WIDTH = 1.0f;
// Sets default values
ASceneCapturesGeneratorPICO::ASceneCapturesGeneratorPICO():
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
		{ESemanticLabelPICO::Wall, {}},
		{ESemanticLabelPICO::Door, {}},
		{ESemanticLabelPICO::Window, {}},
		{ESemanticLabelPICO::Opening, {}},
		{ESemanticLabelPICO::Table, {}},
		{ESemanticLabelPICO::Sofa, {}},
		{ESemanticLabelPICO::Chair, {}},
		{ESemanticLabelPICO::Human, {}},
		{ESemanticLabelPICO::Curtain, {}},
		{ESemanticLabelPICO::Cabinet, {}},
		{ESemanticLabelPICO::Bed, {}},
		{ESemanticLabelPICO::Plant, {}},
		{ESemanticLabelPICO::Screen, {}},
		{ESemanticLabelPICO::VirtualWall, {}},
		{ESemanticLabelPICO::Refrigerator, {}},
		{ESemanticLabelPICO::Washing_Machine, {}},
		{ESemanticLabelPICO::Air_Conditioner, {}},
		{ESemanticLabelPICO::Lamp, {}},
		{ESemanticLabelPICO::Wall_Art, {}},
	};
}

// Called when the game starts or when spawned
void ASceneCapturesGeneratorPICO::BeginPlay()
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
		SceneLoadInfo.SemanticFilter.Add(ESemanticLabelPICO::Floor);
	}

	if (bEnableProceduralMeshForCeiling)
	{
		SceneLoadInfo.SemanticFilter.Add(ESemanticLabelPICO::Ceiling);
	}
}

// Called every frame
void ASceneCapturesGeneratorPICO::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASceneCapturesGeneratorPICO::LoadSceneDataAsync(const FPXRLoadSceneDataEventDelegate& OnLoadFinished)
{
	SceneDataLoadDelegate = OnLoadFinished;
	HandleSceneDataUpdatedEvent();
}

void ASceneCapturesGeneratorPICO::EnableAutoLoadingSceneData(bool InAutoLoadingEnabled)
{
	if (InAutoLoadingEnabled)
	{
		if (!UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->SceneCaptureDataUpdatedDelegate.Contains(this, GET_FUNCTION_NAME_CHECKED(ASceneCapturesGeneratorPICO, HandleSceneDataUpdatedEvent)))
		{
			UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->SceneCaptureDataUpdatedDelegate.AddDynamic(this, &ASceneCapturesGeneratorPICO::HandleSceneDataUpdatedEvent);
		}
	}
	else
	{
		if (UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->SceneCaptureDataUpdatedDelegate.Contains(this, GET_FUNCTION_NAME_CHECKED(ASceneCapturesGeneratorPICO, HandleSceneDataUpdatedEvent)))
		{
			UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->SceneCaptureDataUpdatedDelegate.RemoveDynamic(this, &ASceneCapturesGeneratorPICO::HandleSceneDataUpdatedEvent);
		}
	}
}

FVector ASceneCapturesGeneratorPICO::ConvertUnityPositionToUE(const FVector& InPosition, float WorldToMetersScale)
{
	return FVector(InPosition.Z, InPosition.X, InPosition.Y) * WorldToMetersScale;
}

FQuat ASceneCapturesGeneratorPICO::ConvertUnityRotationToUE(const FQuat& InRotation)
{
	return FQuat(InRotation.Z, InRotation.X, InRotation.Y, InRotation.W);
}

void ASceneCapturesGeneratorPICO::SpawnSceneCaptures_Offline(const FMRSceneInfosPICO_Offline& Scene_Offline)
{
	float WorldToMetersScale = 100.0f;
	if (this->GetWorld() != nullptr)
	{
		WorldToMetersScale=this->GetWorld()->GetWorldSettings()->WorldToMeters;
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
		case ESemanticLabelPICO::Unknown:
			break;
		case ESemanticLabelPICO::Floor:
		case ESemanticLabelPICO::Ceiling:
			{
				SpawnPolygonCapture(MRScene.SemanticLabel,Transform,Vertices,true);
			}
			break;
		case ESemanticLabelPICO::Wall:
		case ESemanticLabelPICO::Door:
		case ESemanticLabelPICO::Window:
		case ESemanticLabelPICO::Opening:
		case ESemanticLabelPICO::Wall_Art:
		case ESemanticLabelPICO::VirtualWall:
			{
				FVector OriginScale=FVector(MRScene.Box2DInfo.Extent.Y*WorldToMetersScale,MRScene.Box2DInfo.Extent.X*WorldToMetersScale,WALL_WIDTH);
				SpawnAndRescaling2DCapture(MRScene.SemanticLabel,Location,Rotation,OriginScale);
			}
			break;
		case ESemanticLabelPICO::Table:
		case ESemanticLabelPICO::Sofa:
		case ESemanticLabelPICO::Chair:
		case ESemanticLabelPICO::Human:
		case ESemanticLabelPICO::Curtain:
		case ESemanticLabelPICO::Cabinet:
		case ESemanticLabelPICO::Bed:
		case ESemanticLabelPICO::Plant:
		case ESemanticLabelPICO::Refrigerator:
		case ESemanticLabelPICO::Washing_Machine:
		case ESemanticLabelPICO::Air_Conditioner:
		case ESemanticLabelPICO::Lamp:
		case ESemanticLabelPICO::Stairway:
		case ESemanticLabelPICO::Screen:
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

bool ASceneCapturesGeneratorPICO::SpawnAndRescaling2DCapture(ESemanticLabelPICO Label, const FVector& Location, const FRotator& Rotation,const FVector& OriginScale)
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
			if (GenerateMaps[Label].ScalingMode == ESceneCaptureScalingModePICO::Stretch)
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

bool ASceneCapturesGeneratorPICO::SpawnAndRescaling3DCapture(ESemanticLabelPICO Label, const FVector& Location, const FRotator& Rotation,const FVector& OriginScale)
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
				
			if (GenerateMaps[Label].ScalingMode==ESceneCaptureScalingModePICO::Stretch)
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
bool ASceneCapturesGeneratorPICO::SpawnPolygonCapture(ESemanticLabelPICO Label, const FTransform& Transform, const TArray<FVector>& Vertices,bool bOffline)
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
		
		if(Label == ESemanticLabelPICO::Floor ? UMRFunctionLibraryPICO::CreateSceneBoundingPolygonWithUVAdjustment(Actor,!bEnableProceduralMeshCollisionForFloor,
		false,ProceduralMeshUVAdjustmentForFloor,Transform,Vertices,ProceduralMeshMaterialForFloor,bOffline)
		:UMRFunctionLibraryPICO::CreateSceneBoundingPolygonWithUVAdjustment(Actor,!bEnableProceduralMeshCollisionForCeiling,
		true,ProceduralMeshUVAdjustmentForCeiling,Transform,Vertices,ProceduralMeshMaterialForCeiling,bOffline))
		{
			return true;
		}
	}

	return false;
	
}

void ASceneCapturesGeneratorPICO::SpawnSceneCaptures(const TArray<FMRSceneInfoPICO>& SceneInfos)
{
	for (auto SceneInfo : SceneInfos)
	{
		FRotator Rotation=FRotator(SceneInfo.ScenePose.GetRotation());
		FVector Location=SceneInfo.ScenePose.GetLocation();
		
		switch (SceneInfo.SceneType) {
		case ESceneTypePICO::BoundingBox2D:
			{
				FBoundingBox2DPICO Box2D;
				UMRFunctionLibraryPICO::GetSceneBoundingBox2DPICO(SceneInfo.UUID,Box2D);
				Location+=Box2D.Center;
				FVector OriginScale=FVector(WALL_WIDTH,Box2D.Extent.Width,Box2D.Extent.Height);
				SpawnAndRescaling2DCapture(SceneInfo.Semantic,Location,Rotation,OriginScale);
			}
			break;
		case ESceneTypePICO::BoundingPolygon:
			{
				TArray<FVector> PolygonVertices;
				UMRFunctionLibraryPICO::GetSceneBoundingPolygonPICO(SceneInfo.UUID,PolygonVertices);
				SpawnPolygonCapture(SceneInfo.Semantic,SceneInfo.ScenePose,PolygonVertices,false);
			}
			break;
		case ESceneTypePICO::BoundingBox3D:
			{
				FBoundingBox3DPICO Box3D;
				UMRFunctionLibraryPICO::GetSceneBoundingBox3DPICO(SceneInfo.UUID,Box3D);
				FVector OriginScale=FVector(Box3D.Extent.Depth,Box3D.Extent.Width,Box3D.Extent.Height);
				Location+=Box3D.Center.GetLocation();
				SpawnAndRescaling3DCapture(SceneInfo.Semantic,Location,Rotation,OriginScale);
			}
			break;
		}
	}

	
}

bool ASceneCapturesGeneratorPICO::LoadOfflineSceneData(FString ImportPath, FMRSceneInfosPICO_Offline& OutSceneInfos)
{
	OutSceneInfos.OutSceneInfos.Reset();

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
				UE_LOG(LogMRPICO, Display, TEXT("JsonObjectToUStruct Success"));
			}
			else
			{
				UE_LOG(LogMRPICO, Error, TEXT("JsonObjectToUStruct Failed"));
			}
		}
		return true;
	}

	return false;
}

void ASceneCapturesGeneratorPICO::ClearSceneCaptures()
{
	for (auto SceneCapture:SceneCaptures)
	{
		SceneCapture->Destroy();
	}
	
	SceneCaptures.Empty();
}

TArray<AActor*> ASceneCapturesGeneratorPICO::GetGeneratedActors()
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

void ASceneCapturesGeneratorPICO::HandleSceneDataUpdatedEvent()
{
	URequestSceneCapturesPICO_AsyncAction* RequestSceneCapturesAction = URequestSceneCapturesPICO_AsyncAction::RequestSceneCapturesPICO_Async(SceneLoadInfo);
	RequestSceneCapturesAction->OnSuccess.AddDynamic(this, &ASceneCapturesGeneratorPICO::HandleSceneLoadInfosEvent);
	RequestSceneCapturesAction->Activate();
}

void ASceneCapturesGeneratorPICO::HandleSceneLoadInfosEvent(EResultPICO Result, const TArray<FMRSceneInfoPICO>& SceneInfos)
{
	if (Result==EResultPICO::XR_Success)
	{
		ClearSceneCaptures();
		SpawnSceneCaptures(SceneInfos);
		SceneDataLoadDelegate.ExecuteIfBound(Result);
	}
}

void ASceneCapturesGeneratorPICO::SetScaleBasedOnRotationAndOriginScale(USceneComponent* SceneComponent, const FVector& OriginScale, const FQuat& BaseRotation, const FVector& BaseScale)
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
