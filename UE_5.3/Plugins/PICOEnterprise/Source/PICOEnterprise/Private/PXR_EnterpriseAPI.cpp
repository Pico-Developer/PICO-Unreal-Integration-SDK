// Copyright® 2015-2023 PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Unreal® Engine, Copyright 1998 – 2023, Epic Games, Inc. All rights reserved. 

#include "PXR_EnterpriseAPI.h"

#include "IXRTrackingSystem.h"
#include "PXR_HMDFunctionLibrary.h"
#include "PXR_InterfaceWrapper.h"

DEFINE_LOG_CATEGORY_STATIC(PxrSystemAPI, Log, All);

TMap<EDeviceControlEnum, FPICOSetDeviceActionDelegate> UPICOXRSystemAPI::SetDeviceActionDelegates;
TMap<FAppManagerStruct, FPICOAppManagerDelegate> UPICOXRSystemAPI::AppManagerDelegates;
FPICOSetAutoConnectWifiDelegate UPICOXRSystemAPI::SetAutoConnectWifiDelegate;
FPICOClearAutoConnectWifiDelegate UPICOXRSystemAPI::ClearAutoConnectWifiDelegate;
TMap<EHomeEventEnum, FPICOSetHomeKeyDelegate> UPICOXRSystemAPI::SetHomeKeyDelegates;
TMap<EHomeEventEnum, FPICOSetHomeKeyAllDelegate> UPICOXRSystemAPI::SetHomeKeyAllDelegates;
TMap<bool, FPICODisablePowerKeyDelegate> UPICOXRSystemAPI::DisablePowerKeyDelegates;
FPICOSetScreenOffDelayDelegate UPICOXRSystemAPI::SetScreenOffDelayDelegate;
FPICOWriteConfigFileToDataLocalDelegate UPICOXRSystemAPI::WriteConfigDelegate;
FPICOResetAllKeyToDefaultDelegate UPICOXRSystemAPI::ResetAllKeyDelegate;
FPICOSetWDJsonDelegate UPICOXRSystemAPI::SetWDJsonDelegate;
FPICOSetWDModelsDelegate UPICOXRSystemAPI::SetWDModelsDelegate;

FPICOEnableLargeSpaceDelegate UPICOXRSystemAPI::EnableLargeSpaceDelegate;
FPICOSwitchLargeSpaceStatusDelegate UPICOXRSystemAPI::SwitchLargeSpaceStatusDelegate;
FPICOExportMapsDelegate UPICOXRSystemAPI::ExportMapsDelegate;
FPICOImportMapsDelegate UPICOXRSystemAPI::ImportMapsDelegate;
FPICOControlSetAutoConnectWIFIWithErrorCodeDelegate UPICOXRSystemAPI::ControlSetAutoConnectWIFIWithErrorCodeDelegate;
FPICOGetSwitchSystemFunctionStatusDelegate UPICOXRSystemAPI::GetSwitchSystemFunctionStatusDelegate;
FPICOCastInitDelegate UPICOXRSystemAPI::PICOCastInitDelegate;
FPICOSetControllerPairTimeDelegate UPICOXRSystemAPI::SetControllerPairTimeDelegate;
FPICOGetControllerPairTimeDelegate UPICOXRSystemAPI::GetControllerPairTimeDelegate;
FPICOSetSystemCountryCodeDelegate UPICOXRSystemAPI::SetSystemCountryCodeDelegate;
FPICOSetIPDDelegate UPICOXRSystemAPI::SetIPDDelegate;
FPICOSetArUcoMarkerDelegate UPICOXRSystemAPI::SetArUcoMarkerDelegate;
FPICOSetArUcoMarkerFDelegate UPICOXRSystemAPI::SetArUcoMarkerFDelegate;
FPICOFileCopyDelegate UPICOXRSystemAPI::FileCopyDelegate;
FPICOIsMapInEffectDelegate UPICOXRSystemAPI::IsMapInEffectDelegate;
FPICOImportMapByPathDelegate UPICOXRSystemAPI::ImportMapByPathDelegate;
UObject* UPICOXRSystemAPI::MarkerWorldContext = nullptr;
UObject* UPICOXRSystemAPI::MarkerFWorldContext = nullptr;
// Sets default values for this component's properties
UPICOXRSystemAPI::UPICOXRSystemAPI()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UPICOXRSystemAPI::BeginPlay()
{
	Super::BeginPlay();
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "BindSystemAPIService", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "UnBindSystemAPIService", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

// Called every frame
void UPICOXRSystemAPI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

int32 UPICOXRSystemAPI::PXR_SetControllerKeyState(EControllerKeyEnum ControllerKeyEnum, ESwitchEnum SwitchEnum, int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetControllerKeyState", "(III)I", false);
		Result = (int32)FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)ControllerKeyEnum, (int)SwitchEnum, Ext);
	}
#endif
	return Result;
}

FString UPICOXRSystemAPI::PXR_GetDeviceInfo(ESystemInfoEnum InfoEnum)
{
	FString Result = FString("");
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetDeviceInfo", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, static_cast<int32>(InfoEnum));
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return  Result;
}

void UPICOXRSystemAPI::PXR_SetDeviceAction(EDeviceControlEnum DeviceControlEnum, FPICOSetDeviceActionDelegate SetDeviceActionDelegate)
{
	SetDeviceActionDelegates.Add(DeviceControlEnum, SetDeviceActionDelegate);
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetDeviceAction", "(I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, static_cast<int32>(DeviceControlEnum));
	}
#endif
}

void UPICOXRSystemAPI::PXR_AppManager(EPackageControlEnum PackageControlEnum, FString Path, int32 Ext, FPICOAppManagerDelegate AppManagerDelegate)
{
	FAppManagerStruct AppManagerStruct;
	AppManagerStruct.PackageControl = PackageControlEnum;
	AppManagerStruct.AppPath = Path;
	AppManagerDelegates.Add(AppManagerStruct, AppManagerDelegate);
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jstring jstring_Path = Env->NewStringUTF(TCHAR_TO_UTF8(*Path));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "APPManger", "(ILjava/lang/String;I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, static_cast<int32>(PackageControlEnum), jstring_Path, Ext);
		Env->DeleteLocalRef(jstring_Path);
	}
#endif
}

void UPICOXRSystemAPI::PXR_SetAutoConnectWifi(FString WifiName, FString WifiPSD, int32 Ext, FPICOSetAutoConnectWifiDelegate InSetAutoConnectWifiDelegate)
{
	SetAutoConnectWifiDelegate = InSetAutoConnectWifiDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jstring jstring_WifiName = Env->NewStringUTF(TCHAR_TO_UTF8(*WifiName));
		jstring jstring_WifiPSD = Env->NewStringUTF(TCHAR_TO_UTF8(*WifiPSD));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetAutoConnectWIFI", "(Ljava/lang/String;Ljava/lang/String;I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, jstring_WifiName, jstring_WifiPSD, Ext);
		Env->DeleteLocalRef(jstring_WifiName);
		Env->DeleteLocalRef(jstring_WifiPSD);
	}
#endif
}

void UPICOXRSystemAPI::PXR_ClearAutoConnectWifi(FPICOClearAutoConnectWifiDelegate InClearAutoConnectWifiDelegate)
{
	ClearAutoConnectWifiDelegate = InClearAutoConnectWifiDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ClearAutoConnectWIFI", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_SetHomeKey(EHomeEventEnum EventEnum, EHomeFunctionEnum FunctionEnum, FPICOSetHomeKeyDelegate SetHomeKeyDelegate)
{
	SetHomeKeyDelegates.Add(EventEnum, SetHomeKeyDelegate);
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetHomeKey", "(II)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, static_cast<int32>(EventEnum), static_cast<int32>(FunctionEnum));
	}
#endif
}

void UPICOXRSystemAPI::PXR_SetHomeKeyAll(EHomeEventEnum EventEnum, EHomeFunctionEnum FunctionEnum, int32 TimeSetup, FString Package, FString ClassName, FPICOSetHomeKeyAllDelegate SetHomeKeyAllDelegate)
{
	SetHomeKeyAllDelegates.Add(EventEnum, SetHomeKeyAllDelegate);
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jstring jstring_Package = Env->NewStringUTF(TCHAR_TO_UTF8(*Package));
		jstring jstring_ClassName = Env->NewStringUTF(TCHAR_TO_UTF8(*ClassName));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetHomeKeyAll", "(IIILjava/lang/String;Ljava/lang/String;)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, static_cast<int32>(EventEnum), static_cast<int32>(FunctionEnum), TimeSetup, jstring_Package, jstring_ClassName);
		Env->DeleteLocalRef(jstring_Package);
		Env->DeleteLocalRef(jstring_ClassName);
	}
#endif
}

void UPICOXRSystemAPI::PXR_DisablePowerKey(bool bIsSingleTap, bool bEnable, FPICODisablePowerKeyDelegate InDisablePowerKeyDelegate)
{

	DisablePowerKeyDelegates.Add(bIsSingleTap, InDisablePowerKeyDelegate);
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "DisablePowerKey", "(ZZ)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, bIsSingleTap, bEnable);
	}
#endif
}

bool UPICOXRSystemAPI::PXR_SetSystemAutoSleepTime(ESleepDelayTimeEnum TimeEnum)
{
	int Result = 1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetSystemAutoSleepTime", "(I)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, static_cast<int32>(TimeEnum));
	}
#endif
	return Result == 0;
}

void UPICOXRSystemAPI::PXR_SetScreenOffDelay(EScreenOffDelayTimeEnum TimeEnum, FPICOSetScreenOffDelayDelegate InSetScreenOffDelayDelegate)
{
	SetScreenOffDelayDelegate = InSetScreenOffDelayDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetScreenOffDelay", "(I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, static_cast<int32>(TimeEnum));
	}
#endif
}

FPICOEnterpriseIntDelegate UPICOXRSystemAPI::SwitchSystemFunctionDelegate;
void UPICOXRSystemAPI::PXR_SwitchSystemFunction(ESystemFunctionSwitchEnum SystemFunction, ESwitchEnum SwitchEnum, int32 Ext, FPICOEnterpriseIntDelegate Delegate)
{
	SwitchSystemFunctionDelegate = Delegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SwitchSystemFunction", "(III)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, static_cast<int32>(SystemFunction), static_cast<int32>(SwitchEnum), Ext);
	}
#endif
}

#if PLATFORM_ANDROID
extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSwitchSystemFunctionCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::SwitchSystemFunctionDelegate.ExecuteIfBound(Result);
}
#endif

