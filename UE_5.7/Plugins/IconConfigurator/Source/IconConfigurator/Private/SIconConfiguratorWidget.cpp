// Copyright PICO Technology Co., Ltd. All rights reserved.
// This plugin incorporates portions of the Unreal® Engine. Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.
// Copyright Epic Games, Inc. All Rights Reserved.

#include "SIconConfiguratorWidget.h"

#include "SIconConfigurator2DViewport.h"
#include "SIconConfigurator3DViewport.h"
#include "IconImagePipeline.h"
#include "AISplitHelpers.h"

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "SIconConfiguratorWidget"

// ---------------------------------------------------------------
// Internal widget: thumbnail checkerboard background (reuses SIconConfigurator2DViewport draw logic)
// ---------------------------------------------------------------
class SThumbnailCheckerboard : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SThumbnailCheckerboard) {}
	SLATE_END_ARGS()

	void Construct(const FArguments&) {}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(60.f, 60.f);
	}

	virtual int32 OnPaint(
		const FPaintArgs& Args,
		const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect,
		FSlateWindowElementList& OutDrawElements,
		int32 LayerId,
		const FWidgetStyle& InWidgetStyle,
		bool bParentEnabled) const override
	{
		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const float CellSize = 8.f;
		const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

		for (float y = 0.f; y < Size.Y; y += CellSize)
		{
			for (float x = 0.f; x < Size.X; x += CellSize)
			{
				const int32 CellX = FMath::FloorToInt(x / CellSize);
				const int32 CellY = FMath::FloorToInt(y / CellSize);
				const bool bDark = (CellX + CellY) % 2 == 0;
				const FLinearColor Color = bDark
					? FLinearColor(0.15f, 0.15f, 0.15f, 1.f)
					: FLinearColor(0.45f, 0.45f, 0.45f, 1.f);

				FSlateDrawElement::MakeBox(
					OutDrawElements, LayerId,
					AllottedGeometry.ToPaintGeometry(
						FVector2D(FMath::Min(CellSize, Size.X - x),
						          FMath::Min(CellSize, Size.Y - y)),
						FSlateLayoutTransform(FVector2D(x, y))),
					WhiteBrush, ESlateDrawEffect::None, Color);
			}
		}
		return LayerId;
	}
};

// ---- Color constants ----
static const FLinearColor kBlue    = FLinearColor(0.231f, 0.471f, 0.957f);
static const FLinearColor kDarkBg  = FLinearColor(0.12f,  0.12f,  0.12f,  1.f);
static const FLinearColor kTabInactive = FLinearColor(0.18f, 0.18f, 0.18f, 1.f);

// ---------------------------------------------------------------
// Construct
// ---------------------------------------------------------------
void SIconConfiguratorWidget::Construct(const FArguments& InArgs)
{
	// Initialize layers
	Layers.SetNum(kMaxLayers);
	AILayers.SetNum(kMaxLayers);

	// Initialize thumbnail brushes
	for (int32 i = 0; i < kMaxLayers; ++i)
	{
		ThumbnailBrushes[i] = FSlateBrush();
		ThumbnailBrushes[i].DrawAs = ESlateBrushDrawType::Image;
		AIThumbnailBrushes[i] = FSlateBrush();
		AIThumbnailBrushes[i].DrawAs = ESlateBrushDrawType::Image;
	}
	AISplitThumbnailBrush = FSlateBrush();
	AISplitThumbnailBrush.DrawAs = ESlateBrushDrawType::Image;

	// Initialize language options (second param is BCP-47 tag; directory is derived by Bcp47ToAndroidQualifier on Apply)
	auto AddLang = [&](const FString& Display, const FString& Bcp47)
	{
		LangDisplayOptions.Add(MakeShared<FString>(Display));
		LangCodes.Add(Bcp47);
	};

	static const TCHAR* kLanguages[][2] =
	{
		{ TEXT("Simplified Chinese"),         TEXT("zh-CN")  },
		{ TEXT("Japanese"),                   TEXT("ja-JP")  },
		{ TEXT("Korean"),                     TEXT("ko-KR")  },
	};
	for (const TCHAR* const* Lang : kLanguages)
	{
		AddLang(FString::Printf(TEXT("%s (%s)"), Lang[0], Lang[1]), Lang[1]);
	}

	// Default Localization entry (English)
	TSharedPtr<FLocEntry> DefaultLoc = MakeShared<FLocEntry>();
	DefaultLoc->LangCode    = TEXT("");
	DefaultLoc->LangDisplay = TEXT("English (United States)");
	LocEntries.Add(DefaultLoc);

	// ---- Main layout ----
	ChildSlot
	[
		SNew(SVerticalBox)

		// Upper section: scrollable content
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			.Padding(FMargin(12.f, 12.f, 12.f, 8.f))
			[
				SNew(SVerticalBox)

		// ① Tab header row
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 10.f)
		[ BuildTabHeader() ]

		// Compatibility note
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 8.f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("CompatNote",
				"3D icons are only applicable to Project Swan. "
				"2D icons will still be displayed on PICO 4 and PICO 4 Ultra."))
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 9))
			.AutoWrapText(true)
		]

		// ② Tab content
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(TabSwitcher, SWidgetSwitcher)
					+ SWidgetSwitcher::Slot() [ BuildManualContent()  ]
					+ SWidgetSwitcher::Slot() [ BuildAISplitContent() ]
				]

				// ③ Preview (single shared viewport; data pointer swapped on tab switch)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 14.f, 0.f, 0.f)
				[ BuildPreviewSection() ]

				// ④ Localization
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.f, 14.f, 0.f, 0.f)
				[ BuildLocalizationSection() ]
			]
		]

			// ⑤ Apply button (always visible at the bottom)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(12.f, 8.f))
		[
			SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.ButtonColorAndOpacity(FSlateColor(kBlue))
				.ContentPadding(FMargin(0.f, 10.f))
				.OnClicked(this, &SIconConfiguratorWidget::OnApply)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Apply", "Apply"))
					.ColorAndOpacity(FLinearColor::White)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]
		]
	];

	// Restore previous session state (no-op if no file exists)
	LoadState();
}

// ---------------------------------------------------------------
// ① Tab header row
// ---------------------------------------------------------------
TSharedRef<SWidget> SIconConfiguratorWidget::BuildTabHeader()
{
	return SNew(SHorizontalBox)

	// "Layer" label
	+ SHorizontalBox::Slot()
	.FillWidth(1.f)
	.VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("LayerLabel", "Layer"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
	]

	// [Manual] button
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.Padding(0.f, 0.f, 2.f, 0.f)
	[
		SNew(SButton)
		.HAlign(HAlign_Center)
		.ContentPadding(FMargin(18.f, 6.f))
		.ButtonColorAndOpacity(TAttribute<FSlateColor>::CreateSP(this, &SIconConfiguratorWidget::GetTabButtonColor, 0))
		.OnClicked(FOnClicked::CreateSP(this, &SIconConfiguratorWidget::OnTabSwitch, 0))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TabManual", "Manual"))
			.ColorAndOpacity(TAttribute<FSlateColor>::CreateSP(this, &SIconConfiguratorWidget::GetTabTextColor, 0))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
		]
	]

	// [AI Split] button
	+ SHorizontalBox::Slot()
	.AutoWidth()
	[
		SNew(SButton)
		.HAlign(HAlign_Center)
		.ContentPadding(FMargin(18.f, 6.f))
		.ButtonColorAndOpacity(TAttribute<FSlateColor>::CreateSP(this, &SIconConfiguratorWidget::GetTabButtonColor, 1))
		.OnClicked(FOnClicked::CreateSP(this, &SIconConfiguratorWidget::OnTabSwitch, 1))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TabAISplit", "AI Split"))
			.ColorAndOpacity(TAttribute<FSlateColor>::CreateSP(this, &SIconConfiguratorWidget::GetTabTextColor, 1))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
		]
	];
}

// ---------------------------------------------------------------
// ② Manual Tab content
// ---------------------------------------------------------------
TSharedRef<SWidget> SIconConfiguratorWidget::BuildManualContent()
{
	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);

	// Title row with "+" button (visible when layers < max)
	Box->AddSlot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 6.f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ManualLayers", "Layers"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.ContentPadding(FMargin(6.f, 2.f))
			.Visibility_Lambda([this]() -> EVisibility
			{
				return Layers.Num() < kMaxLayers ? EVisibility::Visible : EVisibility::Collapsed;
			})
			.OnClicked(this, &SIconConfiguratorWidget::OnAddLayer)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("+")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]
		]
	];

	// Dynamic layer rows container
	Box->AddSlot()
	.AutoHeight()
	[
		SAssignNew(ManualLayersContainer, SBox)
	];

	// Populate initial layer rows
	RebuildManualLayers();

	return Box;
}

// ---------------------------------------------------------------
// Rebuild dynamic manual layer rows
// ---------------------------------------------------------------
void SIconConfiguratorWidget::RebuildManualLayers()
{
	if (!ManualLayersContainer.IsValid()) return;

	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	for (int32 i = 0; i < Layers.Num(); ++i)
	{
		FText Label;
		bool bRequired;
		if (i == 0)
		{
			Label = LOCTEXT("BgLayer", "Background layer");
			bRequired = true;
		}
		else if (i == 1)
		{
			Label = LOCTEXT("Fg1Layer", "Foreground layer1");
			bRequired = true;
		}
		else
		{
			Label = LOCTEXT("Fg2Layer", "Foreground layer2");
			bRequired = false;
		}

		Box->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 4.f)
		[
			BuildLayerRow(i, Label, bRequired)
		];
	}
	ManualLayersContainer->SetContent(Box);
}

// ---------------------------------------------------------------
// Layer row (thumbnail + name + Upload Image button)
// ---------------------------------------------------------------
TSharedRef<SWidget> SIconConfiguratorWidget::BuildLayerRow(
	int32 LayerIndex, const FText& Label, bool bRequired)
{
	// Name + required asterisk row
	TSharedRef<SHorizontalBox> NameRow = SNew(SHorizontalBox);
	NameRow->AddSlot().AutoWidth().VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.Text(Label)
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	];
	if (bRequired)
	{
		NameRow->AddSlot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT(" *")))
			.ColorAndOpacity(FLinearColor::Red)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
		];
	}

	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
	.ToolTipText(Label)

	// Thumbnail 60×60
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	.Padding(0.f, 0.f, 10.f, 0.f)
	[
		SNew(SBox)
		.WidthOverride(60.f)
		.HeightOverride(60.f)
		[
			SNew(SOverlay)
			// Checkerboard background
			+ SOverlay::Slot()
			[
				SNew(SThumbnailCheckerboard)
			]
			// Texture layer (shown when an image is loaded)
			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(TAttribute<const FSlateBrush*>::CreateSP(
					this, &SIconConfiguratorWidget::GetThumbnailBrush, LayerIndex))
			]
		]
	]

	// Layer name (centered)
	+ SHorizontalBox::Slot()
	.FillWidth(1.f)
	.VAlign(VAlign_Center)
	[
		NameRow
	]

	// Upload Image button
	+ SHorizontalBox::Slot()
	.AutoWidth()
	.VAlign(VAlign_Center)
	[
		SNew(SButton)
		.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
		.ContentPadding(FMargin(4.f, 2.f))
		.OnClicked(FOnClicked::CreateSP(this, &SIconConfiguratorWidget::OnPickFile, LayerIndex))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("UploadImage", "\u2191  Upload Image"))
			.ColorAndOpacity(FSlateColor(kBlue))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
		]
	];

	// Move Up button (hidden for first layer)
	if (LayerIndex > 0)
	{
		Row->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.f, 0.f, 0.f, 0.f)
		[
			SNew(SButton)
			.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.ContentPadding(FMargin(2.f, 0.f))
			.ToolTipText(LOCTEXT("MoveUp", "Move layer up"))
			.OnClicked(FOnClicked::CreateSP(this, &SIconConfiguratorWidget::OnMoveLayerUp, LayerIndex))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("\u25B2")))
				.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			]
		];
	}

	// Move Down button (hidden for last layer)
	if (LayerIndex < Layers.Num() - 1)
	{
		Row->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(2.f, 0.f, 0.f, 0.f)
		[
			SNew(SButton)
			.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.ContentPadding(FMargin(2.f, 0.f))
			.ToolTipText(LOCTEXT("MoveDown", "Move layer down"))
			.OnClicked(FOnClicked::CreateSP(this, &SIconConfiguratorWidget::OnMoveLayerDown, LayerIndex))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("\u25BC")))
				.ColorAndOpacity(FLinearColor(0.5f, 0.5f, 0.5f))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			]
		];
	}

	// Delete button for optional layers
	if (!bRequired)
	{
		Row->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.ContentPadding(FMargin(4.f, 0.f))
			.OnClicked(FOnClicked::CreateSP(this, &SIconConfiguratorWidget::OnRemoveLayer, LayerIndex))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("\u00D7")))
				.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			]
		];
	}

	return Row;
}

