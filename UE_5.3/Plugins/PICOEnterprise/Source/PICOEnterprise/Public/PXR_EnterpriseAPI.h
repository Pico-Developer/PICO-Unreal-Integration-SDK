// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "Engine/TextureRenderTarget2D.h"
#if PLATFORM_ANDROID
#include <jni.h>
#include "Android/AndroidJNI.h"
#include "Android/AndroidApplication.h"
#endif
#include "PXR_EnterpriseAPI.generated.h"

UENUM(BlueprintType)
enum class ESystemInfoEnum :uint8
{
	ELECTRIC_QUANTITY,
	PUI_VERSION,
	EQUIPMENT_MODEL,
	EQUIPMENT_SN,
	CUSTOMER_SN,
	INTERNAL_STORAGE_SPACE_OF_THE_DEVICE,
	DEVICE_BLUETOOTH_STATUS,
	BLUETOOTH_NAME_CONNECTED,
	BLUETOOTH_MAC_ADDRESS,
	DEVICE_WIFI_STATUS,
	WIFI_NAME_CONNECTED,
	WLAN_MAC_ADDRESS,
	DEVICE_IP,
	CHARGING_STATUS
};

UENUM(BlueprintType)
enum class EDeviceControlEnum :uint8
{
	DEVICE_CONTROL_REBOOT   UMETA(DisplayName = "Device Reboot"),
	DEVICE_CONTROL_SHUTDOWN UMETA(DisplayName = "Device ShutDown")
};

UENUM(BlueprintType)
enum class EPackageControlEnum :uint8
{
	PACKAGE_SILENCE_INSTALL    UMETA(DisplayName = "Install Package"),
	PACKAGE_SILENCE_UNINSTALL  UMETA(DisplayName = "Uninstall Package")
};

UENUM(BlueprintType)
enum class ESwitchEnum :uint8
{
	S_ON   UMETA(DisplayName = "On"),
	S_OFF  UMETA(DisplayName = "Off")
};

UENUM(BlueprintType)
enum class EHomeEventEnum :uint8
{
	HOME_SINGLE_CLICK,
	HOME_DOUBLE_CLICK,
	HOME_LONG_PRESS,
	HOME_SINGLE_CLICK_RIGHT_CTL,
	HOME_DOUBLE_CLICK_RIGHT_CTL,
	HOME_LONG_PRESS_RIGHT_CTL,
	HOME_SINGLE_CLICK_LEFT_CTL,
	HOME_DOUBLE_CLICK_LEFT_CTL,
	HOME_LONG_PRESS_LEFT_CTL,
	HOME_SINGLE_CLICK_HMD,
	HOME_DOUBLE_CLICK_HMD,
	HOME_LONG_PRESS_HMD
};

UENUM(BlueprintType)
enum class EHomeFunctionEnum :uint8
{
	VALUE_HOME_GO_TO_SETTING     UMETA(DisplayName = "Open Setting"),
	VALUE_HOME_BACK              UMETA(DisplayName = "Back"),
	VALUE_HOME_RECENTER          UMETA(DisplayName = "Recenter"),
	VALUE_HOME_OPEN_APP          UMETA(DisplayName = "Open App"),
	VALUE_HOME_DISABLE           UMETA(DisplayName = "Disable"),
	VALUE_HOME_GO_TO_HOME        UMETA(DisplayName = "Go to Home"),
	VALUE_HOME_SEND_BROADCAST    UMETA(DisplayName = "Send Broadcast"),
	VALUE_HOME_CLEAN_MEMORY      UMETA(DisplayName = "Clean Memory"),
	VALUE_HOME_QUICK_SETTING     UMETA(DisplayName = "Quick Setting"),
	VALUE_HOME_SCREEN_CAP        UMETA(DisplayName = "Screen Cap"),
	VALUE_HOME_SCREEN_RECORD     UMETA(DisplayName = "Screen Record")

};

UENUM(BlueprintType)
enum class EScreenOffDelayTimeEnum :uint8
{
	THREE          UMETA(DisplayName = "3 Seconds"),
	TEN            UMETA(DisplayName = "10 Seconds"),
	THIRTY         UMETA(DisplayName = "30 Seconds"),
	SIXTY          UMETA(DisplayName = "1 Minute"),
	THREE_HUNDRED  UMETA(DisplayName = "5 Minutes"),
	SIX_HUNDRED    UMETA(DisplayName = "10 Minutes"),
	NEVER          UMETA(DisplayName = "Never")
};

UENUM(BlueprintType)
enum class ESleepDelayTimeEnum :uint8
{
	FIFTEEN                        UMETA(DisplayName = "15 Seconds"),
	THIRTY                         UMETA(DisplayName = "30 Seconds"),
	SIXTY                          UMETA(DisplayName = "1 Minute"),
	THREE_HUNDRED                  UMETA(DisplayName = "5 Minutes"),
	SIX_HUNDRED                    UMETA(DisplayName = "10 Minutes"),
	ONE_THOUSAND_AND_EIGHT_HUNDRED UMETA(DisplayName = "30 Minutes"),
	NEVER                          UMETA(DisplayName = "Never")
};

UENUM(BlueprintType)
enum class ESystemFunctionSwitchEnum :uint8
{
	SFS_USB											UMETA(DisplayName = "USB Debug"),
	SFS_AUTOSLEEP									UMETA(DisplayName = "Auto Sleep"),
	SFS_SCREENON_CHARGING							UMETA(DisplayName = "ScreenOn Charging"),
	SFS_OTG_CHARGING								UMETA(DisplayName = "OTG Charging"),
	SFS_RETURN_MENU_IN_2DMODE						UMETA(DisplayName = "Show Back Menu in 2D mode"),
	SFS_COMBINATION_KEY								UMETA(DisplayName = "Combination Key"),
	SFS_CALIBRATION_WITH_POWER_ON					UMETA(DisplayName = "Calibration wiht power on"),
	SFS_SYSTEM_UPDATE								UMETA(DisplayName = "System Update"),
	SFS_CAST_SERVICE								UMETA(DisplayName = "Cast Service"),
	SFS_EYE_PROTECTION								UMETA(DisplayName = "Eye Protection"),
	SFS_SECURITY_ZONE_PERMANENTLY					UMETA(DisplayName = "Security Zone Permanently"),
	SFS_GLOBAL_CALIBRATION							UMETA(DisplayName = "Global Calibration"),
	SFS_Auto_Calibration							UMETA(DisplayName = "Auto Calibration"),
	SFS_USB_BOOT									UMETA(DisplayName = "USB Boot"),
	SFS_VOLUME_UI									UMETA(DisplayName = "Volume Global UI Prompt Switch"),
	SFS_CONTROLLER_UI								UMETA(DisplayName = "Controller Connection Global UI Prompt switch"),
	SFS_NAVGATION_SWITCH							UMETA(DisplayName = "Open/Close the Navigation"),
	SFS_SHORTCUT_SHOW_RECORD_UI						UMETA(DisplayName = "Show Record UI"),
	SFS_SHORTCUT_SHOW_FIT_UI						UMETA(DisplayName = "Show Fit UI"),
	SFS_SHORTCUT_SHOW_CAST_UI						UMETA(DisplayName = "Show Cast UI"),
	SFS_SHORTCUT_SHOW_CAPTURE_UI					UMETA(DisplayName = "Show Capture UI"),
	SFS_STOP_MEM_INFO_SERVICE						UMETA(DisplayName = "Stop Mem Infor Service"),
	SFS_START_APP_BOOT_COMPLETED					UMETA(DisplayName = "Start App Boot Completed"),
	SFS_USB_FORCE_HOST								UMETA(DisplayName = "Usb Force Host"),
	SFS_SET_DEFAULT_SAFETY_ZONE						UMETA(DisplayName = "Set Default Safety Zone"),
	SFS_ALLOW_RESET_BOUNDARY						UMETA(DisplayName = "Allow Reset Boundary"),
	SFS_BOUNDARY_CONFIRMATION_SCREEN				UMETA(DisplayName = "Boundary Confirmation Screen"),
	SFS_LONG_PRESS_HOME_TO_RECENTER					UMETA(DisplayName = "Long Press Home To Recenter"),
	SFS_POWER_CTRL_WIFI_ENABLE						UMETA(DisplayName = "Power Ctrl Wifi Enable"),
	SFS_WIFI_DISABLE								UMETA(DisplayName = "Wifi Disable"),
	SFS_SIX_DOF_SWITCH								UMETA(DisplayName = "Six Dof Switch"),
	SFS_INVERSE_DISPERSION							UMETA(DisplayName = "Inverse Dispersion"),
	SFS_LOGCAT										UMETA(DisplayName = "Logcat"),
	SFS_PSENSOR										UMETA(DisplayName = "PSensor"),
	SFS_SYSTEM_UPDATE_OTA							UMETA(DisplayName = "System Update OTA"),
	SFS_SYSTEM_UPDATE_APP							UMETA(DisplayName = "System Update App"),
	SFS_SHORTCUT_SHOW_WLAN_UI						UMETA(DisplayName = "Shortcut Show Wlan UI"),
	SFS_SHORTCUT_SHOW_BOUNDARY_UI					UMETA(DisplayName = "Shortcut Show Boundary UI"),
	SFS_SHORTCUT_SHOW_BLUETOOTH_UI					UMETA(DisplayName = "Shortcut Show Bluetooth UI"),
	SFS_SHORTCUT_SHOW_CLEAN_TASK_UI					UMETA(DisplayName = "Shortcut Show Clean Task UI"),
	SFS_SHORTCUT_SHOW_IPD_ADJUSTMENT_UI				UMETA(DisplayName = "Shortcut Show IPD Adjustment UI"),
	SFS_SHORTCUT_SHOW_POWER_UI						UMETA(DisplayName = "Shortcut Show Power UI"),
	SFS_SHORTCUT_SHOW_EDIT_UI						UMETA(DisplayName = "Shortcut Show Edit UI"),
	SFS_BASIC_SETTING_APP_LIBRARY_UI				UMETA(DisplayName = "Basic Setting App Library UI"),
	SFS_BASIC_SETTING_SHORTCUT_UI					UMETA(DisplayName = "Basic Setting Shortcut UI"),
	SFS_LED_FLASHING_WHEN_SCREEN_OFF				UMETA(DisplayName = "LED Flashing When Screen Off"),
	SFS_BASIC_SETTING_CUSTOMIZE_SETTING_UI			UMETA(DisplayName = "Basic Setting Customize Setting UI"),
	SFS_BASIC_SETTING_SHOW_APP_QUIT_CONFIRM_DIALOG  UMETA(DisplayName = "Basic Setting Show App Quit Confirm Dialog"),
	SFS_BASIC_SETTING_KILL_BACKGROUND_VR_APP		UMETA(DisplayName = "Basic Setting Kill Background VR App"),
	SFS_BASIC_SETTING_SHOW_CAST_NOTIFICATION		UMETA(DisplayName = "Basic Setting Show Cast Notification"),
	SFS_AUTOMATIC_IPD								UMETA(DisplayName = "Automatic IPD"),
	SFS_QUICK_SEETHROUGH_MODE						UMETA(DisplayName = "Quick Seethrough Mode"),
	SFS_HIGN_REFERSH_MODE							UMETA(DisplayName = "High Refersh Mode"),
	SFS_SEETHROUGH_APP_KEEP_RUNNING					UMETA(DisplayName = "SeeThrough App Keep Running"),
	SFS_OUTDOOR_TRACKING_ENHANCEMENT				UMETA(DisplayName = "Outdoor Tracking Enhancement"),
	SFS_AUTOIPD_AUTO_COMFIRM						UMETA(DisplayName = "AutoIPD Auto Comfirm"),
	SFS_LAUNCH_AUTOIPD_IF_GLASSES_WEARED			UMETA(DisplayName = "Launch AutoIPD if Glasses Weared"),
	SFS_GESTURE_RECOGNITION_HOME_ENABLE				UMETA(DisplayName = "Gesture Recognition Home Enable"),
	SFS_GESTURE_RECOGNITION_RESET_ENABLE			UMETA(DisplayName = "Gesture Recognition Reset Enable"),
	SFS_AUTO_COPY_FILES_FROM_USB_DEVICE				UMETA(DisplayName = "Auto Copy Files from USB Device"),
	//60~63
	SFS_WIFI_P2P_AUTO_CONNECT						UMETA(DisplayName = "Wifi P2P auto connect"),
	SFS_LOCK_SCREEN_FILE_COPY_ENABLE				UMETA(DisplayName = "Lock screen file copy enable"),
	SFS_TRACKING_ENABLE_DYNAMIC_MARKER				UMETA(DisplayName = "Tracking enable dynamic marker"),
	SFS_ENABLE_3DOF_CONTROLLER_TRACKING				UMETA(DisplayName = "Enable 3dof controller tracking"),
};

