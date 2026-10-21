// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialEditor.h"
#include "PICONiagaraConverter.h"
#include "PICOSpatialRuntimeSettings.h"
#include "ISettingsModule.h"
#include "PropertyEditorModule.h"
#include "Settings/ProjectPackagingSettings.h"
#include "ToolMenus.h"
#include "Exporters/GLTFExporter.h"
#include "Options/GLTFExportOptions.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameMapsSettings.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "LevelInstance/LevelInstanceEditorInstanceActor.h"
#include "GeneralProjectSettings.h"
#include "PICOSpatial/Public/PICOSpatialFunctionLibrary.h"
#include "AssetExportTask.h"
#include "FileHelpers.h"
#include "StaticMeshExporterUSDOptions.h"
#include "SkeletalMeshExporterUSDOptions.h"
#include "MaterialExporterUSDOptions.h"
#include "PICOSpatialUSDModule.h"
#include "NiagaraEmitter.h"
#include "NiagaraSystem.h"
#include "PICOSpatialNiagaraAsset.h"
#include "NiagaraMeshRendererProperties.h"
#include "NiagaraSpriteRendererProperties.h"
#include "UObject/SavePackage.h"
#include "NiagaraSystemEditorData.h"
#include "NiagaraScriptSource.h"
#include "NiagaraDataInterfaceCurve.h"
#include "NiagaraDataInterfaceColorCurve.h"
#include "NiagaraDataInterfaceVectorCurve.h"
#include "NiagaraNodeOutput.h"
#include "NiagaraScriptVariable.h"
#include "NiagaraGraph.h"
#include "Misc/MessageDialog.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialFunctionInterface.h"
#include "Engine/Blueprint.h"
#include "Blueprint/BlueprintSupport.h"
#include "UObject/TopLevelAssetPath.h"
#include "PICOSpatialWorldDependenciesAsset.h"
#include "TimerManager.h"
#include "Editor.h"
#include "AssetToolsModule.h"
#include "EditorAssetLibrary.h"
#include "IAssetTools.h"
#include "AssetTypeActions_Base.h"
#include "UObject/GarbageCollection.h"
#include "PICOSpatialAssetMappingsAsset.h"
#include "PICOSpatialNiagaraAsset.h"
#include "ECS/PICOSpatialECSMaterialResourceAsset.h"
#include "ECS/PICOSpatialECSMeshResourceAsset.h"

#if WITH_EDITOR
#include "PackageTools.h"
#endif // WITH_EDITOR

#define LOCTEXT_NAMESPACE "FPICOSpatialEditorModule"

DEFINE_LOG_CATEGORY(LogPICOSpatialEditor);

class FPICOSpatialSimpleAssetTypeActions final : public FAssetTypeActions_Base
{
public:
	FPICOSpatialSimpleAssetTypeActions(UClass* InClass, FText InName, FColor InColor, EAssetTypeCategories::Type InCategories)
		: SupportedClass(InClass)
		, Name(MoveTemp(InName))
		, Color(InColor)
		, Categories(InCategories)
	{
	}

	virtual FText GetName() const override
	{
		return Name;
	}

	virtual FColor GetTypeColor() const override
	{
		return Color;
	}

	virtual UClass* GetSupportedClass() const override
	{
		return SupportedClass;
	}

	virtual uint32 GetCategories() override
	{
		return Categories;
	}

private:
	UClass* SupportedClass = nullptr;
	FText Name;
	FColor Color = FColor::White;
	EAssetTypeCategories::Type Categories = EAssetTypeCategories::Misc;
};

static EAssetTypeCategories::Type GPICOSpatialAssetCategory = EAssetTypeCategories::Misc;
static TArray<TSharedPtr<IAssetTypeActions>> GPICOSpatialAssetTypeActions;
static bool GPICOSpatialCookGarbageCollectScheduled = false;

static const TCHAR* GPICOSpatialWorldDependenciesPackageName = TEXT("/Game/AssetsPICO/DA_WorldDependencies");
static const TCHAR* GPICOSpatialWorldDependenciesAssetReference = TEXT("/Script/PICOSpatial.PICOSpatialWorldDependenciesAsset'/Game/AssetsPICO/DA_WorldDependencies.DA_WorldDependencies'");

static UPICOSpatialWorldDependenciesAsset* LoadOrCreateWorldDependenciesAsset()
{
	// The asset lives under the project's generated PICO assets directory.
	// Virtual path: /Game/AssetsPICO/DA_WorldDependencies
	if (UPICOSpatialWorldDependenciesAsset* Existing = LoadObject<UPICOSpatialWorldDependenciesAsset>(nullptr, GPICOSpatialWorldDependenciesAssetReference))
	{
		return Existing;
	}

	const FString PackageName = GPICOSpatialWorldDependenciesPackageName;
	const FString AssetName = TEXT("DA_WorldDependencies");

	// Resolve the on-disk path from the virtual package name so the .uasset
	// is written to the project Content/AssetsPICO directory.
	FString PackageFilename;
	if (!FPackageName::TryConvertLongPackageNameToFilename(PackageName, PackageFilename, FPackageName::GetAssetPackageExtension()))
	{
		UE_LOG(LogPICOSpatialEditor, Error, TEXT("Failed to resolve package filename for: %s"), *PackageName);
		return nullptr;
	}

	UPackage* Package = CreatePackage(*PackageName);
	if (!Package)
	{
		return nullptr;
	}

	// Set the on-disk filename so SavePackages writes to the correct location
	Package->SetLoadedPath(FPackagePath::FromLocalPath(PackageFilename));

	UPICOSpatialWorldDependenciesAsset* Created = NewObject<UPICOSpatialWorldDependenciesAsset>(
		Package,
		UPICOSpatialWorldDependenciesAsset::StaticClass(),
		*AssetName,
		RF_Public | RF_Standalone);

	if (!Created)
	{
		return nullptr;
	}

	FAssetRegistryModule::AssetCreated(Created);
	Created->MarkPackageDirty();

	TArray<UPackage*> PackagesToSave;
	PackagesToSave.Add(Package);
	UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);

	return Created;
}

static FSoftObjectPath NormalizeWorldSoftObjectPath(const FSoftObjectPath& WorldPath)
{
	FString PathString = WorldPath.ToString();
	if (PathString.IsEmpty())
	{
		return FSoftObjectPath();
	}

	int32 OptionsIndex = INDEX_NONE;
	if (PathString.FindChar(TEXT('?'), OptionsIndex))
	{
		PathString.LeftInline(OptionsIndex);
	}

	FString PackageName = WorldPath.GetLongPackageName();
	if (PackageName.IsEmpty())
	{
		PackageName = FPackageName::ObjectPathToPackageName(PathString);
	}

	if (!FPackageName::IsValidLongPackageName(PackageName))
	{
		return FSoftObjectPath();
	}

	const FString AssetName = FPackageName::GetLongPackageAssetName(PackageName);
	if (AssetName.IsEmpty())
	{
		return FSoftObjectPath();
	}

	return FSoftObjectPath(FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName));
}

static void CollectSpatialContainerPackageNames(IAssetRegistry& AssetRegistry, TSet<FName>& OutPackageNames)
{
	const FTopLevelAssetPath SpatialContainerBaseClassPath(TEXT("/Script/PICOSpatial"), TEXT("PICOSpatialContainer"));
	TArray<FTopLevelAssetPath> BaseClassNames = { SpatialContainerBaseClassPath };
	TSet<FTopLevelAssetPath> DerivedClassNames;
	AssetRegistry.GetDerivedClassNames(BaseClassNames, TSet<FTopLevelAssetPath>(), DerivedClassNames);

	FARFilter BlueprintFilter;
	BlueprintFilter.ClassPaths.Add(UBlueprint::StaticClass()->GetClassPathName());
	BlueprintFilter.bRecursiveClasses = true;

	TArray<FAssetData> BlueprintAssets;
	AssetRegistry.GetAssets(BlueprintFilter, BlueprintAssets);

	for (const FAssetData& BlueprintAsset : BlueprintAssets)
	{
		const FString GeneratedClassPath = BlueprintAsset.GetTagValueRef<FString>(FBlueprintTags::GeneratedClassPath);
		if (GeneratedClassPath.IsEmpty())
		{
			continue;
		}

		const FTopLevelAssetPath GeneratedClassPathName(
			FPackageName::ExportTextPathToObjectPath(FStringView(GeneratedClassPath)));
		if (DerivedClassNames.Contains(GeneratedClassPathName))
		{
			OutPackageNames.Add(BlueprintAsset.PackageName);
		}
	}
}

static bool AddWorldToTargetList(
	const FSoftObjectPath& WorldPath,
	const TCHAR* SourceLabel,
	TArray<UWorld*>& TargetWorlds,
	TSet<FName>& TargetWorldPackageNames)
{
	const FSoftObjectPath NormalizedWorldPath = NormalizeWorldSoftObjectPath(WorldPath);
	if (!NormalizedWorldPath.IsValid())
	{
		UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Skipping invalid target world from %s: %s"), SourceLabel, *WorldPath.ToString());
		return false;
	}

	const FName WorldPackageName(*NormalizedWorldPath.GetLongPackageName());
	if (TargetWorldPackageNames.Contains(WorldPackageName))
	{
		return true;
	}

	UWorld* World = Cast<UWorld>(NormalizedWorldPath.TryLoad());
	if (!World)
	{
		UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Failed to load target world from %s: %s"), SourceLabel, *NormalizedWorldPath.ToString());
		return false;
	}

	TargetWorlds.Add(World);
	TargetWorldPackageNames.Add(WorldPackageName);
	UE_LOG(LogPICOSpatialEditor, Log, TEXT("Found target world from %s: %s"), SourceLabel, *World->GetPathName());
	return true;
}

static FSoftObjectPath GetGameDefaultMapWorldPath()
{
	const FString GameDefaultMap = UGameMapsSettings::GetGameDefaultMap();
	if (GameDefaultMap.IsEmpty())
	{
		return FSoftObjectPath();
	}

	return FSoftObjectPath(GameDefaultMap);
}

static void ReportAssetExportFailures(const TArray<FString>& FailedAssetExports)
{
	if (FailedAssetExports.IsEmpty())
	{
		return;
	}

	for (const FString& FailedAssetExport : FailedAssetExports)
	{
		UE_LOG(LogPICOSpatialEditor, Warning, TEXT("PICO Spatial asset export failed: %s"), *FailedAssetExport);
	}

	const int32 MaxDisplayedFailures = 20;
	FString Message = FString::Printf(
		TEXT("PICO Spatial cook completed, but %d dependent asset(s) failed to export.\n\nThese assets were not added to the generated world dependency data. The PICO system engine will not be able to render those converted assets until the export issues are fixed.\n\n"),
		FailedAssetExports.Num());

	const int32 DisplayedFailures = FMath::Min(MaxDisplayedFailures, FailedAssetExports.Num());
	for (int32 Index = 0; Index < DisplayedFailures; ++Index)
	{
		Message += FString::Printf(TEXT("- %s\n"), *FailedAssetExports[Index]);
	}

	if (FailedAssetExports.Num() > MaxDisplayedFailures)
	{
		Message += FString::Printf(TEXT("\n...and %d more. Check the Output Log for the full list."), FailedAssetExports.Num() - MaxDisplayedFailures);
	}

	FMessageDialog::Open(
		EAppMsgType::Ok,
		FText::FromString(Message),
		LOCTEXT("PICOSpatialAssetExportFailuresTitle", "PICO Spatial Asset Export Warnings"));
}