// ---------------------------------------------------------------
// ② AI Split Tab content
// ---------------------------------------------------------------
TSharedRef<SWidget> SIconConfiguratorWidget::BuildAISplitContent()
{
	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox)

	// Description text
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 8.f)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("AISplitDesc", "Upload flat image. AI will split it automatically."))
		.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		.AutoWrapText(true)
	]

	// Image row (thumbnail + Upload Image)
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 10.f)
	[
		SNew(SHorizontalBox)
		// Thumbnail
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 10.f, 0.f)
		[
			SNew(SBox)
			.WidthOverride(60.f)
			.HeightOverride(60.f)
			[
				SNew(SOverlay)
				// Checkerboard background (same as Manual layer thumbnails)
				+ SOverlay::Slot()
				[
					SNew(SThumbnailCheckerboard)
				]
				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(TAttribute<const FSlateBrush*>::CreateSP(
						this, &SIconConfiguratorWidget::GetAISplitBrush))
				]
			]
		]
		+ SHorizontalBox::Slot().FillWidth(1.f)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.ContentPadding(FMargin(4.f, 2.f))
			.OnClicked(this, &SIconConfiguratorWidget::OnPickAISplitFile)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("UploadImageAI", "\u2191  Upload Image"))
				.ColorAndOpacity(FSlateColor(kBlue))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
			]
		]
	]

	// I agree checkbox
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 8.f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 6.f, 0.f)
		[
			SNew(SCheckBox)
			.OnCheckStateChanged(this, &SIconConfiguratorWidget::OnAISplitAgreedChanged)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("IAgree", "I agree"))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
		]
	]

	// Generate button
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 10.f)
	[
		SNew(SButton)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.ButtonColorAndOpacity(FSlateColor(kBlue))
		.ContentPadding(FMargin(0.f, 10.f))
		.IsEnabled(TAttribute<bool>::CreateSP(this, &SIconConfiguratorWidget::IsGenerateEnabled))
		.OnClicked(this, &SIconConfiguratorWidget::OnGenerateClicked)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Generate", "Generate"))
			.ColorAndOpacity(FLinearColor::White)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		]
	]

	// Progress bar
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 4.f)
	[
		SNew(SBox)
		.Visibility(TAttribute<EVisibility>::CreateSP(this, &SIconConfiguratorWidget::GetAISplitProgressVis))
		[
			SNew(SProgressBar)
			.Percent(TAttribute<TOptional<float>>::CreateSP(this, &SIconConfiguratorWidget::GetAISplitProgressVal))
		]
	]

	// Status text
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 4.f)
	[
		SNew(STextBlock)
		.Visibility(TAttribute<EVisibility>::CreateSP(this, &SIconConfiguratorWidget::GetAISplitProgressVis))
		.Text(TAttribute<FText>::CreateSP(this, &SIconConfiguratorWidget::GetAISplitStatusText))
		.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		.AutoWrapText(true)
	]

	// Error text
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 4.f)
	[
		SNew(STextBlock)
		.Visibility(TAttribute<EVisibility>::CreateSP(this, &SIconConfiguratorWidget::GetAISplitErrorVis))
		.Text(TAttribute<FText>::CreateLambda([this]() { return FText::FromString(AISplitErrorMsg); }))
		.ColorAndOpacity(FLinearColor(1.f, 0.3f, 0.3f))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
		.AutoWrapText(true)
	]

	// Disclaimer text box
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.15f, 0.15f, 0.15f))
		.Padding(FMargin(8.f, 6.f))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Disclaimer",
				"The AI auto-layering function is driven by algorithms, and the "
				"generated results may have deviations. You can use manual layering "
				"at any time to ensure the desired effect."))
			.ColorAndOpacity(FLinearColor(0.55f, 0.55f, 0.55f))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			.AutoWrapText(true)
		]
	];

	// AI result layers (populated after Generate completes, read-only thumbnails)
	Box->AddSlot()
	.AutoHeight()
	.Padding(0.f, 10.f, 0.f, 0.f)
	[
		SAssignNew(AIResultLayersContainer, SBox)
	];

	return Box;
}

// ---------------------------------------------------------------
// ③ Preview section (single viewport; LayerData pointer swapped on tab switch)
// ---------------------------------------------------------------
TSharedRef<SWidget> SIconConfiguratorWidget::BuildPreviewSection()
{
	return SNew(SVerticalBox)

	// "Preview" title
	+ SVerticalBox::Slot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 6.f)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("Preview", "Preview"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
	]

	// 2D + 3D side by side
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		SNew(SBox)
		.HeightOverride(240.f)
		[
			SNew(SHorizontalBox)

			// 2D preview (left)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(0.f, 0.f, 4.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("2DPreview", "2D Preview"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f))
					.Margin(FMargin(0.f, 0.f, 0.f, 2.f))
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SAssignNew(Viewport2D, SIconConfigurator2DViewport)
					.LayerData(&Layers)
				]
			]

			// 3D preview (right)
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(4.f, 0.f, 0.f, 0.f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("3DSpatial", "3D Spatial"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
					.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f))
					.Margin(FMargin(0.f, 0.f, 0.f, 2.f))
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.f)
				[
					SAssignNew(Viewport3D, SIconConfigurator3DViewport)
					.LayerData(&Layers)
				]
			]
		]
	];
}

// ---------------------------------------------------------------
// ④ Localization section
// ---------------------------------------------------------------
TSharedRef<SWidget> SIconConfiguratorWidget::BuildLocalizationSection()
{
	TSharedRef<SVerticalBox> Section = SNew(SVerticalBox);

	// Title row + "+" button
	Section->AddSlot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 6.f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Localization", "Localization"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.ContentPadding(FMargin(6.f, 2.f))
			.IsEnabled_Lambda([this]() { return LocEntries.Num() < LangCodes.Num() + 1; })
			.OnClicked(this, &SIconConfiguratorWidget::OnAddLocalization)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("+")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
			]
		]
	];

	// Dynamic entry container
	Section->AddSlot()
	.AutoHeight()
	[
		SAssignNew(LocEntriesContainer, SBox)
	];

	// Populate initial entries
	RebuildLocEntries();

	return Section;
}

// ---------------------------------------------------------------
// Localization row
// ---------------------------------------------------------------
TSharedRef<SWidget> SIconConfiguratorWidget::BuildLocRow(
	TSharedPtr<FLocEntry> Entry, bool bIsDefault)
{
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);

	// Language column: default English uses static text, others use ComboBox
	if (bIsDefault)
	{
		Row->AddSlot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 8.f, 0.f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(Entry->LangDisplay))
			.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
		];
	}
	else
	{
		// Only set default language for brand-new entries (preserve existing selection on rebuild)
		if (Entry->LangCode.IsEmpty() && LangDisplayOptions.Num() > 0)
		{
			// Find the first code not already used by other entries
			FString DefaultCode    = LangCodes[0];
			FString DefaultDisplay = *LangDisplayOptions[0];
			for (int32 k = 0; k < LangCodes.Num(); ++k)
			{
				bool bUsed = false;
				for (const TSharedPtr<FLocEntry>& Other : LocEntries)
				{
					if (Other != Entry && Other->LangCode == LangCodes[k])
					{
						bUsed = true;
						break;
					}
				}
				if (!bUsed)
				{
					DefaultCode    = LangCodes[k];
					DefaultDisplay = *LangDisplayOptions[k];
					break;
				}
			}
			Entry->LangCode    = DefaultCode;
			Entry->LangDisplay = DefaultDisplay;
		}

		// Compute currently-selected item pointer for this entry
		TSharedPtr<FString> CurrentItem;
		for (int32 k = 0; k < LangCodes.Num(); ++k)
		{
			if (LangCodes[k] == Entry->LangCode)
			{
				CurrentItem = LangDisplayOptions[k];
				break;
			}
		}

		Row->AddSlot()
		.FillWidth(1.f)
		.VAlign(VAlign_Center)
		.Padding(0.f, 0.f, 8.f, 0.f)
		[
			SNew(SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&LangDisplayOptions)
			.InitiallySelectedItem(CurrentItem)
			.OnSelectionChanged_Lambda([this, Entry](TSharedPtr<FString> Selected, ESelectInfo::Type)
			{
				if (!Selected.IsValid()) return;
				int32 Idx = LangDisplayOptions.IndexOfByPredicate(
					[&](const TSharedPtr<FString>& S){ return S == Selected; });
				if (!LangCodes.IsValidIndex(Idx)) return;
				// Reject duplicate language selection
				const FString& NewCode = LangCodes[Idx];
				for (const TSharedPtr<FLocEntry>& Other : LocEntries)
				{
					if (Other != Entry && Other->LangCode == NewCode)
					{
						FNotificationInfo Info(LOCTEXT("DuplicateLang", "This language is already added."));
						Info.ExpireDuration = 3.f;
						Info.bFireAndForget = true;
						FSlateNotificationManager::Get().AddNotification(Info);
						return;
					}
				}
				Entry->LangCode    = NewCode;
				Entry->LangDisplay = *Selected;
				SaveState();
			})
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item) -> TSharedRef<SWidget>
			{
				return SNew(STextBlock)
					.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
					.Margin(FMargin(4.f, 2.f));
			})
			[
				SNew(STextBlock)
				.Text_Lambda([Entry]() { return FText::FromString(Entry->LangDisplay); })
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
			]
		];
	}

	// App name input field
	Row->AddSlot()
	.FillWidth(1.f)
	.VAlign(VAlign_Center)
	.Padding(0.f, 0.f, bIsDefault ? 0.f : 6.f, 0.f)
	[
		SNew(SEditableTextBox)
		.HintText(LOCTEXT("AppNameHint", "App name"))
		.Text_Lambda([Entry]() { return FText::FromString(*Entry->AppName); })
		.OnTextChanged_Lambda([this, Entry](const FText& NewText)
		{
			FString Str = NewText.ToString();
			if (Str.Len() > 50) Str = Str.Left(50);
			*Entry->AppName = Str;
			SaveState();
		})
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
	];

	// Delete button (non-default rows only)
	if (!bIsDefault)
	{
		Row->AddSlot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ButtonStyle(&FAppStyle::Get().GetWidgetStyle<FButtonStyle>("NoBorder"))
			.ContentPadding(FMargin(4.f, 0.f))
			.OnClicked_Lambda([this, Entry]() -> FReply
			{
				LocEntries.Remove(Entry);
				RebuildLocEntries();
				SaveState();
				return FReply::Handled();
			})
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("\u00D7")))
				.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.6f))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			]
		];
	}

	return SNew(SBox)
	.Padding(FMargin(0.f, 2.f))
	[ Row ];
}

void SIconConfiguratorWidget::RebuildLocEntries()
{
	if (!LocEntriesContainer.IsValid()) return;

	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	for (int32 i = 0; i < LocEntries.Num(); ++i)
	{
		Box->AddSlot()
		.AutoHeight()
		[
			BuildLocRow(LocEntries[i], i == 0)
		];
	}
	LocEntriesContainer->SetContent(Box);
}

