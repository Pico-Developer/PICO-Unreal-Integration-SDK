#include "PICOSpatialWorldDependenciesAssetFactory.h"

#include "AssetTypeCategories.h"
#include "PICOSpatialWorldDependenciesAsset.h"

UPICOSpatialWorldDependenciesAssetFactory::UPICOSpatialWorldDependenciesAssetFactory()
{
	SupportedClass = UPICOSpatialWorldDependenciesAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPICOSpatialWorldDependenciesAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UPICOSpatialWorldDependenciesAsset>(InParent, Class, Name, Flags);
}

FText UPICOSpatialWorldDependenciesAssetFactory::GetDisplayName() const
{
	return NSLOCTEXT("PICOSpatial", "PICOSpatialWorldDependenciesAssetFactoryDisplayName", "PICO Spatial World Dependencies");
}

uint32 UPICOSpatialWorldDependenciesAssetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}

const TArray<FText>& UPICOSpatialWorldDependenciesAssetFactory::GetMenuCategorySubMenus() const
{
	static const TArray<FText> SubMenus = { NSLOCTEXT("PICOSpatial", "PICOSpatialFactorySubMenu", "PICO Spatial") };
	return SubMenus;
}

