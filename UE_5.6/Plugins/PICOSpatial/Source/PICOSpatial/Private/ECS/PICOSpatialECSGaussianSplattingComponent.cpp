#include "ECS/PICOSpatialECSGaussianSplattingComponent.h"

#include "HAL/CriticalSection.h"
#include "Misc/App.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "PICOSpatial.h"
#include "PICOSpatialRuntimeSettings.h"

#if PLATFORM_ANDROID
namespace
{
TMap<FPICOSpatialECSGaussianAsyncLoadContext*, TSharedPtr<FPICOSpatialECSGaussianAsyncLoadContext>>& GetGaussianAsyncContextRegistry()
{
	static TMap<FPICOSpatialECSGaussianAsyncLoadContext*, TSharedPtr<FPICOSpatialECSGaussianAsyncLoadContext>> Registry;
	return Registry;
}

FCriticalSection& GetGaussianAsyncContextRegistryLock()
{
        static FCriticalSection RegistryLock;
        return RegistryLock;
}

void RegisterGaussianAsyncContext(const TSharedPtr<FPICOSpatialECSGaussianAsyncLoadContext>& Context)
{
        FScopeLock ScopeLock(&GetGaussianAsyncContextRegistryLock());
        GetGaussianAsyncContextRegistry().Add(Context.Get(), Context);
}

TSharedPtr<FPICOSpatialECSGaussianAsyncLoadContext> FindGaussianAsyncContext(FPICOSpatialECSGaussianAsyncLoadContext* Context)
{
        FScopeLock ScopeLock(&GetGaussianAsyncContextRegistryLock());
        if (const TSharedPtr<FPICOSpatialECSGaussianAsyncLoadContext>* Found = GetGaussianAsyncContextRegistry().Find(Context))
        {
                return *Found;
        }

        return nullptr;
}

void UnregisterGaussianAsyncContext(FPICOSpatialECSGaussianAsyncLoadContext* Context)
{
        FScopeLock ScopeLock(&GetGaussianAsyncContextRegistryLock());
        GetGaussianAsyncContextRegistry().Remove(Context);
}
}
#endif

void UPICOSpatialECSGaussianSplattingComponent::BeginDestroy()
{
#if PLATFORM_ANDROID
	ReleaseNativeResource();
#endif
	Super::BeginDestroy();
}

void UPICOSpatialECSGaussianSplattingComponent::setPath(const FString& InFilePath)
{
	Path = InFilePath;
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		ReloadNativeResource();
	}
#endif
}

bool UPICOSpatialECSGaussianSplattingComponent::reload()
{
#if PLATFORM_ANDROID
	if (HasNativeEntity())
	{
		return ReloadNativeResource();
	}
#endif
	return false;
}

void UPICOSpatialECSGaussianSplattingComponent::clearResource()
{
#if PLATFORM_ANDROID
	ReleaseNativeResource();
#endif
}

FString UPICOSpatialECSGaussianSplattingComponent::getPath() const
{
	return Path;
}

bool UPICOSpatialECSGaussianSplattingComponent::isResourceValid() const
{
#if PLATFORM_ANDROID
	return GaussianResource.isValid();
#else
	return false;
#endif
}

int64 UPICOSpatialECSGaussianSplattingComponent::getResourceId() const
{
#if PLATFORM_ANDROID
	return static_cast<int64>(GaussianResource.isValid() ? GaussianResource.getResourceId() : 0);
#else
	return 0;
#endif
}

bool UPICOSpatialECSGaussianSplattingComponent::isLoadPending() const
{
#if PLATFORM_ANDROID
	return PendingLoadJob != nullptr;
#else
	return false;
#endif
}

