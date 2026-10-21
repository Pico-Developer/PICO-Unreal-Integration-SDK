// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Texture2D.h"

/** Runtime state of a single icon layer */
struct FIconLayerData
{
	// Local file path (used for display and reload)
	FString FilePath;

	// UE Texture2D after import (nullptr if not imported)
	TWeakObjectPtr<UTexture2D> ImportedTexture;

	// Transient preview Texture2D (loaded directly from disk, not stored in Content)
	TWeakObjectPtr<UTexture2D> PreviewTexture;

	// Visibility
	bool bVisible = true;

	// Opacity 0~1
	float Opacity = 1.0f;

	// 2D offset in pixels
	FVector2D Offset = FVector2D::ZeroVector;

	// 2D scale
	float Scale = 1.0f;

	FIconLayerData() = default;
};
