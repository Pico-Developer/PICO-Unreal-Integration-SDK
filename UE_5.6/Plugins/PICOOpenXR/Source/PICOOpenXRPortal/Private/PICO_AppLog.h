// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
DEFINE_LOG_CATEGORY_STATIC(LogAppLog, Log, All);

/**
 * 
 */
class PICOOPENXRPORTAL_API PICO_AppLog
{
public:
	PICO_AppLog();
	~PICO_AppLog();

	static bool InitAppLog();
	static void ShutdownAppLog();

	static void AddAppLog(const FString& param, const FString& value);
	
private:
	static FString GetJsonStringFromParamAndValue(const FString& param, const FString& value);

	static FString StrAppID;
	static FString StrChannelName;
	static FString StrXRSDK;
	static bool IsInitialized;
};