// ---------------------------------------------------------------
// Slate attribute binding helpers
// ---------------------------------------------------------------
const FSlateBrush* SIconConfiguratorWidget::GetThumbnailBrush(int32 LayerIndex) const
{
	// Use TWeakObjectPtr validity as the authoritative check: the raw pointer stored
	// in FSlateBrush can become dangling after a GC pass (e.g. during Android packaging),
	// while the weak ptr is always safely updated by the GC.
	static FSlateBrush NoBrush;
	NoBrush.DrawAs = ESlateBrushDrawType::NoDrawType;

	if (LayerIndex < 0 || LayerIndex >= Layers.Num()) return &NoBrush;
	if (!Layers[LayerIndex].PreviewTexture.IsValid())  return &NoBrush;

	return &ThumbnailBrushes[LayerIndex];
}

const FSlateBrush* SIconConfiguratorWidget::GetAISplitBrush() const
{
	if (AISplitFilePath.IsEmpty())
	{
		static FSlateBrush TransparentBrush;
		TransparentBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
		return &TransparentBrush;
	}
	return &AISplitThumbnailBrush;
}

FVector2D SIconConfiguratorWidget::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	constexpr float DesiredWidth = 420.f;
	return FVector2D(DesiredWidth, DesiredWidth * 2.f);
}

FSlateColor SIconConfiguratorWidget::GetTabButtonColor(int32 TabIndex) const
{
	return FSlateColor((ActiveTab == TabIndex) ? kBlue : kTabInactive);
}

FSlateColor SIconConfiguratorWidget::GetTabTextColor(int32 TabIndex) const
{
	return FSlateColor((ActiveTab == TabIndex)
		? FLinearColor::White
		: FLinearColor(0.6f, 0.6f, 0.6f));
}

// ---------------------------------------------------------------
// Callbacks
// ---------------------------------------------------------------

FReply SIconConfiguratorWidget::OnTabSwitch(int32 TabIndex)
{
	ActiveTab = TabIndex;
	if (TabSwitcher.IsValid())
	{
		TabSwitcher->SetActiveWidgetIndex(TabIndex);
	}

	// Swap shared viewport data pointer to match the active tab
	TArray<FIconLayerData>* TargetLayers = (TabIndex == 0) ? &Layers : &AILayers;
	if (Viewport2D.IsValid()) Viewport2D->SetLayerData(TargetLayers);
	if (Viewport3D.IsValid()) Viewport3D->SetLayerData(TargetLayers);
	RefreshViewports();

	Invalidate(EInvalidateWidgetReason::Paint);
	SaveState();
	return FReply::Handled();
}

FReply SIconConfiguratorWidget::OnPickFile(int32 LayerIndex)
{
	IDesktopPlatform* DP = FDesktopPlatformModule::Get();
	if (!DP) return FReply::Handled();

	TArray<FString> OutFiles;
	const FString Filter = TEXT("Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.tga)|*.png;*.jpg;*.jpeg;*.bmp;*.tga");
	const void* ParentWnd = FSlateApplication::Get().GetActiveTopLevelWindow().IsValid()
		? FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle()
		: nullptr;

	if (DP->OpenFileDialog(ParentWnd, TEXT("Select Image"),
		FPaths::GetPath(Layers[LayerIndex].FilePath),
		TEXT(""), Filter, EFileDialogFlags::None, OutFiles)
		&& OutFiles.Num() > 0)
	{
		constexpr int64 MaxBytes = 4 * 1024 * 1024;
		if (IFileManager::Get().FileSize(*OutFiles[0]) > MaxBytes)
		{
			FNotificationInfo Info(LOCTEXT("FileTooLarge", "File size must be \u2264 4 MB."));
			Info.ExpireDuration = 3.f;
			Info.bFireAndForget = true;
			FSlateNotificationManager::Get().AddNotification(Info);
			return FReply::Handled();
		}
		{
			UTexture2D* TestTex = FImageUtils::ImportFileAsTexture2D(OutFiles[0]);
			if (TestTex && TestTex->GetSizeX() != TestTex->GetSizeY())
			{
				FNotificationInfo Info(LOCTEXT("AspectRatioError", "Image must have a 1:1 aspect ratio (width == height)."));
				Info.ExpireDuration = 3.f;
				Info.bFireAndForget = true;
				FSlateNotificationManager::Get().AddNotification(Info);
				return FReply::Handled();
			}
		}
		Layers[LayerIndex].FilePath = OutFiles[0];
		LoadPreviewTexture(LayerIndex);
		UpdateThumbnail(LayerIndex);
		RefreshViewports();
		SaveState();
	}
	return FReply::Handled();
}

FReply SIconConfiguratorWidget::OnPickAISplitFile()
{
	IDesktopPlatform* DP = FDesktopPlatformModule::Get();
	if (!DP) return FReply::Handled();

	TArray<FString> OutFiles;
	const FString Filter = TEXT("Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.tga)|*.png;*.jpg;*.jpeg;*.bmp;*.tga");
	const void* ParentWnd = FSlateApplication::Get().GetActiveTopLevelWindow().IsValid()
		? FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle()
		: nullptr;

	if (DP->OpenFileDialog(ParentWnd, TEXT("Select Image"),
		FPaths::GetPath(AISplitFilePath),
		TEXT(""), Filter, EFileDialogFlags::None, OutFiles)
		&& OutFiles.Num() > 0)
	{
		constexpr int64 MaxBytes = 4 * 1024 * 1024;
		if (IFileManager::Get().FileSize(*OutFiles[0]) > MaxBytes)
		{
			FNotificationInfo Info(LOCTEXT("FileTooLargeAI", "File size must be \u2264 4 MB."));
			Info.ExpireDuration = 3.f;
			Info.bFireAndForget = true;
			FSlateNotificationManager::Get().AddNotification(Info);
			return FReply::Handled();
		}

		AISplitFilePath = OutFiles[0];
		// Reset state so user can generate again with the new file
		AISplitState    = EAISplitState::Idle;
		AISplitErrorMsg.Empty();

		// Remove old texture from GC root before replacing
		if (UTexture2D* OldTex = Cast<UTexture2D>(AISplitThumbnailBrush.GetResourceObject()))
		{
			OldTex->RemoveFromRoot();
		}

		UTexture2D* Tex = FImageUtils::ImportFileAsTexture2D(AISplitFilePath);
		if (Tex)
		{
			if (Tex->GetSizeX() != Tex->GetSizeY())
			{
				AISplitFilePath.Empty();
				FNotificationInfo Info(LOCTEXT("AspectRatioErrorAI", "Image must have a 1:1 aspect ratio (width == height)."));
				Info.ExpireDuration = 3.f;
				Info.bFireAndForget = true;
				FSlateNotificationManager::Get().AddNotification(Info);
				return FReply::Handled();
			}
			if (Tex->GetSizeX() > 1024)
			{
				AISplitFilePath.Empty();
				FNotificationInfo Info(LOCTEXT("ResolutionErrorAI", "Image resolution must not exceed 1024x1024."));
				Info.ExpireDuration = 3.f;
				Info.bFireAndForget = true;
				FSlateNotificationManager::Get().AddNotification(Info);
				return FReply::Handled();
			}
			Tex->SetFlags(RF_Transient);
			Tex->AddToRoot(); // Prevent GC — FSlateBrush in non-UObject doesn't count as a reference
			AISplitThumbnailBrush.SetResourceObject(Tex);
			AISplitThumbnailBrush.ImageSize = FVector2D(
				static_cast<float>(Tex->GetSizeX()),
				static_cast<float>(Tex->GetSizeY()));
		}
	}
	return FReply::Handled();
}

