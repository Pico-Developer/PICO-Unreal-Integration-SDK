#include "ECS/PICOSpatialECSMeshResourceAsset.h"

#include "Engine/StaticMesh.h"
#include "PICOSpatial.h"
#include "UObject/Package.h"

UStaticMesh* UPICOSpatialECSMeshResourceAsset::getMesh() const
{
	return Mesh.Get();
}

FString UPICOSpatialECSMeshResourceAsset::getMeshAssetPath() const
{
	if (UStaticMesh* MeshPtr = Mesh.Get())
	{
		if (UPackage* Package = MeshPtr->GetPackage())
		{
			return Package->GetPathName();
		}
	}
	return FString();
}

bool UPICOSpatialECSMeshResourceAsset::ResolveNativeMeshResource(bool bForceRecreate)
{
#if PLATFORM_ANDROID
	GetOrCreateNativeMeshResource(bForceRecreate);
	return bHasNativeMeshResource && NativeMeshResource.isValid();
#else
	return false;
#endif
}

bool UPICOSpatialECSMeshResourceAsset::isNativeMeshResourceValid() const
{
#if PLATFORM_ANDROID
	return bHasNativeMeshResource && NativeMeshResource.isValid();
#else
	return false;
#endif
}

int64 UPICOSpatialECSMeshResourceAsset::getNativeMeshResourceId() const
{
#if PLATFORM_ANDROID
	if (bHasNativeMeshResource && NativeMeshResource.isValid())
	{
		return static_cast<int64>(NativeMeshResource.getResourceId());
	}
#endif
	return 0;
}

FString UPICOSpatialECSMeshResourceAsset::getNativeMeshResourceName() const
{
#if PLATFORM_ANDROID
	if (bHasNativeMeshResource && NativeMeshResource.isValid())
	{
		if (const char* NamePtr = NativeMeshResource.getName())
		{
			return FString(UTF8_TO_TCHAR(NamePtr));
		}
	}
#endif
	return FString();
}

bool UPICOSpatialECSMeshResourceAsset::getNativeMeshBounds(FVector& OutCenterCm, FVector& OutHalfExtentCm)
{
#if PLATFORM_ANDROID
	const spatialpack::MeshResource& MeshRef = GetOrCreateNativeMeshResource(false);
	if (!MeshRef.isValid())
	{
		return false;
	}
	const spatialpack::BoundingBox Bounds = MeshRef.getBounds();
	OutCenterCm = ToFVector(Bounds.center, 100.0f);
	OutHalfExtentCm = ToFVector(Bounds.halfExtent, 100.0f);
	return true;
#else
	OutCenterCm = FVector::ZeroVector;
	OutHalfExtentCm = FVector::ZeroVector;
	return false;
#endif
}

bool UPICOSpatialECSMeshResourceAsset::useUEMesh(UStaticMesh* InMesh)
{
        SourceType = EPICOSpatialECSMeshSourceType::FromUEMesh;
        Mesh = InMesh;
        BundleEntryPath = FString();
	return ResolveNativeMeshResource(true);
}

bool UPICOSpatialECSMeshResourceAsset::loadFromBundle(const FString& InBundleEntryPath)
{
        SourceType = EPICOSpatialECSMeshSourceType::LoadFromBundle;
        Mesh = nullptr;
        BundleEntryPath = InBundleEntryPath;
	return ResolveNativeMeshResource(true);
}

FString UPICOSpatialECSMeshResourceAsset::getBundleEntryPath() const
{
        return BundleEntryPath;
}

#if PLATFORM_ANDROID
const spatialpack::MeshResource& UPICOSpatialECSMeshResourceAsset::GetOrCreateNativeMeshResource(bool bForceRecreate)
{
	if (!bHasNativeMeshResource || bForceRecreate)
	{
		bHasNativeMeshResource = BuildNativeMeshResource();
	}
	return NativeMeshResource;
}

bool UPICOSpatialECSMeshResourceAsset::BuildNativeMeshResource()
{
	NativeMeshResource = spatialpack::MeshResource();

	switch (SourceType)
	{
        case EPICOSpatialECSMeshSourceType::FromUEMesh:
	{
		if (UStaticMesh* MeshPtr = Mesh.Get())
		{
			if (UPackage* Package = MeshPtr->GetPackage())
			{
				const FString MeshPath = Package->GetPathName();
				if (spatialpack::MeshResource* Cached = FPICOSpatial::Get().CacheMeshResource(MeshPath))
				{
					if (Cached->isValid())
					{
						NativeMeshResource = *Cached;
					}
				}
			}
		}
		break;
	}
        case EPICOSpatialECSMeshSourceType::LoadFromBundle:
	{
                if (!BundleEntryPath.IsEmpty())
		{
                        const FString CacheKey = FString::Printf(TEXT("ecs_mapped_mesh:%s"), *BundleEntryPath);
                        if (spatialpack::MeshResource* Cached = FPICOSpatial::Get().CacheMappedMeshResource(CacheKey, BundleEntryPath))
                        {
                                if (Cached->isValid())
                                {
                                        NativeMeshResource = *Cached;
                                }
                        }
                }
		break;
	}
	default:
		break;
	}

	return NativeMeshResource.isValid();
}
#endif
