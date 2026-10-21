#pragma once

#include "CoreMinimal.h"
#include "ECS/PICOSpatialECSComponent.h"
#include "PICOSpatialECSVideoPlayerComponent.generated.h"

class UPICOSpatialECSVideoPlayerComponent;

#if PLATFORM_ANDROID
struct FPICOSpatialECSVideoPlayerCallbackContext
{
	TWeakObjectPtr<UPICOSpatialECSVideoPlayerComponent> Component;
	uint64 Generation = 0;
	bool bActive = false;
};
#endif

UENUM(BlueprintType)
enum class EPICOSpatialVideoDimensionMode : uint8
{
	Mono,
	SideBySide,
	MultipleView,
};

UENUM(BlueprintType)
enum class EPICOSpatialVideoBlendingMode : uint8
{
	Opaque,
	Transparent,
};

UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class PICOSPATIAL_API UPICOSpatialECSVideoPlayerComponent : public UPICOSpatialECSComponent
{
	GENERATED_BODY()

public:
	virtual EPICOSpatialECSComponentType GetComponentType() const override
	{
		return EPICOSpatialECSComponentType::VideoPlayer;
	}

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void setDataSourcePath(const FString& InFilePath);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void setDataSourceName(const FString& InVideoName);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void prepareAsync();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void play();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void pause();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void resume();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void stop();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void reset();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void seekTo(int64 InPositionMs);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void setLoop(bool bInLoop);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void setPlaybackSpeed(float InSpeed);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Video Player Component")
	void setVolume(float InVolume);

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Video Player Component")
	FString getDataSourcePath() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Video Player Component")
	bool isPlayerValid() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Video Player Component")
	int64 getCurrentPositionMs() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Video Player Component")
	int64 getDurationMs() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Video Player Component")
	bool isPlaying() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Video Player Component")
	bool isComplete() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Video Player Component")
	int32 getVideoWidth() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Video Player Component")
	int32 getVideoHeight() const;

	virtual void BeginDestroy() override;

#if PLATFORM_ANDROID
public:
	void ReleaseNativePlayer();

protected:
	virtual void OnInitializeNativeComponent(spatialpack::SceneEntity& InNativeEntity) override;
	virtual void ApplyAllPropertiesToNative() override;

private:
	void EnsureNativeBindings();
	void EnsurePlayer();
	void RebuildVideoPlayerBinding();
	void ClearPlayerCallbackContext();
	void HandlePlayerStateChanged(spatialpack::MediaPlaybackState State);
	void CachePlaybackStateFromNative();

	spatialpack::CypressMediaPlayer Player;
	spatialpack::MeshResource CachedMesh;
	spatialpack::Material CachedMaterial;
	uint64 PlayerCallbackGeneration = 0;
	bool bHasConsumedAutoPlay = false;
	TSharedPtr<FPICOSpatialECSVideoPlayerCallbackContext> ActivePlayerCallbackContext;
#endif

	int64 LastKnownPositionMs = 0;
	int64 LastKnownDurationMs = 0;
	bool bLastKnownPlaying = false;
	bool bLastKnownComplete = false;
	int32 LastKnownVideoWidth = 0;
	int32 LastKnownVideoHeight = 0;

	// Video source path. Use an absolute file path, a path relative to Saved, or a file name from Content/Movies.
	// The ".mp4" extension is optional, e.g. "Intro" resolves like "Intro.mp4".
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component", meta = (ToolTip = "Video source path. Use an absolute file path, a path relative to Saved, or a file name from Content/Movies. The .mp4 extension is optional, for example Intro resolves like Intro.mp4."))
	FString DataSourcePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float PlaneWidth = 280.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component", meta = (ClampMin = "0.01", UIMin = "0.01"))
	float PlaneHeight = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component")
	EPICOSpatialVideoDimensionMode DimensionMode = EPICOSpatialVideoDimensionMode::Mono;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component")
	EPICOSpatialVideoBlendingMode BlendingMode = EPICOSpatialVideoBlendingMode::Transparent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component")
	bool bDepthTest = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component")
	bool bDepthWrite = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component")
	bool bAutoPrepare = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component")
	bool bAutoPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component")
	bool bLoop = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component", meta = (ClampMin = "0.1", ClampMax = "4.0", UIMin = "0.1", UIMax = "4.0"))
	float PlaybackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Video Player Component", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float Volume = 1.0f;
};