FReply SIconConfiguratorWidget::OnAddLocalization()
{
	if (LocEntries.Num() >= LangCodes.Num() + 1)
	{
		FNotificationInfo Info(LOCTEXT("AllSupportedLangsAdded", "All supported languages are already added."));
		Info.ExpireDuration = 3.f;
		Info.bFireAndForget = true;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	TSharedPtr<FLocEntry> NewEntry = MakeShared<FLocEntry>();
	LocEntries.Add(NewEntry);
	RebuildLocEntries();
	SaveState();
	return FReply::Handled();
}

FReply SIconConfiguratorWidget::OnAddLayer()
{
	if (Layers.Num() >= kMaxLayers) return FReply::Handled();

	const int32 NewIdx = Layers.Num();
	Layers.SetNum(NewIdx + 1);
	ThumbnailBrushes[NewIdx] = FSlateBrush();
	ThumbnailBrushes[NewIdx].DrawAs = ESlateBrushDrawType::Image;

	RebuildManualLayers();
	RefreshViewports();
	SaveState();
	return FReply::Handled();
}

FReply SIconConfiguratorWidget::OnRemoveLayer(int32 LayerIndex)
{
	if (Layers.Num() <= kMinLayers) return FReply::Handled();

	// Release the rooted texture before clearing
	if (Layers[LayerIndex].PreviewTexture.IsValid())
	{
		UTexture2D* OldTex = Layers[LayerIndex].PreviewTexture.Get();
		if (OldTex->IsRooted()) OldTex->RemoveFromRoot();
	}

	// Clear data for the removed layer
	Layers[LayerIndex].FilePath.Empty();
	Layers[LayerIndex].PreviewTexture = nullptr;
	Layers[LayerIndex].ImportedTexture = nullptr;
	ThumbnailBrushes[LayerIndex].SetResourceObject(nullptr);
	ThumbnailBrushes[LayerIndex].ImageSize = FVector2D::ZeroVector;

	// Shift layers down if needed (remove from middle)
	if (LayerIndex < Layers.Num() - 1)
	{
		for (int32 i = LayerIndex; i < Layers.Num() - 1; ++i)
		{
			Layers[i] = Layers[i + 1];
			ThumbnailBrushes[i] = ThumbnailBrushes[i + 1];
		}
	}

	// Clear the last slot and shrink
	const int32 LastIdx = Layers.Num() - 1;
	Layers[LastIdx] = FIconLayerData();
	ThumbnailBrushes[LastIdx] = FSlateBrush();
	ThumbnailBrushes[LastIdx].DrawAs = ESlateBrushDrawType::Image;
	Layers.SetNum(Layers.Num() - 1);

	RebuildManualLayers();
	RefreshViewports();
	SaveState();
	return FReply::Handled();
}

FReply SIconConfiguratorWidget::OnMoveLayerUp(int32 LayerIndex)
{
	if (LayerIndex <= 0 || LayerIndex >= Layers.Num()) return FReply::Handled();

	// Swap layer data
	Layers.Swap(LayerIndex, LayerIndex - 1);

	// Swap thumbnail brushes
	FSlateBrush TempBrush = ThumbnailBrushes[LayerIndex];
	ThumbnailBrushes[LayerIndex] = ThumbnailBrushes[LayerIndex - 1];
	ThumbnailBrushes[LayerIndex - 1] = TempBrush;

	RebuildManualLayers();
	RefreshViewports();
	SaveState();
	return FReply::Handled();
}

FReply SIconConfiguratorWidget::OnMoveLayerDown(int32 LayerIndex)
{
	if (LayerIndex < 0 || LayerIndex >= Layers.Num() - 1) return FReply::Handled();

	// Swap layer data
	Layers.Swap(LayerIndex, LayerIndex + 1);

	// Swap thumbnail brushes
	FSlateBrush TempBrush = ThumbnailBrushes[LayerIndex];
	ThumbnailBrushes[LayerIndex] = ThumbnailBrushes[LayerIndex + 1];
	ThumbnailBrushes[LayerIndex + 1] = TempBrush;

	RebuildManualLayers();
	RefreshViewports();
	SaveState();
	return FReply::Handled();
}

FReply SIconConfiguratorWidget::OnApply()
{
	// Route to the active tab's layer data
	const TArray<FIconLayerData>& SrcLayers = (ActiveTab == 0) ? Layers : AILayers;

	auto ShowNotify = [](const FText& Msg)
	{
		FNotificationInfo Info(Msg);
		Info.ExpireDuration  = 3.f;
		Info.bFireAndForget  = true;
		FSlateNotificationManager::Get().AddNotification(Info);
	};

	// Validate: Background (index 0) and Foreground 1 (index 1) must both have images
	if (SrcLayers.Num() < 2 || SrcLayers[0].FilePath.IsEmpty() || SrcLayers[1].FilePath.IsEmpty())
	{
		ShowNotify(LOCTEXT("ApplyErrLayers",
			"Background layer and Foreground 1 layer are required."));
		return FReply::Handled();
	}

	// Validate: English app name is required before any file I/O
	if (LocEntries.Num() > 0 && LocEntries[0].IsValid() && LocEntries[0]->AppName->IsEmpty())
	{
		ShowNotify(LOCTEXT("EnglishNameRequired", "English app name is required."));
		return FReply::Handled();
	}

	const FString ResDir      = FPaths::ProjectDir() / TEXT("Build/Android/res");
	const FString DrawableDir = ResDir / TEXT("drawable");
	const FString MipmapDir   = ResDir / TEXT("mipmap-xxxhdpi");
	const FString ValuesDir   = ResDir / TEXT("values");
	const FString XmlDir      = ResDir / TEXT("xml");

	IFileManager& FM = IFileManager::Get();
	FM.MakeDirectory(*DrawableDir, true);
	FM.MakeDirectory(*MipmapDir,   true);
	FM.MakeDirectory(*ValuesDir,   true);
	FM.MakeDirectory(*XmlDir,      true);

	// Per-layer processing: decode → squarify → write layer PNG → generate SDF → write SDF PNG
	TArray<FString>    LayerResNames;   // file names use real layer index; index=0 is always the background
	TArray<FString>    SdfResNames;
	TArray<FRgbaImage> SquaredLayers;   // used to composite the launcher icon
	TArray<float>      LayerOpacities;
	int32 FailedCount = 0;

	for (int32 i = 0; i < SrcLayers.Num(); ++i)
	{
		if (SrcLayers[i].FilePath.IsEmpty()) continue;

		FRgbaImage Src;
		if (!IconImagePipeline::DecodeFileToRgba(SrcLayers[i].FilePath, Src))
		{
			++FailedCount;
			continue;
		}

		const FRgbaImage Squared = IconImagePipeline::ToSquare(Src);

		const FString LayerName = FString::Printf(TEXT("icon_3d_layer_%d"), i);
		const FString SdfName   = FString::Printf(TEXT("icon_3d_sdf_%d"),   i);

		IconImagePipeline::EncodeRgbaToPngFile(
			Squared, DrawableDir / (LayerName + TEXT(".png")));

		const FRgbaImage Sdf = IconImagePipeline::GenerateSdfDefault(Squared);
		IconImagePipeline::EncodeRgbaToPngFile(
			Sdf, DrawableDir / (SdfName + TEXT(".png")));

		LayerResNames.Add(LayerName);
		SdfResNames.Add(SdfName);
		SquaredLayers.Add(Squared);
		LayerOpacities.Add(SrcLayers[i].Opacity);
	}

	if (LayerResNames.Num() < 2)
	{
		ShowNotify(LOCTEXT("ApplyErrDecode",
			"Failed to decode enough layer images (need 2+)."));
		return FReply::Handled();
	}

	// Composite 2D preview image → mipmap-xxxhdpi/ic_spatial_launcher.png
	const int32 OutSize = (SquaredLayers.Num() > 0) ? SquaredLayers[0].Width : 512;
	const FRgbaImage Launcher = IconImagePipeline::CompositeLayers(
		SquaredLayers, LayerOpacities, OutSize);
	IconImagePipeline::EncodeRgbaToPngFile(
		Launcher, MipmapDir / TEXT("ic_spatial_launcher.png"));

	// Generate drawables_3d.xml: icon_sdf_list + icon_3d_list, both in reverse order (top layer first)
	FString Xml = TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<resources>\n");

	Xml += TEXT("    <array name=\"icon_sdf_list\">\n");
	for (int32 k = SdfResNames.Num() - 1; k >= 0; --k)
	{
		Xml += FString::Printf(TEXT("        <item>@drawable/%s</item>\n"), *SdfResNames[k]);
	}
	Xml += TEXT("    </array>\n");

	Xml += TEXT("    <array name=\"icon_3d_list\">\n");
	for (int32 k = LayerResNames.Num() - 1; k >= 0; --k)
	{
		Xml += FString::Printf(TEXT("        <item>@drawable/%s</item>\n"), *LayerResNames[k]);
	}
	Xml += TEXT("    </array>\n</resources>\n");

	FFileHelper::SaveStringToFile(
		Xml, *(ValuesDir / TEXT("drawables_3d.xml")),
		FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

	// Strings required by the OBB downloader (from Engine/Build/Android/Java/res/values/strings.xml).
	// UE packaging overwrites the engine's file with the project's res/, so our values/strings.xml
	// must include these strings; otherwise the downloader_progress.xml layout cannot find the resources.
	// Non-default language dirs (values-zh-rCN, etc.) only need to override app_name;
	// the rest falls back to the default values/ directory.
	static const TCHAR* kObbDownloaderStrings =
		TEXT("    <string name=\"notification_download_complete\">Download complete</string>\n")
		TEXT("    <string name=\"notification_download_failed\">Download unsuccessful</string>\n")
		TEXT("    <string name=\"state_unknown\">Starting...</string>\n")
		TEXT("    <string name=\"state_idle\">Waiting for download to start</string>\n")
		TEXT("    <string name=\"state_fetching_url\">Looking for resources to download</string>\n")
		TEXT("    <string name=\"state_connecting\">Connecting to the download server</string>\n")
		TEXT("    <string name=\"state_downloading\">Downloading resources</string>\n")
		TEXT("    <string name=\"state_completed\">Download finished</string>\n")
		TEXT("    <string name=\"state_paused_network_unavailable\">Download paused because no network is available</string>\n")
		TEXT("    <string name=\"state_paused_network_setup_failure\">Download paused. Test a website in browser</string>\n")
		TEXT("    <string name=\"state_paused_by_request\">Download paused</string>\n")
		TEXT("    <string name=\"state_paused_wifi_unavailable\">Download paused because wifi is unavailable</string>\n")
		TEXT("    <string name=\"state_paused_wifi_disabled\">Download paused because wifi is disabled</string>\n")
		TEXT("    <string name=\"state_paused_roaming\">Download paused because you are roaming</string>\n")
		TEXT("    <string name=\"state_paused_sdcard_unavailable\">Download paused because the external storage is unavailable</string>\n")
		TEXT("    <string name=\"state_failed_unlicensed\">Download failed because you may not have purchased this app</string>\n")
		TEXT("    <string name=\"state_failed_fetching_url\">Download failed because the resources could not be found</string>\n")
		TEXT("    <string name=\"state_failed_sdcard_full\">Download failed because the external storage is full</string>\n")
		TEXT("    <string name=\"state_failed_cancelled\">Download cancelled</string>\n")
		TEXT("    <string name=\"state_failed\">Download failed</string>\n")
		TEXT("    <string name=\"kilobytes_per_second\">%1$s KB/s</string>\n")
		TEXT("    <string name=\"time_remaining\">Time remaining: %1$s</string>\n")
		TEXT("    <string name=\"time_remaining_notification\">%1$s left</string>\n")
		TEXT("    <string name=\"text_paused_cellular\">Would you like to enable downloading over cellular connections?</string>\n")
		TEXT("    <string name=\"text_paused_cellular_2\">If you choose not to enable downloading over cellular connections, you can resume the download later.</string>\n")
		TEXT("    <string name=\"text_button_resume_cellular\">Resume download</string>\n")
		TEXT("    <string name=\"text_button_wifi_settings\">Wi-Fi settings</string>\n")
		TEXT("    <string name=\"text_verifying_download\">Verifying Download</string>\n")
		TEXT("    <string name=\"text_validation_complete\">XAPK File Validation Complete.</string>\n")
		TEXT("    <string name=\"text_validation_failed\">XAPK File Validation Failed.</string>\n")
		TEXT("    <string name=\"text_button_pause\">Pause Download</string>\n")
		TEXT("    <string name=\"text_button_resume\">Resume Download</string>\n")
		TEXT("    <string name=\"text_button_cancel\">Cancel</string>\n")
		TEXT("    <string name=\"text_button_cancel_verify\">Cancel Verification</string>\n");

	// Delete all pre-existing non-default language dirs so removed entries don't linger on disk
	FM.IterateDirectory(*ResDir, [&FM](const TCHAR* Path, bool bIsDirectory) -> bool
	{
		if (bIsDirectory && FPaths::GetCleanFilename(Path).StartsWith(TEXT("values-")))
		{
			FM.DeleteDirectory(Path, false, true);
		}
		return true;
	});

	// Purge stale language resources from Gradle staging and build caches
	auto PurgeValuesSubdirs = [&FM](const FString& ParentDir)
	{
		if (!FM.DirectoryExists(*ParentDir)) return;
		FM.IterateDirectory(*ParentDir, [&FM](const TCHAR* Path, bool bIsDirectory) -> bool
		{
			if (bIsDirectory && FPaths::GetCleanFilename(Path).StartsWith(TEXT("values-")))
			{
				FM.DeleteDirectory(Path, false, true);
			}
			return true;
		});
	};

	const FString AndroidIntDir = FPaths::ProjectDir() / TEXT("Intermediate/Android/arm64");
	const FString GradleAppDir = AndroidIntDir / TEXT("gradle/app");

	// UE intermediate res staging (source for Gradle copy)
	PurgeValuesSubdirs(AndroidIntDir / TEXT("res"));
	// Gradle main app staging res
	PurgeValuesSubdirs(GradleAppDir / TEXT("src/main/res"));
	// AFS (Android File Server) project staging res
	PurgeValuesSubdirs(AndroidIntDir / TEXT("gradle/AFSProject/app/src/main/res"));

	// Gradle build intermediates caches
	const FString GradleIntermediatesDir = GradleAppDir / TEXT("build/intermediates");
	if (FM.DirectoryExists(*GradleIntermediatesDir))
	{
		auto PurgeValuesRecursive = [&FM](const FString& ParentDir)
		{
			FM.IterateDirectoryRecursively(*ParentDir, [&FM](const TCHAR* Path, bool bIsDirectory) -> bool
			{
				if (bIsDirectory && FPaths::GetCleanFilename(Path).StartsWith(TEXT("values-")))
				{
					FM.DeleteDirectory(Path, false, true);
				}
				return true;
			});
		};
		const FString MergedResDir = GradleIntermediatesDir / TEXT("incremental/debug/mergeDebugResources/merged.dir");
		const FString PackagedResDir = GradleIntermediatesDir / TEXT("packaged_res/debug/packageDebugResources");
		const FString PkgMergedDir = GradleIntermediatesDir / TEXT("incremental/debug/packageDebugResources/merged.dir");
		PurgeValuesRecursive(MergedResDir);
		PurgeValuesRecursive(PackagedResDir);
		PurgeValuesRecursive(PkgMergedDir);
	}

	TArray<FString> WrittenBcp47Tags;

	for (const TSharedPtr<FLocEntry>& Entry : LocEntries)
	{
		if (!Entry.IsValid() || Entry->AppName->IsEmpty()) continue;

		// LangCode is a BCP-47 tag (default row is empty = en-US base)
		const FString Bcp47 = Entry->LangCode.IsEmpty() ? TEXT("en-US") : Entry->LangCode;
		const FString Qualifier = IconImagePipeline::Bcp47ToAndroidQualifier(Entry->LangCode);
		const bool bIsDefault = Qualifier.IsEmpty();

		const FString LangDir = bIsDefault
			? ValuesDir
			: ResDir / (TEXT("values-") + Qualifier);
		FM.MakeDirectory(*LangDir, true);

		// Default language directory must include OBB downloader strings to avoid losing them when overwriting the engine file
		FString StringsXml = FString::Printf(
			TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<resources>\n")
			TEXT("    <string name=\"app_name\">%s</string>\n"),
			**Entry->AppName);
		if (bIsDefault)
		{
			StringsXml += kObbDownloaderStrings;
		}
		StringsXml += TEXT("</resources>\n");

		FFileHelper::SaveStringToFile(
			StringsXml, *(LangDir / TEXT("strings.xml")),
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);

		WrittenBcp47Tags.AddUnique(Bcp47);
	}

	// Generate res/xml/locales_config.xml (BCP-47 tags)
	if (WrittenBcp47Tags.Num() > 0)
	{
		FString LocXml = TEXT("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
			TEXT("<locale-config xmlns:android=\"http://schemas.android.com/apk/res/android\">\n");
		for (const FString& LocaleTag : WrittenBcp47Tags)
		{
			LocXml += FString::Printf(TEXT("    <locale android:name=\"%s\"/>\n"), *LocaleTag);
		}
		LocXml += TEXT("</locale-config>\n");

		FFileHelper::SaveStringToFile(
			LocXml, *(XmlDir / TEXT("locales_config.xml")),
			FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}

	UE_LOG(LogTemp, Log, TEXT("IconPreviewTool: Applied to %s"), *ResDir);
	ShowNotify(LOCTEXT("ApplySuccess", "Icon configuration applied successfully."));

	// Verify UPL file exists (located inside the plugin's Resources folder)
	const FString UplPath = FPaths::Combine(
		IPluginManager::Get().FindPlugin(TEXT("IconConfigurator"))->GetBaseDir(),
		TEXT("Resources/IconConfigurator_UPL_Android.xml"));
	if (!FPaths::FileExists(UplPath))
	{
		FNotificationInfo WarnInfo(LOCTEXT("UplMissing",
			"Warning: IconConfigurator_UPL_Android.xml not found in plugin Resources/. "
			"Android Manifest injection may fail during packaging."));
		WarnInfo.ExpireDuration = 6.f;
		WarnInfo.bFireAndForget = true;
		FSlateNotificationManager::Get().AddNotification(WarnInfo);
	}

	return FReply::Handled();
}

// ---------------------------------------------------------------
// Utility functions
// ---------------------------------------------------------------

void SIconConfiguratorWidget::LoadPreviewTexture(int32 LayerIndex)
{
	// Release the old rooted texture before replacing it
	if (Layers[LayerIndex].PreviewTexture.IsValid())
	{
		UTexture2D* OldTex = Layers[LayerIndex].PreviewTexture.Get();
		if (OldTex->IsRooted()) OldTex->RemoveFromRoot();
	}

	const FString& FilePath = Layers[LayerIndex].FilePath;
	if (FilePath.IsEmpty()) return;

	UTexture2D* Tex = FImageUtils::ImportFileAsTexture2D(FilePath);
	if (Tex)
	{
		Tex->SetFlags(RF_Transient);
		Tex->AddToRoot(); // Prevent GC from collecting the texture during Android packaging
		Layers[LayerIndex].PreviewTexture = Tex;
	}
}

void SIconConfiguratorWidget::UpdateThumbnail(int32 LayerIndex)
{
	UTexture2D* Tex = Layers[LayerIndex].PreviewTexture.IsValid()
		? Layers[LayerIndex].PreviewTexture.Get() : nullptr;

	if (Tex)
	{
		ThumbnailBrushes[LayerIndex].SetResourceObject(Tex);
		ThumbnailBrushes[LayerIndex].ImageSize = FVector2D(
			static_cast<float>(Tex->GetSizeX()),
			static_cast<float>(Tex->GetSizeY()));
	}
	else
	{
		ThumbnailBrushes[LayerIndex].SetResourceObject(nullptr);
		ThumbnailBrushes[LayerIndex].ImageSize = FVector2D::ZeroVector;
	}
}

void SIconConfiguratorWidget::RefreshViewports()
{
	if (Viewport2D.IsValid()) Viewport2D->Refresh();
	if (Viewport3D.IsValid()) Viewport3D->Refresh();
}

void SIconConfiguratorWidget::LoadAIPreviewTexture(int32 LayerIndex)
{
	// Release the old rooted texture before replacing it
	if (AILayers[LayerIndex].PreviewTexture.IsValid())
	{
		UTexture2D* OldTex = AILayers[LayerIndex].PreviewTexture.Get();
		if (OldTex->IsRooted()) OldTex->RemoveFromRoot();
	}

	const FString& FilePath = AILayers[LayerIndex].FilePath;
	if (FilePath.IsEmpty()) return;

	UTexture2D* Tex = FImageUtils::ImportFileAsTexture2D(FilePath);
	if (Tex)
	{
		Tex->SetFlags(RF_Transient);
		Tex->AddToRoot(); // Prevent GC from collecting the texture during Android packaging
		AILayers[LayerIndex].PreviewTexture = Tex;
	}
}

void SIconConfiguratorWidget::UpdateAIThumbnail(int32 LayerIndex)
{
	UTexture2D* Tex = AILayers[LayerIndex].PreviewTexture.IsValid()
		? AILayers[LayerIndex].PreviewTexture.Get() : nullptr;

	if (Tex)
	{
		AIThumbnailBrushes[LayerIndex].SetResourceObject(Tex);
		AIThumbnailBrushes[LayerIndex].ImageSize = FVector2D(
			static_cast<float>(Tex->GetSizeX()),
			static_cast<float>(Tex->GetSizeY()));
	}
	else
	{
		AIThumbnailBrushes[LayerIndex].SetResourceObject(nullptr);
		AIThumbnailBrushes[LayerIndex].ImageSize = FVector2D::ZeroVector;
	}
}

const FSlateBrush* SIconConfiguratorWidget::GetAIThumbnailBrush(int32 LayerIndex) const
{
	// Use TWeakObjectPtr validity as the authoritative check (same reason as GetThumbnailBrush).
	static FSlateBrush NoBrush;
	NoBrush.DrawAs = ESlateBrushDrawType::NoDrawType;

	if (LayerIndex < 0 || LayerIndex >= kMaxLayers)      return &NoBrush;
	if (LayerIndex >= AILayers.Num())                    return &NoBrush;
	if (!AILayers[LayerIndex].PreviewTexture.IsValid())  return &NoBrush;

	return &AIThumbnailBrushes[LayerIndex];
}

void SIconConfiguratorWidget::RebuildAIResultLayers()
{
	if (!AIResultLayersContainer.IsValid()) return;

	if (AILayers.Num() == 0)
	{
		AIResultLayersContainer->SetContent(SNew(SBox));
		return;
	}

	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);

	// Title
	Box->AddSlot()
	.AutoHeight()
	.Padding(0.f, 0.f, 0.f, 6.f)
	[
		SNew(STextBlock)
		.Text(LOCTEXT("AIResultLayers", "Generated Layers"))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
	];

	for (int32 i = 0; i < AILayers.Num(); ++i)
	{
		FText Label;
		if (i == 0)
			Label = LOCTEXT("AIBgLayer", "Background layer");
		else if (i == 1)
			Label = LOCTEXT("AIFg1Layer", "Foreground layer1");
		else
			Label = LOCTEXT("AIFg2Layer", "Foreground layer2");

		const int32 CapturedIdx = i;
		Box->AddSlot()
		.AutoHeight()
		.Padding(0.f, 0.f, 0.f, 4.f)
		[
			SNew(SHorizontalBox)
			// Thumbnail (read-only)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 10.f, 0.f)
			[
				SNew(SBox)
				.WidthOverride(60.f)
				.HeightOverride(60.f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[ SNew(SThumbnailCheckerboard) ]
					+ SOverlay::Slot()
					[
						SNew(SImage)
						.Image(TAttribute<const FSlateBrush*>::CreateSP(
							this, &SIconConfiguratorWidget::GetAIThumbnailBrush, CapturedIdx))
					]
				]
			]
			// Layer name
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
			]
		];
	}

	AIResultLayersContainer->SetContent(Box);
}

// ---------------------------------------------------------------
// AI Split — Slate attribute binding helpers
// ---------------------------------------------------------------

bool SIconConfiguratorWidget::IsGenerateEnabled() const
{
	return (AISplitState == EAISplitState::Idle
	     || AISplitState == EAISplitState::Done
	     || AISplitState == EAISplitState::Error)
	    && bAISplitAgreed
	    && !AISplitFilePath.IsEmpty();
}

EVisibility SIconConfiguratorWidget::GetAISplitProgressVis() const
{
	return (AISplitState != EAISplitState::Idle
	     && AISplitState != EAISplitState::Done
	     && AISplitState != EAISplitState::Error)
	     ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SIconConfiguratorWidget::GetAISplitErrorVis() const
{
	return (AISplitState == EAISplitState::Error) ? EVisibility::Visible : EVisibility::Collapsed;
}

FText SIconConfiguratorWidget::GetAISplitStatusText() const
{
	switch (AISplitState)
	{
	case EAISplitState::FetchingConfig:   return LOCTEXT("AIS_FetchCfg",  "Fetching config...");
	case EAISplitState::UploadingToTos:   return LOCTEXT("AIS_Upload",    "Uploading image...");
	case EAISplitState::Generating:       return LOCTEXT("AIS_Gen",       "Submitting task...");
	case EAISplitState::Polling:
		return FText::FromString(FString::Printf(TEXT("Processing... %.0f%%"), AISplitProgress * 100.f));
	case EAISplitState::Downloading:
		return FText::FromString(FString::Printf(TEXT("Downloading layers... %.0f%%"), AISplitProgress * 100.f));
	default:
		return FText::GetEmpty();
	}
}

TOptional<float> SIconConfiguratorWidget::GetAISplitProgressVal() const
{
	if (AISplitState == EAISplitState::FetchingConfig
	 || AISplitState == EAISplitState::UploadingToTos
	 || AISplitState == EAISplitState::Generating)
	{
		return TOptional<float>(); // indeterminate
	}
	return AISplitProgress;
}

// ---------------------------------------------------------------
// AI Split — User interaction callbacks
// ---------------------------------------------------------------

FReply SIconConfiguratorWidget::OnGenerateClicked()
{
	if (!bAISplitAgreed || AISplitFilePath.IsEmpty()) return FReply::Handled();

	// Cancel any pending poll ticker from a previous run
	if (AISplitPollTickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(AISplitPollTickHandle);
		AISplitPollTickHandle.Reset();
	}

	// Reset any previous run data
	AISplitLayerUrls.Empty(); AISplitSdfUrls.Empty();
	AISplitDLLayerPaths.Empty(); AISplitDLSdfPaths.Empty();
	AISplitTaskId.Empty(); AISplitObjectKey.Empty(); AISplitUploadedUrl.Empty();
	AISplitDLDone = AISplitDLTotal = 0;
	AISplit_FetchTccConfig();
	return FReply::Handled();
}

void SIconConfiguratorWidget::OnAISplitAgreedChanged(ECheckBoxState NewState)
{
	bAISplitAgreed = (NewState == ECheckBoxState::Checked);
}

// ---------------------------------------------------------------
// AI Split — Pipeline
// ---------------------------------------------------------------

void SIconConfiguratorWidget::AISplit_FetchTccConfig()
{
	AISplitState    = EAISplitState::FetchingConfig;
	AISplitProgress = 0.f;
	AISplitErrorMsg.Empty();

	// Try CN (production) URL first; fall back to internal (debug-test) URL in OnTccConfigDone
	const FString TccUrl = TEXT("https://lf3-config.bytetcc.com/obj/tcc-config-web/tcc-v2-data-pico.spatial-plugin.emulator");
	UE_LOG(LogTemp, Log, TEXT("AISplit: GET %s"), *TccUrl);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetVerb(TEXT("GET"));
	Req->SetURL(TccUrl);
	Req->SetHeader(TEXT("Accept"), TEXT("application/json"));
	Req->OnProcessRequestComplete().BindSP(this, &SIconConfiguratorWidget::AISplit_OnTccConfigDone);
	Req->ProcessRequest();
}

void SIconConfiguratorWidget::AISplit_OnTccConfigDone(FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConn)
{
	const int32 Code = (bConn && Resp.IsValid()) ? Resp->GetResponseCode() : 0;

	// If CN URL returns 404, retry with internal (debug-test) URL
	if (Code == 404)
	{
		const FString FallbackUrl = TEXT("https://lf3-config.bytetcc.com/obj/tcc-config-web/tcc-v2-data-pico.spatial-plugin.emulator-debug-test");
		UE_LOG(LogTemp, Log, TEXT("AISplit: CN TCC 404, retrying internal URL: %s"), *FallbackUrl);

		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> RetryReq = FHttpModule::Get().CreateRequest();
		RetryReq->SetVerb(TEXT("GET"));
		RetryReq->SetURL(FallbackUrl);
		RetryReq->SetHeader(TEXT("Accept"), TEXT("application/json"));
		RetryReq->OnProcessRequestComplete().BindLambda(
			[WeakSelf = TWeakPtr<SIconConfiguratorWidget>(SharedThis(this))](FHttpRequestPtr, FHttpResponsePtr R2, bool bC2)
			{
				TSharedPtr<SIconConfiguratorWidget> Self = WeakSelf.Pin();
				if (!Self.IsValid()) return;

				if (!bC2 || !R2.IsValid() || R2->GetResponseCode() != 200)
				{
					const int32 C2 = R2.IsValid() ? R2->GetResponseCode() : 0;
					Self->AISplit_SetError(FString::Printf(
						TEXT("TCC config unavailable (CN=404, internal=%d). Contact PICO SDK team."), C2));
					return;
				}
				Self->AISplit_ParseTccAndContinue(R2->GetContentAsString());
			});
		RetryReq->ProcessRequest();
		return;
	}

	if (Code != 200)
	{
		AISplit_SetError(FString::Printf(TEXT("Failed to fetch TCC config (HTTP %d)."), Code));
		return;
	}

	AISplit_ParseTccAndContinue(Resp->GetContentAsString());
}

void SIconConfiguratorWidget::AISplit_ParseTccAndContinue(const FString& ResponseBody)
{
	// DEBUG: print first 2000 chars of response to log
	UE_LOG(LogTemp, Log, TEXT("AISplit TCC raw (first 2000): %.2000s"), *ResponseBody);

	AISplitTccConfig = FAISplitTccConfig{};
	if (!AISplitHelpers::ParseTccConfig(ResponseBody, AISplitTccConfig))
	{
		AISplit_SetError(TEXT("Failed to parse TCC config."));
		return;
	}

	// Pick the first region that has a valid API config (cn preferred, then internal, then global).
	// TOS config is optional — if absent we'll fall back to base64 encoding.
	FString Region;
	for (const FString& Key : TArray<FString>{ TEXT("cn"), TEXT("internal"), TEXT("global") })
	{
		if (!AISplitTccConfig.RegionConfig.Contains(Key)) continue;
		const FAISplitRegionConfig& R = AISplitTccConfig.RegionConfig[Key];
		if (!R.Api.GenUrl.IsEmpty())
		{
			Region = Key;
			break;
		}
	}

	if (Region.IsEmpty())
	{
		AISplit_SetError(TEXT("No region with a valid GenUrl found in TCC config."));
		return;
	}

	AISplitRegionCfg = AISplitTccConfig.RegionConfig[Region];
	UE_LOG(LogTemp, Log, TEXT("AISplit: TCC OK [%s] – Bucket=%s Endpoint=%s GenUrl=%s"),
		*Region, *AISplitRegionCfg.Tos.Bucket, *AISplitRegionCfg.Tos.Endpoint, *AISplitRegionCfg.Api.GenUrl);

	AISplit_UploadToTos();
}

void SIconConfiguratorWidget::AISplit_UploadToTos()
{
	AISplitState    = EAISplitState::UploadingToTos;
	AISplitProgress = 0.1f;

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *AISplitFilePath))
	{
		AISplit_SetError(TEXT("Failed to read image file."));
		return;
	}

	// If TOS credentials are present in TCC config, use TOS PUT upload.
	if (!AISplitRegionCfg.Tos.Endpoint.IsEmpty() && !AISplitRegionCfg.Tos.Bucket.IsEmpty())
	{
		const FString Filename = FPaths::GetCleanFilename(AISplitFilePath);
		AISplitObjectKey = FString::Printf(TEXT("%sai_split_%lld_%s"),
			*AISplitRegionCfg.Tos.ObjectDir,
			FDateTime::UtcNow().ToUnixTimestamp(),
			*Filename);

		const FString UploadUrl = FString::Printf(TEXT("https://%s.%s/%s"),
			*AISplitRegionCfg.Tos.Bucket,
			*AISplitRegionCfg.Tos.Endpoint,
			*AISplitObjectKey);

		UE_LOG(LogTemp, Log, TEXT("AISplit: TOS PUT %s"), *UploadUrl);

		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
		Req->SetVerb(TEXT("PUT"));
		Req->SetURL(UploadUrl);
		Req->SetHeader(TEXT("Content-Type"), TEXT("image/png"));
		Req->SetHeader(TEXT("Connection"),   TEXT("close"));
		Req->SetContent(FileData);
		Req->OnProcessRequestComplete().BindSP(this, &SIconConfiguratorWidget::AISplit_OnUploadDone);
		Req->ProcessRequest();
		return;
	}

	// TOS credentials absent: try PICO appstore's own upload/presign endpoint.
	UE_LOG(LogTemp, Warning, TEXT("AISplit: No TOS config — trying server presign upload"));
	AISplitFileData = MoveTemp(FileData);
	AISplit_GetPresignUrl();
}

void SIconConfiguratorWidget::AISplit_GetPresignUrl()
{
	// Derive upload endpoint from gen_url: replace "/gen" suffix with "/upload"
	FString UploadEndpoint = AISplitRegionCfg.Api.GenUrl;
	if (UploadEndpoint.EndsWith(TEXT("/gen")))
		UploadEndpoint = UploadEndpoint.LeftChop(4) + TEXT("/upload");
	else
		UploadEndpoint = UploadEndpoint + TEXT("/upload");

	const int64 DeviceId = AISplitHelpers::GetOrCreateDeviceId();
	const FString SigInput = FString::Printf(TEXT("%lld%lld%s"),
		AISplitRegionCfg.Auth.AppId, DeviceId, *AISplitRegionCfg.Auth.Salt);
	const FString Sig = AISplitHelpers::ComputeSHA256Hex(SigInput);
	const FString Filename = FPaths::GetCleanFilename(AISplitFilePath);

	TSharedPtr<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetNumberField(TEXT("app_id"),    static_cast<double>(AISplitRegionCfg.Auth.AppId));
	Body->SetNumberField(TEXT("device_id"), static_cast<double>(DeviceId));
	Body->SetStringField(TEXT("signature"), Sig);
	Body->SetStringField(TEXT("file_name"), Filename);
	Body->SetNumberField(TEXT("file_size"), static_cast<double>(AISplitFileData.Num()));

	FString BodyStr;
	TSharedRef<TJsonWriter<>> W = TJsonWriterFactory<>::Create(&BodyStr);
	FJsonSerializer::Serialize(Body.ToSharedRef(), W);

	UE_LOG(LogTemp, Log, TEXT("AISplit: POST presign %s"), *UploadEndpoint);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetVerb(TEXT("POST"));
	Req->SetURL(UploadEndpoint);
	Req->SetHeader(TEXT("Content-Type"),    TEXT("application/json"));
	Req->SetHeader(TEXT("Accept-Encoding"), TEXT("identity"));
	if (!AISplitRegionCfg.Api.TtEnv.IsEmpty())
	{
		Req->SetHeader(TEXT("X-Tt-Env"),  AISplitRegionCfg.Api.TtEnv);
		Req->SetHeader(TEXT("X-Use-Ppe"), TEXT("1"));
	}
	Req->SetContentAsString(BodyStr);
	Req->OnProcessRequestComplete().BindSP(this, &SIconConfiguratorWidget::AISplit_OnPresignDone);
	Req->ProcessRequest();
}

void SIconConfiguratorWidget::AISplit_OnPresignDone(FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConn)
{
	const int32 HttpCode = bConn && Resp.IsValid() ? Resp->GetResponseCode() : 0;
	const FString RawBody = (bConn && Resp.IsValid()) ? Resp->GetContentAsString() : TEXT("(no response)");
	UE_LOG(LogTemp, Log, TEXT("AISplit presign HTTP=%d body=%.500s"), HttpCode, *RawBody);

	// If no presign endpoint exists, fall back to sending image as img_base64 in gen request
	if (!bConn || !Resp.IsValid() || HttpCode == 404 || HttpCode == 405)
	{
		UE_LOG(LogTemp, Warning, TEXT("AISplit: presign unavailable (HTTP %d) — trying img_base64 in gen body"), HttpCode);
		AISplitImageBase64 = FBase64::Encode(AISplitFileData);
		AISplitFileData.Empty();
		AISplit_StartGenerate();
		return;
	}

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawBody);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		AISplit_SetError(FString::Printf(TEXT("Upload API (HTTP %d): invalid JSON. Body: %.200s"), HttpCode, *RawBody));
		return;
	}

	double CodeD = -1.0;
	Json->TryGetNumberField(TEXT("code"), CodeD);
	const int32 Code = static_cast<int32>(CodeD);
	if (Code != 0)
	{
		FString Msg;
		Json->TryGetStringField(TEXT("msg"), Msg);
		AISplit_SetError(FString::Printf(TEXT("Upload API error %d: %s"), Code, *Msg));
		return;
	}

	// Extract PUT URL and final accessible URL from response
	const TSharedPtr<FJsonObject>* DataObj;
	if (!Json->TryGetObjectField(TEXT("data"), DataObj) || !(*DataObj).IsValid())
	{
		AISplit_SetError(TEXT("Upload API: missing 'data' in response."));
		return;
	}

	// Try common field names for the presigned PUT URL and the final public URL
	FString PutUrl, FinalUrl;
	(*DataObj)->TryGetStringField(TEXT("upload_url"), PutUrl);
	(*DataObj)->TryGetStringField(TEXT("put_url"),    PutUrl);   // fallback key name
	(*DataObj)->TryGetStringField(TEXT("file_url"),   FinalUrl);
	(*DataObj)->TryGetStringField(TEXT("url"),        FinalUrl); // fallback key name
	(*DataObj)->TryGetStringField(TEXT("img_url"),    FinalUrl); // another possible key

	if (PutUrl.IsEmpty() || FinalUrl.IsEmpty())
	{
		// Log all keys in data for diagnosis
		FString DataRaw;
		TSharedRef<TJsonWriter<>> DW = TJsonWriterFactory<>::Create(&DataRaw);
		FJsonSerializer::Serialize((*DataObj).ToSharedRef(), DW);
		AISplit_SetError(FString::Printf(
			TEXT("Upload API: could not find upload_url/file_url in data. data=%s"), *DataRaw));
		return;
	}

	AISplitPresignPutUrl  = PutUrl;
	AISplitUploadedUrl    = FinalUrl;
	UE_LOG(LogTemp, Log, TEXT("AISplit presign OK: PUT=%s final=%s"), *PutUrl, *FinalUrl);

	// PUT image to presigned URL
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> PutReq = FHttpModule::Get().CreateRequest();
	PutReq->SetVerb(TEXT("PUT"));
	PutReq->SetURL(PutUrl);
	PutReq->SetHeader(TEXT("Content-Type"), TEXT("image/png"));
	PutReq->SetHeader(TEXT("Connection"),   TEXT("close"));
	PutReq->SetContent(AISplitFileData);
	PutReq->OnProcessRequestComplete().BindSP(this, &SIconConfiguratorWidget::AISplit_OnUploadDone);
	PutReq->ProcessRequest();
	AISplitFileData.Empty(); // free memory
}

