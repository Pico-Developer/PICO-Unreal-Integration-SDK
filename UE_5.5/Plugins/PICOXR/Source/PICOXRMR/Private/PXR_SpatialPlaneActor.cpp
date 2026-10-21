// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_SpatialPlaneActor.h"
#include "PXR_EventManager.h"
#include "PXR_MRAsyncActions.h"
#include "PXR_ProviderManager.h"
#include "PXR_SpatialPlaneComponent.h"
#include "Algo/Transform.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"

#define PLANE_UPDATE_FRAME_NUM_MAX 1

APICOSpatialPlaneActor::APICOSpatialPlaneActor(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	if (SpatialPlaneMaterial==nullptr)
	{
		static ConstructorHelpers::FObjectFinder<UMaterial> DefaultMaterialFinder(TEXT("/Script/Engine.Material'/PICOXR/Materials/Mat_WireframeMaterial.Mat_WireframeMaterial'"));
		SpatialPlaneMaterial= DefaultMaterialFinder.Object;
	}
	
	for (EPICOSemanticLabel Val : TEnumRange<EPICOSemanticLabel>())
	{
		SemanticToColors.Emplace(Val, FColor::MakeRandomColor());
	}
}

void APICOSpatialPlaneActor::BeginPlay()
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

void APICOSpatialPlaneActor::HandlePlaneDataUpdatedEvent()
{
	UPICORequestSpatialPlane_AsyncAction* RequestSpatialPlaneAction = UPICORequestSpatialPlane_AsyncAction::PXR_RequestSpatialPlaneInfos_Async();
	RequestSpatialPlaneAction->OnSuccess.AddDynamic(this, &APICOSpatialPlaneActor::HandleRequestSpatialPlaneContentsEvent);
	RequestSpatialPlaneAction->Activate();
}

void APICOSpatialPlaneActor::Tick(float DeltaTime)
{
	TArray<FPICOSpatialPlaneInfo> MRPlaneInfos;
	PlaneInfoQueue.Dequeue(MRPlaneInfos);

	UE_LOG(PxrMR, Verbose, TEXT("ASpatialPlaneActorPICO::Tick MRPlaneInfos:%d"),MRPlaneInfos.Num());

	for (auto PlaneInfo : MRPlaneInfos)
	{
		switch (PlaneInfo.State)
		{
		case EPICOSpatialMeshState::Added:
			{
				UPICOSpatialPlaneComponent* PlaneComponentInstance = PlaneComponentClass==nullptr?NewObject<UPICOSpatialPlaneComponent>(this):NewObject<UPICOSpatialPlaneComponent>(this, PlaneComponentClass);
				UE_LOG(PxrMR, Verbose, TEXT("ASpatialPlaneActorPICO::Tick Added:%s"),*PlaneInfo.UUID.ToString());

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
		case EPICOSpatialMeshState::Stable:
			break;
		case EPICOSpatialMeshState::Updated:
			{
				UE_LOG(PxrMR, Verbose, TEXT("ASpatialPlaneActorPICO::Tick Updated:%s"),*PlaneInfo.UUID.ToString());

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
		case EPICOSpatialMeshState::Removed:
			{
				UE_LOG(PxrMR, Verbose, TEXT("ASpatialPlaneActorPICO::Tick Removed:%s"),*PlaneInfo.UUID.ToString());

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

void APICOSpatialPlaneActor::HandleRequestSpatialPlaneContentsEvent(EPICOResult Result, const TArray<FPICOSpatialPlaneInfo>& PlaneInfos)
{
	TArray<FPICOSpatialPlaneInfo> MRPlaneInfos = {};
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

void APICOSpatialPlaneActor::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	ClearPlane();
}

bool APICOSpatialPlaneActor::StartDraw()
{
	SetActorTickEnabled(true);
	if (!UPICOXRHMDFunctionLibrary::PXR_GetEventManager()->PlaneDataUpdatedDelegate.Contains(this, GET_FUNCTION_NAME_CHECKED(APICOSpatialPlaneActor, HandlePlaneDataUpdatedEvent)))
	{
		UPICOXRHMDFunctionLibrary::PXR_GetEventManager()->PlaneDataUpdatedDelegate.AddDynamic(this, &APICOSpatialPlaneActor::HandlePlaneDataUpdatedEvent);
		return true;
	}
	
	return false;
}

bool APICOSpatialPlaneActor::PauseDraw()
{
	SetActorTickEnabled(false);
	if (UPICOXRHMDFunctionLibrary::PXR_GetEventManager()->PlaneDataUpdatedDelegate.Contains(this, GET_FUNCTION_NAME_CHECKED(APICOSpatialPlaneActor, HandlePlaneDataUpdatedEvent)))
	{
		UPICOXRHMDFunctionLibrary::PXR_GetEventManager()->PlaneDataUpdatedDelegate.RemoveDynamic(this, &APICOSpatialPlaneActor::HandlePlaneDataUpdatedEvent);
		return true;
	}

	return false;
}

void APICOSpatialPlaneActor::SetPlaneVisibility(bool visibility)
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

bool APICOSpatialPlaneActor::ClearPlane()
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
	PXR_PlaneProvider::GetInstance()->ClearPlaneProviderBuffer();
	
	return true;
}

int32 APICOSpatialPlaneActor::GetPlaneNum()
{
	return EntityToPlaneMap.Num();
}

FColor APICOSpatialPlaneActor::GetColorBySceneLabel(EPICOSemanticLabel SceneLabel)
{
	return SemanticToColors.Contains(SceneLabel)?SemanticToColors[SceneLabel]:FColor::MakeRandomColor();
}

bool APICOSpatialPlaneActor::UpdatePlaneByPlaneInfo(UPICOSpatialPlaneComponent* SpatialMesh, const FPICOSpatialPlaneInfo& PlaneInfo)
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
    				EPICOSemanticLabel Semantic = PlaneInfo.Semantic;
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