void UPICOXRSystemAPI::PXR_SetUsbConfigurationOption(EUSBConfigModeEnum UsbConfigModeEnum, int32 Ext)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetUsbConfigurationOption", "(II)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, static_cast<int32>(UsbConfigModeEnum), Ext);
	}
#endif
}

void UPICOXRSystemAPI::PXR_ScreenOn()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ScreenOn", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_ScreenOff()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ScreenOff", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_AcquireWakeLock()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AcquireWakeLock", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_ReleaseWakeLock()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ReleaseWakeLock", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_WriteConfigFileToDataLocal(FString Path, FString Content, FPICOWriteConfigFileToDataLocalDelegate InWriteConfigDelegate)
{
	WriteConfigDelegate = InWriteConfigDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		jstring jstring_Path = Env->NewStringUTF(TCHAR_TO_UTF8(*Path));
		jstring jstring_Content = Env->NewStringUTF(TCHAR_TO_UTF8(*Content));
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "WriteConfigFileToDataLocal", "(Ljava/lang/String;Ljava/lang/String;)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, jstring_Path, jstring_Content);
		Env->DeleteLocalRef(jstring_Path);
		Env->DeleteLocalRef(jstring_Content);
	}
#endif
}

void UPICOXRSystemAPI::PXR_ResetAllKeyToDefault(FPICOResetAllKeyToDefaultDelegate InResetAllKeyDelegate)
{
	ResetAllKeyDelegate = InResetAllKeyDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ResetAllKeyToDefault", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_EnableEnterKey()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "EnableEnterKey", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_DisableEnterKey()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "DisableEnterKey", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_EnableVolumeKey()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "EnableVolumeKey", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_DisableVolumeKey()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "DisableVolumeKey", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_EnableBackKey()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "EnableBackKey", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_DisableBackKey()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "DisableBackKey", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

int32 UPICOXRSystemAPI::PXR_GetCurrentBrightness()
{
	int32 currentBrightness = -1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetCurrentBrightness", "()I", false);
		currentBrightness = (int32)FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
	return currentBrightness;
}

bool UPICOXRSystemAPI::PXR_SetScreenBrightness(int Brightness, int Ext)
{
	int Result = 1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetScreenBrightness", "(II)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Brightness, Ext);
	}
#endif
	return Result == 0;
}

int32 UPICOXRSystemAPI::PXR_GetCurrentVolume()
{
	int32 currentVolume = -1;
#if PLATFORM_ANDROID

	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetCurrentVolume", "()I", false);
		currentVolume = (int32)FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
	return currentVolume;
}

int32 UPICOXRSystemAPI::PXR_GetMaxVolume()
{
	int32 maxVolume = -1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetMaxVolumeNumber", "()I", false);
		maxVolume = (int32)FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
	return maxVolume;
}

void UPICOXRSystemAPI::PXR_SetVolume(int32 Volume)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetVolume", "(I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, Volume);

	}
#endif
}

void UPICOXRSystemAPI::PXR_VolumeUp()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "VolumeUp", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif	
}

void UPICOXRSystemAPI::PXR_VolumeDown()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "VolumeDown", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}


FString UPICOXRSystemAPI::PXR_GetDeviceSN()
{
	ESystemInfoEnum systemInfo = ESystemInfoEnum::EQUIPMENT_SN;
	FString result = FString("");
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetDeviceInfo", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, static_cast<int32>(systemInfo));
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return result;
}

void UPICOXRSystemAPI::PXR_FreezeScreen(bool freeze)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "FreezeScreen", "(Z)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, freeze);
	}
#endif
}

void UPICOXRSystemAPI::PXR_KillAppsByPidOrPackageName(TArray<int> pids, TArray<FString> packageNames, int ext)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		int num = pids.Num();
		jintArray jPids = Env->NewIntArray(num);
		jint* jPidsArray = Env->GetIntArrayElements(jPids, 0);
		for (int32 i = 0; i < num; i++)
		{
			jPidsArray[i] = pids[i];
		}
		auto PackageNamesArray = NewScopedJavaObject(Env, (jobjectArray)Env->NewObjectArray(packageNames.Num(), FJavaWrapper::JavaStringClass, nullptr));
		for (uint32 Param = 0; Param < packageNames.Num(); Param++)
		{
			auto StringValue = FJavaHelper::ToJavaString(Env, packageNames[Param]);
			Env->SetObjectArrayElement(*PackageNamesArray, Param, *StringValue);
		}
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "KillAppsByPidOrPackageName", "([I[Ljava/lang/String;I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, jPids, *PackageNamesArray, ext);
	}
#endif
}

void UPICOXRSystemAPI::PXR_KillBackgroundAppsWithWhiteList(TArray<FString> packageNames, int ext)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto PackageNamesArray = NewScopedJavaObject(Env, (jobjectArray)Env->NewObjectArray(packageNames.Num(), FJavaWrapper::JavaStringClass, nullptr));
		for (uint32 Param = 0; Param < packageNames.Num(); Param++)
		{
			auto StringValue = FJavaHelper::ToJavaString(Env, packageNames[Param]);
			Env->SetObjectArrayElement(*PackageNamesArray, Param, *StringValue);
		}
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "KillBackgroundAppsWithWhiteList", "([Ljava/lang/String;I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, *PackageNamesArray, ext);
	}
#endif
}

void UPICOXRSystemAPI::PXR_OpenMiracast()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "OpenMiracast", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

bool UPICOXRSystemAPI::PXR_IsMiracastOn()
{
	bool result = false;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "IsMiracastOn", "()Z", false);
		result = (bool)FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
	return result;
}

void UPICOXRSystemAPI::PXR_CloseMiracast()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "CloseMiracast", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_StartScan()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "StartScan", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_StopScan()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "StopScan", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_ConnectWifiDisplay(const FWifiDisplayModel& Model)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto     deviceAddress = FJavaHelper::ToJavaString(Env, Model.deviceAddress);
		auto	 deviceName = FJavaHelper::ToJavaString(Env, Model.deviceName);
		auto     isAvailable = Model.isAvailable;
		auto     canConnect = Model.canConnect;
		auto     isRemembered = Model.isRemembered;
		auto	 statusCode = Model.statusCode;
		auto	 status = FJavaHelper::ToJavaString(Env, Model.status);
		auto	 description = FJavaHelper::ToJavaString(Env, Model.description);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ConnectWifiDisplay",
			"(Ljava/lang/String;Ljava/lang/String;ZZZILjava/lang/String;Ljava/lang/String;)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, *deviceAddress, *deviceName, isAvailable, canConnect, isRemembered, statusCode, *status, *description);
	}
#endif
}

void UPICOXRSystemAPI::PXR_DisConnectWifiDisplay()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "DisConnectWifiDisplay", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_ForgetWifiDisplay(FString Address)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto jaddress = FJavaHelper::ToJavaString(Env, Address);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ForgetWifiDisplay", "(Ljava/lang/String;)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, *jaddress);
	}
#endif
}

void UPICOXRSystemAPI::PXR_RenameWifiDisplay(FString Address, FString NewName)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "RenameWifiDisplay", "(Ljava/lang/String;Ljava/lang/String;)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method
			, *FJavaHelper::ToJavaString(Env, Address)
			, *FJavaHelper::ToJavaString(Env, NewName));
	}
#endif
}

void UPICOXRSystemAPI::PXR_UpdateWifiDisplays()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "UpdateWifiDisplays", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_GetConnectedWD(FWifiDisplayModel& Model)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetConnectedWD", "()Ljava/lang/String;", false);
		FString result = "";
		result = FJavaHelper::FStringFromLocalRef(Env, (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method));
		if (!result.IsEmpty())
		{
			TArray<FString> substring;
			result.ParseIntoArray(substring, TEXT("="), true);
			substring.RemoveAt(0);
			for (int i = 0; i < substring.Num(); i++)
			{
				if (substring[i].Contains(TEXT("'")))
				{
					substring[i].Split(TEXT("'"), nullptr, &substring[i]);
					substring[i].Split(TEXT("'"), &substring[i], nullptr);
				}
				else
				{
					substring[i].Split(TEXT(","), &substring[i], nullptr);
				}
			}
			Model.deviceAddress = substring[0];
			Model.deviceName = substring[1];
			Model.isAvailable = substring[2] == "true" ? true : false;
			Model.canConnect = substring[3] == "true" ? true : false;
			Model.isRemembered = substring[4] == "true" ? true : false;
			Model.statusCode = FCString::Atoi(*substring[5]);
			Model.status = substring[6];
			Model.description = substring[7];
		}
	}
#endif
}

void UPICOXRSystemAPI::PXR_SetWDJson(FPICOSetWDJsonDelegate InSetWDJsonDelegate)
{
	SetWDJsonDelegate = InSetWDJsonDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetWDJson", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_SetWDModels(FPICOSetWDModelsDelegate InSetWDModelsDelegate)
{
	SetWDModelsDelegate = InSetWDModelsDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetWDModels", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}


void UPICOXRSystemAPI::PXR_SwitchLargeSpaceScene(FPICOEnableLargeSpaceDelegate InEnableLargeSpaceDelegate, bool open, int ext)
{
	EnableLargeSpaceDelegate = InEnableLargeSpaceDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SwitchLargeSpaceScene", "(ZI)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, open, ext);
	}
#endif
}

void UPICOXRSystemAPI::PXR_GetSwitchLargeSpaceStatus(FPICOSwitchLargeSpaceStatusDelegate InSwitchLargeSpaceStatusDelegate, int ext)
{
	SwitchLargeSpaceStatusDelegate = InSwitchLargeSpaceStatusDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetSwitchLargeSpaceStatus", "(I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, ext);
	}
#endif
}

bool UPICOXRSystemAPI::PXR_SaveLargeSpaceMaps(int ext)
{
	bool result = false;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SaveLargeSpaceMaps", "(I)Z", false);
		result = (bool)FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, Method, ext);
	}
#endif
	return result;
}

void UPICOXRSystemAPI::PXR_ExportMaps(FPICOExportMapsDelegate InExportMapsDelegate, int ext)
{
	ExportMapsDelegate = InExportMapsDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ExportMaps", "(I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, ext);
	}
#endif
}

void UPICOXRSystemAPI::PXR_ImportMaps(FPICOImportMapsDelegate InImportMapsDelegate, int ext)
{
	ImportMapsDelegate = InImportMapsDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ImportMaps", "(I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, ext);
	}
#endif
}

