// Fill out your copyright notice in the Description page of Project Settings.


#include "PICO_PokeHoleComponent.h"
#include "Engine/Texture.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "TextureResource.h"

UPokeHoleComponentPICO::UPokeHoleComponentPICO(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, TargetStereoLayerComponentName(NAME_None)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaskedMaterialRef(TEXT("/Script/Engine.Material'/PICOOpenXR/Materials/M_PokeHole.M_PokeHole'"));
	OverrideMaterial = MaskedMaterialRef.Object;
}

template <typename T>
T* FindComponentByNamePICO(AActor* Actor, const FName& ComponentName)
{
	if (IsValid(Actor) && (ComponentName != NAME_None))
	{
		TArray<T*> ComponentsOfType;
		Actor->GetComponents<T>(ComponentsOfType);
		T** FoundComponent = ComponentsOfType.FindByPredicate([Name = ComponentName.ToString()](T* Component) { return Component->GetName().Equals(Name); });

		if (FoundComponent != nullptr)
		{
			return *FoundComponent;
		}
	}

	return nullptr;
}

void UPokeHoleComponentPICO::OnRegister()
{
	Super::OnRegister();

	ClearAllMeshSections();

	if (TargetStereoLayerComponentName != NAME_None)
	{
		StereoLayer = FindComponentByNamePICO<UStereoLayerComponent>(GetOwner(), TargetStereoLayerComponentName);
	}

	if (StereoLayer == nullptr)
	{
		SetComponentTickEnabled(false);
	}
	else
	{
		bLastQuadPreserveTextureRatio = StereoLayer->bQuadPreserveTextureRatio;
	}

	CastShadow = false;
}

void UPokeHoleComponentPICO::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (StereoLayer == nullptr)
	{
		return;
	}

	bool bRatioChanged = StereoLayer->bQuadPreserveTextureRatio != bLastQuadPreserveTextureRatio;
	if (bRatioChanged)
	{
		bLastQuadPreserveTextureRatio = StereoLayer->bQuadPreserveTextureRatio;
		ClearAllMeshSections();
	}

	if (StereoLayer->bSupportsDepth && GetNumSections() < 1)
	{
		TArray<FVector> VerticePos;
		TArray<int32> TriangleIndics;
		TArray<FVector> Normals;
		TArray<FVector2D> TexUV;
		TArray<FLinearColor> VertexColors;
		TArray<FProcMeshTangent> Tangents;

		CreatePokeHoleMesh(StereoLayer, VerticePos, TriangleIndics, TexUV);
		CreateMeshSection_LinearColor(0, VerticePos, TriangleIndics, Normals, TexUV, VertexColors, Tangents, false);

		if (OverrideMaterial)
		{
			SetMaterial(0, OverrideMaterial);
		}
	}
	else if (!StereoLayer->bSupportsDepth && GetNumSections() > 0)
	{
		ClearAllMeshSections();
	}

	SetWorldTransform(StereoLayer->GetComponentTransform());
}

void UPokeHoleComponentPICO::OnUnregister()
{
	Super::OnUnregister();

	ClearAllMeshSections();
}

static void AddFaceIndices(const int v0, const int v1, const int v2, const int v3, TArray<int32>& Triangles, bool inverse)
{
	if (inverse)
	{
		Triangles.Add(v0);
		Triangles.Add(v2);
		Triangles.Add(v1);
		Triangles.Add(v0);
		Triangles.Add(v3);
		Triangles.Add(v2);
	}
	else
	{
		Triangles.Add(v0);
		Triangles.Add(v1);
		Triangles.Add(v2);
		Triangles.Add(v0);
		Triangles.Add(v2);
		Triangles.Add(v3);
	}
}

