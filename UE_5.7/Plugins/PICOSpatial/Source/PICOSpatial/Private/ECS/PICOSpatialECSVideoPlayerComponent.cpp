#include "ECS/PICOSpatialECSVideoPlayerComponent.h"

#include "HAL/CriticalSection.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeLock.h"

#if PLATFORM_ANDROID
namespace
{
TMap<FPICOSpatialECSVideoPlayerCallbackContext*, TSharedPtr<FPICOSpatialECSVideoPlayerCallbackContext>>& GetVideoPlayerCallbackContextRegistry()
{
	static TMap<FPICOSpatialECSVideoPlayerCallbackContext*, TSharedPtr<FPICOSpatialECSVideoPlayerCallbackContext>> Registry;
	return Registry;
}

FCriticalSection& GetVideoPlayerCallbackContextRegistryLock()
{
        static FCriticalSection RegistryLock;
        return RegistryLock;
}

void RegisterVideoPlayerCallbackContext(const TSharedPtr<FPICOSpatialECSVideoPlayerCallbackContext>& Context)
{
        FScopeLock ScopeLock(&GetVideoPlayerCallbackContextRegistryLock());
        GetVideoPlayerCallbackContextRegistry().Add(Context.Get(), Context);
}

TSharedPtr<FPICOSpatialECSVideoPlayerCallbackContext> FindVideoPlayerCallbackContext(FPICOSpatialECSVideoPlayerCallbackContext* Context)
{
        FScopeLock ScopeLock(&GetVideoPlayerCallbackContextRegistryLock());
        if (const TSharedPtr<FPICOSpatialECSVideoPlayerCallbackContext>* Found = GetVideoPlayerCallbackContextRegistry().Find(Context))
        {
                return *Found;
        }

        return nullptr;
}

void UnregisterVideoPlayerCallbackContext(FPICOSpatialECSVideoPlayerCallbackContext* Context)
{
        FScopeLock ScopeLock(&GetVideoPlayerCallbackContextRegistryLock());
        GetVideoPlayerCallbackContextRegistry().Remove(Context);
}
}
#endif