static bool ShouldExportPICOAssetsToApkAssets()
{
	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	return Settings && Settings->bLoadAssetsFromApkAssets;
}

static FString GetContentPICOAssetsRootPath()
{
	return FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()), TEXT("AssetsPICO"));
}

static FString GetPICOAssetsExportRootPath()
{
	// This helper is only for non-uasset exports. Niagara-derived .uasset content
	// must stay under Content/AssetsPICO and should not follow APK-assets mode.
	if (ShouldExportPICOAssetsToApkAssets())
	{
		return FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), TEXT("Build/Android/assets/AssetsPICO"));
	}

	return GetContentPICOAssetsRootPath();
}

static FString GetApkAssetsPICOAssetsRootPath()
{
	return FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), TEXT("Build/Android/assets/AssetsPICO"));
}

static FString GetApkAssetsRootPath()
{
	return FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), TEXT("Build/Android/assets"));
}

static FString GetProjectBundleFilePath()
{
	return FPaths::Combine(GetApkAssetsRootPath(), TEXT("pico_spatial.bundle"));
}

static FString GetLegacyProjectRootBundleFilePath()
{
	return FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), TEXT("pico_spatial.bundle"));
}

static FString GetIntermediateAndroidAssetsRootPath()
{
	return FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()), TEXT("Intermediate/Android/arm64/assets"));
}

static FString GetUserCustomAssetsExportRootPath()
{
	return FPaths::Combine(GetPICOAssetsExportRootPath(), TEXT("UserCustomAssets"));
}

static void EnsurePICOAssetsDirectoriesExist(const FString& InPICOAssetsPath, const FString& InUserCustomAssetsPath)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*InPICOAssetsPath))
	{
		PlatformFile.CreateDirectoryTree(*InPICOAssetsPath);
	}

	if (!PlatformFile.DirectoryExists(*InUserCustomAssetsPath))
	{
		PlatformFile.CreateDirectoryTree(*InUserCustomAssetsPath);
	}
}

static bool ValidateBundleToolPath(const UPICOSpatialRuntimeSettings* Settings, FString& OutBundleToolPath, FText* OutFailureReason = nullptr)
{
	OutBundleToolPath.Reset();

	if (!Settings || !Settings->bUsedBundleTool)
	{
		return false;
	}

	if (Settings->BundleTool.FilePath.TrimStartAndEnd().IsEmpty())
	{
		if (OutFailureReason)
		{
			*OutFailureReason = LOCTEXT("BundleToolPathEmpty", "The Bundle Tool path is empty.");
		}
		return false;
	}

	OutBundleToolPath = FPaths::ConvertRelativePathToFull(Settings->BundleTool.FilePath);

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.FileExists(*OutBundleToolPath))
	{
		if (OutFailureReason)
		{
			*OutFailureReason = FText::Format(
				LOCTEXT("BundleToolPathNotFound", "The configured Bundle Tool executable was not found:\n{0}"),
				FText::FromString(OutBundleToolPath));
		}
		return false;
	}

#if PLATFORM_WINDOWS
	const FString Extension = FPaths::GetExtension(OutBundleToolPath);
	if (!Extension.Equals(TEXT("exe"), ESearchCase::IgnoreCase))
	{
		if (OutFailureReason)
		{
			*OutFailureReason = FText::Format(
				LOCTEXT("BundleToolWindowsExtensionInvalid", "The configured Bundle Tool must be a .exe file on Windows:\n{0}"),
				FText::FromString(OutBundleToolPath));
		}
		return false;
	}
#elif PLATFORM_MAC
	if (PlatformFile.DirectoryExists(*OutBundleToolPath))
	{
		if (OutFailureReason)
		{
			*OutFailureReason = FText::Format(
				LOCTEXT("BundleToolMacDirectoryInvalid", "The configured Bundle Tool path must be a file, not a directory, on macOS:\n{0}"),
				FText::FromString(OutBundleToolPath));
		}
		return false;
	}
#endif

	return true;
}

static void ScheduleCookGarbageCollect(const TCHAR* Reason)
{
	if (GPICOSpatialCookGarbageCollectScheduled || !GEditor)
	{
		return;
	}

	GPICOSpatialCookGarbageCollectScheduled = true;
	const FString ReasonString = Reason ? Reason : TEXT("PICO Spatial cook");
	GEditor->GetTimerManager()->SetTimerForNextTick([ReasonString]()
		{
			GPICOSpatialCookGarbageCollectScheduled = false;
			UE_LOG(LogPICOSpatialEditor, Log,
				TEXT("Running deferred garbage collection after %s"), *ReasonString);
			CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
		});
}

static void DeleteDirectoryContents(const FString& InDirectoryPath)
{
	IFileManager& FileManager = IFileManager::Get();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	const FString FullDirectoryPath = FPaths::ConvertRelativePathToFull(InDirectoryPath);
	if (!PlatformFile.DirectoryExists(*FullDirectoryPath))
	{
		UE_LOG(LogPICOSpatialEditor, Verbose, TEXT("Directory not found, skip full cleanup: %s"), *FullDirectoryPath);
		return;
	}

	TArray<FString> FoundFiles;
	FileManager.FindFilesRecursive(FoundFiles, *FullDirectoryPath, TEXT("*.*"), true, false);

	int32 DeletedFileCount = 0;
	int32 FailedFileCount = 0;
	for (const FString& FilePath : FoundFiles)
	{
		if (FileManager.Delete(*FilePath, false, true))
		{
			DeletedFileCount++;
		}
		else
		{
			FailedFileCount++;
			UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Failed to delete file during full cleanup: %s"), *FilePath);
		}
	}

	TArray<FString> AllDirectories;
	FileManager.FindFilesRecursive(AllDirectories, *FullDirectoryPath, TEXT("*"), false, true);
	AllDirectories.Sort([](const FString& A, const FString& B)
		{
			return A.Len() > B.Len();
		});

	int32 DeletedDirCount = 0;
	for (const FString& Directory : AllDirectories)
	{
		if (FPaths::IsSamePath(Directory, FullDirectoryPath))
		{
			continue;
		}

		if (PlatformFile.DeleteDirectory(*Directory))
		{
			DeletedDirCount++;
		}
	}

	UE_LOG(LogPICOSpatialEditor, Log,
		TEXT("Full directory cleanup completed - FilesDeleted: %d, FileDeleteFailed: %d, DirsDeleted: %d, Root: %s"),
		DeletedFileCount, FailedFileCount, DeletedDirCount, *FullDirectoryPath);
}

void DeleteContentsExceptUserDirAndOptionallyUasset(const FString& PICOAssetsPath, const FString& UserCustomAssetsPath, bool bDeleteUasset)
{
	IFileManager& FileManager = IFileManager::Get();
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	// Normalize paths with trailing slash for directory comparison
	auto NormalizeDirectoryPath = [](const FString& Path) -> FString
		{
			FString Normalized = FPaths::ConvertRelativePathToFull(Path).Replace(TEXT("\\"), TEXT("/"));
			return Normalized.EndsWith(TEXT("/")) ? Normalized : Normalized + TEXT("/");
		};

	const FString FullPICO = NormalizeDirectoryPath(PICOAssetsPath);
	const FString FullUser = NormalizeDirectoryPath(UserCustomAssetsPath);
	const FString FullContentAssetsRoot = NormalizeDirectoryPath(GetContentPICOAssetsRootPath());
	const FString WorldDependenciesFilename = FPaths::ConvertRelativePathToFull(
		FPaths::Combine(FPaths::ProjectContentDir(), TEXT("AssetsPICO/DA_WorldDependencies.uasset")))
		.Replace(TEXT("\\"), TEXT("/"));

	auto TryDeleteContentUAsset = [&](const FString& FilePath, const FString& NormalizedFilePath) -> TOptional<bool>
		{
			if (!FPaths::GetExtension(FilePath).Equals(TEXT("uasset"), ESearchCase::IgnoreCase))
			{
				return TOptional<bool>();
			}

			if (!NormalizedFilePath.StartsWith(FullContentAssetsRoot))
			{
				return TOptional<bool>();
			}

			FString RelativePath = NormalizedFilePath.Mid(FullContentAssetsRoot.Len());
			FString AssetPath = TEXT("/Game/AssetsPICO/");
			AssetPath += FPaths::ChangeExtension(RelativePath, TEXT(""));
			AssetPath.ReplaceInline(TEXT("\\"), TEXT("/"));

			const bool bDeleted = UEditorAssetLibrary::DeleteAsset(AssetPath);
			if (bDeleted)
			{
				UE_LOG(LogPICOSpatialEditor, Verbose, TEXT("Deleted asset: %s"), *AssetPath);
			}
			else
			{
				UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Failed to delete asset: %s"), *AssetPath);
			}

			return bDeleted;
		};

	// Early exit if root directory doesn't exist
	if (!PlatformFile.DirectoryExists(*FullPICO))
	{
		UE_LOG(LogPICOSpatialEditor, Warning, TEXT("PICO assets directory not found: %s"), *FullPICO);
		return;
	}

	// Find all files recursively
	TArray<FString> FoundFiles;
	FileManager.FindFilesRecursive(FoundFiles, *FullPICO, TEXT("*.*"), true, false);

	int32 DeletedCount = 0;
	int32 SkippedCount = 0;
	int32 FailedCount = 0;

	for (const FString& FilePath : FoundFiles)
	{
		// Normalize file path (NO trailing slash for files)
		FString NormalizedFilePath = FPaths::ConvertRelativePathToFull(FilePath).Replace(TEXT("\\"), TEXT("/"));

		if (FPaths::IsSamePath(NormalizedFilePath, WorldDependenciesFilename))
		{
			SkippedCount++;
			UE_LOG(LogPICOSpatialEditor, Verbose, TEXT("Preserved world dependencies asset: %s"), *FilePath);
			continue;
		}

		// Get parent directory for comparison
		FString FileDirectory = FPaths::GetPath(NormalizedFilePath);
		if (!FileDirectory.EndsWith(TEXT("/")))
		{
			FileDirectory += TEXT("/");
		}

		// Skip files in protected user directory (check parent directory)
		if (FileDirectory.StartsWith(FullUser))
		{
			SkippedCount++;
			UE_LOG(LogPICOSpatialEditor, Verbose, TEXT("Preserved file in user directory: %s"), *FilePath);
			continue;
		}

		// Optionally preserve .uasset files
		if (!bDeleteUasset && FPaths::GetExtension(FilePath).Equals(TEXT("uasset"), ESearchCase::IgnoreCase))
		{
			SkippedCount++;
			UE_LOG(LogPICOSpatialEditor, Verbose, TEXT("Preserved .uasset file: %s"), *FilePath);
			continue;
		}

		// Attempt deletion
		if (const TOptional<bool> bDeletedAsset = TryDeleteContentUAsset(FilePath, NormalizedFilePath))
		{
			if (*bDeletedAsset)
			{
				DeletedCount++;
			}
			else
			{
				FailedCount++;
			}
		}
		else if (FileManager.Delete(*FilePath, false, true))
		{
			DeletedCount++;
			UE_LOG(LogPICOSpatialEditor, Verbose, TEXT("Deleted file: %s"), *FilePath);
		}
		else
		{
			FailedCount++;
			UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Failed to delete file: %s"), *FilePath);
		}
	}

	UE_LOG(LogPICOSpatialEditor, Log,
		TEXT("File cleanup completed - Deleted: %d, Skipped: %d, Failed: %d"),
		DeletedCount, SkippedCount, FailedCount);

	// Delete empty directories (bottom-up traversal)
	TArray<FString> AllDirectories;
	FileManager.FindFilesRecursive(AllDirectories, *FullPICO, TEXT("*"), false, true);

	// Sort directories by depth (deepest first) to ensure bottom-up deletion
	AllDirectories.Sort([](const FString& A, const FString& B)
		{
			// Count directory depth by counting path separators
			FString NormalizedA = A.Replace(TEXT("\\"), TEXT("/"));
			FString NormalizedB = B.Replace(TEXT("\\"), TEXT("/"));

			int32 DepthA = 0;
			int32 DepthB = 0;

			for (TCHAR Char : NormalizedA)
			{
				if (Char == TEXT('/'))
				{
					DepthA++;
				}
			}

			for (TCHAR Char : NormalizedB)
			{
				if (Char == TEXT('/'))
				{
					DepthB++;
				}
			}

			return DepthA > DepthB; // Descending order (deepest first)
		});

	int32 DeletedDirCount = 0;
	int32 SkippedDirCount = 0;

	for (const FString& Directory : AllDirectories)
	{
		FString NormalizedDir = NormalizeDirectoryPath(Directory);

		// Skip the root PICO directory itself
		if (NormalizedDir == FullPICO)
		{
			continue;
		}

		// Skip the protected user directory and its subdirectories
		if (NormalizedDir.StartsWith(FullUser))
		{
			SkippedDirCount++;
			UE_LOG(LogPICOSpatialEditor, Verbose, TEXT("Preserved user directory: %s"), *Directory);
			continue;
		}

		// Check if directory is empty
		TArray<FString> FilesInDir;
		TArray<FString> SubDirsInDir;
		FileManager.FindFiles(FilesInDir, *(Directory / TEXT("*")), true, false);
		FileManager.FindFiles(SubDirsInDir, *(Directory / TEXT("*")), false, true);

		if (FilesInDir.Num() == 0 && SubDirsInDir.Num() == 0)
		{
			// Directory is empty, delete it
			if (FileManager.DeleteDirectory(*Directory, false, false))
			{
				DeletedDirCount++;
				UE_LOG(LogPICOSpatialEditor, Verbose, TEXT("Deleted empty directory: %s"), *Directory);
			}
			else
			{
				UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Failed to delete empty directory: %s"), *Directory);
			}
		}
	}

	UE_LOG(LogPICOSpatialEditor, Log,
		TEXT("Directory cleanup completed - Deleted: %d, Skipped: %d"),
		DeletedDirCount, SkippedDirCount);
}

