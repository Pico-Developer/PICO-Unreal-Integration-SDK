#include "ECS/PICOSpatialECSMaterialResourceAssetFactory.h"

#include "AssetTypeCategories.h"
#include "ECS/PICOSpatialECSMaterialResourceAsset.h"

UPICOSpatialECSMaterialResourceAssetFactory::UPICOSpatialECSMaterialResourceAssetFactory()
{
        SupportedClass = UPICOSpatialECSMaterialResourceAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPICOSpatialECSMaterialResourceAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
        return NewObject<UPICOSpatialECSMaterialResourceAsset>(InParent, Class, Name, Flags);
}

FText UPICOSpatialECSMaterialResourceAssetFactory::GetDisplayName() const
{
        return NSLOCTEXT("PICOSpatial", "PICOSpatialECSMaterialResourceAssetFactoryDisplayName", "PICO Spatial ECS Material Resource Asset");
}

uint32 UPICOSpatialECSMaterialResourceAssetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}

const TArray<FText>& UPICOSpatialECSMaterialResourceAssetFactory::GetMenuCategorySubMenus() const
{
	static const TArray<FText> SubMenus = { NSLOCTEXT("PICOSpatial", "PICOSpatialECSFactorySubMenu", "PICO Spatial") };
	return SubMenus;
}
