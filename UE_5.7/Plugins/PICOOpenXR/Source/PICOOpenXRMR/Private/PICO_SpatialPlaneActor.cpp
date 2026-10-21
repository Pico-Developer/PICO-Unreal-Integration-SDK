// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_SpatialPlaneActor.h"
#include "PICO_MRAsyncActions.h"
#include "PICO_MRFunctionLibrary.h"
#include "PICO_SpatialPlane.h"
#include "ProceduralMeshComponent.h"
#include "PICO_SpatialPlaneComponent.h"
#include "Algo/Transform.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"

#define PLANE_UPDATE_FRAME_NUM_MAX 1

ASpatialPlaneActorPICO::ASpatialPlaneActorPICO(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	if (SpatialPlaneMaterial==nullptr)
	{
		static ConstructorHelpers::FObjectFinder<UMaterial> DefaultMaterialFinder(TEXT("/Script/Engine.Material'/PICOOpenXR/Materials/M_Wireframe.M_Wireframe'"));
		SpatialPlaneMaterial= DefaultMaterialFinder.Object;
	}
	
	for (ESemanticLabelPICO Val : TEnumRange<ESemanticLabelPICO>())
	{
		SemanticToColors.Emplace(Val, FColor::MakeRandomSeededColor(static_cast<int32>(Val)));
	}
}

void ASpatialPlaneActorPICO::BeginPlay()
{
	Super::BeginPlay();

	USceneComponent* RootSceneComponent = NewObject<USceneComponent>(this, USceneComponent::StaticClass());
	RootSceneComponent->SetMobility(EComponentMobility::Static);
	RootSceneComponent->RegisterComponent();
	SetRootComponent(RootSceneComponent);

	if (bDrawOnBeginPlay)
	{
		StartDraw();
	}
}

void ASpatialPlaneActorPICO::HandlePlaneDataUpdatedEvent()
{
	URequestSpatialPlanePICO_AsyncAction* RequestSpatialPlaneAction = URequestSpatialPlanePICO_AsyncAction::RequestSpatialPlaneInfosPICO_Async();
	RequestSpatialPlaneAction->OnSuccess.AddDynamic(this, &ASpatialPlaneActorPICO::HandleRequestSpatialPlaneContentsEvent);
	RequestSpatialPlaneAction->Activate();
}

void ASpatialPlaneActorPICO::Tick(float DeltaTime)
{
	TArray<FSpatialPlaneInfoPICO> MRPlaneInfos;
	PlaneInfoQueue.Dequeue(MRPlaneInfos);

	UE_LOG(LogMRPICO, Verbose, TEXT("ASpatialPlaneActorPICO::Tick MRPlaneInfos:%d"),MRPlaneInfos.Num());

	for (auto PlaneInfo : MRPlaneInfos)
	{

		switch (PlaneInfo.State)
		{
		case ESpatialMeshStatePICO::Added:
			{
				USpatialPlaneComponentPICO* PlaneComponentInstance = PlaneComponentClass==nullptr?NewObject<USpatialPlaneComponentPICO>(this):NewObject<USpatialPlaneComponentPICO>(this, PlaneComponentClass);
				UE_LOG(LogMRPICO, Verbose, TEXT("ASpatialPlaneActorPICO::Tick Added:%s"),*PlaneInfo.UUID.ToString());

				PlaneComponentInstance->RegisterComponent();
				PlaneComponentInstance->SetMaterial(0,SpatialPlaneMaterial);
				PlaneComponentInstance->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
				PlaneComponentInstance->SetVisibility(bSpatialPlaneVisible);
				PlaneComponentInstance->SetCollisionEnabled(CollisionType);
				AddOwnedComponent(PlaneComponentInstance);

				if (EntityToPlaneMap.Contains(PlaneInfo.UUID))
				{
					if (EntityToPlaneMap[PlaneInfo.UUID]!=nullptr)
					{
						EntityToPlaneMap[PlaneInfo.UUID]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						EntityToPlaneMap[PlaneInfo.UUID]->DestroyComponent();
					}
					EntityToPlaneMap.Remove(PlaneInfo.UUID);
				}

				EntityToPlaneMap.Emplace(PlaneInfo.UUID, PlaneComponentInstance);

				UpdatePlaneByPlaneInfo(PlaneComponentInstance, PlaneInfo);
			}
			break;
		case ESpatialMeshStatePICO::Stable:
			break;
		case ESpatialMeshStatePICO::Updated:
			{
				UE_LOG(LogMRPICO, Verbose, TEXT("ASpatialPlaneActorPICO::Tick Updated:%s"),*PlaneInfo.UUID.ToString());

				if (EntityToPlaneMap.Contains(PlaneInfo.UUID))
				{
					if (EntityToPlaneMap[PlaneInfo.UUID] == nullptr)
					{
						continue;
					}
					UpdatePlaneByPlaneInfo(EntityToPlaneMap[PlaneInfo.UUID], PlaneInfo);
				}
			}
			break;
		case ESpatialMeshStatePICO::Removed:
			{
				UE_LOG(LogMRPICO, Verbose, TEXT("ASpatialPlaneActorPICO::Tick Removed:%s"),*PlaneInfo.UUID.ToString());

				if (EntityToPlaneMap.Contains(PlaneInfo.UUID)
					&&EntityToPlaneMap[PlaneInfo.UUID]!=nullptr)
				{

					EntityToPlaneMap[PlaneInfo.UUID]->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					EntityToPlaneMap[PlaneInfo.UUID]->DestroyComponent();
					EntityToPlaneMap.Remove(PlaneInfo.UUID);
				}
			}
			break;
		default: ;
		}
	}
}

