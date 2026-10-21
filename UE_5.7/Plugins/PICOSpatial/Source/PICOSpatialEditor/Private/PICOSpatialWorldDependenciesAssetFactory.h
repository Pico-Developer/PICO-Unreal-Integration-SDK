#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "PICOSpatialWorldDependenciesAssetFactory.generated.h"

UCLASS()
class UPICOSpatialWorldDependenciesAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UPICOSpatialWorldDependenciesAssetFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetDisplayName() const override;
	virtual uint32 GetMenuCategories() const override;
	virtual const TArray<FText>& GetMenuCategorySubMenus() const override;
};