#if PLATFORM_ANDROID
static FString BuildAssetDstBaseLikeLoadAsset(const FString& Name)
{
	// Mirror FPICOSpatial::LoadAsset path construction:
	//   .../Saved/AssetsPICO + Name
	extern FString GFilePathBase;
	const FString ProjectName = !FApp::IsProjectNameEmpty() ? FApp::GetProjectName() : FPlatformProcess::ExecutableName();
	return GFilePathBase
		+ FString("/UnrealGame/")
		+ ProjectName
		+ FString("/")
		+ ProjectName
		+ FString("/Saved/AssetsPICO")
		+ Name;
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

static FString ResolveGaussianStoragePath(const FString& InFilePath)
{
	auto TryExistingFile = [](const FString& Candidate) -> FString
	{
		return IFileManager::Get().FileExists(*Candidate) ? Candidate : FString();
	};

	const FString TrimmedFilePath = InFilePath.TrimStartAndEnd();
	if (TrimmedFilePath.IsEmpty())
	{
		return FString();
	}

	// Align with ModelViewer + LoadAsset:
	// - User types "demo_fox_gs", "demo_fox_gs.ply", or "demo_fox_gs.spz"
	// - We build Name="/UserCustomAssets/<BaseName>"
	// - If the user explicitly provides an extension, honor it first.
	// - Otherwise resolve under Saved/AssetsPICO by trying .ply/.spz
	const FString FileName = FPaths::GetCleanFilename(TrimmedFilePath);
	const FString BaseName = FPaths::GetBaseFilename(FileName);
	const FString ExplicitExtension = FPaths::GetExtension(FileName, false).ToLower();
	const FString Name = TEXT("/UserCustomAssets/") + BaseName;
	const FString AssetDstBase = BuildAssetDstBaseLikeLoadAsset(Name);

	if (ExplicitExtension == TEXT("ply") || ExplicitExtension == TEXT("spz"))
	{
		const FString ExplicitCandidate = AssetDstBase + TEXT(".") + ExplicitExtension;
		if (const FString Hit = TryExistingFile(ExplicitCandidate); !Hit.IsEmpty())
		{
			return Hit;
		}

		// Keep a deterministic fallback for logging/debugging if the explicitly requested file is missing.
		return ExplicitCandidate;
	}

	// Try common gaussian extensions (order matches our expected content).
	const FString PlyCandidate = AssetDstBase + TEXT(".ply");
	if (const FString Hit = TryExistingFile(PlyCandidate); !Hit.IsEmpty())
	{
		return Hit;
	}

	const FString SpzCandidate = AssetDstBase + TEXT(".spz");
	if (const FString Hit = TryExistingFile(SpzCandidate); !Hit.IsEmpty())
	{
		return Hit;
	}

	// Return the most likely path for logging/debugging.
	return PlyCandidate;
}

static FString ResolveGaussianAssetUri(const FString& InFilePath)
{
	const FString TrimmedFilePath = InFilePath.TrimStartAndEnd();
	if (TrimmedFilePath.IsEmpty())
	{
		return FString();
	}

	const FString FileName = FPaths::GetCleanFilename(TrimmedFilePath);
	const FString BaseName = FPaths::GetBaseFilename(FileName);
	const FString ExplicitExtension = FPaths::GetExtension(FileName, false).ToLower();
	const FString Name = TEXT("/UserCustomAssets/") + BaseName;
	const FString AssetUriBase = BuildAssetUriBaseLikeLoadAsset(Name);

	if (ExplicitExtension == TEXT("ply") || ExplicitExtension == TEXT("spz"))
	{
		return AssetUriBase + TEXT(".") + ExplicitExtension;
	}

	return AssetUriBase + TEXT(".ply");
}

static FString MakeFileUri(const FString& AbsolutePath)
{
	// SpatialPack APIs say "uri". Some runtime builds only accept file:// URIs for storage.
	FString NormalizedPath = AbsolutePath;
	NormalizedPath.ReplaceInline(TEXT("\\"), TEXT("/"));
	if (NormalizedPath.StartsWith(TEXT("/")))
	{
		return FString(TEXT("file:///")) + NormalizedPath.Mid(1);
	}
	return FString(TEXT("file:///")) + NormalizedPath;
}

void UPICOSpatialECSGaussianSplattingComponent::ReleaseNativeResource()
{
	CancelPendingLoadJob();
	GaussianResource = spatialpack::GaussianSplattingResource();
	bPendingLoadFromAssets = false;
	PendingResolvedUri.Reset();
	PendingResourceName.Reset();
	PendingAlternateUri.Reset();
	if (HasNativeEntity())
	{
		BindCurrentResource();
	}
}

void UPICOSpatialECSGaussianSplattingComponent::CancelPendingLoadJob()
{
	if (PendingLoadJob)
	{
		UE_LOG(LogPICOSpatial, Log, TEXT("[ECS][Gaussian] Async load cancelled. Obj=%s JobId=%llu Path='%s'"),
			*GetName(),
			(uint64)PendingLoadJobId,
			*PendingResolvedUri);
		spatialpack::GaussianSplattingResource::destroyAsyncJob(PendingLoadJob);
		PendingLoadJob = nullptr;
	}

	PendingLoadJobId = 0;
	PendingLoadAttemptIndex = INDEX_NONE;
	if (ActiveAsyncLoadContext)
	{
		ActiveAsyncLoadContext->bActive = false;
		ActiveAsyncLoadContext->ExpectedJobId = 0;
                UnregisterGaussianAsyncContext(ActiveAsyncLoadContext.Get());
		ActiveAsyncLoadContext.Reset();
	}
}

bool UPICOSpatialECSGaussianSplattingComponent::StartAsyncLoadAttempt(int32 AttemptIndex)
{
	if (PendingResolvedUri.IsEmpty())
	{
		return false;
	}

	if (bPendingLoadFromAssets && AttemptIndex >= 2)
	{
		return false;
	}

	if (!ActiveAsyncLoadContext)
	{
		return false;
	}

	const bool bUseAlternateUri = !bPendingLoadFromAssets && AttemptIndex >= 2;
	const bool bUseName = (AttemptIndex == 0 || AttemptIndex == 2) && !PendingResourceName.IsEmpty();
	const FString& RequestUri = bUseAlternateUri ? PendingAlternateUri : PendingResolvedUri;
	const FTCHARToUTF8 UriUtf8(*RequestUri);
	const FTCHARToUTF8 NameUtf8(bUseName ? *PendingResourceName : TEXT(""));
	const ANSICHAR* NameAnsi = bUseName ? NameUtf8.Get() : nullptr;

	PendingLoadAttemptIndex = AttemptIndex;
	PendingLoadJob = bPendingLoadFromAssets
		? spatialpack::GaussianSplattingResource::loadFromAssetsAsync(
			UriUtf8.Get(),
			ActiveAsyncLoadContext.Get(),
			&UPICOSpatialECSGaussianSplattingComponent::OnAsyncGaussianLoaded,
			NameAnsi)
		: spatialpack::GaussianSplattingResource::loadFromStorageAsync(
			UriUtf8.Get(),
			ActiveAsyncLoadContext.Get(),
			&UPICOSpatialECSGaussianSplattingComponent::OnAsyncGaussianLoaded,
			NameAnsi);

	if (!PendingLoadJob || !PendingLoadJob->isValid())
	{
		PendingLoadJob = nullptr;
		PendingLoadJobId = 0;
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Gaussian] Failed to schedule async load. Obj=%s Attempt=%d Uri='%s' Name='%s'"),
			*GetName(),
			AttemptIndex,
			*RequestUri,
			bUseName ? *PendingResourceName : TEXT("<null>"));
		return false;
	}

	PendingLoadJobId = PendingLoadJob->getJobId();
	ActiveAsyncLoadContext->ExpectedJobId = PendingLoadJobId;
	UE_LOG(LogPICOSpatial, Log, TEXT("[ECS][Gaussian] Async load scheduled. Obj=%s UUID=%llu Attempt=%d JobId=%llu Uri='%s' Name='%s'"),
		*GetName(),
		HasNativeEntity() ? (uint64)GetCachedNativeEntity().getEntityUUID() : 0ull,
		AttemptIndex,
		(uint64)PendingLoadJobId,
		*RequestUri,
		bUseName ? *PendingResourceName : TEXT("<null>"));
	return true;
}

