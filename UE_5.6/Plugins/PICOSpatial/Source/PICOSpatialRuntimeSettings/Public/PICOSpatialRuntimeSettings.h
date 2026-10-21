// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPath.h"
#include "Engine/EngineTypes.h"
#include "PICOSpatialRuntimeSettings.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPICOSpatialRuntimeSettings, Log, All);

/** Enumeration for different spatial container types in PICO Spatial */
UENUM(BlueprintType)
enum class ESpatialContainerTypePICO : uint8
{
	/** Default 2D container mode for traditional flat displays */
	Default2D,
	/** Bounded container with fixed dimensions and boundaries */
	Bounded,
	/** Unbounded container with infinite spatial extent */
	Unbounded,
};

/** Enumeration for unbounded spatial container visual styles */
UENUM(BlueprintType)
enum class EUnboundedStylePICO : uint8
{
	/** Automatic style selection based on system status */
	AUTOMATIC = 0,
	/** Mixed reality style - displays virtual content with video see-through (VST), excluding skybox */
	MIXED = 1,
	/** Progressive style - displays virtual content, skybox, and VST with adjustable immersion level */
	PROGRESSIVE = 2,
	/** Full immersion style - displays virtual content and skybox without VST */
	FULL_IMMERSION = 3,
};

/**
 * Runtime configuration settings for PICO Spatial plugin.
 * Controls spatial rendering, asset conversion, and container behavior.
 */
UCLASS(config = Engine, defaultconfig)
class PICOSPATIALRUNTIMESETTINGS_API UPICOSpatialRuntimeSettings : public UObject
{
public:
	GENERATED_UCLASS_BODY()

	/** Called after object initialization */
	virtual void PostInitProperties() override;
	
	/** Configures PICO Spatial support based on current settings */
	void HandlePICOSpatialSupport();

#if WITH_EDITOR
	/** Called when a property is changed in the editor */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

	// General Settings
	
	/** Whether to automatically start the application in PICO Spatial mode */
	UPROPERTY(config, EditAnywhere, Category = "General", meta = (DisplayName = "Start In PICO Spatial", ToolTip = "Automatically launches the application in PICO Spatial mode on startup"))
	bool bStartInPICOSpatial = false;

	// Asset Conversion Settings
	
