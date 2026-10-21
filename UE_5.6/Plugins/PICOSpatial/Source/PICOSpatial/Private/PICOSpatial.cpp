// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICOSpatial.h"
#include "PICOSpatialParticleEntity.h"
#include "PICOSpatialRuntimeSettings.h"
#include "LevelInstance/LevelInstanceEditorInstanceActor.h"
#include "Components/AudioComponent.h"
#include "PICOSpatialContainer.h"
#include "Engine/Level.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ClearQuad.h"
#include "RenderUtils.h"
#include "RHIStaticStates.h"
#include "PICOSpatialWorldDependenciesAsset.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "Shader.h"
#include "ScreenRendering.h"
#include "CommonRenderResources.h"
#include "RenderGraphBuilder.h"
#include "PICOSpatialGlbUsdViewer.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/EngineVersion.h"
#include "Misc/PackageName.h"
#include "Engine/World.h"
#include "TextureResource.h"
#include "WorldPartition/DataLayer/DataLayerInstance.h"
#include "Components/ArrowComponent.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "PICOSpatialAssetMappingsAsset.h"

#if PLATFORM_ANDROID
#include "Android/AndroidJNI.h"
#include <dlfcn.h> 
#include "Android/AndroidApplication.h"
#include <android_native_app_glue.h>
#include "SpatialPackLoader.h"
extern struct android_app* GNativeAndroidApp;
#include "IVulkanDynamicRHI.h"
#endif

// Forward declaration of helper function
static void ProcessComponentsForContainer(
	FPICOSpatial* PICOSpatialInstance,
	APICOSpatialContainer* Container,
	const TInlineComponentArray<USceneComponent*>& Components,
	FSceneEntityProxy* RootEntityFound,
	uint32 ContainerRootID,
	bool bIsBounded,
	const FBox& ContainerBounds);

static bool PICOSpatialKeepBackgroundContainerEntityProxiesAlive()
{
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("PICO.KeepBackgroundContainerEntityProxiesAlive")))
	{
		return CVar->GetBool();
	}

	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	return Settings && Settings->bKeepBackgroundContainerEntityProxiesAlive;
}

#if PLATFORM_ANDROID
static const TCHAR* GPICOSpatialWorldDependenciesAssetReference =
	TEXT("/Script/PICOSpatial.PICOSpatialWorldDependenciesAsset'/Game/AssetsPICO/DA_WorldDependencies.DA_WorldDependencies'");

static FString PICOSpatialNormalizeWorldPackageNameForDependencies(FString MapName)
{
	if (MapName.IsEmpty())
	{
		return FString();
	}

	int32 OptionsIndex = INDEX_NONE;
	if (MapName.FindChar(TEXT('?'), OptionsIndex))
	{
		MapName.LeftInline(OptionsIndex);
	}

	MapName = UWorld::RemovePIEPrefix(MapName);

	FString PackageName = FPackageName::ObjectPathToPackageName(MapName);
	if (PackageName.IsEmpty())
	{
		PackageName = MapName;
	}

	return FPackageName::IsValidLongPackageName(PackageName) ? PackageName : FString();
}

static FString PICOSpatialNormalizeAssetPath(const FString& InPath)
{
	if (InPath.StartsWith(TEXT("/")))
	{
		return InPath;
	}
	return FString(TEXT("/")) + InPath;
}

static bool PICOSpatialHasManualAssetMappings(const UPICOSpatialRuntimeSettings* Settings)
{
	return Settings && !Settings->AssetMappingsAsset.IsNull();
}

static bool PICOSpatialUseOnlyManualAssetMappings(const UPICOSpatialRuntimeSettings* Settings)
{
	return Settings
		&& Settings->bUseOnlyManualAssetMappings;
}

static FString PICOSpatialNormalizeManualMappingKey(const FString& InPath)
{
	FString Path = InPath.TrimStartAndEnd();
	if (Path.IsEmpty())
	{
		return FString();
	}

	int32 FirstQuote = INDEX_NONE;
	int32 LastQuote = INDEX_NONE;
	if (Path.FindChar(TEXT('\''), FirstQuote) && Path.FindLastChar(TEXT('\''), LastQuote) && LastQuote > FirstQuote)
	{
		Path = Path.Mid(FirstQuote + 1, LastQuote - FirstQuote - 1);
	}

	Path.ReplaceInline(TEXT("\\"), TEXT("/"));
	if (Path.StartsWith(TEXT("Game/")))
	{
		Path = FString(TEXT("/")) + Path;
	}

	int32 DotIndex = INDEX_NONE;
	if (Path.FindChar(TEXT('.'), DotIndex) && DotIndex > 0)
	{
		const FString PackageCandidate = Path.Left(DotIndex);
		if (PackageCandidate.StartsWith(TEXT("/Game/")) || PackageCandidate.StartsWith(TEXT("Game/")))
		{
			Path = PackageCandidate.StartsWith(TEXT("/")) ? PackageCandidate : FString(TEXT("/")) + PackageCandidate;
		}
	}

	return Path;
}

static const UPICOSpatialAssetMappingsAsset* PICOSpatialLoadMappingsAsset()
{
	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	if (!Settings)
	{
		return nullptr;
	}

	if (Settings->AssetMappingsAsset.IsNull())
	{
		return nullptr;
	}

	const UPICOSpatialAssetMappingsAsset* Asset = nullptr;
	Asset = Cast<UPICOSpatialAssetMappingsAsset>(Settings->AssetMappingsAsset.TryLoad());

	return Asset;
}

static FString PICOSpatialResolveMappedAssetPath(EAssetTypePICO Type, const FString& UEAssetPath, bool* bOutUsedMapping)
{
	if (bOutUsedMapping)
	{
		*bOutUsedMapping = false;
	}

	const UPICOSpatialAssetMappingsAsset* Asset = PICOSpatialLoadMappingsAsset();
	if (!Asset)
	{
		return FString();
	}

	const TMap<FString, FString>* Map = nullptr;
	switch (Type)
	{
	case EAssetTypePICO::Mesh:
		Map = &Asset->MeshMappings;
		break;
	case EAssetTypePICO::Material:
		Map = &Asset->MaterialMappings;
		break;
	case EAssetTypePICO::Texture2D:
		Map = &Asset->TextureMappings;
		break;
	default:
		break;
	}

	if (!Map)
	{
		return FString();
	}

	if (const FString* Found = Map->Find(UEAssetPath))
	{
		if (bOutUsedMapping)
		{
			*bOutUsedMapping = true;
		}
		return *Found;
	}

	const FString NormalizedUEAssetPath = PICOSpatialNormalizeManualMappingKey(UEAssetPath);
	if (!NormalizedUEAssetPath.IsEmpty())
	{
		for (const TPair<FString, FString>& Pair : *Map)
		{
			if (PICOSpatialNormalizeManualMappingKey(Pair.Key) == NormalizedUEAssetPath)
			{
				if (bOutUsedMapping)
				{
					*bOutUsedMapping = true;
				}
				return Pair.Value;
			}
		}
	}

	return FString();
}

static FString PICOSpatialGetApkAssetUri(const FString& InPath)
{
	FString NormalizedPath = PICOSpatialNormalizeAssetPath(InPath);
	if (NormalizedPath.StartsWith(TEXT("/AssetsPICO/")))
	{
		return NormalizedPath.RightChop(1);
	}

	return FString(TEXT("AssetsPICO")) + NormalizedPath;
}

static bool PICOSpatialHasSceneAssetExtension(const FString& InPath)
{
	const FString Extension = FPaths::GetExtension(InPath, false).ToLower();
	return Extension == TEXT("usd")
		|| Extension == TEXT("usda")
		|| Extension == TEXT("usdz")
		|| Extension == TEXT("glb")
		|| Extension == TEXT("gltf");
}

static const TCHAR* const* PICOSpatialGetSceneAssetExtensions(int32& OutNumExtensions)
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
static bool PICOSpatialTrySceneAssetCandidates(const FString& InBasePath, LoaderFunc&& TryCandidate)
{
	if (PICOSpatialHasSceneAssetExtension(InBasePath))
	{
		return TryCandidate(InBasePath);
	}

	int32 NumExtensions = 0;
	const TCHAR* const* CandidateExtensions = PICOSpatialGetSceneAssetExtensions(NumExtensions);
	for (int32 Index = 0; Index < NumExtensions; ++Index)
	{
		if (TryCandidate(InBasePath + CandidateExtensions[Index]))
		{
			return true;
		}
	}

	return false;
}

static spatialpack::SceneEntity PICOSpatialLoadSceneEntityFromApkAssets(
	spatialpack::Scene* Scene,
	const FString& InPath,
	spatialpack::SceneEntityLoadFlag LoadFlag,
	FString* OutResolvedAssetUri = nullptr)
{
	if (OutResolvedAssetUri)
	{
		OutResolvedAssetUri->Reset();
	}

	const FString AssetUriBase = PICOSpatialGetApkAssetUri(InPath);
	auto TryLoad = [&](const FString& CandidateUri) -> spatialpack::SceneEntity
	{
		spatialpack::SceneEntity Entity = spatialpack::SceneEntity::loadFromAssets(Scene, TCHAR_TO_UTF8(*CandidateUri), LoadFlag);
		if (Entity.isValid() && OutResolvedAssetUri)
		{
			*OutResolvedAssetUri = CandidateUri;
		}
		return Entity;
	};

	spatialpack::SceneEntity LoadedEntity;

	PICOSpatialTrySceneAssetCandidates(AssetUriBase, [&](const FString& CandidateUri) -> bool
	{
		if (spatialpack::SceneEntity Entity = TryLoad(CandidateUri); Entity.isValid())
		{
			LoadedEntity = Entity;
			return true;
		}

		return false;
	});

	return LoadedEntity;
}
#endif

FPICOSpatial::FPICOSpatial()
{
	UE_LOG(LogPICOSpatial, Log, TEXT("FPICOSpatial() Ready!"));
}

FPICOSpatial::~FPICOSpatial()
{
}

void FPICOSpatial::ResetNativeCachedResources()
{
#if PLATFORM_ANDROID
	MeshResources.Reset();
	MaterialResources.Reset();
	DynamicMaterialInstanceResources.Reset();
	DynamicMaterialInstanceRefCounts.Reset();
	TextureResources.Reset();
	ReportedNativeResourceLoadFailures.Reset();
	NativeResourceFailureRetryTimes.Reset();
#endif
}

#if PLATFORM_ANDROID
namespace
{
	constexpr double NativeResourceFailureRetryDelaySeconds = 5.0;

	FString MakeNativeResourceFailureKey(EAssetTypePICO Type, const FString& UEAssetPath, const FString& MappedAssetPath, bool bUsedMapping, bool bUseOnlyManualMappings)
	{
		return FString::Printf(TEXT("%d|%s|%s|%d|%d"),
			static_cast<int32>(Type),
			*UEAssetPath,
			*MappedAssetPath,
			bUsedMapping ? 1 : 0,
			bUseOnlyManualMappings ? 1 : 0);
	}
}

bool FPICOSpatial::IsNativeResourceLoadFailureCoolingDown(EAssetTypePICO Type, const FString& UEAssetPath, const FString& MappedAssetPath, bool bUsedMapping, bool bUseOnlyManualMappings)
{
	const FString FailureKey = MakeNativeResourceFailureKey(Type, UEAssetPath, MappedAssetPath, bUsedMapping, bUseOnlyManualMappings);
	const double Now = FPlatformTime::Seconds();
	if (const double* NextRetryTime = NativeResourceFailureRetryTimes.Find(FailureKey))
	{
		if (Now < *NextRetryTime)
		{
			return true;
		}

		NativeResourceFailureRetryTimes.Remove(FailureKey);
		ReportedNativeResourceLoadFailures.Remove(FailureKey);
	}

	return false;
}

void FPICOSpatial::LogNativeResourceLoadFailure(EAssetTypePICO Type, const FString& UEAssetPath, const FString& MappedAssetPath, bool bUsedMapping, bool bUseOnlyManualMappings)
{
	const FString FailureKey = MakeNativeResourceFailureKey(Type, UEAssetPath, MappedAssetPath, bUsedMapping, bUseOnlyManualMappings);
	NativeResourceFailureRetryTimes.FindOrAdd(FailureKey) = FPlatformTime::Seconds() + NativeResourceFailureRetryDelaySeconds;

	if (ReportedNativeResourceLoadFailures.Contains(FailureKey))
	{
		return;
	}
	ReportedNativeResourceLoadFailures.Add(FailureKey);

	const TCHAR* ResourceType = TEXT("Unknown");
	switch (Type)
	{
	case EAssetTypePICO::Mesh:
		ResourceType = TEXT("Mesh");
		break;
	case EAssetTypePICO::Material:
		ResourceType = TEXT("Material");
		break;
	case EAssetTypePICO::Texture2D:
		ResourceType = TEXT("Texture2D");
		break;
	default:
		break;
	}

	if (bUseOnlyManualMappings && !bUsedMapping)
	{
		UE_LOG(LogPICOSpatial, Error, TEXT("PICO Spatial manual-only asset mappings enabled, but no native mapping was found. Type:%s UEAsset:%s"), ResourceType, *UEAssetPath);
	}
	else if (bUseOnlyManualMappings)
	{
		UE_LOG(LogPICOSpatial, Error, TEXT("PICO Spatial failed to load required mapped native resource. Type:%s UEAsset:%s MappedEntry:%s"), ResourceType, *UEAssetPath, *MappedAssetPath);
	}
	else if (bUsedMapping)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("PICO Spatial failed to load native resource after mapped/default/loose fallbacks. Type:%s UEAsset:%s MappedEntry:%s"), ResourceType, *UEAssetPath, *MappedAssetPath);
	}
	else
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("PICO Spatial failed to load native resource. Type:%s UEAsset:%s"), ResourceType, *UEAssetPath);
	}
}
#endif

