#include "PICONiagaraConverter.h"
#include "PICONiagaraJsonExporter.h"
#include "NiagaraToPICOShared.h"
#include "PICOSpatialNiagaraAsset.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraScriptSource.h"
#include "NiagaraGraph.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraMeshRendererProperties.h"
#include "PICONiagaraPaths.h"
#include "FileHelpers.h"
#include "UObject/SavePackage.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/IConsoleManager.h"


#define LOCTEXT_NAMESPACE "FPICONiagaraConverter"

DEFINE_LOG_CATEGORY_STATIC(LogPICONiagaraConverter, Log, All);

namespace
{
bool ShouldSilentSaveSourceNiagaraBeforeConvert()
{
	static IConsoleVariable* CVarSilentSaveBeforeConvert =
		IConsoleManager::Get().FindConsoleVariable(TEXT("PICO.Niagara.SilentSaveBeforeConvert"));
	return CVarSilentSaveBeforeConvert && CVarSilentSaveBeforeConvert->GetInt() != 0;
}

ENiagaraPICO_FacingMode ToPICOFacingMode(ENiagaraSpriteFacingMode FacingMode)
{
	switch (FacingMode)
	{
	case ENiagaraSpriteFacingMode::FaceCameraPlane:
		return ENiagaraPICO_FacingMode::CameraPlane;
	case ENiagaraSpriteFacingMode::FaceCameraPosition:
	case ENiagaraSpriteFacingMode::FaceCameraDistanceBlend:
		return ENiagaraPICO_FacingMode::CameraPosition;
	case ENiagaraSpriteFacingMode::FaceCamera:
		return ENiagaraPICO_FacingMode::CameraPosition;
	case ENiagaraSpriteFacingMode::CustomFacingVector:
		return ENiagaraPICO_FacingMode::Free;
	case ENiagaraSpriteFacingMode::Automatic:
	default:
		return ENiagaraPICO_FacingMode::CameraPosition;
	}
}

ENiagaraPICO_FacingMode ToPICOFacingMode(ENiagaraMeshFacingMode FacingMode)
{
	switch (FacingMode)
	{
	case ENiagaraMeshFacingMode::CameraPosition:
		return ENiagaraPICO_FacingMode::CameraPosition;
	case ENiagaraMeshFacingMode::CameraPlane:
		return ENiagaraPICO_FacingMode::CameraPlane;
	case ENiagaraMeshFacingMode::Default:
	case ENiagaraMeshFacingMode::Velocity:
	default:
		return ENiagaraPICO_FacingMode::Free;
	}
}
}

// ========== Main Conversion Function ==========