void SIconConfiguratorWidget::AISplit_OnUploadDone(FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConn)
{
	if (!bConn || !Resp.IsValid() || (Resp->GetResponseCode() != 200 && Resp->GetResponseCode() != 204))
	{
		const int32 Code = Resp.IsValid() ? Resp->GetResponseCode() : 0;
		AISplit_SetError(FString::Printf(TEXT("TOS upload failed (HTTP %d)."), Code));
		return;
	}

	AISplitUploadedUrl = AISplitRegionCfg.Tos.PublicUrlPrefix + AISplitObjectKey;
	UE_LOG(LogTemp, Log, TEXT("AISplit: Upload OK – %s"), *AISplitUploadedUrl);
	AISplit_StartGenerate();
}

void SIconConfiguratorWidget::AISplit_StartGenerate()
{
	AISplitState    = EAISplitState::Generating;
	AISplitProgress = 0.25f;

	const int64 DeviceId = AISplitHelpers::GetOrCreateDeviceId();
	const FString SigInput = FString::Printf(TEXT("%lld%lld%s"),
		AISplitRegionCfg.Auth.AppId, DeviceId, *AISplitRegionCfg.Auth.Salt);
	const FString Sig = AISplitHelpers::ComputeSHA256Hex(SigInput);

	// Build JSON body
	TSharedPtr<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetNumberField(TEXT("app_id"),    static_cast<double>(AISplitRegionCfg.Auth.AppId));
	Body->SetNumberField(TEXT("device_id"), static_cast<double>(DeviceId));
	Body->SetStringField(TEXT("signature"), Sig);

	if (!AISplitImageBase64.IsEmpty())
	{
		// No upload endpoint available — try sending image inline as base64
		UE_LOG(LogTemp, Warning, TEXT("AISplit: sending img_base64 (%d chars)"), AISplitImageBase64.Len());
		Body->SetStringField(TEXT("img_bytes"), AISplitImageBase64);
		AISplitImageBase64.Empty();
	}
	else
	{
		Body->SetStringField(TEXT("img_url"), AISplitUploadedUrl);
	}

	FString BodyStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyStr);
	FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);

	UE_LOG(LogTemp, Log, TEXT("AISplit: POST gen %s"), *AISplitRegionCfg.Api.GenUrl);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetVerb(TEXT("POST"));
	Req->SetURL(AISplitRegionCfg.Api.GenUrl);
	Req->SetHeader(TEXT("Content-Type"),     TEXT("application/json"));
	Req->SetHeader(TEXT("Accept-Encoding"),  TEXT("identity"));
	if (!AISplitRegionCfg.Api.TtEnv.IsEmpty())
	{
		Req->SetHeader(TEXT("X-Tt-Env"),  AISplitRegionCfg.Api.TtEnv);
		Req->SetHeader(TEXT("X-Use-Ppe"), TEXT("1"));
	}
	Req->SetContentAsString(BodyStr);
	Req->OnProcessRequestComplete().BindSP(this, &SIconConfiguratorWidget::AISplit_OnGenerateDone);
	Req->ProcessRequest();
}