void FPICOSpatial::EnsureRuntimeAssetBundlesLoaded()
{
#if PLATFORM_ANDROID
	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	if (!Settings)
	{
		return;
	}

	const bool bUseOnlyManualMappings = PICOSpatialUseOnlyManualAssetMappings(Settings);
	const bool bHasManualMappings = PICOSpatialHasManualAssetMappings(Settings);
	if (!(Settings->bUsedBundleTool || bHasManualMappings))
	{
		return;
	}

	if (Settings->bUsedBundleTool && !bUseOnlyManualMappings && !AssetBundle)
	{
		static const TCHAR* DefaultBundleFile = TEXT("pico_spatial.bundle");
		AssetBundle = spatialpack::AssetBundle::loadFromAssets(TCHAR_TO_UTF8(DefaultBundleFile));
		if (!AssetBundle)
		{
			UE_LOG(LogPICOSpatial, Error, TEXT("Failed to load asset bundle: %s"), DefaultBundleFile);
		}
	}

	if (!AssetMappingsBundle && bHasManualMappings)
	{
		static const TCHAR* MappingsBundleFile = TEXT("pico_spatial_mappings.bundle");
		AssetMappingsBundle = spatialpack::AssetBundle::loadFromAssets(TCHAR_TO_UTF8(MappingsBundleFile));
		if (!AssetMappingsBundle)
		{
			if (bUseOnlyManualMappings)
			{
				UE_LOG(LogPICOSpatial, Error, TEXT("Failed to load required manual mappings asset bundle: %s"), MappingsBundleFile);
			}
			else
			{
				UE_LOG(LogPICOSpatial, Warning, TEXT("Failed to load optional mappings asset bundle: %s"), MappingsBundleFile);
			}
		}
	}
#endif
}

#if PLATFORM_ANDROID
spatialpack::AssetBundle* FPICOSpatial::GetAssetMappingsBundle()
{
        EnsureRuntimeAssetBundlesLoaded();
        return AssetMappingsBundle;
}
#endif

void FPICOSpatial::RegisterRuntimeDelegates()
{
	if (bRuntimeDelegatesRegistered)
	{
		return;
	}

	OnWorldTickStartDelegateHandle = FWorldDelegates::OnWorldTickStart.AddRaw(this, &FPICOSpatial::OnWorldTickStart);
	OnWorldTickEndDelegateHandle = FWorldDelegates::OnWorldTickEnd.AddRaw(this, &FPICOSpatial::OnWorldTickEnd);
	OnPreLoadMapDelegateHandle = FCoreUObjectDelegates::PreLoadMap.AddRaw(this, &FPICOSpatial::OnPreLoadMap);
	OnPostLoadMapDelegateHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddRaw(this, &FPICOSpatial::OnPostLoadMap);
	OnInstanceIndexUpdatedDelegateHandle = FInstancedStaticMeshDelegates::OnInstanceIndexUpdated.AddRaw(this, &FPICOSpatial::HandleStaticMeshComponentInstanceIndexUpdated);
	bRuntimeDelegatesRegistered = true;
}

void FPICOSpatial::UnregisterRuntimeDelegates()
{
	if (!bRuntimeDelegatesRegistered)
	{
		return;
	}

	FWorldDelegates::OnWorldTickStart.Remove(OnWorldTickStartDelegateHandle);
	FWorldDelegates::OnWorldTickEnd.Remove(OnWorldTickEndDelegateHandle);
	FCoreUObjectDelegates::PreLoadMap.Remove(OnPreLoadMapDelegateHandle);
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(OnPostLoadMapDelegateHandle);
	FInstancedStaticMeshDelegates::OnInstanceIndexUpdated.Remove(OnInstanceIndexUpdatedDelegateHandle);

	OnWorldTickStartDelegateHandle.Reset();
	OnWorldTickEndDelegateHandle.Reset();
	OnPreLoadMapDelegateHandle.Reset();
	OnPostLoadMapDelegateHandle.Reset();
	OnInstanceIndexUpdatedDelegateHandle.Reset();
	bRuntimeDelegatesRegistered = false;
}

void FPICOSpatial::RegisterWindowCallbacks()
{
#if PLATFORM_ANDROID
	if (bWindowCallbacksRegistered)
	{
		return;
	}

	spatialpack::WindowContainer::setOnCreateCallback(FPICOSpatial::WindowContainerOnCreatedCallback);
	spatialpack::WindowContainer::setOnDestroyCallback(FPICOSpatial::WindowContainerOnDestroyedCallback);
	spatialpack::WindowContainer::setOnForegroundCallback(FPICOSpatial::WindowContainerOnForegroundCallback);
	spatialpack::WindowContainer::setOnBackgroundCallback(FPICOSpatial::WindowContainerOnBackgroundCallback);
	spatialpack::WindowContainer::setOnFocusCallback(FPICOSpatial::WindowContainerOnFocusCallback);
	spatialpack::WindowContainer::setOnSightCallback(FPICOSpatial::WindowContainerOnSightCallback);
	spatialpack::Stage::setOnCreateCallback(FPICOSpatial::WindowContainerOnCreatedCallback);
	spatialpack::Stage::setOnDestroyCallback(FPICOSpatial::WindowContainerOnDestroyedCallback);
	spatialpack::Stage::setOnForegroundCallback(FPICOSpatial::WindowContainerOnForegroundCallback);
	spatialpack::Stage::setOnBackgroundCallback(FPICOSpatial::WindowContainerOnBackgroundCallback);
	bWindowCallbacksRegistered = true;
#endif
}

void FPICOSpatial::UnregisterWindowCallbacks()
{
#if PLATFORM_ANDROID
	if (!bWindowCallbacksRegistered)
	{
		return;
	}

	spatialpack::WindowContainer::setOnCreateCallback({});
	spatialpack::WindowContainer::setOnDestroyCallback({});
	spatialpack::WindowContainer::setOnForegroundCallback({});
	spatialpack::WindowContainer::setOnBackgroundCallback({});
	spatialpack::WindowContainer::setOnFocusCallback({});
	spatialpack::WindowContainer::setOnSightCallback({});
	spatialpack::Stage::setOnCreateCallback({});
	spatialpack::Stage::setOnDestroyCallback({});
	spatialpack::Stage::setOnForegroundCallback({});
	spatialpack::Stage::setOnBackgroundCallback({});
	bWindowCallbacksRegistered = false;
#endif
}

bool FPICOSpatial::InitializeRuntime()
{
#if PLATFORM_ANDROID
	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] InitializeRuntime begin. Enabled=%d Running=%d SessionStarted=%d Session=%p Instance=%p Scene=%p Loader=%p Delegates=%d WindowCallbacks=%d"),
		bPICOSpatialEnabled, bPICOSpatialRunning, bSessionStarted, Session, Instance, CurrentScene, LoaderHandle, bRuntimeDelegatesRegistered, bWindowCallbacksRegistered);
	if (bSessionStarted && Session && CurrentScene)
	{
		RegisterWindowCallbacks();
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] InitializeRuntime reuse existing session. Session=%p Scene=%p"), Session, CurrentScene);
		return true;
	}

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (!Env || !GNativeAndroidApp || !GNativeAndroidApp->activity || !GNativeAndroidApp->activity->clazz)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("PICO Spatial runtime init skipped because Android activity context is not ready."));
		return false;
	}

	if (!bSpatialPlatformAvailable)
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_IsSpatialPlatform", "()Z", false);
		bSpatialPlatformAvailable = FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}

	if (!bSpatialPlatformAvailable)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] InitializeRuntime aborted because current platform is not spatial."));
		return false;
	}

	if (!LoaderHandle)
	{
		dlerror();
		LoaderHandle = FPlatformProcess::GetDllHandle(TEXT("libspatial_pack_loader.so"));
		dlerror();

		if (!LoaderHandle)
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("Failed to find spatial runtime loader."));
			return false;
		}
	}

	spatialpack::SpatialPackLoader::load(Env, GNativeAndroidApp->activity->clazz);
	UE_LOG(LogPICOSpatial, Log, TEXT("SpatialPackLoader loaded!"));

	const FString InstanceSessionInfo = FApp::GetProjectName() + FString(" + ") + FApp::GetEpicProductIdentifier() + FEngineVersion::Current().ToString(EVersionComponent::Minor);
	UE_LOG(LogPICOSpatial, Log, TEXT("Instance Name:%s"), *InstanceSessionInfo);

	if (!Instance)
	{
		auto InstanceBuilder = spatialpack::SpatialInstanceInitInfoBuilder::create();
		InstanceBuilder.setAndroidContext(GNativeAndroidApp->activity->clazz).setJavaEnv(Env).setInstanceName(TCHAR_TO_UTF8(*InstanceSessionInfo));
		auto InitInfo = InstanceBuilder.build();

		Instance = spatialpack::SpatialInstance::createInstance();
		if (!Instance)
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("Failed to create SpatialInstance."));
			DestroyRuntimeObjects(true);
			return false;
		}

		Instance->initInstanceWithInfo(InitInfo);
	}

	if (!Session)
	{
		auto SessionCreateInfoBuilder = spatialpack::SpatialSessionCreateInfoBuilder::create();
		SessionCreateInfoBuilder.setSessionName(TCHAR_TO_UTF8(*InstanceSessionInfo));
		auto CreateInfo = SessionCreateInfoBuilder.build();

		const spatialpack::SpatialResult CreateSessionResult = Instance->createSessionWithInfo(CreateInfo, &Session);
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] createSessionWithInfo result=%d Session=%p Instance=%p"), (int32)CreateSessionResult, Session, Instance);
		if (CreateSessionResult != spatialpack::SpatialResult::SUCCESS || !Session)
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("Failed to create Spatial Session."));
			DestroyRuntimeObjects(true);
			return false;
		}
	}

	const spatialpack::SpatialResult SessionBeginResult = Session->beginSession();
	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] beginSession result=%d Session=%p"), (int32)SessionBeginResult, Session);
	if (SessionBeginResult != spatialpack::SpatialResult::SUCCESS)
	{
		DestroyRuntimeObjects(true);
		return false;
	}

	bSessionStarted = true;
	CurrentScene = Session->getAppScene();
	FramePhase = EFramePhase::Idle;
	RegisterWindowCallbacks();
	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] InitializeRuntime done. Session=%p Scene=%p FramePhase=%d"), Session, CurrentScene, (int32)FramePhase);
	return CurrentScene != nullptr;
#else
	return false;
#endif
}

void FPICOSpatial::ShutdownRuntime(bool bClearAllNativeCachedResources)
{
	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] ShutdownRuntime begin. ClearCache=%d Enabled=%d Running=%d SessionStarted=%d Session=%p Instance=%p Scene=%p Loader=%p Containers=%d Delegates=%d WindowCallbacks=%d"),
		bClearAllNativeCachedResources, bPICOSpatialEnabled, bPICOSpatialRunning, bSessionStarted, Session, Instance, CurrentScene, LoaderHandle, Containers.Num(), bRuntimeDelegatesRegistered, bWindowCallbacksRegistered);
	bPICOSpatialEnabled = false;
	bPICOSpatialRunning = false;
	UnregisterRuntimeDelegates();

#if PLATFORM_ANDROID
	if (Session && bSessionStarted)
	{
		AdvanceFrameTo(EFramePhase::Begun);
	}

	if (Containers.Num() > 0)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] ShutdownRuntime closing %d containers."), Containers.Num());
		TSet<APICOSpatialContainer*> OpenedContainers = Containers;
		for (APICOSpatialContainer* Container : OpenedContainers)
		{
			if (!Container || !Container->IsValidLowLevel())
			{
				continue;
			}

			if (Container->bIsSpatialContainerOpening)
			{
				UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] ShutdownRuntime closing container actor=%s"), *GetNameSafe(Container));
				Container->CloseSpatialContainerPICO();
			}

			Container->bIsSpatialContainerOpening = false;
			Container->bHasEnterForeground = false;
			Container->bHasFocus = false;
			Container->bIsOnSight = false;
			Container->bBindedToSpatialContainer = false;
			Container->bBackgroundTimeout = false;
		}
	}

	DestroyAllEntityProxies();
	Containers.Empty();
	NativeEntityToSceneComponentMap.Reset();

	if (Session && bSessionStarted)
	{
		AdvanceFrameTo(EFramePhase::Idle);
	}

	if (AssetMappingsBundle)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] ShutdownRuntime destroying asset mappings bundle."));
		spatialpack::AssetBundle::destroyBundle(AssetMappingsBundle);
		AssetMappingsBundle = nullptr;
	}

	if (AssetBundle)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] ShutdownRuntime destroying asset bundle."));
		spatialpack::AssetBundle::destroyBundle(AssetBundle);
		AssetBundle = nullptr;
	}

	UnregisterWindowCallbacks();
	if (bClearAllNativeCachedResources)
	{
		ResetNativeCachedResources();
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] ShutdownRuntime native caches reset."));
	}
	else
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] ShutdownRuntime native caches preserved."));
	}
	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] ShutdownRuntime done."));
#endif
}

void FPICOSpatial::DestroyRuntimeObjects(bool bClearAllNativeCachedResources)
{
#if PLATFORM_ANDROID
	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] DestroyRuntimeObjects begin. ClearCache=%d SessionStarted=%d Session=%p Instance=%p Loader=%p"),
		bClearAllNativeCachedResources, bSessionStarted, Session, Instance, LoaderHandle);

	UnregisterWindowCallbacks();

	if (Session)
	{
		if (bSessionStarted)
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] DestroyRuntimeObjects calling endSession. Session=%p"), Session);
			Session->endSession();
		}

		if (Instance)
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] DestroyRuntimeObjects calling destroySession. Instance=%p Session=%p"), Instance, Session);
			Instance->destroySession(&Session);
		}

		Session = nullptr;
	}

	CurrentScene = nullptr;
	bSessionStarted = false;
	FramePhase = EFramePhase::Idle;
	NativeEntityToSceneComponentMap.Reset();

	if (AssetMappingsBundle)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] DestroyRuntimeObjects destroying asset mappings bundle."));
		spatialpack::AssetBundle::destroyBundle(AssetMappingsBundle);
		AssetMappingsBundle = nullptr;
	}

	if (AssetBundle)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] DestroyRuntimeObjects destroying asset bundle."));
		spatialpack::AssetBundle::destroyBundle(AssetBundle);
		AssetBundle = nullptr;
	}

	if (bClearAllNativeCachedResources)
	{
		ResetNativeCachedResources();
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] DestroyRuntimeObjects native caches reset."));
	}

	if (Instance)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] DestroyRuntimeObjects calling destroyInstance. Instance=%p"), Instance);
		spatialpack::SpatialInstance::destroyInstance(Instance);
		Instance = nullptr;
	}

	if (LoaderHandle)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] DestroyRuntimeObjects freeing loader handle=%p"), LoaderHandle);
		dlerror();
		FPlatformProcess::FreeDllHandle(LoaderHandle);
		dlerror();
		LoaderHandle = nullptr;
	}

	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] DestroyRuntimeObjects done."));