	/** Cache all asset dependencies when the level loads for improved performance */
	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (DisplayName = "Pre-Cache Assets Per Level", ToolTip = "Caches all asset dependencies on level load for better runtime performance"))
	bool bPreCacheAssetsPerLevel = true;

	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (EditCondition = "bPreCacheAssetsPerLevel", DisplayName = "Keep Cached Assets All Levels", ToolTip = "Do not reset cached asset when switching level. This will increase memory usage but faster when switching to an opened level."))
	bool bKeepCachedAssetsAllLevels = true;

	/** Load exported non-uasset spatial assets directly from the Android APK assets directory instead of the app's writable storage. */
	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (DisplayName = "Load Assets From APK Assets", ToolTip = "When enabled on Android, non-uasset exports such as .glb, .gltf, .usd, .usda, .usdz and texture exports like .PNG/.EXR are written under Build/Android/assets/AssetsPICO and loaded directly from the packaged APK assets directory. When disabled, the original Content/AssetsPICO to Saved/AssetsPICO storage flow is preserved. UAsset-based content keeps using the original Content/AssetsPICO flow"))
	bool bLoadAssetsFromApkAssets = true;

	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (AllowedClasses = "/Script/Engine.World", DisplayName = "Cook Target Worlds", ToolTip = "Optional list of worlds to convert when cooking PICO Spatial content. When empty, the editor automatically scans /Game for worlds that depend on an APICOSpatialContainer-derived spatial container. The project Game Default Map is also considered, but every cooked world must reference an APICOSpatialContainer-derived spatial container, including through child-world dependencies such as sublevels or level instances."))
	TArray<FSoftObjectPath> CookTargetWorlds;

	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (AllowedClasses = "/Script/PICOSpatial.PICOSpatialAssetMappingsAsset", DisplayName = "Asset Mappings Asset", ToolTip = "Optional manual mappings from UE asset path to native bundle entry path. When set, mapped assets are loaded from pico_spatial_mappings.bundle first, regardless of whether the bundle tool is enabled."))
	FSoftObjectPath AssetMappingsAsset;

	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (DisplayName = "Use Only Manual Asset Mappings", ToolTip = "When enabled, the editor does not export mesh, material or texture native resources or generate bundles, while Niagara assets still convert normally. Runtime does not fall back to generated bundle or exported files for mapped native resources. Mesh, material and texture resources must come from Asset Mappings Asset and pico_spatial_mappings.bundle."))
	bool bUseOnlyManualAssetMappings = false;

	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (DisplayName = "Use Bundle Tool", ToolTip = "When enabled, auto-exported spatial assets are packed into Build/Android/assets/pico_spatial.bundle. Runtime still falls back to loose exported files when a bundle entry is unavailable, so custom loose model loading remains supported."))
	bool bUsedBundleTool = false;

	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (EditCondition = "bUsedBundleTool", DisplayName = "Bundle Tool", ToolTip = "Path to the bundle tool"))
	FFilePath BundleTool;

	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (EditCondition = "bUsedBundleTool", DisplayName = "Clear Assets After Bundle Conversion", ToolTip = "Automatically remove the intermediate .usd .usda .glb .PNG/.EXR etc after bundle conversion"))
	bool bClearAssetsAfterBundleConversion = true;

	/** Pre-convert Texture2D assets during export, using PNG first and EXR as fallback when needed */
	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (EditCondition = "bUsedBundleTool", DisplayName = "Export USD Shader Graph Material", ToolTip = "Material shader graph will be exported as usd or usda, and also export textures. This will increase app size mainly due to the textures."))
	bool bExportUsdShaderGraphMaterial = false;

	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (EditCondition = "bUsedBundleTool && bExportUsdShaderGraphMaterial", AdvancedDisplay, DisplayName = "Export Unconnected USD Shader Graph Nodes", ToolTip = "Debug option. When enabled, USD shader graph conversion also tries to export material expression nodes that are not reachable from material outputs."))
	bool bExportUnconnectedUsdShaderGraphNodes = false;

	UPROPERTY(config, EditAnywhere, Category = "Assets Converter", meta = (EditCondition = "bUsedBundleTool && bExportUsdShaderGraphMaterial", ConsoleVariable = "PICO.UseUsdBinaryFormat", DisplayName = "Use USD Binary Format (.usd)", ToolTip = "True for binary .usd format, false for ASCII .usda format"))
	bool bUseUsdBinaryFormat = true;

	// Spatial Application Settings
	
	/** Type of spatial container for the application */
	UPROPERTY(config, EditAnywhere, Category = "Spatial APP", meta = (DisplayName = "Spatial Container Type", ToolTip = "Defines the type and behavior of the spatial container"))
	ESpatialContainerTypePICO SpatialContainerTypePICO = ESpatialContainerTypePICO::Default2D;

	/** Automatically hide the window caption bar in spatial mode */
	UPROPERTY(config, EditAnywhere, Category = "Spatial APP", meta = (EditCondition = "SpatialContainerTypePICO != ESpatialContainerTypePICO::Default2D", DisplayName = "Auto Hide Caption Bar", ToolTip = "Hides the caption bar automatically when using spatial containers"))
	bool bAutoHideCaptionBar = false;

	/** Visual style for unbounded spatial containers */
	UPROPERTY(config, EditAnywhere, Category = "Spatial APP", meta = (DisplayName = "Unbounded Style", EditCondition = "SpatialContainerTypePICO == ESpatialContainerTypePICO::Unbounded", ToolTip = "Defines the visual presentation style for unbounded containers"))
	EUnboundedStylePICO UnboundedStylePICO = EUnboundedStylePICO::AUTOMATIC;

	/** Display name for the spatial container */
	//UPROPERTY(config, EditAnywhere, Category = "Spatial APP", meta = (DisplayName = "Container Name", EditCondition = "SpatialContainerTypePICO != ESpatialContainerTypePICO::Default2D", ToolTip = "Custom display name for the spatial container"))
	FString SpatialContainerName = "UEPICOSpatial";

	/** Size dimensions for bounded spatial containers (in centimeters) */
	UPROPERTY(config, EditAnywhere, Category = "Spatial APP", meta = (DisplayName = "Container Size", EditCondition = "SpatialContainerTypePICO == ESpatialContainerTypePICO::Bounded", ToolTip = "Physical dimensions of the bounded container in centimeters"))
	FVector SpatialContainerSize = FVector(100.0f, 100.0f, 100.0f);
	
	/** Automatically scale content to fit within container bounds */
	UPROPERTY(config, EditAnywhere, Category = "Spatial APP", meta = (EditCondition = "SpatialContainerTypePICO == ESpatialContainerTypePICO::Bounded", DisplayName = "Scale Content To Container", ToolTip = "Scales content automatically to fit within the spatial container bounds"))
	bool bScaleContentToSpatialContainer = true;

	/** Enable grounding shadow for enhanced visual grounding */
	UPROPERTY(config, EditAnywhere, Category = "Spatial APP", meta = (ConsoleVariable = "PICO.EnableGroundingShadow", DisplayName = "Enable Grounding Shadow", ToolTip = "Enables grounding shadow. Lower cost than dynamic shadow"))
	bool bEnableGroundingShadow = true;

	/** Enable dynamic shadow for enhanced visual dynamic */
	UPROPERTY(config, EditAnywhere, Category = "Spatial APP", meta = (ConsoleVariable = "PICO.EnableDynamicShadow", DisplayName = "Enable Dynamic Shadow", ToolTip = "Enables Dynamic shadow. When enabled, uses dynamic shadows with higher quality but increased performance cost."))
	bool bEnableDynamicShadow = false;

	/**
	 * When enabled, PICO Spatial takes over rendering for all primitive components associated with
	 * entity proxies: UE main-pass rendering is disabled (bRenderInMainPass = false) and rendering
	 * is handled by the PICO Spatial proxy system instead.
	 * When disabled, PICO Spatial does not interfere with UE rendering at all - each component
	 * keeps its original bRenderInMainPass value unchanged.
	 * Default: true.
	 */
	UPROPERTY(config, EditAnywhere, Category = "Spatial APP", meta = (ConsoleVariable = "PICO.OverrideRenderInMainPass", DisplayName = "Override Render In Main Pass", ToolTip = "When enabled, PICO Spatial disables UE main-pass rendering for spatial primitive components and uses proxy rendering instead. When disabled, UE rendering is not interfered with."))
	bool bOverrideRenderInMainPass = false;

	/** Keep existing entity proxies alive while a spatial container is in background. */
	UPROPERTY(config, EditAnywhere, Category = "Spatial APP", meta = (ConsoleVariable = "PICO.KeepBackgroundContainerEntityProxiesAlive", DisplayName = "Keep Background Container Entity Proxies Alive", ToolTip = "When enabled, background spatial containers keep their previous entity proxies alive without actor scanning or native updates until the background timeout is reached. Disabled by default so background containers release their proxies through the normal dead-proxy cleanup path."))
	bool bKeepBackgroundContainerEntityProxiesAlive = false;

	// Default Window Settings
	
	/** Allow the default window to receive input events */
	UPROPERTY(config, EditAnywhere, Category = "Default Window", meta = (DisplayName = "Receive Input", ToolTip = "Enables input event processing for the default window"))
	bool bReceiveInput = true;

	/** Enable the default window system */
	UPROPERTY(config, EditAnywhere, Category = "Default Window", meta = (DisplayName = "Custom Default Window", ToolTip = "Custom the default 2D window system alongside spatial rendering"))
	bool bCustomDefaultWindow = false;

	UPROPERTY(config, EditAnywhere, Category = "Default Window", meta = (EditCondition = "bCustomDefaultWindow"))
	bool bTransparentDefaultWindow = false;

	/** Alpha transparency percentage for the default window (0-100) */
	UPROPERTY(config, EditAnywhere, Category = "Default Window", meta = (EditCondition = "bCustomDefaultWindow && bTransparentDefaultWindow", UIMin = 0, ClampMin = 0, UIMax = 100, ClampMax = 100, DisplayName = "Window Alpha Percentage", ToolTip = "Controls window transparency: 0 = fully transparent, 100 = fully opaque"))
	int32 WindowAlphaPercentage = 100;

	/** Horizontal position offset for the default window */
	UPROPERTY(config, EditAnywhere, Category = "Default Window", meta = (EditCondition = "bCustomDefaultWindow", DisplayName = "Offset Horizontal", ToolTip = "Horizontal position offset for the default window in pixels"))
	int32 OffsetHorizontal = 0;

	/** Vertical position offset for the default window */
	UPROPERTY(config, EditAnywhere, Category = "Default Window", meta = (EditCondition = "bCustomDefaultWindow", DisplayName = "Offset Vertical", ToolTip = "Vertical position offset for the default window in pixels"))
	int32 OffsetVertical = 0;

	/** Use custom size settings for the default window */
	UPROPERTY(config, EditAnywhere, Category = "Default Window", meta = (EditCondition = "bCustomDefaultWindow", DisplayName = "Custom Window Size", ToolTip = "Enables custom width and height settings for the default window"))
	bool bCustomDefaultWindowSize = false;

	/** Custom width for the default window */
	UPROPERTY(config, EditAnywhere, Category = "Default Window", meta = (EditCondition = "bCustomDefaultWindow && bCustomDefaultWindowSize", DisplayName = "Custom Width", ToolTip = "Custom width for the default window in pixels"))
	int32 DefaultWindowCustomWidth = 100;

	/** Custom height for the default window */
	UPROPERTY(config, EditAnywhere, Category = "Default Window", meta = (EditCondition = "bCustomDefaultWindow && bCustomDefaultWindowSize", DisplayName = "Custom Height", ToolTip = "Custom height for the default window in pixels"))
	int32 DefaultWindowCustomHeight = 100;

	// Internal State (Hidden from editor)
	
	/** Internal: Last known state of UseDefaultWindow */
	UPROPERTY(config)
	bool bLastCustomDefaultWindow = true;

	/** Internal: Flag for resetting UseDefaultWindow setting */
	UPROPERTY(config)
	bool bResetCustomDefaultWindow = false;
};
