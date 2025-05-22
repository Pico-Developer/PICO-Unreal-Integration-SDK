// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_MRFunctionLibrary.h"
#include "PXR_ProviderManager.h"
#include "MRMeshComponent.h"


bool UPICOXRMRFunctionLibrary::PXR_GetAnchorEntityUuid(AActor* BoundActor, FPICOSpatialUUID& OutAnchorUUID,EPICOResult& OutResult)
{
	if (FPICOProviderManager::ShouldUseLegacyMR())
	{
		return  PXR_AnchorProvider::GetInstance()->GetAnchorEntityUUIDLegacy(BoundActor, OutAnchorUUID);
	}
	return PXR_AnchorProvider::GetInstance()->GetAnchorEntityUUID(BoundActor, OutAnchorUUID,OutResult);
}

bool UPICOXRMRFunctionLibrary::PXR_GetAnchorEntityUuidByComponent(const UPICOAnchorComponent* AnchorComponent, FPICOSpatialUUID& OutAnchorUUID, EPICOResult& OutResult)
{
	if (FPICOProviderManager::ShouldUseLegacyMR())
	{
		return  PXR_AnchorProvider::GetInstance()->GetAnchorEntityUUIDLegacyByComponent(AnchorComponent, OutAnchorUUID);
	}
	return PXR_AnchorProvider::GetInstance()->GetAnchorEntityUUIDByComponent(AnchorComponent, OutAnchorUUID,OutResult);
}

bool UPICOXRMRFunctionLibrary::PXR_GetSceneBoundingBox2D(const FPICOSpatialUUID& UUID, FPICOBoundingBox2D& Box2D)
{
	if (FPICOProviderManager::ShouldUseLegacyMR())
	{
		return  PXR_AnchorProvider::GetInstance()->GetSpatialSceneBoundingBox2DLegacy(UUID,Box2D);
	}
	
	return PXR_SceneProvider::GetInstance()->GetSpatialSceneBoundingBox2D(UUID, Box2D);
}

bool UPICOXRMRFunctionLibrary::PXR_GetSceneBoundingPolygon(const FPICOSpatialUUID& UUID, TArray<FVector>& OutPolygonVertices)
{
	if (FPICOProviderManager::ShouldUseLegacyMR())
	{
		return  PXR_AnchorProvider::GetInstance()->GetSpatialSceneBoundingPolygonLegacy(UUID,OutPolygonVertices);
	}
	
	return PXR_SceneProvider::GetInstance()->GetSpatialSceneBoundingPolygon(UUID, OutPolygonVertices);
}

bool UPICOXRMRFunctionLibrary::PXR_GetSceneBoundingBox3D(const FPICOSpatialUUID& UUID, FPICOBoundingBox3D& Box3D)
{
	if (FPICOProviderManager::ShouldUseLegacyMR())
	{
		return  PXR_AnchorProvider::GetInstance()->GetSpatialSceneBoundingBox3DLegacy(UUID,Box3D);
	}
	
	return PXR_SceneProvider::GetInstance()->GetSpatialSceneBoundingBox3D(UUID, Box3D);
}

bool UPICOXRMRFunctionLibrary::PXR_GetAnchorPoseByActor(AActor* BoundActor, FTransform& OutTransform,EPICOResult& OutResult)
{
	if (!IsValid(BoundActor) || !IsValid(BoundActor->GetWorld()))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = Cast<UPICOAnchorComponent>(BoundActor->GetComponentByClass(UPICOAnchorComponent::StaticClass()));
	if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
	{
		if (FPICOProviderManager::ShouldUseLegacyMR())
		{
			return PXR_AnchorProvider::GetInstance()->GetAnchorPoseLegacy(AnchorComponent, OutTransform);
		}
	
		return PXR_AnchorProvider::GetInstance()->GetAnchorPose(AnchorComponent, OutTransform,OutResult);
	}

	return false;
}