void FPICOSpatialEditorModule::StartupModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		if (GPICOSpatialAssetTypeActions.Num() == 0)
		{
			IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
			GPICOSpatialAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("PICOSpatial")), LOCTEXT("PICOSpatialAssetCategoryName", "PICO Spatial"));

			GPICOSpatialAssetTypeActions.Add(MakeShared<FPICOSpatialSimpleAssetTypeActions>(
				UPICOSpatialAssetMappingsAsset::StaticClass(),
				LOCTEXT("PICOSpatialAssetMappingsAssetTypeName", "PICO Spatial Asset Mappings"),
				FColor(70, 170, 255),
				GPICOSpatialAssetCategory));

			GPICOSpatialAssetTypeActions.Add(MakeShared<FPICOSpatialSimpleAssetTypeActions>(
				UPICOSpatialWorldDependenciesAsset::StaticClass(),
				LOCTEXT("PICOSpatialWorldDependenciesAssetTypeName", "PICO Spatial World Dependencies"),
				FColor(255, 170, 70),
				GPICOSpatialAssetCategory));

			GPICOSpatialAssetTypeActions.Add(MakeShared<FPICOSpatialSimpleAssetTypeActions>(
                        UPICOSpatialECSMaterialResourceAsset::StaticClass(),
                        LOCTEXT("PICOSpatialECSMaterialResourceAssetTypeName", "PICO Spatial ECS Material Resource Asset"),
				FColor(170, 255, 120),
				GPICOSpatialAssetCategory));

			GPICOSpatialAssetTypeActions.Add(MakeShared<FPICOSpatialSimpleAssetTypeActions>(
				UPICOSpatialECSMeshResourceAsset::StaticClass(),
				LOCTEXT("PICOSpatialECSMeshResourceAssetTypeName", "PICO Spatial ECS Mesh Resource Asset"),
				FColor(120, 255, 220),
				GPICOSpatialAssetCategory));

			GPICOSpatialAssetTypeActions.Add(MakeShared<FPICOSpatialSimpleAssetTypeActions>(
				UPICOSpatialNiagaraAsset::StaticClass(),
				LOCTEXT("PICOSpatialNiagaraAssetTypeName", "PICO Spatial Niagara Asset"),
				FColor(210, 150, 255),
				GPICOSpatialAssetCategory));

			for (const TSharedPtr<IAssetTypeActions>& Action : GPICOSpatialAssetTypeActions)
			{
				AssetTools.RegisterAssetTypeActions(Action.ToSharedRef());
			}
		}

		SettingsModule->RegisterSettings("Project", "Plugins", "PICO Spatial",
			LOCTEXT("RuntimeSettingsName", "PICO Spatial"),
			LOCTEXT("RuntimeSettingsDescription", "Project settings for PICOSpatial plugin"),
			GetMutableDefault<UPICOSpatialRuntimeSettings>()
		);

		PICOAssetsPath = GetPICOAssetsExportRootPath();
		UserCustomAssetsPath = GetUserCustomAssetsExportRootPath();
		BundleFilePath = GetProjectBundleFilePath();
		EnsurePICOAssetsDirectoriesExist(PICOAssetsPath, UserCustomAssetsPath);

		UProjectPackagingSettings* PackagingSettings = GetMutableDefault<UProjectPackagingSettings>();
		bool bAddedPath = false;
		for (const FDirectoryPath& DirectoryPath : PackagingSettings->DirectoriesToAlwaysStageAsUFS)
		{
			if (DirectoryPath.Path == "AssetsPICO")
			{
				bAddedPath = true;
				break;
			}
		}
		if (!bAddedPath)
		{
			FDirectoryPath AssetsPath;
			AssetsPath.Path = TEXT("AssetsPICO");
			PackagingSettings->DirectoriesToAlwaysStageAsUFS.Add(AssetsPath);
			PackagingSettings->TryUpdateDefaultConfigFile();
		}

		bool bAddedStageAlwaysCookPath = false;
		for (const FDirectoryPath& DirectoryPath : PackagingSettings->DirectoriesToAlwaysCook)
		{
			if (DirectoryPath.Path == "/Game/AssetsPICO")
			{
				bAddedStageAlwaysCookPath = true;
			}
		}
		if (!bAddedStageAlwaysCookPath)
		{
			FDirectoryPath AssetsPath;
			AssetsPath.Path = TEXT("/Game/AssetsPICO");
			PackagingSettings->DirectoriesToAlwaysCook.Add(AssetsPath);
		}
		if (!bAddedStageAlwaysCookPath)
		{
			PackagingSettings->TryUpdateDefaultConfigFile();
		}
	}

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(
		this, &FPICOSpatialEditorModule::RegisterMenuExtensions));
}