static FString ResolveVideoPath(const FString& InValue)
{
	FString Value = InValue;
	Value.TrimStartAndEndInline();
	if (Value.IsEmpty())
	{
		return FString();
	}

	const FString ValueFull = FPaths::ConvertRelativePathToFull(Value);
	if (IFileManager::Get().FileExists(*ValueFull))
	{
		return ValueFull;
	}

	FString FileName = Value;
	if (!FileName.EndsWith(TEXT(".mp4"), ESearchCase::IgnoreCase))
	{
		FileName += TEXT(".mp4");
	}

	auto TryFile = [](const FString& Candidate) -> FString
	{
		return IFileManager::Get().FileExists(*Candidate) ? Candidate : FString();
	};

	const FString MovieFileName = FPaths::GetCleanFilename(FileName);
	const FString ValueWithExt = Value.EndsWith(TEXT(".mp4"), ESearchCase::IgnoreCase) ? Value : (Value + TEXT(".mp4"));

#if PLATFORM_ANDROID
	extern FString GFilePathBase;
	const FString ProjectName = !FApp::IsProjectNameEmpty() ? FApp::GetProjectName() : FPlatformProcess::ExecutableName();
	const FString SavedDirAbs = GFilePathBase + FString("/UnrealGame/") + ProjectName + FString("/") + ProjectName + FString("/Saved");

	if (FPaths::IsRelative(Value)
		&& (Value.StartsWith(TEXT("AssetsPICO/")) || Value.StartsWith(TEXT("AssetsPICO\\"))))
	{
		const FString CandidateAndroidSavedRel = SavedDirAbs / Value;
		if (const FString Hit = TryFile(CandidateAndroidSavedRel); !Hit.IsEmpty())
		{
			return Hit;
		}

		const FString CandidateAndroidSavedRelWithExt = SavedDirAbs / ValueWithExt;
		if (const FString Hit = TryFile(CandidateAndroidSavedRelWithExt); !Hit.IsEmpty())
		{
			return Hit;
		}
	}

	const FString CandidateAndroidSavedMovies = SavedDirAbs / TEXT("Movies") / MovieFileName;
	if (const FString Hit = TryFile(CandidateAndroidSavedMovies); !Hit.IsEmpty())
	{
		return Hit;
	}

	const FString CandidateAndroidAssetsPICOVideo = SavedDirAbs / TEXT("AssetsPICO/Game/Movies") / MovieFileName;
	if (const FString Hit = TryFile(CandidateAndroidAssetsPICOVideo); !Hit.IsEmpty())
	{
		return Hit;
	}
#endif

	if (FPaths::IsRelative(Value))
	{
		const FString CandidateSavedRelative = FPaths::ProjectSavedDir() / FileName;
		const FString CandidateSavedRelativeFull = FPaths::ConvertRelativePathToFull(CandidateSavedRelative);
		if (const FString Hit = TryFile(CandidateSavedRelativeFull); !Hit.IsEmpty())
		{
			return Hit;
		}

		if (Value.StartsWith(TEXT("AssetsPICO/")) || Value.StartsWith(TEXT("AssetsPICO\\")))
		{
			const FString CandidateSavedAssetsRel = FPaths::ProjectSavedDir() / ValueWithExt;
			const FString CandidateSavedAssetsRelFull = FPaths::ConvertRelativePathToFull(CandidateSavedAssetsRel);
			if (const FString Hit = TryFile(CandidateSavedAssetsRelFull); !Hit.IsEmpty())
			{
				return Hit;
			}
		}

		const FString CandidateSavedRelativeLegacy = FPaths::ProjectSavedDir() / ValueWithExt;
		const FString CandidateSavedRelativeLegacyFull = FPaths::ConvertRelativePathToFull(CandidateSavedRelativeLegacy);
		if (const FString Hit = TryFile(CandidateSavedRelativeLegacyFull); !Hit.IsEmpty())
		{
			return Hit;
		}
	}

	const FString CandidateSavedMovies = FPaths::ProjectSavedDir() / TEXT("Movies") / MovieFileName;
	const FString CandidateSavedMoviesFull = FPaths::ConvertRelativePathToFull(CandidateSavedMovies);
	if (const FString Hit = TryFile(CandidateSavedMoviesFull); !Hit.IsEmpty())
	{
		return Hit;
	}

	const FString CandidateSavedAssetsPICOVideo = FPaths::ProjectSavedDir() / TEXT("AssetsPICO/Game/Movies") / MovieFileName;
	const FString CandidateSavedAssetsPICOVideoFull = FPaths::ConvertRelativePathToFull(CandidateSavedAssetsPICOVideo);
	if (const FString Hit = TryFile(CandidateSavedAssetsPICOVideoFull); !Hit.IsEmpty())
	{
		return Hit;
	}

	const FString CandidateContentMovies = FPaths::ProjectContentDir() / TEXT("Movies") / MovieFileName;
	const FString CandidateContentMoviesFull = FPaths::ConvertRelativePathToFull(CandidateContentMovies);
	if (const FString Hit = TryFile(CandidateContentMoviesFull); !Hit.IsEmpty())
	{
		return Hit;
	}

	return CandidateSavedAssetsPICOVideoFull;
}

#if PLATFORM_ANDROID
#include "PICOSpatial.h"
#include "PICOSpatialUtility.h"

static spatialpack::VideoDimensionMode ToSpVideoDimensionMode(EPICOSpatialVideoDimensionMode InMode)
{
	switch (InMode)
	{
	case EPICOSpatialVideoDimensionMode::Mono:
		return spatialpack::VideoDimensionMode::MONO;
	case EPICOSpatialVideoDimensionMode::SideBySide:
		return spatialpack::VideoDimensionMode::SIDE_BY_SIDE;
	case EPICOSpatialVideoDimensionMode::MultipleView:
		return spatialpack::VideoDimensionMode::MULTIPLE_VIEW;
	default:
		return spatialpack::VideoDimensionMode::MONO;
	}
}

static spatialpack::MaterialBlendingMode ToSpMaterialBlendingMode(EPICOSpatialVideoBlendingMode InMode)
{
	switch (InMode)
	{
	case EPICOSpatialVideoBlendingMode::Opaque:
		return spatialpack::MaterialBlendingMode::OPAQUE;
	case EPICOSpatialVideoBlendingMode::Transparent:
		return spatialpack::MaterialBlendingMode::TRANSPARENT;
	default:
		return spatialpack::MaterialBlendingMode::TRANSPARENT;
	}
}
#endif

