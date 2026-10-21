// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AISplitConfig.h"

namespace AISplitHelpers
{
	/**
	 * Parse TCC HTTP response body into FAISplitTccConfig.
	 * Called after async HTTP GET completes.
	 */
	bool ParseTccConfig(const FString& ResponseBody, FAISplitTccConfig& OutConfig);

	/**
	 * Compute SHA-256 hex digest of a UTF-8 string.
	 */
	FString ComputeSHA256Hex(const FString& Input);

	/**
	 * Get or create a persistent int64 device ID.
	 */
	int64 GetOrCreateDeviceId();
}