bool FPICONiagaraConverter::ConvertNiagaraSystem(UNiagaraSystem* NiagaraSystem)
{
	if (!NiagaraSystem)
	{
		UE_LOG(LogPICONiagaraConverter, Error, TEXT("Cannot convert null Niagara system"));
		return false;
	}

	UE_LOG(LogPICONiagaraConverter, Log, TEXT("Starting conversion of Niagara system: %s"), *NiagaraSystem->GetName());

	// Warm editor state before conversion so Niagara editor data is up to date.
	if (!NiagaraToPICO::PrepareNiagaraSystemForConversion(NiagaraSystem))
	{
		UE_LOG(LogPICONiagaraConverter, Warning, TEXT("Pre-conversion Niagara preparation failed for: %s"), *NiagaraSystem->GetPathName());
	}

	if (ShouldSilentSaveSourceNiagaraBeforeConvert())
	{
		// Optionally save the source Niagara system before conversion.
		TArray<UPackage*> PackagesToSave;
		PackagesToSave.Add(NiagaraSystem->GetPackage());
		FEditorFileUtils::FPromptForCheckoutAndSaveParams SaveParams;
		SaveParams.bCheckDirty = false;
		SaveParams.bPromptToSave = false;
		SaveParams.bIsExplicitSave = true;
		FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, SaveParams);
	}

	// Create the PICO asset
	UPICOSpatialNiagaraAsset* PICOAsset = CreatePICOAsset(NiagaraSystem);
	if (!PICOAsset)
	{
		UE_LOG(LogPICONiagaraConverter, Error, TEXT("Failed to create PICO asset for: %s"), *NiagaraSystem->GetName());
		return false;
	}

	// Process system-level data
	TArray<UNiagaraScriptSourceBase*> SystemSources = NiagaraSystem->GetAllSourceScripts();
	for (UNiagaraScriptSourceBase* Source : SystemSources)
	{
		if (UNiagaraScriptSource* ScriptSource = Cast<UNiagaraScriptSource>(Source))
		{
			ProcessSystemGraph(ScriptSource->NodeGraph, PICOAsset);
			break; // Only process the first valid source
		}
	}

	// Process system script
	const UNiagaraScript* SystemScript = NiagaraSystem->GetSystemUpdateScript();
	ProcessSystemScript(SystemScript, PICOAsset);

	// Process all emitters
	TArray<FNiagaraEmitterHandle> EmitterHandles = NiagaraSystem->GetEmitterHandles();
	for (const FNiagaraEmitterHandle& EmitterHandle : EmitterHandles)
	{
		if (EmitterHandle.IsValid())
		{
			ProcessEmitter(EmitterHandle, PICOAsset);
		}
	}

	// Save the asset
	if (SaveAsset(PICOAsset, NiagaraSystem))
	{
		UE_LOG(LogPICONiagaraConverter, Log, TEXT("Successfully converted Niagara system: %s"), *NiagaraSystem->GetName());
		return true;
	}

	UE_LOG(LogPICONiagaraConverter, Warning, TEXT("Conversion completed but save failed for: %s"), *NiagaraSystem->GetName());
	return false;
}

// ========== System-Level Processing ==========

void FPICONiagaraConverter::ProcessSystemGraph(const UNiagaraGraph* NodeGraph, UPICOSpatialNiagaraAsset* OutAsset)
{
	if (!NodeGraph || !OutAsset)
	{
		return;
	}

	// Delegate to existing namespace function
	NiagaraToPICO::NiagaraGraphToNiagaraAsset(NodeGraph, OutAsset);
}

void FPICONiagaraConverter::ProcessSystemScript(const UNiagaraScript* SystemScript, UPICOSpatialNiagaraAsset* OutAsset)
{
	if (!SystemScript || !OutAsset)
	{
		return;
	}

	for (const FNiagaraVariableWithOffset& Var : SystemScript->RapidIterationParameters.ReadParameterVariables())
	{
		const int32 Offset = Var.Offset;
		const int32 SizeInBytes = Var.GetSizeInBytes();
		
		FString ItemName = NiagaraToPICO::GetItemName(Var.GetName().ToString());
		
		UE_LOG(LogPICONiagaraConverter, VeryVerbose, TEXT("System Variable: %s = %s"), 
			*Var.GetName().ToString(), *ItemName);

		if (Var.GetType() == FNiagaraTypeDefinition::GetFloatDef())
		{
			float Val = *(float*)SystemScript->RapidIterationParameters.GetParameterData(Offset, SizeInBytes);
			
			if (ItemName == NiagaraToPICO::LoopDurationTag)
			{
				OutAsset->LoopDuration = Val;
				UE_LOG(LogPICONiagaraConverter, Verbose, TEXT("Set LoopDuration: %f"), Val);
			}
			else if (ItemName == NiagaraToPICO::LoopDelayTag)
			{
				OutAsset->LoopDelay = Val;
				UE_LOG(LogPICONiagaraConverter, Verbose, TEXT("Set LoopDelay: %f"), Val);
			}
		}
		else if (Var.GetType() == FNiagaraTypeDefinition::GetIntDef())
		{
			int32 Val = *(int32*)SystemScript->RapidIterationParameters.GetParameterData(Offset, SizeInBytes);
			
			if (ItemName == NiagaraToPICO::LoopCountTag)
			{
				OutAsset->LoopCount = Val;
				UE_LOG(LogPICONiagaraConverter, Verbose, TEXT("Set LoopCount: %d"), Val);
			}
		}
	}
}

// ========== Emitter Processing ==========

