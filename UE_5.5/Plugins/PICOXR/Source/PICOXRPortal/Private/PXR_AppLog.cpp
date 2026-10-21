// Fill out your copyright notice in the Description page of Project Settings.


#include "PXR_AppLog.h"
#include "applogrs.hpp"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

//Todo:Unreal app id
FString PXR_AppLog::StrAppID = "793022";
//Todo:PICO_UnrealSDK
FString PXR_AppLog::StrChannelName = "PICO_UnrealSDK";
//Todo:unreal_xr_sdk
FString PXR_AppLog::StrXRSDK = "unreal_xr_sdk";
bool PXR_AppLog::IsInitialized = false;
PXR_AppLog::PXR_AppLog()
{
}

PXR_AppLog::~PXR_AppLog()
{
}

bool PXR_AppLog::InitAppLog()
{
	if (IsInitialized)
	{
		return false;
	}
	IsInitialized = true;
	const char* appid = TCHAR_TO_UTF8(*StrAppID);
	const char* channel = TCHAR_TO_UTF8(*StrChannelName);

	applog::AppLog_init(appid, channel);
	UE_LOG(LogAppLog, Verbose, TEXT("AppLog has been initialized."));
	return true;
}

void PXR_AppLog::ShutdownAppLog()
{
	auto DestroyCallback = []()
	{
		UE_LOG(LogAppLog, Verbose, TEXT("AppLog has been destroyed."));
	};
	
	applog::AppLog_destroy(DestroyCallback);
}

void PXR_AppLog::AddAppLog(const FString& param, const FString& value)
{
	FString JsonString=GetJsonStringFromParamAndValue(param,value);

	const char* event = TCHAR_TO_UTF8(*StrXRSDK);
	const char* params = TCHAR_TO_UTF8(*JsonString);
	
	applog::AppLog_onEvent(event,params);
}

FString PXR_AppLog::GetJsonStringFromParamAndValue(const FString& param, const FString& value)
{
	TSharedPtr<FJsonObject> contentData = MakeShareable(new FJsonObject());
	contentData->SetStringField(param, value);

	FString JsonString;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(contentData.ToSharedRef(), JsonWriter);
	JsonWriter->Close();

	return JsonString;
}