#else
	(void)bClearAllNativeCachedResources;
#endif
}

void FPICOSpatial::Register()
{
#if PLATFORM_ANDROID
	JNIEnv* env = FAndroidApplication::GetJavaEnv();

	bool bIsSpatialPlatform = false;
	if (env)
	{
		static jmethodID Method = FJavaWrapper::FindMethod(env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_IsSpatialPlatform", "()Z", false);
		bIsSpatialPlatform = FJavaWrapper::CallBooleanMethod(env, FJavaWrapper::GameActivityThis, Method);
		UE_LOG(LogPICOSpatial, Log, TEXT("IsSpatialPlatform:%d"), bIsSpatialPlatform);
	}

	bSpatialPlatformAvailable = bIsSpatialPlatform;
#endif

	bool bStartInPICOSpatial = false;
	GConfig->GetBool(TEXT("/Script/PICOSpatialRuntimeSettings.PICOSpatialRuntimeSettings"), TEXT("bStartInPICOSpatial"), bStartInPICOSpatial, GEngineIni);
	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] Register complete. SpatialPlatform=%d bStartInPICOSpatial=%d"), bSpatialPlatformAvailable, bStartInPICOSpatial);

	if (!InitializeRuntime())
	{
		return;
	}

	if (bStartInPICOSpatial)
	{
		EnsureRuntimeAssetBundlesLoaded();
		RegisterRuntimeDelegates();
		bPICOSpatialEnabled = true;
	}
	else
	{
		UnregisterWindowCallbacks();
	}
}

void FPICOSpatial::Unregister()
{
	ShutdownRuntime(true);
	DestroyRuntimeObjects(true);
}


bool FPICOSpatial::EnablePICOSpatial(bool bEnable, bool bClearAllNativeCachedResources)
{
	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] EnablePICOSpatial request. Enable=%d ClearCache=%d CurrentEnabled=%d Running=%d SessionStarted=%d Session=%p Instance=%p Scene=%p Containers=%d"),
		bEnable, bClearAllNativeCachedResources, bPICOSpatialEnabled, bPICOSpatialRunning, bSessionStarted, Session, Instance, CurrentScene, Containers.Num());
	if (bEnable)
	{
		if (!InitializeRuntime())
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] EnablePICOSpatial failed during InitializeRuntime."));
			return false;
		}

		EnsureRuntimeAssetBundlesLoaded();
		RegisterRuntimeDelegates();
		bPICOSpatialEnabled = true;
		UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] EnablePICOSpatial success. Enabled=%d Session=%p Scene=%p"), bPICOSpatialEnabled, Session, CurrentScene);
		return true;
	}

	ShutdownRuntime(bClearAllNativeCachedResources);
	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] EnablePICOSpatial disable finished. Enabled=%d Session=%p Instance=%p"), bPICOSpatialEnabled, Session, Instance);
	return true;
}

bool FPICOSpatial::IsPICOSpatialEnabled() const
{
	return bPICOSpatialEnabled;
}

void FPICOSpatial::PreCacheResources(const TMap<FString, EAssetTypePICO>& Dependencies)
{
#if PLATFORM_ANDROID
	if (!bPICOSpatialEnabled)
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("Skip PreCacheResources because PICOSpatial is disabled."));
		return;
	}

	const UPICOSpatialRuntimeSettings* PICOSpatialRuntimeSettings = GetDefault<UPICOSpatialRuntimeSettings>();
	if (PICOSpatialRuntimeSettings && PICOSpatialRuntimeSettings->bPreCacheAssetsPerLevel && CurrentScene)
	{
		if (!PICOSpatialRuntimeSettings->bKeepCachedAssetsAllLevels)
		{
			MeshResources.Reset();
			MaterialResources.Reset();
			DynamicMaterialInstanceResources.Reset();
			DynamicMaterialInstanceRefCounts.Reset();
			TextureResources.Reset();
			ReportedNativeResourceLoadFailures.Reset();
			NativeResourceFailureRetryTimes.Reset();
		}

		SCOPED_NAMED_EVENT(PreCacheResourcesPICO, FColor::Red);
		for (auto& Pair : Dependencies)
		{
			switch (Pair.Value)
			{
			case EAssetTypePICO::Mesh:
				CacheMeshResource(Pair.Key);
				break;
			case EAssetTypePICO::Material:
				CacheMaterialResource(Pair.Key, "");
				break;
			case EAssetTypePICO::Texture2D:
				CacheTextureResource(Pair.Key);
				break;
			default:
				break;
			}
		}
		UE_LOG(LogPICOSpatial, Log, TEXT("PreCacheResources"));
	}
#endif // PLATFORM_ANDROID
}

spatialpack::MeshResource* FPICOSpatial::CacheMeshResource(const FString& MeshAsset)
{
#if PLATFORM_ANDROID
	if (!bPICOSpatialEnabled)
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("Skip CacheMeshResource because PICOSpatial is disabled. MeshAsset:%s"), *MeshAsset);
		return nullptr;
	}

	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	const bool bUseOnlyManualMappings = PICOSpatialUseOnlyManualAssetMappings(Settings);
	bool bUsedMapping = false;
	const FString MappedMeshAsset = PICOSpatialResolveMappedAssetPath(EAssetTypePICO::Mesh, MeshAsset, &bUsedMapping);

	spatialpack::MeshResource* CachedMesh = MeshResources.Find(MeshAsset);
	if (CachedMesh && !CachedMesh->isValid())
	{
		MeshResources.Remove(MeshAsset);
		CachedMesh = nullptr;
	}
	if (CachedMesh == nullptr && IsNativeResourceLoadFailureCoolingDown(EAssetTypePICO::Mesh, MeshAsset, MappedMeshAsset, bUsedMapping, bUseOnlyManualMappings))
	{
		return nullptr;
	}
	if (CachedMesh == nullptr)
	{
		spatialpack::MeshResource Mesh = {};
		auto TryLoadMappedMeshFromBundle = [&](spatialpack::AssetBundle* BundleToUse, const FString& NativeEntryPath)
		{
			if (CachedMesh || !BundleToUse || NativeEntryPath.IsEmpty())
			{
				return;
			}

			Mesh = spatialpack::MeshResource::loadFromBundle(TCHAR_TO_UTF8(*NativeEntryPath), BundleToUse);
			if (Mesh.isValid())
			{
				CachedMesh = &MeshResources.FindOrAdd(MeshAsset, Mesh);
				UE_LOG(LogPICOSpatial, Log, TEXT("MeshResources From Bundle(Mappings) Num:%d, MeshAsset:%s, Mesh:%hs"), MeshResources.Num(), *MeshAsset, Mesh.getName());
			}
		};

		auto TryLoadDefaultMeshFromBundle = [&](spatialpack::AssetBundle* BundleToUse, const FString& UEPath)
		{
			if (CachedMesh || !BundleToUse)
			{
				return;
			}

			FString MeshPath = UEPath;
			MeshPath.RemoveFromStart("/");
			Mesh = spatialpack::MeshResource::loadFromBundle(TCHAR_TO_UTF8(*MeshPath.Append(TEXT("/meshes/0"))), BundleToUse);
			if (Mesh.isValid())
			{
				CachedMesh = &MeshResources.FindOrAdd(MeshAsset, Mesh);
				UE_LOG(LogPICOSpatial, Log, TEXT("MeshResources From Bundle(Default) Num:%d, MeshAsset:%s, Mesh:%hs"), MeshResources.Num(), *MeshAsset, Mesh.getName());
			}
		};

		if (bUsedMapping)
		{
			TryLoadMappedMeshFromBundle(AssetMappingsBundle, MappedMeshAsset);
			if (CachedMesh == nullptr && AssetMappingsBundle && !bUseOnlyManualMappings)
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("Mesh mapping bundle fallback to default bundle: %s -> %s"), *MeshAsset, *MappedMeshAsset);
			}
			if (!bUseOnlyManualMappings)
			{
				TryLoadDefaultMeshFromBundle(AssetBundle, MeshAsset);
			}
		}
		else if (!bUseOnlyManualMappings)
		{
			TryLoadDefaultMeshFromBundle(AssetBundle, MeshAsset);
		}

		if (CachedMesh == nullptr && !bUseOnlyManualMappings)
		{
			if (bUsedMapping)
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("Mesh mapping fallback: %s -> %s"), *MeshAsset, *MappedMeshAsset);
			}
			spatialpack::SceneEntity Entity = LoadAsset(MeshAsset, EAssetTypePICO::Mesh);
			if (Entity.isValid())
			{
				Entity.setEnabled(false);
				Mesh = Entity.getOrAddModelComponent().getMesh();
				if (Mesh.isValid())
				{
					UE_LOG(LogPICOSpatial, Log, TEXT("MeshResources Num:%d, MeshAsset:%s, Mesh:%hs"), MeshResources.Num(), *MeshAsset, Mesh.getName());
					CachedMesh = &MeshResources.FindOrAdd(MeshAsset, Mesh);
				}
				Entity.destroy();
			}
		}

		if (Mesh.isValid())
		{
			TArray<spatialpack::MeshPart> Parts;
			Parts.Reset();
			Mesh.getMeshModel().foreachMeshPart([](void* user, const spatialpack::MeshPart& part) {
				TArray<spatialpack::MeshPart>* Parts = static_cast<TArray<spatialpack::MeshPart>*>(user);
				Parts->Add(part);
				}, &Parts);

			for (int32 i = 0; i < Parts.Num(); i++)
			{
				Parts[i].setMaterialIndex(i);
				Mesh.updateMeshPart(Parts[i]);
			}
		}
	}

	if (CachedMesh == nullptr)
	{
		LogNativeResourceLoadFailure(EAssetTypePICO::Mesh, MeshAsset, MappedMeshAsset, bUsedMapping, bUseOnlyManualMappings);
	}

	return CachedMesh;
#endif // PLATFORM_ANDROID
	return nullptr;
}

spatialpack::MeshResource* FPICOSpatial::CacheMappedMeshResource(const FString& CacheKey, const FString& BundleEntryPath)
{
#if PLATFORM_ANDROID
        if (!bPICOSpatialEnabled || CacheKey.IsEmpty() || BundleEntryPath.IsEmpty())
        {
                return nullptr;
        }

        spatialpack::MeshResource* CachedMesh = MeshResources.Find(CacheKey);
        if (CachedMesh && !CachedMesh->isValid())
        {
                MeshResources.Remove(CacheKey);
                CachedMesh = nullptr;
        }

        if (CachedMesh == nullptr)
        {
                spatialpack::AssetBundle* Bundle = GetAssetMappingsBundle();
                if (Bundle)
                {
                        spatialpack::MeshResource Mesh = spatialpack::MeshResource::loadFromBundle(TCHAR_TO_UTF8(*BundleEntryPath), Bundle);
                        if (Mesh.isValid())
                        {
                                TArray<spatialpack::MeshPart> Parts;
                                Parts.Reset();
                                Mesh.getMeshModel().foreachMeshPart([](void* user, const spatialpack::MeshPart& part) {
                                        TArray<spatialpack::MeshPart>* MeshParts = static_cast<TArray<spatialpack::MeshPart>*>(user);
                                        MeshParts->Add(part);
                                        }, &Parts);

                                for (int32 i = 0; i < Parts.Num(); i++)
                                {
                                        Parts[i].setMaterialIndex(i);
                                        Mesh.updateMeshPart(Parts[i]);
                                }

                                CachedMesh = &MeshResources.FindOrAdd(CacheKey, Mesh);
                                UE_LOG(LogPICOSpatial, Log, TEXT("MeshResources From ECS Mapping Bundle Num:%d, CacheKey:%s, Entry:%s"), MeshResources.Num(), *CacheKey, *BundleEntryPath);
                        }
                }
        }

        return CachedMesh;
#endif
        return nullptr;
}