void FPICOSpatialEditorModule::ShutdownModule()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

	if (SettingsModule != nullptr)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "PICOSpatial");
	}

	if (GPICOSpatialAssetTypeActions.Num() > 0 && FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (const TSharedPtr<IAssetTypeActions>& Action : GPICOSpatialAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
		}
		GPICOSpatialAssetTypeActions.Reset();
	}

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FPICOSpatialEditorModule::ExecuteCookContent()
{
	// ============================================================================
	// Step 1: Initialize Settings and Validate Bundle Tool
	// ============================================================================

	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	if (!Settings)
	{
		UE_LOG(LogPICOSpatialEditor, Error, TEXT("Failed to load PICO Spatial Runtime Settings"));
		return;
	}

	bool bUseBundleTool = false;
	bool bExportUsdShaderGraphMaterial = false;

	PICOAssetsPath = GetPICOAssetsExportRootPath();
	UserCustomAssetsPath = GetUserCustomAssetsExportRootPath();
	EnsurePICOAssetsDirectoriesExist(PICOAssetsPath, UserCustomAssetsPath);

	const bool bUseOnlyManualMappings = Settings->bUseOnlyManualAssetMappings;

	if (Settings->bUsedBundleTool && !bUseOnlyManualMappings)
	{
		FString BundleToolPath;
		FText BundleToolValidationError;

		if (ValidateBundleToolPath(Settings, BundleToolPath, &BundleToolValidationError))
		{
			bUseBundleTool = true;
			bExportUsdShaderGraphMaterial = Settings->bExportUsdShaderGraphMaterial;
		}
		else
		{
			const FText DialogTitle = LOCTEXT("BundleToolConfigInvalidTitle", "Bundle Tool Path Invalid");
			const FText DialogMessage = FText::Format(
				LOCTEXT("BundleToolConfigInvalidMessage",
					"Bundle tool is enabled, but the configured path does not point to a usable executable.\n\n{0}\n\nIf you continue, the bundle tool will be ignored automatically for this cook. Choose Cancel to stop now and fix the path."),
				BundleToolValidationError);

			const EAppReturnType::Type UserChoice = FMessageDialog::Open(EAppMsgType::OkCancel, DialogMessage, DialogTitle);
			if (UserChoice == EAppReturnType::Cancel)
			{
				UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Cook content cancelled because the bundle tool path is invalid"));
				ResetPICOAssetsAndDependencies();
				DeletePicoBundleFile();
				ScheduleCookGarbageCollect(TEXT("bundle tool validation cancellation"));
				return;
			}

			UE_LOG(LogPICOSpatialEditor, Warning,
				TEXT("Bundle tool path is invalid. Continuing without bundle tool. Reason: %s"),
				*BundleToolValidationError.ToString());

			if (UPICOSpatialRuntimeSettings* MutableSettings = GetMutableDefault<UPICOSpatialRuntimeSettings>())
			{
				MutableSettings->bUsedBundleTool = false;
				if (const FProperty* BundleToolEnabledProperty = MutableSettings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UPICOSpatialRuntimeSettings, bUsedBundleTool)))
				{
					MutableSettings->UpdateSinglePropertyInConfigFile(
						BundleToolEnabledProperty,
						MutableSettings->GetDefaultConfigFilename());
					UE_LOG(LogPICOSpatialEditor, Warning,
						TEXT("Disabled PICO Spatial bundle tool in Project Settings because the configured tool path is invalid."));
				}
			}
		}
	}
	else if (bUseOnlyManualMappings)
	{
		UE_LOG(LogPICOSpatialEditor, Log,
			TEXT("Use only manual asset mappings is enabled. Mesh, material and texture native resource exports and automatic bundle generation will be skipped for this cook."));
	}

	// ============================================================================
	// Step 2: Clean Previous Build Artifacts
	// ============================================================================

	ResetPICOAssetsAndDependencies();
	DeletePicoBundleFile();

	// ============================================================================
	// Step 3: Scan Asset Registry and Find Target Worlds
	// ============================================================================

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().SearchAllAssets(true);

	TArray<UWorld*> TargetWorlds;
	TSet<FName> TargetWorldPackageNames;
	const bool bHasConfiguredCookTargetWorlds = Settings->CookTargetWorlds.Num() > 0;

	TSet<FName> SpatialContainerPackageNames;
	CollectSpatialContainerPackageNames(AssetRegistryModule.Get(), SpatialContainerPackageNames);
	if (SpatialContainerPackageNames.IsEmpty())
	{
		UE_LOG(LogPICOSpatialEditor, Warning,
			TEXT("No APICOSpatialContainer-derived classes were found in the asset registry. No worlds can be selected for PICO Spatial cook."));
	}

	// Collect all world assets under /Game
	FARFilter WorldFilter;
	WorldFilter.ClassPaths.Add(UWorld::StaticClass()->GetClassPathName());
	WorldFilter.bRecursivePaths = true;
	WorldFilter.PackagePaths.Add(TEXT("/Game"));

	TArray<FAssetData> AllWorldAssets;
	AssetRegistryModule.Get().GetAssets(WorldFilter, AllWorldAssets);

	auto HasSpatialContainerDependency = [&](FName WorldPackageName, const TCHAR* SourceLabel, bool bWarnIfMissing) -> bool
	{
		TSet<FName> SearchedPackages;
		const bool bHasSpatialContainer = this->DoesPackageDependencyGraphContainAnyPackages(
			AssetRegistryModule.Get(),
			WorldPackageName,
			SpatialContainerPackageNames,
			SearchedPackages);

		if (!bHasSpatialContainer && bWarnIfMissing)
		{
			UE_LOG(LogPICOSpatialEditor, Warning,
				TEXT("Skipping target world from %s because it does not reference any APICOSpatialContainer-derived spatial container: %s"),
				SourceLabel,
				*WorldPackageName.ToString());
		}

		return bHasSpatialContainer;
	};

	auto AddSpatialWorldCandidate = [&](const FSoftObjectPath& WorldPath, const TCHAR* SourceLabel, bool bWarnIfMissingContainer) -> bool
	{
		const FSoftObjectPath NormalizedWorldPath = NormalizeWorldSoftObjectPath(WorldPath);
		if (!NormalizedWorldPath.IsValid())
		{
			UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Skipping invalid target world from %s: %s"), SourceLabel, *WorldPath.ToString());
			return false;
		}

		const FName WorldPackageName(*NormalizedWorldPath.GetLongPackageName());
		if (!HasSpatialContainerDependency(WorldPackageName, SourceLabel, bWarnIfMissingContainer))
		{
			return false;
		}

		return AddWorldToTargetList(NormalizedWorldPath, SourceLabel, TargetWorlds, TargetWorldPackageNames);
	};

	if (bHasConfiguredCookTargetWorlds)
	{
		for (const FSoftObjectPath& ConfiguredWorldPath : Settings->CookTargetWorlds)
		{
			AddSpatialWorldCandidate(ConfiguredWorldPath, TEXT("PICO Spatial Cook Target Worlds"), true);
		}
	}
	else
	{
		for (const FAssetData& WorldAsset : AllWorldAssets)
		{
			AddSpatialWorldCandidate(FSoftObjectPath(WorldAsset.GetSoftObjectPath()), TEXT("APICOSpatialContainer dependency scan"), false);
		}
	}

	if (const FSoftObjectPath GameDefaultMapPath = GetGameDefaultMapWorldPath(); GameDefaultMapPath.IsValid())
	{
		AddSpatialWorldCandidate(GameDefaultMapPath, TEXT("Game Default Map"), true);
	}

	// ============================================================================
	// Step 4: Export Assets from Target Worlds
	// ============================================================================

	FScopedSlowTask WorldProgressTask(static_cast<float>(TargetWorlds.Num()),
		LOCTEXT("CookingWorlds", "Processing worlds..."));
	WorldProgressTask.MakeDialog(true);

	TSet<UObject*> ExportedAssets;
	TSet<UObject*> FailedAssets;
	TSet<UObject*> GlbMaterials;
	TSet<FName> ConvertedNiagaraSystems;
	TSet<FName> FailedNiagaraSystems;
	TMap<FName, FWorldDependenciesPICO> WorldDependenciesMap;
	TMap<const UMaterialInterface*, FString> ConvertedMaterialToUsdPath;
	TArray<FString> FailedAssetExports;
	bool bUserCancelled = false;

	for (UWorld* World : TargetWorlds)
	{
		if (!World)
		{
			continue;
		}

		WorldProgressTask.EnterProgressFrame(1.f, FText::Format(
			LOCTEXT("ProcessingWorld", "Processing world: {0}"),
			FText::FromString(World->GetMapName())));

		if (WorldProgressTask.ShouldCancel())
		{
			bUserCancelled = true;
			break;
		}

		// --- Collect World Dependencies ---
		TMap<UObject*, EAssetTypePICO> NonTextureAssets;
		TMultiMap<UObject*, UObject*> TexturesInMaterial;
		TMap<UObject*, UObject*> TexturesNotInMaterial;
		TSet<FName> SearchedAssets;

		GetAllDependencyAssets(World, NonTextureAssets, SearchedAssets,
			ConvertedNiagaraSystems, FailedNiagaraSystems, nullptr,
			EAssetTypePICO::Unknown, bExportUsdShaderGraphMaterial,
			TexturesInMaterial, TexturesNotInMaterial);

		// --- Calculate Progress ---
		int32 TotalAssetCount = NonTextureAssets.Num();
		if (bExportUsdShaderGraphMaterial)
		{
			TotalAssetCount += TexturesInMaterial.Num() + TexturesNotInMaterial.Num();
		}

		FScopedSlowTask AssetProgressTask(static_cast<float>(TotalAssetCount),
			LOCTEXT("ExportingAssets", "Exporting assets..."));
		AssetProgressTask.MakeDialog(true);

		FWorldDependenciesPICO WorldDependency;

		// --- Phase 1: Export Texture Assets ---
		// USD shader graphs need the final PNG/EXR path while authoring material files.
		TSet<UObject*> ExportedTexturesThisWorld;
		TMap<UObject*, FString> FailedTextureExportsThisWorld;
		if (bUseBundleTool && bExportUsdShaderGraphMaterial && !bUserCancelled)
		{
			if (!ExportTextureAssets(TexturesInMaterial, TexturesNotInMaterial,
				GlbMaterials, ExportedAssets, FailedAssets, ExportedTexturesThisWorld, FailedTextureExportsThisWorld, WorldDependency, AssetProgressTask,
				FailedAssetExports, bUserCancelled))
			{
				break;
			}
		}

		// --- Phase 2: Export Non-Texture Assets (Meshes, Materials) ---
		if (!ExportNonTextureAssets(NonTextureAssets, ExportedAssets, FailedAssets, GlbMaterials,
			ConvertedMaterialToUsdPath, WorldDependency, AssetProgressTask,
			bExportUsdShaderGraphMaterial, FailedAssetExports, bUserCancelled))
		{
			break;
		}

		// Texture export runs before material USD/GLB selection so USD can author final PNG/EXR paths.
		// Once GLB fallback is known, remove textures that are only embedded in GLB materials.
		if (bUseBundleTool && bExportUsdShaderGraphMaterial)
		{
			auto IsTextureNeededOutsideGlbMaterial = [&](UObject* Texture)
			{
				if (!Texture)
				{
					return false;
				}

				if (TexturesNotInMaterial.Contains(Texture))
				{
					return true;
				}

				TArray<UObject*> MaterialOuters;
				TexturesInMaterial.MultiFind(Texture, MaterialOuters);
				for (UObject* MaterialOuter : MaterialOuters)
				{
					if (MaterialOuter && !GlbMaterials.Contains(MaterialOuter))
					{
						return true;
					}
				}

				return false;
			};

			for (UObject* Texture : ExportedTexturesThisWorld)
			{
				if (IsTextureNeededOutsideGlbMaterial(Texture))
				{
					continue;
				}

				const FString TextureBaseOutputPath = PICOAssetsPath / Texture->GetPackage()->GetPathName();
				IFileManager::Get().Delete(*(TextureBaseOutputPath + TEXT(".PNG")), false, true);
				IFileManager::Get().Delete(*(TextureBaseOutputPath + TEXT(".EXR")), false, true);
				ExportedAssets.Remove(Texture);
				WorldDependency.Dependencies.Remove(Texture->GetPackage()->GetPathName());
				UE_LOG(LogPICOSpatialEditor, Verbose,
					TEXT("Removed texture export embedded only in GLB material: %s"),
					*Texture->GetPackage()->GetPathName());
			}

			for (const TPair<UObject*, FString>& FailedTextureExport : FailedTextureExportsThisWorld)
			{
				UObject* Texture = FailedTextureExport.Key;
				if (IsTextureNeededOutsideGlbMaterial(Texture))
				{
					FailedAssetExports.Add(FailedTextureExport.Value);
					continue;
				}

				if (Texture)
				{
					FailedAssets.Remove(Texture);
					UE_LOG(LogPICOSpatialEditor, Verbose,
						TEXT("Ignored failed texture export embedded only in GLB material: %s"),
						*Texture->GetPackage()->GetPathName());
				}
			}
		}

		// --- Record World Dependencies ---
		if (Settings->bPreCacheAssetsPerLevel && WorldDependency.Dependencies.Num() > 0)
		{
			WorldDependenciesMap.Add(FName(*World->GetPackage()->GetPathName()), WorldDependency);
		}
	}

	// ============================================================================
	// Step 5: Handle Cancellation
	// ============================================================================

	if (bUserCancelled)
	{
		UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Cook operation cancelled by user"));
		ResetPICOAssetsAndDependencies();
		ScheduleCookGarbageCollect(TEXT("cook cancellation"));
		return;
	}

	// ============================================================================
	// Step 6: Execute Bundle Tool (Optional)
	// ============================================================================

	if (bUseBundleTool)
	{
		bool bBundleToolCancelledByUser = false;
		if (!ExecuteBundleTool(bBundleToolCancelledByUser))
		{
			if (bBundleToolCancelledByUser)
			{
				UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Bundle tool execution cancelled by user"));
			}
			else
			{
				UE_LOG(LogPICOSpatialEditor, Error, TEXT("Bundle tool execution failed"));
				FMessageDialog::Open(
					EAppMsgType::Ok,
					LOCTEXT("BundleToolExecutionFailedMessage",
						"Bundle conversion failed. Please check your bundle tool first, for example whether the configured file is really the bundle tool and whether it is using an outdated version. You can also try disabling the bundle tool and run the cook again.\n\nThe bundle tool is expected to be stable. If it still fails, we recommend not using the bundle tool for now."),
					LOCTEXT("BundleToolExecutionFailedTitle", "Bundle Tool Failed"));
			}
			ResetPICOAssetsAndDependencies();

			// Defer bundle file deletion to next tick
			GEditor->GetTimerManager()->SetTimerForNextTick([this]()
				{
					DeletePicoBundleFile();
				});
			ScheduleCookGarbageCollect(
				bBundleToolCancelledByUser ? TEXT("bundle tool cancellation") : TEXT("bundle tool failure"));

			return;
		}
	}

	ReportAssetExportFailures(FailedAssetExports);

	// ============================================================================
	// Step 7: Save World Dependencies DataTable
	// ============================================================================

	if (UPICOSpatialWorldDependenciesAsset* DependenciesAsset = LoadOrCreateWorldDependenciesAsset())
	{
		DependenciesAsset->WorldDependencies = WorldDependenciesMap;
		DependenciesAsset->MarkPackageDirty();

		TArray<UPackage*> PackagesToSave;
		PackagesToSave.Add(DependenciesAsset->GetPackage());

		bool bSaved = UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);

		if (bSaved)
		{
			UE_LOG(LogPICOSpatialEditor, Log, TEXT("World dependencies saved successfully"));
		}
		else
		{
			UE_LOG(LogPICOSpatialEditor, Error, TEXT("Failed to save world dependencies"));
		}
	}
	else
	{
		UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Failed to load world dependencies asset"));
	}

	UE_LOG(LogPICOSpatialEditor, Log, TEXT("Cook content completed successfully"));
	ScheduleCookGarbageCollect(TEXT("successful cook"));
}

