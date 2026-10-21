// Copyright 2023 PICO Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "EditorReimportHandler.h"
#include "PICOHapticAssetFactory.generated.h"

/**
 * Factory for importing PICO Haptic Format (.phf) files
 */
UCLASS(hidecategories=Object)
class UPICOHapticAssetFactory : public UFactory, public FReimportHandler
{
	GENERATED_UCLASS_BODY()

public:
	// UFactory Interface
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual bool DoesSupportClass(UClass* Class) override;
	virtual UClass* ResolveSupportedClass() override;
	virtual bool FactoryCanImport(const FString& Filename) override;

	// FReimportHandler Interface
	virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual EReimportResult::Type Reimport(UObject* Obj) override;
	virtual int32 GetPriority() const override;

private:
	bool ImportPHFFile(const FString& Filename, class UPICOHapticAsset* Asset, FFeedbackContext* Warn);
	
	TMap<class UPICOHapticAsset*, FString> AssetImportPaths;
};
