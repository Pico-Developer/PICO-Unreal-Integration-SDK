// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatialGlbUsdViewer.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/App.h"
#include "Misc/Paths.h"
#include "PICOSpatial.h"
#include "PICOSpatialContainer.h"
#include "Components/ArrowComponent.h"

#if PLATFORM_ANDROID
namespace
{
template<typename Func>
static void TraverseAllEntityChildrenRecursive(spatialpack::SceneEntity& Root, Func&& Visitor)
{
	if (!Root.isValid())
	{
		return;
	}

	spatialpack::SceneEntity Child = Root.getFirstChild();
	while (Child.isValid())
	{
		if (!Visitor(Child))
		{
			return;
		}

		TraverseAllEntityChildrenRecursive(Child, Forward<Func>(Visitor));

		Child = Child.getNextSibling();
	}
}

static bool ViewerHasSceneAssetExtension(const FString& InPath)
{
	const FString Extension = FPaths::GetExtension(InPath, false).ToLower();
	return Extension == TEXT("usd")
		|| Extension == TEXT("usda")
		|| Extension == TEXT("usdz")
		|| Extension == TEXT("glb")
		|| Extension == TEXT("gltf");
}

static const TCHAR* const* ViewerGetSceneAssetExtensions(int32& OutNumExtensions)
{
	static const TCHAR* CandidateExtensions[] = {
		TEXT(".usd"),
		TEXT(".usda"),
		TEXT(".usdz"),
		TEXT(".glb"),
		TEXT(".gltf")
	};

	OutNumExtensions = UE_ARRAY_COUNT(CandidateExtensions);
	return CandidateExtensions;
}

template <typename LoaderFunc>
static bool ViewerTrySceneAssetCandidates(const FString& InBasePath, LoaderFunc&& TryCandidate)
{
	if (ViewerHasSceneAssetExtension(InBasePath))
	{
		return TryCandidate(InBasePath);
	}

	int32 NumExtensions = 0;
	const TCHAR* const* CandidateExtensions = ViewerGetSceneAssetExtensions(NumExtensions);
	for (int32 Index = 0; Index < NumExtensions; ++Index)
	{
		if (TryCandidate(InBasePath + CandidateExtensions[Index]))
		{
			return true;
		}
	}

	return false;
}

static FString BuildAssetUriBaseLikeLoadAsset(const FString& Name)
{
	FString NormalizedPath = Name;
	if (!NormalizedPath.StartsWith(TEXT("/")))
	{
		NormalizedPath = FString(TEXT("/")) + NormalizedPath;
	}

	if (NormalizedPath.StartsWith(TEXT("/AssetsPICO/")))
	{
		return NormalizedPath.RightChop(1);
	}

	return FString(TEXT("AssetsPICO")) + NormalizedPath;
}

static FString MakeFileUri(const FString& AbsolutePath)
{
	FString NormalizedPath = AbsolutePath;
	NormalizedPath.ReplaceInline(TEXT("\\"), TEXT("/"));
	if (NormalizedPath.StartsWith(TEXT("/")))
	{
		return FString(TEXT("file:///")) + NormalizedPath.Mid(1);
	}

	return FString(TEXT("file:///")) + NormalizedPath;
}

static FString FileUriToPath(const FString& InUri)
{
	FString Path = InUri;
	if (Path.RemoveFromStart(TEXT("file://"), ESearchCase::IgnoreCase))
	{
		if (Path.Len() >= 3 && Path[0] == TEXT('/') && Path[2] == TEXT(':'))
		{
			Path = Path.Mid(1);
		}
	}
	return Path;
}

static bool IsLikelyAndroidStoragePath(const FString& InPath)
{
	const FString TrimmedPath = InPath.TrimStartAndEnd();
	if (TrimmedPath.IsEmpty())
	{
		return false;
	}

	const FString LowerPath = TrimmedPath.ToLower();
	return LowerPath.StartsWith(TEXT("file://"))
		|| LowerPath.StartsWith(TEXT("/storage/"))
		|| LowerPath.StartsWith(TEXT("/sdcard/"))
		|| LowerPath.StartsWith(TEXT("/mnt/"))
		|| LowerPath.StartsWith(TEXT("/data/"));
}

static FString NormalizeToUserCustomRelativePath(const FString& InPath)
{
	FString NormalizedPath = InPath.TrimStartAndEnd();
	NormalizedPath.ReplaceInline(TEXT("\\"), TEXT("/"));
	if (NormalizedPath.IsEmpty())
	{
		return FString();
	}

	auto TryStripPrefix = [&NormalizedPath](const FString& Prefix) -> bool
	{
		if (NormalizedPath.StartsWith(Prefix, ESearchCase::IgnoreCase))
		{
			NormalizedPath = NormalizedPath.Mid(Prefix.Len());
			return true;
		}
		return false;
	};

	const FString ContentUserCustomRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("AssetsPICO/UserCustomAssets/")).Replace(TEXT("\\"), TEXT("/"));
	const FString ApkUserCustomRoot = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir() / TEXT("Build/Android/assets/AssetsPICO/UserCustomAssets/")).Replace(TEXT("\\"), TEXT("/"));

	TryStripPrefix(ContentUserCustomRoot);
	TryStripPrefix(ApkUserCustomRoot);
	TryStripPrefix(TEXT("/UserCustomAssets/"));
	TryStripPrefix(TEXT("UserCustomAssets/"));
	TryStripPrefix(TEXT("/AssetsPICO/UserCustomAssets/"));
	TryStripPrefix(TEXT("AssetsPICO/UserCustomAssets/"));

	while (NormalizedPath.StartsWith(TEXT("/")))
	{
		NormalizedPath.RightChopInline(1);
	}

	if (IsLikelyAndroidStoragePath(NormalizedPath))
	{
		return FString();
	}

	// Preserve old editor file-picker behavior for absolute host paths by mapping them to a packaged basename fallback.
	if (!FPaths::IsRelative(NormalizedPath))
	{
		return FPaths::GetCleanFilename(NormalizedPath);
	}

	return NormalizedPath;
}