void FPICOSpatialEditorModule::RegisterMenuExtensions()
{
	const FString SpatialContainerBlueprintPath = TEXT("/Script/Engine.Blueprint'/PICOSpatial/PICOSpatial/BP_PICOSpatialContainer.BP_PICOSpatialContainer'");
	UObject* LoadedBlueprint = StaticLoadObject(UBlueprint::StaticClass(), nullptr, *SpatialContainerBlueprintPath);

	if (LoadedBlueprint)
	{
		UE_LOG(LogPICOSpatialEditor, Log, TEXT("Successfully preloaded BP_PICOSpatialContainer on editor startup"));
	}
	else
	{
		UE_LOG(LogPICOSpatialEditor, Verbose, TEXT("Skipped preloading legacy BP_PICOSpatialContainer path: %s"), *SpatialContainerBlueprintPath);
	}

	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	FToolMenuSection& ToolbarSection = ToolbarMenu->FindOrAddSection("PICOSpatialMenu");
	ToolbarSection.AddEntry(FToolMenuEntry::InitToolBarButton(
		TEXT("CookContent"),
		FExecuteAction::CreateLambda([this]()
			{
				UEditorLoadingAndSavingUtils::SaveDirtyPackages(true, true);

				TArray<FString> PathsToScan = { TEXT("/Game") };
				FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
				AssetRegistryModule.Get().ScanPathsSynchronous(PathsToScan, true);

				if (AssetRegistryModule.Get().IsLoadingAssets())
				{
					static FDelegateHandle OnFilesLoadedHandle;
					if (!OnFilesLoadedHandle.IsValid())
					{
						// If asset registry is currently loading, wait for it to complete
						OnFilesLoadedHandle = AssetRegistryModule.Get().OnFilesLoaded().AddLambda([this]()
							{
								// Use a timer to delay execution by one tick to ensure all dependencies are updated
								GEditor->GetTimerManager()->SetTimerForNextTick([this]()
									{
										this->ExecuteCookContent();
									});

								FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
								AssetRegistryModule.Get().OnFilesLoaded().Remove(OnFilesLoadedHandle);
								OnFilesLoadedHandle.Reset();
							});
					}
				}
				else
				{
					// Asset registry already loaded, delay execution by one tick
					GEditor->GetTimerManager()->SetTimerForNextTick([this]()
						{
							this->ExecuteCookContent();
						});
				}
			}),
		INVTEXT("CookPICOSpatialContent"),
		INVTEXT("Cook Content For PICO Spatial.")
	));
}

bool FPICOSpatialEditorModule::ExportNonTextureAssets(
	const TMap<UObject*, EAssetTypePICO>& Assets,
	TSet<UObject*>& ExportedAssets,
	TSet<UObject*>& FailedAssets,
	TSet<UObject*>& OutGlbMaterials,
	TMap<const UMaterialInterface*, FString>& OutConvertedMaterialToUsdPath,
	FWorldDependenciesPICO& OutWorldDependency,
	FScopedSlowTask& ProgressTask,
	bool bExportUsdShaderGraphMaterial,
	TArray<FString>& OutFailedAssetExports,
	bool& bOutCancelled)
{
	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	const bool bUseOnlyManualMappings = Settings && Settings->bUseOnlyManualAssetMappings;

	for (const TPair<UObject*, EAssetTypePICO>& AssetPair : Assets)
	{
		if (ProgressTask.ShouldCancel())
		{
			bOutCancelled = true;
			return false;
		}

		UObject* Asset = AssetPair.Key;
		if (!Asset)
		{
			continue;
		}

		ProgressTask.EnterProgressFrame(1.f, FText::Format(
			LOCTEXT("ExportingAsset", "Exporting: {0}"),
			FText::FromString(Asset->GetName())));

		// Skip already exported assets
		if (ExportedAssets.Contains(Asset))
		{
			if (Settings && Settings->bPreCacheAssetsPerLevel &&
				!Asset->GetClass()->IsChildOf(UWorld::StaticClass()))
			{
				OutWorldDependency.Dependencies.Add(Asset->GetPackage()->GetPathName(), AssetPair.Value);
			}
			continue;
		}

		if (FailedAssets.Contains(Asset))
		{
			UE_LOG(LogPICOSpatialEditor, Verbose,
				TEXT("Skipping asset that already failed export in this cook: %s"),
				*Asset->GetPackage()->GetPathName());
			continue;
		}

		if (bUseOnlyManualMappings &&
			(AssetPair.Value == EAssetTypePICO::Mesh || AssetPair.Value == EAssetTypePICO::Material))
		{
			ExportedAssets.Add(Asset);
			if (Settings->bPreCacheAssetsPerLevel)
			{
				OutWorldDependency.Dependencies.Add(Asset->GetPackage()->GetPathName(), AssetPair.Value);
			}
			UE_LOG(LogPICOSpatialEditor, Verbose,
				TEXT("Use only manual asset mappings is enabled. Skipping native resource export for %s"),
				*Asset->GetPackage()->GetPathName());
			continue;
		}

		// --- Setup Export Task ---
		const FString OutputPath = PICOAssetsPath /
			Asset->GetPackage()->GetPathName() + TEXT(".glb");

		UAssetExportTask* ExportTask = NewObject<UAssetExportTask>();
		ExportTask->Object = Asset;
		ExportTask->Filename = OutputPath;
		ExportTask->bSelected = false;
		ExportTask->bReplaceIdentical = true;
		ExportTask->bPrompt = false;
		ExportTask->bUseFileArchive = false;
		ExportTask->bWriteEmptyFiles = false;
		ExportTask->bAutomated = true;

		// --- Configure GLTF Export Options ---
		UGLTFExportOptions* GLTFOptions = NewObject<UGLTFExportOptions>();
		GLTFOptions->ResetToDefault();
		GLTFOptions->bExportLights = false;
		GLTFOptions->bExportCameras = false;
		GLTFOptions->bExportAnimationSequences = false;

		if (Asset->GetClass()->IsChildOf(UMaterialInterface::StaticClass()))
		{
			GLTFOptions->BakeMaterialInputs = EGLTFMaterialBakeMode::Simple;
		}
		else if (Asset->GetClass()->IsChildOf(UStaticMesh::StaticClass()) ||
			Asset->GetClass()->IsChildOf(USkeletalMesh::StaticClass()))
		{
			GLTFOptions->BakeMaterialInputs = EGLTFMaterialBakeMode::Disabled;
			GLTFOptions->TextureImageFormat = EGLTFTextureImageFormat::None;
		}

		ExportTask->Options = GLTFOptions;

		// --- Handle Material Conversion ---
		bool bExportAsGlb = true;
		if (const UMaterialInterface* Material = Cast<UMaterialInterface>(Asset))
		{
#if WITH_EDITOR
			if (bExportUsdShaderGraphMaterial)
			{
				if (ConvertMaterialToUSDRecursive(OutConvertedMaterialToUsdPath, Material))
				{
					bExportAsGlb = false;
				}
			}

#endif
		}

		// --- Execute Export ---
		bool bExportSucceeded = true;
		if (bExportAsGlb)
		{
			bExportSucceeded = UExporter::RunAssetExportTask(ExportTask);
		}

		if (!bExportSucceeded)
		{
			UE_LOG(LogPICOSpatialEditor, Warning,
				TEXT("Failed to export asset: Type=%s, Name=%s, Path=%s"),
				*Asset->GetClass()->GetName(),
				*Asset->GetName(),
				*Asset->GetPackage()->GetPathName());
			OutFailedAssetExports.Add(FString::Printf(
				TEXT("%s | Type=%s | Output=%s"),
				*Asset->GetPackage()->GetPathName(),
				*Asset->GetClass()->GetName(),
				*OutputPath));
			FailedAssets.Add(Asset);
		}
		else
		{
			ExportedAssets.Add(Asset);
			if (bExportAsGlb && Asset->IsA<UMaterialInterface>())
			{
				OutGlbMaterials.Add(Asset);
			}
		}

		// --- Cleanup ---
		if (ExportTask->Options)
		{
			ExportTask->Options->ConditionalBeginDestroy();
		}
		ExportTask->ConditionalBeginDestroy();

		// --- Record Dependency ---
		if (Settings && Settings->bPreCacheAssetsPerLevel &&
			bExportSucceeded &&
			!Asset->GetClass()->IsChildOf(UWorld::StaticClass()))
		{
			OutWorldDependency.Dependencies.Add(Asset->GetPackage()->GetPathName(), AssetPair.Value);
		}
	}

	return true;
}