bool UPICOSpatialECSGaussianSplattingComponent::StartAsyncLoad(const FString& ResolvedUri, const FString& ResourceName, bool bInLoadFromAssets)
{
	CancelPendingLoadJob();
	ActiveAsyncLoadContext = MakeShared<FPICOSpatialECSGaussianAsyncLoadContext>();
	ActiveAsyncLoadContext->Component = this;
	ActiveAsyncLoadContext->Generation = ++AsyncLoadGeneration;
	ActiveAsyncLoadContext->bActive = true;
        RegisterGaussianAsyncContext(ActiveAsyncLoadContext);

	bPendingLoadFromAssets = bInLoadFromAssets;
	PendingResolvedUri = ResolvedUri;
	PendingResourceName = ResourceName;
	PendingAlternateUri = bPendingLoadFromAssets ? FString() : MakeFileUri(ResolvedUri);
	GaussianResource = spatialpack::GaussianSplattingResource();
	BindCurrentResource();
	if (StartAsyncLoadAttempt(0))
	{
		return true;
	}

	CompleteActiveAsyncLoadContext();
	return false;
}

void UPICOSpatialECSGaussianSplattingComponent::FinalizeLoadedResource(const spatialpack::GaussianSplattingResource& InResource)
{
	GaussianResource = InResource;
	BindCurrentResource();
}

