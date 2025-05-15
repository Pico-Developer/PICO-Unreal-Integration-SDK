// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

// IWYU pragma: private, include "PXR_EnterpriseAPI.h"
#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
class UObject;
enum class ECastInitResult : uint8;
enum class ECastReturnCode : uint8;
enum class EControllerKeyEnum : uint8;
enum class EControllerPairTimeEnum : uint8;
enum class ECustomizeSettingsTab : uint8;
enum class EDeviceControlEnum : uint8;
enum class EHomeEventEnum : uint8;
enum class EHomeFunctionEnum : uint8;
enum class EInstallOTAReturnCode : uint8;
enum class EKeyState : uint8;
enum class EPackageControlEnum : uint8;
enum class EPICOCastAuthorization : uint8;
enum class EPICOCastOptionOrStatusEnum : uint8;
enum class EPICOCastOptionValueEnum : uint8;
enum class EPICOCastUrlTypeEnum : uint8;
enum class EScreencastAudioOutput : uint8;
enum class EScreenOffDelayTimeEnum : uint8;
enum class ESleepDelayTimeEnum : uint8;
enum class ESwitchEnum : uint8;
enum class ESystemFunctionSwitchEnum : uint8;
enum class ESystemInfoEnum : uint8;
enum class EUSBConfigModeEnum : uint8;
struct FCastMediaFormat;
struct FMarkResultCallback;
struct FWifiDisplayModel;
#ifdef PICOENTERPRISE_PXR_EnterpriseAPI_generated_h
#error "PXR_EnterpriseAPI.generated.h already included, missing '#pragma once' in PXR_EnterpriseAPI.h"
#endif
#define PICOENTERPRISE_PXR_EnterpriseAPI_generated_h

#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_314_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FWifiDisplayModel_Statics; \
	PICOENTERPRISE_API static class UScriptStruct* StaticStruct();


template<> PICOENTERPRISE_API UScriptStruct* StaticStruct<struct FWifiDisplayModel>();

#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_356_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FAppManagerStruct_Statics; \
	PICOENTERPRISE_API static class UScriptStruct* StaticStruct();


template<> PICOENTERPRISE_API UScriptStruct* StaticStruct<struct FAppManagerStruct>();

#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_374_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FCastMediaFormat_Statics; \
	PICOENTERPRISE_API static class UScriptStruct* StaticStruct();


template<> PICOENTERPRISE_API UScriptStruct* StaticStruct<struct FCastMediaFormat>();

#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_383_GENERATED_BODY \
	friend struct Z_Construct_UScriptStruct_FMarkResultCallback_Statics; \
	PICOENTERPRISE_API static class UScriptStruct* StaticStruct();


template<> PICOENTERPRISE_API UScriptStruct* StaticStruct<struct FMarkResultCallback>();