bool FPICOSpatialEditorModule::ExportTextureAssets(
	const TMultiMap<UObject*, UObject*>& TexturesInMaterial,
	const TMap<UObject*, UObject*>& TexturesNotInMaterial,
	const TSet<UObject*>& GlbMaterials,
	TSet<UObject*>& ExportedAssets,
	TSet<UObject*>& FailedAssets,
	TSet<UObject*>& OutExportedTexturesThisCall,
	TMap<UObject*, FString>& OutFailedTextureExportsThisCall,
	FWorldDependenciesPICO& OutWorldDependency,
	FScopedSlowTask& ProgressTask,
	TArray<FString>& OutFailedAssetExports,
	bool& bOutCancelled)
{
	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();

	// --- Collect Textures to Export ---
	TMap<UObject*, UObject*> TexturesToExport;

	// Process material-referenced textures
	for (const TPair<UObject*, UObject*>& TexturePair : TexturesInMaterial)
	{
		if (ProgressTask.ShouldCancel())
		{
			bOutCancelled = true;
			return false;
		}

		UObject* Texture = TexturePair.Key;
		UObject* MaterialOuter = TexturePair.Value;

		if (!Texture || !MaterialOuter)
		{
			continue;
		}

		const bool bReferencedByNonMaterial = TexturesNotInMaterial.Contains(Texture);
		const bool bMaterialIsGlb = GlbMaterials.Contains(MaterialOuter);

		// Skip textures embedded in GLB materials (unless referenced elsewhere)
		if (!bReferencedByNonMaterial && bMaterialIsGlb)
		{
			UE_LOG(LogPICOSpatialEditor, Verbose,
				TEXT("Skipping texture (embedded in GLB): %s"),
				*Texture->GetPackage()->GetPathName());
			ProgressTask.EnterProgressFrame(1.f);
			continue;
		}

		TexturesToExport.Add(Texture, MaterialOuter);
	}

	// Process non-material-referenced textures (always export)
	for (const TPair<UObject*, UObject*>& TexturePair : TexturesNotInMaterial)
	{
		if (ProgressTask.ShouldCancel())
		{
			bOutCancelled = true;
			return false;
		}

		UObject* Texture = TexturePair.Key;
		if (Texture && !TexturesToExport.Contains(Texture))
		{
			TexturesToExport.Add(Texture, TexturePair.Value);
		}
	}

	// --- Export Textures ---
	for (const TPair<UObject*, UObject*>& TexturePair : TexturesToExport)
	{
		if (ProgressTask.ShouldCancel())
		{
			bOutCancelled = true;
			return false;
		}

		UObject* Texture = TexturePair.Key;

		if (!Texture || ExportedAssets.Contains(Texture) || FailedAssets.Contains(Texture))
		{
			if (Texture && ExportedAssets.Contains(Texture) &&
				Settings && Settings->bPreCacheAssetsPerLevel &&
				!Texture->GetClass()->IsChildOf(UWorld::StaticClass()))
			{
				OutWorldDependency.Dependencies.Add(Texture->GetPackage()->GetPathName(), EAssetTypePICO::Texture2D);
			}
			else if (Texture && FailedAssets.Contains(Texture))
			{
				UE_LOG(LogPICOSpatialEditor, Verbose,
					TEXT("Skipping texture that already failed export in this cook: %s"),
					*Texture->GetPackage()->GetPathName());
			}
			ProgressTask.EnterProgressFrame(1.f);
			continue;
		}

		ProgressTask.EnterProgressFrame(1.f, FText::Format(
			LOCTEXT("ExportingTexture", "Exporting texture: {0}"),
			FText::FromString(Texture->GetName())));

		const FString BaseOutputPath = PICOAssetsPath /
			Texture->GetPackage()->GetPathName();
		FString OutputPath = BaseOutputPath + TEXT(".PNG");

		UAssetExportTask* ExportTask = NewObject<UAssetExportTask>();
		ExportTask->Object = Texture;
		ExportTask->Filename = OutputPath;
		ExportTask->bSelected = false;
		ExportTask->bReplaceIdentical = true;
		ExportTask->bPrompt = false;
		ExportTask->bUseFileArchive = false;
		ExportTask->bWriteEmptyFiles = false;
		ExportTask->bAutomated = true;

		bool bExportSucceeded = UExporter::RunAssetExportTask(ExportTask);
		if (!bExportSucceeded)
		{
			UE_LOG(LogPICOSpatialEditor, Warning,
				TEXT("Failed to export texture as PNG, retrying as EXR: %s"),
				*Texture->GetPackage()->GetPathName());

			OutputPath = BaseOutputPath + TEXT(".EXR");
			ExportTask->Filename = OutputPath;
			bExportSucceeded = UExporter::RunAssetExportTask(ExportTask);
		}

		if (!bExportSucceeded)
		{
			UE_LOG(LogPICOSpatialEditor, Warning,
				TEXT("Failed to export texture as PNG and EXR: %s"),
				*Texture->GetPackage()->GetPathName());
			OutFailedTextureExportsThisCall.Add(Texture, FString::Printf(
				TEXT("%s | Type=%s | Output=%s"),
				*Texture->GetPackage()->GetPathName(),
				*Texture->GetClass()->GetName(),
				*BaseOutputPath));
			FailedAssets.Add(Texture);
		}
		else if (OutputPath.EndsWith(TEXT(".EXR")))
		{
			UE_LOG(LogPICOSpatialEditor, Log,
				TEXT("Texture export fell back to EXR: %s"),
				*Texture->GetPackage()->GetPathName());
		}

		if (bExportSucceeded)
		{
			ExportedAssets.Add(Texture);
			OutExportedTexturesThisCall.Add(Texture);
		}
		ExportTask->ConditionalBeginDestroy();

		// --- Record Dependency ---
		if (Settings && Settings->bPreCacheAssetsPerLevel &&
			bExportSucceeded &&
			!Texture->GetClass()->IsChildOf(UWorld::StaticClass()))
		{
			OutWorldDependency.Dependencies.Add(Texture->GetPackage()->GetPathName(), EAssetTypePICO::Texture2D);
		}
	}

	return true;
}

bool FPICOSpatialEditorModule::ConvertMaterialToUSDRecursive(TMap<const class UMaterialInterface*, FString>& ConvertedMaterial, const class UMaterialInterface* Material)
{
	if (!Material)
	{
		return false;
	}

	if (ConvertedMaterial.Contains(Material))
	{
		return true;
	}

	static IConsoleVariable* CVarUseUsdBinaryFormat = IConsoleManager::Get().FindConsoleVariable(TEXT("PICO.UseUsdBinaryFormat"));
	FString UsdExtension = TEXT(".usd"); // Default to binary format
	if (CVarUseUsdBinaryFormat)
	{
		UsdExtension = CVarUseUsdBinaryFormat->GetBool() ? TEXT(".usd") : TEXT(".usda");
	}

	FString UsdPath = PICOAssetsPath / Material->GetPackage()->GetPathName() + UsdExtension;

	if (const UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material))
	{
		const UMaterialInterface* RootMaterial = MaterialInstance->Parent;
		while (const UMaterialInstance* ParentInstance = Cast<UMaterialInstance>(RootMaterial))
		{
			RootMaterial = ParentInstance->Parent;
		}

		if (ConvertMaterialToUSDRecursive(ConvertedMaterial, RootMaterial))
		{
			FString* ParentUsdPathFound = ConvertedMaterial.Find(RootMaterial);
			if (ParentUsdPathFound)
			{
				if (FPICOSpatialUSDModule::CreateMaterialInstance(*ParentUsdPathFound, UsdPath, Material))
				{
					ConvertedMaterial.Add(Material, UsdPath);
					return true;
				}
			}
		}
	}
	else
	{
		if (FPICOSpatialUSDModule::ConvertMaterialToUsd(Material, UsdPath))
		{
			ConvertedMaterial.Add(Material, UsdPath);
			return true;
		}
		return false;
	}

	return false;
}

void FPICOSpatialEditorModule::GetAllDependencyAssets(UObject* Object, TMap<UObject*, EAssetTypePICO>& DependencyAssetsNoTexture,
	TSet<FName>& Searched, TSet<FName>& ConvertedNiagaraSystems, TSet<FName>& FailedNiagaraSystems,
	UObject* Outer, EAssetTypePICO OuterType, bool bExportUsdShaderGraphMaterial,
	TMultiMap<UObject*/*Texture*/, UObject*/*Outer*/>& InMaterialTextures, TMap<UObject*/*Texture*/, UObject*/*Outer*/>& NotInMaterialTextures)
{
	if (Object == nullptr)
	{
		return;
	}

	EAssetTypePICO CurrentType = EAssetTypePICO::Unknown;
	UNiagaraSystem* NiagaraSystem = nullptr;
	if (Cast<UStaticMesh>(Object))
	{
		CurrentType = EAssetTypePICO::Mesh;
	}
	else if (Cast<UMaterialInterface>(Object))
	{
		CurrentType = EAssetTypePICO::Material;
	}
	else if (Cast<USkeletalMesh>(Object))
	{
		CurrentType = EAssetTypePICO::Mesh;
	}
	else if (Cast<UTexture2D>(Object))
	{
		CurrentType = EAssetTypePICO::Texture2D;
	}
	else if ((NiagaraSystem = Cast<UNiagaraSystem>(Object)) != nullptr)
	{
	}

	const bool bIsMaterialFunction = Cast<UMaterialFunctionInterface>(Object) != nullptr;
	const bool bIsMaterialContext =
		CurrentType == EAssetTypePICO::Material || bIsMaterialFunction;

	TArray<FAssetIdentifier> Dependencies;
	const FName OuterPathName = FName(*Object->GetPackage()->GetPathName());
	if (Searched.Contains(OuterPathName))
	{
		return;
	}

	if (bExportUsdShaderGraphMaterial && CurrentType == EAssetTypePICO::Texture2D)
	{
		// If ExportTexture2D is true, we need search all texture2d every time to distinguish the texture2d that is referenced by material
	}
	else
	{
		Searched.Add(OuterPathName);
	}

	if (NiagaraSystem && !ConvertedNiagaraSystems.Contains(OuterPathName) &&
		!FailedNiagaraSystems.Contains(OuterPathName))
	{
		if (FPICONiagaraConverter::ConvertNiagaraSystem(NiagaraSystem))
		{
			ConvertedNiagaraSystems.Add(OuterPathName);
		}
		else
		{
			FailedNiagaraSystems.Add(OuterPathName);
			UE_LOG(LogPICOSpatialEditor, Warning,
				TEXT("Niagara conversion failed and will not be retried in this cook: %s"),
				*OuterPathName.ToString());
		}
	}

	if (CurrentType != EAssetTypePICO::Unknown)
	{
		if (CurrentType != EAssetTypePICO::Texture2D)
		{
			DependencyAssetsNoTexture.Add(Object, CurrentType);
		}
		else
		{
			if (bExportUsdShaderGraphMaterial)
			{
				if (OuterType == EAssetTypePICO::Material && Cast<UMaterialInterface>(Outer))
				{
					InMaterialTextures.Add(Object, Outer);
				}
				else if (OuterType != EAssetTypePICO::Material)
				{
					NotInMaterialTextures.Add(Object, Outer);
				}
				else
				{
					// Material functions are not exported as standalone GLB or USD assets.
				}
			}
		}
	}

	FAssetRegistryModule::GetRegistry().GetDependencies(OuterPathName, Dependencies, UE::AssetRegistry::EDependencyCategory::All);
	for (const FAssetIdentifier& Identifier : Dependencies)
	{
		TArray<FAssetData> Assets;
		FAssetRegistryModule::GetRegistry().GetAssetsByPackageName(Identifier.PackageName, Assets);

		for (const FAssetData& Asset : Assets)
		{
			if (UObject* Item = Asset.GetAsset())
			{
				// Skip UWorld to avoid circular references
				if (!Item->IsA(UWorld::StaticClass()))
				{
					UObject* NextOuter = Object;
					if (bIsMaterialFunction && OuterType == EAssetTypePICO::Material && Outer)
					{
						NextOuter = Outer;
					}

					const EAssetTypePICO NextOuterType = bIsMaterialContext
						? EAssetTypePICO::Material
						: CurrentType;
					GetAllDependencyAssets(Item, DependencyAssetsNoTexture, Searched,
						ConvertedNiagaraSystems, FailedNiagaraSystems, NextOuter, NextOuterType,
						bExportUsdShaderGraphMaterial, InMaterialTextures, NotInMaterialTextures);
				}
			}
		}
	}
}

