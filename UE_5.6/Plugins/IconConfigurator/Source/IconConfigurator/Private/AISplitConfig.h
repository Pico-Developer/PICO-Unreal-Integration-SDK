// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FAISplitAuthConfig
{
	int64 AppId = 0;
	FString Salt;
};

struct FAISplitTosConfig
{
	FString Endpoint;
	FString Bucket;
	FString Ak;
	FString PublicUrlPrefix;
	FString ObjectDir;
};

struct FAISplitApiConfig
{
	FString GenUrl;
	FString InfoUrl;
	FString TtEnv;
};

struct FAISplitTimeoutConfig
{
	int32 Connect = 15000;
	int32 Read = 60000;
	int32 PollInterval = 3000;
};

struct FAISplitRegionConfig
{
	FAISplitAuthConfig Auth;
	FAISplitTosConfig Tos;
	FAISplitApiConfig Api;
};

struct FAISplitTccConfig
{
	bool bEnable = false;
	TMap<FString, FAISplitRegionConfig> RegionConfig;
	FAISplitTimeoutConfig TimeoutMs;
};