void UPICOXRSystemAPI::PXR_GetCpuUsages(TArray<float>& OutData)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetCpuUsages", "()[F", false);
		auto FloatValuesArray = NewScopedJavaObject(Env, (jfloatArray)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method));
		jfloat* FloatValues = Env->GetFloatArrayElements(*FloatValuesArray, 0);
		jsize NumProducts = Env->GetArrayLength(*FloatValuesArray);
		OutData.Empty();
		for (int i = 0; i < NumProducts; i++)
		{
			OutData.Add(FloatValues[i]);
		}
	}
#endif
}

void UPICOXRSystemAPI::PXR_GetDeviceTemperatures(int inType, int inSource, TArray<float>& OutData)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetDeviceTemperatures", "(II)[F", false);
		auto FloatValuesArray = NewScopedJavaObject(Env, (jfloatArray)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, inType, inSource));
		jfloat* FloatValues = Env->GetFloatArrayElements(*FloatValuesArray, 0);
		jsize NumProducts = Env->GetArrayLength(*FloatValuesArray);
		OutData.Empty();
		for (int i = 0; i < NumProducts; i++)
		{
			OutData.Add(FloatValues[i]);
		}
	}
#endif
}

void UPICOXRSystemAPI::PXR_Capture()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "Capture", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_Record()
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "Record", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_ControlSetAutoConnectWIFIWithErrorCode(FPICOControlSetAutoConnectWIFIWithErrorCodeDelegate InControlSetAutoConnectWIFIWithErrorCodeDelegate, FString ssid, FString pwd, int ext)
{
	ControlSetAutoConnectWIFIWithErrorCodeDelegate = InControlSetAutoConnectWIFIWithErrorCodeDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto jSsid = FJavaHelper::ToJavaString(Env, ssid);
		auto jPwd = FJavaHelper::ToJavaString(Env, pwd);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ControlSetAutoConnectWIFIWithErrorCode", "(Ljava/lang/String;Ljava/lang/String;I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, *jSsid, *jPwd, ext);
	}
#endif
}

void UPICOXRSystemAPI::PXR_AppKeepAlive(FString appPackageName, bool keepAlive, int ext)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto jAppPackageName = FJavaHelper::ToJavaString(Env, appPackageName);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AppKeepAlive", "(Ljava/lang/String;ZI)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, *jAppPackageName, keepAlive, ext);
	}
#endif
}

void UPICOXRSystemAPI::PXR_TimingStartup(int year, int month, int day, int hour, int minute, bool open)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "TimingStartup", "(IIIIIZ)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, year, month, day, hour, minute, open);
	}
#endif
}

void UPICOXRSystemAPI::PXR_TimingShutdown(int year, int month, int day, int hour, int minute, bool open)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "TimingShutdown", "(IIIIIZ)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, year, month, day, hour, minute, open);
	}
#endif
}

void UPICOXRSystemAPI::PXR_StartVrSettingsItem(int vrSettingsEnum, bool hideOtherItem, int ext)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "StartVrSettingsItem", "(IZI)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, vrSettingsEnum, hideOtherItem, ext);
	}
#endif
}

void UPICOXRSystemAPI::PXR_SwitchVolumeToHomeAndEnter(ESwitchEnum switchEnum, int ext)
{
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SwitchVolumeToHomeAndEnter", "(II)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)switchEnum, ext);
	}
#endif
}

ESwitchEnum UPICOXRSystemAPI::PXR_IsVolumeChangeToHomeAndEnter()
{
	int i = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "IsVolumeChangeToHomeAndEnter", "()I", false);
		i = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
	return ESwitchEnum(i);
}

EInstallOTAReturnCode UPICOXRSystemAPI::PXR_InstallOTAPackage(const FString& OtaPackagePath, int32 Ext)
{
	EInstallOTAReturnCode Result = EInstallOTAReturnCode::SUCCESS;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto jOtaPackagePath = FJavaHelper::ToJavaString(Env, OtaPackagePath);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "InstallOTAPackage", "(Ljava/lang/String;I)I", false);
		int32 ResultCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, *jOtaPackagePath, Ext);
		Result = (EInstallOTAReturnCode)ResultCode;
	}
#endif
	return Result;
}

FString UPICOXRSystemAPI::PXR_GetAutoConnectWiFiConfig(int32 Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetAutoConnectWiFiConfig", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Result;
}

FString UPICOXRSystemAPI::PXR_GetTimingStartupStatus(int32 Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetTimingStartupStatus", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Result;
}

FString UPICOXRSystemAPI::PXR_GetTimingShutdownStatus(int32 Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetTimingShutdownStatus", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Result;
}

EKeyState UPICOXRSystemAPI::PXR_GetControllerKeyState(EControllerKeyEnum ControllerKeyEnum, int32 Ext)
{
	EKeyState State = EKeyState::DISABLE;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetControllerKeyState", "(II)I", false);
		int32 ReturnCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)ControllerKeyEnum, Ext);
		State = (EKeyState)ReturnCode;
	}
#endif
	return State;
}

ESwitchEnum UPICOXRSystemAPI::PXR_GetPowerOffWithUSBCable(int32 Ext)
{
	ESwitchEnum Result = ESwitchEnum::S_OFF;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetPowerOffWithUSBCable", "(I)I", false);
		int32 ReturnCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		Result = (ESwitchEnum)ReturnCode;
	}
#endif
	return Result;
}

EScreenOffDelayTimeEnum UPICOXRSystemAPI::PXR_GetScreenOffDelay(int32 Ext)
{
	EScreenOffDelayTimeEnum Result = EScreenOffDelayTimeEnum::NEVER;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetScreenOffDelay", "(I)I", false);
		int32 ReturnCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		Result = (EScreenOffDelayTimeEnum)ReturnCode;
	}
#endif
	return Result;
}

ESleepDelayTimeEnum UPICOXRSystemAPI::PXR_GetSleepDelay(int32 Ext)
{
	ESleepDelayTimeEnum Result = ESleepDelayTimeEnum::NEVER;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetSleepDelay", "(I)I", false);
		int32 ReturnCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		Result = (ESleepDelayTimeEnum)ReturnCode;
	}
#endif
	return Result;
}

FString UPICOXRSystemAPI::PXR_GetPowerKeyStatus(int32 Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetPowerKeyStatus", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Result;
}

EKeyState UPICOXRSystemAPI::PXR_GetEnterKeyStatus(int32 Ext)
{
	EKeyState State = EKeyState::DISABLE;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetEnterKeyStatus", "(I)I", false);
		int32 ReturnCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		State = (EKeyState)ReturnCode;
	}
#endif
	return State;
}

EKeyState UPICOXRSystemAPI::PXR_GetVolumeKeyStatus(int32 Ext)
{
	EKeyState State = EKeyState::DISABLE;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetVolumeKeyStatus", "(I)I", false);
		int32 ReturnCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		State = (EKeyState)ReturnCode;
	}
#endif
	return State;
}

EKeyState UPICOXRSystemAPI::PXR_GetBackKeyStatus(int32 Ext)
{
	EKeyState State = EKeyState::DISABLE;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetBackKeyStatus", "(I)I", false);
		int32 ReturnCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		State = (EKeyState)ReturnCode;
	}
#endif
	return State;
}

FString UPICOXRSystemAPI::PXR_GetHomeKeyStatus(EHomeEventEnum HomeEventEnum, int32 Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetHomeKeyStatus", "(II)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)HomeEventEnum, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Result;
}

FString UPICOXRSystemAPI::PXR_GetUsbConfigurationOption(int32 Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetUsbConfigurationOption", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Result;
}

FString UPICOXRSystemAPI::PXR_GetCurrentLauncher(int32 Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetCurrentLauncher", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Result;
}

void UPICOXRSystemAPI::PXR_GetSwitchSystemFunctionStatus(FPICOGetSwitchSystemFunctionStatusDelegate InGetSwitchSystemFunctionStatusDelegate, ESystemFunctionSwitchEnum SystemFunctionSwitchEnum, int32 Ext)
{
	GetSwitchSystemFunctionStatusDelegate = InGetSwitchSystemFunctionStatusDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetSwitchSystemFunctionStatus", "(II)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)SystemFunctionSwitchEnum, Ext);
	}
#endif
}

ECastReturnCode UPICOXRSystemAPI::PXR_PICOCastInit(FPICOCastInitDelegate InPICOCastInitDelegate, int32 Ext)
{
	PICOCastInitDelegate = InPICOCastInitDelegate;
	ECastReturnCode Result = ECastReturnCode::FAIL;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "PICOCastInit", "(I)I", false);
		int32 ReturnCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		Result = (ECastReturnCode)ReturnCode;
	}
#endif
	return Result;
}

ECastReturnCode UPICOXRSystemAPI::PXR_PICOCastSetShowAuthorization(EPICOCastAuthorization Authorization, int32 Ext)
{
	ECastReturnCode Result = ECastReturnCode::SUCCESS;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "PICOCastSetShowAuthorization", "(II)I", false);
		int32 ReturnCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)Authorization, Ext);
		Result = (ECastReturnCode)ReturnCode;
	}
#endif
	return Result;
}

EPICOCastAuthorization UPICOXRSystemAPI::PXR_PICOCastGetShowAuthorization(int32 Ext)
{
	EPICOCastAuthorization Result = EPICOCastAuthorization::ASK_EVERY_TIME;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "PICOCastGetShowAuthorization", "(I)I", false);
		int32 ResultCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		Result = (EPICOCastAuthorization)ResultCode;
	}
#endif
	return Result;
}

FString UPICOXRSystemAPI::PXR_PICOCastGetUrl(EPICOCastUrlTypeEnum UrlType, int32 Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "PICOCastGetUrl", "(II)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)UrlType, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Result;
}

ECastReturnCode UPICOXRSystemAPI::PXR_PICOCastStopCast(int32 Ext)
{
	ECastReturnCode Result = ECastReturnCode::SUCCESS;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "PICOCastStopCast", "(I)I", false);
		int32 ResultCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		Result = (ECastReturnCode)ResultCode;
	}
#endif
	return Result;
}

ECastReturnCode UPICOXRSystemAPI::PXR_PICOCastSetOption(EPICOCastOptionOrStatusEnum OptionEnum, EPICOCastOptionValueEnum ValueEnum, int32 Ext)
{
	ECastReturnCode Result = ECastReturnCode::SUCCESS;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "PICOCastSetOption", "(III)I", false);
		int32 ResultCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)OptionEnum, (int)ValueEnum, Ext);
		Result = (ECastReturnCode)ResultCode;
	}
#endif
	return Result;
}