bool FPICOSpatialEditorModule::DoesPackageDependencyGraphContainAnyPackages(
	IAssetRegistry& AssetRegistry,
	FName PackageName,
	const TSet<FName>& TargetPackageNames,
	TSet<FName>& SearchedPackages)
{
	if (TargetPackageNames.Contains(PackageName))
	{
		return true;
	}

	// Prevent circular dependencies
	if (SearchedPackages.Contains(PackageName))
	{
		return false;
	}

	SearchedPackages.Add(PackageName);

	// Get direct dependencies
	TArray<FAssetIdentifier> Dependencies;
	AssetRegistry.GetDependencies(PackageName, Dependencies, UE::AssetRegistry::EDependencyCategory::All);

	for (const FAssetIdentifier& Dependency : Dependencies)
	{
		if (Dependency.PackageName == NAME_None)
		{
			continue;
		}

		if (TargetPackageNames.Contains(Dependency.PackageName))
		{
			return true;
		}

		// Skip if already searched
		if (SearchedPackages.Contains(Dependency.PackageName))
		{
			continue;
		}

		// Recursively collect dependencies of this dependency
		if (DoesPackageDependencyGraphContainAnyPackages(AssetRegistry, Dependency.PackageName,
			TargetPackageNames, SearchedPackages))
		{
			return true;
		}
	}

	return false;
}

bool FPICOSpatialEditorModule::ExecuteBundleTool(bool& bOutUserCancelled)
{
	bOutUserCancelled = false;

	// ============================================================================
	// Step 1: Validate Settings and Tool Path
	// ============================================================================

	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	if (!Settings)
	{
		UE_LOG(LogPICOSpatialEditor, Error, TEXT("Failed to load PICO Spatial Runtime Settings"));
		return false;
	}

	FString BundleToolPath;
	FText BundleToolValidationError;
	if (!ValidateBundleToolPath(Settings, BundleToolPath, &BundleToolValidationError))
	{
		UE_LOG(LogPICOSpatialEditor, Error, TEXT("Bundle tool validation failed: %s"), *BundleToolValidationError.ToString());
		return false;
	}

	// ============================================================================
	// Step 2: Platform-Specific Preparation
	// ============================================================================

#if PLATFORM_WINDOWS
	// Verify .exe extension on Windows
	FString Extension = FPaths::GetExtension(BundleToolPath);
	if (!Extension.Equals(TEXT("exe"), ESearchCase::IgnoreCase))
	{
		UE_LOG(LogPICOSpatialEditor, Warning,
			TEXT("Bundle tool on Windows should have .exe extension, found: .%s"), *Extension);
	}
#elif PLATFORM_MAC || PLATFORM_LINUX
	// Ensure executable permission on Unix-like systems
	FString ChmodParams = FString::Printf(TEXT("+x \"%s\""), *BundleToolPath);
	int32 ChmodReturnCode = 0;
	FString ChmodOutput, ChmodError;

	if (FPlatformProcess::ExecProcess(TEXT("/bin/chmod"), *ChmodParams, &ChmodReturnCode, &ChmodOutput, &ChmodError))
	{
		if (ChmodReturnCode == 0)
		{
			UE_LOG(LogPICOSpatialEditor, Verbose, TEXT("Set executable permission for: %s"), *BundleToolPath);
		}
		else
		{
			UE_LOG(LogPICOSpatialEditor, Warning,
				TEXT("chmod failed with code %d: %s"), ChmodReturnCode, *ChmodError);
		}
	}
	else
	{
		UE_LOG(LogPICOSpatialEditor, Error, TEXT("Failed to execute chmod: %s"), *ChmodError);
		return false;
	}
#endif

	// ============================================================================
	// Step 3: Prepare Command Line Arguments
	// ============================================================================

	FString SourcePath = PICOAssetsPath;
	FString OutputPath = BundleFilePath.IsEmpty() ? GetProjectBundleFilePath() : BundleFilePath;
	FString WorkingDirectory = FPaths::GetPath(BundleToolPath);
	const FString OutputDirectory = FPaths::GetPath(OutputPath);

	// Validate directories exist
	if (!FPaths::DirectoryExists(SourcePath))
	{
		UE_LOG(LogPICOSpatialEditor, Error, TEXT("Source directory does not exist: %s"), *SourcePath);
		return false;
	}

	if (!FPaths::DirectoryExists(OutputDirectory))
	{
		IFileManager::Get().MakeDirectory(*OutputDirectory, true);
	}

	if (!FPaths::DirectoryExists(WorkingDirectory))
	{
		UE_LOG(LogPICOSpatialEditor, Error, TEXT("Working directory does not exist: %s"), *WorkingDirectory);
		return false;
	}

	// Platform-aware path quoting
	auto QuotePath = [](const FString& Path) -> FString
		{
#if PLATFORM_WINDOWS
			// Windows: Quote if contains space or special characters
			if (Path.Contains(TEXT(" ")) || Path.Contains(TEXT("&")) || Path.Contains(TEXT("%")))
			{
				return FString::Printf(TEXT("\"%s\""), *Path);
			}
#else
			// Unix: Quote if contains space
			if (Path.Contains(TEXT(" ")))
			{
				return FString::Printf(TEXT("\"%s\""), *Path);
			}
#endif
			return Path;
		};

	FString QuotedSourcePath = QuotePath(SourcePath);
	FString QuotedOutputPath = QuotePath(OutputPath);
	FString UserCustomAssetsAbsPath = FPaths::Combine(SourcePath, TEXT("UserCustomAssets"));

	FString Params;
	if (FPaths::DirectoryExists(UserCustomAssetsAbsPath))
	{
		FString QuotedUserCustomAssetsPath = QuotePath(UserCustomAssetsAbsPath);
		Params = FString::Printf(
			TEXT("pack -s %s --exclude-dirs %s -o %s --formats glb usd usda --recursive --override-output --texture-dir %s --compress-textures astc4x4"),
			*QuotedSourcePath, *QuotedUserCustomAssetsPath, *QuotedOutputPath, *QuotedSourcePath);
	}
	else
	{
		Params = FString::Printf(
			TEXT("pack -s %s -o %s --formats glb usd usda --recursive --override-output --texture-dir %s --compress-textures astc4x4"),
			*QuotedSourcePath, *QuotedOutputPath, *QuotedSourcePath);
	}

	UE_LOG(LogPICOSpatialEditor, Log, TEXT("Executing bundle tool: %s"), *BundleToolPath);
	UE_LOG(LogPICOSpatialEditor, Log, TEXT("Parameters: %s"), *Params);
	UE_LOG(LogPICOSpatialEditor, Log, TEXT("Working directory: %s"), *WorkingDirectory);

	// ============================================================================
	// Step 4: Execute Bundle Tool Process
	// ============================================================================

	void* ReadPipe = nullptr;
	void* WritePipe = nullptr;

	if (!FPlatformProcess::CreatePipe(ReadPipe, WritePipe) || !ReadPipe || !WritePipe)
	{
		UE_LOG(LogPICOSpatialEditor, Error, TEXT("Failed to create pipe for bundle tool"));
		return false;
	}

	FProcHandle ProcHandle = FPlatformProcess::CreateProc(
		*BundleToolPath,
		*Params,
		false,      // bLaunchDetached
		true,       // bLaunchHidden
		true,       // bLaunchReallyHidden
		nullptr,    // OutProcessID
		0,          // PriorityModifier
		*WorkingDirectory,
		WritePipe,
		ReadPipe
	);

	bool bSuccess = false;

	if (!ProcHandle.IsValid())
	{
		UE_LOG(LogPICOSpatialEditor, Error, TEXT("Failed to start bundle tool process"));
		FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
		return false;
	}

	// ============================================================================
	// Step 5: Monitor Process Execution
	// ============================================================================

	UE_LOG(LogPICOSpatialEditor, Log, TEXT("Bundle tool started, monitoring execution..."));

	const double MaxWaitTime = 600.0; // 10 minutes
	FScopedSlowTask BundleToolTask(MaxWaitTime, LOCTEXT("BundleToolTask", "Running bundle tool..."));
	BundleToolTask.MakeDialog(true);

	const double StartTime = FPlatformTime::Seconds();
	double LastProgressUpdate = StartTime;
	bool bTimedOut = false;
	bool bUserCancelled = false;

	FString AllOutput;
	FString PendingOutput;
	FString LastLogLine;
	FString LastEmittedLogLine;

	auto CleanBundleToolLine = [](const FString& RawLine) -> FString
		{
			auto IsAnsiTerminator = [](TCHAR Ch) -> bool
				{
					return (Ch >= TEXT('A') && Ch <= TEXT('Z')) || (Ch >= TEXT('a') && Ch <= TEXT('z'));
				};

			auto IsAnsiParameter = [](TCHAR Ch) -> bool
				{
					return FChar::IsDigit(Ch) || Ch == TEXT(';') || Ch == TEXT('?');
				};

			FString Cleaned;
			Cleaned.Reserve(RawLine.Len());

			for (int32 Index = 0; Index < RawLine.Len();)
			{
				const TCHAR Ch = RawLine[Index];
				const bool bActualAnsiSequence = (Ch == 0x1b) && (Index + 1 < RawLine.Len()) && RawLine[Index + 1] == TEXT('[');
				const bool bLiteralAnsiSequence = (Ch == TEXT('e')) && (Index + 1 < RawLine.Len()) && RawLine[Index + 1] == TEXT('[');
				if (bActualAnsiSequence || bLiteralAnsiSequence)
				{
					int32 SequenceIndex = Index + 2;
					while (SequenceIndex < RawLine.Len() && IsAnsiParameter(RawLine[SequenceIndex]))
					{
						++SequenceIndex;
					}

					if (SequenceIndex < RawLine.Len() && IsAnsiTerminator(RawLine[SequenceIndex]))
					{
						Index = SequenceIndex + 1;
						continue;
					}
				}

				// Bundle tool emits braille spinner glyphs that are useless in UE logs.
				if (Ch >= 0x2800 && Ch <= 0x28FF)
				{
					++Index;
					continue;
				}

				if (Ch >= 0x20)
				{
					Cleaned.AppendChar(Ch);
				}

				++Index;
			}

			Cleaned.TrimStartAndEndInline();
			while (Cleaned.ReplaceInline(TEXT("  "), TEXT(" ")) > 0)
			{
			}

			return Cleaned;
		};

	auto LogBundleToolLine = [&](const FString& Line)
		{
			if (Line.IsEmpty() || Line == LastEmittedLogLine)
			{
				return;
			}

			LastLogLine = Line;
			LastEmittedLogLine = Line;

			if (Line.Contains(TEXT("error"), ESearchCase::IgnoreCase))
			{
				UE_LOG(LogPICOSpatialEditor, Error, TEXT("[BundleTool] %s"), *Line);
			}
			else if (Line.Contains(TEXT("warning"), ESearchCase::IgnoreCase))
			{
				UE_LOG(LogPICOSpatialEditor, Warning, TEXT("[BundleTool] %s"), *Line);
			}
			else
			{
				UE_LOG(LogPICOSpatialEditor, Log, TEXT("[BundleTool] %s"), *Line);
			}
		};

	auto ProcessBundleToolOutputChunk = [&](const FString& OutputChunk)
		{
			if (OutputChunk.IsEmpty())
			{
				return;
			}

			PendingOutput += OutputChunk;
			PendingOutput.ReplaceInline(TEXT("\r"), TEXT("\n"));

			TArray<FString> Lines;
			PendingOutput.ParseIntoArray(Lines, TEXT("\n"), false);

			const bool bEndsWithLineBreak = PendingOutput.EndsWith(TEXT("\n"));
			const int32 CompleteLineCount = bEndsWithLineBreak ? Lines.Num() : FMath::Max(Lines.Num() - 1, 0);
			for (int32 LineIndex = 0; LineIndex < CompleteLineCount; ++LineIndex)
			{
				const FString CleanedLine = CleanBundleToolLine(Lines[LineIndex]);
				if (!CleanedLine.IsEmpty())
				{
					LogBundleToolLine(CleanedLine);
				}
			}

			PendingOutput = bEndsWithLineBreak || Lines.Num() == 0 ? FString() : Lines.Last();
		};

	while (FPlatformProcess::IsProcRunning(ProcHandle))
	{
		// Read process output
		FString NewOutput = FPlatformProcess::ReadPipe(ReadPipe);
		if (!NewOutput.IsEmpty())
		{
			AllOutput += NewOutput;
			ProcessBundleToolOutputChunk(NewOutput);
		}

		// Update progress (once per second)
		double CurrentTime = FPlatformTime::Seconds();
		if (CurrentTime - LastProgressUpdate >= 1.0)
		{
			double Elapsed = CurrentTime - StartTime;

			FText ProgressMessage;
			if (!LastLogLine.IsEmpty())
			{
				FString DisplayLine = LastLogLine.Len() > 100
					? LastLogLine.Left(97) + TEXT("...")
					: LastLogLine;

				ProgressMessage = FText::Format(
					LOCTEXT("BundleToolProgressWithLog", "Running bundle tool... ({0}s / {1}s)\n{2}"),
					FText::AsNumber((int32)Elapsed),
					FText::AsNumber((int32)MaxWaitTime),
					FText::FromString(DisplayLine)
				);
			}
			else
			{
				ProgressMessage = FText::Format(
					LOCTEXT("BundleToolProgress", "Running bundle tool... ({0}s / {1}s)"),
					FText::AsNumber((int32)Elapsed),
					FText::AsNumber((int32)MaxWaitTime)
				);
			}

			BundleToolTask.EnterProgressFrame(1.0f, ProgressMessage);
			LastProgressUpdate = CurrentTime;
		}

		// Check for timeout
		if (CurrentTime - StartTime > MaxWaitTime)
		{
			UE_LOG(LogPICOSpatialEditor, Error,
				TEXT("Bundle tool execution timeout after %.0f seconds"), MaxWaitTime);
			FPlatformProcess::TerminateProc(ProcHandle);
			bTimedOut = true;
			break;
		}

		// Check for user cancellation
		if (BundleToolTask.ShouldCancel())
		{
			UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Bundle tool execution cancelled by user"));
			FPlatformProcess::TerminateProc(ProcHandle);
			bUserCancelled = true;
			bOutUserCancelled = true;
			break;
		}

		FPlatformProcess::Sleep(0.01f);
	}

	// ============================================================================
	// Step 6: Process Final Output and Get Exit Code
	// ============================================================================

	// Read remaining output
	FString FinalOutput = FPlatformProcess::ReadPipe(ReadPipe);
	if (!FinalOutput.IsEmpty())
	{
		AllOutput += FinalOutput;
		ProcessBundleToolOutputChunk(FinalOutput);
	}

	if (!PendingOutput.IsEmpty())
	{
		const FString CleanedPendingLine = CleanBundleToolLine(PendingOutput);
		if (!CleanedPendingLine.IsEmpty())
		{
			LogBundleToolLine(CleanedPendingLine);
		}
		PendingOutput.Reset();
	}

	// Get exit code
	int32 ReturnCode = 0;
	FPlatformProcess::GetProcReturnCode(ProcHandle, &ReturnCode);

	// Determine success
	if (bTimedOut || bUserCancelled)
	{
		bSuccess = false;
	}
	else
	{
		bSuccess = (ReturnCode == 0);
	}

	// ============================================================================
	// Step 7: Log Results and Cleanup
	// ============================================================================

	if (bSuccess)
	{
		UE_LOG(LogPICOSpatialEditor, Log, TEXT("Bundle tool exited successfully (exit code: 0)"));

		// Treat a missing or empty bundle as a failed conversion even if the tool exits with 0.
		if (FPaths::FileExists(OutputPath))
		{
			int64 FileSize = IFileManager::Get().FileSize(*OutputPath);
			if (FileSize > 0)
			{
				UE_LOG(LogPICOSpatialEditor, Log,
					TEXT("Generated bundle file (%.2f MB): %s"),
					FileSize / (1024.0 * 1024.0), *OutputPath);
			}
			else
			{
				UE_LOG(LogPICOSpatialEditor, Error,
					TEXT("Bundle tool succeeded but generated an empty bundle file: %s"), *OutputPath);
				bSuccess = false;
			}
		}
		else
		{
			UE_LOG(LogPICOSpatialEditor, Error,
				TEXT("Bundle tool succeeded but output file not found: %s"), *OutputPath);
			bSuccess = false;
		}
	}
	else if (!bTimedOut && !bUserCancelled)
	{
		UE_LOG(LogPICOSpatialEditor, Error,
			TEXT("Bundle tool failed with return code: %d"), ReturnCode);
		UE_LOG(LogPICOSpatialEditor, Error, TEXT("Bundle tool path: %s"), *BundleToolPath);
		UE_LOG(LogPICOSpatialEditor, Error, TEXT("Parameters: %s"), *Params);

		// Log last 10 lines of output for debugging
		if (!AllOutput.IsEmpty())
		{
			TArray<FString> AllLines;
			AllOutput.ParseIntoArrayLines(AllLines);
			int32 StartIndex = FMath::Max(0, AllLines.Num() - 10);

			if (AllLines.Num() > 0)
			{
				UE_LOG(LogPICOSpatialEditor, Error,
					TEXT("Last %d lines of output:"), AllLines.Num() - StartIndex);
				for (int32 i = StartIndex; i < AllLines.Num(); ++i)
				{
					FString TrimmedLine = AllLines[i].TrimStartAndEnd();
					if (!TrimmedLine.IsEmpty())
					{
						UE_LOG(LogPICOSpatialEditor, Error, TEXT("  %s"), *TrimmedLine);
					}
				}
			}
		}
	}

	// Clean up resources
	FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
	FPlatformProcess::CloseProc(ProcHandle);

	// ============================================================================
	// Step 8: Post-Processing
	// ============================================================================

	if (bSuccess && Settings->bClearAssetsAfterBundleConversion)
	{
		UE_LOG(LogPICOSpatialEditor, Log, TEXT("Cleaning temporary assets after bundle creation..."));
		DeleteContentsExceptUserDirAndOptionallyUasset(PICOAssetsPath, UserCustomAssetsPath, false);
	}

	return bSuccess;
}