void ASpatialPlaneActorPICO::HandleRequestSpatialPlaneContentsEvent(EResultPICO Result, const TArray<FSpatialPlaneInfoPICO>& PlaneInfos)
{
	TArray<FSpatialPlaneInfoPICO> MRPlaneInfos = {};
	int32 Counter = 0;
	bool bIsDone = false;
	const int32 PlaneCountPerFrame= FMath::CeilToInt((static_cast<float>(PlaneInfos.Num())) / PLANE_UPDATE_FRAME_NUM_MAX);

	for (auto PlaneInfo : PlaneInfos)
	{
		Counter++;
		if (Counter < PlaneCountPerFrame)
		{
			bIsDone = false;
			MRPlaneInfos.Add(PlaneInfo);
		}
		else
		{
			bIsDone = true;
			MRPlaneInfos.Add(PlaneInfo);
			PlaneInfoQueue.Enqueue(MRPlaneInfos);
			Counter = 0;
			MRPlaneInfos.Empty();
		}
	}

	if (!bIsDone)
	{
		PlaneInfoQueue.Enqueue(MRPlaneInfos);
		MRPlaneInfos.Empty();
	}
}

void ASpatialPlaneActorPICO::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	ClearPlane();
}

bool ASpatialPlaneActorPICO::StartDraw()
{
	SetActorTickEnabled(true);
	if (!UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->PlaneDataUpdatedDelegate.Contains(this, GET_FUNCTION_NAME_CHECKED(ASpatialPlaneActorPICO, HandlePlaneDataUpdatedEvent)))
	{
		UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->PlaneDataUpdatedDelegate.AddDynamic(this, &ASpatialPlaneActorPICO::HandlePlaneDataUpdatedEvent);
		return true;
	}
	
	return false;
}

bool ASpatialPlaneActorPICO::PauseDraw()
{
	SetActorTickEnabled(false);
	if (UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->PlaneDataUpdatedDelegate.Contains(this, GET_FUNCTION_NAME_CHECKED(ASpatialPlaneActorPICO, HandlePlaneDataUpdatedEvent)))
	{
		UMRFunctionLibraryPICO::GetMRDelegateManagerPICO()->PlaneDataUpdatedDelegate.RemoveDynamic(this, &ASpatialPlaneActorPICO::HandlePlaneDataUpdatedEvent);
		return true;
	}

	return false;
}

void ASpatialPlaneActorPICO::SetPlaneVisibility(bool visibility)
{
	bSpatialPlaneVisible = visibility;
	for (const auto Pair : EntityToPlaneMap)
	{
		if (Pair.Value)
		{
			Pair.Value->SetVisibility(visibility);
		}
	}
}

bool ASpatialPlaneActorPICO::ClearPlane()
{
	for (const auto Pair : EntityToPlaneMap)
	{
		if (Pair.Value)
		{
			Pair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			Pair.Value->DestroyComponent();
		}
	}
	EntityToPlaneMap.Empty();
	PlaneInfoQueue.Empty();
	FSpatialPlaneExtensionPICO::GetInstance()->ClearPlaneProviderBuffer();
	
	return true;
}

int32 ASpatialPlaneActorPICO::GetPlaneNum()
{
	return EntityToPlaneMap.Num();
}

FColor ASpatialPlaneActorPICO::GetColorBySceneLabel(ESemanticLabelPICO SceneLabel)
{
	return SemanticToColors.Contains(SceneLabel)?SemanticToColors[SceneLabel]:FColor::MakeRandomSeededColor(static_cast<int32>(SceneLabel));
}

bool ASpatialPlaneActorPICO::UpdatePlaneByPlaneInfo(USpatialPlaneComponentPICO* SpatialMesh, const FSpatialPlaneInfoPICO& PlaneInfo)
{
	if (SpatialMesh)
	{
		SpatialMesh->SetPlaneSemantic(PlaneInfo.Semantic);
		SpatialMesh->SetPlaneOrientation(PlaneInfo.PlaneOrientation);
		TArray<int32> TempIndices= PlaneInfo.Indices;
		TArray<FVector> TempVertices= PlaneInfo.Vertices;
		SpatialMesh->SetWorldLocationAndRotation(PlaneInfo.PlanePose.GetLocation(), PlaneInfo.PlanePose.GetRotation());
		
		if (TempVertices.Num() && TempIndices.Num())
		{
			TArray<FVector> EmptyNormals;
			TArray<FVector2D> EmptyUV;
			TArray<FProcMeshTangent> EmptyTangents;
			TArray<FLinearColor> VertexColors;
			for (int32 Index = 0; Index < TempVertices.Num(); ++Index)
			{
				ESemanticLabelPICO Semantic = PlaneInfo.Semantic;
				VertexColors.Add(GetColorBySceneLabel(Semantic));
			}
			
			//Create or update the mesh depending on if we've been created before
			if (SpatialMesh->GetNumSections() > 0 && SpatialMesh->IsEqualWithCached(TempIndices))
			{
				SpatialMesh->UpdateMeshSection_LinearColor(0, TempVertices, EmptyNormals, EmptyUV, VertexColors, EmptyTangents);
			}
			else
			{
				SpatialMesh->CreateMeshSection_LinearColor(0, TempVertices, TempIndices, EmptyNormals, EmptyUV, VertexColors, EmptyTangents, CollisionType != ECollisionEnabled::Type::NoCollision);
				SpatialMesh->SetCachedIndices(TempIndices);
			}
		}
		return true;
	}

	return false;
}