UENUM(BlueprintType)
enum class EUSBConfigModeEnum :uint8
{
	MTP,
	CHARGE
};

UENUM(BlueprintType)
enum class EInstallOTAReturnCode : uint8
{
	SUCCESS = 0,
	FAIL = 1,

	LOWVERSIONOFOTA = 21
};

UENUM(BlueprintType)
enum class ECastReturnCode : uint8
{
	FAIL = 0,
	SUCCESS = 1,
};

UENUM(BlueprintType)
enum class ECastInitResult : uint8
{
	DISCONNECT = 0,
	CONNECT = 1,
	NO_MIC_PERMISSION = 2
};

UENUM(BlueprintType)
enum class EPICOCastAuthorization : uint8
{
	ASK_EVERY_TIME = 0,
	ALWAYS_ALLOW = 1,
	NOT_ACCEPTED = 2
};

UENUM(BlueprintType)
enum class EPICOCastOptionOrStatusEnum : uint8
{
	OPTION_RESOLUTION_LEVEL = 0     UMETA(DisplayName = "Resolution"),
	OPTION_BITRATE_LEVEL = 1        UMETA(DisplayName = "Bit Rate"),
	OPTION_AUDIO_ENABLE = 2         UMETA(DisplayName = "Audio Enable"),

	PICO_CAST_STATUS = 3            UMETA(DisplayName = "Cast Status"),
};

UENUM(BlueprintType)
enum class EPICOCastOptionValueEnum : uint8
{
	OPTION_VALUE_RESOLUTION_HIGH,
	OPTION_VALUE_RESOLUTION_MIDDLE,
	OPTION_VALUE_RESOLUTION_AUTO,
	OPTION_VALUE_RESOLUTION_HIGH_2K,
	OPTION_VALUE_RESOLUTION_HIGH_4K,
	OPTION_VALUE_BITRATE_HIGH,
	OPTION_VALUE_BITRATE_MIDDLE,
	OPTION_VALUE_BITRATE_LOW,
	OPTION_VALUE_AUDIO_ON,
	OPTION_VALUE_AUDIO_OFF,
	STATUS_VALUE_STATE_STARTED,
	STATUS_VALUE_STATE_STOPPED,
	STATUS_VALUE_ERROR
};

UENUM(BlueprintType)
enum class EPICOCastUrlTypeEnum : uint8
{
	NORMAL_URL        UMETA(ToolTip = "If the authorization window is not set, the authorization window will pop up."),
	NO_CONFIRM_URL    UMETA(ToolTip = "A confirmation window will not pop up on the browser side. After entering the address, the screen will start directly."),
	RTMP_URL          UMETA(ToolTip = "Return the rtmp live stream address -- the all-in-one computer will not pop up the authorization window.")
};

UENUM(BlueprintType)
enum class EKeyState : uint8
{
	DISABLE = 0,
	ENABLE = 1
};

UENUM(BlueprintType)
enum class EControllerKeyEnum : uint8
{
	CONTROLLER_KEY_JOYSTICK,
	CONTROLLER_KEY_MENU,
	CONTROLLER_KEY_TRIGGER,
	CONTROLLER_KEY_RIGHT_A,
	CONTROLLER_KEY_RIGHT_B,
	CONTROLLER_KEY_LEFT_X,
	CONTROLLER_KEY_LEFT_Y,
	CONTROLLER_KEY_LEFT_GRIP,
	CONTROLLER_KEY_RIGHT_GRIP
};

UENUM(BlueprintType)
enum class EControllerPairTimeEnum : uint8
{
	DEFAULT                    UMETA(DisplayName = "Default"),
	FIFTEEN                    UMETA(DisplayName = "15 Seconds"),
	SIXTY                      UMETA(DisplayName = "60 Seconds"),
	ONE_HUNDRED_AND_TWENTY     UMETA(DisplayName = "2 Minutes"),
	SIX_HUNDRED                UMETA(DisplayName = "10 Minutes"),
	NEVER                      UMETA(DisplayName = "Never")
};

UENUM(BlueprintType)
enum class ESkipInitSetting : uint8
{
	INIT_SETTING_NONE = 0,
	INIT_SETTING_HANDLE_CONNECTION_TEACHING = 1,
	INIT_SETTING_TRIGGER_KEY_TEACHING = 2,
	INIT_SETTING_SELECT_LANGUAGE = 4,
	INIT_SETTING_SELECT_COUNTRY = 8,
	INIT_SETTING_WIFI_SETTING = 16,
	INIT_SETTING_QUICK_SETTING = 32
};

UENUM(BlueprintType)
enum class EScreencastAudioOutput : uint8
{
	AUDIO_SINK = 0,
	AUDIO_TARGET = 1,
	AUDIO_SINK_TARGET = 2
};

UENUM(BlueprintType)
enum class ECustomizeSettingsTab : uint8
{
	CUSTOMIZE_SETTINGS_TAB_WLAN = 0,
	CUSTOMIZE_SETTINGS_TAB_CONTROLLER = 1,
	CUSTOMIZE_SETTINGS_TAB_BLUETOOTH = 2,
	CUSTOMIZE_SETTINGS_TAB_DISPLAY = 3,
	CUSTOMIZE_SETTINGS_TAB_LAB = 4,
	CUSTOMIZE_SETTINGS_TAB_GENERAL_LOCKSCREEN = 5,
	CUSTOMIZE_SETTINGS_TAB_GENERAL_FACTORY_RESET = 6
};

UENUM(BlueprintType)
enum class EPoseErrorType : uint8
{
	BRIGHT_LIGHT_ERROR = 0,
	LOW_LIGHT_ERROR = 1,
	LOW_FEATURE_COUNT_ERROR = 2,
	CAMERA_CALIBRATION_ERROR = 3,
	RELOCATION_IN_PROGRESS = 4,
	INITILIZATION_IN_PROGRESS = 5,
	NO_CAMERA_ERROR = 6,
	NO_IMU_ERROR = 7,
	IMU_JITTER_ERROR = 8,
	UNKNOWN_ERROR = 9
};

USTRUCT(BlueprintType)
struct FWifiDisplayModel
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FString  deviceAddress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FString  deviceName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	bool isAvailable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	bool canConnect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	bool isRemembered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int statusCode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FString status;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FString description;

	FWifiDisplayModel()
		:deviceAddress("")
		, deviceName("")
		, isAvailable(false)
		, canConnect(false)
		, isRemembered(false)
		, statusCode(0)
		, status("")
		, description("")
	{
	}
};

USTRUCT()
struct FAppManagerStruct
{
	GENERATED_BODY()

	EPackageControlEnum PackageControl;
	FString AppPath;
	FORCEINLINE friend bool operator==(const FAppManagerStruct& Lhs, const FAppManagerStruct& Rhs)
	{
		return (Lhs.PackageControl == Rhs.PackageControl) && (Lhs.AppPath == Rhs.AppPath);
	}
};

FORCEINLINE uint32 GetTypeHash(const FAppManagerStruct& Key)
{
	return HashCombine(GetTypeHash(Key.PackageControl), GetTypeHash(Key.AppPath));
}

USTRUCT(BlueprintType)
struct FCastMediaFormat
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 Bitrate = 0;
};

USTRUCT(BlueprintType)
struct FMarkerResultCallback
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 ValidFlag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 MarkerType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 MarkerID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FVector MarkerTranslation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FQuat MarkerOrientation = FQuat::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	TArray<int32> Reserve;
};

USTRUCT(BlueprintType)
struct FMarkerFResultCallback
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 ValidFlag = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 MarkerType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 MarkerID = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FVector MarkerTranslation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FQuat MarkerOrientation = FQuat::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	TArray<float> Reserve;
};

USTRUCT(BlueprintType)
struct FSixDofData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int64 Timestamp = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FVector Translation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FQuat Orientation = FQuat::Identity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	uint8 Type = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	uint8 Confidence = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	TArray<EPoseErrorType> PoseErrorType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	float PlaneHeight = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	uint8 PlaneStatus = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	uint8 RelocationStatus = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	TArray<uint8> Reserved;
};

USTRUCT(BlueprintType)
struct FVSTCameraFrameItem
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	uint8 CameraId = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 Width = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 Height = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 Format = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 ExposureDuration = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int64 Timestamp = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int64 QTimerTimestamp = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int64 FrameNumber = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	int32 DataSize = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	TArray<uint8> Data;
};

USTRUCT(BlueprintType)
struct FAlgoResult
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FSixDofData Pose;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FSixDofData RelocationPose;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FVector LinearVelocity = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FVector LinearAcceleration = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FVector AngularVelocity = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FVector AngularAcceleration = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	TArray<uint8> Reserved;
};

USTRUCT(BlueprintType)
struct FVSTCameraFrameData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FVSTCameraFrameItem FrameItem;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	bool bIsRGB = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FMatrix RGBTSWMatrix = FMatrix::Identity;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	bool bIsAntiDistortion = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FAlgoResult SixDofPose;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	TArray<uint8> Reserved;
};

USTRUCT(BlueprintType)
struct FRGBCameraParams
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FVector2D InternalF = FVector2D::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FVector2D InternalC = FVector2D::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FVector ExternalTranslation = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PXR|PXRSystemAPI")
	FQuat ExternalOrientation = FQuat::Identity;
};

UENUM(BlueprintType)
enum class ESystemKey : uint8
{
	ENTER_KEY = 0,
	BACK_KEY = 1,
	VOLUME_KEY = 2,
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetDeviceActionDelegate, int32, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOAppManagerDelegate, int32, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetAutoConnectWifiDelegate, bool, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOClearAutoConnectWifiDelegate, bool, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetHomeKeyDelegate, bool, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetHomeKeyAllDelegate, bool, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICODisablePowerKeyDelegate, int32, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetScreenOffDelayDelegate, int32, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOWriteConfigFileToDataLocalDelegate, bool, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOResetAllKeyToDefaultDelegate, bool, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetWDJsonDelegate, const FString, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetWDModelsDelegate, const TArray<FWifiDisplayModel>&, Models);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOEnableLargeSpaceDelegate, bool, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSwitchLargeSpaceStatusDelegate, const FString, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOExportMapsDelegate, bool, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOImportMapsDelegate, bool, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOControlSetAutoConnectWIFIWithErrorCodeDelegate, int32, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOGetSwitchSystemFunctionStatusDelegate, int32, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOCastInitDelegate, ECastInitResult, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetControllerPairTimeDelegate, int32, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOGetControllerPairTimeDelegate, EControllerPairTimeEnum, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetSystemCountryCodeDelegate, int32, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetIPDDelegate, int32, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetArUcoMarkerDelegate, const TArray<FMarkerResultCallback>&, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOSetArUcoMarkerFDelegate, const TArray<FMarkerFResultCallback>&, Result);
DECLARE_DYNAMIC_DELEGATE_FiveParams(FPICOFileCopyDelegate, bool, bStart, bool, bProcess, float, Process, bool, bFinish, int32, ErrorCode);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOIsMapInEffectDelegate, int32, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOImportMapByPathDelegate, int32, Result);
//590
DECLARE_DYNAMIC_DELEGATE_OneParam(FPICOEnterpriseIntDelegate, int32, Result);