void FPICOSpatialEditorModule::ResetPICOAssetsAndDependencies()
{
	// Delete generated file contents from the /Game-backed Content root, APK assets root,
	// and the Android staging assets directory. The staging directory is regenerated by packaging.
	DeleteContentsExceptUserDirAndOptionallyUasset(
		GetContentPICOAssetsRootPath(),
		FPaths::Combine(GetContentPICOAssetsRootPath(), TEXT("UserCustomAssets")),
		true);
	DeleteContentsExceptUserDirAndOptionallyUasset(
		GetApkAssetsPICOAssetsRootPath(),
		FPaths::Combine(GetApkAssetsPICOAssetsRootPath(), TEXT("UserCustomAssets")),
		true);
	DeleteDirectoryContents(GetIntermediateAndroidAssetsRootPath());

	UE_LOG(LogPICOSpatialEditor, Log,
		TEXT("Cleared Content AssetsPICO, APK AssetsPICO, and intermediate Android assets directories (preserved UserCustomAssets)"));

	if (UPICOSpatialWorldDependenciesAsset* DependenciesAsset = LoadOrCreateWorldDependenciesAsset())
	{
		DependenciesAsset->WorldDependencies.Reset();
		DependenciesAsset->MarkPackageDirty();

		TArray<UPackage*> PackagesToSave;
		PackagesToSave.Add(DependenciesAsset->GetPackage());
		UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
	}
	else
	{
		UE_LOG(LogPICOSpatialEditor, Warning,
			TEXT("Failed to load world dependencies asset - not cleared"));
	}
}

void FPICOSpatialEditorModule::DeletePicoBundleFile()
{
	TArray<FString> BundleFilesToDelete;
	BundleFilesToDelete.Add(BundleFilePath.IsEmpty() ? GetProjectBundleFilePath() : BundleFilePath);
	BundleFilesToDelete.Add(GetLegacyProjectRootBundleFilePath());

	for (const FString& BundleFileToDelete : BundleFilesToDelete)
	{
		if (!FPaths::FileExists(BundleFileToDelete))
		{
			continue;
		}

		IFileManager& FileManager = IFileManager::Get();
		if (FileManager.Delete(*BundleFileToDelete, false, true))
		{
			UE_LOG(LogPICOSpatialEditor, Log, TEXT("Deleted outdated bundle file: %s"), *BundleFileToDelete);
		}
		else
		{
			UE_LOG(LogPICOSpatialEditor, Warning, TEXT("Failed to delete bundle file: %s"), *BundleFileToDelete);
		}
	}
}

IMPLEMENT_MODULE(FPICOSpatialEditorModule, PICOSpatialEditor);

#undef LOCTEXT_NAMESPACE