static FString BuildUserCustomAssetName(const FString& InPath)
{
	const FString RelativePath = NormalizeToUserCustomRelativePath(InPath);
	if (RelativePath.IsEmpty())
	{
		return FString();
	}

	return FString(TEXT("/UserCustomAssets/")) + RelativePath;
}

static spatialpack::SceneEntity LoadSceneEntityFromExplicitStorage(spatialpack::Scene* Scene, const FString& InPath)
{
	spatialpack::SceneEntity LoadedEntity;
	if (Scene == nullptr)
	{
		return LoadedEntity;
	}

	const spatialpack::SceneEntityLoadFlag LoadFlag = spatialpack::SceneEntityLoadFlag::DISABLE_MESH_MERGE;
	ViewerTrySceneAssetCandidates(InPath.TrimStartAndEnd(), [&](const FString& CandidatePath) -> bool
	{
		auto TryLoad = [&](const FString& StoragePath) -> bool
		{
			spatialpack::SceneEntity Entity = spatialpack::SceneEntity::loadFromStorage(Scene, TCHAR_TO_UTF8(*StoragePath), LoadFlag);
			if (Entity.isValid())
			{
				LoadedEntity = Entity;
				return true;
			}
			return false;
		};

		if (CandidatePath.StartsWith(TEXT("file://"), ESearchCase::IgnoreCase))
		{
			if (TryLoad(CandidatePath))
			{
				return true;
			}

			return TryLoad(FileUriToPath(CandidatePath));
		}

		if (TryLoad(CandidatePath))
		{
			return true;
		}

		return TryLoad(MakeFileUri(CandidatePath));
	});

	return LoadedEntity;
}
} // namespace
#endif

// Sets default values for this component's properties
UPICOSpatialGlbUsdViewer::UPICOSpatialGlbUsdViewer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickInterval = 0.0f;

	// ...
}

void UPICOSpatialGlbUsdViewer::BeginPlay()
{
	Super::BeginPlay();
#if PLATFORM_ANDROID
	LoadModelEntity();
#endif // PLATFORM_ANDROID
}

void UPICOSpatialGlbUsdViewer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if PLATFORM_ANDROID
	ReleaseLoadedEntity();
#endif
	Super::EndPlay(EndPlayReason);
}

void UPICOSpatialGlbUsdViewer::OnVisibilityChanged()
{
	Super::OnVisibilityChanged();
#if PLATFORM_ANDROID
	SyncLoadedEntityVisibility();
#endif
}

void UPICOSpatialGlbUsdViewer::OnHiddenInGameChanged()
{
	Super::OnHiddenInGameChanged();
#if PLATFORM_ANDROID
	SyncLoadedEntityVisibility();
#endif
}

void UPICOSpatialGlbUsdViewer::SetModelPath(const FString& InModelPath)
{
	ModelPath.FilePath = InModelPath;
#if PLATFORM_ANDROID
	SetComponentTickEnabled(true);
	LoadModelEntity();
#endif
}

bool UPICOSpatialGlbUsdViewer::ReloadModel()
{
#if PLATFORM_ANDROID
	SetComponentTickEnabled(true);
	return LoadModelEntity();
#else
	return false;
#endif
}

void UPICOSpatialGlbUsdViewer::ClearModel()
{
#if PLATFORM_ANDROID
	ReleaseLoadedEntity();
#endif
}

FString UPICOSpatialGlbUsdViewer::GetModelPath() const
{
	return ModelPath.FilePath;
}