EPICOCastOptionValueEnum UPICOXRSystemAPI::PXR_PICOCastGetOptionOrStatus(EPICOCastOptionOrStatusEnum OptionEnum, int32 Ext)
{
	EPICOCastOptionValueEnum Result = EPICOCastOptionValueEnum::OPTION_VALUE_RESOLUTION_AUTO;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "PICOCastGetOptionOrStatus", "(II)I", false);
		int32 ResultCode = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)OptionEnum, Ext);
		Result = (EPICOCastOptionValueEnum)ResultCode;
	}
#endif
	return Result;
}

void UPICOXRSystemAPI::PXR_SetControllerPairTime(FPICOSetControllerPairTimeDelegate InSetControllerPairTimeDelegate, EControllerPairTimeEnum TimeEnum, int32 Ext)
{
	SetControllerPairTimeDelegate = InSetControllerPairTimeDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetControllerPairTime", "(II)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)TimeEnum, Ext);
	}
#endif
}

void UPICOXRSystemAPI::PXR_GetControllerPairTime(FPICOGetControllerPairTimeDelegate InGetControllerPairTimeDelegate, int32 Ext)
{
	GetControllerPairTimeDelegate = InGetControllerPairTimeDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetControllerPairTime", "(I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
	}
#endif
}

int32 UPICOXRSystemAPI::PXR_SetSystemLanguage(const FString& SystemLanguage, int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto jstring_SystemLanguage = FJavaHelper::ToJavaString(Env, SystemLanguage);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetSystemLanguage", "(Ljava/lang/String;I)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, *jstring_SystemLanguage, Ext);
	}
#endif
	return Result;
}

FString UPICOXRSystemAPI::PXR_GetSystemLanguage(int32 Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetSystemLanguage", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Result;
}

int32 UPICOXRSystemAPI::PXR_ConfigWifi(const FString& Ssid, const FString& Pwd, int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto jstring_Ssid = FJavaHelper::ToJavaString(Env, Ssid);
		auto jstring_Pwd = FJavaHelper::ToJavaString(Env, Pwd);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ConfigWifi", "(Ljava/lang/String;Ljava/lang/String;I)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, *jstring_Ssid, *jstring_Pwd, Ext);
	}
#endif
	return Result;
}

TArray<FString> UPICOXRSystemAPI::PXR_GetConfiguredWifi(int32 Ext)
{
	TArray<FString> Results;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetConfiguredWifi", "(I)[Ljava/lang/String;", false);
		jobjectArray JavaStringArray = (jobjectArray)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		jsize NumProducts = Env->GetArrayLength(JavaStringArray);
		for (int32 Index = 0; Index < NumProducts; ++Index)
		{
			jstring Result = (jstring)(Env->GetObjectArrayElement(JavaStringArray, Index));
			const char* JavaChars = Env->GetStringUTFChars(Result, 0);
			Results.Add(FString(UTF8_TO_TCHAR(JavaChars)));
			Env->ReleaseStringUTFChars(Result, JavaChars);
			Env->DeleteLocalRef(Result);
		}
	}
#endif
	return Results;
}

int32 UPICOXRSystemAPI::PXR_SetSystemCountryCode(FPICOSetSystemCountryCodeDelegate InSetSystemCountryCodeDelegate, const FString& CountryCode, int32 Ext)
{
	SetSystemCountryCodeDelegate = InSetSystemCountryCodeDelegate;
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto jstring_CountryCode = FJavaHelper::ToJavaString(Env, CountryCode);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetSystemCountryCode", "(Ljava/lang/String;I)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, *jstring_CountryCode, Ext);
	}
#endif
	return Result;
}

FString UPICOXRSystemAPI::PXR_GetSystemCountryCode(int32 Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetSystemCountryCode", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Result;
}

int32 UPICOXRSystemAPI::PXR_SetSkipInitSettingPage(int32 Flag, int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetSkipInitSettingPage", "(II)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Flag, Ext);
	}
#endif
	return Result;
}

int32 UPICOXRSystemAPI::PXR_GetSkipInitSettingPage(int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetSkipInitSettingPage", "(I)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
	}
#endif
	return Result;
}

int32 UPICOXRSystemAPI::PXR_IsInitSettingComplete(int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "IsInitSettingComplete", "(I)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
	}
#endif
	return Result;
}

int32 UPICOXRSystemAPI::PXR_StartActivity(const FString& PackageName, const FString& ClassName, const FString& Action, const FString& Extra, const TArray<FString>& Categories, const TArray<int32>& Flags, int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "StartActivity", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;[Ljava/lang/String;[II)I", false);

		auto jstring_PackageName = FJavaHelper::ToJavaString(Env, PackageName);
		auto jstring_ClassName = FJavaHelper::ToJavaString(Env, ClassName);
		auto jstring_Action = FJavaHelper::ToJavaString(Env, Action);
		auto jstring_Extra = FJavaHelper::ToJavaString(Env, Extra);
		auto jstring_Default = FJavaHelper::ToJavaString(Env, TEXT(""));

		int32 CategorySize = Categories.Num();
		jobjectArray jarray_Categories = Env->NewObjectArray(CategorySize, Env->FindClass("java/lang/String"), *jstring_Default);
		for (int32 Index = 0; Index < CategorySize; ++Index)
		{
			auto jstring_Category = FJavaHelper::ToJavaString(Env, Categories[Index]);
			Env->SetObjectArrayElement(jarray_Categories, Index, *jstring_Category);
		}

		int32 FlagSize = Flags.Num();
		jintArray jarray_Flags = Env->NewIntArray(FlagSize);
		jint jint_Flags[FlagSize];
		for (int32 Index = 0; Index < FlagSize; ++Index)
		{
			jint_Flags[Index] = Flags[Index];
		}
		Env->SetIntArrayRegion(jarray_Flags, 0, FlagSize, jint_Flags);

		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, *jstring_PackageName, *jstring_ClassName, *jstring_Action, *jstring_Extra, jarray_Categories, jarray_Flags, Ext);

		Env->DeleteLocalRef(jarray_Categories);
		Env->DeleteLocalRef(jarray_Flags);
	}
#endif
	return Result;
}

int32 UPICOXRSystemAPI::PXR_CustomizeAppLibrary(const TArray<FString>& PackageNames, ESwitchEnum SwitchEnum, int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "CustomizeAppLibrary", "([Ljava/lang/String;II)I", false);

		auto jstring_Default = FJavaHelper::ToJavaString(Env, TEXT(""));
		int32 PackageNamesSize = PackageNames.Num();
		jobjectArray jarray_PackageNames = Env->NewObjectArray(PackageNamesSize, Env->FindClass("java/lang/String"), *jstring_Default);
		for (int32 Index = 0; Index < PackageNamesSize; ++Index)
		{
			auto jstring_PackageName = FJavaHelper::ToJavaString(Env, PackageNames[Index]);
			Env->SetObjectArrayElement(jarray_PackageNames, Index, *jstring_PackageName);
		}
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, jarray_PackageNames, (int)SwitchEnum, Ext);
		Env->DeleteLocalRef(jarray_PackageNames);
	}
#endif
	return Result;
}

TArray<int32> UPICOXRSystemAPI::PXR_GetControllerBattery(int32 Ext)
{
	TArray<int32> Results;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetControllerBattery", "(I)[I", false);
		jintArray JavaIntArray = (jintArray)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		if (JavaIntArray != nullptr)
		{
			jint* JavaInts = Env->GetIntArrayElements(JavaIntArray, 0);
			if (JavaInts)
			{
				jsize IntsLen = Env->GetArrayLength(JavaIntArray);
				Results.Empty(IntsLen);
				for (int32 Idx = 0; Idx < IntsLen; Idx++)
				{
					Results.Add(JavaInts[Idx]);
				}
				Env->ReleaseIntArrayElements(JavaIntArray, JavaInts, 0);
			}
			Env->DeleteLocalRef(JavaIntArray);
		}
	}
#endif
	return Results;
}

int32 UPICOXRSystemAPI::PXR_GetControllerConnectState(int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetControllerConnectState", "(I)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
	}
#endif
	return Result;
}

FString UPICOXRSystemAPI::PXR_GetAppLibraryHideList(int32 Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetAppLibraryHideList", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Result;
}

int32 UPICOXRSystemAPI::PXR_SetScreenCastAudioOutput(EScreencastAudioOutput ScreencastAudioOutput, int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetScreenCastAudioOutput", "(II)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)ScreencastAudioOutput, Ext);
	}
#endif
	return Result;
}

EScreencastAudioOutput UPICOXRSystemAPI::PXR_GetScreenCastAudioOutput(int32 Ext)
{
	EScreencastAudioOutput Result = EScreencastAudioOutput::AUDIO_SINK;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetScreenCastAudioOutput", "(I)I", false);
		Result = (EScreencastAudioOutput)FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
	}
#endif
	UE_LOG(PxrSystemAPI, Warning, TEXT("Result: %d"), (int)Result);
	return Result;
}

int32 UPICOXRSystemAPI::PXR_CustomizeSettingsTabStatus(ECustomizeSettingsTab CustomizeSettingsTab, ESwitchEnum SwitchEnum, int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "CustomizeSettingsTabStatus", "(III)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)CustomizeSettingsTab, (int)SwitchEnum, Ext);
	}
#endif
	return Result;
}

ESwitchEnum UPICOXRSystemAPI::PXR_GetCustomizeSettingsTabStatus(ECustomizeSettingsTab CustomizeSettingsTab, int32 Ext)
{
	ESwitchEnum Result = ESwitchEnum::S_ON;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetCustomizeSettingsTabStatus", "(II)I", false);
		Result = (ESwitchEnum)FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)CustomizeSettingsTab, Ext);
	}
#endif
	return Result;
}

void UPICOXRSystemAPI::PXR_SetIPD(FPICOSetIPDDelegate InSetIPDDelegate, float IPD)
{
	SetIPDDelegate = InSetIPDDelegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetIPD", "(F)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, IPD);
	}
#endif
}

int32 UPICOXRSystemAPI::PXR_SetPICOCastMediaFormat(const FCastMediaFormat& CastMediaFormat, int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetPICOCastMediaFormat", "(II)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, CastMediaFormat.Bitrate, Ext);
	}
#endif
	return Result;
}

void UPICOXRSystemAPI::PXR_SetArUcoMarker(UObject* InWorldContext, FPICOSetArUcoMarkerDelegate InSetArUcoMarkerDelegate)
{
	SetArUcoMarkerDelegate = InSetArUcoMarkerDelegate;
	MarkerWorldContext = InWorldContext;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetArUcoMarker", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

void UPICOXRSystemAPI::PXR_SetArUcoMarkerF(UObject* InWorldContext, FPICOSetArUcoMarkerFDelegate InSetArUcoMarkerFDelegate)
{
	SetArUcoMarkerFDelegate = InSetArUcoMarkerFDelegate;
	MarkerFWorldContext = InWorldContext;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetArUcoMarkerF", "()V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
}

bool UPICOXRSystemAPI::PXR_InitEnterpriseService(FString& Token)
{
	Token.Empty();
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetToken", "()Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Token = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
		}
	}