void UPokeHoleComponentPICO::CreatePokeHoleMesh(UStereoLayerComponent* InStereoLayer, TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector2D>& UV0)
{
	if (InStereoLayer == nullptr)
	{
		return;
	}

	if(Shape->IsA<UStereoLayerShapeQuad>())
	{
		FIntPoint TexSize = InStereoLayer->GetTexture() != nullptr ? InStereoLayer->GetTexture()->GetResource()->TextureRHI->GetTexture2D()->GetSizeXY() : FIntPoint(0, 0);
		float AspectRatio = TexSize.X ? (float)TexSize.Y / (float)TexSize.X : 3.0f / 4.0f;

		float QuadSizeX = InStereoLayer->GetQuadSize().X;
		float QuadSizeY = InStereoLayer->bQuadPreserveTextureRatio ? InStereoLayer->GetQuadSize().X * AspectRatio : InStereoLayer->GetQuadSize().Y;

		Vertices.Init(FVector::ZeroVector, 4);
		Vertices[0] = FVector(0.0, -QuadSizeX / 2, -QuadSizeY / 2) * QuadScale;
		Vertices[1] = FVector(0.0, QuadSizeX / 2, -QuadSizeY / 2) * QuadScale;
		Vertices[2] = FVector(0.0, QuadSizeX / 2, QuadSizeY / 2) * QuadScale;
		Vertices[3] = FVector(0.0, -QuadSizeX / 2, QuadSizeY / 2) * QuadScale;

		UV0.Init(FVector2D::ZeroVector, 4);
		UV0[0] = FVector2D(0, 1);
		UV0[1] = FVector2D(1, 1);
		UV0[2] = FVector2D(1, 0);
		UV0[3] = FVector2D(0, 0);

		Triangles.Reserve(6);
		AddFaceIndices(0, 1, 2, 3, Triangles, false);
	}
	else if (Shape->IsA<UStereoLayerShapeCylinder>())
	{
		float Arc, Radius, Height;
		const UStereoLayerShapeCylinder* CylinderProps = Cast<UStereoLayerShapeCylinder>(Shape);
		Arc = CylinderProps->OverlayArc;
		Radius = CylinderProps->Radius;
		Height = CylinderProps->Height;

		FIntPoint TexSize = InStereoLayer->GetTexture() != nullptr ? InStereoLayer->GetTexture()->GetResource()->TextureRHI->GetTexture2D()->GetSizeXY() : FIntPoint(0, 0);
		float AspectRatio = TexSize.X ? (float)TexSize.Y / (float)TexSize.X : 3.0f / 4.0f;
		
		float CylinderHeight = InStereoLayer->bQuadPreserveTextureRatio ? Arc * AspectRatio : Height;

		const FVector XAxis = FVector(1, 0, 0);
		const FVector YAxis = FVector(0, 1, 0);
		const FVector HalfHeight = FVector(0, 0, CylinderHeight / 2);

		const float ArcAngle = Arc / Radius;
		const int Sides = (int)((ArcAngle * 180) / (PI * 5));
		Vertices.Init(FVector::ZeroVector, 2 * (Sides + 1));
		UV0.Init(FVector2D::ZeroVector, 2 * (Sides + 1));
		Triangles.Init(0, Sides * 6);

		float CurrentAngle = -ArcAngle / 2;
		const float AngleStep = ArcAngle / Sides;

		for (int Side = 0; Side < Sides + 1; Side++)
		{
			FVector MidVertex = Radius * (FMath::Cos(CurrentAngle) * XAxis + FMath::Sin(CurrentAngle) * YAxis);
			Vertices[2 * Side] = (MidVertex - HalfHeight) * CylinderScale;
			Vertices[(2 * Side) + 1] = (MidVertex + HalfHeight) * CylinderScale;

			UV0[2 * Side] = FVector2D((Side / (float)Sides), 1);
			UV0[(2 * Side) + 1] = FVector2D((Side / (float)Sides), 0);

			CurrentAngle += AngleStep;

			if (Side < Sides)
			{
				Triangles[6 * Side + 0] = 2 * Side;
				Triangles[6 * Side + 2] = 2 * Side + 1;
				Triangles[6 * Side + 1] = 2 * (Side + 1) + 1;
				Triangles[6 * Side + 3] = 2 * Side;
				Triangles[6 * Side + 5] = 2 * (Side + 1) + 1;
				Triangles[6 * Side + 4] = 2 * (Side + 1);
			}
		}
	}
	else if (Shape->IsA<UStereoLayerShapeCubemap>())
	{
		Vertices.Init(FVector::ZeroVector, 8);
		Vertices[0] = FVector(-1.0, -1.0, -1.0) * CubemapScale;
		Vertices[1] = FVector(-1.0, -1.0, 1.0) * CubemapScale;
		Vertices[2] = FVector(-1.0, 1.0, -1.0) * CubemapScale;
		Vertices[3] = FVector(-1.0, 1.0, 1.0) * CubemapScale;
		Vertices[4] = FVector(1.0, -1.0, -1.0) * CubemapScale;
		Vertices[5] = FVector(1.0, -1.0, 1.0) * CubemapScale;
		Vertices[6] = FVector(1.0, 1.0, -1.0) * CubemapScale;
		Vertices[7] = FVector(1.0, 1.0, 1.0) * CubemapScale;

		AddFaceIndices(0, 1, 3, 2, Triangles, !bFlipCubeFace);
		AddFaceIndices(4, 5, 7, 6, Triangles, bFlipCubeFace);
		AddFaceIndices(0, 1, 5, 4, Triangles, bFlipCubeFace);
		AddFaceIndices(2, 3, 7, 6, Triangles, !bFlipCubeFace);
		AddFaceIndices(0, 2, 6, 4, Triangles, !bFlipCubeFace);
		AddFaceIndices(1, 3, 7, 5, Triangles, bFlipCubeFace);
	}
	else if (Shape->IsA<UStereoLayerShapeEquirect>())
	{
		const UStereoLayerShapeEquirect* EquirectProps = Cast<UStereoLayerShapeEquirect>(Shape);

		float Radius = FMath::Abs(CustomRadius > 0 ? CustomRadius : EquirectProps->Radius);

		Radius = Radius > 0 ? Radius : 10000.0f;
		
		Vertices.Empty();
		Triangles.Empty();
		UV0.Empty();

		for (int lat = 0; lat <= SphereSegments; ++lat)
		{
			float theta = lat * PI / SphereSegments;
			float sinTheta = FMath::Sin(theta);
			float cosTheta = FMath::Cos(theta);

			for (int lon = 0; lon <= SphereSegments; ++lon)
			{
				float phi = lon * 2 * PI / SphereSegments;
				float sinPhi = FMath::Sin(phi);
				float cosPhi = FMath::Cos(phi);

				FVector vertex;
				vertex.X = Radius * sinTheta * cosPhi;
				vertex.Y = Radius * sinTheta * sinPhi;
				vertex.Z = Radius * cosTheta;

				Vertices.Add(vertex);
				UV0.Add(FVector2D((float)lon / SphereSegments, (float)lat / SphereSegments));
			}
		}

		for (int lat = 0; lat < SphereSegments; ++lat)
		{
			for (int lon = 0; lon < SphereSegments; ++lon)
			{
				int first = (lat * (SphereSegments + 1)) + lon;
				int second = first + SphereSegments + 1;

				if (bFlipSphereFace)
				{
					Triangles.Add(first);
					Triangles.Add(second);
					Triangles.Add(first + 1);

					Triangles.Add(second);
					Triangles.Add(second + 1);
					Triangles.Add(first + 1);
				}
				else
				{
					Triangles.Add(first);
					Triangles.Add(first + 1);
					Triangles.Add(second);

					Triangles.Add(second);
					Triangles.Add(first + 1);
					Triangles.Add(second + 1);
				}
			}
		}

	}
}
