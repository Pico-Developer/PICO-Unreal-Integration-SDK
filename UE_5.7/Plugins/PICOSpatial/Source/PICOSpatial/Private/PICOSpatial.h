// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PICOSpatialParticleEntity.h"
#include "PICOSpatialFunctionLibrary.h"
#include "PICOSpatialStaticMeshEntity.h"
#include "PICOSpatialShapeEntity.h"
#include "PICOSpatialInstancedStaticMeshEntity.h"
#include "PICOSpatialTextEntity.h"
#include "PICOSpatialCustomSceneEntity.h"
#include "PICOSpatialSkeletalMeshEntity.h"
#include "PICOSpatialDynamicMeshEntity.h"
#include "PICOSpatialWidgetEntity.h"
#include "PICOSpatialSceneCaptureEntity.h"
#include "PICOSpatialDirectionalLightEntity.h"

// Forward declarations
class APICOSpatialContainer;
class USceneComponent;
using FEntityProxyKey = uint64;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnWindowContainerCreatedDelegate, uint64 /*AppFormID*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWindowContainerDestroyedDelegate, uint64 /*AppFormID*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWindowContainerForegroundDelegate, uint64 /*AppFormID*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnWindowContainerBackgroundDelegate, uint64 /*AppFormID*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWindowContainerFocusDelegate, uint64 /*AppFormID*/, bool /*bHasFocus*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWindowContainerSightDelegate, uint64 /*AppFormID*/, bool /*bIsOnSight*/);

/**
 * Tracks which SDK frame-boundary call was last made.
 * Valid sequence per frame: Idle -> Waited -> Begun -> Idle (via endFrame).
 */
enum class EFramePhase : uint8
{
	/** Initial state, or after endFrame completed */
	Idle,
	/** After waitFrame completed */
	Waited,
	/** After beginFrame completed */
	Begun,
};

/**
 * PICO Spatial manager class.
 * Manages spatial session, scene entities, and rendering integration with PICO Spatial SDK.
 */
class FPICOSpatial
{
public:
	FPICOSpatial();
	virtual ~FPICOSpatial();

	/** Get singleton instance */
	static FPICOSpatial& Get()
	{
		static FPICOSpatial PICOSpatialPICO;
		return PICOSpatialPICO;
	}

	/** Initialize PICO Spatial SDK and register delegates */
	void Register();
	
	/** Cleanup and unregister delegates */
	void Unregister();

	/** Enable or disable PICO Spatial mode */
	bool EnablePICOSpatial(bool bEnable, bool bClearAllNativeCachedResources = false);

	/** Get whether PICO Spatial is currently enabled */
	bool IsPICOSpatialEnabled() const;
	
	/** Enable or disable default window input */
	bool EnableDefaultWindowInput(bool bEnable);
	
	/** Modify default window properties */
	bool ModifyDefaultWindow(int32 Alpha, bool bCustomSize, int32 Width, int32 Height, int32 OffsetX, int32 OffsetY);

	/** Pre-cache assets for a level */
	void PreCacheResources(const TMap<FString, EAssetTypePICO>& Dependencies);
	
	/** Cache mesh resource by asset path */
	spatialpack::MeshResource* CacheMeshResource(const FString& MeshAsset);
        spatialpack::MeshResource* CacheMappedMeshResource(const FString& CacheKey, const FString& BundleEntryPath);
	
	/** Cache material resource by asset path */
	spatialpack::Material* CacheMaterialResource(const FString& Material, const FString& DynamicMaterialInstance);
        spatialpack::Material* CacheMappedMaterialResource(const FString& CacheKey, const FString& BundleEntryPath, const FString& DynamicMaterialInstance = FString());

	void AddDynamicMaterialInstanceReference(const FString& Material);
	bool RemoveDynamicMaterialInstance(const FString& Material, bool bForceRemove = false);

	/** Cache texture resource by asset path */
	spatialpack::TextureResource* CacheTextureResource(const FString& Texture);

#if PLATFORM_ANDROID
	spatialpack::AssetBundle* GetAssetBundle() const
	{
		return AssetBundle;
	}

        spatialpack::AssetBundle* GetAssetMappingsBundle();
#endif

	/** Called at the start of world tick */
	void OnWorldTickStart(UWorld* World, ELevelTick TickType, float DeltaTime);
	
	/** Called at the end of world tick */
	void OnWorldTickEnd(UWorld* InWorld, ELevelTick InTickType, float InDeltaSeconds);
	
	/** Called before loading a new map */
	void OnPreLoadMap(const FString& MapName);
	
	/** Called after loading a new map */
	void OnPostLoadMap(UWorld* LoadedWorld);

	/** Handle instanced static mesh component instance index updates */
	void HandleStaticMeshComponentInstanceIndexUpdated(UInstancedStaticMeshComponent* InComponent, TArrayView<const FInstancedStaticMeshDelegates::FInstanceIndexUpdateData> InIndexUpdates);

