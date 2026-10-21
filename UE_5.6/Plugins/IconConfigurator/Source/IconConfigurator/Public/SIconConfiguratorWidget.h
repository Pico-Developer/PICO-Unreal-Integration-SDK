// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Styling/SlateBrush.h"
#include "IconLayerData.h"
#include "AISplitConfig.h"
#include "Interfaces/IHttpRequest.h"
#include "Containers/Ticker.h"

class SIconConfigurator2DViewport;
class SIconConfigurator3DViewport;
class SWidgetSwitcher;
class SBox;

/**
 * Main tool panel (single vertical column layout):
 *   ① Tab header row (Layer | Manual / AI Split)
 *   ② Tab content switcher (SWidgetSwitcher)
 *   ③ Preview section (2D left / 3D right, side by side)
 *   ④ Localization section
 *   ⑤ Apply button (always visible at the bottom)
 */
class SIconConfiguratorWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SIconConfiguratorWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

private:
	// ---- Internal data structures ----
	struct FLocEntry
	{
		FString LangCode;            // BCP-47 tag: "" = default (en-US), "zh-CN", "ja-JP", "ko-KR"
		FString LangDisplay;         // Display name
		TSharedRef<FString> AppName = MakeShared<FString>();
	};

	// ---- Layer constants ----
	static constexpr int32 kMinLayers = 2;
	static constexpr int32 kMaxLayers = 3;

	// ---- UI build helpers ----
	TSharedRef<SWidget> BuildTabHeader();
	TSharedRef<SWidget> BuildManualContent();
	TSharedRef<SWidget> BuildAISplitContent();
	TSharedRef<SWidget> BuildLayerRow(int32 LayerIndex, const FText& Label, bool bRequired);
	TSharedRef<SWidget> BuildPreviewSection();
	TSharedRef<SWidget> BuildLocalizationSection();
	TSharedRef<SWidget> BuildLocRow(TSharedPtr<FLocEntry> Entry, bool bIsDefault);
	void RebuildLocEntries();
	void RebuildManualLayers();
	void RebuildAIResultLayers();

	// ---- Slate attribute binding helpers ----
	const FSlateBrush* GetThumbnailBrush(int32 LayerIndex) const;
	const FSlateBrush* GetAIThumbnailBrush(int32 LayerIndex) const;
	const FSlateBrush* GetAISplitBrush() const;
	FSlateColor GetTabButtonColor(int32 TabIndex) const;
	FSlateColor  GetTabTextColor(int32 TabIndex) const;

	// ---- Callbacks ----
	FReply OnPickFile(int32 LayerIndex);
	FReply OnPickAISplitFile();
	FReply OnTabSwitch(int32 TabIndex);
	FReply OnAddLocalization();
	FReply OnAddLayer();
	FReply OnRemoveLayer(int32 LayerIndex);
	FReply OnMoveLayerUp(int32 LayerIndex);
	FReply OnMoveLayerDown(int32 LayerIndex);
	FReply OnApply();

	// ---- Utility functions ----
	void LoadPreviewTexture(int32 LayerIndex);
	void UpdateThumbnail(int32 LayerIndex);
	void RefreshViewports();

	void LoadAIPreviewTexture(int32 LayerIndex);
	void UpdateAIThumbnail(int32 LayerIndex);

	// ---- State persistence ----
	void SaveState() const;
	void LoadState();

	// ---- Layer data ----
	TArray<FIconLayerData> Layers;
	FSlateBrush ThumbnailBrushes[3];

	// ---- AI Split layer data (independent from Manual) ----
	TArray<FIconLayerData> AILayers;
	FSlateBrush AIThumbnailBrushes[3];

	// ---- AI Split data ----
	FString     AISplitFilePath;
	FSlateBrush AISplitThumbnailBrush;

	// ---- AI Split state machine ----
	enum class EAISplitState : uint8
	{
		Idle, FetchingConfig, UploadingToTos, Generating, Polling, Downloading, Done, Error
	};
	EAISplitState AISplitState      = EAISplitState::Idle;
	FString       AISplitErrorMsg;
	float         AISplitProgress   = 0.f;
	bool          bAISplitAgreed    = false;

	// ---- AI Split pipeline runtime ----
	FAISplitTccConfig    AISplitTccConfig;
	FAISplitRegionConfig AISplitRegionCfg;
	FString              AISplitObjectKey;
	FString              AISplitUploadedUrl;
	FString              AISplitPresignPutUrl;   // PUT URL returned by presign
	TArray<uint8>        AISplitFileData;         // File bytes to upload (cached for presign flow)
	FString              AISplitImageBase64;       // Pure base64 without data: prefix (fallback path)
	FString              AISplitTaskId;
	TArray<FString>      AISplitLayerUrls;
	TArray<FString>      AISplitSdfUrls;
	TArray<FString>      AISplitDLLayerPaths;
	TArray<FString>      AISplitDLSdfPaths;
	int32                AISplitDLDone  = 0;
	int32                AISplitDLTotal = 0;
	FTSTicker::FDelegateHandle AISplitPollTickHandle;

	// ---- AI Split callbacks ----
	FReply OnGenerateClicked();
	void   OnAISplitAgreedChanged(ECheckBoxState NewState);

	// ---- AI Split pipeline steps ----
	void AISplit_FetchTccConfig();
	void AISplit_OnTccConfigDone(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bConn);
	void AISplit_ParseTccAndContinue(const FString& ResponseBody);
	void AISplit_UploadToTos();
	void AISplit_OnUploadDone(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bConn);
	void AISplit_GetPresignUrl();
	void AISplit_OnPresignDone(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bConn);
	void AISplit_StartGenerate();
	void AISplit_OnGenerateDone(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bConn);
	void AISplit_ScheduleNextPoll();
	void AISplit_DoPoll();
	void AISplit_OnPollDone(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bConn);
	void AISplit_DownloadAll();
	void AISplit_OnOneFileDone(FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bConn,
	                           FString DestPath, int32 Idx, bool bIsSdf);
	void AISplit_FillAILayers();
	void AISplit_SetError(const FString& Msg);

	// ---- AI Split Slate attribute helpers ----
	bool             IsGenerateEnabled() const;
	EVisibility      GetAISplitProgressVis() const;
	EVisibility      GetAISplitErrorVis() const;
	FText            GetAISplitStatusText() const;
	TOptional<float> GetAISplitProgressVal() const;

	// ---- Localization ----
	TArray<TSharedPtr<FLocEntry>>  LocEntries;
	TArray<TSharedPtr<FString>>    LangDisplayOptions;
	TArray<FString>                LangCodes;

	// ---- Tab state ----
	int32 ActiveTab = 0;

	// ---- Child widget references ----
	TSharedPtr<SIconConfigurator2DViewport> Viewport2D;   // shared; pointer swapped on tab switch
	TSharedPtr<SIconConfigurator3DViewport> Viewport3D;   // shared; pointer swapped on tab switch
	TSharedPtr<SWidgetSwitcher>        TabSwitcher;
	TSharedPtr<SBox>                   LocEntriesContainer;
	TSharedPtr<SBox>                   ManualLayersContainer;
	TSharedPtr<SBox>                   AIResultLayersContainer;
};