void UPICOSpatialECSVideoPlayerComponent::setDataSourcePath(const FString& InFilePath)
{
	const bool bDataSourceChanged = DataSourcePath != InFilePath;
	DataSourcePath = InFilePath;
	if (bDataSourceChanged)
	{
		LastKnownPositionMs = 0;
		LastKnownDurationMs = 0;
		bLastKnownPlaying = false;
		bLastKnownComplete = false;
		LastKnownVideoWidth = 0;
		LastKnownVideoHeight = 0;
	}

#if PLATFORM_ANDROID
	if (bDataSourceChanged)
	{
		bHasConsumedAutoPlay = false;
	}

	const FString ResolvedPath = ResolveVideoPath(DataSourcePath);
	if (!ResolvedPath.IsEmpty() && !IFileManager::Get().FileExists(*ResolvedPath))
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][VideoPlayer] DataSource not found. Path='%s'"), *ResolvedPath);
	}
	if (HasNativeEntity() && Player.isValid())
	{
		Player.reset();
		Player.setDataSource(TCHAR_TO_UTF8(*ResolvedPath));
		if (bAutoPrepare)
		{
			Player.prepareAsync();
		}
	}
#endif
}

void UPICOSpatialECSVideoPlayerComponent::setDataSourceName(const FString& InVideoName)
{
	FString FileName = InVideoName;
	FileName.TrimStartAndEndInline();
	if (FileName.IsEmpty())
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][VideoPlayer] setDataSourceName empty. Clearing path."));
		setDataSourcePath(FString());
		return;
	}

	if (!FileName.EndsWith(TEXT(".mp4"), ESearchCase::IgnoreCase))
	{
		FileName += TEXT(".mp4");
	}

	uint64 Uuid = 0;
#if PLATFORM_ANDROID
	Uuid = HasNativeEntity() ? (uint64)GetCachedNativeEntity().getEntityUUID() : 0ull;
#endif
	const FString ResolvedPath = ResolveVideoPath(FileName);
	UE_LOG(LogPICOSpatial, Log, TEXT("[ECS][VideoPlayer] setDataSourceName UUID=%llu Name='%s' ResolvedPath='%s' Exists=%d"),
		Uuid,
		*InVideoName,
		*ResolvedPath,
		IFileManager::Get().FileExists(*ResolvedPath) ? 1 : 0);
	setDataSourcePath(FileName);
}

void UPICOSpatialECSVideoPlayerComponent::prepareAsync()
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		Player.prepareAsync();
	}
#endif
}

void UPICOSpatialECSVideoPlayerComponent::play()
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		if (Player.play())
		{
			bLastKnownPlaying = true;
			bLastKnownComplete = false;
		}
	}
#endif
}

void UPICOSpatialECSVideoPlayerComponent::pause()
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		CachePlaybackStateFromNative();
		Player.pause();
		bLastKnownPlaying = false;
	}
#endif
}

void UPICOSpatialECSVideoPlayerComponent::resume()
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		Player.resume();
		bLastKnownPlaying = true;
		bLastKnownComplete = false;
	}
#endif
}

void UPICOSpatialECSVideoPlayerComponent::stop()
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		CachePlaybackStateFromNative();
		Player.stop();
		bLastKnownPlaying = false;
	}
#endif
}

void UPICOSpatialECSVideoPlayerComponent::reset()
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		Player.reset();
		LastKnownPositionMs = 0;
		LastKnownDurationMs = 0;
		bLastKnownPlaying = false;
		bLastKnownComplete = false;
		LastKnownVideoWidth = 0;
		LastKnownVideoHeight = 0;
	}
#endif
}

void UPICOSpatialECSVideoPlayerComponent::seekTo(int64 InPositionMs)
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		if (Player.seekTo(InPositionMs))
		{
			LastKnownPositionMs = FMath::Max<int64>(0, InPositionMs);
		}
	}
#endif
}

void UPICOSpatialECSVideoPlayerComponent::setLoop(bool bInLoop)
{
	bLoop = bInLoop;
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		Player.setLoop(bLoop);
	}
#endif
}

void UPICOSpatialECSVideoPlayerComponent::setPlaybackSpeed(float InSpeed)
{
	PlaybackSpeed = FMath::Clamp(InSpeed, 0.1f, 4.0f);
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		Player.setPlaybackSpeed(PlaybackSpeed);
	}
#endif
}

void UPICOSpatialECSVideoPlayerComponent::setVolume(float InVolume)
{
	Volume = FMath::Clamp(InVolume, 0.0f, 1.0f);
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		Player.setVolume(Volume);
	}
