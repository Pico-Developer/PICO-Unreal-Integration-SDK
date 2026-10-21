// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICOHapticAssetFactory.h"
#include "PICOHapticAsset.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"

UPICOHapticAssetFactory::UPICOHapticAssetFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UPICOHapticAsset::StaticClass();
	Formats.Add(TEXT("phf;PICO Haptic Format"));
	
	bCreateNew = false;
	bEditorImport = true;
	bText = true;
}

UObject* UPICOHapticAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	UPICOHapticAsset* Asset = NewObject<UPICOHapticAsset>(InParent, InClass, InName, Flags);

	if (Asset && ImportPHFFile(Filename, Asset, Warn))
	{
		AssetImportPaths.Add(Asset, Filename);
		return Asset;
	}

	return nullptr;
}

bool UPICOHapticAssetFactory::DoesSupportClass(UClass* Class)
{
	return Class == UPICOHapticAsset::StaticClass();
}

UClass* UPICOHapticAssetFactory::ResolveSupportedClass()
{
	return UPICOHapticAsset::StaticClass();
}

bool UPICOHapticAssetFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);
	return Extension.Equals(TEXT("phf"), ESearchCase::IgnoreCase);
}

bool UPICOHapticAssetFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	UPICOHapticAsset* Asset = Cast<UPICOHapticAsset>(Obj);
	if (Asset)
	{
		FString* Path = AssetImportPaths.Find(Asset);
		if (Path && !Path->IsEmpty())
		{
			OutFilenames.Add(*Path);
			return true;
		}
	}
	return false;
}

void UPICOHapticAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UPICOHapticAsset* Asset = Cast<UPICOHapticAsset>(Obj);
	if (Asset && ensure(NewReimportPaths.Num() == 1))
	{
		AssetImportPaths.Add(Asset, NewReimportPaths[0]);
	}
}

EReimportResult::Type UPICOHapticAssetFactory::Reimport(UObject* Obj)
{
	UPICOHapticAsset* Asset = Cast<UPICOHapticAsset>(Obj);
	if (!Asset)
	{
		return EReimportResult::Failed;
	}

	FString* PathPtr = AssetImportPaths.Find(Asset);
	if (!PathPtr || PathPtr->IsEmpty() || !FPaths::FileExists(*PathPtr))
	{
		return EReimportResult::Failed;
	}

	bool OutCanceled = false;
	if (ImportPHFFile(*PathPtr, Asset, GWarn))
	{
		Asset->MarkPackageDirty();
		return EReimportResult::Succeeded;
	}

	return EReimportResult::Failed;
}

int32 UPICOHapticAssetFactory::GetPriority() const
{
	return ImportPriority;
}

bool UPICOHapticAssetFactory::ImportPHFFile(const FString& Filename, UPICOHapticAsset* Asset, FFeedbackContext* Warn)
{
	if (!Asset)
	{
		return false;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *Filename))
	{
		if (Warn)
		{
			Warn->Logf(ELogVerbosity::Error, TEXT("Failed to load PHF file: %s"), *Filename);
		}
		return false;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	
	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		if (Warn)
		{
			Warn->Logf(ELogVerbosity::Error, TEXT("Failed to parse JSON from PHF file: %s"), *Filename);
		}
		return false;
	}

	// Parse PHF version
	if (JsonObject->HasField(TEXT("phfVersion")))
	{
		Asset->PHFVersion = JsonObject->GetStringField(TEXT("phfVersion"));
	}

	// Parse frame duration
	if (JsonObject->HasField(TEXT("frameDuration")))
	{
		Asset->FrameDuration = FMath::RoundToInt(JsonObject->GetNumberField(TEXT("frameDuration")));
	}

	// Lambda to parse pattern data array
	auto ParsePatternData = [&](const FString& FieldName, TArray<FPICOHapticFrameData>& OutPatternData) -> bool
	{
		if (!JsonObject->HasField(FieldName))
		{
			return false;
		}

		const TArray<TSharedPtr<FJsonValue>>* PatternArray;
		if (!JsonObject->TryGetArrayField(FieldName, PatternArray))
		{
			return false;
		}

		OutPatternData.Empty(PatternArray->Num());

		for (const TSharedPtr<FJsonValue>& JsonValue : *PatternArray)
		{
			if (!JsonValue.IsValid() || JsonValue->Type != EJson::Object)
			{
				continue;
			}

			TSharedPtr<FJsonObject> FrameObject = JsonValue->AsObject();
			FPICOHapticFrameData FrameData;

			if (FrameObject->HasField(TEXT("frameseq")))
			{
				FrameData.FrameSeq = FMath::RoundToInt(FrameObject->GetNumberField(TEXT("frameseq")));
			}

			if (FrameObject->HasField(TEXT("play")))
			{
				FrameData.Play = FMath::RoundToInt(FrameObject->GetNumberField(TEXT("play")));
			}

			if (FrameObject->HasField(TEXT("frequency")))
			{
				FrameData.Frequency = static_cast<float>(FrameObject->GetNumberField(TEXT("frequency")));
			}

			if (FrameObject->HasField(TEXT("loop")))
			{
				FrameData.Loop = FMath::RoundToInt(FrameObject->GetNumberField(TEXT("loop")));
			}

			if (FrameObject->HasField(TEXT("gain")))
			{
				FrameData.Gain = static_cast<float>(FrameObject->GetNumberField(TEXT("gain")));
			}

			OutPatternData.Add(FrameData);
		}

		return true;
	};

	// Parse left hand pattern data
	if (!ParsePatternData(TEXT("patternData_L"), Asset->PatternData_L))
	{
		if (Warn)
		{
			Warn->Logf(ELogVerbosity::Warning, TEXT("No left hand pattern data found in PHF file: %s"), *Filename);
		}
	}

	// Parse right hand pattern data
	if (!ParsePatternData(TEXT("patternData_R"), Asset->PatternData_R))
	{
		if (Warn)
		{
			Warn->Logf(ELogVerbosity::Warning, TEXT("No right hand pattern data found in PHF file: %s"), *Filename);
		}
	}

	if (Warn)
	{
		Warn->Logf(ELogVerbosity::Log, TEXT("Successfully imported PHF file: %s (Version: %s, Duration: %dms, Left frames: %d, Right frames: %d)"),
			*Filename, *Asset->PHFVersion, Asset->FrameDuration, Asset->PatternData_L.Num(), Asset->PatternData_R.Num());
	}

	return true;
}