#endif
	return Token.Len() > 0;
}

int32 UPICOXRSystemAPI::PXR_GetHeadTrackingConfidence()
{
	return FInterfaceWrapper::GetInstance()->PXR_GetHeadTrackingConfidence();
}

bool UPICOXRSystemAPI::PXR_OpenVSTCamera()
{
	return FInterfaceWrapper::GetInstance()->PXR_OpenVSTCamera();
}

bool UPICOXRSystemAPI::PXR_CloseVSTCamera()
{
	return FInterfaceWrapper::GetInstance()->PXR_CloseVSTCamera();
}

void FromFrameItemExtToFrameData(FVSTCameraFrameData& FrameData, const frame_item_ext_t& FrameItemExt)
{
	FrameData.FrameItem.CameraId = FrameItemExt.frame.camera_id;
	FrameData.FrameItem.Width = FrameItemExt.frame.width;
	FrameData.FrameItem.Height = FrameItemExt.frame.height;
	FrameData.FrameItem.Format = FrameItemExt.frame.format;
	FrameData.FrameItem.ExposureDuration = FrameItemExt.frame.exposure_duration;
	FrameData.FrameItem.Timestamp = FrameItemExt.frame.timestamp;
	FrameData.FrameItem.QTimerTimestamp = FrameItemExt.frame.qtimer_timestamp;
	FrameData.FrameItem.FrameNumber = FrameItemExt.frame.framenumber;
	FrameData.FrameItem.DataSize = FrameItemExt.frame.datasize;
	FrameData.FrameItem.Data.SetNum(FrameData.FrameItem.DataSize);
	FMemory::Memcpy(FrameData.FrameItem.Data.GetData(), FrameItemExt.frame.data, sizeof(uint8) * FrameData.FrameItem.DataSize);

	FrameData.bIsRGB = FrameItemExt.is_rgb;
	for (int32 X = 0; X < 4; X++)
	{
		for (int32 Y = 0; Y < 4; Y++)
		{
			FrameData.RGBTSWMatrix.M[X][Y] = static_cast<float>(FrameItemExt.rgb_tsw_matrix[X * 4 + Y]);
		}
	}
	FrameData.bIsAntiDistortion = FrameItemExt.is_anti_distortion;

	FrameData.SixDofPose.Pose.Timestamp = FrameItemExt.six_dof_pose.pose.timestamp;
	FrameData.SixDofPose.Pose.Translation = FVector(FrameItemExt.six_dof_pose.pose.x, FrameItemExt.six_dof_pose.pose.y, FrameItemExt.six_dof_pose.pose.z);
	FrameData.SixDofPose.Pose.Orientation = FQuat(FrameItemExt.six_dof_pose.pose.rx, FrameItemExt.six_dof_pose.pose.ry, FrameItemExt.six_dof_pose.pose.rz, FrameItemExt.six_dof_pose.pose.rw);
	FrameData.SixDofPose.Pose.Type = FrameItemExt.six_dof_pose.pose.type;
	FrameData.SixDofPose.Pose.Confidence = FrameItemExt.six_dof_pose.pose.confidence;
	FrameData.SixDofPose.Pose.PoseErrorType.Empty();
	int64 SixDofPoseError = FrameItemExt.six_dof_pose.pose.error;
	for (uint8 Index = 0; Index <= (uint8)EPoseErrorType::UNKNOWN_ERROR; ++Index)
	{
		int64 BitValue = 1LL << Index;
		if (BitValue & SixDofPoseError)
		{
			FrameData.SixDofPose.Pose.PoseErrorType.Add((EPoseErrorType)Index);
		}
	}
	FrameData.SixDofPose.Pose.PlaneHeight = FrameItemExt.six_dof_pose.pose.plane_height;
	FrameData.SixDofPose.Pose.PlaneStatus = FrameItemExt.six_dof_pose.pose.plane_status;
	FrameData.SixDofPose.Pose.RelocationStatus = FrameItemExt.six_dof_pose.pose.relocation_status;
	FrameData.SixDofPose.Pose.Reserved.SetNum(24);
	FMemory::Memcpy(FrameData.SixDofPose.Pose.Reserved.GetData(), FrameItemExt.six_dof_pose.pose.reserved, sizeof(FrameItemExt.six_dof_pose.pose.reserved));

	FrameData.SixDofPose.RelocationPose.Timestamp = FrameItemExt.six_dof_pose.relocation_pose.timestamp;
	FrameData.SixDofPose.RelocationPose.Translation = FVector(FrameItemExt.six_dof_pose.relocation_pose.x, FrameItemExt.six_dof_pose.relocation_pose.y, FrameItemExt.six_dof_pose.relocation_pose.z);
	FrameData.SixDofPose.RelocationPose.Orientation = FQuat(FrameItemExt.six_dof_pose.relocation_pose.rx, FrameItemExt.six_dof_pose.relocation_pose.ry, FrameItemExt.six_dof_pose.relocation_pose.rz, FrameItemExt.six_dof_pose.relocation_pose.rw);
	FrameData.SixDofPose.RelocationPose.Type = FrameItemExt.six_dof_pose.relocation_pose.type;
	FrameData.SixDofPose.RelocationPose.Confidence = FrameItemExt.six_dof_pose.relocation_pose.confidence;
	FrameData.SixDofPose.RelocationPose.PoseErrorType.Empty();
	int64 SixDofRelocationPoseError = FrameItemExt.six_dof_pose.relocation_pose.error;
	for (uint8 Index = 0; Index <= (uint8)EPoseErrorType::UNKNOWN_ERROR; ++Index)
	{
		int64 BitValue = 1LL << Index;
		if (BitValue & SixDofRelocationPoseError)
		{
			FrameData.SixDofPose.RelocationPose.PoseErrorType.Add((EPoseErrorType)Index);
		}
	}
	FrameData.SixDofPose.RelocationPose.PlaneHeight = FrameItemExt.six_dof_pose.relocation_pose.plane_height;
	FrameData.SixDofPose.RelocationPose.PlaneStatus = FrameItemExt.six_dof_pose.relocation_pose.plane_status;
	FrameData.SixDofPose.RelocationPose.RelocationStatus = FrameItemExt.six_dof_pose.relocation_pose.relocation_status;
	FrameData.SixDofPose.RelocationPose.Reserved.SetNum(24);
	FMemory::Memcpy(FrameData.SixDofPose.RelocationPose.Reserved.GetData(), FrameItemExt.six_dof_pose.relocation_pose.reserved, sizeof(FrameItemExt.six_dof_pose.relocation_pose.reserved));

	FrameData.SixDofPose.LinearVelocity = FVector(FrameItemExt.six_dof_pose.vx, FrameItemExt.six_dof_pose.vy, FrameItemExt.six_dof_pose.vz);
	FrameData.SixDofPose.LinearAcceleration = FVector(FrameItemExt.six_dof_pose.ax, FrameItemExt.six_dof_pose.ay, FrameItemExt.six_dof_pose.az);
	FrameData.SixDofPose.AngularVelocity = FVector(FrameItemExt.six_dof_pose.wx, FrameItemExt.six_dof_pose.wy, FrameItemExt.six_dof_pose.wz);
	FrameData.SixDofPose.AngularAcceleration = FVector(FrameItemExt.six_dof_pose.w_ax, FrameItemExt.six_dof_pose.w_ay, FrameItemExt.six_dof_pose.w_az);

	FrameData.SixDofPose.Reserved.SetNum(48);
	FMemory::Memcpy(FrameData.SixDofPose.Reserved.GetData(), FrameItemExt.six_dof_pose.reserved, sizeof(FrameItemExt.six_dof_pose.reserved));

	FrameData.Reserved.SetNum(64);
	FMemory::Memcpy(FrameData.Reserved.GetData(), FrameItemExt.reserved, sizeof(FrameItemExt.reserved));
}

bool UPICOXRSystemAPI::PXR_AcquireVSTCameraFrame(FVSTCameraFrameData& FrameData)
{
	frame_item_ext_t FrameItemExt;
	if (!FInterfaceWrapper::GetInstance()->PXR_AcquireVSTCameraFrame(FrameItemExt))
	{
		return false;
	}

	FromFrameItemExtToFrameData(FrameData, FrameItemExt);
	return true;
}

bool UPICOXRSystemAPI::PXR_AcquireVSTCameraFrameAntiDistortion(const FString& Token, int32 Width, int32 Height, FVSTCameraFrameData& FrameData)
{
	frame_item_ext_t FrameItemExt;
	if (!FInterfaceWrapper::GetInstance()->PXR_AcquireVSTCameraFrameAntiDistortion(TCHAR_TO_ANSI(*Token), Width, Height, FrameItemExt))
	{
		return false;
	}

	FromFrameItemExtToFrameData(FrameData, FrameItemExt);
	return true;
}

bool UPICOXRSystemAPI::PXR_GetCameraParameters(const FString& Token, FRGBCameraParams& Params)
{
	rgb_camera_params RGBCameraParams;
	if (!FInterfaceWrapper::GetInstance()->PXR_GetCameraParameters(TCHAR_TO_ANSI(*Token), RGBCameraParams))
	{
		return false;
	}

	Params.InternalF = FVector2D(RGBCameraParams.fx, RGBCameraParams.fy);
	Params.InternalC = FVector2D(RGBCameraParams.cx, RGBCameraParams.cy);
	Params.ExternalTranslation = FVector(RGBCameraParams.x, RGBCameraParams.y, RGBCameraParams.z);
	Params.ExternalOrientation = FQuat(RGBCameraParams.rx, RGBCameraParams.ry, RGBCameraParams.rz, RGBCameraParams.rw);
	return true;
}

int32 UPICOXRSystemAPI::PXR_GotoSeeThroughFloorSetting(int32 Ext)
{
	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GotoSeeThroughFloorSetting", "(I)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
	}
#endif
	return Result;
}

int32 UPICOXRSystemAPI::PXR_FileCopy(const FString& SourcePath, const FString& TargetPath, FPICOFileCopyDelegate InFileCopyDelegate)
{
	FileCopyDelegate = InFileCopyDelegate;

	int32 Result = 0;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "FileCopy", "(Ljava/lang/String;Ljava/lang/String;)I", false);
		auto jstring_SourcePath = FJavaHelper::ToJavaString(Env, SourcePath);
		auto jstring_TargetPath = FJavaHelper::ToJavaString(Env, TargetPath);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, *jstring_SourcePath, *jstring_TargetPath);
	}