bool UPICOXRMRFunctionLibrary::PXR_GetAnchorPoseByComponent(UPICOAnchorComponent* AnchorComponent, FTransform& OutTransform,EPICOResult& OutResult)
{
	if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
	{
		if (FPICOProviderManager::ShouldUseLegacyMR())
		{
			return PXR_AnchorProvider::GetInstance()->GetAnchorPoseLegacy(AnchorComponent, OutTransform);
		}
	
		return PXR_AnchorProvider::GetInstance()->GetAnchorPose(AnchorComponent, OutTransform,OutResult);
	}

	return false;
}

AActor* UPICOXRMRFunctionLibrary::PXR_SpawnActorFromLoadResult(UObject* WorldContext, const FAnchorLoadResult& LoadResult, UClass* ActorClass)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!IsValid(World))
	{
		PXR_LOGV(PxrMR, "UPICOXRMRFunctionLibrary::PXR_SpawnActorFromLoadResult is Invalid");
		return nullptr;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.ObjectFlags |= RF_Transient;

	AActor* AnchorActor = World->SpawnActor(ActorClass, nullptr, nullptr, SpawnInfo);
	if (!IsValid(AnchorActor))
	{
		PXR_LOGV(PxrMR, "UPICOXRMRFunctionLibrary::PXR_SpawnActorFromLoadResult Spawn Actor Failed");
		return nullptr;
	}

	UPICOAnchorComponent* AnchorComponent = Cast<UPICOAnchorComponent>(AnchorActor->GetComponentByClass(UPICOAnchorComponent::StaticClass()));
	if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
	{
		PXR_LOGV(PxrMR, "UPICOXRMRFunctionLibrary::PXR_SpawnActorFromLoadResult Anchor is Already Valid, Return Old Anchor Actor");
		return AnchorActor;
	}
	if (AnchorComponent == nullptr)
	{
		AnchorComponent = Cast<UPICOAnchorComponent>(AnchorActor->AddComponentByClass(UPICOAnchorComponent::StaticClass(), false, FTransform::Identity, false));
	}

	AnchorComponent->SetAnchorHandle(LoadResult.AnchorHandle);
	return AnchorActor;
}

bool UPICOXRMRFunctionLibrary::PXR_IsAnchorValidForActor(AActor* BoundActor)
{
	if (!IsValid(BoundActor))
	{
		return false;
	}

	UPICOAnchorComponent* AnchorComponent = Cast<UPICOAnchorComponent>(BoundActor->GetComponentByClass(UPICOAnchorComponent::StaticClass()));
	if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
	{
		return true;
	}

	return false;
}

bool UPICOXRMRFunctionLibrary::PXR_IsAnchorValidForComponent(UPICOAnchorComponent* AnchorComponent)
{
	if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
	{
		return true;
	}

	return false;
}

FString UPICOXRMRFunctionLibrary::PXR_FromAnchorToString(const FPICOSpatialHandle& Anchor)
{
	return Anchor.ToString();
}

FString UPICOXRMRFunctionLibrary::PXR_FromUUIDToString(const FPICOSpatialUUID& AnchorUUID)
{
	return AnchorUUID.ToString();
}

FPICOSpatialUUID UPICOXRMRFunctionLibrary::PXR_FromStringToUUID(const FString& AnchorUUIDString)
{
	FPICOSpatialUUID OutAnchorUUID;
	TArray<FString> UUIDArray;
	AnchorUUIDString.ParseIntoArray(UUIDArray, TEXT("-"));
	if (UUIDArray.Num() == PXR_UUID_SIZE)
	{
		for (int32 Index = 0; Index < PXR_UUID_SIZE; ++Index)
		{
			OutAnchorUUID.UUIDArray[Index] = (uint64_t)FCString::Strtoui64(*UUIDArray[Index], nullptr, 16);
		}
	}
	return OutAnchorUUID;
}

bool UPICOXRMRFunctionLibrary::PXR_CloseSpatialMeshScanning()
{
	return FPICOProviderManager::PXR_StopSenseDataProvider(EPICOProviderType::Pico_Provider_Mesh);
}

