// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "PICO_VerificationStyle.h"
#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr< FSlateStyleSet > FPICOOpenXRVerificationStyle::PICOOpenXRStyleInstance = NULL;

void FPICOOpenXRVerificationStyle::Initialize()
{
	if (!PICOOpenXRStyleInstance.IsValid())
	{
		PICOOpenXRStyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*PICOOpenXRStyleInstance);
	}
}

void FPICOOpenXRVerificationStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*PICOOpenXRStyleInstance);
	ensure(PICOOpenXRStyleInstance.IsUnique());
	PICOOpenXRStyleInstance.Reset();
}

FName FPICOOpenXRVerificationStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("PICOOpenXRVerificationStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon90x90(90.0f, 90.0f);

TSharedRef< FSlateStyleSet > FPICOOpenXRVerificationStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("PICOOpenXRVerificationStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("PICOOpenXR")->GetBaseDir() / TEXT("Resources"));

	Style->Set("PICOOpenXRVerification.PluginAction", new IMAGE_BRUSH(TEXT("Icon_90x"), Icon90x90));
	
	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FPICOOpenXRVerificationStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FPICOOpenXRVerificationStyle::Get()
{
	return *PICOOpenXRStyleInstance;
}