	/** Create entity proxy for scene component */
	FSceneEntityProxy* CreateEntityProxy(FSceneEntityProxy* RootSceneEntityProxy, USceneComponent* SceneComponent, const EEntityProxyType& Type);
	
	/** Destroy entity proxy by ID and type */
	void DestroyEntityProxy(FEntityProxyKey ID, const EEntityProxyType& Type);
	
	/** Destroy all entity proxies */
	void DestroyAllEntityProxies();
	
	/** Find entity proxy by ID and type */
	FSceneEntityProxy* FindEntityProxy(FEntityProxyKey ID, const EEntityProxyType& Type);

	/** Build a collision-free proxy key from a component and its container/root owner. */
	static FEntityProxyKey MakeEntityProxyKey(uint32 ComponentID, uint32 RootOwnerID)
	{
		return (static_cast<uint64>(ComponentID) << 32) | static_cast<uint64>(RootOwnerID);
	}

	/** Poll and process input events from spatial scene */
	void PollAndProcessInputEvent();

	/** Copy texture on render thread */
	void CopyTexture_RenderThread(FRHICommandListImmediate& RHICmdList, FRHITexture* SrcTexture, FIntRect SrcRect, FRHITexture* DstTexture, FIntRect DstRect,
		bool bClearBlack, ERenderTargetActions RTAction, ERHIAccess FinalDstAccess, ETextureCopyBlendModifier SrcTextureCopyModifier) const;
	
	/** Initialize render target */
	void InitRT(UTextureRenderTarget2D* T);

	/** Whether PICO Spatial is enabled */
	bool bPICOSpatialEnabled = false;
	
	/** Whether PICO Spatial is currently running */
	bool bPICOSpatialRunning = false;
	
	/** Whether spatial session has started */
	bool bSessionStarted = false;

	/** Current SDK frame boundary phase; used by AdvanceFrameTo() */
	EFramePhase FramePhase = EFramePhase::Idle;

	/** Handle to spatial pack loader library */
	void* LoaderHandle = nullptr;
	
	/** PICO Spatial instance */
	spatialpack::SpatialInstance* Instance = nullptr;
	
	/** PICO Spatial session */
	spatialpack::Session* Session = nullptr;
	
	/** Current app scene */
	spatialpack::AppScene* CurrentScene = nullptr;

	/** Delegate handle for world tick start */
	FDelegateHandle OnWorldTickStartDelegateHandle;

	/** Delegate handle for world tick end */
	FDelegateHandle OnWorldTickEndDelegateHandle;

	/** Delegate handle for pre-load map */
	FDelegateHandle OnPreLoadMapDelegateHandle;

	/** Delegate handle for post-load map */
	FDelegateHandle OnPostLoadMapDelegateHandle;

	/** Delegate handle for instanced static mesh index updated */
	FDelegateHandle OnInstanceIndexUpdatedDelegateHandle;

	/** Whether runtime delegates are currently registered */
	bool bRuntimeDelegatesRegistered = false;

	/** Whether the current platform supports PICO Spatial runtime */
	bool bSpatialPlatformAvailable = false;

	/** Whether native window callbacks are currently registered */
	bool bWindowCallbacksRegistered = false;

	/** Map from native entity UUID to scene entity proxy */
	TMap<spatialpack::EntityUUID, FSceneEntityProxy*> NativeEntityToSceneComponentMap;

	/** Entity proxies that were alive in the previous frame */
	TMap<FEntityProxyKey, EEntityProxyType> PreviousFrameLivingEntityProxies;
	
	/** Entity proxies that are alive in the current frame */
	TMap<FEntityProxyKey, EEntityProxyType> CurrentFrameLivingEntityProxies;

	/** Scene entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FSceneEntityProxy>> SceneEntityProxies_GameThread;

	/** Custom scene entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FCustomSceneEntityProxy>> CustomSceneEntityProxies_GameThread;
	
	/** Directional light entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FDirectionalLightEntityProxy>> DirectionalLightEntityProxies_GameThread;
	
	/** Static mesh entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FStaticMeshEntityProxy>> StaticMeshEntityProxies_GameThread;
	
	/** Shape entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FShapeEntityProxy>> ShapeEntityProxies_GameThread;
	
	/** Instanced static mesh entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FInstancedStaticMeshEntityProxy>> InstancedStaticMeshEntityProxies_GameThread;
	
	/** Skeletal mesh entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FSkeletalMeshEntityProxy>> SkeletalMeshEntityProxies_GameThread;
	
	/** Dynamic mesh entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FDynamicMeshEntityProxy>> DynamicMeshEntityProxies_GameThread;
	
	/** Text entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FTextEntityProxy>> TextEntityProxies_GameThread;
	
	/** Widget entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FWidgetEntityProxy>> WidgetProxies_GameThread;
	
	/** Scene capture entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FSceneCaptureEntityProxy>> SceneCaptureProxies_GameThread;
	
	/** Particle entity proxies on game thread */
	TMap<FEntityProxyKey, TSharedPtr<FParticleEntityProxy>> ParticleProxies_GameThread;