spatialpack::Material* FPICOSpatial::CacheMaterialResource(const FString& InMaterial, const FString& DynamicMaterialInstance)
{
#if PLATFORM_ANDROID
	if (!bPICOSpatialEnabled)
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("Skip CacheMaterialResource because PICOSpatial is disabled. Material:%s DynamicInstance:%s"), *InMaterial, *DynamicMaterialInstance);
		return nullptr;
	}

	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	const bool bUseOnlyManualMappings = PICOSpatialUseOnlyManualAssetMappings(Settings);
	bool bUsedMapping = false;
	const FString MappedMaterial = PICOSpatialResolveMappedAssetPath(EAssetTypePICO::Material, InMaterial, &bUsedMapping);

	spatialpack::Material* CachedMaterial = MaterialResources.Find(InMaterial);
	if (CachedMaterial && !CachedMaterial->isValid())
	{
		MaterialResources.Remove(InMaterial);
		CachedMaterial = nullptr;
	}
	if (CachedMaterial == nullptr && IsNativeResourceLoadFailureCoolingDown(EAssetTypePICO::Material, InMaterial, MappedMaterial, bUsedMapping, bUseOnlyManualMappings))
	{
		return nullptr;
	}
	if (CachedMaterial == nullptr)
	{
		auto TryLoadMappedMaterialFromBundle = [&](spatialpack::AssetBundle* BundleToUse, const FString& NativeEntryPath)
		{
			if (CachedMaterial || !BundleToUse || NativeEntryPath.IsEmpty())
			{
				return;
			}

			spatialpack::Material material = BundleToUse->loadMaterial(TCHAR_TO_UTF8(*NativeEntryPath));
			if (material.isValid())
			{
				spatialpack::UnlitMaterial unlitMaterial = spatialpack::UnlitMaterial::castFromMaterial(material);
				if (unlitMaterial.isValid())
				{
					unlitMaterial.setApplyToneMapping(false);
				}

				CachedMaterial = &MaterialResources.FindOrAdd(InMaterial, material);
				UE_LOG(LogPICOSpatial, Log, TEXT("MaterialResources From Bundle(Mappings) Num:%d, UMaterialInterface:%s, Material:%hs"), MaterialResources.Num(), *InMaterial, material.getName());
			}
		};

		auto TryLoadDefaultMaterialFromBundle = [&](spatialpack::AssetBundle* BundleToUse, const FString& UEPath)
		{
			if (CachedMaterial || !BundleToUse)
			{
				return;
			}

			FString MaterialPath = UEPath;
			MaterialPath.RemoveFromStart("/");
			spatialpack::Material material = BundleToUse->loadMaterial(TCHAR_TO_UTF8(*(MaterialPath + "/Root/Material")));
			if (!material.isValid())
			{
				material = BundleToUse->loadMaterial(TCHAR_TO_UTF8(*(MaterialPath + "/materials/0")));
			}

			if (material.isValid())
			{
				spatialpack::UnlitMaterial unlitMaterial = spatialpack::UnlitMaterial::castFromMaterial(material);
				if (unlitMaterial.isValid())
				{
					unlitMaterial.setApplyToneMapping(false);
				}

				CachedMaterial = &MaterialResources.FindOrAdd(InMaterial, material);
				UE_LOG(LogPICOSpatial, Log, TEXT("MaterialResources From Bundle(Default) Num:%d, UMaterialInterface:%s, Material:%hs"), MaterialResources.Num(), *InMaterial, material.getName());
			}
		};

		if (bUsedMapping)
		{
			TryLoadMappedMaterialFromBundle(AssetMappingsBundle, MappedMaterial);
			if (CachedMaterial == nullptr && AssetMappingsBundle && !bUseOnlyManualMappings)
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("Material mapping bundle fallback to default bundle: %s -> %s"), *InMaterial, *MappedMaterial);
			}
			if (!bUseOnlyManualMappings)
			{
				TryLoadDefaultMaterialFromBundle(AssetBundle, InMaterial);
			}
		}
		else if (!bUseOnlyManualMappings)
		{
			TryLoadDefaultMaterialFromBundle(AssetBundle, InMaterial);
		}

		if (CachedMaterial == nullptr && !bUseOnlyManualMappings)
		{
			if (bUsedMapping)
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("Material mapping fallback: %s -> %s"), *InMaterial, *MappedMaterial);
			}
			spatialpack::SceneEntity Entity = LoadAsset(InMaterial, EAssetTypePICO::Material);
			if (Entity.isValid())
			{
				Entity.setEnabled(false);
				spatialpack::Material material = Entity.getOrAddModelComponent().getMaterial();
				if (material.isValid())
				{
					// Try to cast to UnlitMaterial and disable tonemapping if successful
					spatialpack::UnlitMaterial unlitMaterial = spatialpack::UnlitMaterial::castFromMaterial(material);
					if (unlitMaterial.isValid())
					{
						unlitMaterial.setApplyToneMapping(false);
					}

					CachedMaterial = &MaterialResources.FindOrAdd(InMaterial, material);
					UE_LOG(LogPICOSpatial, Log, TEXT("MaterialResources Num:%d, UMaterialInterface:%s, Material:%hs"), MaterialResources.Num(), *InMaterial, material.getName());
				}
				Entity.destroy();
			}
		}
	}

	if (CachedMaterial == nullptr)
	{
		LogNativeResourceLoadFailure(EAssetTypePICO::Material, InMaterial, MappedMaterial, bUsedMapping, bUseOnlyManualMappings);
	}

	if (!DynamicMaterialInstance.IsEmpty())
	{
		spatialpack::Material* CachedDynamicMaterialInstance = DynamicMaterialInstanceResources.Find(DynamicMaterialInstance);
		if (CachedDynamicMaterialInstance && !CachedDynamicMaterialInstance->isValid())
		{
			DynamicMaterialInstanceResources.Remove(DynamicMaterialInstance);
			CachedDynamicMaterialInstance = nullptr;
		}
		if (CachedDynamicMaterialInstance == nullptr)
		{
			if (CachedMaterial && (*CachedMaterial).isValid())
			{
				spatialpack::Material DynamicMaterialClone = (*CachedMaterial).clone();
				if (DynamicMaterialClone.isValid())
				{
					CachedDynamicMaterialInstance = &DynamicMaterialInstanceResources.FindOrAdd(DynamicMaterialInstance, DynamicMaterialClone);
				}
			}
		}
		return CachedDynamicMaterialInstance;
	}

	return CachedMaterial;
#endif // PLATFORM_ANDROID
	return nullptr;
}

spatialpack::Material* FPICOSpatial::CacheMappedMaterialResource(const FString& CacheKey, const FString& BundleEntryPath, const FString& DynamicMaterialInstance)
{
#if PLATFORM_ANDROID
        if (!bPICOSpatialEnabled || CacheKey.IsEmpty() || BundleEntryPath.IsEmpty())
        {
                return nullptr;
        }

        spatialpack::Material* CachedMaterial = MaterialResources.Find(CacheKey);
        if (CachedMaterial && !CachedMaterial->isValid())
        {
                MaterialResources.Remove(CacheKey);
                CachedMaterial = nullptr;
        }

        if (CachedMaterial == nullptr)
        {
                spatialpack::AssetBundle* Bundle = GetAssetMappingsBundle();
                if (Bundle)
                {
                        spatialpack::Material Material = Bundle->loadMaterial(TCHAR_TO_UTF8(*BundleEntryPath));
                        if (Material.isValid())
                        {
                                spatialpack::UnlitMaterial UnlitMaterial = spatialpack::UnlitMaterial::castFromMaterial(Material);
                                if (UnlitMaterial.isValid())
                                {
                                        UnlitMaterial.setApplyToneMapping(false);
                                }

                                CachedMaterial = &MaterialResources.FindOrAdd(CacheKey, Material);
                                UE_LOG(LogPICOSpatial, Log, TEXT("MaterialResources From ECS Mapping Bundle Num:%d, CacheKey:%s, Entry:%s"), MaterialResources.Num(), *CacheKey, *BundleEntryPath);
                        }
                }
        }

        if (!DynamicMaterialInstance.IsEmpty())
        {
                spatialpack::Material* CachedDynamicMaterialInstance = DynamicMaterialInstanceResources.Find(DynamicMaterialInstance);
                if (CachedDynamicMaterialInstance && !CachedDynamicMaterialInstance->isValid())
                {
                        DynamicMaterialInstanceResources.Remove(DynamicMaterialInstance);
                        CachedDynamicMaterialInstance = nullptr;
                }

                if (CachedDynamicMaterialInstance == nullptr && CachedMaterial && CachedMaterial->isValid())
                {
                        spatialpack::Material DynamicMaterialClone = CachedMaterial->clone();
                        if (DynamicMaterialClone.isValid())
                        {
                                CachedDynamicMaterialInstance = &DynamicMaterialInstanceResources.FindOrAdd(DynamicMaterialInstance, DynamicMaterialClone);
                        }
                }

                return CachedDynamicMaterialInstance;
        }

        return CachedMaterial;
#endif
        return nullptr;
}

void FPICOSpatial::AddDynamicMaterialInstanceReference(const FString& Material)
{
#if PLATFORM_ANDROID
	if (!Material.IsEmpty())
	{
		DynamicMaterialInstanceRefCounts.FindOrAdd(Material)++;
	}
#endif // PLATFORM_ANDROID
}

bool FPICOSpatial::RemoveDynamicMaterialInstance(const FString& Material, bool bForceRemove)
{
#if PLATFORM_ANDROID
	if (Material.IsEmpty())
	{
		return false;
	}

	if (!bForceRemove)
	{
		if (int32* RefCount = DynamicMaterialInstanceRefCounts.Find(Material))
		{
			(*RefCount)--;
			if (*RefCount > 0)
			{
				return false;
			}
		}
	}

	DynamicMaterialInstanceRefCounts.Remove(Material);
	DynamicMaterialInstanceResources.Remove(Material);
	return true;
#endif // PLATFORM_ANDROID
	return false;
}

spatialpack::TextureResource* FPICOSpatial::CacheTextureResource(const FString& Texture)
{
#if PLATFORM_ANDROID
	if (!bPICOSpatialEnabled)
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("Skip CacheTextureResource because PICOSpatial is disabled. Texture:%s"), *Texture);
		return nullptr;
	}

	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	const bool bUseOnlyManualMappings = PICOSpatialUseOnlyManualAssetMappings(Settings);
	bool bUsedMapping = false;
	const FString MappedTexture = PICOSpatialResolveMappedAssetPath(EAssetTypePICO::Texture2D, Texture, &bUsedMapping);

	spatialpack::TextureResource* TextureResource = TextureResources.Find(Texture);
	if (TextureResource && !TextureResource->isValid())
	{
		TextureResources.Remove(Texture);
		TextureResource = nullptr;
	}
	if (TextureResource == nullptr && IsNativeResourceLoadFailureCoolingDown(EAssetTypePICO::Texture2D, Texture, MappedTexture, bUsedMapping, bUseOnlyManualMappings))
	{
		return nullptr;
	}
	if (TextureResource == nullptr)
	{
		const TArray<FString, TInlineAllocator<2>> TextureExtensions = { TEXT(".PNG"), TEXT(".EXR") };
		auto TryLoadMappedTextureFromBundle = [&](spatialpack::AssetBundle* BundleToUse, const FString& NativeEntryPath)
		{
			if (TextureResource || !BundleToUse || NativeEntryPath.IsEmpty())
			{
				return;
			}

			TArray<FString, TInlineAllocator<2>> CandidatePaths;
			if (FPaths::GetExtension(NativeEntryPath).IsEmpty())
			{
				for (const FString& Extension : TextureExtensions)
				{
					CandidatePaths.Add(NativeEntryPath + Extension);
				}
			}
			else
			{
				CandidatePaths.Add(NativeEntryPath);
			}

			for (const FString& CandidatePath : CandidatePaths)
			{
				spatialpack::TextureResource texture = BundleToUse->loadTexture(TCHAR_TO_UTF8(*CandidatePath));
				if (texture.isValid())
				{
					TextureResource = &TextureResources.FindOrAdd(Texture, texture);
					UE_LOG(LogPICOSpatial, Log, TEXT("TextureResources From Bundle(Mappings) Num:%d, UTexture2D:%s, texture:%hs"), TextureResources.Num(), *Texture, texture.getName());
					break;
				}
			}
		};

		auto TryLoadDefaultTextureFromBundle = [&](spatialpack::AssetBundle* BundleToUse, const FString& UEPath)
		{
			if (TextureResource || !BundleToUse)
			{
				return;
			}

			for (const FString& Extension : TextureExtensions)
			{
				FString TexturePath = FString("/textures") + UEPath + Extension;
				spatialpack::TextureResource texture = BundleToUse->loadTexture(TCHAR_TO_UTF8(*TexturePath));
				if (texture.isValid())
				{
					TextureResource = &TextureResources.FindOrAdd(Texture, texture);
					UE_LOG(LogPICOSpatial, Log, TEXT("TextureResources From Bundle(Default) Num:%d, UTexture2D:%s, texture:%hs"), TextureResources.Num(), *Texture, texture.getName());
					break;
				}
			}
		};

		if (bUsedMapping)
		{
			TryLoadMappedTextureFromBundle(AssetMappingsBundle, MappedTexture);
			if (TextureResource == nullptr && AssetMappingsBundle && !bUseOnlyManualMappings)
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("Texture mapping bundle fallback to default bundle: %s -> %s"), *Texture, *MappedTexture);
			}
			if (!bUseOnlyManualMappings)
			{
				TryLoadDefaultTextureFromBundle(AssetBundle, Texture);
			}
		}
		else if (!bUseOnlyManualMappings)
		{
			TryLoadDefaultTextureFromBundle(AssetBundle, Texture);
		}

		if (TextureResource == nullptr && !bUseOnlyManualMappings)
		{
			if (bUsedMapping)
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("Texture mapping fallback: %s -> %s"), *Texture, *MappedTexture);
			}

			const bool bLoadAssetsFromApkAssets = Settings && Settings->bLoadAssetsFromApkAssets;
			if (bLoadAssetsFromApkAssets)
			{
				for (const FString& Extension : TextureExtensions)
				{
					const FString AssetUri = PICOSpatialGetApkAssetUri(Texture + Extension);
					spatialpack::TextureResource TextureFromAssets = spatialpack::TextureResource::loadFromAssetsWithOption(TCHAR_TO_UTF8(*AssetUri));
					if (TextureFromAssets.isValid())
					{
						TextureResource = &TextureResources.FindOrAdd(Texture, TextureFromAssets);
						UE_LOG(LogPICOSpatial, Log, TEXT("TextureResources From APK Assets Num:%d, UTexture2D:%s, Texture:%hs"), TextureResources.Num(), *Texture, TextureFromAssets.getName());
						break;
					}

					if (Extension == TextureExtensions.Last())
					{
						UE_LOG(LogPICOSpatial, Warning, TEXT("Failed to load texture from APK assets: %s"), *AssetUri);
					}
				}
			}

			if (TextureResource == nullptr && !bLoadAssetsFromApkAssets)
			{
				extern FString GFilePathBase;
				for (const FString& Extension : TextureExtensions)
				{
					FString Source = FPaths::ProjectContentDir() / TEXT("AssetsPICO") / Texture + Extension;
					FString Dst = GFilePathBase + FString("/UnrealGame/")
						+ (!FApp::IsProjectNameEmpty() ? FApp::GetProjectName() : FPlatformProcess::ExecutableName())
						+ FString("/")
						+ (!FApp::IsProjectNameEmpty() ? FApp::GetProjectName() : FPlatformProcess::ExecutableName())
						+ FString("/Saved/AssetsPICO") + Texture + Extension;
					if (!IFileManager::Get().FileExists(*Dst))
					{
						if (!IFileManager::Get().FileExists(*Source) || IFileManager::Get().Copy(*Dst, *Source) != COPY_OK)
						{
							if (Extension == TextureExtensions.Last())
							{
								UE_LOG(LogPICOSpatial, Warning, TEXT("No valid asset:%s"), *Source);
							}
							continue;
						}
					}

					spatialpack::TextureResource TextureFromStorage = spatialpack::TextureResource::loadFromStorageWithOption(TCHAR_TO_UTF8(*Dst));
					if (TextureFromStorage.isValid())
					{
						TextureResource = &TextureResources.FindOrAdd(Texture, TextureFromStorage);
						UE_LOG(LogPICOSpatial, Log, TEXT("TextureResources Num:%d, UTexture2D:%s, Texture:%hs"), TextureResources.Num(), *Texture, TextureFromStorage.getName());
						break;
					}
				}
			}
		}
	}

	if (TextureResource == nullptr)
	{
		LogNativeResourceLoadFailure(EAssetTypePICO::Texture2D, Texture, MappedTexture, bUsedMapping, bUseOnlyManualMappings);
	}

	return TextureResource;
