#include "PICOSpatialAssetMappingsAssetFactory.h"

#include "AssetTypeCategories.h"
#include "PICOSpatialAssetMappingsAsset.h"

UPICOSpatialAssetMappingsAssetFactory::UPICOSpatialAssetMappingsAssetFactory()
{
	SupportedClass = UPICOSpatialAssetMappingsAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPICOSpatialAssetMappingsAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UPICOSpatialAssetMappingsAsset>(InParent, Class, Name, Flags);
}

FText UPICOSpatialAssetMappingsAssetFactory::GetDisplayName() const
{
	return NSLOCTEXT("PICOSpatial", "PICOSpatialAssetMappingsFactoryDisplayName", "PICO Spatial Asset Mappings");
}

uint32 UPICOSpatialAssetMappingsAssetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}

const TArray<FText>& UPICOSpatialAssetMappingsAssetFactory::GetMenuCategorySubMenus() const
{
	static const TArray<FText> SubMenus = { NSLOCTEXT("PICOSpatial", "PICOSpatialAssetFactorySubMenu", "PICO Spatial") };
	return SubMenus;
}