void SIconConfiguratorWidget::AISplit_OnGenerateDone(FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConn)
{
	if (!bConn || !Resp.IsValid())
	{
		AISplit_SetError(TEXT("Network error calling gen API."));
		return;
	}

	const int32 HttpCode = Resp->GetResponseCode();
	const FString RawBody = Resp->GetContentAsString();
	UE_LOG(LogTemp, Log, TEXT("AISplit gen response HTTP=%d body=%.2000s"), HttpCode, *RawBody);

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawBody);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		AISplit_SetError(TEXT("Gen API: invalid JSON response."));
		return;
	}

	double CodeD = -1.0;
	Json->TryGetNumberField(TEXT("code"), CodeD);
	const int32 Code = static_cast<int32>(CodeD);
	if (Code != 0)
	{
		FString Msg;
		Json->TryGetStringField(TEXT("msg"), Msg);
		AISplit_SetError(FString::Printf(TEXT("Gen API error %d: %s"), Code, *Msg));
		return;
	}

	const TSharedPtr<FJsonObject>* DataObj;
	if (!Json->TryGetObjectField(TEXT("data"), DataObj) || !(*DataObj).IsValid())
	{
		AISplit_SetError(TEXT("Gen API: missing 'data'."));
		return;
	}
	(*DataObj)->TryGetStringField(TEXT("task_id"), AISplitTaskId);
	if (AISplitTaskId.IsEmpty())
	{
		AISplit_SetError(TEXT("Gen API: empty task_id."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AISplit: task_id=%s"), *AISplitTaskId);
	AISplit_ScheduleNextPoll();
}

void SIconConfiguratorWidget::AISplit_ScheduleNextPoll()
{
	AISplitState = EAISplitState::Polling;
	const float DelaySeconds = FMath::Max(1.f, AISplitTccConfig.TimeoutMs.PollInterval / 1000.f);

	TWeakPtr<SIconConfiguratorWidget> WeakSelf = SharedThis(this);
	AISplitPollTickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateLambda([WeakSelf](float) -> bool
		{
			TSharedPtr<SIconConfiguratorWidget> Self = WeakSelf.Pin();
			if (Self.IsValid()) Self->AISplit_DoPoll();
			return false; // single-shot
		}),
		DelaySeconds);
}

void SIconConfiguratorWidget::AISplit_DoPoll()
{
	UE_LOG(LogTemp, Log, TEXT("AISplit: polling task=%s"), *AISplitTaskId);

	const int64 DeviceId = AISplitHelpers::GetOrCreateDeviceId();
	const FString SigInput = FString::Printf(TEXT("%lld%lld%s"),
		AISplitRegionCfg.Auth.AppId, DeviceId, *AISplitRegionCfg.Auth.Salt);
	const FString Sig = AISplitHelpers::ComputeSHA256Hex(SigInput);

	TSharedPtr<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetNumberField(TEXT("app_id"),    static_cast<double>(AISplitRegionCfg.Auth.AppId));
	Body->SetNumberField(TEXT("device_id"), static_cast<double>(DeviceId));
	Body->SetStringField(TEXT("signature"), Sig);
	Body->SetStringField(TEXT("task_id"),   AISplitTaskId);

	FString BodyStr;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyStr);
	FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
	Req->SetVerb(TEXT("POST"));
	Req->SetURL(AISplitRegionCfg.Api.InfoUrl);
	Req->SetHeader(TEXT("Content-Type"),    TEXT("application/json"));
	Req->SetHeader(TEXT("Accept-Encoding"), TEXT("identity"));
	if (!AISplitRegionCfg.Api.TtEnv.IsEmpty())
	{
		Req->SetHeader(TEXT("X-Tt-Env"),  AISplitRegionCfg.Api.TtEnv);
		Req->SetHeader(TEXT("X-Use-Ppe"), TEXT("1"));
	}
	Req->SetContentAsString(BodyStr);
	Req->OnProcessRequestComplete().BindSP(this, &SIconConfiguratorWidget::AISplit_OnPollDone);
	Req->ProcessRequest();
}