void FPICONiagaraConverter::ProcessEmitter(const FNiagaraEmitterHandle& EmitterHandle, UPICOSpatialNiagaraAsset* OutAsset)
{
	if (!OutAsset)
	{
		return;
	}

	FNiagaraEmitterPICO EmitterPICO;
	
	// Set basic emitter properties
	EmitterPICO.EmitterName = EmitterHandle.GetName().ToString();
	EmitterPICO.EmitterID = EmitterHandle.GetId();
	
	FVersionedNiagaraEmitterData* EmitterData = EmitterHandle.GetInstance().GetEmitterData();
	if (!EmitterData)
	{
		UE_LOG(LogPICONiagaraConverter, Warning, TEXT("Emitter has no data: %s"), *EmitterPICO.EmitterName);
		return;
	}

	EmitterPICO.bLocalSpace = EmitterData->bLocalSpace;

	UE_LOG(LogPICONiagaraConverter, Verbose, TEXT("Processing emitter: %s"), *EmitterPICO.EmitterName);

	// Process event handlers (parent-child relationships)
	ProcessEmitterEventHandlers(EmitterData, EmitterHandle, EmitterPICO, OutAsset);

	// Process emitter graph
	if (EmitterData->GraphSource)
	{
		if (UNiagaraScriptSource* Source = Cast<UNiagaraScriptSource>(EmitterData->GraphSource))
		{
			if (Source->NodeGraph)
			{
				ProcessEmitterGraph(Source->NodeGraph, EmitterPICO);
			}
		}
	}

	// Process renderers
	ProcessRenderers(EmitterData, EmitterPICO);

	// Process scripts
	ProcessEmitterScripts(EmitterData, EmitterPICO);

	// Add the emitter to the asset
	OutAsset->GuidToEmittersMap.Add(EmitterHandle.GetId(), EmitterPICO);
}

void FPICONiagaraConverter::ProcessEmitterGraph(const UNiagaraGraph* NodeGraph, FNiagaraEmitterPICO& OutEmitter)
{
	if (!NodeGraph)
	{
		return;
	}

	// Delegate to existing namespace function
	NiagaraToPICO::NiagaraGraphToNiagaraEmitter(NodeGraph, OutEmitter);
}

void FPICONiagaraConverter::ProcessEmitterEventHandlers(FVersionedNiagaraEmitterData* EmitterData, 
	const FNiagaraEmitterHandle& EmitterHandle,
	FNiagaraEmitterPICO& OutEmitter, 
	UPICOSpatialNiagaraAsset* OutAsset)
{
	if (!EmitterData || !OutAsset)
	{
		return;
	}

	if (EmitterData->EventHandlerScriptProps.Num() > 0)
	{
		if (EmitterData->EventHandlerScriptProps.Num() > 1)
		{
			UE_LOG(LogPICONiagaraConverter, Warning,
				TEXT("Emitter %s has %d event handlers, but PICO Spatial currently supports only one sub-particle. Only the first valid handler will be used."),
				*OutEmitter.EmitterName, EmitterData->EventHandlerScriptProps.Num());
		}

		// Currently only one sub-particle per emitter is supported.
		const auto* SelectedScriptProp = &EmitterData->EventHandlerScriptProps[0];
		for (const auto& ScriptProp : EmitterData->EventHandlerScriptProps)
		{
			if (ScriptProp.SourceEmitterID.IsValid() && ScriptProp.SpawnNumber > 0)
			{
				SelectedScriptProp = &ScriptProp;
				break;
			}
		}

		if (SelectedScriptProp->SourceEmitterID.IsValid() && SelectedScriptProp->SpawnNumber > 0)
		{
			OutEmitter.SpawnNumsByEvent = SelectedScriptProp->SpawnNumber;
			OutEmitter.ParticleType = ENiagaraPICO_ParticleType::SubParticle;

			if (OutAsset->ParentToSubGuidMap.Contains(SelectedScriptProp->SourceEmitterID))
			{
				const FGuid ExistingSubEmitterId = OutAsset->ParentToSubGuidMap[SelectedScriptProp->SourceEmitterID];
				UE_LOG(LogPICONiagaraConverter, Warning,
					TEXT("Parent emitter %s already has sub-particle %s. Ignore additional sub-particle %s because only one sub-particle is currently supported."),
					*SelectedScriptProp->SourceEmitterID.ToString(), *ExistingSubEmitterId.ToString(), *EmitterHandle.GetId().ToString());
				return;
			}

			OutAsset->ParentToSubGuidMap.Add(SelectedScriptProp->SourceEmitterID, EmitterHandle.GetId());
			
			UE_LOG(LogPICONiagaraConverter, Verbose, TEXT("Emitter %s is a sub-particle with spawn count: %d"), 
				*OutEmitter.EmitterName, SelectedScriptProp->SpawnNumber);
		}
	}
}

