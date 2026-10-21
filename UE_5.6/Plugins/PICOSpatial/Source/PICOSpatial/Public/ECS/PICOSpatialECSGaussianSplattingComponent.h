#pragma once

#include "CoreMinimal.h"
#include "ECS/PICOSpatialECSComponent.h"
#include "PICOSpatialECSGaussianSplattingComponent.generated.h"

class UPICOSpatialECSGaussianSplattingComponent;

#if PLATFORM_ANDROID
struct FPICOSpatialECSGaussianAsyncLoadContext
{
	TWeakObjectPtr<UPICOSpatialECSGaussianSplattingComponent> Component;
	uint64 Generation = 0;
	spatialpack::AsyncJobId ExpectedJobId = 0;
	bool bActive = false;
};
#endif

UCLASS(Blueprintable, BlueprintType, EditInlineNew)
class PICOSPATIAL_API UPICOSpatialECSGaussianSplattingComponent : public UPICOSpatialECSComponent
{
	GENERATED_BODY()

public:
	virtual EPICOSpatialECSComponentType GetComponentType() const override
	{
		return EPICOSpatialECSComponentType::GaussianSplatting;
	}

	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Gaussian Splatting Component")
	void setPath(const FString& InFilePath);

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Gaussian Splatting Component")
	bool reload();

	UFUNCTION(BlueprintCallable, Category = "PICO Spatial|ECS|Gaussian Splatting Component")
	void clearResource();

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Gaussian Splatting Component")
	FString getPath() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Gaussian Splatting Component")
	bool isResourceValid() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Gaussian Splatting Component")
	int64 getResourceId() const;

	UFUNCTION(BlueprintPure, Category = "PICO Spatial|ECS|Gaussian Splatting Component")
	bool isLoadPending() const;

#if PLATFORM_ANDROID
public:
	void ReleaseNativeResource();

protected:
	virtual void OnInitializeNativeComponent(spatialpack::SceneEntity& InNativeEntity) override;
	virtual void ApplyAllPropertiesToNative() override;

private:
	bool ReloadNativeResource();
	bool StartAsyncLoad(const FString& ResolvedUri, const FString& ResourceName, bool bInLoadFromAssets);
	bool StartAsyncLoadAttempt(int32 AttemptIndex);
	void CancelPendingLoadJob();
	void FinalizeLoadedResource(const spatialpack::GaussianSplattingResource& InResource);
	void CompleteActiveAsyncLoadContext();
	static void OnAsyncGaussianLoaded(spatialpack::AsyncJobId JobId, void* User, const spatialpack::GaussianSplattingResource& Resource, const spatialpack::ResourceErrorInfo& ErrorInfo);
	void BindCurrentResource();

	spatialpack::GaussianSplattingResource GaussianResource;
	spatialpack::GaussianSplattingResourceAsyncLoadJob* PendingLoadJob = nullptr;
	spatialpack::AsyncJobId PendingLoadJobId = 0;
	int32 PendingLoadAttemptIndex = INDEX_NONE;
	uint64 AsyncLoadGeneration = 0;
	TSharedPtr<FPICOSpatialECSGaussianAsyncLoadContext> ActiveAsyncLoadContext;
	bool bPendingLoadFromAssets = false;
	FString PendingResolvedUri;
	FString PendingResourceName;
	FString PendingAlternateUri;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaussian Splatting Component", meta = (DisplayName = "Gaussian File Path", ToolTip = "ModelViewer-style: you can type just a name like cotton, cotton.ply, or demo_fox_gs. Runtime resolves under AssetsPICO/UserCustomAssets. When 'Load Assets From APK Assets' is enabled it loads from APK assets, otherwise it keeps using the storage-based flow."))
	FString Path;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaussian Splatting Component", meta = (DisplayName = "Use Async Load", ToolTip = "If enabled on Android, the component uses GaussianSplattingResource async loading and binds the resource when the callback arrives. Disable this to force synchronous loading for debugging."))
	bool bUseAsyncLoad = true;
};