#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_407_DELEGATE \
PICOENTERPRISE_API void FPICOSetDeviceActionDelegate_DelegateWrapper(const FScriptDelegate& PICOSetDeviceActionDelegate, int32 Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_408_DELEGATE \
PICOENTERPRISE_API void FPICOAppManagerDelegate_DelegateWrapper(const FScriptDelegate& PICOAppManagerDelegate, int32 Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_409_DELEGATE \
PICOENTERPRISE_API void FPICOSetAutoConnectWifiDelegate_DelegateWrapper(const FScriptDelegate& PICOSetAutoConnectWifiDelegate, bool Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_410_DELEGATE \
PICOENTERPRISE_API void FPICOClearAutoConnectWifiDelegate_DelegateWrapper(const FScriptDelegate& PICOClearAutoConnectWifiDelegate, bool Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_411_DELEGATE \
PICOENTERPRISE_API void FPICOSetHomeKeyDelegate_DelegateWrapper(const FScriptDelegate& PICOSetHomeKeyDelegate, bool Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_412_DELEGATE \
PICOENTERPRISE_API void FPICOSetHomeKeyAllDelegate_DelegateWrapper(const FScriptDelegate& PICOSetHomeKeyAllDelegate, bool Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_413_DELEGATE \
PICOENTERPRISE_API void FPICODisablePowerKeyDelegate_DelegateWrapper(const FScriptDelegate& PICODisablePowerKeyDelegate, int32 Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_414_DELEGATE \
PICOENTERPRISE_API void FPICOSetScreenOffDelayDelegate_DelegateWrapper(const FScriptDelegate& PICOSetScreenOffDelayDelegate, int32 Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_415_DELEGATE \
PICOENTERPRISE_API void FPICOWriteConfigFileToDataLocalDelegate_DelegateWrapper(const FScriptDelegate& PICOWriteConfigFileToDataLocalDelegate, bool Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_416_DELEGATE \
PICOENTERPRISE_API void FPICOResetAllKeyToDefaultDelegate_DelegateWrapper(const FScriptDelegate& PICOResetAllKeyToDefaultDelegate, bool Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_417_DELEGATE \
PICOENTERPRISE_API void FPICOSetWDJsonDelegate_DelegateWrapper(const FScriptDelegate& PICOSetWDJsonDelegate, const FString& Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_418_DELEGATE \
PICOENTERPRISE_API void FPICOSetWDModelsDelegate_DelegateWrapper(const FScriptDelegate& PICOSetWDModelsDelegate, TArray<FWifiDisplayModel> const& Models);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_419_DELEGATE \
PICOENTERPRISE_API void FPICOEnableLargeSpaceDelegate_DelegateWrapper(const FScriptDelegate& PICOEnableLargeSpaceDelegate, bool Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_420_DELEGATE \
PICOENTERPRISE_API void FPICOSwitchLargeSpaceStatusDelegate_DelegateWrapper(const FScriptDelegate& PICOSwitchLargeSpaceStatusDelegate, const FString& Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_421_DELEGATE \
PICOENTERPRISE_API void FPICOExportMapsDelegate_DelegateWrapper(const FScriptDelegate& PICOExportMapsDelegate, bool Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_422_DELEGATE \
PICOENTERPRISE_API void FPICOImportMapsDelegate_DelegateWrapper(const FScriptDelegate& PICOImportMapsDelegate, bool Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_423_DELEGATE \
PICOENTERPRISE_API void FPICOControlSetAutoConnectWIFIWithErrorCodeDelegate_DelegateWrapper(const FScriptDelegate& PICOControlSetAutoConnectWIFIWithErrorCodeDelegate, int32 Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_424_DELEGATE \
PICOENTERPRISE_API void FPICOGetSwitchSystemFunctionStatusDelegate_DelegateWrapper(const FScriptDelegate& PICOGetSwitchSystemFunctionStatusDelegate, int32 Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_425_DELEGATE \
PICOENTERPRISE_API void FPICOCastInitDelegate_DelegateWrapper(const FScriptDelegate& PICOCastInitDelegate, ECastInitResult Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_426_DELEGATE \
PICOENTERPRISE_API void FPICOSetControllerPairTimeDelegate_DelegateWrapper(const FScriptDelegate& PICOSetControllerPairTimeDelegate, int32 Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_427_DELEGATE \
PICOENTERPRISE_API void FPICOGetControllerPairTimeDelegate_DelegateWrapper(const FScriptDelegate& PICOGetControllerPairTimeDelegate, EControllerPairTimeEnum Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_428_DELEGATE \
PICOENTERPRISE_API void FPICOSetSystemCountryCodeDelegate_DelegateWrapper(const FScriptDelegate& PICOSetSystemCountryCodeDelegate, int32 Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_429_DELEGATE \
PICOENTERPRISE_API void FPICOSetIPDDelegate_DelegateWrapper(const FScriptDelegate& PICOSetIPDDelegate, int32 Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_430_DELEGATE \
PICOENTERPRISE_API void FPICOSetArUcoMarkerDelegate_DelegateWrapper(const FScriptDelegate& PICOSetArUcoMarkerDelegate, TArray<FMarkResultCallback> const& Result);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_SPARSE_DATA
#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execPXR_SetArUcoMarker); \
	DECLARE_FUNCTION(execPXR_SetPICOCastMediaFormat); \
	DECLARE_FUNCTION(execPXR_SetIPD); \
	DECLARE_FUNCTION(execPXR_GetCustomizeSettingsTabStatus); \
	DECLARE_FUNCTION(execPXR_CustomizeSettingsTabStatus); \
	DECLARE_FUNCTION(execPXR_GetScreenCastAudioOutput); \
	DECLARE_FUNCTION(execPXR_SetScreenCastAudioOutput); \
	DECLARE_FUNCTION(execPXR_GetAppLibraryHideList); \
	DECLARE_FUNCTION(execPXR_GetControllerConnectState); \
	DECLARE_FUNCTION(execPXR_GetControllerBattery); \
	DECLARE_FUNCTION(execPXR_CustomizeAppLibrary); \
	DECLARE_FUNCTION(execPXR_StartActivity); \
	DECLARE_FUNCTION(execPXR_IsInitSettingComplete); \
	DECLARE_FUNCTION(execPXR_GetSkipInitSettingPage); \
	DECLARE_FUNCTION(execPXR_SetSkipInitSettingPage); \
	DECLARE_FUNCTION(execPXR_GetSystemCountryCode); \
	DECLARE_FUNCTION(execPXR_SetSystemCountryCode); \
	DECLARE_FUNCTION(execPXR_GetConfiguredWifi); \
	DECLARE_FUNCTION(execPXR_ConfigWifi); \
	DECLARE_FUNCTION(execPXR_GetSystemLanguage); \
	DECLARE_FUNCTION(execPXR_SetSystemLanguage); \
	DECLARE_FUNCTION(execPXR_GetControllerPairTime); \
	DECLARE_FUNCTION(execPXR_SetControllerPairTime); \
	DECLARE_FUNCTION(execPXR_PICOCastGetOptionOrStatus); \
	DECLARE_FUNCTION(execPXR_PICOCastSetOption); \
	DECLARE_FUNCTION(execPXR_PICOCastStopCast); \
	DECLARE_FUNCTION(execPXR_PICOCastGetUrl); \
	DECLARE_FUNCTION(execPXR_PICOCastGetShowAuthorization); \
	DECLARE_FUNCTION(execPXR_PICOCastSetShowAuthorization); \
	DECLARE_FUNCTION(execPXR_PICOCastInit); \
	DECLARE_FUNCTION(execPXR_GetSwitchSystemFunctionStatus); \
	DECLARE_FUNCTION(execPXR_GetCurrentLauncher); \
	DECLARE_FUNCTION(execPXR_GetUsbConfigurationOption); \
	DECLARE_FUNCTION(execPXR_GetHomeKeyStatus); \
	DECLARE_FUNCTION(execPXR_GetBackKeyStatus); \
	DECLARE_FUNCTION(execPXR_GetVolumeKeyStatus); \
	DECLARE_FUNCTION(execPXR_GetEnterKeyStatus); \
	DECLARE_FUNCTION(execPXR_GetPowerKeyStatus); \
	DECLARE_FUNCTION(execPXR_GetSleepDelay); \
	DECLARE_FUNCTION(execPXR_GetScreenOffDelay); \
	DECLARE_FUNCTION(execPXR_GetPowerOffWithUSBCable); \
	DECLARE_FUNCTION(execPXR_GetControllerKeyState); \
	DECLARE_FUNCTION(execPXR_GetTimingShutdownStatus); \
	DECLARE_FUNCTION(execPXR_GetTimingStartupStatus); \
	DECLARE_FUNCTION(execPXR_GetAutoConnectWiFiConfig); \
	DECLARE_FUNCTION(execPXR_InstallOTAPackage); \
	DECLARE_FUNCTION(execPXR_IsVolumeChangeToHomeAndEnter); \
	DECLARE_FUNCTION(execPXR_SwitchVolumeToHomeAndEnter); \
	DECLARE_FUNCTION(execPXR_StartVrSettingsItem); \
	DECLARE_FUNCTION(execPXR_TimingShutdown); \
	DECLARE_FUNCTION(execPXR_TimingStartup); \
	DECLARE_FUNCTION(execPXR_AppKeepAlive); \
	DECLARE_FUNCTION(execPXR_ControlSetAutoConnectWIFIWithErrorCode); \
	DECLARE_FUNCTION(execPXR_Record); \
	DECLARE_FUNCTION(execPXR_Capture); \
	DECLARE_FUNCTION(execPXR_GetDeviceTemperatures); \
	DECLARE_FUNCTION(execPXR_GetCpuUsages); \
	DECLARE_FUNCTION(execPXR_ImportMaps); \
	DECLARE_FUNCTION(execPXR_ExportMaps); \
	DECLARE_FUNCTION(execPXR_SaveLargeSpaceMaps); \
	DECLARE_FUNCTION(execPXR_GetSwitchLargeSpaceStatus); \
	DECLARE_FUNCTION(execPXR_SwitchLargeSpaceScene); \
	DECLARE_FUNCTION(execPXR_SetWDModels); \
	DECLARE_FUNCTION(execPXR_SetWDJson); \
	DECLARE_FUNCTION(execPXR_GetConnectedWD); \
	DECLARE_FUNCTION(execPXR_UpdateWifiDisplays); \
	DECLARE_FUNCTION(execPXR_RenameWifiDisplay); \
	DECLARE_FUNCTION(execPXR_ForgetWifiDisplay); \
	DECLARE_FUNCTION(execPXR_DisConnectWifiDisplay); \
	DECLARE_FUNCTION(execPXR_ConnectWifiDisplay); \
	DECLARE_FUNCTION(execPXR_StopScan); \
	DECLARE_FUNCTION(execPXR_StartScan); \
	DECLARE_FUNCTION(execPXR_CloseMiracast); \
	DECLARE_FUNCTION(execPXR_IsMiracastOn); \
	DECLARE_FUNCTION(execPXR_OpenMiracast); \
	DECLARE_FUNCTION(execPXR_KillBackgroundAppsWithWhiteList); \
	DECLARE_FUNCTION(execPXR_KillAppsByPidOrPackageName); \
	DECLARE_FUNCTION(execPXR_FreezeScreen); \
	DECLARE_FUNCTION(execPXR_GetDeviceSN); \
	DECLARE_FUNCTION(execPXR_VolumeDown); \
	DECLARE_FUNCTION(execPXR_VolumeUp); \
	DECLARE_FUNCTION(execPXR_SetVolume); \
	DECLARE_FUNCTION(execPXR_GetMaxVolume); \
	DECLARE_FUNCTION(execPXR_GetCurrentVolume); \
	DECLARE_FUNCTION(execPXR_SetBrightness); \
	DECLARE_FUNCTION(execPXR_GetCurrentBrightness); \
	DECLARE_FUNCTION(execPXR_DisableBackKey); \
	DECLARE_FUNCTION(execPXR_EnableBackKey); \
	DECLARE_FUNCTION(execPXR_DisableVolumeKey); \
	DECLARE_FUNCTION(execPXR_EnableVolumeKey); \
	DECLARE_FUNCTION(execPXR_DisableEnterKey); \
	DECLARE_FUNCTION(execPXR_EnableEnterKey); \
	DECLARE_FUNCTION(execPXR_ResetAllKeyToDefault); \
	DECLARE_FUNCTION(execPXR_WriteConfigFileToDataLocal); \
	DECLARE_FUNCTION(execPXR_ReleaseWakeLock); \
	DECLARE_FUNCTION(execPXR_AcquireWakeLock); \
	DECLARE_FUNCTION(execPXR_ScreenOff); \
	DECLARE_FUNCTION(execPXR_ScreenOn); \
	DECLARE_FUNCTION(execPXR_SetUsbConfigurationOption); \
	DECLARE_FUNCTION(execPXR_SwitchSystemFunction); \
	DECLARE_FUNCTION(execPXR_SetScreenOffDelay); \
	DECLARE_FUNCTION(execPXR_SetSleepDelay); \
	DECLARE_FUNCTION(execPXR_DisablePowerKey); \
	DECLARE_FUNCTION(execPXR_SetHomeKeyAll); \
	DECLARE_FUNCTION(execPXR_SetHomeKey); \
	DECLARE_FUNCTION(execPXR_ClearAutoConnectWifi); \
	DECLARE_FUNCTION(execPXR_SetAutoConnectWifi); \
	DECLARE_FUNCTION(execPXR_AppManager); \
	DECLARE_FUNCTION(execPXR_SetDeviceAction); \
	DECLARE_FUNCTION(execPXR_GetDeviceInfo);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execPXR_SetArUcoMarker); \
	DECLARE_FUNCTION(execPXR_SetPICOCastMediaFormat); \
	DECLARE_FUNCTION(execPXR_SetIPD); \
	DECLARE_FUNCTION(execPXR_GetCustomizeSettingsTabStatus); \
	DECLARE_FUNCTION(execPXR_CustomizeSettingsTabStatus); \
	DECLARE_FUNCTION(execPXR_GetScreenCastAudioOutput); \
	DECLARE_FUNCTION(execPXR_SetScreenCastAudioOutput); \
	DECLARE_FUNCTION(execPXR_GetAppLibraryHideList); \
	DECLARE_FUNCTION(execPXR_GetControllerConnectState); \
	DECLARE_FUNCTION(execPXR_GetControllerBattery); \
	DECLARE_FUNCTION(execPXR_CustomizeAppLibrary); \
	DECLARE_FUNCTION(execPXR_StartActivity); \
	DECLARE_FUNCTION(execPXR_IsInitSettingComplete); \
	DECLARE_FUNCTION(execPXR_GetSkipInitSettingPage); \
	DECLARE_FUNCTION(execPXR_SetSkipInitSettingPage); \
	DECLARE_FUNCTION(execPXR_GetSystemCountryCode); \
	DECLARE_FUNCTION(execPXR_SetSystemCountryCode); \
	DECLARE_FUNCTION(execPXR_GetConfiguredWifi); \
	DECLARE_FUNCTION(execPXR_ConfigWifi); \
	DECLARE_FUNCTION(execPXR_GetSystemLanguage); \
	DECLARE_FUNCTION(execPXR_SetSystemLanguage); \
	DECLARE_FUNCTION(execPXR_GetControllerPairTime); \
	DECLARE_FUNCTION(execPXR_SetControllerPairTime); \
	DECLARE_FUNCTION(execPXR_PICOCastGetOptionOrStatus); \
	DECLARE_FUNCTION(execPXR_PICOCastSetOption); \
	DECLARE_FUNCTION(execPXR_PICOCastStopCast); \
	DECLARE_FUNCTION(execPXR_PICOCastGetUrl); \
	DECLARE_FUNCTION(execPXR_PICOCastGetShowAuthorization); \
	DECLARE_FUNCTION(execPXR_PICOCastSetShowAuthorization); \
	DECLARE_FUNCTION(execPXR_PICOCastInit); \
	DECLARE_FUNCTION(execPXR_GetSwitchSystemFunctionStatus); \
	DECLARE_FUNCTION(execPXR_GetCurrentLauncher); \
	DECLARE_FUNCTION(execPXR_GetUsbConfigurationOption); \
	DECLARE_FUNCTION(execPXR_GetHomeKeyStatus); \
	DECLARE_FUNCTION(execPXR_GetBackKeyStatus); \
	DECLARE_FUNCTION(execPXR_GetVolumeKeyStatus); \
	DECLARE_FUNCTION(execPXR_GetEnterKeyStatus); \
	DECLARE_FUNCTION(execPXR_GetPowerKeyStatus); \
	DECLARE_FUNCTION(execPXR_GetSleepDelay); \
	DECLARE_FUNCTION(execPXR_GetScreenOffDelay); \
	DECLARE_FUNCTION(execPXR_GetPowerOffWithUSBCable); \
	DECLARE_FUNCTION(execPXR_GetControllerKeyState); \
	DECLARE_FUNCTION(execPXR_GetTimingShutdownStatus); \
	DECLARE_FUNCTION(execPXR_GetTimingStartupStatus); \
	DECLARE_FUNCTION(execPXR_GetAutoConnectWiFiConfig); \
	DECLARE_FUNCTION(execPXR_InstallOTAPackage); \
	DECLARE_FUNCTION(execPXR_IsVolumeChangeToHomeAndEnter); \
	DECLARE_FUNCTION(execPXR_SwitchVolumeToHomeAndEnter); \
	DECLARE_FUNCTION(execPXR_StartVrSettingsItem); \
	DECLARE_FUNCTION(execPXR_TimingShutdown); \
	DECLARE_FUNCTION(execPXR_TimingStartup); \
	DECLARE_FUNCTION(execPXR_AppKeepAlive); \
	DECLARE_FUNCTION(execPXR_ControlSetAutoConnectWIFIWithErrorCode); \
	DECLARE_FUNCTION(execPXR_Record); \
	DECLARE_FUNCTION(execPXR_Capture); \
	DECLARE_FUNCTION(execPXR_GetDeviceTemperatures); \
	DECLARE_FUNCTION(execPXR_GetCpuUsages); \
	DECLARE_FUNCTION(execPXR_ImportMaps); \
	DECLARE_FUNCTION(execPXR_ExportMaps); \
	DECLARE_FUNCTION(execPXR_SaveLargeSpaceMaps); \
	DECLARE_FUNCTION(execPXR_GetSwitchLargeSpaceStatus); \
	DECLARE_FUNCTION(execPXR_SwitchLargeSpaceScene); \
	DECLARE_FUNCTION(execPXR_SetWDModels); \
	DECLARE_FUNCTION(execPXR_SetWDJson); \
	DECLARE_FUNCTION(execPXR_GetConnectedWD); \
	DECLARE_FUNCTION(execPXR_UpdateWifiDisplays); \
	DECLARE_FUNCTION(execPXR_RenameWifiDisplay); \
	DECLARE_FUNCTION(execPXR_ForgetWifiDisplay); \
	DECLARE_FUNCTION(execPXR_DisConnectWifiDisplay); \
	DECLARE_FUNCTION(execPXR_ConnectWifiDisplay); \
	DECLARE_FUNCTION(execPXR_StopScan); \
	DECLARE_FUNCTION(execPXR_StartScan); \
	DECLARE_FUNCTION(execPXR_CloseMiracast); \
	DECLARE_FUNCTION(execPXR_IsMiracastOn); \
	DECLARE_FUNCTION(execPXR_OpenMiracast); \
	DECLARE_FUNCTION(execPXR_KillBackgroundAppsWithWhiteList); \
	DECLARE_FUNCTION(execPXR_KillAppsByPidOrPackageName); \
	DECLARE_FUNCTION(execPXR_FreezeScreen); \
	DECLARE_FUNCTION(execPXR_GetDeviceSN); \
	DECLARE_FUNCTION(execPXR_VolumeDown); \
	DECLARE_FUNCTION(execPXR_VolumeUp); \
	DECLARE_FUNCTION(execPXR_SetVolume); \
	DECLARE_FUNCTION(execPXR_GetMaxVolume); \
	DECLARE_FUNCTION(execPXR_GetCurrentVolume); \
	DECLARE_FUNCTION(execPXR_SetBrightness); \
	DECLARE_FUNCTION(execPXR_GetCurrentBrightness); \
	DECLARE_FUNCTION(execPXR_DisableBackKey); \
	DECLARE_FUNCTION(execPXR_EnableBackKey); \
	DECLARE_FUNCTION(execPXR_DisableVolumeKey); \
	DECLARE_FUNCTION(execPXR_EnableVolumeKey); \
	DECLARE_FUNCTION(execPXR_DisableEnterKey); \
	DECLARE_FUNCTION(execPXR_EnableEnterKey); \
	DECLARE_FUNCTION(execPXR_ResetAllKeyToDefault); \
	DECLARE_FUNCTION(execPXR_WriteConfigFileToDataLocal); \
	DECLARE_FUNCTION(execPXR_ReleaseWakeLock); \
	DECLARE_FUNCTION(execPXR_AcquireWakeLock); \
	DECLARE_FUNCTION(execPXR_ScreenOff); \
	DECLARE_FUNCTION(execPXR_ScreenOn); \
	DECLARE_FUNCTION(execPXR_SetUsbConfigurationOption); \
	DECLARE_FUNCTION(execPXR_SwitchSystemFunction); \
	DECLARE_FUNCTION(execPXR_SetScreenOffDelay); \
	DECLARE_FUNCTION(execPXR_SetSleepDelay); \
	DECLARE_FUNCTION(execPXR_DisablePowerKey); \
	DECLARE_FUNCTION(execPXR_SetHomeKeyAll); \
	DECLARE_FUNCTION(execPXR_SetHomeKey); \
	DECLARE_FUNCTION(execPXR_ClearAutoConnectWifi); \
	DECLARE_FUNCTION(execPXR_SetAutoConnectWifi); \
	DECLARE_FUNCTION(execPXR_AppManager); \
	DECLARE_FUNCTION(execPXR_SetDeviceAction); \
	DECLARE_FUNCTION(execPXR_GetDeviceInfo);


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_ACCESSORS
#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUPICOXRSystemAPI(); \
	friend struct Z_Construct_UClass_UPICOXRSystemAPI_Statics; \
public: \
	DECLARE_CLASS(UPICOXRSystemAPI, UActorComponent, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/PICOEnterprise"), NO_API) \
	DECLARE_SERIALIZER(UPICOXRSystemAPI)


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_INCLASS \
private: \
	static void StaticRegisterNativesUPICOXRSystemAPI(); \
	friend struct Z_Construct_UClass_UPICOXRSystemAPI_Statics; \
public: \
	DECLARE_CLASS(UPICOXRSystemAPI, UActorComponent, COMPILED_IN_FLAGS(0 | CLASS_Config), CASTCLASS_None, TEXT("/Script/PICOEnterprise"), NO_API) \
	DECLARE_SERIALIZER(UPICOXRSystemAPI)


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UPICOXRSystemAPI(const FObjectInitializer& ObjectInitializer); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UPICOXRSystemAPI) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UPICOXRSystemAPI); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UPICOXRSystemAPI); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UPICOXRSystemAPI(UPICOXRSystemAPI&&); \
	NO_API UPICOXRSystemAPI(const UPICOXRSystemAPI&); \
public: \
	NO_API virtual ~UPICOXRSystemAPI();


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_ENHANCED_CONSTRUCTORS \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UPICOXRSystemAPI(UPICOXRSystemAPI&&); \
	NO_API UPICOXRSystemAPI(const UPICOXRSystemAPI&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UPICOXRSystemAPI); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UPICOXRSystemAPI); \
	DEFINE_DEFAULT_CONSTRUCTOR_CALL(UPICOXRSystemAPI) \
	NO_API virtual ~UPICOXRSystemAPI();


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_432_PROLOG
#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_SPARSE_DATA \
	FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_RPC_WRAPPERS \
	FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_ACCESSORS \
	FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_INCLASS \
	FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_SPARSE_DATA \
	FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_ACCESSORS \
	FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_INCLASS_NO_PURE_DECLS \
	FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h_435_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> PICOENTERPRISE_API UClass* StaticClass<class UPICOXRSystemAPI>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_CI_MC_OpenXR_Unreal2_XRSDK_Unreal_PicoXR_Unreal_52_Plugins_PICOEnterprise_Source_PICOEnterprise_Public_PXR_EnterpriseAPI_h


#define FOREACH_ENUM_ESYSTEMINFOENUM(op) \
	op(ESystemInfoEnum::ELECTRIC_QUANTITY) \
	op(ESystemInfoEnum::PUI_VERSION) \
	op(ESystemInfoEnum::EQUIPMENT_MODEL) \
	op(ESystemInfoEnum::EQUIPMENT_SN) \
	op(ESystemInfoEnum::CUSTOMER_SN) \
	op(ESystemInfoEnum::INTERNAL_STORAGE_SPACE_OF_THE_DEVICE) \
	op(ESystemInfoEnum::DEVICE_BLUETOOTH_STATUS) \
	op(ESystemInfoEnum::BLUETOOTH_NAME_CONNECTED) \
	op(ESystemInfoEnum::BLUETOOTH_MAC_ADDRESS) \
	op(ESystemInfoEnum::DEVICE_WIFI_STATUS) \
	op(ESystemInfoEnum::WIFI_NAME_CONNECTED) \
	op(ESystemInfoEnum::WLAN_MAC_ADDRESS) \
	op(ESystemInfoEnum::DEVICE_IP) \
	op(ESystemInfoEnum::CHARGING_STATUS) 

enum class ESystemInfoEnum : uint8;
template<> struct TIsUEnumClass<ESystemInfoEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<ESystemInfoEnum>();

#define FOREACH_ENUM_EDEVICECONTROLENUM(op) \
	op(EDeviceControlEnum::DEVICE_CONTROL_REBOOT) \
	op(EDeviceControlEnum::DEVICE_CONTROL_SHUTDOWN) 

enum class EDeviceControlEnum : uint8;
template<> struct TIsUEnumClass<EDeviceControlEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EDeviceControlEnum>();

#define FOREACH_ENUM_EPACKAGECONTROLENUM(op) \
	op(EPackageControlEnum::PACKAGE_SILENCE_INSTALL) \
	op(EPackageControlEnum::PACKAGE_SILENCE_UNINSTALL) 

enum class EPackageControlEnum : uint8;
template<> struct TIsUEnumClass<EPackageControlEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EPackageControlEnum>();

#define FOREACH_ENUM_ESWITCHENUM(op) \
	op(ESwitchEnum::S_ON) \
	op(ESwitchEnum::S_OFF) 

enum class ESwitchEnum : uint8;
template<> struct TIsUEnumClass<ESwitchEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<ESwitchEnum>();

#define FOREACH_ENUM_EHOMEEVENTENUM(op) \
	op(EHomeEventEnum::HOME_SINGLE_CLICK) \
	op(EHomeEventEnum::HOME_DOUBLE_CLICK) \
	op(EHomeEventEnum::HOME_LONG_PRESS) \
	op(EHomeEventEnum::HOME_SINGLE_CLICK_RIGHT_CTL) \
	op(EHomeEventEnum::HOME_DOUBLE_CLICK_RIGHT_CTL) \
	op(EHomeEventEnum::HOME_LONG_PRESS_RIGHT_CTL) \
	op(EHomeEventEnum::HOME_SINGLE_CLICK_LEFT_CTL) \
	op(EHomeEventEnum::HOME_DOUBLE_CLICK_LEFT_CTL) \
	op(EHomeEventEnum::HOME_LONG_PRESS_LEFT_CTL) \
	op(EHomeEventEnum::HOME_SINGLE_CLICK_HMD) \
	op(EHomeEventEnum::HOME_DOUBLE_CLICK_HMD) \
	op(EHomeEventEnum::HOME_LONG_PRESS_HMD) 

enum class EHomeEventEnum : uint8;
template<> struct TIsUEnumClass<EHomeEventEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EHomeEventEnum>();

#define FOREACH_ENUM_EHOMEFUNCTIONENUM(op) \
	op(EHomeFunctionEnum::VALUE_HOME_GO_TO_SETTING) \
	op(EHomeFunctionEnum::VALUE_HOME_BACK) \
	op(EHomeFunctionEnum::VALUE_HOME_RECENTER) \
	op(EHomeFunctionEnum::VALUE_HOME_OPEN_APP) \
	op(EHomeFunctionEnum::VALUE_HOME_DISABLE) \
	op(EHomeFunctionEnum::VALUE_HOME_GO_TO_HOME) \
	op(EHomeFunctionEnum::VALUE_HOME_SEND_BROADCAST) \
	op(EHomeFunctionEnum::VALUE_HOME_CLEAN_MEMORY) \
	op(EHomeFunctionEnum::VALUE_HOME_QUICK_SETTING) \
	op(EHomeFunctionEnum::VALUE_HOME_SCREEN_CAP) \
	op(EHomeFunctionEnum::VALUE_HOME_SCREEN_RECORD) 

enum class EHomeFunctionEnum : uint8;
template<> struct TIsUEnumClass<EHomeFunctionEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EHomeFunctionEnum>();

#define FOREACH_ENUM_ESCREENOFFDELAYTIMEENUM(op) \
	op(EScreenOffDelayTimeEnum::THREE) \
	op(EScreenOffDelayTimeEnum::TEN) \
	op(EScreenOffDelayTimeEnum::THIRTY) \
	op(EScreenOffDelayTimeEnum::SIXTY) \
	op(EScreenOffDelayTimeEnum::THREE_HUNDRED) \
	op(EScreenOffDelayTimeEnum::SIX_HUNDRED) \
	op(EScreenOffDelayTimeEnum::NEVER) 

enum class EScreenOffDelayTimeEnum : uint8;
template<> struct TIsUEnumClass<EScreenOffDelayTimeEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EScreenOffDelayTimeEnum>();

#define FOREACH_ENUM_ESLEEPDELAYTIMEENUM(op) \
	op(ESleepDelayTimeEnum::FIFTEEN) \
	op(ESleepDelayTimeEnum::THIRTY) \
	op(ESleepDelayTimeEnum::SIXTY) \
	op(ESleepDelayTimeEnum::THREE_HUNDRED) \
	op(ESleepDelayTimeEnum::SIX_HUNDRED) \
	op(ESleepDelayTimeEnum::ONE_THOUSAND_AND_EIGHT_HUNDRED) \
	op(ESleepDelayTimeEnum::NEVER) 

enum class ESleepDelayTimeEnum : uint8;
template<> struct TIsUEnumClass<ESleepDelayTimeEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<ESleepDelayTimeEnum>();

#define FOREACH_ENUM_ESYSTEMFUNCTIONSWITCHENUM(op) \
	op(ESystemFunctionSwitchEnum::SFS_USB) \
	op(ESystemFunctionSwitchEnum::SFS_AUTOSLEEP) \
	op(ESystemFunctionSwitchEnum::SFS_SCREENON_CHARGING) \
	op(ESystemFunctionSwitchEnum::SFS_OTG_CHARGING) \
	op(ESystemFunctionSwitchEnum::SFS_RETURN_MENU_IN_2DMODE) \
	op(ESystemFunctionSwitchEnum::SFS_COMBINATION_KEY) \
	op(ESystemFunctionSwitchEnum::SFS_CALIBRATION_WITH_POWER_ON) \
	op(ESystemFunctionSwitchEnum::SFS_SYSTEM_UPDATE) \
	op(ESystemFunctionSwitchEnum::SFS_CAST_SERVICE) \
	op(ESystemFunctionSwitchEnum::SFS_EYE_PROTECTION) \
	op(ESystemFunctionSwitchEnum::SFS_SECURITY_ZONE_PERMANENTLY) \
	op(ESystemFunctionSwitchEnum::SFS_GLOBAL_CALIBRATION) \
	op(ESystemFunctionSwitchEnum::SFS_Auto_Calibration) \
	op(ESystemFunctionSwitchEnum::SFS_USB_BOOT) \
	op(ESystemFunctionSwitchEnum::SFS_VOLUME_UI) \
	op(ESystemFunctionSwitchEnum::SFS_CONTROLLER_UI) \
	op(ESystemFunctionSwitchEnum::SFS_NAVGATION_SWITCH) \
	op(ESystemFunctionSwitchEnum::SFS_SHORTCUT_SHOW_RECORD_UI) \
	op(ESystemFunctionSwitchEnum::SFS_SHORTCUT_SHOW_FIT_UI) \
	op(ESystemFunctionSwitchEnum::SFS_SHORTCUT_SHOW_CAST_UI) \
	op(ESystemFunctionSwitchEnum::SFS_SHORTCUT_SHOW_CAPTURE_UI) \
	op(ESystemFunctionSwitchEnum::SFS_STOP_MEM_INFO_SERVICE) \
	op(ESystemFunctionSwitchEnum::SFS_START_APP_BOOT_COMPLETED) \
	op(ESystemFunctionSwitchEnum::SFS_USB_FORCE_HOST) \
	op(ESystemFunctionSwitchEnum::SFS_SET_DEFAULT_SAFETY_ZONE) \
	op(ESystemFunctionSwitchEnum::SFS_ALLOW_RESET_BOUNDARY) \
	op(ESystemFunctionSwitchEnum::SFS_BOUNDARY_CONFIRMATION_SCREEN) \
	op(ESystemFunctionSwitchEnum::SFS_LONG_PRESS_HOME_TO_RECENTER) \
	op(ESystemFunctionSwitchEnum::SFS_POWER_CTRL_WIFI_ENABLE) \
	op(ESystemFunctionSwitchEnum::SFS_WIFI_DISABLE) \
	op(ESystemFunctionSwitchEnum::SFS_SIX_DOF_SWITCH) \
	op(ESystemFunctionSwitchEnum::SFS_INVERSE_DISPERSION) \
	op(ESystemFunctionSwitchEnum::SFS_LOGCAT) \
	op(ESystemFunctionSwitchEnum::SFS_PSENSOR) \
	op(ESystemFunctionSwitchEnum::SFS_SYSTEM_UPDATE_OTA) \
	op(ESystemFunctionSwitchEnum::SFS_SYSTEM_UPDATE_APP) \
	op(ESystemFunctionSwitchEnum::SFS_SHORTCUT_SHOW_WLAN_UI) \
	op(ESystemFunctionSwitchEnum::SFS_SHORTCUT_SHOW_BOUNDARY_UI) \
	op(ESystemFunctionSwitchEnum::SFS_SHORTCUT_SHOW_BLUETOOTH_UI) \
	op(ESystemFunctionSwitchEnum::SFS_SHORTCUT_SHOW_CLEAN_TASK_UI) \
	op(ESystemFunctionSwitchEnum::SFS_SHORTCUT_SHOW_IPD_ADJUSTMENT_UI) \
	op(ESystemFunctionSwitchEnum::SFS_SHORTCUT_SHOW_POWER_UI) \
	op(ESystemFunctionSwitchEnum::SFS_SHORTCUT_SHOW_EDIT_UI) \
	op(ESystemFunctionSwitchEnum::SFS_BASIC_SETTING_APP_LIBRARY_UI) \
	op(ESystemFunctionSwitchEnum::SFS_BASIC_SETTING_SHORTCUT_UI) \
	op(ESystemFunctionSwitchEnum::SFS_LED_FLASHING_WHEN_SCREEN_OFF) \
	op(ESystemFunctionSwitchEnum::SFS_BASIC_SETTING_CUSTOMIZE_SETTING_UI) \
	op(ESystemFunctionSwitchEnum::SFS_BASIC_SETTING_SHOW_APP_QUIT_CONFIRM_DIALOG) \
	op(ESystemFunctionSwitchEnum::SFS_BASIC_SETTING_KILL_BACKGROUND_VR_APP) \
	op(ESystemFunctionSwitchEnum::SFS_BASIC_SETTING_SHOW_CAST_NOTIFICATION) \
	op(ESystemFunctionSwitchEnum::SFS_AUTOMATIC_IPD) \
	op(ESystemFunctionSwitchEnum::SFS_QUICK_SEETHROUGH_MODE) \
	op(ESystemFunctionSwitchEnum::SFS_HIGN_REFERSH_MODE) 

enum class ESystemFunctionSwitchEnum : uint8;
template<> struct TIsUEnumClass<ESystemFunctionSwitchEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<ESystemFunctionSwitchEnum>();

#define FOREACH_ENUM_EUSBCONFIGMODEENUM(op) \
	op(EUSBConfigModeEnum::MTP) \
	op(EUSBConfigModeEnum::CHARGE) 

enum class EUSBConfigModeEnum : uint8;
template<> struct TIsUEnumClass<EUSBConfigModeEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EUSBConfigModeEnum>();

#define FOREACH_ENUM_EINSTALLOTARETURNCODE(op) \
	op(EInstallOTAReturnCode::SUCCESS) \
	op(EInstallOTAReturnCode::FAIL) \
	op(EInstallOTAReturnCode::LOWVERSIONOFOTA) 

enum class EInstallOTAReturnCode : uint8;
template<> struct TIsUEnumClass<EInstallOTAReturnCode> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EInstallOTAReturnCode>();

#define FOREACH_ENUM_ECASTRETURNCODE(op) \
	op(ECastReturnCode::FAIL) \
	op(ECastReturnCode::SUCCESS) 

enum class ECastReturnCode : uint8;
template<> struct TIsUEnumClass<ECastReturnCode> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<ECastReturnCode>();

#define FOREACH_ENUM_ECASTINITRESULT(op) \
	op(ECastInitResult::DISCONNECT) \
	op(ECastInitResult::CONNECT) \
	op(ECastInitResult::NO_MIC_PERMISSION) 

enum class ECastInitResult : uint8;
template<> struct TIsUEnumClass<ECastInitResult> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<ECastInitResult>();

#define FOREACH_ENUM_EPICOCASTAUTHORIZATION(op) \
	op(EPICOCastAuthorization::ASK_EVERY_TIME) \
	op(EPICOCastAuthorization::ALWAYS_ALLOW) \
	op(EPICOCastAuthorization::NOT_ACCEPTED) 

enum class EPICOCastAuthorization : uint8;
template<> struct TIsUEnumClass<EPICOCastAuthorization> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EPICOCastAuthorization>();

#define FOREACH_ENUM_EPICOCASTOPTIONORSTATUSENUM(op) \
	op(EPICOCastOptionOrStatusEnum::OPTION_RESOLUTION_LEVEL) \
	op(EPICOCastOptionOrStatusEnum::OPTION_BITRATE_LEVEL) \
	op(EPICOCastOptionOrStatusEnum::OPTION_AUDIO_ENABLE) \
	op(EPICOCastOptionOrStatusEnum::PICO_CAST_STATUS) 

enum class EPICOCastOptionOrStatusEnum : uint8;
template<> struct TIsUEnumClass<EPICOCastOptionOrStatusEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EPICOCastOptionOrStatusEnum>();

#define FOREACH_ENUM_EPICOCASTOPTIONVALUEENUM(op) \
	op(EPICOCastOptionValueEnum::OPTION_VALUE_RESOLUTION_HIGH) \
	op(EPICOCastOptionValueEnum::OPTION_VALUE_RESOLUTION_MIDDLE) \
	op(EPICOCastOptionValueEnum::OPTION_VALUE_RESOLUTION_AUTO) \
	op(EPICOCastOptionValueEnum::OPTION_VALUE_RESOLUTION_HIGH_2K) \
	op(EPICOCastOptionValueEnum::OPTION_VALUE_RESOLUTION_HIGH_4K) \
	op(EPICOCastOptionValueEnum::OPTION_VALUE_BITRATE_HIGH) \
	op(EPICOCastOptionValueEnum::OPTION_VALUE_BITRATE_MIDDLE) \
	op(EPICOCastOptionValueEnum::OPTION_VALUE_BITRATE_LOW) \
	op(EPICOCastOptionValueEnum::OPTION_VALUE_AUDIO_ON) \
	op(EPICOCastOptionValueEnum::OPTION_VALUE_AUDIO_OFF) \
	op(EPICOCastOptionValueEnum::STATUS_VALUE_STATE_STARTED) \
	op(EPICOCastOptionValueEnum::STATUS_VALUE_STATE_STOPPED) \
	op(EPICOCastOptionValueEnum::STATUS_VALUE_ERROR) 

enum class EPICOCastOptionValueEnum : uint8;
template<> struct TIsUEnumClass<EPICOCastOptionValueEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EPICOCastOptionValueEnum>();

#define FOREACH_ENUM_EPICOCASTURLTYPEENUM(op) \
	op(EPICOCastUrlTypeEnum::NORMAL_URL) \
	op(EPICOCastUrlTypeEnum::NO_CONFIRM_URL) \
	op(EPICOCastUrlTypeEnum::RTMP_URL) 

enum class EPICOCastUrlTypeEnum : uint8;
template<> struct TIsUEnumClass<EPICOCastUrlTypeEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EPICOCastUrlTypeEnum>();

#define FOREACH_ENUM_EKEYSTATE(op) \
	op(EKeyState::DISABLE) \
	op(EKeyState::ENABLE) 

enum class EKeyState : uint8;
template<> struct TIsUEnumClass<EKeyState> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EKeyState>();

#define FOREACH_ENUM_ECONTROLLERKEYENUM(op) \
	op(EControllerKeyEnum::CONTROLLER_KEY_JOYSTICK) \
	op(EControllerKeyEnum::CONTROLLER_KEY_MENU) \
	op(EControllerKeyEnum::CONTROLLER_KEY_TRIGGER) \
	op(EControllerKeyEnum::CONTROLLER_KEY_RIGHT_A) \
	op(EControllerKeyEnum::CONTROLLER_KEY_RIGHT_B) \
	op(EControllerKeyEnum::CONTROLLER_KEY_LEFT_X) \
	op(EControllerKeyEnum::CONTROLLER_KEY_LEFT_Y) \
	op(EControllerKeyEnum::CONTROLLER_KEY_LEFT_GRIP) \
	op(EControllerKeyEnum::CONTROLLER_KEY_RIGHT_GRIP) 

enum class EControllerKeyEnum : uint8;
template<> struct TIsUEnumClass<EControllerKeyEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EControllerKeyEnum>();

#define FOREACH_ENUM_ECONTROLLERPAIRTIMEENUM(op) \
	op(EControllerPairTimeEnum::DEFAULT) \
	op(EControllerPairTimeEnum::FIFTEEN) \
	op(EControllerPairTimeEnum::SIXTY) \
	op(EControllerPairTimeEnum::ONE_HUNDRED_AND_TWENTY) \
	op(EControllerPairTimeEnum::SIX_HUNDRED) \
	op(EControllerPairTimeEnum::NEVER) 

enum class EControllerPairTimeEnum : uint8;
template<> struct TIsUEnumClass<EControllerPairTimeEnum> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EControllerPairTimeEnum>();

#define FOREACH_ENUM_ESKIPINITSETTING(op) \
	op(ESkipInitSetting::INIT_SETTING_NONE) \
	op(ESkipInitSetting::INIT_SETTING_HANDLE_CONNECTION_TEACHING) \
	op(ESkipInitSetting::INIT_SETTING_TRIGGER_KEY_TEACHING) \
	op(ESkipInitSetting::INIT_SETTING_SELECT_LANGUAGE) \
	op(ESkipInitSetting::INIT_SETTING_SELECT_COUNTRY) \
	op(ESkipInitSetting::INIT_SETTING_WIFI_SETTING) \
	op(ESkipInitSetting::INIT_SETTING_QUICK_SETTING) 

enum class ESkipInitSetting : uint8;
template<> struct TIsUEnumClass<ESkipInitSetting> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<ESkipInitSetting>();

#define FOREACH_ENUM_ESCREENCASTAUDIOOUTPUT(op) \
	op(EScreencastAudioOutput::AUDIO_SINK) \
	op(EScreencastAudioOutput::AUDIO_TARGET) \
	op(EScreencastAudioOutput::AUDIO_SINK_TARGET) 

enum class EScreencastAudioOutput : uint8;
template<> struct TIsUEnumClass<EScreencastAudioOutput> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<EScreencastAudioOutput>();

#define FOREACH_ENUM_ECUSTOMIZESETTINGSTAB(op) \
	op(ECustomizeSettingsTab::CUSTOMIZE_SETTINGS_TAB_WLAN) \
	op(ECustomizeSettingsTab::CUSTOMIZE_SETTINGS_TAB_CONTROLLER) \
	op(ECustomizeSettingsTab::CUSTOMIZE_SETTINGS_TAB_BLUETOOTH) \
	op(ECustomizeSettingsTab::CUSTOMIZE_SETTINGS_TAB_DISPLAY) \
	op(ECustomizeSettingsTab::CUSTOMIZE_SETTINGS_TAB_LAB) \
	op(ECustomizeSettingsTab::CUSTOMIZE_SETTINGS_TAB_GENERAL_LOCKSCREEN) \
	op(ECustomizeSettingsTab::CUSTOMIZE_SETTINGS_TAB_GENERAL_FACTORY_RESET) 

enum class ECustomizeSettingsTab : uint8;
template<> struct TIsUEnumClass<ECustomizeSettingsTab> { enum { Value = true }; };
template<> PICOENTERPRISE_API UEnum* StaticEnum<ECustomizeSettingsTab>();

PRAGMA_ENABLE_DEPRECATION_WARNINGS