// ========== Renderer Processing ==========

void FPICONiagaraConverter::ProcessRenderers(FVersionedNiagaraEmitterData* EmitterData, FNiagaraEmitterPICO& OutEmitter)
{
	if (!EmitterData)
	{
		return;
	}

	const int32 MaxRenderNums = MAX_RENDER_NUMS;
	int32 RenderCount = 0;

	for (UNiagaraRendererProperties* Renderer : EmitterData->GetRenderers())
	{
		if (!Renderer || !Renderer->GetIsEnabled())
		{
			continue;
		}

		RenderCount++;
		if (RenderCount > MaxRenderNums)
		{
			UE_LOG(LogPICONiagaraConverter, Warning, TEXT("Exceeded maximum renderer count (%d) for emitter: %s"), 
				MaxRenderNums, *OutEmitter.EmitterName);
			break;
		}

		if (UNiagaraSpriteRendererProperties* SpriteRenderer = Cast<UNiagaraSpriteRendererProperties>(Renderer))
		{
			ProcessSpriteRenderer(SpriteRenderer, OutEmitter);
		}
		else if (UNiagaraMeshRendererProperties* MeshRenderer = Cast<UNiagaraMeshRendererProperties>(Renderer))
		{
			ProcessMeshRenderer(MeshRenderer, OutEmitter);
		}
	}
}

void FPICONiagaraConverter::ProcessSpriteRenderer(UNiagaraSpriteRendererProperties* SpriteRenderer, FNiagaraEmitterPICO& OutEmitter)
{
	if (!SpriteRenderer)
	{
		return;
	}

	FNiagaraSpriteRendererPICO RendererPICO;
	OutEmitter.MainRendererType = ENiagaraPICO_RendererType::Sprite;
	RendererPICO.RenderType = ENiagaraPICO_RendererType::Sprite;
	RendererPICO.FacingMode = ToPICOFacingMode(SpriteRenderer->FacingMode);
	RendererPICO.RenderName = SpriteRenderer->GetName();
	RendererPICO.RenderMaterial = SpriteRenderer->Material;
	RendererPICO.SubImageSize = SpriteRenderer->SubImageSize;
	OutEmitter.SpriteRenderers.Add(RendererPICO);

	UE_LOG(LogPICONiagaraConverter, Verbose, TEXT("Added sprite renderer: %s"), *RendererPICO.RenderName);
}

void FPICONiagaraConverter::ProcessMeshRenderer(UNiagaraMeshRendererProperties* MeshRenderer, FNiagaraEmitterPICO& OutEmitter)
{
	if (!MeshRenderer)
	{
		return;
	}

	FNiagaraMeshRendererPICO RendererPICO;
	OutEmitter.MainRendererType = ENiagaraPICO_RendererType::Mesh;
	RendererPICO.RenderType = ENiagaraPICO_RendererType::Mesh;
	RendererPICO.FacingMode = ToPICOFacingMode(MeshRenderer->FacingMode);
	RendererPICO.RenderName = MeshRenderer->GetName();
	
	// Use first mesh if available
	if (MeshRenderer->Meshes.Num() > 0)
	{
		RendererPICO.Mesh = MeshRenderer->Meshes[0].Mesh;
	}

	// Check for material overrides
	if (MeshRenderer->bOverrideMaterials && MeshRenderer->OverrideMaterials.Num() > 0)
	{
		RendererPICO.bEnableOverrideMeshMaterial = true;
		RendererPICO.OverrideMeshMaterial = MeshRenderer->OverrideMaterials[0].ExplicitMat;
	}
	
	OutEmitter.MeshRenders.Add(RendererPICO);

	UE_LOG(LogPICONiagaraConverter, Verbose, TEXT("Added mesh renderer: %s"), *RendererPICO.RenderName);
}

