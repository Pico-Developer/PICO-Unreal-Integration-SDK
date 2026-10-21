#include "ECS/PICOSpatialECSMaterialResourceAsset.h"

#include "Materials/MaterialInterface.h"
#include "PICOSpatial.h"
#include "UObject/Package.h"

bool UPICOSpatialECSMaterialResourceAsset::ResolveNativeMaterial(bool bForceRecreate)
{
#if PLATFORM_ANDROID
	GetOrCreateNativeMaterial(bForceRecreate);
	return bHasNativeMaterial && NativeMaterial.isValid();
#else
	return false;
#endif
}

bool UPICOSpatialECSMaterialResourceAsset::isNativeMaterialValid() const
{
#if PLATFORM_ANDROID
	return bHasNativeMaterial && NativeMaterial.isValid();
#else
	return false;
#endif
}

int64 UPICOSpatialECSMaterialResourceAsset::getNativeMaterialId() const
{
#if PLATFORM_ANDROID
	if (bHasNativeMaterial && NativeMaterial.isValid())
	{
		return static_cast<int64>(NativeMaterial.getResourceId());
	}
#endif
	return 0;
}

FString UPICOSpatialECSMaterialResourceAsset::getNativeMaterialName() const
{
#if PLATFORM_ANDROID
	if (bHasNativeMaterial && NativeMaterial.isValid())
	{
		if (const char* NamePtr = NativeMaterial.getName())
		{
			return FString(UTF8_TO_TCHAR(NamePtr));
		}
	}
#endif
	return FString();
}

bool UPICOSpatialECSMaterialResourceAsset::useUEMaterial(UMaterialInterface* InMaterial)
{
        SourceType = EPICOSpatialECSMaterialSourceType::FromUEMaterial;
        Material = InMaterial;
        BundleEntryPath = FString();
	return ResolveNativeMaterial(true);
}

bool UPICOSpatialECSMaterialResourceAsset::loadFromBundle(const FString& InBundleEntryPath)
{
        SourceType = EPICOSpatialECSMaterialSourceType::LoadFromBundle;
        Material = nullptr;
        BundleEntryPath = InBundleEntryPath;
	return ResolveNativeMaterial(true);
}

UMaterialInterface* UPICOSpatialECSMaterialResourceAsset::getUEMaterial() const
{
	return Material.Get();
}

FString UPICOSpatialECSMaterialResourceAsset::getUEMaterialAssetPath() const
{
	if (UMaterialInterface* Mat = Material.Get())
	{
		UPackage* Package = Mat->GetPackage();
		if (Package)
		{
			return Package->GetPathName();
		}
	}
	return FString();
}

FString UPICOSpatialECSMaterialResourceAsset::getBundleEntryPath() const
{
        return BundleEntryPath;
}

#if PLATFORM_ANDROID
const spatialpack::Material& UPICOSpatialECSMaterialResourceAsset::GetOrCreateNativeMaterial(bool bForceRecreate)
{
	if (!bHasNativeMaterial || bForceRecreate)
	{
		bHasNativeMaterial = BuildNativeMaterial();
	}
	return NativeMaterial;
}

bool UPICOSpatialECSMaterialResourceAsset::BuildNativeMaterial()
{
	NativeMaterial = spatialpack::Material();

	switch (SourceType)
	{
	case EPICOSpatialECSMaterialSourceType::FromUEMaterial:
	{
		if (UMaterialInterface* Mat = Material.Get())
		{
			FString MaterialPath;
			if (UPackage* Package = Mat->GetPackage())
			{
				MaterialPath = Package->GetPathName();
			}
			if (!MaterialPath.IsEmpty())
			{
				if (spatialpack::Material* Cached = FPICOSpatial::Get().CacheMaterialResource(MaterialPath, ""))
				{
					if (Cached->isValid())
					{
						NativeMaterial = *Cached;
					}
				}
			}
		}
		break;
	}
	case EPICOSpatialECSMaterialSourceType::LoadFromBundle:
	{
                if (!BundleEntryPath.IsEmpty())
		{
                        const FString CacheKey = FString::Printf(TEXT("ecs_mapped_material:%s"), *BundleEntryPath);
                        if (spatialpack::Material* Cached = FPICOSpatial::Get().CacheMappedMaterialResource(CacheKey, BundleEntryPath))
			{
                                if (Cached->isValid())
                                {
                                        NativeMaterial = *Cached;
                                }
			}
		}
		break;
	}
	default:
		break;
	}

	return NativeMaterial.isValid();
}
#endif