#endif // PLATFORM_ANDROID
	return nullptr;
}

#if PLATFORM_ANDROID
spatialpack::SceneEntity FPICOSpatial::LoadAsset(FString Name, EAssetTypePICO Type, bool bDirectReturnEntity)
{
	spatialpack::SceneEntity RootEntity;
	RootEntity.setEnabled(false);

	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	const bool bLoadAssetsFromApkAssets = Settings && Settings->bLoadAssetsFromApkAssets;

	const spatialpack::SceneEntityLoadFlag LoadFlag = spatialpack::SceneEntityLoadFlag::DISABLE_MESH_MERGE;

	if (bLoadAssetsFromApkAssets)
	{
		FString AssetUri;
		RootEntity = PICOSpatialLoadSceneEntityFromApkAssets(CurrentScene, Name, LoadFlag, &AssetUri);

		if (!RootEntity.isValid())
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("Failed to load asset from APK assets: %s"), *PICOSpatialGetApkAssetUri(Name));
			return RootEntity;
		}
		else
		{
			UE_LOG(LogPICOSpatial, Verbose, TEXT("Loaded asset from APK assets: %s"), *AssetUri);
			if (bDirectReturnEntity)
			{
				return RootEntity;
			}
		}
	}
	
	extern FString GFilePathBase;
	FString AssetDstBase = GFilePathBase + FString("/UnrealGame/")
		+ (!FApp::IsProjectNameEmpty() ? FApp::GetProjectName() : FPlatformProcess::ExecutableName())
		+ FString("/")
		+ (!FApp::IsProjectNameEmpty() ? FApp::GetProjectName() : FPlatformProcess::ExecutableName())
		+ FString("/Saved/AssetsPICO") + Name;

	FString AssetDst = "";

	PICOSpatialTrySceneAssetCandidates(AssetDstBase, [&](const FString& CandidatePath) -> bool
	{
		if (IFileManager::Get().FileExists(*CandidatePath))
		{
			AssetDst = CandidatePath;
			return true;
		}

		return false;
	});

	if (AssetDst.IsEmpty())
	{
		if (!RootEntity.isValid())
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("No valid asset found for: %s"), *Name);
		}
		return RootEntity;
	}

	if (!RootEntity.isValid())
	{
		RootEntity = spatialpack::SceneEntity::loadFromStorage(CurrentScene, TCHAR_TO_UTF8(*AssetDst), LoadFlag);
	}
	
	if (bDirectReturnEntity)
	{
		return RootEntity;
	}

	spatialpack::SceneEntity Entity;
	
	if (!RootEntity.isValid())
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("Failed load asset:%s"), *Name);
	}
	else
	{
		Entity = FindFirstEntityWithModelComponent(RootEntity);

		if (!Entity.isValid())
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("No entity with ModelComponent found in asset:%s"), *Name);
		}
	}

	return Entity;
}
#endif

bool FPICOSpatial::EnableDefaultWindowInput(bool bEnable)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_ToggleVolumeSpaceWindowInput", "(Z)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, bEnable);
		return true;
	}
#endif
	return false;
}

bool FPICOSpatial::ModifyDefaultWindow(int32 Alpha, bool bCustomSize, int32 Width, int32 Height, int32 OffsetX, int32 OffsetY)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_ModifyWindow", "(ZIZIIII)V", false);
		const bool bTransparent = Alpha < 100;
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, bTransparent, Alpha, bCustomSize, Width, Height, OffsetX, OffsetY);
		return true;
	}
#endif
	return false;
}

void FPICOSpatial::AdvanceFrameTo(EFramePhase TargetPhase)
{
#if PLATFORM_ANDROID
	if (!Session)
	{
		return;
	}

	// Helper lambdas that perform a single SDK call and advance the tracked phase.
	// Using lambdas keeps the auto-advance logic DRY without polluting the class interface.
	auto DoWait = [this]()
	{
		if (FramePhase != EFramePhase::Idle)
		{
			UE_LOG(LogPICOSpatial, Warning,
				TEXT("AdvanceFrameTo: waitFrame called out-of-order (phase=%d), skipping"),
				(int32)FramePhase);
			return;
		}
		{
			SCOPED_NAMED_EVENT(WaitFramePICO, FColor::Red);
			Session->waitFrame(true);
		}
		FramePhase = EFramePhase::Waited;
	};

	auto DoBegin = [this]()
	{
		if (FramePhase != EFramePhase::Waited)
		{
			UE_LOG(LogPICOSpatial, Warning,
				TEXT("AdvanceFrameTo: beginFrame called out-of-order (phase=%d), skipping"),
				(int32)FramePhase);
			return;
		}
		{
			SCOPED_NAMED_EVENT(BeginFramePICO, FColor::Red);
			Session->beginFrame();
		}
		FramePhase = EFramePhase::Begun;
	};

	auto DoEnd = [this]()
	{
		if (FramePhase != EFramePhase::Begun)
		{
			UE_LOG(LogPICOSpatial, Warning,
				TEXT("AdvanceFrameTo: endFrame called out-of-order (phase=%d), skipping"),
				(int32)FramePhase);
			return;
		}
		{
			SCOPED_NAMED_EVENT(EndFramePICO, FColor::Red);
			Session->endFrame();
		}
		FramePhase = EFramePhase::Idle;
	};

	// Auto-advance through any skipped phases to reach TargetPhase.
	switch (TargetPhase)
	{
	case EFramePhase::Waited:
		DoWait();
		break;

	case EFramePhase::Begun:
		if (FramePhase == EFramePhase::Idle)   { DoWait(); }
		DoBegin();
		break;

	case EFramePhase::Idle: // caller wants endFrame
		if (FramePhase == EFramePhase::Idle)   { DoWait(); DoBegin(); }
		if (FramePhase == EFramePhase::Waited) { DoBegin(); }
		DoEnd();
		break;
	}
#endif
}

void FPICOSpatial::OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaTime)
{
	if (World == nullptr || !bSessionStarted)
	{
		return;
	}

	// Only run on full game ticks; skip LEVELTICK_TimeOnly (pause) and editor preview ticks
	if (TickType != LEVELTICK_All && TickType != LEVELTICK_ViewportsOnly)
	{
		return;
	}

	if (bPICOSpatialRunning)
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("Spatial OnWorldTickStart"));

		SCOPED_NAMED_EVENT(OnWorldTickStartPICO, FColor::Red);

		// Advance frame state machine to Begun (auto-calls waitFrame then beginFrame)
		AdvanceFrameTo(EFramePhase::Begun);

		//poll and process input event
		{
			SCOPED_NAMED_EVENT(PollAndProcessInputEventPICO, FColor::Red);
			PollAndProcessInputEvent();
		}
	}
}

void FPICOSpatial::OnWorldTickEnd(UWorld* InWorld, ELevelTick InTickType, float InDeltaSeconds)
{
	if (InWorld == nullptr || !bSessionStarted)
	{
		return;
	}

	// Mirror the tick-type guard in OnWorldTickStart to keep wait/begin/endFrame paired
	if (InTickType != LEVELTICK_All && InTickType != LEVELTICK_ViewportsOnly)
	{
		return;
	}

	if (bPICOSpatialRunning)
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("Spatial OnWorldTickEnd"));

		SCOPED_NAMED_EVENT(OnWorldTickEndPICO, FColor::Red);

		// InWorld is already the world we want; GetWorldContextFromWorldChecked would just
		// perform a linear search through all world contexts and return InWorld again.
		UWorld* World = InWorld;

		// Check if we need to update entity proxies
		const bool bShouldUpdateProxies = (Containers.Num() > 0);
		
		if (bShouldUpdateProxies)
		{
			SCOPED_NAMED_EVENT(UpdateEntityProxiesPICO, FColor::Red);
			
			const TArray<ULevel*>& Levels = World->GetLevels();
			if (Levels.Num() > 0)
			{
				// Clear entity proxies from current frame before processing to avoid accumulation
				// Without this, entries from previous frames would persist, causing incorrect cleanup
				CurrentFrameLivingEntityProxies.Reset();
				// Pre-allocate approximate capacity based on previous frame's entity count
				CurrentFrameLivingEntityProxies.Reserve(PreviousFrameLivingEntityProxies.Num() + 128);

				TMap<AActor*, TInlineComponentArray<USceneComponent*>> ActorComponentsThisFrame;
				auto GetCachedSceneComponents = [&ActorComponentsThisFrame](AActor* Actor) -> const TInlineComponentArray<USceneComponent*>&
				{
					if (const TInlineComponentArray<USceneComponent*>* CachedComponents = ActorComponentsThisFrame.Find(Actor))
					{
						return *CachedComponents;
					}

					TInlineComponentArray<USceneComponent*>& Components = ActorComponentsThisFrame.Add(Actor);
					Actor->GetComponents(Components);
					return Components;
				};
				
				auto MarkContainerPreviousProxiesAlive = [this](uint32 ContainerRootID)
				{
					const uint64 RootIDBits = static_cast<uint64>(ContainerRootID);
					for (const TPair<FEntityProxyKey, EEntityProxyType>& PreviousProxy : PreviousFrameLivingEntityProxies)
					{
						if ((PreviousProxy.Key & 0xffffffffull) == RootIDBits)
						{
							CurrentFrameLivingEntityProxies.Add(PreviousProxy.Key, PreviousProxy.Value);
						}
					}
				};

				// Iterate through each spatial container first
				for (APICOSpatialContainer* Container : Containers)
				{
					if (Container == nullptr || !Container->bBindedToSpatialContainer)
					{
						continue;
					}

					FSceneEntityProxy* RootEntityFound = Container->SpatialContainerRootEntityProxy.Get();
					if (RootEntityFound == nullptr || Container->Root3dComponent == nullptr)
					{
						continue;
					}

					const uint32 ContainerRootID = Container->Root3dComponent->GetUniqueID();

					// If the container has been closed (not just backgrounded), skip it so that
					// its proxies are absent from CurrentFrameLivingEntityProxies and will be
					// destroyed at the end of this tick by the dead-proxy cleanup pass.
					if (!Container->bIsSpatialContainerOpening || Container->bBackgroundTimeout)
					{
						continue;
					}

					// Background containers do not scan actors, refresh proxy data, or update
					// native entities. The CVar only controls whether previous proxies are kept
					// alive while backgrounded.
					if (!Container->bHasEnterForeground)
					{
						if (PICOSpatialKeepBackgroundContainerEntityProxiesAlive())
						{
							MarkContainerPreviousProxiesAlive(ContainerRootID);
						}
						continue;
					}

					// Cache container-specific data once
					const ESpatialContainerTypePICO ContainerType = Container->SpatialContainerTypePICO;

					// Cache container bounds for bounded type once (not every frame)
					// For Bounded containers, bounds only change when container transform changes
					FBox ContainerBounds;
					const bool bIsBounded = (ContainerType == ESpatialContainerTypePICO::Bounded);
					if (bIsBounded && Container->GetRootComponent())
					{
						ContainerBounds = Container->GetRootComponent()->Bounds.GetBox();
					}

					// Check if using manual actor list
					const bool bUsingManualList = Container->bUseManualActorList;

					// Process actors based on mode
					if (bUsingManualList)
					{
						// Manual mode: Process only actors in the list
						// Early exit if list is empty
						if (Container->ManualActorList.Num() == 0)
						{
							continue;
						}

						for (const TObjectPtr<AActor>& ActorPtr : Container->ManualActorList)
						{
							AActor* Actor = ActorPtr.Get();
								
							// Comprehensive validation to skip invalid actors
							// OnDestroyed event handles automatic cleanup, but we need immediate skip for safety
							if (Actor == nullptr || !IsValid(Actor) || 
							    Actor->IsPendingKillPending() || 
							    Actor->IsUnreachable() || 
							    Actor->IsEditorOnly())
							{
								continue;
							}
								
							// Skip if actor's world doesn't match (safety check)
							if (Actor->GetWorld() != World)
							{
								continue;
							}
								
							// Skip container actors to avoid self-processing
							if (Actor->IsA(APICOSpatialContainer::StaticClass()))
							{
								continue;
							}

							// Process each component
							ProcessComponentsForContainer(this, Container, GetCachedSceneComponents(Actor), RootEntityFound, ContainerRootID, bIsBounded, ContainerBounds);
						}
					}
					else
					{
						// Automatic mode: Process all actors from levels
						for (const ULevel* Level : Levels)
						{
							// Skip invalid or non-current instanced levels
							if (Level == nullptr || (Level->IsInstancedLevel() && !Level->IsCurrentLevel()))
							{
								continue;
							}

							// Cache actors array reference to avoid repeated TArray copies
							const TArray<AActor*>& LevelActors = Level->Actors;
							
							// Early exit if no actors in this level
							if (LevelActors.Num() == 0)
							{
								continue;
							}
							
							for (AActor* Actor : LevelActors)
							{
								// Early validation to skip invalid actors
								if (Actor == nullptr || !IsValid(Actor) || 
								    Actor->IsPendingKillPending() || 
								    Actor->IsUnreachable() || 
								    Actor->IsEditorOnly() || 
								    Actor->IsA(APICOSpatialContainer::StaticClass()))
								{
									continue;
								}

								// Skip if actor's world doesn't match
								// This prevents processing actors from other worlds (e.g., PIE vs Editor world)
								if (Actor->GetWorld() != World)
								{
									continue;
								}

								// Process each component
								ProcessComponentsForContainer(this, Container, GetCachedSceneComponents(Actor), RootEntityFound, ContainerRootID, bIsBounded, ContainerBounds);
							}
						}
					}
				}
			}
		}

		// Remove dead entity proxies - entities that were alive in previous frame but not in current frame
		for (auto It = PreviousFrameLivingEntityProxies.CreateIterator(); It; ++It)
		{
			if (!CurrentFrameLivingEntityProxies.Contains(It->Key))
			{
				DestroyEntityProxy(It->Key, It->Value);
			}
		}

		// Swap: current frame becomes previous frame for next tick
		PreviousFrameLivingEntityProxies.Reset();
		Swap(PreviousFrameLivingEntityProxies, CurrentFrameLivingEntityProxies);

		// Advance frame state machine to Idle (calls endFrame)
		AdvanceFrameTo(EFramePhase::Idle);
	}

	// Manage spatial running state
	if (!bPICOSpatialRunning && bPICOSpatialEnabled && Containers.Num() > 0)
	{
		bPICOSpatialRunning = true;
	}
	else if (bPICOSpatialRunning && (!bPICOSpatialEnabled || Containers.Num() == 0))
	{
		bPICOSpatialRunning = false;

		// Wrap native entity destruction in a valid frame boundary.
		// AdvanceFrameTo auto-fills any skipped phases so this is safe regardless
		// of whether OnWorldTickStart ran this tick or not.
		AdvanceFrameTo(EFramePhase::Begun);

		if (!bPICOSpatialEnabled && Containers.Num() > 0)
		{
			TSet<APICOSpatialContainer*> OpenedContainers = Containers; //make a copy to avoid modification during iteration
			for (APICOSpatialContainer* Container : OpenedContainers)
			{
				if (Container && Container->IsValidLowLevel())
				{
					Container->CloseSpatialContainerPICO();
				}
			}
		}

		if (!PreviousFrameLivingEntityProxies.IsEmpty())
		{

			for (auto It = PreviousFrameLivingEntityProxies.CreateIterator(); It; ++It)
			{
				DestroyEntityProxy(It->Key, It->Value);
			}

			PreviousFrameLivingEntityProxies.Reset();
			CurrentFrameLivingEntityProxies.Reset();
		}

		AdvanceFrameTo(EFramePhase::Idle);
	}
}