bool UPICOSpatialGlbUsdViewer::LoadModelEntity()
{
#if PLATFORM_ANDROID
	ReleaseLoadedEntity();

	const FString ConfiguredModelPath = GetModelPath().TrimStartAndEnd();
	if (ConfiguredModelPath.IsEmpty())
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[GlbUsdViewer] Load skipped because ModelPath is empty. Obj=%s"), *GetName());
		return false;
	}

	FPICOSpatial& PICOSpatial = FPICOSpatial::Get();
	if (PICOSpatial.CurrentScene == nullptr)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[GlbUsdViewer] Load skipped because CurrentScene is null. Obj=%s Path='%s'"),
			*GetName(),
			*ConfiguredModelPath);
		return false;
	}

	if (IsLikelyAndroidStoragePath(ConfiguredModelPath))
	{
		LoadedEntity = LoadSceneEntityFromExplicitStorage(PICOSpatial.CurrentScene, ConfiguredModelPath);
		if (LoadedEntity.isValid())
		{
			UE_LOG(LogPICOSpatial, Log, TEXT("[GlbUsdViewer] Loaded model from explicit Android storage path. Obj=%s Path='%s'"),
				*GetName(),
				*ConfiguredModelPath);
			RefreshTickPolicy();
			SyncLoadedEntityVisibility();
			return true;
		}

		UE_LOG(LogPICOSpatial, Warning, TEXT("[GlbUsdViewer] Explicit Android storage load failed, falling back to UserCustomAssets. Obj=%s Path='%s'"),
			*GetName(),
			*ConfiguredModelPath);
	}

	const FString UserCustomAssetName = BuildUserCustomAssetName(ConfiguredModelPath);
	if (!UserCustomAssetName.IsEmpty())
	{
		LoadedEntity = PICOSpatial.LoadAsset(UserCustomAssetName, EAssetTypePICO::Mesh, true);
		if (LoadedEntity.isValid())
		{
			UE_LOG(LogPICOSpatial, Log, TEXT("[GlbUsdViewer] Loaded model from UserCustomAssets fallback. Obj=%s Name='%s'"),
				*GetName(),
				*UserCustomAssetName);
			RefreshTickPolicy();
			SyncLoadedEntityVisibility();
			return true;
		}
	}

	RefreshTickPolicy();
	UE_LOG(LogPICOSpatial, Warning, TEXT("[GlbUsdViewer] Failed to load model from explicit path and UserCustomAssets fallback. Obj=%s Path='%s' Fallback='%s'"),
		*GetName(),
		*ConfiguredModelPath,
		*UserCustomAssetName);
#endif
	return false;
}

void UPICOSpatialGlbUsdViewer::ReleaseLoadedEntity()
{
#if PLATFORM_ANDROID
	bIsAttached = false;
	if (LoadedEntity.isValid())
	{
		LoadedEntity.destroy();
		LoadedEntity = spatialpack::SceneEntity();
	}
	RefreshTickPolicy();
#else
	bIsAttached = false;
#endif
}

void UPICOSpatialGlbUsdViewer::SyncLoadedEntityVisibility()
{
#if PLATFORM_ANDROID
	if (!LoadedEntity.isValid())
	{
		return;
	}

	const bool bVisible = IsVisible() && !bHiddenInGame;
	if (bVisible != LoadedEntity.isEnabled())
	{
		LoadedEntity.setEnabled(bVisible);
	}
#endif
}

void UPICOSpatialGlbUsdViewer::RefreshTickPolicy()
{
#if PLATFORM_ANDROID
	const bool bShouldTick = LoadedEntity.isValid();
	SetComponentTickEnabled(bShouldTick);
	PrimaryComponentTick.TickInterval = bIsAttached ? 0.2f : 0.0f;
#endif
}

// Called every frame
void UPICOSpatialGlbUsdViewer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if PLATFORM_ANDROID 
	if (!IsValid(Container) || Container->Root3dComponent == nullptr || Container->SpatialContainer == nullptr)
	{
		if (bIsAttached)
		{
			bIsAttached = false;
			RefreshTickPolicy();
		}
		return;
	}

	if (LoadedEntity.isValid())
	{
		if (!bIsAttached)
		{
				const FEntityProxyKey ProxyKey = FPICOSpatial::MakeEntityProxyKey(GetUniqueID(), Container->Root3dComponent->GetUniqueID());
				FSceneEntityProxy* EntityFound = FPICOSpatial::Get().FindEntityProxy(ProxyKey, EEntityProxyType::SceneEntity);
				if (EntityFound && EntityFound->NativeEntity.IsValid() && EntityFound->NativeEntity->IsValid())
				{
					LoadedEntity.setParent(EntityFound->NativeEntity.Get()->entity);

				Container->SpatialContainer->addEntity(LoadedEntity);
				TraverseAllEntityChildrenRecursive(LoadedEntity, [this](spatialpack::SceneEntity& E)->bool {
					if (E.isValid())
					{
						Container->SpatialContainer->addEntity(E);
					}
					return true;
					});
				bIsAttached = true;
				RefreshTickPolicy();
				SyncLoadedEntityVisibility();
			}
		}
	}
#endif // PLATFORM_ANDROID
}