void SIconConfiguratorWidget::AISplit_OnPollDone(FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConn)
{
	if (!bConn || !Resp.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AISplit poll: network error (bConn=%d)"), bConn);
		AISplit_SetError(TEXT("Network error polling info API."));
		return;
	}

	const int32 HttpCode = Resp->GetResponseCode();
	const FString RawBody = Resp->GetContentAsString();
	UE_LOG(LogTemp, Log, TEXT("AISplit poll HTTP=%d body=%.500s"), HttpCode, *RawBody);

	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(RawBody);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		AISplit_SetError(TEXT("Info API: invalid JSON."));
		return;
	}

	double CodeD2 = -1.0;
	Json->TryGetNumberField(TEXT("code"), CodeD2);
	const int32 Code = static_cast<int32>(CodeD2);

	if (Code == 10002)
	{
		// Still processing
		const TSharedPtr<FJsonObject>* DataObj;
		if (Json->TryGetObjectField(TEXT("data"), DataObj) && (*DataObj).IsValid())
		{
			double ProgressPct = -1.0;
			if ((*DataObj)->TryGetNumberField(TEXT("progress"), ProgressPct) && ProgressPct >= 0.0)
			{
				AISplitProgress = static_cast<float>(ProgressPct) / 100.f;
			}
		}
		AISplit_ScheduleNextPoll();
		return;
	}

	if (Code == 0)
	{
		const TSharedPtr<FJsonObject>* DataObj;
		if (!Json->TryGetObjectField(TEXT("data"), DataObj) || !(*DataObj).IsValid())
		{
			AISplit_SetError(TEXT("Info API: missing data."));
			return;
		}

		// Extract layer URLs (server order: foreground-first → we reverse → background-first)
		AISplitLayerUrls.Empty();
		AISplitSdfUrls.Empty();

		auto ExtractUrls = [&](const FString& FieldName, TArray<FString>& OutUrls)
		{
			const TArray<TSharedPtr<FJsonValue>>* Arr;
			if (!(*DataObj)->TryGetArrayField(FieldName, Arr)) return;
			for (const TSharedPtr<FJsonValue>& Val : *Arr)
			{
				const TSharedPtr<FJsonObject>* EntryObj;
				if (Val->TryGetObject(EntryObj))
				{
					FString Url;
					if ((*EntryObj)->TryGetStringField(TEXT("url"), Url))
						OutUrls.Add(Url);
				}
			}
			// Server already returns layers in background-first order; no reversal needed.
		};

		ExtractUrls(TEXT("layer"), AISplitLayerUrls);
		ExtractUrls(TEXT("sdf"),   AISplitSdfUrls);

		if (AISplitLayerUrls.IsEmpty() || AISplitSdfUrls.IsEmpty())
		{
			AISplit_SetError(TEXT("Info API: empty layer or sdf arrays."));
			return;
		}

		UE_LOG(LogTemp, Log, TEXT("AISplit: Got %d layers, %d sdfs"),
			AISplitLayerUrls.Num(), AISplitSdfUrls.Num());

		AISplit_DownloadAll();
		return;
	}

	FString Msg;
	Json->TryGetStringField(TEXT("msg"), Msg);
	AISplit_SetError(FString::Printf(TEXT("Info API error %d: %s"), Code, *Msg));
}

void SIconConfiguratorWidget::AISplit_DownloadAll()
{
	AISplitState = EAISplitState::Downloading;

	const int32 NLayers = FMath::Min(AISplitLayerUrls.Num(), kMaxLayers);
	const int32 NSdfs   = FMath::Min(AISplitSdfUrls.Num(),   kMaxLayers);

	AISplitDLLayerPaths.SetNum(NLayers);
	AISplitDLSdfPaths.SetNum(NSdfs);
	AISplitDLDone  = 0;
	AISplitDLTotal = NLayers + NSdfs;

	if (AISplitDLTotal == 0)
	{
		AISplit_SetError(TEXT("Nothing to download."));
		return;
	}

	const FString DLDir = FPaths::ProjectSavedDir() / TEXT("AISplit") / AISplitTaskId;
	IFileManager::Get().MakeDirectory(*DLDir, true);

	auto DownloadOne = [&](const FString& Url, int32 Idx, bool bIsSdf)
	{
		const FString DestPath = DLDir / FString::Printf(TEXT("%s_%d.png"), bIsSdf ? TEXT("sdf") : TEXT("layer"), Idx);

		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
		Req->SetVerb(TEXT("GET"));
		Req->SetURL(Url);
		Req->OnProcessRequestComplete().BindSP(this, &SIconConfiguratorWidget::AISplit_OnOneFileDone,
			DestPath, Idx, bIsSdf);
		Req->ProcessRequest();
	};

	for (int32 i = 0; i < NLayers; ++i) DownloadOne(AISplitLayerUrls[i], i, false);
	for (int32 i = 0; i < NSdfs;   ++i) DownloadOne(AISplitSdfUrls[i],   i, true);
}

void SIconConfiguratorWidget::AISplit_OnOneFileDone(
	FHttpRequestPtr /*Req*/, FHttpResponsePtr Resp, bool bConn,
	FString DestPath, int32 Idx, bool bIsSdf)
{
	if (bConn && Resp.IsValid() && Resp->GetResponseCode() == 200)
	{
		FFileHelper::SaveArrayToFile(Resp->GetContent(), *DestPath);
		if (bIsSdf)
			AISplitDLSdfPaths[Idx] = DestPath;
		else
			AISplitDLLayerPaths[Idx] = DestPath;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AISplit: Download failed for idx=%d isSdf=%d"), Idx, bIsSdf);
	}

	++AISplitDLDone;
	AISplitProgress = 0.6f + 0.4f * (static_cast<float>(AISplitDLDone) / AISplitDLTotal);

	if (AISplitDLDone == AISplitDLTotal)
	{
		AISplit_FillAILayers();
	}
}