bool UPICOXRMRFunctionLibrary::PXR_ChangeSpatialMeshLodSetting(EPICOSpatialMeshLod SpatialMeshLod)
{
	bool bResult=false;
	
	if (PXR_MeshProvider::GetInstance()->GetCurrentSpatialMeshLod()!=SpatialMeshLod)
	{
		bResult=FPICOProviderManager::PXR_StopSenseDataProvider(EPICOProviderType::Pico_Provider_Mesh);

		if (bResult)
		{
			bResult=FPICOProviderManager::PXR_DestroySenseDataProvider(EPICOProviderType::Pico_Provider_Mesh);
		
			if (bResult)
			{
				FPICOSenseDataProviderCreateInfoMesh cFPICOSenseDataProviderCreateInfoMesh = {};
				cFPICOSenseDataProviderCreateInfoMesh.Lod = SpatialMeshLod;
				const UPICOXRSettings* Settings = GetDefault<UPICOXRSettings>();

				if(Settings)
				{
					if (Settings->bSemanticsAlignWithTriangle)
					{
						cFPICOSenseDataProviderCreateInfoMesh.ConfigArray.Add(EPICOSpatialMeshConfig::Spatial_Mesh_Config_Semantic);
					}
					else if (Settings->bSemanticsAlignWithVertex)
					{
						cFPICOSenseDataProviderCreateInfoMesh.ConfigArray.Add(EPICOSpatialMeshConfig::Spatial_Mesh_Config_Semantic);
						cFPICOSenseDataProviderCreateInfoMesh.ConfigArray.Add(EPICOSpatialMeshConfig::Spatial_Mesh_Config_Semantic_Align_With_Vertex);
					}
				}
				
				bResult=FPICOProviderManager::PXR_CreateSenseDataProvider(cFPICOSenseDataProviderCreateInfoMesh);
			}
		}
	}
	
	return bResult;
}

EPICOSpatialMeshLod UPICOXRMRFunctionLibrary::PXR_GetSpatialMeshLodSetting()
{
	return PXR_MeshProvider::GetInstance()->GetCurrentSpatialMeshLod();
}

EPICOMRState UPICOXRMRFunctionLibrary::PXR_GetSpatialMeshScanningState()
{
	return PXR_MeshProvider::GetInstance()->GetSenseDataProviderState();
}

EPICOMRState UPICOXRMRFunctionLibrary::PXR_GetSpatialAnchorServiceState()
{
	return PXR_AnchorProvider::GetInstance()->GetSenseDataProviderState();
}

EPICOMRState UPICOXRMRFunctionLibrary::PXR_GetSceneCaptureServiceState()
{
	return PXR_SceneProvider::GetInstance()->GetSenseDataProviderState();
}

void UPICOXRMRFunctionLibrary::PXR_ResetSpatialMeshInfosState()
{
	PXR_MeshProvider::GetInstance()->ClearMeshProviderBuffer();
}

bool UPICOXRMRFunctionLibrary::PXR_CloseSpatialAnchorService()
{
	return FPICOProviderManager::PXR_StopSenseDataProvider(EPICOProviderType::Pico_Provider_Anchor);
}

bool UPICOXRMRFunctionLibrary::PXR_CloseSceneCaptureService()
{
	return FPICOProviderManager::PXR_StopSenseDataProvider(EPICOProviderType::Pico_Provider_Scene_Capture);
}


bool UPICOXRMRFunctionLibrary::PXR_DestroyAnchorByComponent(UPICOAnchorComponent* AnchorComponent,EPICOResult& OutResult)
{
	bool bResult=false;

	if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
	{
		if (FPICOProviderManager::ShouldUseLegacyMR())
		{
			bResult=PXR_AnchorProvider::GetInstance()->DestroyAnchorByHandleLegacy(AnchorComponent->GetAnchorHandle());
		}
		else
		{
			bResult=PXR_AnchorProvider::GetInstance()->DestroyAnchorByHandle(AnchorComponent->GetAnchorHandle(),OutResult);
		}
		AnchorComponent->ResetAnchorHandle();
	}

	return bResult;
}


