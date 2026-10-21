// Copyright 2023 PICO Inc. All Rights Reserved.

#include "PICOOpenXRRuntimeSettings.h"
#include "PICOHapticAssetFactory.h"
#include "PICOHapticAssetThumbnailRenderer.h"
#include "PICOHapticAsset.h"
#include "Modules/ModuleInterface.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "OpenXRHMDSettings.h"
#include "Engine/RendererSettings.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "ThumbnailRendering/ThumbnailManager.h"

#define LOCTEXT_NAMESPACE "FPICOOpenXREditorModule"

static void EnsureMacMobileMultiViewEnabled()
{
#if PLATFORM_MAC
    const FString MacEnginePath = FConfigCacheIni::NormalizeConfigIniPath(FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("Mac/MacEngine.ini")));

    if (GConfig)
    {
        GConfig->LoadFile(MacEnginePath);

        bool bMobileMultiView = false;
        GConfig->GetBool(TEXT("/Script/Engine.RendererSettings"), TEXT("vr.MobileMultiView"), bMobileMultiView, MacEnginePath);

        if (!bMobileMultiView)
        {
            GConfig->SetBool(TEXT("/Script/Engine.RendererSettings"), TEXT("vr.MobileMultiView"), true, MacEnginePath);
            GConfig->Flush(false, MacEnginePath);
        }

        GConfig->UnloadFile(MacEnginePath);
    }

    if (URendererSettings* RendererSettings = GetMutableDefault<URendererSettings>())
    {
        RendererSettings->bMobileMultiView = true;
    }
#endif
}

class FPICOOpenXREditorModule
    : public IModuleInterface
{
    virtual void StartupModule() override
    {
        EnsureMacMobileMultiViewEnabled();

        ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

        if (SettingsModule != nullptr)
        {
            SettingsModule->RegisterSettings("Project", "Plugins", "PICOOpenXR",
                LOCTEXT("RuntimeSettingsName", "PICO OpenXR"),
                LOCTEXT("RuntimeSettingsDescription", "Project settings for PICO OpenXR Extension plugin"),
                GetMutableDefault<UPICOOpenXRRuntimeSettings>()
            );
        }

        UOpenXRHMDSettings* OpenXRHMDSettings = GetMutableDefault<UOpenXRHMDSettings>();
        if (OpenXRHMDSettings)
        {
            OpenXRHMDSettings->bIsFBFoveationEnabled = true; 
            OpenXRHMDSettings->UpdateSinglePropertyInConfigFile(OpenXRHMDSettings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(UOpenXRHMDSettings, bIsFBFoveationEnabled)), OpenXRHMDSettings->GetDefaultConfigFilename());
        }

        UThumbnailManager::Get().RegisterCustomRenderer(UPICOHapticAsset::StaticClass(), UPICOHapticAssetThumbnailRenderer::StaticClass());
    }

    virtual void ShutdownModule() override
    {
        ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

        if (SettingsModule != nullptr)
        {
            SettingsModule->UnregisterSettings("Project", "Plugins", "PICOOpenXR");
        }

        if (UObjectInitialized())
        {
            UThumbnailManager::Get().UnregisterCustomRenderer(UPICOHapticAsset::StaticClass());
        }
    }
};

IMPLEMENT_MODULE(FPICOOpenXREditorModule, PICOOpenXREditor);

#undef LOCTEXT_NAMESPACE