void UPICOSpatialECSGaussianSplattingComponent::OnAsyncGaussianLoaded(
	spatialpack::AsyncJobId JobId,
	void* User,
	const spatialpack::GaussianSplattingResource& Resource,
	const spatialpack::ResourceErrorInfo& ErrorInfo)
{
	FPICOSpatialECSGaussianAsyncLoadContext* Context = static_cast<FPICOSpatialECSGaussianAsyncLoadContext*>(User);
	if (Context == nullptr)
	{
		return;
	}

        const TSharedPtr<FPICOSpatialECSGaussianAsyncLoadContext> RegisteredContext = FindGaussianAsyncContext(Context);
        if (!RegisteredContext.IsValid() || !Context->bActive || Context->ExpectedJobId != JobId)
	{
		return;
	}

	UPICOSpatialECSGaussianSplattingComponent* Self = Context->Component.Get();
	if (Self == nullptr || !Self->ActiveAsyncLoadContext.IsValid() || Self->ActiveAsyncLoadContext.Get() != Context)
	{
		Context->bActive = false;
                UnregisterGaussianAsyncContext(Context);
		return;
	}

	if (JobId != Self->PendingLoadJobId)
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Gaussian] Ignoring stale async callback. Obj=%s CallbackJobId=%llu PendingJobId=%llu"),
			*Self->GetName(),
			(uint64)JobId,
			(uint64)Self->PendingLoadJobId);
		return;
	}

	const int32 CompletedAttemptIndex = Self->PendingLoadAttemptIndex;
	spatialpack::GaussianSplattingResourceAsyncLoadJob* CompletedJob = Self->PendingLoadJob;
	if (CompletedJob)
	{
		spatialpack::GaussianSplattingResource::destroyAsyncJob(CompletedJob);
	}
	Self->PendingLoadJob = nullptr;
	Self->PendingLoadJobId = 0;
	Self->PendingLoadAttemptIndex = INDEX_NONE;

	UE_LOG(LogPICOSpatial, Log, TEXT("[ECS][Gaussian] Async load completed. Obj=%s Attempt=%d Valid=%d Code=%d Msg='%s'"),
		*Self->GetName(),
		CompletedAttemptIndex,
		Resource.isValid() ? 1 : 0,
		(int32)ErrorInfo.code,
		ErrorInfo.msg ? UTF8_TO_TCHAR(ErrorInfo.msg) : TEXT(""));

	if (Resource.isValid())
	{
		Self->FinalizeLoadedResource(Resource);
		Self->CompleteActiveAsyncLoadContext();
		return;
	}

	for (int32 NextAttemptIndex = CompletedAttemptIndex + 1; NextAttemptIndex < 4; ++NextAttemptIndex)
	{
		if (Self->StartAsyncLoadAttempt(NextAttemptIndex))
		{
			return;
		}
	}

	Self->GaussianResource = spatialpack::GaussianSplattingResource();
	Self->BindCurrentResource();
	Self->CompleteActiveAsyncLoadContext();
	UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Gaussian] Async load failed after all attempts. Obj=%s Path='%s' Name='%s'"),
		*Self->GetName(),
		*Self->PendingResolvedUri,
		*Self->PendingResourceName);
}