bool UPICOXRMRFunctionLibrary::PXR_DestroyAnchorByActor(AActor* BoundActor,EPICOResult& OutResult)
{
	bool bResult=false;
	if (!IsValid(BoundActor) || !IsValid(BoundActor->GetWorld()))
	{
		return bResult;
	}

	UPICOAnchorComponent* AnchorComponent = Cast<UPICOAnchorComponent>(BoundActor->GetComponentByClass(UPICOAnchorComponent::StaticClass()));

	if (IsValid(AnchorComponent) && AnchorComponent->IsAnchorValid())
	{
		if (FPICOProviderManager::ShouldUseLegacyMR())
		{
			bResult=PXR_AnchorProvider::GetInstance()->DestroyAnchorByHandleLegacy(AnchorComponent->GetAnchorHandle());
		}
		else
		{
			bResult=PXR_AnchorProvider::GetInstance()->DestroyAnchorByHandle(AnchorComponent->GetAnchorHandle(),OutResult);
		}
		AnchorComponent->ResetAnchorHandle();
	}

	return bResult;
}

bool UPICOXRMRFunctionLibrary::PXR_CreateSceneBoundingPolygon(AActor* BoundActor,bool bNeverCreateCollision,bool bFlipPolygon,bool UseWireframe, const FTransform& Transform, const TArray<FVector>& BoundaryVertices,UMaterialInterface* DefaultMeshMaterial)
{
	auto MRMeshComponent = NewObject<UProceduralMeshComponent>(BoundActor);
	TArray<int32> Indices;
	TArray<FVector> Normals;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
	TArray<FVector2D> UV0;
	
	if (!IsValid(BoundActor)||!BoundaryVertices.Num()||!DefaultMeshMaterial)
	{
		return false;
	}
	
	MRMeshComponent->SetUsingAbsoluteLocation(true);
	MRMeshComponent->SetUsingAbsoluteRotation(true);
	MRMeshComponent->SetUsingAbsoluteScale(true);
	
	MRMeshComponent->SetMaterial(0, DefaultMeshMaterial);

	MRMeshComponent->SetupAttachment(BoundActor->GetRootComponent());
	MRMeshComponent->RegisterComponent();
	
	auto Vertices = BoundaryVertices;
	const auto NumPolygons = Vertices.Num();
	Indices.Reset(3 * NumPolygons);
	if (bFlipPolygon)
	{
		for (auto Index = 0; Index < NumPolygons; ++Index)
		{
			Indices.Add(0);
			Indices.Add((Index + 2) % NumPolygons);
			Indices.Add((Index + 1) % NumPolygons);
		}
	}
	else
	{
		for (auto Index = 0; Index < NumPolygons; ++Index)
		{
			Indices.Add(0);
			Indices.Add((Index + 1) % NumPolygons);
			Indices.Add((Index + 2) % NumPolygons);
		}
	}
			
	MRMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Indices, Normals, UV0, VertexColors, Tangents, !bNeverCreateCollision);
	MRMeshComponent->SetWorldLocation(Transform.GetLocation());
	MRMeshComponent->SetWorldRotation(Transform.GetRotation());
	return true;
}

