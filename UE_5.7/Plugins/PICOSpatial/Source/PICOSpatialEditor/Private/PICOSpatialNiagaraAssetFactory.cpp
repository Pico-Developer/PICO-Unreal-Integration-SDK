#include "PICOSpatialNiagaraAssetFactory.h"

#include "AssetTypeCategories.h"
#include "PICOSpatialNiagaraAsset.h"

UPICOSpatialNiagaraAssetFactory::UPICOSpatialNiagaraAssetFactory()
{
	SupportedClass = UPICOSpatialNiagaraAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UPICOSpatialNiagaraAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UPICOSpatialNiagaraAsset>(InParent, Class, Name, Flags);
}

FText UPICOSpatialNiagaraAssetFactory::GetDisplayName() const
{
	return NSLOCTEXT("PICOSpatial", "PICOSpatialNiagaraAssetFactoryDisplayName", "PICO Spatial Niagara Asset");
}

uint32 UPICOSpatialNiagaraAssetFactory::GetMenuCategories() const
{
	return EAssetTypeCategories::Misc;
}

const TArray<FText>& UPICOSpatialNiagaraAssetFactory::GetMenuCategorySubMenus() const
{
	static const TArray<FText> SubMenus = { NSLOCTEXT("PICOSpatial", "PICOSpatialFactorySubMenu", "PICO Spatial") };
	return SubMenus;
}