// ========== Script Processing ==========

void FPICONiagaraConverter::ProcessEmitterScripts(FVersionedNiagaraEmitterData* EmitterData, FNiagaraEmitterPICO& OutEmitter)
{
	if (!EmitterData)
	{
		return;
	}

	TArray<UNiagaraScript*> AllScripts;
	EmitterData->GetScripts(AllScripts, false, false);
	
	// Delegate to existing namespace function
	NiagaraToPICO::NiagaraScriptsToNiagaraAsset(AllScripts, OutEmitter);
}

// ========== Asset Creation ==========

UPICOSpatialNiagaraAsset* FPICONiagaraConverter::CreatePICOAsset(UNiagaraSystem* NiagaraSystem)
{
	if (!NiagaraSystem)
	{
		return nullptr;
	}

	// Create the package path
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	FString PackagePathName = GetPICONiagaraContentAssetsRootPath() / NiagaraSystem->GetPathName();
	FString PackagePath = FPaths::GetPath(PackagePathName);
	
	if (!PlatformFile.DirectoryExists(*PackagePath))
	{
		if (!PlatformFile.CreateDirectoryTree(*PackagePath))
		{
			UE_LOG(LogPICONiagaraConverter, Error, TEXT("Failed to create directory: %s"), *PackagePath);
			return nullptr;
		}
	}

	// Create the package
	FString PackageName = TEXT("/Game/AssetsPICO") + NiagaraSystem->GetPackage()->GetPathName();
	UPackage* Outer = CreatePackage(*PackageName);
	if (!Outer)
	{
		UE_LOG(LogPICONiagaraConverter, Error, TEXT("Failed to create package: %s"), *PackageName);
		return nullptr;
	}

	// Create the asset
	FString AssetName = NiagaraSystem->GetName();
	UPICOSpatialNiagaraAsset* Asset = NewObject<UPICOSpatialNiagaraAsset>(
		Outer, 
		*AssetName, 
		RF_Public | RF_Standalone
	);

	if (Asset)
	{
		Asset->NiagaraSystemName = AssetName;
		UE_LOG(LogPICONiagaraConverter, Verbose, TEXT("Created PICO asset: %s"), *AssetName);
	}

	return Asset;
}

bool FPICONiagaraConverter::SaveAsset(UPICOSpatialNiagaraAsset* Asset, UNiagaraSystem* NiagaraSystem)
{
	if (!Asset || !NiagaraSystem)
	{
		return false;
	}

	// Mark the package as dirty
	Asset->MarkPackageDirty();
	
	// Prepare save arguments
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	SaveArgs.SaveFlags = SAVE_None;

	// Generate save path
	UPackage* Package = Asset->GetPackage();
	FString SavePath = FPackageName::LongPackageNameToFilename(
		Package->GetPathName(), 
		FPackageName::GetAssetPackageExtension()
	);

	// Save the package
	bool bSaved = UPackage::SavePackage(Package, Asset, *SavePath, SaveArgs);
	
	if (bSaved)
	{
		// Register with asset registry
		FAssetRegistryModule::AssetCreated(Asset);
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		AssetRegistryModule.Get().ScanPathsSynchronous({FPaths::GetPath(SavePath)});

		if (!PICONiagaraJsonExporter::ExportIfEnabled(Asset, SavePath))
		{
			UE_LOG(LogPICONiagaraConverter, Warning, TEXT("Failed to export JSON sidecar for: %s"), *SavePath);
		}
		
		UE_LOG(LogPICONiagaraConverter, Log, TEXT("Successfully saved PICO Niagara asset: %s"), *SavePath);
	}
	else
	{
		UE_LOG(LogPICONiagaraConverter, Error, TEXT("Failed to save PICO Niagara asset: %s"), *SavePath);
	}

	return bSaved;
}

#undef LOCTEXT_NAMESPACE