void FPICOSpatial::OnPreLoadMap(const FString& MapName)
{
	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] OnPreLoadMap map=%s Enabled=%d Running=%d SessionStarted=%d Session=%p Containers=%d"), *MapName, bPICOSpatialEnabled, bPICOSpatialRunning, bSessionStarted, Session, Containers.Num());

	// Ensure native SDK operations below are enclosed in a valid frame boundary.
	// AdvanceFrameTo auto-fills any skipped phases (wait -> begin).
	AdvanceFrameTo(EFramePhase::Begun);

	// Close all opened containers first so the SDK-side container state is
	// properly torn down before we destroy the UE-side entity proxies.
	// Actors are still valid at PreLoadMap time, so CloseSpatialContainerPICO()
	// can be called safely here.
	if (Containers.Num() > 0)
	{
		TSet<APICOSpatialContainer*> OpenedContainers = Containers;
		for (APICOSpatialContainer* Container : OpenedContainers)
		{
			if (Container && Container->IsValidLowLevel() && Container->bIsSpatialContainerOpening)
			{
				Container->CloseSpatialContainerPICO();
			}
		}
	}

	DestroyAllEntityProxies();
	Containers.Empty();

#if PLATFORM_ANDROID
	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	const bool bLoadAssetsFromApkAssets = Settings && Settings->bLoadAssetsFromApkAssets;
	EnsureRuntimeAssetBundlesLoaded();

	static bool bDecompressedAssetsPICO = false;
	if (!bDecompressedAssetsPICO)
	{
		bDecompressedAssetsPICO = DecompressAssetsPICO();
	}

	if (Settings && Settings->bPreCacheAssetsPerLevel && bPICOSpatialEnabled)
	{
		const FString TargetWorldPackageName = PICOSpatialNormalizeWorldPackageNameForDependencies(MapName);
		if (TargetWorldPackageName.IsEmpty())
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("Skip level pre-cache because map name could not be normalized: %s"), *MapName);
		}
		else
		{
			UPICOSpatialWorldDependenciesAsset* WorldDependenciesAsset =
				LoadObject<UPICOSpatialWorldDependenciesAsset>(nullptr, GPICOSpatialWorldDependenciesAssetReference);
			if (!WorldDependenciesAsset)
			{
				UE_LOG(LogPICOSpatial, Warning, TEXT("Failed to load world dependencies asset: %s"), GPICOSpatialWorldDependenciesAssetReference);
			}

			const FWorldDependenciesPICO* WorldDependencies = WorldDependenciesAsset
				? WorldDependenciesAsset->WorldDependencies.Find(FName(*TargetWorldPackageName))
				: nullptr;

			PreCacheResources(WorldDependencies ? WorldDependencies->Dependencies : TMap<FString, EAssetTypePICO>());
		}
	}
	else if (Settings && Settings->bPreCacheAssetsPerLevel)
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("Skip level pre-cache on pre load because PICOSpatial is disabled."));
	}
#endif

	// Close the frame opened at the start of this function.
	AdvanceFrameTo(EFramePhase::Idle);
}

void FPICOSpatial::OnPostLoadMap(UWorld* LoadedWorld)
{
	if (LoadedWorld == nullptr)
	{
		UE_LOG(LogPICOSpatial, Error, TEXT("OnPostLoadMap NONE!"));
		return;
	}

	UE_LOG(LogPICOSpatial, Warning, TEXT("[Lifecycle] OnPostLoadMap world=%s Enabled=%d Running=%d SessionStarted=%d Session=%p Containers=%d"), *LoadedWorld->GetFName().ToString(), bPICOSpatialEnabled, bPICOSpatialRunning, bSessionStarted, Session, Containers.Num());

	// Ensure native SDK operations below are enclosed in a valid frame boundary.
	AdvanceFrameTo(EFramePhase::Begun);

	// Close the frame opened at the start of this function.
	AdvanceFrameTo(EFramePhase::Idle);
}

void FPICOSpatial::HandleStaticMeshComponentInstanceIndexUpdated(UInstancedStaticMeshComponent* InComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates)
{
	if (InComponent)
	{
		InComponent->UpdateBounds();
		for (auto Container : Containers)
		{
			if (Container == nullptr || Container->Root3dComponent == nullptr)
			{
				continue;
			}
			const FEntityProxyKey ID = FPICOSpatial::MakeEntityProxyKey(InComponent->GetUniqueID(), Container->Root3dComponent->GetUniqueID());
			EEntityProxyType Type = GetEntityProxyType(InComponent);
			FInstancedStaticMeshEntityProxy* EntityFound = static_cast<FInstancedStaticMeshEntityProxy*>(FindEntityProxy(ID, Type));
			if (EntityFound)
			{
				EntityFound->bNeedReConstructInstances = true;
			}
		}
	}
}

// Helper function to process components for a container
static void ProcessComponentsForContainer(
	FPICOSpatial* PICOSpatialInstance,
	APICOSpatialContainer* Container,
	const TInlineComponentArray<USceneComponent*>& Components,
	FSceneEntityProxy* RootEntityFound,
	uint32 ContainerRootID,
	bool bIsBounded,
	const FBox& ContainerBounds)
{
	// Validate container is still valid before and during processing.
	// IsValid() covers nullptr + IsPendingKillPending() + IsUnreachable().
	if (!IsValid(Container))
	{
		return;
	}

	// Early exit if no components to process
	if (Components.Num() == 0)
	{
		return;
	}

	// Process each component for this container
	for (USceneComponent* SceneComponent : Components)
	{
		// IsValid() covers nullptr, PendingKill and Unreachable in one call
		if (!IsValid(SceneComponent) || SceneComponent->IsEditorOnly())
		{
			continue;
		}

		// Verify component's owner is still valid
		AActor* ComponentOwner = SceneComponent->GetOwner();
		if (!IsValid(ComponentOwner))
		{
			continue;
		}

		// Determine if component should create entity proxy
		UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(SceneComponent);
		bool bIsUsedToCreateEntityProxy = false;

		if (PrimComp)
		{
			if (const UCustomSceneEntityComponent* CustomComp = Cast<UCustomSceneEntityComponent>(SceneComponent))
			{
				// Cast is required here because we need the pointer to read ECSEntity
				bIsUsedToCreateEntityProxy = (CustomComp->ECSEntity != nullptr);
			}
			else
			{
				bIsUsedToCreateEntityProxy = PrimComp->IsVisible() || SceneComponent->ComponentHasTag("EnableInputPICO");
			}

			// Skip out-of-bounds components early to avoid expensive proxy operations
			if (bIsUsedToCreateEntityProxy && bIsBounded)
			{
				if (!ContainerBounds.Intersect(PrimComp->Bounds.GetBox()))
				{
					continue;
				}
			}
		}
		else
		{
			// Use IsA<T>() instead of Cast<T>() for type-only checks: no pointer
			// adjustment needed, just a class hierarchy walk - cheaper than dynamic_cast.
			if (SceneComponent->IsA<UPICOSpatialGlbUsdViewer>() ||
				SceneComponent->IsA<UAudioComponent>()            ||
				SceneComponent->IsA<USceneCaptureComponent2D>()   ||
				SceneComponent->IsA<UDirectionalLightComponent>())
			{
				bIsUsedToCreateEntityProxy = true;
			}
		}

		if (!bIsUsedToCreateEntityProxy)
		{
			continue;
		}

		// Re-validate container: it could have become invalid during iteration
		if (!IsValid(Container))
		{
			return;
		}

		// Create or update entity proxy
		const FEntityProxyKey ID = FPICOSpatial::MakeEntityProxyKey(SceneComponent->GetUniqueID(), ContainerRootID);
		const EEntityProxyType Type = GetEntityProxyType(SceneComponent);
		FSceneEntityProxy* EntityFound = PICOSpatialInstance->FindEntityProxy(ID, Type);

		if (EntityFound == nullptr)
		{
			if (Type == EEntityProxyType::CustomSceneEntity)
			{
				UE_LOG(LogPICOSpatial, Verbose, TEXT("[ECS] Creating CustomSceneEntityProxy. Component=%s Owner=%s Container=%s"),
					*SceneComponent->GetName(),
					ComponentOwner ? *ComponentOwner->GetName() : TEXT("<null>"),
					Container ? *Container->GetName() : TEXT("<null>"));
			}
			EntityFound = PICOSpatialInstance->CreateEntityProxy(RootEntityFound, SceneComponent, Type);
		}
		else
		{
			EntityFound->UpdateEntityProxy(SceneComponent);
		}

		if (EntityFound != nullptr)
		{
			auto* NativeEntityPtr = EntityFound->NativeEntity.Get();
			bool bProxyReady = NativeEntityPtr != nullptr && NativeEntityPtr->IsValid();

			// NativeEntity may be null before first initialization
			if (!bProxyReady)
			{
				bProxyReady = EntityFound->InitializeNativeEntity(PICOSpatialInstance->CurrentScene, Container->SpatialContainer);
				NativeEntityPtr = EntityFound->NativeEntity.Get();
				bProxyReady = bProxyReady && NativeEntityPtr != nullptr && NativeEntityPtr->IsValid();
			}

			if (bProxyReady)
			{
				EntityFound->NotifyProxyCreatedIfNeeded();
				EntityFound->UpdateNativeEntity();
				PICOSpatialInstance->CurrentFrameLivingEntityProxies.Add(ID, Type);
			}
		}
	}
}