#endif
	return Result;
}

void UPICOXRSystemAPI::PXR_IsMapInEffect(const FString& MapPath, FPICOIsMapInEffectDelegate InIsMapInEffectDelegate, int32 Ext)
{
	IsMapInEffectDelegate = InIsMapInEffectDelegate;

#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "isMapInEffect", "(Ljava/lang/String;I)V", false);
		auto jstring_MapPath = FJavaHelper::ToJavaString(Env, MapPath);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, *jstring_MapPath, Ext);
	}
#endif
}

void UPICOXRSystemAPI::PXR_ImportMapByPath(const FString& MapPath, FPICOImportMapByPathDelegate InImportMapByPathDelegate, int32 Ext)
{
	ImportMapByPathDelegate = InImportMapByPathDelegate;

#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "ImportMapByPath", "(Ljava/lang/String;I)V", false);
		auto jstring_MapPath = FJavaHelper::ToJavaString(Env, MapPath);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, *jstring_MapPath, Ext);
	}
#endif
}

FPICOEnterpriseIntDelegate UPICOXRSystemAPI::SetWifiP2PDeviceNameDelegate;
void UPICOXRSystemAPI::PXR_SetWifiP2PDeviceName(const FString& DeviceName, int Ext, FPICOEnterpriseIntDelegate Delegate)
{
	SetWifiP2PDeviceNameDelegate = Delegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto jstring_DeviceName = FJavaHelper::ToJavaString(Env, DeviceName);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetWifiP2PDeviceName", "(Ljava/lang/String;I)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, *jstring_DeviceName, Ext);
	}
#endif
}

FString UPICOXRSystemAPI::PXR_GetWifiP2PDeviceName(int Ext)
{
	FString Result;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GetWifiP2PDeviceName", "(I)Ljava/lang/String;", false);
		jstring JavaString = (jstring)FJavaWrapper::CallObjectMethod(Env, FJavaWrapper::GameActivityThis, Method, Ext);
		if (JavaString != NULL)
		{
			const char* JavaChars = Env->GetStringUTFChars(JavaString, 0);
			Result = FString(UTF8_TO_TCHAR(JavaChars));
			Env->ReleaseStringUTFChars(JavaString, JavaChars);
			Env->DeleteLocalRef(JavaString);
}
	}
#endif
	return Result;
}

bool UPICOXRSystemAPI::PXR_SetSystemKeyUsability(ESystemKey key, int Ext)
{
	int Result = 1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetSystemKeyUsability", "(II)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, (int)key, Ext);
	}
#endif
	return Result == 0;
}

bool UPICOXRSystemAPI::PXR_SetLauncher(FString PackageName)
{
	int Result = 1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto jstring_PackageName = FJavaHelper::ToJavaString(Env, PackageName);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetLauncher", "(Ljava/lang/String;)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, *jstring_PackageName);
	}
#endif
	return Result == 0;
}

bool UPICOXRSystemAPI::PXR_OpenTimingStartup(int Year, int Month, int Day, int Hour, int Minute)
{
	int Result = 1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "OpenTimingStartup", "(IIIII)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Year, Month, Day, Hour, Minute);
	}
#endif
	return Result == 0;
}

bool UPICOXRSystemAPI::PXR_CloseTimingStartup()
{
	int Result = 1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "CloseTimingStartup", "()I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
	return Result == 0;
}

bool UPICOXRSystemAPI::PXR_OpenTimingShutdown(int Year, int Month, int Day, int Hour, int Minute)
{
	int Result = 1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "OpenTimingShutdown", "(IIIII)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, Year, Month, Day, Hour, Minute);
	}
#endif
	return Result == 0;
}

bool UPICOXRSystemAPI::PXR_CloseTimingShutdown()
{
	int Result = 1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "CloseTimingShutdown", "()I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
	return Result == 0;
}

bool UPICOXRSystemAPI::PXR_SetTimeZone(FString TimeZone)
{
	int Result = 1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto jstring_TimeZone = FJavaHelper::ToJavaString(Env, TimeZone);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "SetTimeZone", "(Ljava/lang/String;)I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method, *jstring_TimeZone);
	}
#endif
	return Result == 0;
}

FPICOEnterpriseIntDelegate UPICOXRSystemAPI::AppCopyrightVerifyDelegate;
void UPICOXRSystemAPI::PXR_AppCopyrightVerify(FString PackageName, FPICOEnterpriseIntDelegate Delegate)
{
	AppCopyrightVerifyDelegate = Delegate;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		auto jstring_PackageName = FJavaHelper::ToJavaString(Env, PackageName);
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AppCopyrightVerify", "(Ljava/lang/String;)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, Method, *jstring_PackageName);
}
#endif
}

bool UPICOXRSystemAPI::PXR_GotoEnvironmentTextureCheck()
{
	int Result = 1;
#if PLATFORM_ANDROID
	if (JNIEnv* Env = FAndroidApplication::GetJavaEnv())
	{
		static jmethodID Method = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "GotoEnvironmentTextureCheck", "()I", false);
		Result = FJavaWrapper::CallIntMethod(Env, FJavaWrapper::GameActivityThis, Method);
	}
#endif
	return Result == 0;
}

#if PLATFORM_ANDROID
extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCAppCopyrightVerifyCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::AppCopyrightVerifyDelegate.ExecuteIfBound(Result);
}
#endif // PLATFORM_ANDROID

#if PLATFORM_ANDROID
extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetWifiP2PDeviceNameCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::SetWifiP2PDeviceNameDelegate.ExecuteIfBound(Result);
}
#endif // PLATFORM_ANDROID

void UPICOEnterpriseFunctionLibrary::PXR_ToUnrealPose(FVector& UnrealTranslation, FQuat& UnrealOrientation, const FVector& RMUVector, const FQuat& RMUOrientation)
{
	UnrealTranslation = FVector(-RMUVector.Z, RMUVector.X, RMUVector.Y);
	UnrealOrientation = FQuat(-RMUOrientation.Z, RMUOrientation.X, RMUOrientation.Y, -RMUOrientation.W);
}

void UPICOEnterpriseFunctionLibrary::PXR_ToRGBCameraPose(FVector& CameraTranslation, FQuat& CameraOrientation, const FRGBCameraParams& CameraParameters, const FVector& HeadTranslation, const FQuat& HeadOrientation)
{
	FMatrix HeadMatrix = FQuatRotationTranslationMatrix::Make(HeadOrientation, HeadTranslation);
	FMatrix CameraMatrix = FQuatRotationTranslationMatrix::Make(CameraParameters.ExternalOrientation, CameraParameters.ExternalTranslation);
	FMatrix RGBMatrix = HeadMatrix * CameraMatrix;
	FMatrix RotateX180 = FQuatRotationMatrix::Make(FQuat::MakeFromEuler(FVector(180, 0, 0)));
	RGBMatrix *= RotateX180;
	CameraTranslation = RGBMatrix.GetOrigin();
	CameraOrientation = RGBMatrix.ToQuat();
	PXR_ToUnrealPose(CameraTranslation, CameraOrientation, CameraTranslation, CameraOrientation);
}

void UPICOEnterpriseFunctionLibrary::PXR_UpdateRenderTargetFromRGB(const TArray<uint8>& RawData, int32 Width, int32 Height, UTextureRenderTarget2D* RenderTarget2D, uint8 OverrideAlpha)
{
	if (!RenderTarget2D || Width * Height * 3 != RawData.Num())
	{
		return;
	}

	EPixelFormat Format = RenderTarget2D->GetFormat();
	if (RenderTarget2D->SizeX != Width || RenderTarget2D->SizeY != Height || Format != EPixelFormat::PF_B8G8R8A8)
	{
		RenderTarget2D->InitCustomFormat(Width, Height, EPixelFormat::PF_B8G8R8A8, false);
	}

	int32 PixelNum = Width * Height;
	TArray<uint8> TargetTextureData;
	TargetTextureData.SetNum(PixelNum * 4);
	for (int32 Index = 0; Index < PixelNum; ++Index)
	{
		int32 TargetIndex = Index * 4;
		int32 SourceIndex = Index * 3;
		TargetTextureData[TargetIndex + 0] = RawData[SourceIndex + 0];
		TargetTextureData[TargetIndex + 1] = RawData[SourceIndex + 1];
		TargetTextureData[TargetIndex + 2] = RawData[SourceIndex + 2];
		TargetTextureData[TargetIndex + 3] = OverrideAlpha;
	}

	FRenderTarget* RenderTarget = RenderTarget2D->GameThread_GetRenderTargetResource();
	FUpdateTextureRegion2D Region = FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);
	ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
		[=](FRHICommandListImmediate& RHICmdList)
		{
			FTexture2DRHIRef TextureRHI = RenderTarget->GetRenderTargetTexture();
			check(TextureRHI.IsValid());
			RHIUpdateTexture2D(
				TextureRHI,
				0,
				Region,
				Width * 4,
				TargetTextureData.GetData()
			);
		}
	);
}

void UPICOEnterpriseFunctionLibrary::PXR_UpdateRenderTargetFromYUVNV21(const TArray<uint8>& RawData, int32 Width, int32 Height, UTextureRenderTarget2D* RenderTarget2D, uint8 OverrideAlpha)
{
	if (!RenderTarget2D || Width * Height * 3 / 2 != RawData.Num())
	{
		return;
	}

	EPixelFormat Format = RenderTarget2D->GetFormat();
	if (RenderTarget2D->SizeX != Width || RenderTarget2D->SizeY != Height || Format != EPixelFormat::PF_B8G8R8A8)
	{
		RenderTarget2D->InitCustomFormat(Width, Height, EPixelFormat::PF_B8G8R8A8, false);
	}

	int32 PixelNum = Width * Height;
	TArray<uint8> TargetTextureData;
	TargetTextureData.SetNum(PixelNum * 4);

	int32 Index = 0;
	for (int32 i = 0; i < Height; ++i) {
		for (int32 j = 0; j < Width; ++j) {
			int32 y = (0xff & ((int32)RawData[i * Width + j]));
			int32 v = (0xff & ((int32)RawData[PixelNum + (i >> 1) * Width + (j & ~1) + 0]));
			int32 u = (0xff & ((int32)RawData[PixelNum + (i >> 1) * Width + (j & ~1) + 1]));
			y = y < 16 ? 16 : y;

			int32 r = (int)(1.164f * (y - 16) + 1.596f * (v - 128));
			int32 g = (int)(1.164f * (y - 16) - 0.813f * (v - 128) - 0.391f * (u - 128));
			int32 b = (int)(1.164f * (y - 16) + 2.018f * (u - 128));

			r = r < 0 ? 0 : (r > 255 ? 255 : r);
			g = g < 0 ? 0 : (g > 255 ? 255 : g);
			b = b < 0 ? 0 : (b > 255 ? 255 : b);

			TargetTextureData[Index++] = b;
			TargetTextureData[Index++] = g;
			TargetTextureData[Index++] = r;
			TargetTextureData[Index++] = OverrideAlpha;
		}
	}

	FRenderTarget* RenderTarget = RenderTarget2D->GameThread_GetRenderTargetResource();
	FUpdateTextureRegion2D Region = FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);
	ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
		[=](FRHICommandListImmediate& RHICmdList)
		{
			FTexture2DRHIRef TextureRHI = RenderTarget->GetRenderTargetTexture();
			check(TextureRHI.IsValid());
			RHIUpdateTexture2D(
				TextureRHI,
				0,
				Region,
				Width * 4,
				TargetTextureData.GetData()
			);
		}
	);
}