#endif
}

FString UPICOSpatialECSVideoPlayerComponent::getDataSourcePath() const
{
	return DataSourcePath;
}

bool UPICOSpatialECSVideoPlayerComponent::isPlayerValid() const
{
#if PLATFORM_ANDROID
	return Player.isValid();
#else
	return false;
#endif
}

int64 UPICOSpatialECSVideoPlayerComponent::getCurrentPositionMs() const
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		return Player.getCurrentPosition();
	}
#endif
	return LastKnownPositionMs;
}

int64 UPICOSpatialECSVideoPlayerComponent::getDurationMs() const
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		return Player.getDuration();
	}
#endif
	return LastKnownDurationMs;
}

bool UPICOSpatialECSVideoPlayerComponent::isPlaying() const
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		return Player.isPlaying();
	}
#endif
	return bLastKnownPlaying;
}

bool UPICOSpatialECSVideoPlayerComponent::isComplete() const
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		return Player.isComplete();
	}
#endif
	return bLastKnownComplete;
}

int32 UPICOSpatialECSVideoPlayerComponent::getVideoWidth() const
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		return Player.getVideoWidth();
	}
#endif
	return LastKnownVideoWidth;
}

int32 UPICOSpatialECSVideoPlayerComponent::getVideoHeight() const
{
#if PLATFORM_ANDROID
	if (Player.isValid())
	{
		return Player.getVideoHeight();
	}
#endif
	return LastKnownVideoHeight;
}

void UPICOSpatialECSVideoPlayerComponent::BeginDestroy()
{
#if PLATFORM_ANDROID
	ReleaseNativePlayer();
#endif
	Super::BeginDestroy();
}

#if PLATFORM_ANDROID
void UPICOSpatialECSVideoPlayerComponent::ReleaseNativePlayer()
{
	if (Player.isValid())
	{
                CachePlaybackStateFromNative();
		Player.setOnStateChangedListener(nullptr, nullptr);
		Player.stop();
		Player.reset();
		Player = spatialpack::CypressMediaPlayer();
	}
	ClearPlayerCallbackContext();
}

void UPICOSpatialECSVideoPlayerComponent::OnInitializeNativeComponent(spatialpack::SceneEntity& InNativeEntity)
{
	InNativeEntity.getOrAddVideoPlayerComponent();
	EnsureNativeBindings();
}

void UPICOSpatialECSVideoPlayerComponent::ApplyAllPropertiesToNative()
{
	if (!HasNativeEntity())
	{
		return;
	}

	const FString ResolvedPath = ResolveVideoPath(DataSourcePath);
	EnsureNativeBindings();
	if (!ResolvedPath.IsEmpty() && bAutoPrepare)
	{
		Player.reset();
		Player.setDataSource(TCHAR_TO_UTF8(*ResolvedPath));
		Player.prepareAsync();
	}
}

void UPICOSpatialECSVideoPlayerComponent::EnsureNativeBindings()
{
	EnsurePlayer();
	RebuildVideoPlayerBinding();
}

void UPICOSpatialECSVideoPlayerComponent::EnsurePlayer()
{
	if (Player.isValid())
	{
		return;
	}

	Player = spatialpack::CypressMediaPlayer::createPlayer();
	if (!Player.isValid())
	{
		UE_LOG(LogPICOSpatial, Warning, TEXT("[ECS][VideoPlayer] Failed to create CypressMediaPlayer. Obj=%s"), *GetName());
		return;
	}

	ActivePlayerCallbackContext = MakeShared<FPICOSpatialECSVideoPlayerCallbackContext>();
	ActivePlayerCallbackContext->Component = this;
	ActivePlayerCallbackContext->Generation = ++PlayerCallbackGeneration;
	ActivePlayerCallbackContext->bActive = true;
        RegisterVideoPlayerCallbackContext(ActivePlayerCallbackContext);

	Player.setOnStateChangedListener(ActivePlayerCallbackContext.Get(), [](void* User, spatialpack::MediaPlaybackState State)
	{
		FPICOSpatialECSVideoPlayerCallbackContext* Context = static_cast<FPICOSpatialECSVideoPlayerCallbackContext*>(User);
		if (Context == nullptr)
		{
			return;
		}

                const TSharedPtr<FPICOSpatialECSVideoPlayerCallbackContext> RegisteredContext = FindVideoPlayerCallbackContext(Context);
                if (!RegisteredContext.IsValid() || !Context->bActive)
		{
			return;
		}

		UPICOSpatialECSVideoPlayerComponent* Self = Context->Component.Get();
		if (Self == nullptr || !Self->ActivePlayerCallbackContext.IsValid() || Self->ActivePlayerCallbackContext.Get() != Context)
		{
			Context->bActive = false;
                        UnregisterVideoPlayerCallbackContext(Context);
			return;
		}

		if (Self)
		{
			Self->HandlePlayerStateChanged(State);
		}
	});
}