void UPICOSpatialECSGaussianSplattingComponent::CompleteActiveAsyncLoadContext()
{
	if (ActiveAsyncLoadContext)
	{
		ActiveAsyncLoadContext->bActive = false;
		ActiveAsyncLoadContext->ExpectedJobId = 0;
		GetGaussianAsyncContextRegistry().Remove(ActiveAsyncLoadContext.Get());
		ActiveAsyncLoadContext.Reset();
	}
}

void UPICOSpatialECSGaussianSplattingComponent::OnInitializeNativeComponent(spatialpack::SceneEntity& InNativeEntity)
{
	InNativeEntity.getOrAddGaussianSplattingComponent();
}

void UPICOSpatialECSGaussianSplattingComponent::ApplyAllPropertiesToNative()
{
	if (!HasNativeEntity())
	{
		return;
	}

	ReloadNativeResource();
}

bool UPICOSpatialECSGaussianSplattingComponent::ReloadNativeResource()
{
	const FString TrimmedPath = Path.TrimStartAndEnd();
	if (TrimmedPath.IsEmpty())
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Gaussian] Reload skipped: empty path. Obj=%s"), *GetName());
		ReleaseNativeResource();
		return false;
	}

	const FString FileName = FPaths::GetCleanFilename(TrimmedPath);
	const UPICOSpatialRuntimeSettings* Settings = GetDefault<UPICOSpatialRuntimeSettings>();
	const bool bLoadFromAssets = Settings && Settings->bLoadAssetsFromApkAssets;
	const FString ResolvedUri = bLoadFromAssets ? ResolveGaussianAssetUri(FileName) : ResolveGaussianStoragePath(FileName);
	const FString ResourceName = FPaths::GetCleanFilename(ResolvedUri);
	const FTCHARToUTF8 PathUtf8(*ResolvedUri);
	const ANSICHAR* PathAnsi = PathUtf8.Get();
	const FTCHARToUTF8 NameUtf8(ResourceName.IsEmpty() ? TEXT("") : *ResourceName);
	const ANSICHAR* NameAnsi = ResourceName.IsEmpty() ? nullptr : NameUtf8.Get();

	UE_LOG(LogPICOSpatial, Log, TEXT("[ECS][Gaussian] Resolve load path. Obj=%s UUID=%llu Path='%s' Resolved='%s' Name='%s' FromAssets=%d"),
		*GetName(),
		HasNativeEntity() ? (uint64)GetCachedNativeEntity().getEntityUUID() : 0ull,
		*TrimmedPath,
		*ResolvedUri,
		*ResourceName,
		bLoadFromAssets ? 1 : 0);

	if (!bLoadFromAssets)
	{
		const bool bFileExists = !ResolvedUri.IsEmpty() && IFileManager::Get().FileExists(*ResolvedUri);
		const int64 FileSize = bFileExists ? IFileManager::Get().FileSize(*ResolvedUri) : -1;
		if (!bFileExists)
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Gaussian] Storage file not found. Path='%s' Resolved='%s' Obj=%s"),
				*TrimmedPath,
				*ResolvedUri,
				*GetName());
			ReleaseNativeResource();
			return false;
		}

		if (FileSize <= 0)
		{
			UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Gaussian] Storage file size invalid. Size=%lld Path='%s' Obj=%s"),
				(long long)FileSize,
				*ResolvedUri,
				*GetName());
		}
	}

	if (bUseAsyncLoad)
	{
		if (StartAsyncLoad(ResolvedUri, ResourceName, bLoadFromAssets))
		{
			return false;
		}

		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Gaussian] Async scheduling failed, falling back to synchronous load. Obj=%s Path='%s'"),
			*GetName(),
			*ResolvedUri);
	}
	else
	{
		CancelPendingLoadJob();
		GaussianResource = spatialpack::GaussianSplattingResource();
		BindCurrentResource();
	}

	// Try a few URI/name variants to match native sample behavior and tolerate runtime URI expectations.
	auto TryLoad = [&](const ANSICHAR* Uri, const ANSICHAR* InNameAnsi, const TCHAR* Tag) -> bool
	{
		GaussianResource = bLoadFromAssets
			? spatialpack::GaussianSplattingResource::loadFromAssets(Uri, InNameAnsi)
			: spatialpack::GaussianSplattingResource::loadFromStorage(Uri, InNameAnsi);
		const spatialpack::ResourceErrorInfo Err = spatialpack::ResourceErrorInfo::getLastErrorInfo();
		UE_LOG(LogPICOSpatial, Verbose, TEXT("[ECS][Gaussian] load attempt=%s mode=%s valid=%d code=%d msg='%s' uri='%s' name='%s'"),
			Tag,
			bLoadFromAssets ? TEXT("assets") : TEXT("storage"),
			GaussianResource.isValid() ? 1 : 0,
			(int32)Err.code,
			Err.msg ? UTF8_TO_TCHAR(Err.msg) : TEXT(""),
			UTF8_TO_TCHAR(Uri ? Uri : ""),
			InNameAnsi ? UTF8_TO_TCHAR(InNameAnsi) : TEXT("<null>"));
		return GaussianResource.isValid();
	};

	if (!TryLoad(PathAnsi, NameAnsi, TEXT("raw+name")))
	{
		if (!TryLoad(PathAnsi, nullptr, TEXT("raw+nullname")))
		{
			if (!bLoadFromAssets)
			{
				const FString FileUri = MakeFileUri(ResolvedUri);
				const FTCHARToUTF8 FileUriUtf8(*FileUri);
				const ANSICHAR* FileUriAnsi = FileUriUtf8.Get();
				if (!TryLoad(FileUriAnsi, NameAnsi, TEXT("fileuri+name")))
				{
					TryLoad(FileUriAnsi, nullptr, TEXT("fileuri+nullname"));
				}
			}
		}
	}

	BindCurrentResource();
	bPendingLoadFromAssets = bLoadFromAssets;
	PendingResolvedUri = ResolvedUri;
	PendingResourceName = ResourceName;
	PendingAlternateUri = bLoadFromAssets ? FString() : MakeFileUri(ResolvedUri);

	if (!GaussianResource.isValid())
	{
		const spatialpack::ResourceErrorInfo Err = spatialpack::ResourceErrorInfo::getLastErrorInfo();
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Gaussian] load failed. Mode=%s Code=%d Msg='%s'"),
			bLoadFromAssets ? TEXT("assets") : TEXT("storage"),
			(int32)Err.code,
			Err.msg ? UTF8_TO_TCHAR(Err.msg) : TEXT("<null>"));
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][Gaussian] Failed to load static gaussian resource. Path='%s' Name='%s' Obj=%s"),
			*ResolvedUri,
			*ResourceName,
			*GetName());
		return false;
	}

	return true;
}

void UPICOSpatialECSGaussianSplattingComponent::BindCurrentResource()
{
	if (!HasNativeEntity())
	{
		return;
	}

	auto GaussianComponent = GetCachedNativeEntity().getOrAddGaussianSplattingComponent();
	GaussianComponent.setGaussianSplattingResource(GaussianResource);
	UE_LOG(LogPICOSpatial, Log, TEXT("[ECS][Gaussian] BindCurrentResource Obj=%s UUID=%llu ResourceValid=%d ResourceId=%llu"),
		*GetName(),
		(uint64)GetCachedNativeEntity().getEntityUUID(),
		GaussianResource.isValid() ? 1 : 0,
		GaussianResource.isValid() ? (uint64)GaussianResource.getResourceId() : 0ull);
}
#endif