UCLASS(ClassGroup = (PXRComponent), meta = (BlueprintSpawnableComponent))
class PICOENTERPRISE_API UPICOXRSystemAPI : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPICOXRSystemAPI();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Only C++ callable functions
	int32 PXR_SetControllerKeyState(EControllerKeyEnum ControllerKeyEnum, ESwitchEnum SwitchEnum, int32 Ext);

	// Blueprintcallable functions
	/// <summary>Gets the requested type of device information.
	/// @note All the PICO XR System APIs are only for Enterprise headsets.
	/// </summary>
	/// <param name="Info">(In) Enum, the type of device information to retrieve. Options are as follows:
	/// <ul>
	/// <li>ELECTRIC_QUANTITY: Battery level</li>
	/// <li>PUI_VERSION: PUI version</li>
	/// <li>EQUIPMENT_MODEL: Device model</li>
	/// <li>EQUIPMENT_SN: Device SN</li>
	/// <li>CUSTOMER_SN: Customer SN</li>
	/// <li>INTERNAL_STORAGE_SPACE_OF_THE_DEVICE: Storage space of the device</li>
	/// <li>DEVICE_BLUETOOTH_STATUS: Bluetooth status</li>
	/// <li>BLUETOOTH_NAME_CONNECTED: Name of bluetooth connection</li>
	/// <li>BLUETOOTH_MAC_ADDRESS: Mac address of bluetooth</li>
	/// <li>DEVICE_WIFI_STATUS: WIFI connection status</li>
	/// <li>WIFI_NAME_CONNECTED: Name of connected WiFi</li>
	/// <li>WLAN_MAC_ADDRESS: Mac address of WLAN</li>
	/// <li>DEVICE_IP: Device IP</li>
	/// <li>CHARGING_STATUS: Charging status (2-charged; 3-uncharged)</li>
	/// </ul>
	/// </param>
	/// <returns>FString: The requested type of device information.</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetDeviceInfo(ESystemInfoEnum InfoEnum);

	static TMap<EDeviceControlEnum, FPICOSetDeviceActionDelegate> SetDeviceActionDelegates;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetDeviceAction(EDeviceControlEnum DeviceControlEnum, FPICOSetDeviceActionDelegate SetDeviceActionDelegate);

	static TMap<FAppManagerStruct, FPICOAppManagerDelegate> AppManagerDelegates;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_AppManager(EPackageControlEnum PackageControlEnum, FString Path, int32 Ext, FPICOAppManagerDelegate AppManagerDelegate);//Can not know how to design delegate

	/// <summary>Connects to a specified Wi-Fi.</summary>
	/// <param name="WifiName">(In) FString, the ID of specified Wi-Fi.</param>
	/// <param name="WifiPSD">(In) FString, the password of specified Wi-Fi.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <param name="SetAutoConnectWifiDelegate">(In) Delegate, bind the callback event to get the setup result. The callback event parameters are: Bool,
	/// <ul>
	/// <li>`true` - success;</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </param>
	/// <returns>None</returns>
	static FPICOSetAutoConnectWifiDelegate SetAutoConnectWifiDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetAutoConnectWifi(FString WifiName, FString WifiPSD, int32 Ext, FPICOSetAutoConnectWifiDelegate SetAutoConnectWifiDelegate);

	/// <summary>Disables auto connecting to the specified Wi-Fi.</summary>
	/// <param name="ClearAutoConnectWifiDelegate">(In) Delegate, bind the callback event to get the result. The callback event parameters are: Bool,
	/// <ul>
	/// <li>`true` - success;</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </param>
	/// <returns>None</returns>
	static FPICOClearAutoConnectWifiDelegate ClearAutoConnectWifiDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_ClearAutoConnectWifi(FPICOClearAutoConnectWifiDelegate ClearAutoConnectWifiDelegate);

	/// <summary>Customizes the function of the Home key. Note: This will redefine the Home key and affect the function of the Home key defined by the system. Please use this function with discretion.</summary>
	/// <param name="EventEnum">(In) EHomeEventEnum, the Home key event. The possible values are: 
	/// <ul>
	/// <li>`HOME_SINGLE_CLICK` - Single click the Home key</li>
	/// <li>`HOME_DOUBLE_CLICK` - Double click the Home key</li>
	/// <li>`HOME_LONG_PRESS` - Hold down the Home key</li>
	/// <li>`HOME_SINGLE_CLICK_RIGHT_CTL` - Single click the Home key with the right controller</li>
	/// <li>`HOME_DOUBLE_CLICK_RIGHT_CTL` - Double click the Home key with the right controller</li>
	/// <li>`HOME_LONG_PRESS_RIGHT_CTL` - Hold down the Home key with the right controller</li>
	/// <li>`HOME_SINGLE_CLICK_LEFT_CTL` - Single click the Home key with the left controller</li>
	/// <li>`HOME_DOUBLE_CLICK_LEFT_CTL` - Double click the Home key with the left controller</li>
	/// <li>`HOME_LONG_PRESS_LEFT_CTL` - Hold down the Home key with the left controller</li>
	/// <li>`HOME_SINGLE_CLICK_HMD` - Single click the Home key with the HMD</li>
	/// <li>`HOME_DOUBLE_CLICK_HMD` - Double click the Home key with the HMD</li>
	/// <li>`HOME_LONG_PRESS_HMD` - Hold down the Home key with the HMD</li>
	/// </ul>
	/// </param>
	/// <param name="FunctionEnum">(In) EHomeFunctionEnum, the function of the Home key event. The possible values are: 
	/// <ul>
	/// <li>`VALUE_HOME_GO_TO_SETTING` - Go to Settings</li>
	/// <li>`VALUE_HOME_BACK` - Return</li>
	/// <li>`VALUE_HOME_RECENTER` - Re-center</li>
	/// <li>`VALUE_HOME_OPEN_APP` - Open the target App</li>
	/// <li>`VALUE_HOME_DISABLE` - Disable the Home key</li>
	/// <li>`VALUE_HOME_GO_TO_HOME` - Go to Launcher</li>
	/// <li>`VALUE_HOME_SEND_BROADCAST` - Send a broadcast when the Home key is clicked</li>
	/// <li>`VALUE_HOME_CLEAN_MEMORY` - Clear the background system memory</li>
	/// <li>`VALUE_HOME_QUICK_SETTING` - Launch the shortcut settings</li>
	/// <li>`VALUE_HOME_SCREEN_CAP` - Launch screen capture</li>
	/// <li>`VALUE_HOME_SCREEN_RECORD` - Launch screen record</li>
	/// </ul>
	/// </param>
	/// <param name="SetHomeKeyDelegate">(In) Delegate, bind the callback event to get the Home key setup result. The callback event parameters are: Bool,
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </param>
	/// <returns>None</returns>
	static TMap<EHomeEventEnum, FPICOSetHomeKeyDelegate> SetHomeKeyDelegates;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetHomeKey(EHomeEventEnum EventEnum, EHomeFunctionEnum FunctionEnum, FPICOSetHomeKeyDelegate SetHomeKeyDelegate);

	/// <summary>Customizes the function of the Home key.
	/// @note This will redefine the Home key and affect the function of the Home key defined by the system. Please use this function with discretion.
	/// </summary>
	/// <param name="EventEnum">(In) EHomeEventEnum, Home key event: 
	/// <ul>
	/// <li>`HOME_SINGLE_CLICK` - Single click the Home key</li>
	/// <li>`HOME_DOUBLE_CLICK` - Double click the Home key</li>
	/// <li>`HOME_LONG_PRESS` - Hold down the Home key</li>
	/// <li>`HOME_SINGLE_CLICK_RIGHT_CTL` - Single click the Home key with the right controller</li>
	/// <li>`HOME_DOUBLE_CLICK_RIGHT_CTL` - Double click the Home key with the right controller</li>
	/// <li>`HOME_LONG_PRESS_RIGHT_CTL` - Hold down the Home key with the right controller</li>
	/// <li>`HOME_SINGLE_CLICK_LEFT_CTL` - Single click the Home key with the left controller</li>
	/// <li>`HOME_DOUBLE_CLICK_LEFT_CTL` - Double click the Home key with the left controller</li>
	/// <li>`HOME_LONG_PRESS_LEFT_CTL` - Hold down the Home key with the left controller</li>
	/// <li>`HOME_SINGLE_CLICK_HMD` - Single click the Home key with the HMD</li>
	/// <li>`HOME_DOUBLE_CLICK_HMD` - Double click the Home key with the HMD</li>
	/// <li>`HOME_LONG_PRESS_HMD` - Hold down the Home key with the HMD</li>
	/// </ul>
	/// </param>
	/// <param name="FunctionEnum">(In) EHomeFunctionEnum, the function of the Home key event: 
	/// <ul>
	/// <li>`VALUE_HOME_GO_TO_SETTING` - Go to Settings</li>
	/// <li>`VALUE_HOME_BACK` - Return</li>
	/// <li>`VALUE_HOME_RECENTER` - Re-center</li>
	/// <li>`VALUE_HOME_OPEN_APP` - Open the target App</li>
	/// <li>`VALUE_HOME_DISABLE` - Disable the Home key</li>
	/// <li>`VALUE_HOME_GO_TO_HOME` - Go to Launcher</li>
	/// <li>`VALUE_HOME_SEND_BROADCAST` - Send a broadcast when the Home key is clicked</li>
	/// <li>`VALUE_HOME_CLEAN_MEMORY` - Clear the background system memory</li>
	/// <li>`VALUE_HOME_QUICK_SETTING` - Launch the shortcut settings</li>
	/// <li>`VALUE_HOME_SCREEN_CAP` - Launch screen capture</li>
	/// <li>`VALUE_HOME_SCREEN_RECORD` - Launch screen record</li>
	/// </ul>
	/// </param>
	/// <param name="TimeSetup">(In) Int, only `HOME_DOUBLE_CLICK` and `HOME_LONG_PRESS` (deprecated) require this time setup. If it is `HOME_SINGLE_CLICK`, use `0`.</param>
	/// <param name="Package">(In) FString, specify the app package name when the function is set as `VALUE_HOME_OPEN_APP` (deprecated).</param>
	/// <param name="ClassName">(In) FString, specify the class name when the function is set as `VALUE_HOME_OPEN_APP` (deprecated).</param>
	/// <param name="SetHomeKeyAllDelegate">(In) Delegate, bind the callback event to get the setup result. The callback event parameters are: Bool,
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </param>
	/// <returns>None</returns>
	static TMap<EHomeEventEnum, FPICOSetHomeKeyAllDelegate> SetHomeKeyAllDelegates;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetHomeKeyAll(EHomeEventEnum EventEnum, EHomeFunctionEnum FunctionEnum, int32 TimeSetup, FString Package, FString ClassName, FPICOSetHomeKeyAllDelegate SetHomeKeyAllDelegate);

	/// <summary>Sets the Power key event.</summary>
	/// <param name="isSingleTap">(In) Bool, whether to trigger the Power key event through single click. The possible values are: 
	/// <ul>
	/// <li>`true` - Single click to trigger</li>
	/// <li>`false` - Double click to trigger</li>
	/// </ul>
	/// </param>
	/// <param name="Enable">(In) Bool, whether to enable the Power key. The possible values are: 
	/// <ul>
	/// <li>`true` - Enable the Power key</li>
	/// <li>`false` - Disable the Power key</li>
	/// </ul>
	/// </param>
	/// <param name="DisablePowerKeyDelegate">(In) Delegate, bind the callback event to get the result. The callback event parameters are: Int,
	/// <ul>
	/// <li>`0` - Success</li>
	/// <li>`1` - Failure</li>
	/// </ul>
	/// </param>
	/// <returns>None</returns>
	static TMap<bool, FPICODisablePowerKeyDelegate> DisablePowerKeyDelegates;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_DisablePowerKey(bool bIsSingleTap, bool bEnable, FPICODisablePowerKeyDelegate DisablePowerKeyDelegate);

	static FPICOSetScreenOffDelayDelegate SetScreenOffDelayDelegate;
	/// <summary>Sets the time the system sleeps when the device is not in use.</summary>
	/// <param name="TimeEnum">(In) Enum (ESleepDelayTimeEnum), system sleep timeout. The possible values are: 
	/// <ul>
	/// <li>`FIFTEEN` - 15 seconds</li>
	/// <li>`THIRTY` - 30 seconds</li>
	/// <li>`SIXTY` - 1 minute</li>
	/// <li>`THREE_HUNDRED` - 5 minutes</li>
	/// <li>`SIX_HUNDRED` - 10 minutes</li>
	/// <li>`ONE_THOUSAND_AND_EIGHT_HUNDRED` - 30 minutes</li>
	/// <li>`NEVER` - Never sleep</li>
	/// </ul>
	/// </param>
	/// <returns>(Out) bool, Result</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_SetSystemAutoSleepTime(ESleepDelayTimeEnum TimeEnum);

	/// <summary>Sets the time the screen turns off when the device is not in use.</summary>
	/// <param name="TimeEnum">(In) Enum (EScreenOffDelayTimeEnum), screen off timeout. The possible values are: 
	/// <ul>
	/// <li>`THREE` - 3 seconds</li>
	/// <li>`TEN` - 10 seconds</li>
	/// <li>`THIRTY` - 30 seconds</li>
	/// <li>`SIXTY` - 1 minute</li>
	/// <li>`THREE_HUNDRED` - 5 minutes</li>
	/// <li>`SIX_HUNDRED` - 10 minutes</li>
	/// <li>`NEVER` - Never turn off the screen</li>
	/// </ul>
	/// </param>
	/// <param name="SetScreenOffDelayDelegate">(In) Delegate, bind the callback event to get the setup result. The callback event parameters are: Int,
	/// <ul>
	/// <li>`0` - success;</li>
	/// <li>`1` - failure;</li>
	/// <li>`10` - the time set is too large</li>
	/// </ul>
	/// </param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetScreenOffDelay(EScreenOffDelayTimeEnum TimeEnum, FPICOSetScreenOffDelayDelegate SetScreenOffDelayDelegate);

	static FPICOEnterpriseIntDelegate SwitchSystemFunctionDelegate;
	/// <summary>Switches a specified system function on/off.</summary>
	/// <param name="SystemFunction">(In) Enum (ESystemFunctionSwitchEnum), the system function to switch on/off. Possible values include:
	/// <ul>
	/// <li>`SFS_USB` - "USB Debug": USB debugging switch</li>
	/// <li>`SFS_AUTOSLEEP` - "Auto Sleep": Auto sleep switch</li>
	/// <li>`SFS_SCREENON_CHARGING` - "ScreenOn Charging": Screen-on charging switch</li>
	/// <li>`SFS_OTG_CHARGING` - "OTG Charging": OTG charging switch</li>
	/// <li>`SFS_RETURN_MENU_IN_2DMODE` - "Show Back Menu in 2D mode": Show Back Menu in 2D mode switch</li>
	/// <li>`SFS_COMBINATION_KEY` - "Combination Key": Combination key switch</li>
	/// <li>`SFS_CALIBRATION_WITH_POWER_ON` - "Calibration with power on": Calibration with power on switch</li>
	/// <li>`SFS_SYSTEM_UPDATE` - "System Update": System update switch</li>
	/// <li>`SFS_CAST_SERVICE` - "Cast Service": Cast service switch. This property does not take effect when the industry solution switch is on.</li>
	/// <li>`SFS_EYE_PROTECTION` - "Eye Protection": Eye protection switch</li>
	/// <li>`SFS_SECURITY_ZONE_PERMANENTLY` - "Security Zone Permanently": Security zone permanently on/off switch</li>
	/// <li>`SFS_Auto_Calibration` - "Auto Calibration": Auto calibration switch</li>
	/// <li>`SFS_USB_BOOT` - "USB Boot": USB Boot switch</li>
	/// <li>`SFS_VOLUME_UI` - "Volume UI": global volume UI</li>
	/// <li>`SFS_CONTROLLER_UI` - "Controller UI": Global controller connected UI</li>
	/// <li>`SFS_NAVGATION_SWITCH` - "Navigation Switch": Navigation bar</li>
	/// <li>`SFS_SHORTCUT_SHOW_RECORD_UI` - "Shortcut Show Record UI": Screen recording button UI</li>
	/// <li>`SFS_SHORTCUT_SHOW_FIT_UI` - "Shortcut Show Fit UI": PICO fit UI</li>
	/// <li>`SFS_SHORTCUT_SHOW_CAST_UI` - "Shortcut Show Cast UI": Screencast button UI</li>
	/// <li>`SFS_SHORTCUT_SHOW_CAPTURE_UI` - "Shortcut Show Capture UI": Screenshot button UI</li>
	/// <li>`SFS_USB_FORCE_HOST` - "USB Force Host": Set the Neo3 device as the host device</li>
	/// <li>`SFS_SET_DEFAULT_SAFETY_ZONE` - "Set Default Safety Zone": Set a default play area for a Neo3 device</li>
	/// <li>`SFS_ALLOW_RESET_BOUNDARY` - "Allow Reset Boundary": Allow to reset customized boundary</li>
	/// <li>`SFS_BOUNDARY_CONFIRMATION_SCREEN` - "Boundary Confirmation Screen": Whether to display the boundary confirmation screen</li>
	/// <li>`SFS_LONG_PRESS_HOME_TO_RECENTER` - "Long Press Home to Recenter": Long press the Home key to recenter</li>
	/// <li>`SFS_POWER_CTRL_WIFI_ENABLE` - "Power Ctrl Wifi Enable": Neo3 device stays connected to the network when the device sleeps/turns off</li>
	/// <li>`SFS_WIFI_DISABLE` - "Wifi Disable": Disable Wi-Fi for Neo3 device</li>
	/// <li>`SFS_SIX_DOF_SWITCH` - "Six DOF Switch": 6DoF position tracking</li>
	/// <li>`SFS_INVERSE_DISPERSION` - "Inverse Dispersion": Anti-dispersion</li>
	/// <li>`SFS_LOGCAT` - "Logcat": Log switch (log directory: /data/logs)</li>
	/// <li>`SFS_PSENSOR` - "PSensor": PSensor switch</li>
	/// <li>`SFS_SYSTEM_UPDATE_OTA` - "System Update OTA": OTA system update switch</li>
	/// <li>`SFS_SYSTEM_UPDATE_APP` - "System Update App": App system update switch</li>
	/// <li>`SFS_SHORTCUT_SHOW_WLAN_UI` - "Shortcut Show Wlan UI": WLAN shortcut UI switch</li>
	/// <li>`SFS_SHORTCUT_SHOW_BOUNDARY_UI` - "Shortcut Show Boundary UI": Boundary UI shortcut switch</li>
	/// <li>`SFS_SHORTCUT_SHOW_BLUETOOTH_UI` - "Shortcut Show Bluetooth UI": Bluetooth UI shortcut switch</li>
	/// <li>`SFS_SHORTCUT_SHOW_CLEAN_TASK_UI` - "Shortcut Show Clean Task UI": Clean Task UI shortcut switch</li>
	/// <li>`SFS_SHORTCUT_SHOW_IPD_ADJUSTMENT_UI` - "Shortcut Show IPD Adjustment UI": IPD Adjustment UI shortcut switch</li>
	/// <li>`SFS_SHORTCUT_SHOW_POWER_UI` - "Shortcut Show Power UI": Power UI shortcut switch</li>
	/// <li>`SFS_SHORTCUT_SHOW_EDIT_UI` - "Shortcut Show Edit UI": Edit UI shortcut switch</li>
	/// <li>`SFS_BASIC_SETTING_APP_LIBRARY_UI` - "Basic Setting App Library UI": App Library UI in Basic Settings switch</li>
	/// <li>`SFS_BASIC_SETTING_SHORTCUT_UI` - "Basic Setting Shortcut UI": Shortcut UI in Basic Settings switch</li>
	/// <li>`SFS_LED_FLASHING_WHEN_SCREEN_OFF` - "LED Flashing When Screen Off": LED flash when screen is off switch</li>
	/// </ul>
	/// </param>
	/// <param name="SwitchEnum">(In) Enum (ESwitchEnum), to switch the system function on/off. Possible values include:
	/// <ul>
	/// <li>`S_ON` - "On": Switch on</li>
	/// <li>`S_OFF` - "Off": Switch off</li>
	/// </ul>
	/// </param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SwitchSystemFunction(ESystemFunctionSwitchEnum SystemFunction, ESwitchEnum SwitchEnum, int32 Ext, FPICOEnterpriseIntDelegate Delegate);

	/// <summary>Sets USB configuration (MTP, charging).</summary>
	/// <param name="UsbConfigModeEnum">(In) Enum (EUSBConfigModeEnum), USB config mode. Possible values include:
	/// <ul>
	/// <li>`MTP` - Media Transfer Protocol mode. The MTP mode allows the transfer of media files.</li>
	/// <li>`CHARGE` - Charging mode. The Changing mode does not allow file transfer.</li>
	/// </ul>
	/// </param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetUsbConfigurationOption(EUSBConfigModeEnum UsbConfigModeEnum, int32 Ext);

	/// <summary>Switches the screen on.</summary>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_ScreenOn();

	/// <summary>Switches the screen off.
	/// @note Power management permission check is required for calling this function for the first time.
	/// </summary>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_ScreenOff();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_AcquireWakeLock();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_ReleaseWakeLock();

	/// <summary>Writes the configuration file to /data/local/tmp path.
	/// @note This function is only applicable to enterprise devices.
	/// </summary>
	/// <param name="Path">(In) String, the path that the configuration file is saved to.</param>
	/// <param name="Content">(In) String, the content to write into the configuration file.</param>
	/// <param name="InWriteConfigDelegate">(In) Delegate, bind the callback event to get the result. Callback event params: Bool,
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// <returns>None</returns>
	static FPICOWriteConfigFileToDataLocalDelegate WriteConfigDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_WriteConfigFileToDataLocal(FString Path, FString Content, FPICOWriteConfigFileToDataLocalDelegate InWriteConfigDelegate);

	/// <summary>Restores system default key configurations.</summary>
	/// <param name="InResetAllKeyDelegate">(In) Delegate, bind the callback event to get the result.
	/// Callback event params:
	/// <ul>
	/// <li>`Bool`:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </li>
	/// </ul>
	/// </param>
	/// <returns>None</returns>
	static FPICOResetAllKeyToDefaultDelegate ResetAllKeyDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_ResetAllKeyToDefault(FPICOResetAllKeyToDefaultDelegate InResetAllKeyDelegate);

	/// <summary>Enables the Enter key on headset.</summary>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_EnableEnterKey();

	/// <summary>Disables the Enter key on headset.</summary>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_DisableEnterKey();

	/// <summary>Enables the Volume key on headset.</summary>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_EnableVolumeKey();

	/// <summary>Disables the Volume key on headset.</summary>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_DisableVolumeKey();

	/// <summary>Enables the Back key on headset.</summary>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_EnableBackKey();

	/// <summary>Enables the Back key on headset.</summary>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_DisableBackKey();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_GetCurrentBrightness();

	/// <summary>Sets the brightness for the screen.
	/// @note System permission is required if this function is called for the first time.
	/// </summary>
	/// <param name="Brightness">(In) Int, target brightness, value range: [0, 255]</param>
	/// <param name="Ext">(In) Int, Reserved bit</param>
	/// <returns>(Out) bool, Result</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_SetScreenBrightness(int Brightness, int Ext);

	/// <summary>Gets the current brightness of the screen.</summary>
	/// <returns>(Out) Int, the current brightness, value range: [0, 255]</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_GetCurrentVolume();

	/// <summary>Gets the maximum volume allowed for the device.</summary>
	/// <returns>(Out) Int, the maximum volume allowed.</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_GetMaxVolume();

	/// <summary>Sets a volume for the device.</summary>
	/// <param name="Volume">(In) Int, target volume, value range: [1, 15]</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetVolume(int32 Volume);

	
	/// <summary>Increases the volume of the device.</summary>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_VolumeUp();

	/// <summary>Decreases a volume for the device.</summary>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_VolumeDown();

	/// <summary>Gets the device's SN code.</summary>
	/// <returns>String, the device's SN code.</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetDeviceSN();

	/// <summary>Sets a volume for the device.</summary>
	/// <param name="Volume">(In) Int, target volume, value range: [1, 15]</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_FreezeScreen(bool freeze);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_KillAppsByPidOrPackageName(TArray<int> pids, TArray<FString> packageNames, int ext);

	/// <summary>Force quits background app(s) except for those in the allowlist.</summary>
	/// <param name="PackageNames">(In) FString Array, an array of package name(s) in the allowlist. These app(s) will not be force quit.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to 0 by default.</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_KillBackgroundAppsWithWhiteList(TArray<FString> packageNames, int ext);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_OpenMiracast();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_IsMiracastOn();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_CloseMiracast();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_StartScan();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_StopScan();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_ConnectWifiDisplay(const FWifiDisplayModel& Model);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_DisConnectWifiDisplay();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_ForgetWifiDisplay(FString Address);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_RenameWifiDisplay(FString Address, FString NewName);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_UpdateWifiDisplays();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_GetConnectedWD(FWifiDisplayModel& Model);

	static FPICOSetWDJsonDelegate SetWDJsonDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetWDJson(FPICOSetWDJsonDelegate InSetWDJsonDelegate);

	static FPICOSetWDModelsDelegate SetWDModelsDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetWDModels(FPICOSetWDModelsDelegate InSetWDModelsDelegate);

	/// <summary>Enables/disables large space at system level. Component PicoXR System API is required. Bind a callback event to acquire switch result.</summary>
	/// <param name="Target">(In) Object, connects to the Pico XRSystem API component.</param>
	/// <param name="InEnableLargeSpaceDelegate">(In) Delegate, bind the callback event for getting the result.
	/// Callback event params: Bool, 
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </param>
	/// <param name="Open">(In) Bool, whether to enable large space:
	/// <ul>
	/// <li>`true` - enable large space</li>
	/// <li>`false` - disable large space</li>
	/// </ul>
	/// </param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>None</returns>
	static FPICOEnableLargeSpaceDelegate EnableLargeSpaceDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SwitchLargeSpaceScene(FPICOEnableLargeSpaceDelegate InEnableLargeSpaceDelegate, bool open, int ext = 0);

	static FPICOSwitchLargeSpaceStatusDelegate SwitchLargeSpaceStatusDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_GetSwitchLargeSpaceStatus(FPICOSwitchLargeSpaceStatusDelegate InSwitchLargeSpaceStatusDelegate, int ext = 0);

	/// <summary>Saves large space maps. Component PicoXR System API is required.</summary>
	/// <param name="Target">(In) Object, connects to the Pico XRSystem API component.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>Bool:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_SaveLargeSpaceMaps(int ext = 0);

	/// <summary>Exports the map file. The exported file is saved to `internal storage/maps/export`.</summary>
	/// <param name="Target">(In) Object, connects to the Pico XRSystem API component.</param>
	/// <param name="InExportMapsDelegate">(In) Delegate, bind the callback event for getting the result.
	/// Callback event params: FString:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>None</returns>
	static FPICOExportMapsDelegate ExportMapsDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_ExportMaps(FPICOExportMapsDelegate InExportMapsDelegate, int ext = 0);

	/// <summary>Imports a map file. The file should be copied to `internal storage/maps/` beforehand, and then execute the import method.</summary>
	/// <param name="Target">(In) Object, connects to the Pico XRSystem API component.</param>
	/// <param name="InImportMapsDelegate">(In) Delegate, bind the callback event to get the result.
	/// Callback event params: FString:
	/// <ul>
	/// <li>`true` - success</li>
	/// <li>`false` - failure</li>
	/// </ul>
	/// </param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>None</returns>
	static FPICOImportMapsDelegate ImportMapsDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_ImportMaps(FPICOImportMapsDelegate InImportMapsDelegate, int ext = 0);

	/// <summary>Gets the CPU usage of the current device.</summary>
	/// <param name="Target">(In) Object, connects to the Pico XRSystem API component.</param>
	/// <param name="OutData">(Out) Float, the current CPU usage of the device.</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_GetCpuUsages(TArray<float>& OutData);

	/// <summary>Gets the temperature (℃) of the current device.</summary>
	/// <param name="Target">(In) Object, connects to the Pico XRSystem API component.</param>
	/// <param name="Type">(In) Int, the requested type of device temperature.
	/// <ul>
	/// <li>`0`: CPU temperature (DEVICE_TEMPERATURE_CPU)</li>
	/// <li>`1`: GPU temperature (DEVICE_TEMPERATURE_GPU)</li>
	/// <li>`2`: Battery temperature (DEVICE_TEMPERATURE_BATTERY)</li>
	/// <li>`3`: Surface temperature (DEVICE_TEMPERATURE_SKIN)</li>
	/// </ul></param>
	/// <param name="Source">(In) Int, the requested source of device temperature.
	/// <ul>
	/// <li>`0`: current temperature (TEMPERATURE_CURRENT)</li>
	/// <li>`1`: temperature threshold for throttling (TEMPERATURE_THROTTLING)</li>
	/// <li>`2`: temperature threshold for shutdown (TEMPERATURE_SHUTDOWN)</li>
	/// <li>`3`: temperature threshold for throttling (TEMPERATURE_THROTTLING_BELOW_VR_MIN). If the actual temperature is higher than the threshold, the lowest clock frequency for VR mode will not be met.</li>
	/// </ul></param>
	/// <param name="OutData">(Out) Float, an array of requested device temperature(s) (℃).</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_GetDeviceTemperatures(int inType, int inSource, TArray<float>& OutData);

	/// <summary>Captures the current screen.</summary>
	/// <param name="Target">(In) Object, connects to the Pico XRSystem API component.</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_Capture();

	/// <summary>Records the screen. Call this function again to stop recording.</summary>
	/// <param name="Target">(In) Object, connects to the Pico XRSystem API component.</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_Record();


	/// <summary>Connects the device to a specified WIFI.</summary>
	/// <param name="Target">(In) Object, connects to the Pico XRSystem API component.</param>
	/// <param name="InControlSetAutoConnectWIFIWithErrorCodeDelegate">(In) Dynamic proxy, can be bound to a custom event to output the "Result" parameter.</param>
	/// <param name="Ssid">(In) String, the WIFI name.</param>
	/// <param name="Pwd">(In) String, the WIFI password.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <param name="Result">(Out) Int, the error code for whether the WIFI connection is successful.
	/// <ul>
	/// <li>`0`: SUCCESS_CODE (connected)</li>
	/// <li>`1`: WIFI_CONNECT_PASSWORD_ERROR (password error) (TEMPERATURE_THROTTLING)</li>
	/// <li>`2`:WIFI_CONNECT_UNKNOWN_ERROR (unknown error)</li>
	/// </ul>
	/// </param>
	/// <returns>None</returns>
	static FPICOControlSetAutoConnectWIFIWithErrorCodeDelegate ControlSetAutoConnectWIFIWithErrorCodeDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_ControlSetAutoConnectWIFIWithErrorCode(FPICOControlSetAutoConnectWIFIWithErrorCodeDelegate InControlSetAutoConnectWIFIWithErrorCodeDelegate, FString ssid, FString pwd, int ext);

	/// <summary>Keeps an app active, i.e. raising the priority of the app, thereby in general situations, the system will not force quit the app.</summary>
	/// <param name="Target">(In) Object, connects to the Pico XRSystem API component.</param>
	/// <param name="AppPackageName">(In) String, the app package name.</param>
	/// <param name="KeepAlive">(In) Int, whether to keep the app active (i.e., whether to raise the priority of the app)
	/// <ul>
	/// <li>`true`: Yes</li>
	/// <li>`false`: No</li>
	/// </ul>
	/// </param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_AppKeepAlive(FString appPackageName, bool keepAlive, int ext);

	/// <summary>Sets a timer to turn on the PICO VR headset, including its specific time value and whether to enable the timer.</summary>
	/// <param name="Year">(In) Int, the year of the timer set.</param>
	/// <param name="Month">(In) Int, the month of the timer set.</param>
	/// <param name="Day">(In) Int, the day of the timer set.</param>
	/// <param name="Hour">(In) Int, the hour of the timer set.</param>
	/// <param name="Minute">(In) Int, the minute of the timer set.</param>
	/// <param name="Open">(In) Bool, whether to enable the timer:
	/// <ul>
	/// <li>`true`: enable</li>
	/// <li>`false`: disable</li>
	/// </ul>
	/// </param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_TimingStartup(int year, int month, int day, int hour, int minute, bool open);

	/// <summary>Sets a timer to shut down the PICO VR headset, including its specific time value and whether to enable the timer.</summary>
	/// <param name="Year">(In) Int, the year of the timer set.</param>
	/// <param name="Month">(In) Int, the month of the timer set.</param>
	/// <param name="Day">(In) Int, the day of the timer set.</param>
	/// <param name="Hour">(In) Int, the hour of the timer set.</param>
	/// <param name="Minute">(In) Int, the minute of the timer set.</param>
	/// <param name="Open">(In) Bool, whether to enable the timer:
	/// <ul>
	/// <li>`true`: enable</li>
	/// <li>`false`: disable</li>
	/// </ul>
	/// </param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_TimingShutdown(int year, int month, int day, int hour, int minute, bool open);

	/// <summary>Launches the specified settings page of the PICO VR headset.</summary>
	/// <param name="VrSettingsEnum">(In) Enum, defines which specific settings page to launch:
	/// <ul>
	/// <li>`0`: WIFI</li>
	/// <li>`1`: Bluetooth</li>
	/// <li>`2`: Controller</li>
	/// <li>`3`: Lab</li>
	/// <li>`4`: Brightness</li>
	/// <li>`5`: General</li>
	/// <li>`6`: Notification</li>
	/// </ul>
	/// </param>
	/// <param name="HideOtherItem">(In) Bool, defines whether to hide other pages:
	/// <ul>
	/// <li>`true`: Yes</li>
	/// <li>`false`: No</li>
	/// </ul>
	/// </param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_StartVrSettingsItem(int vrSettingsEnum, bool hideOtherItem, int ext);

	/// <summary>Modifies the function of the volume buttons. For example, you can change "Volume -" to "Home", "Volume +" to "Confirm", or revert to the volume buttons, if required.</summary>
	/// <param name="SwitchEnum">(In) Enum, defines whether to change the function of the volume button:
	/// <ul>
	/// <li>`on`: change</li>
	/// <li>`off`: do not change</li>
	/// </ul>
	/// </param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>None</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SwitchVolumeToHomeAndEnter(ESwitchEnum switchEnum, int ext);

	/// <summary>Gets the function of the current volume button. For example, to determine if the current volume button is modified to "Home" or "Confirm" button.</summary>
	/// <returns>Bool, the current status of the volume button:
	/// <ul>
	/// <li>`on`: changed</li>
	/// <li>`off`: not changed</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	ESwitchEnum PXR_IsVolumeChangeToHomeAndEnter();

	/// <summary>Upgrades the OTA.</summary>
	/// <param name="OtaPackagePath">(In) String, the location of the OTA package.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>Int, the status of the OTA upgrade:
	/// <ul>
	/// <li>`0`: success</li>
	/// <li>`1`: failure</li>
	/// <li>`2`: OTA package version is outdated</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	EInstallOTAReturnCode PXR_InstallOTAPackage(const FString& OtaPackagePath, int32 Ext);

	/// <summary>Gets the configuration of the Wi-Fi network that the device automatically connects to.</summary>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>String, the SSID and password of the Wi-Fi network.</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetAutoConnectWiFiConfig(int32 Ext);

	/// <summary>Gets the scheduled auto startup settings for the device.</summary>
	/// <param name="Target">(In) Object, set to `self` by default.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>
	/// <ul>
	/// <li>`open`: Bool, the status of scheduled auto startup:
	/// <ul>
	/// <li>`true`: enabled</li>
	/// <li>`false`: disabled</li>
	/// </ul>
	/// </li>
	/// <li>`time`: String, the time when the device auto starts up. Returned when `open` is `true`.</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetTimingStartupStatus(int32 Ext);

	/// <summary>Gets the scheduled auto shutdown settings for the device.</summary>
	/// <param name="Target">(In) Object, set to `self` by default.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>
	/// <ul>
	/// <li>`open`: Bool, the status of scheduled auto shutdown:
	/// <ul>
	/// <li>`true`: enabled</li>
	/// <li>`false`: disabled</li>
	/// </ul>
	/// </li>
	/// <li>`time`: String, the time when the device auto shuts down. Only returned when `open` is `true`.</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetTimingShutdownStatus(int32 Ext);

	/// <summary>Gets the status of a specified controller key.</summary>
	/// <param name="Target">(In) Object, set to `self` by default.</param>
	/// <param name="ControllerKeyEnum">(In) Enum, the enumerations of controller key:
	/// <ul>
	/// <li>`CONTROLLER KEY JOYSTICK`</li>
	/// <li>`CONTROLLER KEY MENU`</li>
	/// <li>`CONTROLLER KEY TRIGGER`</li>
	/// <li>`CONTROLLER KEY RIGHT A`</li>
	/// <li>`CONTROLLER KEY RIGHT B`</li>
	/// <li>`CONTROLLER KEY LEFT X`</li>
	/// <li>`CONTROLLER_KEY_LEFT_Y`</li>
	/// <li>`CONTROLLER_KEY_LEFT_GRIP`</li>
	/// <li>`CONTROLLER_KEY_RIGHT_GRIP`</li>
	/// </ul>
	/// </param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>Int, the key's status:
	/// <ul>
	/// <li>`0`: disabled</li>
	/// <li>`1`: enabled</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	EKeyState PXR_GetControllerKeyState(EControllerKeyEnum ControllerKeyEnum, int32 Ext);

	/// <summary>Gets the status of the switch for setting whether to power off the USB cable when the device is shut down.</summary>
	/// <param name="Target">(In) Object, set to `self` by default.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>Enum, the switch's status:
	/// <ul>
	/// <li>`PBS_SwitchEnum#S_ON`: on</li>
	/// <li>`PBS_SwitchEnum#S_OFF`: off</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	ESwitchEnum PXR_GetPowerOffWithUSBCable(int32 Ext);

	/// <summary>Gets the current screen off time.</summary>
	/// <param name="Target">(In) Object, set to `self` by default.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>Int, the current screen off time in seconds.</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	EScreenOffDelayTimeEnum PXR_GetScreenOffDelay(int32 Ext);

	/// <summary>Gets the current system sleep off time.</summary>
	/// <param name="Target">(In) Object, set to `self` by default.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>Int, the current system sleep off time in seconds.</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	ESleepDelayTimeEnum PXR_GetSleepDelay(int32 Ext);

	/// <summary>Gets the Power key's settings.</summary>
	/// <param name="Target">(In) Object, set to `self` by default.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>A dictionary with the following keys:
	/// <ul>
	/// <li>`null`: Not set</li>
	/// <li>`singleTap`: Whether a single-tap event has been set</li>
	/// <li>`longTap`: Whether a long-press event has been set</li>
	/// <li>`longPressTime`: The time after which the long-press event takes place. Returned when `longTap` is `true`.</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetPowerKeyStatus(int32 Ext);

	/// <summary>Gets the Enter key's status.</summary>
	/// <param name="Target">(In) Object, set to `self` by default.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>Int, the key's status:
	/// <ul>
	/// <li>`0`: disabled</li>
	/// <li>`1`: enabled</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	EKeyState PXR_GetEnterKeyStatus(int32 Ext);

	/// <summary>Gets the Volume key's status.</summary>
	/// <param name="Target">(In) Object, set to `self` by default.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>Int, the key's status:
	/// <ul>
	/// <li>`0`: disabled</li>
	/// <li>`1`: enabled</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	EKeyState PXR_GetVolumeKeyStatus(int32 Ext);

	/// <summary>Gets the Back key's status.</summary>
	/// <param name="Target">(In) Object, set to `self` by default.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>Int, the key's status:
	/// <ul>
	/// <li>`0`: disabled</li>
	/// <li>`1`: enabled</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	EKeyState PXR_GetBackKeyStatus(int32 Ext);

	/// <summary>Gets the event setting for the Home key.</summary>
	/// <param name="Target">(In) Object, set to `self` by default.</param>
	/// <param name="HomeEventEnum">(In) Enum, the event type for the Home key. Possible values:
	/// <br>`SINGLE_CLICK`: single-click event
	/// <br>`DOUBLE_CLICK`: double-click event
	/// <br>`LONG_PRESS`: long-press event
	/// </param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>The return value varies based on the event type:
	/// <ul>
	/// <li>For `SINGLE_CLICK` and `DOUBLE_CLICK`, the event(s) you set will be returned.</li>
	/// <li>For `LONG_PRESS`, the time and event you set will be returned. If you have not set a time for a long-press event, the time value will be `null`.</li>
	/// <li>If you have not set any event for the specified event type, the response will return `null`.</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetHomeKeyStatus(EHomeEventEnum HomeEventEnum, int32 Ext);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetUsbConfigurationOption(int32 Ext);

	/// <summary>Gets the current launcher.</summary>
	/// <returns>String, the package name or class name of the current launcher.</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetCurrentLauncher(int32 Ext);

	/// <summary>Gets the status of a specified system function switch.</summary>
	/// <param name="SystemFunctionSwitchEnum">(In) The system function switch:
	/// <ul>
	/// <li>`USB Debug`</li>
	/// <li>`Auto Sleep`</li>
	/// <li>`ScreenOn Charging`</li>
	/// <li>`OTG Charging`</li>
	/// <li>`Show Back Menu in 2D Mode`</li>
	/// <li>`Combination Key`</li>
	/// <li>`Calibration with power on`</li>
	/// <li>`System Update`</li>
	/// <li>`Cast Service`</li>
	/// <li>`Eye Protection`</li>
	/// <li>`Security Zone Permantly`</li>
	/// <li>`Global Calibration`</li>
	/// <li>`Auto Calibration`</li>
	/// <li>`USB Boot`</li>
	/// <li>`Volume Global UI Prompt Switch`</li>
	/// <li>`Controller Connection Global UI Prompt Switch`</li>
	/// <li>`Open/Close the Navigation`</li>
	/// <li>`Show Record UI`</li>
	/// <li>`Show Fit UI`</li>
	/// <li>`Show Cast UI`</li>
	/// <li>`Show Capture UI`</li>
	/// <li>`Stop Mem Infor Service`</li>
	/// <li>`Start App Boot Completed`</li>
	/// <li>`Usb Force Host`</li>
	/// <li>`Set Default Safty Zone`</li>
	/// <li>`Allow Reset Boundary`</li>
	/// <li>`Boundary Confirmation Screen`</li>
	/// <li>`Long Press Home For Recenter`</li>
	/// <li>`Power Ctrl Wifi Enable`</li>
	/// <li>`Wifi Disable`</li>
	/// <li>`Six Dof Switch`</li>
	/// <li>`Inverse Dispersion`</li>
	/// <li>`Logcat`</li>
	/// <li>`PSensor`</li>
	/// <li>`System Update OTA`</li>
	/// <li>`System Update App`</li>
	/// <li>`Shortcut Show Wlan UI`</li>
	/// <li>`Shortcut Show Boundary UI`</li>
	/// <li>`Shortcut Show Bluetooth UI`</li>
	/// <li>`Shortcut Show Clean Task UI`</li>
	/// <li>`Shortcut Show IPD Adjustment UI`</li>
	/// <li>`Shortcut Show Power UI`</li>
	/// <li>`Shortcut Show Edit UI`</li>
	/// <li>`Basic Setting App Library UI`</li>
	/// <li>`Basic Setting Shortcut UI`</li>
	/// </ul>
	/// </param>
	/// <returns>None</returns>
	static FPICOGetSwitchSystemFunctionStatusDelegate GetSwitchSystemFunctionStatusDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_GetSwitchSystemFunctionStatus(FPICOGetSwitchSystemFunctionStatusDelegate InGetSwitchSystemFunctionStatusDelegate, ESystemFunctionSwitchEnum SystemFunctionSwitchEnum, int32 Ext);

	/// <summary>Initializes the screencast service.</summary>
	/// <param name="InPICOCastInitDelegate">(In) The callback:
	/// <ul>
	/// <li>`0`: disconnect</li>
	/// <li>`1`: connect</li>
	/// <li>`2`: no mic permission</li>
	/// </ul>
	/// </param>
	/// <returns>Int:
	/// <ul>
	/// <li>`0`: failure</li>
	/// <li>`1`: success</li>
	/// </ul>
	/// </returns>
	static FPICOCastInitDelegate PICOCastInitDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	ECastReturnCode PXR_PICOCastInit(FPICOCastInitDelegate InPICOCastInitDelegate, int32 Ext);

	/// <summary>Sets whether to show the screencast authorization window.</summary>
	/// <param name="Authorization">(In) The authorization setting:
	/// <ul>
	/// <li>`0`: ask every time (default)</li>
	/// <li>`1`: always allow</li>
	/// <li>`2`: not accepted</li>
	/// </ul>
	/// </param>
	/// <returns>Int, the status of the operation:
	/// <ul>
	/// <li>`0`: failure</li>
	/// <li>`1`: success</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	ECastReturnCode PXR_PICOCastSetShowAuthorization(EPICOCastAuthorization Authorization, int32 Ext);

	/// <summary>Gets the setting of whether to show the screencast authorization window.</summary>
	/// <returns>Int, the current authorization setting:
	/// <ul>
	/// <li>`0`: ask every time (default)</li>
	/// <li>`1`: always allow</li>
	/// <li>`2`: not accepted</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	EPICOCastAuthorization PXR_PICOCastGetShowAuthorization(int32 Ext);

	/// <summary>Gets the URL for screencast.</summary>
	/// <param name="UrlType">(In) Enum, the type of URL:
	/// <ul>
	/// <li>`NormalURL`: Normal URL. The screencast authorization window will show if it is not set.</li>
	/// <li>`NoConfirmURL`: Non-confirm URL. The screencast authorization window will not show in the browser. Screencast will start once you enter the URL.</li>
	/// <li>`RtmpURL`: Returns the RTMP live streaming URL. The screencast authorization window will not appear on the VR headset's screen.</li>
	/// </ul>
	/// </param>
	/// <returns>String, the URL for screencast.</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_PICOCastGetUrl(EPICOCastUrlTypeEnum UrlType, int32 Ext);

	/// <summary>Stops screencast.</summary>
	/// <returns>Int, the status of stopping screencast:
	/// <ul>
	/// <li>`0`: failure</li>
	/// <li>`1`: success</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	ECastReturnCode PXR_PICOCastStopCast(int32 Ext);

	/// <summary>Sets screencast-related properties.</summary>
	/// <param name="OptionEnum">(In) The enumerations of the property to set:
	/// <br>- `OPTION_RESOLUTION_LEVEL`: resolution level
	/// <br>- `OPTION_BITRATE_LEVEL`: bitrate level
	/// <br>- `OPTION_AUDIO_ENABLE`: whether to enable the audio</param>
	/// <param name="ValueEnum">(In) The values that can be set for each property:
	/// <br>- For `OPTION_RESOLUTION_LEVEL`:
	/// <br>  - `OPTION_VALUE_RESOLUTION_HIGH`
	/// <br>  - `OPTION_VALUE_RESOLUTION_MIDDLE`
	/// <br>  - `OPTION_VALUE_RESOLUTION_AUTO`
	/// <br>  - `OPTION_VALUE_RESOLUTION_HIGH_2K`
	/// <br>  - `OPTION_VALUE_RESOLUTION_HIGH_4K`
	/// <br>- For `OPTION_BITRATE_LEVEL`:
	/// <br>  - `OPTION_VALUE_BITRATE_HIGH`
	/// <br>  - `OPTION_VALUE_BITRATE_MIDDLE`
	/// <br>  - `OPTION_VALUE_BITRATE_LOW`
	/// <br>- For `OPTION_AUDIO_ENABLE`:
	/// <br>  - `OPTION_VALUE_AUDIO_ON`
	/// <br>  - `OPTION_VALUE_AUDIO_OFF`</param>
	/// <returns>Int
	/// <ul>
	/// <li>`0`: failure</li>
	/// <li>`1`: success</li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	ECastReturnCode PXR_PICOCastSetOption(EPICOCastOptionOrStatusEnum OptionEnum, EPICOCastOptionValueEnum ValueEnum, int32 Ext);

	/// <summary>Gets the screencast-related property setting for the current device.</summary>
	/// <param name="OptionEnum">(In) The enumerations of the screencast property to get setting for:
	/// <br>- `OPTION_RESOLUTION_LEVEL`: resolution level
	/// <br>- `OPTION_BITRATE_LEVEL`: bitrate level
	/// <br>- `OPTION_AUDIO_ENABLE`: whether the audio is enabled
	/// <br>- `PICOCAST_STATUS`: returns the current screemcast status</param>
	/// <returns>String, the setting of the selected property:
	/// <ul>
	/// <li>For `OPTION_RESOLUTION_LEVEL`:
	///   <ul>
	///     <li>`OPTION_VALUE_RESOLUTION_HIGH`</li>
	///     <li>`OPTION_VALUE_RESOLUTION_MIDDLE`</li>
	///     <li>`OPTION_VALUE_RESOLUTION_AUTO`</li>
	///     <li>`OPTION_VALUE_RESOLUTION_HIGH_2K`</li>
	///     <li>`OPTION_VALUE_RESOLUTION_HIGH_4K`</li>
	///   </ul>
	/// </li>
	/// <li>For `OPTION_BITRATE_LEVEL`:
	///   <ul>
	///     <li>`OPTION_VALUE_BITRATE_HIGH`</li>
	///     <li>`OPTION_VALUE_BITRATE_MIDDLE`</li>
	///     <li>`OPTION_VALUE_BITRATE_LOW`</li>
	///   </ul>
	/// </li>
	/// <li>For `OPTION_AUDIO_ENABLE`:
	///   <ul>
	///     <li>`OPTION_VALUE_AUDIO_ON`</li>
	///     <li>`OPTION_VALUE_AUDIO_OFF`</li>
	///   </ul>
	/// </li>
	/// <li>`PICOCAST_STATUS`:
	///   <ul>
	///     <li>`STATUS_VALUE_STATE_STARTED`</li>
	///     <li>`STATUS_VALUE_STATE_STOPPED`</li>
	///     <li>`STATUS_VALUE_ERROR`</li>
	///   </ul>
	/// </li>
	/// </ul>
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	EPICOCastOptionValueEnum PXR_PICOCastGetOptionOrStatus(EPICOCastOptionOrStatusEnum OptionEnum, int32 Ext);

	/// <summary>Customizes the duration of the controller in pairing mode.</summary>
	/// <param name="TimeEnum">(In) Enum, pairing mode duration:
	/// <br>- `SIX`: 6 seconds
	/// <br>- `FIFTEEN`: 15 seconds
	/// <br>- `SIXTY`: 60 seconds
	/// <br>- `ONE_HUNDRED_AND_TWENTY`: 2 minutes
	/// <br>- `SIX_HUNDRED`: 10 minutes
	/// <br>- `NEVER`: No pairing mode duration</param>
	/// <param name="InSetControllerPairTimeDelegate">(In) Delegate, bind the callback event to get the result:
	/// <br>Callback event parameters: Int, `0` - failure; `1` - success</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>None</returns>
	static FPICOSetControllerPairTimeDelegate SetControllerPairTimeDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetControllerPairTime(FPICOSetControllerPairTimeDelegate InSetControllerPairTimeDelegate, EControllerPairTimeEnum TimeEnum, int32 Ext);

	/// <summary>
	/// Gets the duration of the controller in pairing mode.
	/// </summary>
	/// <param name="InGetControllerPairTimeDelegate">(In) Delegate, bind the callback event to get the result:
	/// Callback event parameters: Int, `0` - failure; `1` - success</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>Result: EControllerPairTimeEnum
	/// - `SIX`: 6 seconds
	/// - `FIFTEEN`: 15 seconds
	/// - `SIXTY`: 60 seconds
	/// - `ONE_HUNDRED_AND_TWENTY`: 2 minutes
	/// - `SIX_HUNDRED`: 10 minutes
	/// - `NEVER`: No pairing mode duration</returns>
	static FPICOGetControllerPairTimeDelegate GetControllerPairTimeDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_GetControllerPairTime(FPICOGetControllerPairTimeDelegate InGetControllerPairTimeDelegate, int32 Ext);

	/// <summary>
	/// (Only supported by PICO 4 Series devices) Customizes the language on the device.
	/// </summary>
	/// <param name="SystemLanguage">(In) FString, supported language codes (The system language is co-determined by both the language code and the device's country code).</param>
	/// <param name="Ext">(In) Int, reserved parameter.
	/// - `0`: Default
	/// - `1`: Reboot nativeshell and 2D apps on the system will exit.</param>
	/// <returns>
	/// - `0`: Success
	/// - `1`: Failure
	/// - `22`: The language is not supported.
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_SetSystemLanguage(const FString& SystemLanguage, int32 Ext);

	/// <summary>
	/// (Only supported by PICO 4 Series) Gets the current language on the device.
	/// </summary>
	/// <param name="Ext">(In) Int, reserved parameter, pass 0 by default.</param>
	/// <returns>
	/// System language: FString, the language set on the device.
	/// - "fi": Suomi (Finnish)
	/// - "el": Ελληνικά (Greek)
	/// - "ru": Русский (Russian)
	/// - "en": English (United States)/English (United Kingdom)
	/// - "zh": 中文 (简体)(Chinese, Simplified)/中文 (繁體)(Chinese, Traditional (Taiwan))/中文 (香港)(Chinese, Traditional (Hongkong))
	/// - "pt": Português (Portugal/Brasil)
	/// - "fr": Français (French)
	/// - "cs": Čeština (Czech)
	/// - "tr": Türkçe (Turkish)
	/// - "pl": Polski (Polish)
	/// - "th": ไทย (Thai)
	/// - "sv": Svenska (Swedish)
	/// - "ja": 日本語 (Japanese)
	/// - "nl": Nederlands (Dutch)
	/// - "da": Dansk (Dansh)
	/// - "nb": Norsk bokmål (Norwegian Bokmål)
	/// - "ms": Melayu (Malay)
	/// - "de": Deutsch (German)
	/// - "es": Español (España) (Spanish)/Estados Unidos (Spanish(Latin American))
	/// - "ko": 한국어 (Korean)
	/// - "ro": Română (Romanian)
	/// - "it": Italiano (Italian)
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetSystemLanguage(int32 Ext);

	/// <summary>
	/// (Only supported by PICO 4 Series) Set the default connected Wi-Fi.
	/// </summary>
	/// <param name="Ssid">(In) FString, the ID of specified Wi-Fi.</param>
	/// <param name="Pwd">(In) FString, the password of specified Wi-Fi.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to 0 by default.</param>
	/// <returns>
	/// - 0: Success
	/// - 1: Failure
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_ConfigWifi(const FString& Ssid, const FString& Pwd, int32 Ext);

	/// <summary>
	/// (Only supported by PICO 4 Series) Gets the name of the connected WiFi(s).
	/// </summary>
	/// <param name="Ext">(In) Int, reserved parameter, set to 0 by default.</param>
	/// <returns>FString array, an array of Wifi names</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	TArray<FString> PXR_GetConfiguredWifi(int32 Ext);

	/// <summary>
	/// (Only supported by PICO 4 Series) Customizes the country on the device.
	/// </summary>
	/// <param name="CountryCode">(In) FString, the country code.
	/// <br>- `GB`: United Kingdom
	/// <br>- `FR`: France
	/// <br>- `DE`: Germany
	/// <br>- `IL`: Israel
	/// <br>- `IT`: Italy
	/// <br>- `JP`: Japan
	/// <br>- `KR`: Korea
	/// <br>- `NL`: Netherlands
	/// <br>- `SG`: Singapore
	/// <br>- `ES`: Spain
	/// <br>- `TR`: Turkey
	/// <br>- `MY`: Malaysia
	/// <br>- `AT`: Austria
	/// <br>- `BE`: Belgium
	/// <br>- `BG`: Bulgaria
	/// <br>- `HR`: Croatia
	/// <br>- `CZ`: Czechia
	/// <br>- `DK`: Denmark
	/// <br>- `EE`: Estonia
	/// <br>- `FI`: Finland
	/// <br>- `GR`: Greek
	/// <br>- `HU`: Hungary
	/// <br>- `IE`: Ireland
	/// <br>- `LV`: Latvia
	/// <br>- `LT`: Lithuania
	/// <br>- `LU`: Luxembourg
	/// <br>- `MT`: Malta
	/// <br>- `PL`: Poland
	/// <br>- `PT`: Portugal
	/// <br>- `RO`: Romania
	/// <br>- `SK`: Slovakia
	/// <br>- `SI`: Slovenia
	/// <br>- `SE`: Sweden
	/// <br>- `LI`: Liechtenstein
	/// <br>- `IS`: Iceland
	/// <br>- `NO`: Norway
	/// <br>- `CH`: Switzerland
	/// <br>- `AD`: Andorra
	/// <br>- `SM`: San Marino
	/// <br>- `VA`: Vatican City
	/// <br>- `MC`: Monaco
	/// <br>- `US`: United States
	/// <br>- `CA`: Canada
	/// <br>- `NZ`: New Zealand
	/// <br>- `AU`: Australia</param>
	/// <param name="InSetSystemCountryCodeDelegate">Delegate, bind the callback event to get the result:
	/// <br>Callback event parameters: Int; `0` - success, `1` - failure</param>
	/// <param name="Ext">Int, reserved parameter, pass `0` by default.</param>
	/// <returns>None</returns>
	static FPICOSetSystemCountryCodeDelegate SetSystemCountryCodeDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_SetSystemCountryCode(FPICOSetSystemCountryCodeDelegate InSetSystemCountryCodeDelegate, const FString& CountryCode, int32 Ext);

	/// <summary>
	/// (Only supported by PICO 4 Series) Gets the country code of the country set on the device.
	/// </summary>
	/// <param name="Ext">(In) Int, reserved parameter, pass 0 by default.</param>
	/// <returns>Country Code: FString, the country code of the country set on the device.
	/// <br>- `GB`: United Kingdom
	/// <br>- `FR`: France
	/// <br>- `DE`: Germany
	/// <br>- `IL`: Israel
	/// <br>- `IT`: Italy
	/// <br>- `JP`: Japan
	/// <br>- `KR`: Korea
	/// <br>- `NL`: Netherlands
	/// <br>- `SG`: Singapore
	/// <br>- `ES`: Spain
	/// <br>- `TR`: Turkey
	/// <br>- `MY`: Malaysia
	/// <br>- `AT`: Austria
	/// <br>- `BE`: Belgium
	/// <br>- `BG`: Bulgaria
	/// <br>- `HR`: Croatia
	/// <br>- `CZ`: Czechia
	/// <br>- `DK`: Denmark
	/// <br>- `EE`: Estonia
	/// <br>- `FI`: Finland
	/// <br>- `GR`: Greek
	/// <br>- `HU`: Hungary
	/// <br>- `IE`: Ireland
	/// <br>- `LV`: Latvia
	/// <br>- `LT`: Lithuania
	/// <br>- `LU`: Luxembourg
	/// <br>- `MT`: Malta
	/// <br>- `PL`: Poland
	/// <br>- `PT`: Portugal
	/// <br>- `RO`: Romania
	/// <br>- `SK`: Slovakia
	/// <br>- `SI`: Slovenia
	/// <br>- `SE`: Sweden
	/// <br>- `LI`: Liechtenstein
	/// <br>- `IS`: Iceland
	/// <br>- `NO`: Norway
	/// <br>- `CH`: Switzerland
	/// <br>- `AD`: Andorra
	/// <br>- `SM`: San Marino
	/// <br>- `VA`: Vatican City
	/// <br>- `MC`: Monaco
	/// <br>- `US`: United States
	/// <br>- `CA`: Canada
	/// <br>- `NZ`: New Zealand
	/// <br>- `AU`: Australia</returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetSystemCountryCode(int32 Ext);

	/// <summary>
	/// (Only supported by PICO 4 Series) Sets to skip specific initial settings.
	/// </summary>
	/// <param name="Flag">(In) Int; The first 6 elements are valid, 7-32 reserved.
	/// <br>- For each element: `1` - show, `0` - hide
	/// <br>- `INIT_SETTING_HANDLE_CONNECTION_TEACHING`: Controller connection teaching.
	/// <br>- `INIT_SETTING_TRIGGER_KEY_TEACHING`: Trigger key teaching.
	/// <br>- `INIT_SETTING_SELECT_LANGUAGE`: Select the system language.
	/// <br>- `INIT_SETTING_SELECT_COUNTRY`: Select the country (only supported outside China).
	/// <br>- `INIT_SETTING_WIFI_SETTING`: WiFi settings.
	/// <br>- `INIT_SETTING_QUICK_SETTING`: Quick settings.</param>
	/// <returns>
	/// - 0: Success
	/// - 1: Failure
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_SetSkipInitSettingPage(int32 Flag, int32 Ext);

	/// <summary>
	/// (Only supported by PICO 4 Series) Gets what initial settings are skipped.
	/// </summary>
	/// <param name="Ext">(In) Int, reserved parameter, pass `0` by default.</param>
	/// <returns>
	/// - Flag: Int, the flags set in `SetSkipInitSettingPage`
	/// - `1` :show
	/// - `0`: hide
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_GetSkipInitSettingPage(int32 Ext);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_IsInitSettingComplete(int32 Ext);

	/// <summary>
	/// (Supported by PICO Neo3 series and PICO4) Launches another app.
	/// </summary>
	/// <param name="PackageName">(In) The app's package name</param>
	/// <param name="ClassName">(In) The app class name</param>
	/// <param name="Action">(In) The action to start an app</param>
	/// <param name="Extra">(In) FString, the basic types of standard fields that can be used as extra data. See [here](https://developer.android.com/reference/android/content/Intent#standard-extra-data) for details.</param>
	/// <param name="Categories">(In) FString Array, Standard categories that can be used to further clarify an Intent. Add a new category to the intent. See [here](https://developer.android.com/reference/android/content/Intent#addCategory(java.lang.String)) for details.</param>
	/// <param name="Flags">(In) Add additional flags to the intent. See [here](https://developer.android.com/reference/android/content/Intent#flags) for details.</param>
	/// <param name="Ext">(In) Int, reserved parameter, pass `0` by default.</param>
	/// <returns>
	/// - 0: Success
	/// - 1: Failure
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_StartActivity(const FString& PackageName, const FString& ClassName, const FString& Action, const FString& Extra, const TArray<FString>& Categories, const TArray<int32>& Flags, int32 Ext);

	/// <summary>
	/// Shows or hides specified apps in the app library.
	/// </summary>
	/// <param name="Target">(In) Set to `self` by default.</param>
	/// <param name="PackageNames">(In) String, the package names of your applications. Separated by comma.</param>
	/// <param name="Switch">(In) The switch value:
	/// - `on`
	/// - `off`</param>
	/// <returns>
	/// A list with the following values:
	/// - 0: Success
	/// - 1: Failure
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_CustomizeAppLibrary(const TArray<FString>& PackageNames, ESwitchEnum SwitchEnum, int32 Ext);

	/// <summary>
	/// Gets the current battery level of controllers.
	/// </summary>
	/// <param name="Target">(In) Set to `self` by default.</param>
	/// <param name="Ext">(In) Reserved parameter, pass `0` by default.</param>
	/// <returns>
	/// An integer array with the following values:
	/// - array[0]: Left controller battery level
	/// - array[1]: Right controller battery level
	/// - -1: Success
	/// - 1-5: Battery level
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	TArray<int32> PXR_GetControllerBattery(int32 Ext);

	/// <summary>
	/// Gets the connection state of controllers.
	/// </summary>
	/// <param name="Target">(In) Set to `self` by default.</param>
	/// <param name="Ext">(In) Reserved parameter, pass `0` by default.</param>
	/// <returns>
	/// An integer value representing the connection state of the controllers:
	/// - 0: Both controllers are not connected
	/// - 1: Only the left controller is connected
	/// - 2: Only the right controller is connected
	/// - 3: Both controllers are connected
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_GetControllerConnectState(int32 Ext);

	/// <summary>
	/// Gets the hidden applications in your app library.
	/// </summary>
	/// <param name="Target">(In) Set to `self` by default.</param>
	/// <param name="Ext">(In) Int, reserved parameter, set to `0` by default.</param>
	/// <returns>
	/// The package names of the hidden applications. Separated by comma.
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetAppLibraryHideList(int32 Ext);

	/// <summary>
	/// Sets the device that outputs audio during screen casting.
	/// </summary>
	/// <param name="Target">(In) Set to `self` by default.</param>
	/// <param name="ScreencastAudioOutput">(In) Enum, the audio output end:
	/// - `AUDIO SINK`: the HMD
	/// - `AUDIO TARGET`: the receiver
	/// - `AUDIO SINK TARGET`: both the HMD and the receiver</param>
	/// <param name="Ext">(In) Reserved parameter, pass `0` by default.</param>
	/// <returns>
	/// - `0`: Success
	/// - `1`: Failure
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_SetScreenCastAudioOutput(EScreencastAudioOutput ScreencastAudioOutput, int32 Ext);

	/// <summary>
	/// Gets the device that outputs audio during screen casting.
	/// </summary>
	/// <param name="Target">(In) Set to `self` by default.</param>
	/// <param name="Ext">(In) Reserved parameter, pass `0` by default.</param>
	/// <returns>
	/// The audio output end:
	/// - `AUDIO SINK`: the HMD
	/// - `AUDIO TARGET`: the receiver
	/// - `AUDIO SINK TARGET`: both the HMD and the receiver
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	EScreencastAudioOutput PXR_GetScreenCastAudioOutput(int32 Ext);

	/// <summary>
	/// Displays or hides the specified tab or option on the Settings pane.
	/// </summary>
	/// <param name="Target">(In) Set to `self` by default.</param>
	/// <param name="CustomizeSettingsTab">(In) Enum, specifies the tab or option to display or hide.</param>
	/// <param name="Switch">(In) Enum, specifies whether to display or hide the tab or option.</param>
	/// <returns>
	/// - 0: Success
	/// - 1: Failure
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_CustomizeSettingsTabStatus(ECustomizeSettingsTab CustomizeSettingsTab, ESwitchEnum SwitchEnum, int32 Ext);

	/// <summary>
	/// Gets the status set for the specified tab or option on the Settings pane.
	/// </summary>
	/// <param name="Target">(In) Set to `self` by default.</param>
	/// <param name="CustomizeSettingsTab">(In) Enum, specifies the tab or option to get status for:</param>
	/// <returns>
	/// The status of the specified tab or option:
	/// - `On`: display
	/// - `Off`: hide
	/// </returns>
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	ESwitchEnum PXR_GetCustomizeSettingsTabStatus(ECustomizeSettingsTab CustomizeSettingsTab, int32 Ext);

	static FPICOSetIPDDelegate SetIPDDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetIPD(FPICOSetIPDDelegate InSetIPDDelegate, float IPD);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_SetPICOCastMediaFormat(const FCastMediaFormat& CastMediaFormat, int32 Ext);

	static FPICOSetArUcoMarkerDelegate SetArUcoMarkerDelegate;
	static UObject* MarkerWorldContext;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI", meta = (WorldContext = "InWorldContext"))
	void PXR_SetArUcoMarker(UObject* InWorldContext, FPICOSetArUcoMarkerDelegate InSetArUcoMarkerDelegate);
	
	static FPICOSetArUcoMarkerFDelegate SetArUcoMarkerFDelegate;
	static UObject* MarkerFWorldContext;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI", meta = (WorldContext = "InWorldContext"))
	void PXR_SetArUcoMarkerF(UObject* InWorldContext, FPICOSetArUcoMarkerFDelegate InSetArUcoMarkerFDelegate);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_InitEnterpriseService(FString& Token);
	
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_GetHeadTrackingConfidence();
	
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_OpenVSTCamera();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_CloseVSTCamera();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_AcquireVSTCameraFrame(FVSTCameraFrameData& FrameData);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_AcquireVSTCameraFrameAntiDistortion(const FString& Token, int32 Width, int32 Height, FVSTCameraFrameData& FrameData);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_GetCameraParameters(const FString& Token, FRGBCameraParams& Params);
	
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_GotoSeeThroughFloorSetting(int32 Ext);

	static FPICOFileCopyDelegate FileCopyDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	int32 PXR_FileCopy(const FString& SourcePath, const FString& TargetPath, FPICOFileCopyDelegate InFileCopyDelegate);

	static FPICOIsMapInEffectDelegate IsMapInEffectDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_IsMapInEffect(const FString& MapPath, FPICOIsMapInEffectDelegate InIsMapInEffectDelegate, int32 Ext);

	static FPICOImportMapByPathDelegate ImportMapByPathDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_ImportMapByPath(const FString& MapPath, FPICOImportMapByPathDelegate InImportMapByPathDelegate, int32 Ext);

	static FPICOEnterpriseIntDelegate SetWifiP2PDeviceNameDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_SetWifiP2PDeviceName(const FString& DeviceName, int Ext, FPICOEnterpriseIntDelegate Delegate);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	FString PXR_GetWifiP2PDeviceName(int Ext);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
    bool PXR_SetSystemKeyUsability(ESystemKey key, int Ext);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_SetLauncher(FString PackageName);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_OpenTimingStartup(int Year, int Month, int Day, int Hour, int Minute);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_CloseTimingStartup();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_OpenTimingShutdown(int Year, int Month, int Day, int Hour, int Minute);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_CloseTimingShutdown();

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_SetTimeZone(FString TimeZone);

	static FPICOEnterpriseIntDelegate AppCopyrightVerifyDelegate;
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	void PXR_AppCopyrightVerify(FString PackageName, FPICOEnterpriseIntDelegate Delegate);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	bool PXR_GotoEnvironmentTextureCheck();
};

UCLASS()
class PICOENTERPRISE_API UPICOEnterpriseFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	static void PXR_ToUnrealPose(FVector& UnrealTranslation, FQuat& UnrealOrientation, const FVector& RMUVector, const FQuat& RMUOrientation);
	
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	static void PXR_ToRGBCameraPose(FVector& CameraTranslation, FQuat& CameraOrientation, const FRGBCameraParams& CameraParameters, const FVector& HeadTranslation, const FQuat& HeadOrientation);

	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	static void PXR_UpdateRenderTargetFromRGB(const TArray<uint8>& RawData, int32 Width, int32 Height, UTextureRenderTarget2D* RenderTarget2D, uint8 OverrideAlpha = 255);
	
	UFUNCTION(BlueprintCallable, Category = "PXR|PXRSystemAPI")
	static void PXR_UpdateRenderTargetFromYUVNV21(const TArray<uint8>& RawData, int32 Width, int32 Height, UTextureRenderTarget2D* RenderTarget2D, uint8 OverrideAlpha = 255);
};