FSceneEntityProxy* FPICOSpatial::CreateEntityProxy(FSceneEntityProxy* RootSceneEntityProxy, USceneComponent* SceneComponent, const EEntityProxyType& Type)
{
	check(IsInGameThread());
	const FEntityProxyKey ID = MakeEntityProxyKey(SceneComponent->GetUniqueID(), RootSceneEntityProxy->Owner->GetUniqueID());
	FSceneEntityProxy* EntityProxy = nullptr;
	switch (Type)
	{
	case EEntityProxyType::SceneEntity:
	{
		EntityProxy = SceneEntityProxies_GameThread.Add(ID, MakeShareable(new FSceneEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::CustomSceneEntity:
	{
		EntityProxy = CustomSceneEntityProxies_GameThread.Add(ID, MakeShareable(new FCustomSceneEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::StaticMeshEntity:
	{
		EntityProxy = StaticMeshEntityProxies_GameThread.Add(ID, MakeShareable(new FStaticMeshEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::ShapeEntity:
	{
		EntityProxy = ShapeEntityProxies_GameThread.Add(ID, MakeShareable(new FShapeEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::InstancedStaticMeshEntity:
	{
		EntityProxy = InstancedStaticMeshEntityProxies_GameThread.Add(ID, MakeShareable(new FInstancedStaticMeshEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::SkeletalMeshEntity:
	{
		EntityProxy = SkeletalMeshEntityProxies_GameThread.Add(ID, MakeShareable(new FSkeletalMeshEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::DynamicMeshEntity:
	{
		EntityProxy = DynamicMeshEntityProxies_GameThread.Add(ID, MakeShareable(new FDynamicMeshEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::TextEntity:
	{
		EntityProxy = TextEntityProxies_GameThread.Add(ID, MakeShareable(new FTextEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::WidgetEntity:
	{
		EntityProxy = WidgetProxies_GameThread.Add(ID, MakeShareable(new FWidgetEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::SceneCapture2dEntity:
	{
		EntityProxy = SceneCaptureProxies_GameThread.Add(ID, MakeShareable(new FSceneCaptureEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::ParticleEntity:
	{
		EntityProxy = ParticleProxies_GameThread.Add(ID, MakeShareable(new FParticleEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::DirectionalLightEntity:
	{
		EntityProxy = DirectionalLightEntityProxies_GameThread.Add(ID, MakeShareable(new FDirectionalLightEntityProxy(RootSceneEntityProxy, SceneComponent))).Get();
		break;
	}
	case EEntityProxyType::Unknown:
		break;
	default:
		break;
	}
	return EntityProxy;
}

void FPICOSpatial::DestroyEntityProxy(FEntityProxyKey ID, const EEntityProxyType& Type)
{
	switch (Type)
	{
	case EEntityProxyType::SceneEntity:
		SceneEntityProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::CustomSceneEntity:
		CustomSceneEntityProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::StaticMeshEntity:
		StaticMeshEntityProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::ShapeEntity:
		ShapeEntityProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::InstancedStaticMeshEntity:
		InstancedStaticMeshEntityProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::SkeletalMeshEntity:
		SkeletalMeshEntityProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::DynamicMeshEntity:
		DynamicMeshEntityProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::TextEntity:
		TextEntityProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::WidgetEntity:
		WidgetProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::SceneCapture2dEntity:
		SceneCaptureProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::ParticleEntity:
		ParticleProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::DirectionalLightEntity:
		DirectionalLightEntityProxies_GameThread.Remove(ID);
		break;
	case EEntityProxyType::Unknown:
		break;
	default:
		break;
	}
}

void FPICOSpatial::DestroyAllEntityProxies()
{
	PreviousFrameLivingEntityProxies.Reset();
	CurrentFrameLivingEntityProxies.Reset();
	SceneEntityProxies_GameThread.Reset();
	CustomSceneEntityProxies_GameThread.Reset();
	StaticMeshEntityProxies_GameThread.Reset();
	ShapeEntityProxies_GameThread.Reset();
	InstancedStaticMeshEntityProxies_GameThread.Reset();
	SkeletalMeshEntityProxies_GameThread.Reset();
	DynamicMeshEntityProxies_GameThread.Reset();
	TextEntityProxies_GameThread.Reset();
	WidgetProxies_GameThread.Reset();
	SceneCaptureProxies_GameThread.Reset();
	ParticleProxies_GameThread.Reset();
	DirectionalLightEntityProxies_GameThread.Reset();
}

FSceneEntityProxy* FPICOSpatial::FindEntityProxy(FEntityProxyKey ID, const EEntityProxyType& Type)
{
	switch (Type)
	{
	case EEntityProxyType::SceneEntity:
	{
		TSharedPtr<FSceneEntityProxy>* EntityFound = SceneEntityProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}
	case EEntityProxyType::CustomSceneEntity:
	{
		TSharedPtr<FCustomSceneEntityProxy>* EntityFound = CustomSceneEntityProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}
	case EEntityProxyType::StaticMeshEntity:
	{
		TSharedPtr<FStaticMeshEntityProxy>* EntityFound = StaticMeshEntityProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}
	case EEntityProxyType::ShapeEntity:
	{
		TSharedPtr<FShapeEntityProxy>* EntityFound = ShapeEntityProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}
	case EEntityProxyType::InstancedStaticMeshEntity:
	{
		TSharedPtr<FInstancedStaticMeshEntityProxy>* EntityFound = InstancedStaticMeshEntityProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}
	case EEntityProxyType::SkeletalMeshEntity:
	{
		TSharedPtr<FSkeletalMeshEntityProxy>* EntityFound = SkeletalMeshEntityProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}
	case EEntityProxyType::DynamicMeshEntity:
	{
		TSharedPtr<FDynamicMeshEntityProxy>* EntityFound = DynamicMeshEntityProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}
	case EEntityProxyType::TextEntity:
	{
		TSharedPtr<FTextEntityProxy>* EntityFound = TextEntityProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}
	case EEntityProxyType::WidgetEntity:
	{
		TSharedPtr<FWidgetEntityProxy>* EntityFound = WidgetProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}	
	case EEntityProxyType::SceneCapture2dEntity:
	{
		TSharedPtr<FSceneCaptureEntityProxy>* EntityFound = SceneCaptureProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}
	case EEntityProxyType::ParticleEntity:
	{
		TSharedPtr<FParticleEntityProxy>* EntityFound = ParticleProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}
	case EEntityProxyType::DirectionalLightEntity:
	{
		TSharedPtr<FDirectionalLightEntityProxy>* EntityFound = DirectionalLightEntityProxies_GameThread.Find(ID);
		return EntityFound == nullptr ? nullptr : EntityFound->Get();
		break;
	}
	case EEntityProxyType::Unknown:
		break;
	default:
		return nullptr;
		break;
	}
	return nullptr;
}

void FPICOSpatial::PollAndProcessInputEvent()
{
#if PLATFORM_ANDROID
	// Early exit if scene is invalid
	if (CurrentScene == nullptr)
	{
		return;
	}

	// Cache event manager pointer to avoid repeated function calls
	UEventManagerPICO* EventManager = UPICOSpatialFunctionLibrary::GetEventManagerPICO();
	if (EventManager == nullptr)
	{
		return;
	}

	const spatialpack::ScenePullEventCollection& EventCollection = CurrentScene->pullAllEvents();
	const int32 EventCount = EventCollection.getCount();

	// Early exit if no events to process
	if (EventCount == 0)
	{
		return;
	}

	// Lambda to get scene entity proxy and primitive component with validation
	auto GetEntityProxyAndComponent = [this](spatialpack::EntityUUID uuid, FSceneEntityProxy*& OutProxy, UPrimitiveComponent*& OutComponent) -> bool
	{
		FSceneEntityProxy** SceneEntityProxyFound = NativeEntityToSceneComponentMap.Find(uuid);
		if (SceneEntityProxyFound == nullptr || *SceneEntityProxyFound == nullptr)
		{
			return false;
		}
		
		OutProxy = *SceneEntityProxyFound;
		OutComponent = Cast<UPrimitiveComponent>(OutProxy->Owner);
		return OutComponent != nullptr;
	};

	// Process each event
	for (int32 i = 0; i < EventCount; ++i)
	{
		const spatialpack::ScenePullEvent& Event = EventCollection.getScenePullEvent(i);

		// Skip invalid events
		if (Event.eventData == nullptr)
		{
			continue;
		}

		switch (Event.type)
		{
		case spatialpack::ScenePullEventType::HOVER:
		{
			// eventData is guaranteed non-null by the check above; static_cast is safe
			const spatialpack::HoverEvent* hoverEvent = static_cast<const spatialpack::HoverEvent*>(Event.eventData);

			FSceneEntityProxy* OutProxy = nullptr;
			UPrimitiveComponent* PrimitiveComponent = nullptr;
			if (!GetEntityProxyAndComponent(hoverEvent->uuid, OutProxy, PrimitiveComponent))
			{
				break;
			}

			const FVector Location = ToFVector(hoverEvent->location3d, 100.0f);
			const EInputActionPICO HoverAction = ConvertHoverAction(hoverEvent->hoverAction);

			// Dispatch mouse-over edge events only on transition frames
			if (HoverAction == EInputActionPICO::Started)
			{
				PrimitiveComponent->DispatchMouseOverEvents(nullptr, PrimitiveComponent);
			}
			else if (HoverAction == EInputActionPICO::Completed)
			{
				PrimitiveComponent->DispatchMouseOverEvents(PrimitiveComponent, nullptr);
				EventManager->OnEndHover.Broadcast(PrimitiveComponent);
			}
			EventManager->OnHover.Broadcast(PrimitiveComponent, HoverAction, Location);
			break;
		}

		case spatialpack::ScenePullEventType::TAP:
		{
			const spatialpack::TapEvent* tapEvent = static_cast<const spatialpack::TapEvent*>(Event.eventData);

			FSceneEntityProxy* OutProxy = nullptr;
			UPrimitiveComponent* PrimitiveComponent = nullptr;
			if (!GetEntityProxyAndComponent(tapEvent->uuid, OutProxy, PrimitiveComponent))
			{
				break;
			}

			const FVector Location = ToFVector(tapEvent->location3d, 100.0f);

			// Dispatch all tap events
			PrimitiveComponent->DispatchOnClicked();
			PrimitiveComponent->DispatchOnReleased();
			EventManager->OnPressed.Broadcast(PrimitiveComponent, Location);
			EventManager->OnReleased.Broadcast(PrimitiveComponent, Location);
			EventManager->OnClicked.Broadcast(PrimitiveComponent, Location);
			break;
		}

		case spatialpack::ScenePullEventType::LONG_PRESS:
		{
			const spatialpack::LongPressEvent* longPressEvent = static_cast<const spatialpack::LongPressEvent*>(Event.eventData);

			FSceneEntityProxy* OutProxy = nullptr;
			UPrimitiveComponent* PrimitiveComponent = nullptr;
			if (!GetEntityProxyAndComponent(longPressEvent->uuid, OutProxy, PrimitiveComponent))
			{
				break;
			}

			const FVector Location = ToFVector(longPressEvent->location3d, 100.0f);
			const EInputActionPICO LongPressAction = ConvertLongPressAction(longPressEvent->longPressAction);

			switch (LongPressAction)
			{
			case EInputActionPICO::Started:
				PrimitiveComponent->DispatchOnClicked();
				EventManager->OnPressed.Broadcast(PrimitiveComponent, Location);
				break;
			case EInputActionPICO::Completed:
				PrimitiveComponent->DispatchOnReleased();
				EventManager->OnReleased.Broadcast(PrimitiveComponent, Location);
				EventManager->OnClicked.Broadcast(PrimitiveComponent, Location);
				break;
			default:
				break;
			}
			break;
		}

		case spatialpack::ScenePullEventType::DRAG:
		{
			const spatialpack::DragEvent* dragEvent = static_cast<const spatialpack::DragEvent*>(Event.eventData);

			FSceneEntityProxy* OutProxy = nullptr;
			UPrimitiveComponent* PrimitiveComponent = nullptr;
			if (!GetEntityProxyAndComponent(dragEvent->uuid, OutProxy, PrimitiveComponent))
			{
				break;
			}

			const FVector StartLocation = ToFVector(dragEvent->startLocation3d, 100.0f);
			const FVector Location = ToFVector(dragEvent->location3d, 100.0f);
			const EInputActionPICO DragAction = ConvertDragAction(dragEvent->dragAction);

			// Broadcast drag event
			EventManager->OnDrag.Broadcast(PrimitiveComponent, DragAction, StartLocation, Location);

			// Dispatch touch events based on action
			switch (DragAction)
			{
			case EInputActionPICO::Started:
				PrimitiveComponent->DispatchOnInputTouchBegin(ETouchIndex::Type::CursorPointerIndex);
				break;
			case EInputActionPICO::Canceled:
			case EInputActionPICO::Completed:
				PrimitiveComponent->DispatchOnInputTouchEnd(ETouchIndex::Type::CursorPointerIndex);
				break;
			default:
				break;
			}
			break;
		}

		default:
			break;
		}
	}
#endif
}

void FPICOSpatial::CopyTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture* SrcTexture, FIntRect SrcRect, FRHITexture* DstTexture, FIntRect DstRect, bool bClearBlack, ERenderTargetActions RTAction, ERHIAccess FinalDstAccess, ETextureCopyBlendModifier SrcTextureCopyModifier) const
{
	check(IsInRenderingThread());

	const uint32 ViewportWidth = DstRect.Width();
	const uint32 ViewportHeight = DstRect.Height();
	const FIntPoint TargetSize(ViewportWidth, ViewportHeight);

	const float SrcTextureWidth = SrcTexture->GetSizeX();
	const float SrcTextureHeight = SrcTexture->GetSizeY();
	float U = 0.f, V = 0.f, USize = 1.f, VSize = 1.f;
	if (SrcRect.IsEmpty())
	{
		SrcRect.Min.X = 0;
		SrcRect.Min.Y = 0;
		SrcRect.Max.X = SrcTextureWidth;
		SrcRect.Max.Y = SrcTextureHeight;
	}
	else
	{
		U = SrcRect.Min.X / SrcTextureWidth;
		V = SrcRect.Min.Y / SrcTextureHeight;
		USize = SrcRect.Width() / SrcTextureWidth;
		VSize = SrcRect.Height() / SrcTextureHeight;
	}

	RHICmdList.Transition(FRHITransitionInfo(DstTexture, ERHIAccess::Unknown, ERHIAccess::RTV));

	FRHITexture* ColorRT = DstTexture->GetTexture2DArray() ? DstTexture->GetTexture2DArray() : DstTexture->GetTexture2D();
	FRHIRenderPassInfo RenderPassInfo(ColorRT, RTAction);
	RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("PICOSpatial_CopyTexture"));
	{
		if (bClearBlack || SrcTextureCopyModifier == ETextureCopyBlendModifier::Opaque)
		{
			const FIntRect ClearRect(0, 0, DstTexture->GetSizeX(), DstTexture->GetSizeY());
			RHICmdList.SetViewport(ClearRect.Min.X, ClearRect.Min.Y, 0, ClearRect.Max.X, ClearRect.Max.Y, 1.0f);

			if (bClearBlack)
			{
				DrawClearQuad(RHICmdList, FLinearColor::Black);
			}
			else
			{
				// For opaque texture copies, we want to make sure alpha is initialized to 1.0f
				DrawClearQuadAlpha(RHICmdList, 1.0f);
			}
		}

		RHICmdList.SetViewport(DstRect.Min.X, DstRect.Min.Y, 0, DstRect.Max.X, DstRect.Max.Y, 1.0f);

		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);

		// We need to differentiate between types of layers: opaque, unpremultiplied alpha (regular texture copy) and premultiplied alpha (emulation texture)
		switch (SrcTextureCopyModifier)
		{
		case ETextureCopyBlendModifier::Opaque:
			GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB>::GetRHI();
			break;
		case ETextureCopyBlendModifier::TransparentAlphaPassthrough:
			GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA>::GetRHI();
			break;
		case ETextureCopyBlendModifier::PremultipliedAlphaBlend:
			// Because StereoLayerRender actually enables alpha blending as it composites the layers into the emulation texture
			// the color values for the emulation swapchain are PREMULTIPLIED ALPHA. That means we don't want to multiply alpha again!
			// So we can just do SourceColor * 1.0f + DestColor (1 - SourceAlpha)
			GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
			break;
		default:
			check(!"Unsupported copy modifier");
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			break;
		}

		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.PrimitiveType = PT_TriangleList;

		FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);

		TShaderMapRef<FScreenVS> VertexShader(ShaderMap);

		TShaderRef<FScreenPS> ScreenPS;

		const bool bIsArraySource = SrcTexture->GetDesc().IsTextureArray();

		if (LIKELY(!bIsArraySource))
		{
			ScreenPS = TShaderMapRef<FScreenPS>(ShaderMap);
		}
		else
		{
			// Texture array source (e.g. multiview): sample from slice 0
			TShaderMapRef<FScreenFromSlice0PS> ScreenPSRef(ShaderMap);
			ScreenPS = ScreenPSRef;
		}

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = ScreenPS.GetPixelShader();

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

		RHICmdList.Transition(FRHITransitionInfo(SrcTexture, ERHIAccess::Unknown, ERHIAccess::SRVMask));

		// ScreenPS is always valid here: both branches of the if/else above assign it
		const bool bSameSize = DstRect.Size() == SrcRect.Size();
		FRHISamplerState* PixelSampler = bSameSize ? TStaticSamplerState<SF_Point>::GetRHI() : TStaticSamplerState<SF_Bilinear>::GetRHI();
		SetShaderParametersLegacyPS(RHICmdList, ScreenPS, PixelSampler, SrcTexture);

		// Cache RendererModule pointer across calls - module registration never changes at runtime
		static const FName RendererModuleName("Renderer");
		static IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);

		if (RendererModule)
		{
			RendererModule->DrawRectangle(
				RHICmdList,
				0, 0,
				ViewportWidth, ViewportHeight,
				U, V,
				USize, VSize,
				TargetSize,
				FIntPoint(1, 1),
				VertexShader,
				EDRF_Default);
		}
	}
	RHICmdList.EndRenderPass();

	RHICmdList.Transition(FRHITransitionInfo(DstTexture, ERHIAccess::RTV, FinalDstAccess));
}

void FPICOSpatial::InitRT(UTextureRenderTarget2D* T)
{
	FTextureRenderTargetResource* RenderTargetResource = T->GameThread_GetRenderTargetResource();
	{
		ENQUEUE_RENDER_COMMAND(FlushDeferredResourceUpdateCommand)(
			[RenderTargetResource](FRHICommandListImmediate& RHICmdList)
			{
				RenderTargetResource->FlushDeferredResourceUpdate(RHICmdList);
			});
		FlushRenderingCommands();

		T->UpdateResourceImmediate(false);

		ENQUEUE_RENDER_COMMAND(ResetSceneTextureExtentHistory)(
			[RenderTargetResource](FRHICommandListImmediate& RHICmdList)
			{
				RenderTargetResource->ResetSceneTextureExtentsHistory();
			});
		FlushRenderingCommands();
	}
}

#if PLATFORM_ANDROID
spatialpack::SceneEntity FPICOSpatial::FindFirstEntityWithModelComponent(const spatialpack::SceneEntity& entity)
{
	// Check if current entity is valid
	if (!entity.isValid())
	{
		return spatialpack::SceneEntity();
	}

	// Check if current entity has ModelComponent
	if (entity.hasComponent(spatialpack::ModelComponent::getType()))
	{
		return entity;
	}

	// Recursively check all children
	spatialpack::SceneEntity child = entity.getFirstChild();
	while (child.isValid())
	{
		// Recursively search in child and its descendants
		spatialpack::SceneEntity found = FindFirstEntityWithModelComponent(child);
		if (found.isValid())
		{
			return found;
		}

		// Move to next sibling
		child = child.getNextSibling();
	}

	// No entity with ModelComponent found in this branch
	return spatialpack::SceneEntity();
}
#endif

FOnWindowContainerCreatedDelegate FPICOSpatial::OnWindowContainerCreatedDelegate;
void FPICOSpatial::WindowContainerOnCreatedCallback(spatialpack::AppFormId Id)
{
	AsyncTask(ENamedThreads::GameThread, [Id]()
		{
			if (FPICOSpatial::OnWindowContainerCreatedDelegate.IsBound())
			{
				FPICOSpatial::OnWindowContainerCreatedDelegate.Broadcast(Id);
			}
		});
}

FOnWindowContainerDestroyedDelegate FPICOSpatial::OnWindowContainerDestroyedDelegate;
void FPICOSpatial::WindowContainerOnDestroyedCallback(spatialpack::AppFormId Id)
{
	AsyncTask(ENamedThreads::GameThread, [Id]()
		{
			if (FPICOSpatial::OnWindowContainerDestroyedDelegate.IsBound())
			{
				FPICOSpatial::OnWindowContainerDestroyedDelegate.Broadcast(Id);
			}
		});
}

FOnWindowContainerForegroundDelegate FPICOSpatial::OnWindowContainerForegroundDelegate;
void FPICOSpatial::WindowContainerOnForegroundCallback(spatialpack::AppFormId Id)
{
	AsyncTask(ENamedThreads::GameThread, [Id]()
		{
			if (FPICOSpatial::OnWindowContainerForegroundDelegate.IsBound())
			{
				FPICOSpatial::OnWindowContainerForegroundDelegate.Broadcast(Id);
			}
		});
}

FOnWindowContainerBackgroundDelegate FPICOSpatial::OnWindowContainerBackgroundDelegate;
void FPICOSpatial::WindowContainerOnBackgroundCallback(spatialpack::AppFormId Id)
{
	AsyncTask(ENamedThreads::GameThread, [Id]()
		{
			if (FPICOSpatial::OnWindowContainerBackgroundDelegate.IsBound())
			{
				FPICOSpatial::OnWindowContainerBackgroundDelegate.Broadcast(Id);
			}
		});
}

FOnWindowContainerFocusDelegate FPICOSpatial::OnWindowContainerFocusDelegate;
void FPICOSpatial::WindowContainerOnFocusCallback(spatialpack::AppFormId Id, bool bHasFocus)
{
	AsyncTask(ENamedThreads::GameThread, [Id, bHasFocus]()
		{
			if (FPICOSpatial::OnWindowContainerFocusDelegate.IsBound())
			{
				FPICOSpatial::OnWindowContainerFocusDelegate.Broadcast(Id, bHasFocus);
			}
		});
}

FOnWindowContainerSightDelegate FPICOSpatial::OnWindowContainerSightDelegate;
void FPICOSpatial::WindowContainerOnSightCallback(spatialpack::AppFormId Id, bool bIsOnSight)
{
	AsyncTask(ENamedThreads::GameThread, [Id, bIsOnSight]()
		{	
			if (FPICOSpatial::OnWindowContainerSightDelegate.IsBound())
			{
				FPICOSpatial::OnWindowContainerSightDelegate.Broadcast(Id, bIsOnSight);
			}
		});
}

bool FPICOSpatial::DecompressAssetsPICO()
{
#if PLATFORM_ANDROID
	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	const bool bLoadAssetsFromApkAssets = Settings && Settings->bLoadAssetsFromApkAssets;
	FString AssetsPicoSourceDirectory = FPaths::ProjectContentDir() / TEXT("AssetsPICO");
	FString MoviesSourceDirectory = FPaths::ProjectContentDir() / TEXT("Movies");

	// Build destination directory path (directly to Saved, not Saved/AssetsPICO)
	extern FString GFilePathBase;
	FString SavedDirectory = GFilePathBase + FString("/UnrealGame/")
		+ (!FApp::IsProjectNameEmpty() ? FApp::GetProjectName() : FPlatformProcess::ExecutableName())
		+ FString("/")
		+ (!FApp::IsProjectNameEmpty() ? FApp::GetProjectName() : FPlatformProcess::ExecutableName())
		+ FString("/Saved");

	int32 TotalFailedCount = 0;

	int32 AssetsPicoCopiedCount = 0;
	int32 AssetsPicoSkippedCount = 0;
	int32 AssetsPicoFailedCount = 0;

	if (bLoadAssetsFromApkAssets)
	{
		UE_LOG(LogPICOSpatial, Log, TEXT("DecompressAssetsPICO[AssetsPICO] skipped because APK-assets loading is enabled"));
	}
	else if (IFileManager::Get().DirectoryExists(*AssetsPicoSourceDirectory))
	{
		// Find all files recursively in source directory
		TArray<FString> FoundFiles;
		IFileManager::Get().FindFilesRecursive(FoundFiles, *AssetsPicoSourceDirectory, TEXT("*"), true, false);

		if (FoundFiles.Num() == 0)
		{
			UE_LOG(LogPICOSpatial, Log, TEXT("DecompressAssetsPICO[AssetsPICO] no files found. Src=%s"), *AssetsPicoSourceDirectory);
		}
		else
		{
			for (const FString& SourceFile : FoundFiles)
			{
				// Skip .uasset files
				if (SourceFile.EndsWith(TEXT(".uasset")))
				{
					AssetsPicoSkippedCount++;
					continue;
				}

				// Calculate relative path from source directory
				FString RelativePath = SourceFile;
				FPaths::MakePathRelativeTo(RelativePath, *(FPaths::ProjectContentDir() / TEXT("")));

				// Build destination file path
				FString DestinationFile = SavedDirectory / RelativePath;

				// Check if file already exists at destination
				if (IFileManager::Get().FileExists(*DestinationFile))
				{
					AssetsPicoSkippedCount++;
					continue;
				}

				// Create destination directory if it doesn't exist
				FString DestinationDir = FPaths::GetPath(DestinationFile);
				if (!IFileManager::Get().DirectoryExists(*DestinationDir))
				{
					if (!IFileManager::Get().MakeDirectory(*DestinationDir, true))
					{
						UE_LOG(LogPICOSpatial, Warning, TEXT("DecompressAssetsPICO[AssetsPICO] failed to create directory: %s"), *DestinationDir);
						AssetsPicoFailedCount++;
						continue;
					}
				}

				// Copy file
				if (IFileManager::Get().Copy(*DestinationFile, *SourceFile) == COPY_OK)
				{
					AssetsPicoCopiedCount++;
				}
				else
				{
					AssetsPicoFailedCount++;
					UE_LOG(LogPICOSpatial, Warning, TEXT("DecompressAssetsPICO[AssetsPICO] failed to copy: %s -> %s"), *SourceFile, *DestinationFile);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("DecompressAssetsPICO[AssetsPICO] source directory does not exist. Src=%s"), *AssetsPicoSourceDirectory);
	}

	UE_LOG(LogPICOSpatial, Log, TEXT("DecompressAssetsPICO[AssetsPICO] completed - Copied: %d, Skipped: %d, Failed: %d"),
		AssetsPicoCopiedCount, AssetsPicoSkippedCount, AssetsPicoFailedCount);
	TotalFailedCount += AssetsPicoFailedCount;

	// Copy Movies directory to Saved/AssetsPICO/Game/Movies
	if (IFileManager::Get().DirectoryExists(*MoviesSourceDirectory))
	{
		TArray<FString> MoviesFiles;
		IFileManager::Get().FindFilesRecursive(MoviesFiles, *MoviesSourceDirectory, TEXT("*.mp4"), true, false);

		int32 MoviesCopiedCount = 0;
		int32 MoviesSkippedCount = 0;
		int32 MoviesFailedCount = 0;

		if (MoviesFiles.Num() == 0)
		{
			UE_LOG(LogPICOSpatial, Log, TEXT("DecompressAssetsPICO[Movies] no mp4 found. Src=%s"), *MoviesSourceDirectory);
		}

		for (const FString& SourceFile : MoviesFiles)
		{
			// Build destination file path: Saved/AssetsPICO/Game/Movies/...
			FString DestinationFile = SavedDirectory / TEXT("AssetsPICO") / TEXT("Game") / TEXT("Movies") / FPaths::GetCleanFilename(SourceFile);

			// Check if file already exists at destination
			if (IFileManager::Get().FileExists(*DestinationFile))
			{
				MoviesSkippedCount++;
				continue;
			}

			// Create destination directory if it doesn't exist
			FString DestinationDir = FPaths::GetPath(DestinationFile);
			if (!IFileManager::Get().DirectoryExists(*DestinationDir))
			{
				if (!IFileManager::Get().MakeDirectory(*DestinationDir, true))
				{
					UE_LOG(LogPICOSpatial, Warning, TEXT("DecompressAssetsPICO[Movies] failed to create directory: %s"), *DestinationDir);
					MoviesFailedCount++;
					continue;
				}
			}

			// Copy file
			if (IFileManager::Get().Copy(*DestinationFile, *SourceFile) == COPY_OK)
			{
				MoviesCopiedCount++;
				UE_LOG(LogPICOSpatial, Verbose, TEXT("DecompressAssetsPICO[Movies] mp4 copied - Src=%s Dest=%s"), *SourceFile, *DestinationFile);
			}
			else
			{
				MoviesFailedCount++;
				UE_LOG(LogPICOSpatial, Warning, TEXT("DecompressAssetsPICO[Movies] failed to copy: %s -> %s"), *SourceFile, *DestinationFile);
			}
		}

		UE_LOG(LogPICOSpatial, Log, TEXT("DecompressAssetsPICO[Movies] completed - Copied: %d, Skipped: %d, Failed: %d"),
			MoviesCopiedCount, MoviesSkippedCount, MoviesFailedCount);

		TotalFailedCount += MoviesFailedCount;
	}
	else
	{
		UE_LOG(LogPICOSpatial, Verbose, TEXT("DecompressAssetsPICO[Movies] source directory does not exist. Src=%s"), *MoviesSourceDirectory);
	}

	return TotalFailedCount == 0;
#else
	UE_LOG(LogPICOSpatial, Warning, TEXT("DecompressAssetsPICO is only supported on Android platform"));
	return false;
#endif
}

#if PLATFORM_ANDROID
JNI_METHOD void Java_com_epicgames_unreal_PICOSpatialActivity_JavaToCOnSpatialContainerStateChangeCallback(JNIEnv* jEnv, jclass /*Clazz*/, jobject activity)
{
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		if (activity != NULL)
		{
			void* activityInstance = static_cast<void*>(Env->NewGlobalRef(activity));
			AsyncTask(ENamedThreads::GameThread, [activityInstance]()
				{
					uint64_t id = spatialpack::AppForm::getIdByActivityContext(activityInstance);
					bool bBoundActivity = false;
					if (APICOSpatialContainer* Container = APICOSpatialContainer::FindOpenedContainer(id))
					{
						if (Container->PICOSpatialActivityInstance == nullptr)
						{
							Container->PICOSpatialActivityInstance = activityInstance;
						}
						else
						{
							JNIEnv* Env = FAndroidApplication::GetJavaEnv();
							if (Env)
							{
								Env->DeleteGlobalRef(static_cast<jobject>(activityInstance));
							}
						}
						bBoundActivity = true;
					}

					if (!bBoundActivity)
					{
						JNIEnv* Env = FAndroidApplication::GetJavaEnv();
						if (Env)
						{
							Env->DeleteGlobalRef(static_cast<jobject>(activityInstance));
						}
					}
				});
		}
	}
}
#endif