bool UPICOXRMRFunctionLibrary::PXR_CreateSceneBoundingPolygonWithUVAdjustment(AActor* BoundActor, bool bNeverCreateCollision, bool bFlipPolygon, const FPICOUVAdjustment& UVAdjustment, const FTransform& Transform, const TArray<FVector>& BoundaryVertices, UMaterialInterface* DefaultMeshMaterial)
{
	auto MRMeshComponent = NewObject<UProceduralMeshComponent>(BoundActor);

	if (!IsValid(BoundActor) || !BoundaryVertices.Num() || !DefaultMeshMaterial)
	{
		return false;
	}

	MRMeshComponent->SetUsingAbsoluteLocation(true);
	MRMeshComponent->SetUsingAbsoluteRotation(true);
	MRMeshComponent->SetUsingAbsoluteScale(true);

	MRMeshComponent->SetMaterial(0, DefaultMeshMaterial);
	MRMeshComponent->SetupAttachment(BoundActor->GetRootComponent());
	MRMeshComponent->RegisterComponent();

	auto Vertices = BoundaryVertices;
	const auto NumPolygons = Vertices.Num();
	TArray<int32> Indices;
	TArray<FVector> Normals;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;
	TArray<FVector2D> UV0;
	Normals.Reserve(NumPolygons);
	UV0.Reserve(NumPolygons);
	Indices.Reserve(3 * NumPolygons);
	const int Index1 = bFlipPolygon ? 2 : 1;
	const int Index2 = bFlipPolygon ? 1 : 2;
	const FVector Normal = -FVector::XAxisVector;

	for (auto Index = 0; Index < NumPolygons; ++Index)
	{
		Indices.Add(0);
		Indices.Add((Index + Index1) % NumPolygons);
		Indices.Add((Index + Index2) % NumPolygons);
		Normals.Push(Normal);
	}

	// Calculate UV coordinates
	// First, find the bounding box of the vertices
	FVector MinBounds(FLT_MAX, FLT_MAX, FLT_MAX);
	FVector MaxBounds(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (const auto& Vertex : Vertices)
	{
		// Manually update the minimum bounds
		if (Vertex.X < MinBounds.X) MinBounds.X = Vertex.X;
		if (Vertex.Y < MinBounds.Y) MinBounds.Y = Vertex.Y;
		if (Vertex.Z < MinBounds.Z) MinBounds.Z = Vertex.Z;

		// Manually update the maximum bounds
		if (Vertex.X > MaxBounds.X) MaxBounds.X = Vertex.X;
		if (Vertex.Y > MaxBounds.Y) MaxBounds.Y = Vertex.Y;
		if (Vertex.Z > MaxBounds.Z) MaxBounds.Z = Vertex.Z;
	}

	// Calculate the size of the bounding box
	FVector BoundsSize = MaxBounds - MinBounds;

	// Calculate UV coordinates for each vertex
	for (const auto& Vertex : Vertices)
	{
		// Map the vertex position to the [0, 1] range
		FVector2D UV;
#if WITH_EDITOR
		UV.X = (Vertex.X - MinBounds.X) / BoundsSize.X;
		UV.Y = bFlipPolygon? 1 - (Vertex.Y - MinBounds.Y) / BoundsSize.Y : (Vertex.Y - MinBounds.Y) / BoundsSize.Y;

#else
		UV.X = (Vertex.Y - MinBounds.Y) / BoundsSize.Y;
		UV.Y = bFlipPolygon? 1 - (Vertex.Z - MinBounds.Z) / BoundsSize.Z : (Vertex.Z - MinBounds.Z) / BoundsSize.Z;
#endif

		float rotationInRadians = FMath::DegreesToRadians(UVAdjustment.Rotation);
		float cosAngle = FMath::Cos(rotationInRadians);
		float sinAngle = FMath::Sin(rotationInRadians);
		FVector2D rotatedUV;
		rotatedUV.X = UV.X * cosAngle - UV.Y * sinAngle;
		rotatedUV.Y = UV.X * sinAngle + UV.Y * cosAngle;
		
		UV = rotatedUV * UVAdjustment.Scale + UVAdjustment.Offset;
		UV0.Add(UV);
	}

	MRMeshComponent->CreateMeshSection_LinearColor(0, Vertices, Indices, Normals, UV0, VertexColors, Tangents, !bNeverCreateCollision);
	MRMeshComponent->SetWorldLocation(Transform.GetLocation());
	MRMeshComponent->SetWorldRotation(Transform.GetRotation());

	return true;
}