void SIconConfiguratorWidget::AISplit_FillAILayers()
{
	const int32 Count = FMath::Min(AISplitDLLayerPaths.Num(), kMaxLayers);

	// Resize AI layers to match result count
	AILayers.SetNum(Count);

	// Clear all AI layer slots so stale data from a previous run doesn't bleed through
	for (int32 i = 0; i < Count; ++i)
	{
		// Release the old rooted texture before clearing
		if (AILayers[i].PreviewTexture.IsValid())
		{
			UTexture2D* OldTex = AILayers[i].PreviewTexture.Get();
			if (OldTex->IsRooted()) OldTex->RemoveFromRoot();
		}
		AILayers[i].FilePath = TEXT("");
		AILayers[i].PreviewTexture  = nullptr;
		AILayers[i].ImportedTexture = nullptr;
		AIThumbnailBrushes[i] = FSlateBrush();
		AIThumbnailBrushes[i].DrawAs = ESlateBrushDrawType::Image;
	}

	// Server layer[0]=foreground, layer[Count-1]=background — fill in reverse:
	// layer[0] → AILayers[Count-1] (Foreground)
	// layer[Count-1] → AILayers[0] (Background)
	for (int32 i = 0; i < Count; ++i)
	{
		const int32 SlotIdx = Count - 1 - i;
		if (!AISplitDLLayerPaths[i].IsEmpty())
		{
			AILayers[SlotIdx].FilePath = AISplitDLLayerPaths[i];
			LoadAIPreviewTexture(SlotIdx);
			UpdateAIThumbnail(SlotIdx);
		}
	}

	RebuildAIResultLayers();
	// Point the shared viewports at AILayers and refresh
	if (Viewport2D.IsValid()) Viewport2D->SetLayerData(&AILayers);
	if (Viewport3D.IsValid()) Viewport3D->SetLayerData(&AILayers);
	RefreshViewports();

	AISplitState    = EAISplitState::Done;
	AISplitProgress = 1.f;

	SaveState();

	FNotificationInfo Info(LOCTEXT("AISplitDone", "AI Split complete. Results shown in AI Tab preview."));
	Info.ExpireDuration = 4.f;
	Info.bFireAndForget = true;
	FSlateNotificationManager::Get().AddNotification(Info);
}

void SIconConfiguratorWidget::AISplit_SetError(const FString& Msg)
{
	AISplitState  = EAISplitState::Error;
	AISplitErrorMsg = Msg;
	UE_LOG(LogTemp, Error, TEXT("AISplit error: %s"), *Msg);

	FNotificationInfo Info(FText::FromString(FString::Printf(TEXT("AI Split failed: %s"), *Msg)));
	Info.ExpireDuration = 5.f;
	Info.bFireAndForget = true;
	FSlateNotificationManager::Get().AddNotification(Info);
}

// ---------------------------------------------------------------
// State persistence
// ---------------------------------------------------------------

static FString GetStatePath()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("IconConfigurator"));
	if (!Plugin.IsValid()) return FString();
	const FString Dir = Plugin->GetBaseDir() / TEXT("Intermediate");
	IFileManager::Get().MakeDirectory(*Dir, true);
	return Dir / TEXT("IconConfiguratorState.json");
}

static TSharedPtr<FJsonObject> LayerToJson(const FIconLayerData& L)
{
	TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
	Obj->SetStringField(TEXT("FilePath"), L.FilePath);
	Obj->SetBoolField(TEXT("bVisible"),   L.bVisible);
	Obj->SetNumberField(TEXT("Opacity"),  L.Opacity);
	Obj->SetNumberField(TEXT("OffsetX"),  L.Offset.X);
	Obj->SetNumberField(TEXT("OffsetY"),  L.Offset.Y);
	Obj->SetNumberField(TEXT("Scale"),    L.Scale);
	return Obj;
}

static FIconLayerData LayerFromJson(const TSharedPtr<FJsonObject>& Obj)
{
	FIconLayerData L;
	Obj->TryGetStringField(TEXT("FilePath"), L.FilePath);
	Obj->TryGetBoolField(TEXT("bVisible"),   L.bVisible);
	double d = 1.0;
	if (Obj->TryGetNumberField(TEXT("Opacity"),  d)) L.Opacity   = static_cast<float>(d);
	if (Obj->TryGetNumberField(TEXT("OffsetX"),  d)) L.Offset.X  = static_cast<float>(d);
	if (Obj->TryGetNumberField(TEXT("OffsetY"),  d)) L.Offset.Y  = static_cast<float>(d);
	if (Obj->TryGetNumberField(TEXT("Scale"),    d)) L.Scale      = static_cast<float>(d);
	return L;
}

void SIconConfiguratorWidget::SaveState() const
{
	const FString Path = GetStatePath();
	if (Path.IsEmpty()) return;

	TSharedPtr<FJsonObject> Root = MakeShared<FJsonObject>();
	Root->SetNumberField(TEXT("ActiveTab"), ActiveTab);

	// Manual layers
	TArray<TSharedPtr<FJsonValue>> ManualArr;
	for (const FIconLayerData& L : Layers)
		ManualArr.Add(MakeShared<FJsonValueObject>(LayerToJson(L)));
	Root->SetArrayField(TEXT("ManualLayers"), ManualArr);

	// AI layers
	TArray<TSharedPtr<FJsonValue>> AIArr;
	for (const FIconLayerData& L : AILayers)
		AIArr.Add(MakeShared<FJsonValueObject>(LayerToJson(L)));
	Root->SetArrayField(TEXT("AILayers"), AIArr);

	// AI split original source image
	Root->SetStringField(TEXT("AISplitFilePath"), AISplitFilePath);

	// Localization entries
	TArray<TSharedPtr<FJsonValue>> LocArr;
	for (const TSharedPtr<FLocEntry>& E : LocEntries)
	{
		TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
		Obj->SetStringField(TEXT("LangCode"),    E->LangCode);
		Obj->SetStringField(TEXT("LangDisplay"), E->LangDisplay);
		Obj->SetStringField(TEXT("AppName"),     *E->AppName);
		LocArr.Add(MakeShared<FJsonValueObject>(Obj));
	}
	Root->SetArrayField(TEXT("LocEntries"), LocArr);

	FString Out;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
	FJsonSerializer::Serialize(Root.ToSharedRef(), Writer);
	FFileHelper::SaveStringToFile(Out, *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

void SIconConfiguratorWidget::LoadState()
{
	const FString Path = GetStatePath();
	if (Path.IsEmpty() || !FPaths::FileExists(Path)) return;

	FString Raw;
	if (!FFileHelper::LoadFileToString(Raw, *Path)) return;

	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()) return;

	// ---- Manual layers ----
	const TArray<TSharedPtr<FJsonValue>>* ManualArr = nullptr;
	if (Root->TryGetArrayField(TEXT("ManualLayers"), ManualArr) && ManualArr)
	{
		const int32 Count = FMath::Clamp(ManualArr->Num(), kMinLayers, kMaxLayers);
		Layers.SetNum(Count);
		for (int32 i = 0; i < kMaxLayers; ++i)
		{
			ThumbnailBrushes[i] = FSlateBrush();
			ThumbnailBrushes[i].DrawAs = ESlateBrushDrawType::Image;
		}
		for (int32 i = 0; i < Count; ++i)
		{
			const TSharedPtr<FJsonObject>* Obj = nullptr;
			if ((*ManualArr)[i]->TryGetObject(Obj) && Obj)
			{
				Layers[i] = LayerFromJson(*Obj);
				if (!Layers[i].FilePath.IsEmpty() && FPaths::FileExists(Layers[i].FilePath))
				{
					LoadPreviewTexture(i);
					UpdateThumbnail(i);
				}
				else
				{
					Layers[i].FilePath.Empty();
				}
			}
		}
	}

	// ---- AI layers ----
	const TArray<TSharedPtr<FJsonValue>>* AIArr = nullptr;
	if (Root->TryGetArrayField(TEXT("AILayers"), AIArr) && AIArr)
	{
		const int32 Count = FMath::Clamp(AIArr->Num(), 0, kMaxLayers);
		if (Count > 0)
		{
			AILayers.SetNum(Count);
			for (int32 i = 0; i < kMaxLayers; ++i)
			{
				AIThumbnailBrushes[i] = FSlateBrush();
				AIThumbnailBrushes[i].DrawAs = ESlateBrushDrawType::Image;
			}
			for (int32 i = 0; i < Count; ++i)
			{
				const TSharedPtr<FJsonObject>* Obj = nullptr;
				if ((*AIArr)[i]->TryGetObject(Obj) && Obj)
				{
					AILayers[i] = LayerFromJson(*Obj);
					if (!AILayers[i].FilePath.IsEmpty() && FPaths::FileExists(AILayers[i].FilePath))
					{
						LoadAIPreviewTexture(i);
						UpdateAIThumbnail(i);
					}
					else
					{
						AILayers[i].FilePath.Empty();
					}
				}
			}
		}
	}

	// ---- AI split original source image ----
	FString SavedAISplitPath;
	if (Root->TryGetStringField(TEXT("AISplitFilePath"), SavedAISplitPath)
		&& !SavedAISplitPath.IsEmpty()
		&& FPaths::FileExists(SavedAISplitPath))
	{
		AISplitFilePath = SavedAISplitPath;

		// Release any previous thumbnail texture from the GC root
		if (UTexture2D* OldTex = Cast<UTexture2D>(AISplitThumbnailBrush.GetResourceObject()))
		{
			OldTex->RemoveFromRoot();
		}

		UTexture2D* Tex = FImageUtils::ImportFileAsTexture2D(AISplitFilePath);
		if (Tex)
		{
			Tex->SetFlags(RF_Transient);
			Tex->AddToRoot();
			AISplitThumbnailBrush.SetResourceObject(Tex);
			AISplitThumbnailBrush.ImageSize = FVector2D(
				static_cast<float>(Tex->GetSizeX()),
				static_cast<float>(Tex->GetSizeY()));
		}
	}

	// ---- Localization ----
	const TArray<TSharedPtr<FJsonValue>>* LocArr = nullptr;
	if (Root->TryGetArrayField(TEXT("LocEntries"), LocArr) && LocArr && LocArr->Num() > 0)
	{
		TSharedPtr<FLocEntry> DefaultLoc = MakeShared<FLocEntry>();
		DefaultLoc->LangCode    = TEXT("");
		DefaultLoc->LangDisplay = TEXT("English (United States)");

		TArray<TSharedPtr<FLocEntry>> SupportedLocs;
		TSet<FString> AddedLangCodes;

		for (int32 i = 0; i < LocArr->Num(); ++i)
		{
			const TSharedPtr<FJsonObject>* Obj = nullptr;
			if (!(*LocArr)[i]->TryGetObject(Obj) || !Obj) continue;

			FString LangCode;
			(*Obj)->TryGetStringField(TEXT("LangCode"), LangCode);

			FString AppName;
			(*Obj)->TryGetStringField(TEXT("AppName"), AppName);

			if (LangCode.IsEmpty())
			{
				*DefaultLoc->AppName = AppName;
				continue;
			}

			const int32 LangIdx = LangCodes.IndexOfByPredicate(
				[&](const FString& Code) { return Code == LangCode; });
			if (!LangCodes.IsValidIndex(LangIdx) || AddedLangCodes.Contains(LangCode))
			{
				continue;
			}

			TSharedPtr<FLocEntry> E = MakeShared<FLocEntry>();
			E->LangCode    = LangCode;
			E->LangDisplay = LangDisplayOptions.IsValidIndex(LangIdx) ? *LangDisplayOptions[LangIdx] : LangCode;
			*E->AppName    = AppName;
			SupportedLocs.Add(E);
			AddedLangCodes.Add(LangCode);
		}

		LocEntries.Empty();
		LocEntries.Add(DefaultLoc);
		LocEntries.Append(SupportedLocs);
	}

	// ---- Active tab (do last so viewports get correct pointer) ----
	double TabVal = 0.0;
	const int32 SavedTab = Root->TryGetNumberField(TEXT("ActiveTab"), TabVal) ? static_cast<int32>(TabVal) : 0;

	// Rebuild UI
	RebuildManualLayers();
	RebuildAIResultLayers();
	RebuildLocEntries();

	// Restore tab (reuses OnTabSwitch pointer-swap + refresh logic)
	OnTabSwitch(SavedTab);
}

#undef LOCTEXT_NAMESPACE