	/** Active spatial containers */
	TSet<APICOSpatialContainer*> Containers;

	/** Delegate for window container created event */
	static FOnWindowContainerCreatedDelegate OnWindowContainerCreatedDelegate;
	
	/** Delegate for window container destroyed event */
	static FOnWindowContainerDestroyedDelegate OnWindowContainerDestroyedDelegate;
	
	/** Delegate for window container foreground event */
	static FOnWindowContainerForegroundDelegate OnWindowContainerForegroundDelegate;
	
	/** Delegate for window container background event */
	static FOnWindowContainerBackgroundDelegate OnWindowContainerBackgroundDelegate;
	
	/** Delegate for window container focus event */
	static FOnWindowContainerFocusDelegate OnWindowContainerFocusDelegate;
	
	/** Delegate for window container sight event */
	static FOnWindowContainerSightDelegate OnWindowContainerSightDelegate;

private:
#if PLATFORM_ANDROID
	/** Asset bundle for PICO Spatial assets */
	spatialpack::AssetBundle* AssetBundle = nullptr;
	spatialpack::AssetBundle* AssetMappingsBundle = nullptr;

	friend class UPICOSpatialGlbUsdViewer;
	/** Load asset from storage */
	spatialpack::SceneEntity LoadAsset(FString Name, EAssetTypePICO Type, bool bDirectReturnEntity = false);

	/** Log native resource load failures once per resource and mode. */
	void LogNativeResourceLoadFailure(EAssetTypePICO Type, const FString& UEAssetPath, const FString& MappedAssetPath, bool bUsedMapping, bool bUseOnlyManualMappings);

	/** Avoid repeated expensive load attempts for missing native resources. */
	bool IsNativeResourceLoadFailureCoolingDown(EAssetTypePICO Type, const FString& UEAssetPath, const FString& MappedAssetPath, bool bUsedMapping, bool bUseOnlyManualMappings);
	
	/** Cached mesh resources */
	TMap<FString, spatialpack::MeshResource> MeshResources;
	
	/** Cached material resources */
	TMap<FString, spatialpack::Material> MaterialResources;

	/** Cached dynamic material instance resources */
	TMap<FString, spatialpack::Material> DynamicMaterialInstanceResources;

	/** Reference counts for dynamic material instance clone resources. */
	TMap<FString, int32> DynamicMaterialInstanceRefCounts;
	
	/** Cached texture resources */
	TMap<FString, spatialpack::TextureResource> TextureResources;

	/** Resource load failure keys that have already been reported to avoid log spam. */
	TSet<FString> ReportedNativeResourceLoadFailures;

	/** Next time a failed resource is allowed to retry loading. */
	TMap<FString, double> NativeResourceFailureRetryTimes;

	spatialpack::SceneEntity FindFirstEntityWithModelComponent(const spatialpack::SceneEntity& entity);
#endif

	/** Callback for window container created */
	static void WindowContainerOnCreatedCallback(spatialpack::AppFormId Id);
	
	/** Callback for window container destroyed */
	static void WindowContainerOnDestroyedCallback(spatialpack::AppFormId Id);
	
	/** Callback for window container foreground */
	static void WindowContainerOnForegroundCallback(spatialpack::AppFormId Id);
	
	/** Callback for window container background */
	static void WindowContainerOnBackgroundCallback(spatialpack::AppFormId Id);
	
	/** Callback for window container focus */
	static void WindowContainerOnFocusCallback(spatialpack::AppFormId Id, bool bHasFocus);
	
	/** Callback for window container sight */
	static void WindowContainerOnSightCallback(spatialpack::AppFormId Id, bool bIsOnSight);

	bool DecompressAssetsPICO();
	bool InitializeRuntime();
	void ShutdownRuntime(bool bClearAllNativeCachedResources);
	void DestroyRuntimeObjects(bool bClearAllNativeCachedResources);
	void RegisterRuntimeDelegates();
	void UnregisterRuntimeDelegates();
	void RegisterWindowCallbacks();
	void UnregisterWindowCallbacks();
	void ResetNativeCachedResources();
	void EnsureRuntimeAssetBundlesLoaded();

	/**
	 * Advance the SDK frame-phase state machine to TargetPhase.
	 * Any intermediate phases that have been skipped are auto-called in order:
	 *   Idle  -> WaitFrame  -> BeginFrame  -> EndFrame -> Idle
	 * Calling with a phase that is already past in the current frame is a no-op
	 * (the caller should never request going backwards).
	 */
	void AdvanceFrameTo(EFramePhase TargetPhase);
};