#if PLATFORM_ANDROID
extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetDeviceActionCallBack(JNIEnv * env, jclass clazz, int EventType, int Result)
{
	if (EventType == 0)
	{
		if (UPICOXRSystemAPI::SetDeviceActionDelegates.Find(EDeviceControlEnum::DEVICE_CONTROL_REBOOT))
		{
			UPICOXRSystemAPI::SetDeviceActionDelegates.Find(EDeviceControlEnum::DEVICE_CONTROL_REBOOT)->ExecuteIfBound(Result);
			UPICOXRSystemAPI::SetDeviceActionDelegates.Remove(EDeviceControlEnum::DEVICE_CONTROL_REBOOT);
		}
	}
	else if (EventType == 1)
	{
		if (UPICOXRSystemAPI::SetDeviceActionDelegates.Find(EDeviceControlEnum::DEVICE_CONTROL_SHUTDOWN))
		{
			UPICOXRSystemAPI::SetDeviceActionDelegates.Find(EDeviceControlEnum::DEVICE_CONTROL_SHUTDOWN)->ExecuteIfBound(Result);
			UPICOXRSystemAPI::SetDeviceActionDelegates.Remove(EDeviceControlEnum::DEVICE_CONTROL_SHUTDOWN);
		}
	}
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCAPPMangerCallBack(JNIEnv * env, jclass clazz, int EventType, jstring AppPath, int Result)
{
	FAppManagerStruct ManagerStruct;
	ManagerStruct.AppPath = FJavaHelper::FStringFromParam(env, AppPath);
	ManagerStruct.PackageControl = static_cast<EPackageControlEnum>(EventType);
	if (UPICOXRSystemAPI::AppManagerDelegates.Find(ManagerStruct))
	{
		UPICOXRSystemAPI::AppManagerDelegates.Find(ManagerStruct)->ExecuteIfBound(Result);
		UPICOXRSystemAPI::AppManagerDelegates.Remove(ManagerStruct);
	}
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCAutoConnectWifiCallBack(JNIEnv * env, jclass clazz, jboolean Result)
{
	if (Result == JNI_TRUE)
	{
		UPICOXRSystemAPI::SetAutoConnectWifiDelegate.ExecuteIfBound(true);
	}
	else
	{
		UPICOXRSystemAPI::SetAutoConnectWifiDelegate.ExecuteIfBound(false);
	}
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCClearAutoConnectWifiCallBack(JNIEnv * env, jclass clazz, jboolean Result)
{
	if (Result == JNI_TRUE)
	{
		UPICOXRSystemAPI::ClearAutoConnectWifiDelegate.ExecuteIfBound(true);
	}
	else
	{
		UPICOXRSystemAPI::ClearAutoConnectWifiDelegate.ExecuteIfBound(false);
	}
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetHomeKeyCallBack(JNIEnv * env, jclass clazz, int EventType, int FunctionType, jboolean Result)
{
	if (Result == JNI_TRUE)
	{
		if (UPICOXRSystemAPI::SetHomeKeyDelegates.Find(static_cast<EHomeEventEnum>(EventType)))
		{
			UPICOXRSystemAPI::SetHomeKeyDelegates.Find(static_cast<EHomeEventEnum>(EventType))->ExecuteIfBound(true);
			UPICOXRSystemAPI::SetHomeKeyDelegates.Remove(static_cast<EHomeEventEnum>(EventType));
		}
	}
	else
	{
		if (UPICOXRSystemAPI::SetHomeKeyDelegates.Find(static_cast<EHomeEventEnum>(EventType)))
		{
			UPICOXRSystemAPI::SetHomeKeyDelegates.Find(static_cast<EHomeEventEnum>(EventType))->ExecuteIfBound(false);
			UPICOXRSystemAPI::SetHomeKeyDelegates.Remove(static_cast<EHomeEventEnum>(EventType));
		}
	}
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetHomeKeyAllCallBack(JNIEnv * env, jclass clazz, int EventType, int FunctionType, jboolean Result)
{
	if (Result == JNI_TRUE)
	{
		if (UPICOXRSystemAPI::SetHomeKeyAllDelegates.Find(static_cast<EHomeEventEnum>(EventType)))
		{
			UPICOXRSystemAPI::SetHomeKeyAllDelegates.Find(static_cast<EHomeEventEnum>(EventType))->ExecuteIfBound(true);
			UPICOXRSystemAPI::SetHomeKeyAllDelegates.Remove(static_cast<EHomeEventEnum>(EventType));
		}
	}
	else
	{
		if (UPICOXRSystemAPI::SetHomeKeyAllDelegates.Find(static_cast<EHomeEventEnum>(EventType)))
		{
			UPICOXRSystemAPI::SetHomeKeyAllDelegates.Find(static_cast<EHomeEventEnum>(EventType))->ExecuteIfBound(false);
			UPICOXRSystemAPI::SetHomeKeyAllDelegates.Remove(static_cast<EHomeEventEnum>(EventType));
		}
	}
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCDisablePowerKeyCallBack(JNIEnv * env, jclass clazz, jboolean isSingleTap, int Result)
{
	if (isSingleTap == JNI_TRUE)
	{
		if (UPICOXRSystemAPI::DisablePowerKeyDelegates.Find(true))
		{
			UPICOXRSystemAPI::DisablePowerKeyDelegates.Find(true)->ExecuteIfBound(Result);
			UPICOXRSystemAPI::DisablePowerKeyDelegates.Remove(true);
		}
	}
	else
	{
		if (UPICOXRSystemAPI::DisablePowerKeyDelegates.Find(false))
		{
			UPICOXRSystemAPI::DisablePowerKeyDelegates.Find(false)->ExecuteIfBound(Result);
			UPICOXRSystemAPI::DisablePowerKeyDelegates.Remove(false);
		}
	}
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetScreenOffDelayCallBack(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::SetScreenOffDelayDelegate.ExecuteIfBound(Result);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCWriteConfigFileToDataLocalCallback(JNIEnv * env, jclass clazz, jboolean Result)
{
	if (Result == JNI_TRUE)
	{
		UPICOXRSystemAPI::WriteConfigDelegate.ExecuteIfBound(true);
	}
	else
	{
		UPICOXRSystemAPI::WriteConfigDelegate.ExecuteIfBound(false);
	}
}
extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetWDJsonCallback(JNIEnv * env, jclass clazz, jstring Result)
{
	FString result = FJavaHelper::FStringFromLocalRef(env, Result);
	UPICOXRSystemAPI::SetWDJsonDelegate.ExecuteIfBound(result);
}
extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetWDModelsCallback(JNIEnv * env, jclass clazz, jstring Result)
{
	FString result = FJavaHelper::FStringFromLocalRef(env, Result);
	TArray<FWifiDisplayModel> models;
	models.Empty();
	TArray<FString> modelsString;
	result.ParseIntoArray(modelsString, TEXT("SPC"), true);
	for (int j = 0; j < modelsString.Num(); j++)
	{
		TArray<FString> substring;
		substring.Empty();
		modelsString[j].ParseIntoArray(substring, TEXT("="), true);
		substring.RemoveAt(0);
		for (int i = 0; i < substring.Num(); i++)
		{
			if (substring[i].Contains(TEXT("'")))
			{
				substring[i].Split(TEXT("'"), nullptr, &substring[i]);
				substring[i].Split(TEXT("'"), &substring[i], nullptr);
			}
			else
			{
				substring[i].Split(TEXT(","), &substring[i], nullptr);
			}
		}
		FWifiDisplayModel model;
		model.deviceAddress = substring[0];
		model.deviceName = substring[1];
		model.isAvailable = substring[2] == "true" ? true : false;
		model.canConnect = substring[3] == "true" ? true : false;
		model.isRemembered = substring[4] == "true" ? true : false;
		model.statusCode = FCString::Atoi(*substring[5]);
		model.status = substring[6];
		model.description = substring[7];
		models.Add(model);
	}

	UPICOXRSystemAPI::SetWDModelsDelegate.ExecuteIfBound(models);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSwitchLargeSpaceSceneCallback(JNIEnv * env, jclass clazz, jboolean Result)
{
	if (Result == JNI_TRUE)
	{
		UPICOXRSystemAPI::EnableLargeSpaceDelegate.ExecuteIfBound(true);
	}
	else
	{
		UPICOXRSystemAPI::EnableLargeSpaceDelegate.ExecuteIfBound(false);
	}
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCLargeSpaceStatusCallback(JNIEnv * env, jclass clazz, jstring Result)
{
	FString result = FJavaHelper::FStringFromLocalRef(env, Result);
	UPICOXRSystemAPI::SwitchLargeSpaceStatusDelegate.ExecuteIfBound(result);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCExportMapsCallback(JNIEnv * env, jclass clazz, jboolean Result)
{
	if (Result == JNI_TRUE)
	{
		UPICOXRSystemAPI::ExportMapsDelegate.ExecuteIfBound(true);
	}
	else
	{
		UPICOXRSystemAPI::ExportMapsDelegate.ExecuteIfBound(false);
	}
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCImportMapsCallback(JNIEnv * env, jclass clazz, jboolean Result)
{
	if (Result == JNI_TRUE)
	{
		UPICOXRSystemAPI::ImportMapsDelegate.ExecuteIfBound(true);
	}
	else
	{
		UPICOXRSystemAPI::ImportMapsDelegate.ExecuteIfBound(false);
	}
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCControlSetAutoConnectWIFIWithErrorCodeCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::ControlSetAutoConnectWIFIWithErrorCodeDelegate.ExecuteIfBound(Result);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCGetSwitchSystemFunctionStatusCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::GetSwitchSystemFunctionStatusDelegate.ExecuteIfBound(Result);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCPICOCastInitCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::PICOCastInitDelegate.ExecuteIfBound((ECastInitResult)Result);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetControllerPairTimeCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::SetControllerPairTimeDelegate.ExecuteIfBound(Result);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCGetControllerPairTimeCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::GetControllerPairTimeDelegate.ExecuteIfBound((EControllerPairTimeEnum)Result);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetSystemCountryCodeCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::SetSystemCountryCodeDelegate.ExecuteIfBound(Result);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetIPDCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::SetIPDDelegate.ExecuteIfBound(Result);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetArUcoMarkerCallback(JNIEnv * env, jclass clazz, jobjectArray MarkArray)
{
	FTransform TrackingToWorld = FTransform::Identity;
	IXRTrackingSystem* TrackingSys = GEngine->XRSystem.Get();
	if (TrackingSys)
	{
		TrackingToWorld = TrackingSys->GetTrackingToWorldTransform();
	}

	TArray<FMarkerResultCallback> Results;
	jsize NumMark = env->GetArrayLength(MarkArray);
	for (int32 Index = 0; Index < NumMark; ++Index)
	{
		FMarkerResultCallback Result;

		jobject MarkObject = env->GetObjectArrayElement(MarkArray, Index);
		jclass MarkClass = env->GetObjectClass(MarkObject);

		FVector Position;
		jfieldID PosXField = env->GetFieldID(MarkClass, "posX", "D");
		Position.X = (float)env->GetDoubleField(MarkObject, PosXField);

		jfieldID PosYField = env->GetFieldID(MarkClass, "posY", "D");
		Position.Y = (float)env->GetDoubleField(MarkObject, PosYField);

		jfieldID PosZField = env->GetFieldID(MarkClass, "posZ", "D");
		Position.Z = (float)env->GetDoubleField(MarkObject, PosZField);

		float TrackingOriginHeight = UPICOXRHMDFunctionLibrary::PXR_GetToDeltaSensorY();

		Result.MarkerTranslation = FVector(-Position.Z, Position.X, Position.Y + TrackingOriginHeight);
		if (IsValid(UPICOXRSystemAPI::MarkerWorldContext))
		{
			UWorld* World = GEngine->GetWorldFromContextObject(UPICOXRSystemAPI::MarkerWorldContext, EGetWorldErrorMode::ReturnNull);
			if (!IsValid(World))
			{
				World = GWorld.GetReference();
			}

			if (IsValid(World))
			{
				float WorldScale = World->GetWorldSettings()->WorldToMeters;
				Result.MarkerTranslation *= WorldScale;
			}
		}
		Result.MarkerTranslation = TrackingToWorld.TransformPosition(Result.MarkerTranslation);

		FQuat Orientation;
		jfieldID RotationXField = env->GetFieldID(MarkClass, "rotationX", "D");
		Orientation.X = (float)env->GetDoubleField(MarkObject, RotationXField);

		jfieldID RotationYField = env->GetFieldID(MarkClass, "rotationY", "D");
		Orientation.Y = (float)env->GetDoubleField(MarkObject, RotationYField);

		jfieldID RotationZField = env->GetFieldID(MarkClass, "rotationZ", "D");
		Orientation.Z = (float)env->GetDoubleField(MarkObject, RotationZField);

		jfieldID RotationWField = env->GetFieldID(MarkClass, "rotationW", "D");
		Orientation.W = (float)env->GetDoubleField(MarkObject, RotationWField);

		Result.MarkerOrientation = FQuat(Orientation.Z, -Orientation.X, -Orientation.Y, Orientation.W);
		Result.MarkerOrientation = TrackingToWorld.TransformRotation(Result.MarkerOrientation);

		jfieldID TimestampField = env->GetFieldID(MarkClass, "dTimestamp", "D");
		Result.Timestamp = (float)env->GetDoubleField(MarkObject, TimestampField);

		jfieldID ValidFlagField = env->GetFieldID(MarkClass, "validFlag", "I");
		Result.ValidFlag = (int32)env->GetIntField(MarkObject, ValidFlagField);

		jfieldID MarkerTypeField = env->GetFieldID(MarkClass, "markerType", "I");
		Result.MarkerType = (int32)env->GetIntField(MarkObject, MarkerTypeField);

		jfieldID MarkerIdField = env->GetFieldID(MarkClass, "iMarkerId", "I");
		Result.MarkerID = (int32)env->GetIntField(MarkObject, MarkerIdField);

		jfieldID ReserveField = env->GetFieldID(MarkClass, "reserve", "[I");
		jintArray ReserveIntArray = (jintArray)env->GetObjectField(MarkObject, ReserveField);
		jint* ReserveInts = env->GetIntArrayElements(ReserveIntArray, 0);
		jsize ReserveSize = env->GetArrayLength(ReserveIntArray);
		for (int32 ReserveIndex = 0; ReserveIndex < ReserveSize; ++ReserveIndex)
		{
			Result.Reserve.Add((int32)ReserveInts[ReserveIndex]);
		}

		Results.Add(Result);
	}
	UPICOXRSystemAPI::SetArUcoMarkerDelegate.ExecuteIfBound(Results);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCSetArUcoMarkerFCallback(JNIEnv * env, jclass clazz, jobjectArray MarkArray)
{
	FTransform TrackingToWorld = FTransform::Identity;
	IXRTrackingSystem* TrackingSys = GEngine->XRSystem.Get();
	if (TrackingSys)
	{
		TrackingToWorld = TrackingSys->GetTrackingToWorldTransform();
	}

	TArray<FMarkerFResultCallback> Results;
	jsize NumMark = env->GetArrayLength(MarkArray);
	for (int32 Index = 0; Index < NumMark; ++Index)
	{
		FMarkerFResultCallback Result;

		jobject MarkObject = env->GetObjectArrayElement(MarkArray, Index);
		jclass MarkClass = env->GetObjectClass(MarkObject);

		FVector Position;
		jfieldID PosXField = env->GetFieldID(MarkClass, "posX", "D");
		Position.X = (float)env->GetDoubleField(MarkObject, PosXField);

		jfieldID PosYField = env->GetFieldID(MarkClass, "posY", "D");
		Position.Y = (float)env->GetDoubleField(MarkObject, PosYField);

		jfieldID PosZField = env->GetFieldID(MarkClass, "posZ", "D");
		Position.Z = (float)env->GetDoubleField(MarkObject, PosZField);

		float TrackingOriginHeight = UPICOXRHMDFunctionLibrary::PXR_GetToDeltaSensorY();

		Result.MarkerTranslation = FVector(-Position.Z, Position.X, Position.Y + TrackingOriginHeight);
		if (IsValid(UPICOXRSystemAPI::MarkerFWorldContext))
		{
			UWorld* World = GEngine->GetWorldFromContextObject(UPICOXRSystemAPI::MarkerFWorldContext, EGetWorldErrorMode::ReturnNull);
			if (!IsValid(World))
			{
				World = GWorld.GetReference();
			}

			if (IsValid(World))
			{
				float WorldScale = World->GetWorldSettings()->WorldToMeters;
				Result.MarkerTranslation *= WorldScale;
			}
		}
		Result.MarkerTranslation = TrackingToWorld.TransformPosition(Result.MarkerTranslation);

		FQuat Orientation;
		jfieldID RotationXField = env->GetFieldID(MarkClass, "rotationX", "D");
		Orientation.X = (float)env->GetDoubleField(MarkObject, RotationXField);

		jfieldID RotationYField = env->GetFieldID(MarkClass, "rotationY", "D");
		Orientation.Y = (float)env->GetDoubleField(MarkObject, RotationYField);

		jfieldID RotationZField = env->GetFieldID(MarkClass, "rotationZ", "D");
		Orientation.Z = (float)env->GetDoubleField(MarkObject, RotationZField);

		jfieldID RotationWField = env->GetFieldID(MarkClass, "rotationW", "D");
		Orientation.W = (float)env->GetDoubleField(MarkObject, RotationWField);

		Result.MarkerOrientation = FQuat(Orientation.Z, -Orientation.X, -Orientation.Y, Orientation.W);
		Result.MarkerOrientation = TrackingToWorld.TransformRotation(Result.MarkerOrientation);

		jfieldID TimestampField = env->GetFieldID(MarkClass, "dTimestamp", "D");
		Result.Timestamp = (float)env->GetDoubleField(MarkObject, TimestampField);

		jfieldID ValidFlagField = env->GetFieldID(MarkClass, "validFlag", "I");
		Result.ValidFlag = (int32)env->GetIntField(MarkObject, ValidFlagField);

		jfieldID MarkerTypeField = env->GetFieldID(MarkClass, "markerType", "I");
		Result.MarkerType = (int32)env->GetIntField(MarkObject, MarkerTypeField);

		jfieldID MarkerIdField = env->GetFieldID(MarkClass, "iMarkerId", "I");
		Result.MarkerID = (int32)env->GetIntField(MarkObject, MarkerIdField);

		jfieldID ReserveField = env->GetFieldID(MarkClass, "reserve", "[F");
		jfloatArray ReserveFloatArray = (jfloatArray)env->GetObjectField(MarkObject, ReserveField);
		jfloat* ReserveFloats = env->GetFloatArrayElements(ReserveFloatArray, 0);
		jsize ReserveSize = env->GetArrayLength(ReserveFloatArray);
		for (int32 ReserveIndex = 0; ReserveIndex < ReserveSize; ++ReserveIndex)
		{
			Result.Reserve.Add((float)ReserveFloats[ReserveIndex]);
		}

		Results.Add(Result);
	}
	UPICOXRSystemAPI::SetArUcoMarkerFDelegate.ExecuteIfBound(Results);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCFileCopyCallback(JNIEnv * env, jclass clazz, jboolean bStart, jboolean bProcess, jdouble Process, jboolean bFinish, int ErrorCode)
{
	UPICOXRSystemAPI::FileCopyDelegate.ExecuteIfBound(bStart, bProcess, static_cast<float>(Process), bFinish, ErrorCode);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCIsMapInEffectCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::IsMapInEffectDelegate.ExecuteIfBound(Result);
}

extern "C" JNIEXPORT void  JNICALL Java_com_epicgames_unreal_GameActivity_JavaToCImportMapByPathCallback(JNIEnv * env, jclass clazz, int Result)
{
	UPICOXRSystemAPI::ImportMapByPathDelegate.ExecuteIfBound(Result);
}

#endif
