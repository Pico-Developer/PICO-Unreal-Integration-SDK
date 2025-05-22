// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PXR_VerificationSettings.h"
#include "PXR_VerificationUtils.h"


void UPXR_RenderSettings::PostInitProperties()
{
    UObject::PostInitProperties();
    URendererSettings* Settings = GetMutableDefault<URendererSettings>();
    Settings->OnSettingChanged().AddUObject(this, &UPXR_RenderSettings::UpdateOnRenderingSettingsChanged);

#ifdef PICO_CUSTOM_ENGINE
    TypeEngine=EUnrealEngineType::Type::PICO_Custom_Engine;
    PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,bMobileSupportSpaceWarp);
#endif
    PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,bMobilePostProcessing);
    PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,MobileShadingPath);
    PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,bMobileAllowMovableSpotlightShadows);
    // Initialize properties and sync to the configuration file.
    PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,MobileAntiAliasing);
    PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,MSAASampleCount);
    PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,bMobileMultiView);
}

void UPXR_RenderSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    UObject::PostEditChangeProperty(PropertyChangedEvent);
    Super::TryUpdateDefaultConfigFile();

    // Handling properties changes
    static const FName MobileAntiAliasingName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, MobileAntiAliasing);
    if (PropertyChangedEvent.Property->GetFName() == MobileAntiAliasingName)
    {
        PICOXR_SYNC_SETTINGS(URendererSettings,UPXR_RenderSettings,MobileAntiAliasing);
    }

    // Handling properties changes
    static const FName MobileMultiViewName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, bMobileMultiView);
    if (PropertyChangedEvent.Property->GetFName() == MobileMultiViewName)
    {
        PICOXR_SYNC_SETTINGS(URendererSettings,UPXR_RenderSettings,bMobileMultiView);
    }
    
    static const FName MSAASampleCountName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, MSAASampleCount);
    if (PropertyChangedEvent.Property->GetFName() == MSAASampleCountName)
    {
        PICOXR_SYNC_SETTINGS(URendererSettings,UPXR_RenderSettings,MSAASampleCount);
    }

    static const FName MobilePostProcessingName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, bMobilePostProcessing);
    if (PropertyChangedEvent.Property->GetFName() == MobilePostProcessingName)
    {
        PICOXR_SYNC_SETTINGS(URendererSettings,UPXR_RenderSettings,bMobilePostProcessing);
    }

    static const FName MobileShadingPathName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, MobileShadingPath);
    if (PropertyChangedEvent.Property->GetFName() == MobileShadingPathName)
    {
        PICOXR_SYNC_SETTINGS(URendererSettings,UPXR_RenderSettings,MobileShadingPath);
    }
    
    static const FName MobileAllowMovableSpotlightShadowsName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, bMobileAllowMovableSpotlightShadows);
    if (PropertyChangedEvent.Property->GetFName() == MobileAllowMovableSpotlightShadowsName)
    {
        PICOXR_SYNC_SETTINGS(URendererSettings,UPXR_RenderSettings,bMobileAllowMovableSpotlightShadows);
    }
    
#ifdef PICO_CUSTOM_ENGINE
    static const FName MobileSupportSpaceWarpName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, bMobileSupportSpaceWarp);
    if (PropertyChangedEvent.Property->GetFName() == MobileSupportSpaceWarpName)
    {
        PICOXR_SYNC_SETTINGS(URendererSettings,UPXR_RenderSettings,bMobileSupportSpaceWarp);
    }
#endif
}

void UPXR_RenderSettings::UpdateOnRenderingSettingsChanged(UObject* Obj, FPropertyChangedEvent& ChangeEvent)
{
    // Handling properties changes
    static const FName MobileAntiAliasingName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, MobileAntiAliasing);
    if (ChangeEvent.Property->GetFName() == MobileAntiAliasingName)
    {
        PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,MobileAntiAliasing);
    }

    // Handling properties changes
    static const FName MobileMultiViewName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, bMobileMultiView);
    if (ChangeEvent.Property->GetFName() == MobileMultiViewName)
    {
        PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,bMobileMultiView);
    }

    static const FName MSAASampleCountName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, MSAASampleCount);
    if (ChangeEvent.Property->GetFName() == MSAASampleCountName)
    {
        PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,MSAASampleCount);
    }

    static const FName MobilePostProcessingName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, bMobilePostProcessing);
    if (ChangeEvent.Property->GetFName() == MobilePostProcessingName)
    {
        PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,bMobilePostProcessing);
    }

    static const FName MobileShadingPathName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, MobileShadingPath);
    if (ChangeEvent.Property->GetFName() == MobileShadingPathName)
    {
        PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,MobileShadingPath);
    }

    static const FName MobileAllowMovableSpotlightShadowsName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, bMobileAllowMovableSpotlightShadows);
    if (ChangeEvent.Property->GetFName() == MobileAllowMovableSpotlightShadowsName)
    {
        PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,bMobileAllowMovableSpotlightShadows);
    }
    
#ifdef PICO_CUSTOM_ENGINE
    static const FName MobileSupportSpaceWarpName = GET_MEMBER_NAME_CHECKED(UPXR_RenderSettings, bMobileSupportSpaceWarp);
    if (ChangeEvent.Property->GetFName() == MobileSupportSpaceWarpName)
    {
        PICOXR_SYNC_SETTINGS(UPXR_RenderSettings,URendererSettings,bMobileSupportSpaceWarp);
    }
#endif
    
}