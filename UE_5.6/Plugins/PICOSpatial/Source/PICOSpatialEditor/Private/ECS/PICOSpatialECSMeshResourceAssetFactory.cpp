#include "ECS/PICOSpatialECSMeshResourceAssetFactory.h"

#include "ECS/PICOSpatialECSMeshResourceAsset.h"
#include "AssetTypeCategories.h"

UPICOSpatialECSMeshResourceAssetFactory::UPICOSpatialECSMeshResourceAssetFactory()
{
	SupportedClass = UPICOSpatialECSMeshResourceAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPICOSpatialECSMeshResourceAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UPICOSpatialECSMeshResourceAsset>(InParent, Class, Name, Flags);
}

FText UPICOSpatialECSMeshResourceAssetFactory::GetDisplayName() const
{
	return NSLOCTEXT("PICOSpatial", "PICOSpatialECSMeshResourceAssetFactoryDisplayName", "PICO Spatial ECS Mesh Resource Asset");
}

uint32 UPICOSpatialECSMeshResourceAssetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}

const TArray<FText>& UPICOSpatialECSMeshResourceAssetFactory::GetMenuCategorySubMenus() const
{
	static const TArray<FText> SubMenus = { NSLOCTEXT("PICOSpatial", "PICOSpatialECSFactorySubMenu", "PICO Spatial") };
	return SubMenus;
}