void UPICOSpatialECSVideoPlayerComponent::RebuildVideoPlayerBinding()
{
	if (!HasNativeEntity())
	{
		return;
	}

	spatialpack::SceneEntity& Entity = GetCachedNativeEntity();
	auto VideoPlayerComponent = Entity.getOrAddVideoPlayerComponent();

	const float WidthM = FMath::Max(PlaneWidth, 0.01f) / 100.0f;
	const float HeightM = FMath::Max(PlaneHeight, 0.01f) / 100.0f;
	CachedMesh = spatialpack::MeshResource::createPlane(WidthM, HeightM, 0.0f);

	spatialpack::VideoMaterial VideoMaterial = spatialpack::VideoMaterial::create(ToSpMaterialBlendingMode(BlendingMode));
	VideoMaterial.setDepthTest(bDepthTest);
	VideoMaterial.setDepthWrite(bDepthWrite);
	VideoMaterial.setDimensionMode(ToSpVideoDimensionMode(DimensionMode));
	VideoMaterial.setTextureParameter({ spatialpack::TextureResource::getBuiltinTexture(spatialpack::BuiltinTextureId::BLACK) });
	CachedMaterial = VideoMaterial.toMaterial();

	VideoPlayerComponent.init(Player, CachedMesh, CachedMaterial);

	Player.setLoop(bLoop);
	PlaybackSpeed = FMath::Clamp(PlaybackSpeed, 0.1f, 4.0f);
	Volume = FMath::Clamp(Volume, 0.0f, 1.0f);
	Player.setPlaybackSpeed(PlaybackSpeed);
	Player.setVolume(Volume);
}

void UPICOSpatialECSVideoPlayerComponent::ClearPlayerCallbackContext()
{
	if (ActivePlayerCallbackContext)
	{
		ActivePlayerCallbackContext->bActive = false;
                UnregisterVideoPlayerCallbackContext(ActivePlayerCallbackContext.Get());
		ActivePlayerCallbackContext.Reset();
	}
}

void UPICOSpatialECSVideoPlayerComponent::CachePlaybackStateFromNative()
{
	if (!Player.isValid())
	{
		return;
	}

	LastKnownPositionMs = Player.getCurrentPosition();
	LastKnownDurationMs = Player.getDuration();
	bLastKnownPlaying = Player.isPlaying();
	bLastKnownComplete = Player.isComplete();
	LastKnownVideoWidth = Player.getVideoWidth();
	LastKnownVideoHeight = Player.getVideoHeight();
}

void UPICOSpatialECSVideoPlayerComponent::HandlePlayerStateChanged(spatialpack::MediaPlaybackState State)
{
	CachePlaybackStateFromNative();

	if (State == spatialpack::MediaPlaybackState::PREPARED)
	{
		Player.setLoop(bLoop);
		Player.setPlaybackSpeed(PlaybackSpeed);
		Player.setVolume(Volume);
		if (bAutoPlay && !bHasConsumedAutoPlay)
		{
			bHasConsumedAutoPlay = true;
			if (Player.play())
			{
				bLastKnownPlaying = true;
				bLastKnownComplete = false;
			}
		}
	}

	switch (State)
	{
	case spatialpack::MediaPlaybackState::STARTED:
		bLastKnownPlaying = true;
		bLastKnownComplete = false;
		break;
	case spatialpack::MediaPlaybackState::PAUSED:
	case spatialpack::MediaPlaybackState::STOPPED:
		bLastKnownPlaying = false;
		break;
	case spatialpack::MediaPlaybackState::COMPLETED:
		bLastKnownPlaying = false;
		bLastKnownComplete = true;
		break;
	case spatialpack::MediaPlaybackState::ERROR:
		bLastKnownPlaying = false;
		break;
	default:
		break;
	}
}
#endif
