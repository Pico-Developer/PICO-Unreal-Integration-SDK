#include "PICOOpenXRConfigEditorWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h"
#include "Containers/Ticker.h"
#include "Engine/RendererSettings.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Styling/AppStyle.h"
#include "Styling/StyleColors.h"
#include "Styling/SlateTypes.h"
#include "UnrealEdMisc.h"
#include "Widgets/SWindow.h"

TSharedRef<SWidget> UPICOOpenXRConfigEditorWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		CreateLayout();
	}

	return Super::RebuildWidget();
}

void UPICOOpenXRConfigEditorWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UPICOOpenXRConfigEditorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		CreateLayout();
	}

	SetDesiredSizeInViewport(FVector2D(1200.0f, 720.0f));

	if (!IsDesignTime())
	{
		TWeakObjectPtr<UPICOOpenXRConfigEditorWidget> WeakThis(this);
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakThis](float)
		{
			if (!WeakThis.IsValid() || !FSlateApplication::IsInitialized())
			{
				return false;
			}

			TSharedPtr<SWidget> CachedWidget = WeakThis->GetCachedWidget();
			if (!CachedWidget.IsValid())
			{
				return false;
			}

			TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(CachedWidget.ToSharedRef());
			if (ParentWindow.IsValid())
			{
				FWindowSizeLimits SizeLimits = ParentWindow->GetSizeLimits();
				SizeLimits.SetMinWidth(720.0f);
				SizeLimits.SetMinHeight(720.0f);
				SizeLimits.SetMaxWidth(720.0f);
				SizeLimits.SetMaxHeight(720.0f);
				ParentWindow->SetSizeLimits(SizeLimits);
			}

			return false;
		}));
	}

	if (TabSwanButton)
	{
		TabSwanButton->OnClicked.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnTabSwan);
	}
	if (TabSwanButtonActive)
	{
		TabSwanButtonActive->OnClicked.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnTabSwan);
	}
	if (TabPICO4Button)
	{
		TabPICO4Button->OnClicked.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnTabPICO4);
	}
	if (TabPICO4ButtonActive)
	{
		TabPICO4ButtonActive->OnClicked.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnTabPICO4);
	}
	if (TabAdvancedButton)
	{
		TabAdvancedButton->OnClicked.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnTabAdvanced);
	}
	if (TabAdvancedButtonActive)
	{
		TabAdvancedButtonActive->OnClicked.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnTabAdvanced);
	}
	if (SwanResolutionCombo)
	{
		SwanResolutionCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnSwanResolutionChanged);
	}
	if (SwanRefreshRateCombo)
	{
		SwanRefreshRateCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnSwanRefreshRateChanged);
	}
	if (SwanFoveationModeCombo)
	{
		SwanFoveationModeCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnSwanFoveationModeChanged);
	}
	if (SwanFoveationLevelCombo)
	{
		SwanFoveationLevelCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnSwanFoveationLevelChanged);
	}
	if (SwanAntiAliasingCombo)
	{
		SwanAntiAliasingCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnSwanAntiAliasingChanged);
	}
	if (SwanMsaaLevelCombo)
	{
		SwanMsaaLevelCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnSwanMsaaLevelChanged);
	}
	if (PICO4ResolutionCombo)
	{
		PICO4ResolutionCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnPICO4ResolutionChanged);
	}
	if (PICO4RefreshRateCombo)
	{
		PICO4RefreshRateCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnPICO4RefreshRateChanged);
	}
	if (PICO4FoveationModeCombo)
	{
		PICO4FoveationModeCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnPICO4FoveationModeChanged);
	}
	if (PICO4FoveationLevelCombo)
	{
		PICO4FoveationLevelCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnPICO4FoveationLevelChanged);
	}
	if (PICO4AntiAliasingCombo)
	{
		PICO4AntiAliasingCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnPICO4AntiAliasingChanged);
	}
	if (PICO4MsaaLevelCombo)
	{
		PICO4MsaaLevelCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnPICO4MsaaLevelChanged);
	}
	if (GlobalShadingPathCombo)
	{
		GlobalShadingPathCombo->OnSelectionChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnGlobalShadingPathChanged);
	}
	if (GlobalMobileHDRToggle)
	{
		GlobalMobileHDRToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnGlobalMobileHDRChanged);
	}
	if (GlobalMovableSpotlightShadowsToggle)
	{
		GlobalMovableSpotlightShadowsToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnGlobalMovableSpotlightShadowsChanged);
	}
	if (SwanResetButton)
	{
		SwanResetButton->OnClicked.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnResetSwan);
	}
	if (PICO4ResetButton)
	{
		PICO4ResetButton->OnClicked.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnResetPICO4);
	}
	if (RestartNowButton)
	{
		RestartNowButton->OnClicked.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnRestartNow);
	}

	if (SwanASWToggle)
	{
		SwanASWToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnAnyToggleChanged);
	}
	if (SwanTonemapSubpassToggle)
	{
		SwanTonemapSubpassToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnAnyToggleChanged);
	}
	if (SwanSuperResolutionToggle)
	{
		SwanSuperResolutionToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnAnyToggleChanged);
	}
	if (SwanSharpenToggle)
	{
		SwanSharpenToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnAnyToggleChanged);
	}
	if (SwanAdaptiveResolutionToggle)
	{
		SwanAdaptiveResolutionToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnAnyToggleChanged);
	}
	if (PICO4SuperResolutionToggle)
	{
		PICO4SuperResolutionToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnAnyToggleChanged);
	}
	if (PICO4ASWToggle)
	{
		PICO4ASWToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnAnyToggleChanged);
	}
	if (PICO4TonemapSubpassToggle)
	{
		PICO4TonemapSubpassToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnAnyToggleChanged);
	}
	if (PICO4SharpenToggle)
	{
		PICO4SharpenToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnAnyToggleChanged);
	}
	if (PICO4AdaptiveResolutionToggle)
	{
		PICO4AdaptiveResolutionToggle->OnCheckStateChanged.AddDynamic(this, &UPICOOpenXRConfigEditorWidget::OnAnyToggleChanged);
	}

	SetActiveTabIndex(0);
	bSuppressAutoApply = true;
	if (!LoadFromDefaultDeviceProfilesIni())
	{
		OnResetSwan();
		OnResetPICO4();
	}
	ApplyMobileHDRConstraints(false);
	bSuppressAutoApply = false;
}

void UPICOOpenXRConfigEditorWidget::CreateLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	const FLinearColor BackgroundColor = FStyleColors::Background.GetSpecifiedColor();
	const FLinearColor SurfaceColor = FStyleColors::Panel.GetSpecifiedColor();
	const FLinearColor PanelColor = FStyleColors::Secondary.GetSpecifiedColor();
	const FLinearColor PrimaryTextColor = FStyleColors::Foreground.GetSpecifiedColor();
	const FLinearColor SecondaryTextColor = FStyleColors::ForegroundHover.GetSpecifiedColor();
	const FLinearColor AccentColor = FStyleColors::AccentBlue.GetSpecifiedColor();
	const float LabelColumnWidth = 220.0f;

	FSlateFontInfo TitleFont = FAppStyle::Get().GetFontStyle("NormalFont");
	TitleFont.Size = 18;
	FSlateFontInfo SectionFont = FAppStyle::Get().GetFontStyle("NormalFont");
	SectionFont.Size = 13;
	FSlateFontInfo LabelFont = FAppStyle::Get().GetFontStyle("NormalFont");
	LabelFont.Size = 11;

	const FButtonStyle& BaseButtonStyle = FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
	FButtonStyle TabButtonStyle = BaseButtonStyle;
	TabButtonStyle.Normal.TintColor = FSlateColor(PanelColor);
	TabButtonStyle.Hovered.TintColor = FSlateColor(AccentColor);
	TabButtonStyle.Pressed.TintColor = FSlateColor(AccentColor);
	FButtonStyle TabButtonActiveStyle = BaseButtonStyle;
	TabButtonActiveStyle.Normal.TintColor = FSlateColor(AccentColor);
	TabButtonActiveStyle.Hovered.TintColor = FSlateColor(AccentColor);
	TabButtonActiveStyle.Pressed.TintColor = FSlateColor(AccentColor);

	UBorder* RootBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RootBorder"));
	RootBorder->SetPadding(FMargin(12.0f));
	RootBorder->SetBrushColor(BackgroundColor);
	WidgetTree->RootWidget = RootBorder;

	UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("RootOverlay"));
	RootBorder->SetContent(RootOverlay);

	UVerticalBox* RootBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("RootBox"));
	if (UOverlaySlot* RootSlot = Cast<UOverlaySlot>(RootOverlay->AddChild(RootBox)))
	{
		RootSlot->SetHorizontalAlignment(HAlign_Fill);
		RootSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UBorder* RestartToastOuter = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RestartToastOuter"));
	RestartToastOuter->SetPadding(FMargin(1.0f));
	RestartToastOuter->SetBrushColor(FStyleColors::Warning.GetSpecifiedColor());
	RestartToastOuter->SetVisibility(ESlateVisibility::Collapsed);
	RestartNoticeWidget = RestartToastOuter;

	UBorder* RestartToastInner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("RestartToastInner"));
	RestartToastInner->SetPadding(FMargin(14.0f, 10.0f));
	RestartToastInner->SetBrushColor(SurfaceColor);
	RestartToastOuter->SetContent(RestartToastInner);

	UHorizontalBox* RestartToastRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RestartToastRow"));
	RestartToastInner->SetContent(RestartToastRow);

	UImage* RestartIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("RestartToastIcon"));
	FSlateBrush IconBrush;
	if (const FSlateBrush* WarningBrush = FAppStyle::Get().GetBrush("Icons.WarningWithColor"))
	{
		IconBrush = *WarningBrush;
		IconBrush.ImageSize = FVector2D(16.0f, 16.0f);
	}
	RestartIcon->SetBrush(IconBrush);
	RestartToastRow->AddChildToHorizontalBox(RestartIcon)->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));

	UTextBlock* RestartMsg = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestartToastMessage"));
	RestartMsg->SetText(FText::FromString(TEXT("You must restart Unreal Editor for your changes to take effect.")));
	RestartMsg->SetFont(LabelFont);
	RestartMsg->SetColorAndOpacity(PrimaryTextColor);
	RestartToastRow->AddChildToHorizontalBox(RestartMsg)->SetPadding(FMargin(0.0f, 0.0f, 12.0f, 0.0f));

	USizeBox* RestartSpacer = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RestartToastSpacer"));
	UHorizontalBoxSlot* RestartSpacerSlot = RestartToastRow->AddChildToHorizontalBox(RestartSpacer);
	RestartSpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	UButton* RestartButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RestartNowButton"));
	RestartButton->SetStyle(BaseButtonStyle);
	RestartButton->SetBackgroundColor(PanelColor);
	UTextBlock* RestartButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestartNowButtonText"));
	RestartButtonText->SetText(FText::FromString(TEXT("Restart Now")));
	RestartButtonText->SetFont(LabelFont);
	RestartButtonText->SetColorAndOpacity(PrimaryTextColor);
	RestartButton->AddChild(RestartButtonText);
	RestartToastRow->AddChildToHorizontalBox(RestartButton);
	RestartNowButton = RestartButton;

	if (UOverlaySlot* ToastSlot = Cast<UOverlaySlot>(RootOverlay->AddChild(RestartToastOuter)))
	{
		ToastSlot->SetHorizontalAlignment(HAlign_Fill);
		ToastSlot->SetVerticalAlignment(VAlign_Bottom);
	}

	UHorizontalBox* TitleRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TitleRow"));
	RootBox->AddChildToVerticalBox(TitleRow)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(FText::FromString(TEXT("PerformancePack Configuration")));
	TitleText->SetFont(TitleFont);
	TitleText->SetColorAndOpacity(PrimaryTextColor);
	TitleRow->AddChildToHorizontalBox(TitleText)->SetPadding(FMargin(4.0f, 0.0f));

	UHorizontalBox* TabRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("TabRow"));
	RootBox->AddChildToVerticalBox(TabRow)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));

	auto CreateTabButtonSwitcher = [&](const FName& Name, const FString& Label, UButton*& OutButton, UButton*& OutActiveButton, UWidgetSwitcher*& OutSwitcher)
	{
		UWidgetSwitcher* Switcher = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), *FString::Printf(TEXT("%sSwitcher"), *Name.ToString()));

		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		Button->SetStyle(TabButtonStyle);
		Button->SetBackgroundColor(PanelColor);
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("%sText"), *Name.ToString()));
		Text->SetText(FText::FromString(Label));
		Text->SetFont(LabelFont);
		Text->SetColorAndOpacity(PrimaryTextColor);
		Button->AddChild(Text);

		UButton* ActiveButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *FString::Printf(TEXT("%sActive"), *Name.ToString()));
		ActiveButton->SetStyle(TabButtonActiveStyle);
		ActiveButton->SetBackgroundColor(AccentColor);
		UTextBlock* ActiveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("%sActiveText"), *Name.ToString()));
		ActiveText->SetText(FText::FromString(Label));
		ActiveText->SetFont(LabelFont);
		ActiveText->SetColorAndOpacity(PrimaryTextColor);
		ActiveButton->AddChild(ActiveText);

		Switcher->AddChild(Button);
		Switcher->AddChild(ActiveButton);

		TabRow->AddChildToHorizontalBox(Switcher)->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
		OutButton = Button;
		OutActiveButton = ActiveButton;
		OutSwitcher = Switcher;
	};

	CreateTabButtonSwitcher(TEXT("TabSwan"), TEXT("Swan Series"), TabSwanButton, TabSwanButtonActive, TabSwanSwitcher);
	CreateTabButtonSwitcher(TEXT("TabPICO4"), TEXT("PICO 4 Series"), TabPICO4Button, TabPICO4ButtonActive, TabPICO4Switcher);
	CreateTabButtonSwitcher(TEXT("TabAdvanced"), TEXT("Advanced Settings"), TabAdvancedButton, TabAdvancedButtonActive, TabAdvancedSwitcher);

	UScrollBox* VerticalScroll = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("MainVerticalScroll"));
	VerticalScroll->SetOrientation(EOrientation::Orient_Vertical);
	VerticalScroll->SetScrollBarVisibility(ESlateVisibility::Collapsed);
	VerticalScroll->SetAlwaysShowScrollbar(false);
	VerticalScroll->SetAlwaysShowScrollbarTrack(false);
	VerticalScroll->SetScrollbarThickness(FVector2D(0.0f, 0.0f));
	UVerticalBoxSlot* ScrollSlot = RootBox->AddChildToVerticalBox(VerticalScroll);
	ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

	UBorder* ContentBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("ContentBorder"));
	ContentBorder->SetPadding(FMargin(12.0f));
	ContentBorder->SetBrushColor(SurfaceColor);
	if (UScrollBoxSlot* ContentSlot = Cast<UScrollBoxSlot>(VerticalScroll->AddChild(ContentBorder)))
	{
		ContentSlot->SetHorizontalAlignment(HAlign_Fill);
		ContentSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UVerticalBox* ContentBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ContentBox"));
	ContentBorder->SetContent(ContentBox);

	MainSwitcher = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("MainSwitcher"));
	UVerticalBoxSlot* SwitcherSlot = ContentBox->AddChildToVerticalBox(MainSwitcher);
	SwitcherSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));

	auto CreateSectionBox = [&](const FName& Name, const FString& Title, UVerticalBox* ParentBox)
	{
		UBorder* SectionBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		SectionBorder->SetPadding(FMargin(10.0f));
		SectionBorder->SetBrushColor(PanelColor);

		UVerticalBox* SectionContent = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *FString::Printf(TEXT("%sContent"), *Name.ToString()));
		SectionBorder->SetContent(SectionContent);

		UTextBlock* SectionTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("%sTitle"), *Name.ToString()));
		SectionTitle->SetText(FText::FromString(Title));
		SectionTitle->SetFont(SectionFont);
		SectionTitle->SetColorAndOpacity(PrimaryTextColor);
		SectionContent->AddChildToVerticalBox(SectionTitle)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));

		ParentBox->AddChildToVerticalBox(SectionBorder)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 10.0f));
		return SectionContent;
	};

	auto CreateLabel = [&](const FString& TextValue, const FLinearColor& Color)
	{
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), NAME_None);
		Text->SetText(FText::FromString(TextValue));
		Text->SetFont(LabelFont);
		Text->SetColorAndOpacity(Color);
		return Text;
	};

	auto CreateFixedLabel = [&](const FString& TextValue, const FLinearColor& Color)
	{
		USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), NAME_None);
		Box->SetWidthOverride(LabelColumnWidth);
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), NAME_None);
		Text->SetText(FText::FromString(TextValue));
		Text->SetFont(LabelFont);
		Text->SetColorAndOpacity(Color);
		Text->SetAutoWrapText(true);
		Text->SetWrapTextAt(LabelColumnWidth);
		Box->AddChild(Text);
		return Box;
	};

	auto AddToggleRow = [&](UVerticalBox* Parent, const FString& Label, bool bChecked, UCheckBox*& OutToggle)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), NAME_None);
		UCheckBox* Toggle = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), NAME_None);
		Toggle->SetIsChecked(bChecked);
		Row->AddChildToHorizontalBox(Toggle)->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
		Row->AddChildToHorizontalBox(CreateLabel(Label, PrimaryTextColor));
		Parent->AddChildToVerticalBox(Row)->SetPadding(FMargin(0.0f, 4.0f));
		OutToggle = Toggle;
	};

	auto AddBasicRow = [&](UGridPanel* Grid, int32 Row, const FString& Label, const TArray<FString>& Options, UComboBoxString*& OutCombo, UTextBlock*& OutStatus)
	{
		Grid->AddChildToGrid(CreateFixedLabel(Label, PrimaryTextColor), Row, 0)->SetPadding(FMargin(4.0f, 4.0f));
		UComboBoxString* Combo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), NAME_None);
		for (const FString& Option : Options)
		{
			Combo->AddOption(Option);
		}
		if (Options.Num() > 0)
		{
			Combo->SetSelectedOption(Options[0]);
		}
		Grid->AddChildToGrid(Combo, Row, 1)->SetPadding(FMargin(4.0f, 4.0f));
		UTextBlock* StatusLabel = CreateLabel(TEXT(""), SecondaryTextColor);
		Grid->AddChildToGrid(StatusLabel, Row, 2)->SetPadding(FMargin(4.0f, 4.0f));
		OutCombo = Combo;
		OutStatus = StatusLabel;
	};

	auto AddBasicCheckRow = [&](UGridPanel* Grid, int32 Row, const FString& Label, bool bChecked, UCheckBox*& OutToggle, UTextBlock*& OutStatus)
	{
		Grid->AddChildToGrid(CreateFixedLabel(Label, PrimaryTextColor), Row, 0)->SetPadding(FMargin(4.0f, 4.0f));
		UCheckBox* Toggle = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), NAME_None);
		Toggle->SetIsChecked(bChecked);
		Grid->AddChildToGrid(Toggle, Row, 1)->SetPadding(FMargin(4.0f, 4.0f));
		UTextBlock* StatusLabel = CreateLabel(TEXT(""), SecondaryTextColor);
		Grid->AddChildToGrid(StatusLabel, Row, 2)->SetPadding(FMargin(4.0f, 4.0f));
		OutToggle = Toggle;
		OutStatus = StatusLabel;
	};

	auto CreateDeviceTab = [&](const FName& Name, const FString& Title, bool bIsSwan)
	{
		UBorder* TabBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
		TabBorder->SetPadding(FMargin(12.0f));
		TabBorder->SetBrushColor(SurfaceColor);

		UVerticalBox* TabBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), *FString::Printf(TEXT("%sBox"), *Name.ToString()));
		TabBorder->SetContent(TabBox);

		UTextBlock* TabTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("%sTitle"), *Name.ToString()));
		TabTitle->SetText(FText::FromString(Title));
		TabTitle->SetFont(SectionFont);
		TabTitle->SetColorAndOpacity(PrimaryTextColor);
		TabBox->AddChildToVerticalBox(TabTitle)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		UVerticalBox* BasicSection = CreateSectionBox(*FString::Printf(TEXT("%sBasicSection"), *Name.ToString()), TEXT("Basic Settings"), TabBox);
		UGridPanel* BasicGrid = WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass(), *FString::Printf(TEXT("%sBasicGrid"), *Name.ToString()));
		BasicSection->AddChildToVerticalBox(BasicGrid);


		BasicGrid->AddChildToGrid(CreateFixedLabel(TEXT("Setting"), SecondaryTextColor), 0, 0)->SetPadding(FMargin(4.0f, 2.0f));
	BasicGrid->AddChildToGrid(CreateLabel(TEXT("Current"), SecondaryTextColor), 0, 1)->SetPadding(FMargin(4.0f, 2.0f));
	BasicGrid->AddChildToGrid(CreateLabel(TEXT("Status"), SecondaryTextColor), 0, 2)->SetPadding(FMargin(4.0f, 2.0f));

		UComboBoxString*& ResolutionCombo = bIsSwan ? SwanResolutionCombo : PICO4ResolutionCombo;
		UComboBoxString*& RefreshRateCombo = bIsSwan ? SwanRefreshRateCombo : PICO4RefreshRateCombo;
		UComboBoxString*& FoveationModeCombo = bIsSwan ? SwanFoveationModeCombo : PICO4FoveationModeCombo;
		UComboBoxString*& FoveationLevelCombo = bIsSwan ? SwanFoveationLevelCombo : PICO4FoveationLevelCombo;
		UComboBoxString*& AntiAliasingCombo = bIsSwan ? SwanAntiAliasingCombo : PICO4AntiAliasingCombo;

		UTextBlock*& ResolutionStatus = bIsSwan ? SwanResolutionStatus : PICO4ResolutionStatus;
		UTextBlock*& RefreshRateStatus = bIsSwan ? SwanRefreshRateStatus : PICO4RefreshRateStatus;
		UTextBlock*& FoveationModeStatus = bIsSwan ? SwanFoveationModeStatus : PICO4FoveationModeStatus;
		UTextBlock*& AntiAliasingStatus = bIsSwan ? SwanAntiAliasingStatus : PICO4AntiAliasingStatus;

		const TArray<FString> ResolutionOptions = bIsSwan ? TArray<FString>({ TEXT("2.5K"), TEXT("2K"), TEXT("4K") }) : TArray<FString>({ TEXT("2K"), TEXT("2.5K") });
		AddBasicRow(BasicGrid, 1, TEXT("Resolution"), ResolutionOptions, ResolutionCombo, ResolutionStatus);
	AddBasicRow(BasicGrid, 2, TEXT("Refresh Rate"), { TEXT("90Hz"), TEXT("72Hz") }, RefreshRateCombo, RefreshRateStatus);
	AddBasicRow(BasicGrid, 3, TEXT("Foveation Mode"), { TEXT("Off"), TEXT("Fixed Foveation"), TEXT("Eye-tracked Foveation") }, FoveationModeCombo, FoveationModeStatus);
	UWidget*& FoveationLevelLabel = bIsSwan ? SwanFoveationLevelLabel : PICO4FoveationLevelLabel;
	FoveationLevelLabel = CreateFixedLabel(TEXT("Foveation Level"), PrimaryTextColor);
		FoveationLevelLabel->SetVisibility(ESlateVisibility::Collapsed);
		BasicGrid->AddChildToGrid(FoveationLevelLabel, 4, 0)->SetPadding(FMargin(4.0f, 4.0f));

		FoveationLevelCombo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), *FString::Printf(TEXT("%sFoveationLevelCombo"), *Name.ToString()));
		FoveationLevelCombo->AddOption(TEXT("Performance"));
	FoveationLevelCombo->AddOption(TEXT("Balanced"));
	FoveationLevelCombo->AddOption(TEXT("Quality"));
	FoveationLevelCombo->SetSelectedOption(TEXT("Balanced"));
		FoveationLevelCombo->SetVisibility(ESlateVisibility::Collapsed);
		BasicGrid->AddChildToGrid(FoveationLevelCombo, 4, 1)->SetPadding(FMargin(4.0f, 4.0f));

		UTextBlock*& FoveationLevelStatus = bIsSwan ? SwanFoveationLevelStatus : PICO4FoveationLevelStatus;
		FoveationLevelStatus = CreateLabel(TEXT(""), SecondaryTextColor);
		FoveationLevelStatus->SetVisibility(ESlateVisibility::Collapsed);
		BasicGrid->AddChildToGrid(FoveationLevelStatus, 4, 2)->SetPadding(FMargin(4.0f, 4.0f));

		AddBasicRow(BasicGrid, 5, TEXT("Anti-Aliasing"), { TEXT("MSAA"), TEXT("TAA"), TEXT("FXAA") }, AntiAliasingCombo, AntiAliasingStatus);

		UWidget*& MsaaLevelLabel = bIsSwan ? SwanMsaaLevelLabel : PICO4MsaaLevelLabel;
		MsaaLevelLabel = CreateFixedLabel(TEXT("MSAA Level"), PrimaryTextColor);
		MsaaLevelLabel->SetVisibility(ESlateVisibility::Collapsed);
		BasicGrid->AddChildToGrid(MsaaLevelLabel, 6, 0)->SetPadding(FMargin(4.0f, 4.0f));

		UComboBoxString*& MsaaLevelComboRef = bIsSwan ? SwanMsaaLevelCombo : PICO4MsaaLevelCombo;
		MsaaLevelComboRef = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), *FString::Printf(TEXT("%sMsaaLevelCombo"), *Name.ToString()));
		MsaaLevelComboRef->AddOption(TEXT("2"));
		MsaaLevelComboRef->AddOption(TEXT("4"));
		MsaaLevelComboRef->AddOption(TEXT("8"));
		MsaaLevelComboRef->SetSelectedOption(TEXT("4"));
		MsaaLevelComboRef->SetVisibility(ESlateVisibility::Collapsed);
		BasicGrid->AddChildToGrid(MsaaLevelComboRef, 6, 1)->SetPadding(FMargin(4.0f, 4.0f));

		UTextBlock*& MsaaLevelStatusRef = bIsSwan ? SwanMsaaLevelStatus : PICO4MsaaLevelStatus;
		MsaaLevelStatusRef = CreateLabel(TEXT(""), SecondaryTextColor);
		MsaaLevelStatusRef->SetVisibility(ESlateVisibility::Collapsed);
		BasicGrid->AddChildToGrid(MsaaLevelStatusRef, 6, 2)->SetPadding(FMargin(4.0f, 4.0f));

		UCheckBox*& SuperResToggle = bIsSwan ? SwanSuperResolutionToggle : PICO4SuperResolutionToggle;
		UCheckBox*& SharpenToggle = bIsSwan ? SwanSharpenToggle : PICO4SharpenToggle;
		UCheckBox*& AdaptiveToggle = bIsSwan ? SwanAdaptiveResolutionToggle : PICO4AdaptiveResolutionToggle;
		UCheckBox*& TonemapSubpassToggle = bIsSwan ? SwanTonemapSubpassToggle : PICO4TonemapSubpassToggle;
		UTextBlock* SuperResStatus = nullptr;
		UTextBlock* SharpenStatus = nullptr;
		UTextBlock* AdaptiveStatus = nullptr;
		AddBasicCheckRow(BasicGrid, 7, TEXT("Super Resolution"), true, SuperResToggle, SuperResStatus);
	AddBasicCheckRow(BasicGrid, 8, TEXT("Sharpen"), true, SharpenToggle, SharpenStatus);
	AddBasicCheckRow(BasicGrid, 9, TEXT("Adaptive Resolution"), true, AdaptiveToggle, AdaptiveStatus);
		UTextBlock* TonemapSubpassStatus = nullptr;
		AddBasicCheckRow(BasicGrid, 10, TEXT("TonemapSubpass"), false, TonemapSubpassToggle, TonemapSubpassStatus);

		UHorizontalBox* FooterRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), *FString::Printf(TEXT("%sFooterRow"), *Name.ToString()));
		TabBox->AddChildToVerticalBox(FooterRow)->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));

		USizeBox* FooterSpacer = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), *FString::Printf(TEXT("%sFooterSpacer"), *Name.ToString()));
		UHorizontalBoxSlot* SpacerSlot = FooterRow->AddChildToHorizontalBox(FooterSpacer);
		SpacerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

		UButton* ResetButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *FString::Printf(TEXT("%sResetButton"), *Name.ToString()));
		ResetButton->SetStyle(TabButtonStyle);
		ResetButton->SetBackgroundColor(PanelColor);
		UTextBlock* ResetText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *FString::Printf(TEXT("%sResetText"), *Name.ToString()));
		ResetText->SetText(FText::FromString(TEXT("Reset to Default")));
		ResetText->SetFont(LabelFont);
		ResetText->SetColorAndOpacity(PrimaryTextColor);
		ResetButton->AddChild(ResetText);
		FooterRow->AddChildToHorizontalBox(ResetButton);

		if (bIsSwan)
		{
			SwanResetButton = ResetButton;
		}
		else
		{
			PICO4ResetButton = ResetButton;
		}

		return TabBorder;
	};

	auto CreateAdvancedTab = [&]()
	{
		UBorder* TabBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("AdvancedTab"));
		TabBorder->SetPadding(FMargin(12.0f));
		TabBorder->SetBrushColor(SurfaceColor);

		UVerticalBox* TabBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("AdvancedTabBox"));
		TabBorder->SetContent(TabBox);

		UTextBlock* TabTitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("AdvancedTabTitle"));
		TabTitle->SetText(FText::FromString(TEXT("Advanced Settings")));
		TabTitle->SetFont(SectionFont);
		TabTitle->SetColorAndOpacity(PrimaryTextColor);
		TabBox->AddChildToVerticalBox(TabTitle)->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));

		UVerticalBox* AdvancedSection = CreateSectionBox(TEXT("GlobalAdvancedSection"), TEXT("General Advanced Settings"), TabBox);
		UGridPanel* AdvancedGrid = WidgetTree->ConstructWidget<UGridPanel>(UGridPanel::StaticClass(), TEXT("GlobalAdvancedGrid"));
		AdvancedSection->AddChildToVerticalBox(AdvancedGrid);
		AdvancedGrid->SetColumnFill(1, 1.0f);

		auto AddProjectSettingsComboRow = [&](int32 Row, const FString& Label, const TArray<FString>& Options, UComboBoxString*& OutCombo)
		{
			AdvancedGrid->AddChildToGrid(CreateFixedLabel(Label, PrimaryTextColor), Row, 0)->SetPadding(FMargin(4.0f, 6.0f));
			UComboBoxString* Combo = WidgetTree->ConstructWidget<UComboBoxString>(UComboBoxString::StaticClass(), NAME_None);
			for (const FString& Option : Options)
			{
				Combo->AddOption(Option);
			}
			if (Options.Num() > 0)
			{
				Combo->SetSelectedOption(Options[0]);
			}
			AdvancedGrid->AddChildToGrid(Combo, Row, 1)->SetPadding(FMargin(4.0f, 6.0f));
			OutCombo = Combo;
		};

		auto AddProjectSettingsCheckRow = [&](int32 Row, const FString& Label, bool bChecked, UCheckBox*& OutToggle)
		{
			AdvancedGrid->AddChildToGrid(CreateFixedLabel(Label, PrimaryTextColor), Row, 0)->SetPadding(FMargin(4.0f, 6.0f));
			UCheckBox* Toggle = WidgetTree->ConstructWidget<UCheckBox>(UCheckBox::StaticClass(), NAME_None);
			Toggle->SetIsChecked(bChecked);
			AdvancedGrid->AddChildToGrid(Toggle, Row, 1)->SetPadding(FMargin(4.0f, 6.0f));
			OutToggle = Toggle;
		};

		AddProjectSettingsComboRow(0, TEXT("Mobile Shading"), { TEXT("Forward Shading"), TEXT("Deferred Shading") }, GlobalShadingPathCombo);
		AddProjectSettingsCheckRow(1, TEXT("Mobile HDR"), false, GlobalMobileHDRToggle);
		AddProjectSettingsCheckRow(2, TEXT("Support Movable SpotlightShadows"), false, GlobalMovableSpotlightShadowsToggle);

		return TabBorder;
	};

	if (UWidgetSwitcherSlot* SwanTabSlot = Cast<UWidgetSwitcherSlot>(MainSwitcher->AddChild(CreateDeviceTab(TEXT("SwanTab"), TEXT("Swan Series Device Configuration"), true))))
	{
		SwanTabSlot->SetHorizontalAlignment(HAlign_Fill);
		SwanTabSlot->SetVerticalAlignment(VAlign_Fill);
	}
	if (UWidgetSwitcherSlot* PICO4TabSlot = Cast<UWidgetSwitcherSlot>(MainSwitcher->AddChild(CreateDeviceTab(TEXT("PICO4Tab"), TEXT("PICO 4 Series Device Configuration"), false))))
	{
		PICO4TabSlot->SetHorizontalAlignment(HAlign_Fill);
		PICO4TabSlot->SetVerticalAlignment(VAlign_Fill);
	}
	if (UWidgetSwitcherSlot* AdvancedTabSlot = Cast<UWidgetSwitcherSlot>(MainSwitcher->AddChild(CreateAdvancedTab())))
	{
		AdvancedTabSlot->SetHorizontalAlignment(HAlign_Fill);
		AdvancedTabSlot->SetVerticalAlignment(VAlign_Fill);
	}
}

void UPICOOpenXRConfigEditorWidget::SetActiveTabIndex(int32 Index)
{
	if (MainSwitcher)
	{
		MainSwitcher->SetActiveWidgetIndex(Index);
	}
	if (TabSwanSwitcher)
	{
		TabSwanSwitcher->SetActiveWidgetIndex(Index == 0 ? 1 : 0);
	}
	if (TabPICO4Switcher)
	{
		TabPICO4Switcher->SetActiveWidgetIndex(Index == 1 ? 1 : 0);
	}
	if (TabAdvancedSwitcher)
	{
		TabAdvancedSwitcher->SetActiveWidgetIndex(Index == 2 ? 1 : 0);
	}
}

void UPICOOpenXRConfigEditorWidget::OnTabSwan()
{
	SetActiveTabIndex(0);
}

void UPICOOpenXRConfigEditorWidget::OnTabPICO4()
{
	SetActiveTabIndex(1);
}

void UPICOOpenXRConfigEditorWidget::OnTabAdvanced()
{
	SetActiveTabIndex(2);
}

void UPICOOpenXRConfigEditorWidget::UpdateStatusLabel(UTextBlock* StatusLabel, const FString& StatusText, const FLinearColor& StatusColor)
{
	if (!StatusLabel)
	{
		return;
	}
	StatusLabel->SetText(FText::FromString(StatusText));
	StatusLabel->SetColorAndOpacity(StatusColor);
}

void UPICOOpenXRConfigEditorWidget::UpdateSwanStatus()
{
	UpdateStatusLabel(SwanResolutionStatus, TEXT(""), FLinearColor::White);
	UpdateStatusLabel(SwanRefreshRateStatus, TEXT(""), FLinearColor::White);
	UpdateStatusLabel(SwanFoveationModeStatus, TEXT(""), FLinearColor::White);
	UpdateStatusLabel(SwanAntiAliasingStatus, TEXT(""), FLinearColor::White);

	const FString FoveationMode = SwanFoveationModeCombo ? SwanFoveationModeCombo->GetSelectedOption() : TEXT("");
	const bool bFoveationLevelVisible = FoveationMode != TEXT("Off");
	const ESlateVisibility FoveationLevelVis = bFoveationLevelVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (SwanFoveationLevelLabel)
	{
		SwanFoveationLevelLabel->SetVisibility(FoveationLevelVis);
	}
	if (SwanFoveationLevelCombo)
	{
		SwanFoveationLevelCombo->SetVisibility(FoveationLevelVis);
	}
	if (SwanFoveationLevelStatus)
	{
		SwanFoveationLevelStatus->SetVisibility(FoveationLevelVis);
	}
	UpdateStatusLabel(SwanFoveationLevelStatus, TEXT(""), FLinearColor::White);

	const FString AntiAliasing = SwanAntiAliasingCombo ? SwanAntiAliasingCombo->GetSelectedOption() : TEXT("");
	const bool bMsaaLevelVisible = AntiAliasing == TEXT("MSAA");
	const ESlateVisibility MsaaLevelVis = bMsaaLevelVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (SwanMsaaLevelLabel)
	{
		SwanMsaaLevelLabel->SetVisibility(MsaaLevelVis);
	}
	if (SwanMsaaLevelCombo)
	{
		SwanMsaaLevelCombo->SetVisibility(MsaaLevelVis);
	}
	if (SwanMsaaLevelStatus)
	{
		SwanMsaaLevelStatus->SetVisibility(MsaaLevelVis);
	}
	UpdateStatusLabel(SwanMsaaLevelStatus, TEXT(""), FLinearColor::White);
}

void UPICOOpenXRConfigEditorWidget::UpdatePICO4Status()
{
	UpdateStatusLabel(PICO4ResolutionStatus, TEXT(""), FLinearColor::White);
	UpdateStatusLabel(PICO4RefreshRateStatus, TEXT(""), FLinearColor::White);
	UpdateStatusLabel(PICO4FoveationModeStatus, TEXT(""), FLinearColor::White);
	UpdateStatusLabel(PICO4AntiAliasingStatus, TEXT(""), FLinearColor::White);

	const FString FoveationMode = PICO4FoveationModeCombo ? PICO4FoveationModeCombo->GetSelectedOption() : TEXT("");
	const bool bFoveationLevelVisible = FoveationMode != TEXT("Off");
	const ESlateVisibility FoveationLevelVis = bFoveationLevelVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (PICO4FoveationLevelLabel)
	{
		PICO4FoveationLevelLabel->SetVisibility(FoveationLevelVis);
	}
	if (PICO4FoveationLevelCombo)
	{
		PICO4FoveationLevelCombo->SetVisibility(FoveationLevelVis);
	}
	if (PICO4FoveationLevelStatus)
	{
		PICO4FoveationLevelStatus->SetVisibility(FoveationLevelVis);
	}
	UpdateStatusLabel(PICO4FoveationLevelStatus, TEXT(""), FLinearColor::White);

	const FString AntiAliasing = PICO4AntiAliasingCombo ? PICO4AntiAliasingCombo->GetSelectedOption() : TEXT("");
	const bool bMsaaLevelVisible = AntiAliasing == TEXT("MSAA");
	const ESlateVisibility MsaaLevelVis = bMsaaLevelVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	if (PICO4MsaaLevelLabel)
	{
		PICO4MsaaLevelLabel->SetVisibility(MsaaLevelVis);
	}
	if (PICO4MsaaLevelCombo)
	{
		PICO4MsaaLevelCombo->SetVisibility(MsaaLevelVis);
	}
	if (PICO4MsaaLevelStatus)
	{
		PICO4MsaaLevelStatus->SetVisibility(MsaaLevelVis);
	}
	UpdateStatusLabel(PICO4MsaaLevelStatus, TEXT(""), FLinearColor::White);
}

void UPICOOpenXRConfigEditorWidget::ApplyMobileHDRConstraints(bool bMarkRestart)
{
	if (!GlobalMobileHDRToggle || !GlobalShadingPathCombo)
	{
		return;
	}

	const bool bSwanTonemapSubpass = SwanTonemapSubpassToggle && SwanTonemapSubpassToggle->IsChecked();
	const bool bPICO4TonemapSubpass = PICO4TonemapSubpassToggle && PICO4TonemapSubpassToggle->IsChecked();

	const bool bRequireMobileHDR = bSwanTonemapSubpass || bPICO4TonemapSubpass;

	if (bRequireMobileHDR)
	{
		const bool bNeedChange = !GlobalMobileHDRToggle->IsChecked();
		bUpdatingMobileHDRConstraint = true;
		GlobalMobileHDRToggle->SetIsChecked(true);
		bUpdatingMobileHDRConstraint = false;
		GlobalMobileHDRToggle->SetIsEnabled(false);
		if (bMarkRestart && bNeedChange)
		{
			bNeedRestartEditor = true;
			if (RestartNoticeWidget)
			{
				RestartNoticeWidget->SetVisibility(ESlateVisibility::Visible);
			}
			ScheduleAutoApply();
		}
	}
	else
	{
		GlobalMobileHDRToggle->SetIsEnabled(true);
	}
}

void UPICOOpenXRConfigEditorWidget::OnSwanResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdateSwanStatus();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnSwanRefreshRateChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdateSwanStatus();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnSwanShadingPathChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdateSwanStatus();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnSwanMobileHDRChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdateSwanStatus();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnSwanFoveationModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdateSwanStatus();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnSwanFoveationLevelChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdateSwanStatus();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnSwanAntiAliasingChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdateSwanStatus();
	ApplyMobileHDRConstraints(true);
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnSwanMsaaLevelChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdateSwanStatus();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnPICO4ResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdatePICO4Status();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnPICO4RefreshRateChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdatePICO4Status();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnPICO4ShadingPathChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdatePICO4Status();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnPICO4MobileHDRChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdatePICO4Status();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnPICO4FoveationModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdatePICO4Status();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnPICO4FoveationLevelChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdatePICO4Status();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnPICO4AntiAliasingChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdatePICO4Status();
	ApplyMobileHDRConstraints(true);
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnPICO4MsaaLevelChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdatePICO4Status();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnSwanQuickPresetChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!bUpdatingSwanPreset)
	{
		ApplySwanPreset(SelectedItem);
	}
}

void UPICOOpenXRConfigEditorWidget::OnPICO4QuickPresetChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (!bUpdatingPICO4Preset)
	{
		ApplyPICO4Preset(SelectedItem);
	}
}

void UPICOOpenXRConfigEditorWidget::OnGlobalShadingPathChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	bNeedRestartEditor = true;
	if (RestartNoticeWidget)
	{
		RestartNoticeWidget->SetVisibility(ESlateVisibility::Visible);
	}
	ApplyMobileHDRConstraints(true);
	UpdateSwanStatus();
	UpdatePICO4Status();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnGlobalMobileHDRChanged(bool bIsChecked)
{
	if (bUpdatingMobileHDRConstraint)
	{
		return;
	}
	bNeedRestartEditor = true;
	if (RestartNoticeWidget)
	{
		RestartNoticeWidget->SetVisibility(ESlateVisibility::Visible);
	}
	ApplyMobileHDRConstraints(true);
	UpdateSwanStatus();
	UpdatePICO4Status();
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnGlobalMovableSpotlightShadowsChanged(bool bIsChecked)
{
	bNeedRestartEditor = true;
	if (RestartNoticeWidget)
	{
		RestartNoticeWidget->SetVisibility(ESlateVisibility::Visible);
	}
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnAnyToggleChanged(bool bIsChecked)
{
	UpdateSwanStatus();
	UpdatePICO4Status();
	ApplyMobileHDRConstraints(true);
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnRestartNow()
{
	bNeedRestartEditor = false;
	if (RestartNoticeWidget)
	{
		RestartNoticeWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	const bool bWarn = false;
	FUnrealEdMisc::Get().RestartEditor(bWarn);
}

void UPICOOpenXRConfigEditorWidget::OnResetSwan()
{
	if (SwanResolutionCombo)
	{
		SwanResolutionCombo->SetSelectedOption(TEXT("2.5K"));
	}
	if (SwanRefreshRateCombo)
	{
		SwanRefreshRateCombo->SetSelectedOption(TEXT("90Hz"));
	}
	if (SwanFoveationModeCombo)
	{
		SwanFoveationModeCombo->SetSelectedOption(TEXT("Eye-tracked Foveation"));
	}
	if (SwanFoveationLevelCombo)
	{
		SwanFoveationLevelCombo->SetSelectedOption(TEXT("Quality"));
	}
	if (SwanAntiAliasingCombo)
	{
		SwanAntiAliasingCombo->SetSelectedOption(TEXT("MSAA"));
	}
	if (SwanMsaaLevelCombo)
	{
		SwanMsaaLevelCombo->SetSelectedOption(TEXT("4"));
	}
	if (SwanSuperResolutionToggle)
	{
		SwanSuperResolutionToggle->SetIsChecked(true);
	}
	if (SwanSharpenToggle)
	{
		SwanSharpenToggle->SetIsChecked(true);
	}
	if (SwanAdaptiveResolutionToggle)
	{
		SwanAdaptiveResolutionToggle->SetIsChecked(true);
	}

	UpdateSwanStatus();
	ApplyMobileHDRConstraints(false);
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::OnResetPICO4()
{
	if (PICO4ResolutionCombo)
	{
		PICO4ResolutionCombo->SetSelectedOption(TEXT("2K"));
	}
	if (PICO4RefreshRateCombo)
	{
		PICO4RefreshRateCombo->SetSelectedOption(TEXT("90Hz"));
	}
	if (PICO4FoveationModeCombo)
	{
		PICO4FoveationModeCombo->SetSelectedOption(TEXT("Eye-tracked Foveation"));
	}
	if (PICO4FoveationLevelCombo)
	{
		PICO4FoveationLevelCombo->SetSelectedOption(TEXT("Quality"));
	}
	if (PICO4AntiAliasingCombo)
	{
		PICO4AntiAliasingCombo->SetSelectedOption(TEXT("MSAA"));
	}
	if (PICO4MsaaLevelCombo)
	{
		PICO4MsaaLevelCombo->SetSelectedOption(TEXT("4"));
	}
	if (PICO4SuperResolutionToggle)
	{
		PICO4SuperResolutionToggle->SetIsChecked(true);
	}
	if (PICO4SharpenToggle)
	{
		PICO4SharpenToggle->SetIsChecked(true);
	}
	if (PICO4AdaptiveResolutionToggle)
	{
		PICO4AdaptiveResolutionToggle->SetIsChecked(true);
	}

	UpdatePICO4Status();
	ApplyMobileHDRConstraints(false);
	ScheduleAutoApply();
}

void UPICOOpenXRConfigEditorWidget::ApplyConfig(bool bShowDialog)
{
	const bool bWroteEngine = WriteDefaultEngineIni();
	const bool bWroteProfiles = WriteDefaultDeviceProfilesIni();
	const bool bSuccess = bWroteEngine && bWroteProfiles;
	if (bShowDialog)
	{
		const FString FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectConfigDir() / TEXT("DefaultDeviceProfiles.ini"));
	const FString EnginePath = FPaths::ConvertRelativePathToFull(FPaths::EngineConfigDir() / TEXT("BaseDeviceProfiles.ini"));
	FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(bSuccess ? FString::Printf(TEXT("Generated: %s\nUpdated: %s"), *FilePath, *EnginePath) : FString::Printf(TEXT("Failed to generate: %s\nor failed to update: %s"), *FilePath, *EnginePath)));
	}
	else if (!bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("PerformancePack: apply failed to write configs."));
	}
}

void UPICOOpenXRConfigEditorWidget::ScheduleAutoApply()
{
	if (bSuppressAutoApply)
	{
		return;
	}

	AutoApplyTriggerTime = FPlatformTime::Seconds() + 0.25;

	if (bAutoApplyPending)
	{
		return;
	}

	bAutoApplyPending = true;
	TWeakObjectPtr<UPICOOpenXRConfigEditorWidget> WeakThis(this);
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakThis](float)
	{
		if (!WeakThis.IsValid())
		{
			return false;
		}

		const double Now = FPlatformTime::Seconds();
		if (!WeakThis->bAutoApplyPending)
		{
			return false;
		}

		if (Now < WeakThis->AutoApplyTriggerTime)
		{
			return true;
		}

		WeakThis->bAutoApplyPending = false;
		WeakThis->ApplyConfig(false);
		return false;
	}));
}

bool UPICOOpenXRConfigEditorWidget::LoadFromDefaultDeviceProfilesIni()
{
	const FString IniPath = FPaths::ProjectConfigDir() / TEXT("DefaultDeviceProfiles.ini");
	if (!IFileManager::Get().FileExists(*IniPath))
	{
		return false;
	}

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *IniPath))
	{
		return false;
	}

	auto ReadSectionCVars = [&](const FString& SectionName) -> TMap<FString, FString>
	{
		TMap<FString, FString> Result;
		const FString Header = FString::Printf(TEXT("[%s DeviceProfile]"), *SectionName);
		int32 StartIndex = INDEX_NONE;
		for (int32 Index = 0; Index < Lines.Num(); Index++)
		{
			if (Lines[Index].TrimStartAndEnd() == Header)
			{
				StartIndex = Index + 1;
				break;
			}
		}
		if (StartIndex == INDEX_NONE)
		{
			return Result;
		}

		for (int32 Index = StartIndex; Index < Lines.Num(); Index++)
		{
			const FString Line = Lines[Index].TrimStartAndEnd();
			if (Line.StartsWith(TEXT("[")) && Line.EndsWith(TEXT("]")))
			{
				break;
			}
			if (!Line.StartsWith(TEXT("+CVars=")))
			{
				continue;
			}

			FString Payload = Line.Mid(7);
			FString Key;
			FString Value;
			if (!Payload.Split(TEXT("="), &Key, &Value))
			{
				continue;
			}
			Result.Add(Key, Value);
		}
		return Result;
	};

	const TMap<FString, FString> SwanCVars = ReadSectionCVars(TEXT("Swan"));
	const TMap<FString, FString> PICO4CVars = ReadSectionCVars(TEXT("PICO4"));
	if (SwanCVars.Num() == 0 && PICO4CVars.Num() == 0)
	{
		return false;
	}

	auto SetComboIfPresent = [](UComboBoxString* Combo, const FString& Value)
	{
		if (!Combo || Value.IsEmpty())
		{
			return;
		}
		if (Combo->FindOptionIndex(Value) != INDEX_NONE)
		{
			Combo->SetSelectedOption(Value);
		}
	};

	auto SetToggleFrom01 = [](UCheckBox* Toggle, const FString& Value)
	{
		if (!Toggle || Value.IsEmpty())
		{
			return;
		}
		Toggle->SetIsChecked(Value == TEXT("1") || Value.Equals(TEXT("true"), ESearchCase::IgnoreCase));
	};

	SetComboIfPresent(SwanResolutionCombo, SwanCVars.FindRef(TEXT("PerformancePack.Resolution")));
	SetComboIfPresent(SwanRefreshRateCombo, SwanCVars.FindRef(TEXT("PerformancePack.RefreshRate")));
	SetComboIfPresent(SwanFoveationModeCombo, SwanCVars.FindRef(TEXT("PerformancePack.FoveationMode")));
	SetComboIfPresent(SwanFoveationLevelCombo, SwanCVars.FindRef(TEXT("PerformancePack.FoveationLevel")));
	SetComboIfPresent(SwanAntiAliasingCombo, SwanCVars.FindRef(TEXT("PerformancePack.AntiAliasing")));
	SetComboIfPresent(SwanMsaaLevelCombo, SwanCVars.FindRef(TEXT("PerformancePack.MSAALevel")));
	SetToggleFrom01(SwanSuperResolutionToggle, SwanCVars.FindRef(TEXT("PerformancePack.SuperResolution")));
	SetToggleFrom01(SwanSharpenToggle, SwanCVars.FindRef(TEXT("PerformancePack.Sharpen")));
	SetToggleFrom01(SwanAdaptiveResolutionToggle, SwanCVars.FindRef(TEXT("PerformancePack.AdaptiveResolution")));

	SetComboIfPresent(PICO4ResolutionCombo, PICO4CVars.FindRef(TEXT("PerformancePack.Resolution")));
	SetComboIfPresent(PICO4RefreshRateCombo, PICO4CVars.FindRef(TEXT("PerformancePack.RefreshRate")));
	SetComboIfPresent(PICO4FoveationModeCombo, PICO4CVars.FindRef(TEXT("PerformancePack.FoveationMode")));
	SetComboIfPresent(PICO4FoveationLevelCombo, PICO4CVars.FindRef(TEXT("PerformancePack.FoveationLevel")));
	SetComboIfPresent(PICO4AntiAliasingCombo, PICO4CVars.FindRef(TEXT("PerformancePack.AntiAliasing")));
	SetComboIfPresent(PICO4MsaaLevelCombo, PICO4CVars.FindRef(TEXT("PerformancePack.MSAALevel")));
	SetToggleFrom01(PICO4SuperResolutionToggle, PICO4CVars.FindRef(TEXT("PerformancePack.SuperResolution")));
	SetToggleFrom01(PICO4SharpenToggle, PICO4CVars.FindRef(TEXT("PerformancePack.Sharpen")));
	SetToggleFrom01(PICO4AdaptiveResolutionToggle, PICO4CVars.FindRef(TEXT("PerformancePack.AdaptiveResolution")));

	SetToggleFrom01(SwanASWToggle, SwanCVars.FindRef(TEXT("PerformancePack.ApplicationSpaceWarp")));
	SetToggleFrom01(PICO4ASWToggle, PICO4CVars.FindRef(TEXT("PerformancePack.ApplicationSpaceWarp")));

	const FString SwanTonemap = SwanCVars.Contains(TEXT("PerformancePack.TonemapSubpass")) ? SwanCVars.FindRef(TEXT("PerformancePack.TonemapSubpass")) : SwanCVars.FindRef(TEXT("r.Mobile.TonemapSubpass"));
	const FString PICO4Tonemap = PICO4CVars.Contains(TEXT("PerformancePack.TonemapSubpass")) ? PICO4CVars.FindRef(TEXT("PerformancePack.TonemapSubpass")) : PICO4CVars.FindRef(TEXT("r.Mobile.TonemapSubpass"));
	SetToggleFrom01(SwanTonemapSubpassToggle, SwanTonemap);
	SetToggleFrom01(PICO4TonemapSubpassToggle, PICO4Tonemap);

	{
		const FString EngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
		const FString Section = TEXT("/Script/Engine.RendererSettings");
		int32 MobileShadingPath = 0;
		bool bMobileHDR = false;
		bool bMovableSpotlightShadows = false;
		if (GConfig)
		{
			GConfig->GetInt(*Section, TEXT("r.Mobile.ShadingPath"), MobileShadingPath, EngineIniPath);
			GConfig->GetBool(*Section, TEXT("r.MobileHDR"), bMobileHDR, EngineIniPath);
			GConfig->GetBool(*Section, TEXT("r.Mobile.EnableMovableSpotlightsShadow"), bMovableSpotlightShadows, EngineIniPath);
		}
		SetComboIfPresent(GlobalShadingPathCombo, MobileShadingPath == 1 ? TEXT("Deferred Shading") : TEXT("Forward Shading"));
		if (GlobalMobileHDRToggle)
		{
			GlobalMobileHDRToggle->SetIsChecked(bMobileHDR);
		}
		if (GlobalMovableSpotlightShadowsToggle)
		{
			GlobalMovableSpotlightShadowsToggle->SetIsChecked(bMovableSpotlightShadows);
		}
	}

	UpdateSwanStatus();
	UpdatePICO4Status();
	return true;
}

bool UPICOOpenXRConfigEditorWidget::WriteDefaultEngineIni() const
{
	const FString EngineIniPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	const int32 MobileShadingPath = (GlobalShadingPathCombo && GlobalShadingPathCombo->GetSelectedOption() == TEXT("Deferred Shading")) ? 1 : 0;
	const bool bMobileHDR = GlobalMobileHDRToggle && GlobalMobileHDRToggle->IsChecked();
	const bool bMovableSpotlightShadows = GlobalMovableSpotlightShadowsToggle && GlobalMovableSpotlightShadowsToggle->IsChecked();

	URendererSettings* RendererSettings = GetMutableDefault<URendererSettings>();
	if (!RendererSettings)
	{
		return false;
	}

	RendererSettings->MobileShadingPath = static_cast<TEnumAsByte<EMobileShadingPath::Type>>(MobileShadingPath);
	RendererSettings->bMobilePostProcessing = bMobileHDR;
	RendererSettings->bMobileAllowMovableSpotlightShadows = bMovableSpotlightShadows;

	const FString DefaultConfigFilename = RendererSettings->GetDefaultConfigFilename();
	const FProperty* ShadingPathProperty = RendererSettings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(URendererSettings, MobileShadingPath));
	const FProperty* MobileHDRProperty = RendererSettings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(URendererSettings, bMobilePostProcessing));
	const FProperty* MovableSpotlightShadowProperty = RendererSettings->GetClass()->FindPropertyByName(GET_MEMBER_NAME_CHECKED(URendererSettings, bMobileAllowMovableSpotlightShadows));
	if (!ShadingPathProperty || !MobileHDRProperty || !MovableSpotlightShadowProperty)
	{
		return false;
	}

	RendererSettings->UpdateSinglePropertyInConfigFile(ShadingPathProperty, EngineIniPath);
	RendererSettings->UpdateSinglePropertyInConfigFile(MobileHDRProperty, EngineIniPath);
	RendererSettings->UpdateSinglePropertyInConfigFile(MovableSpotlightShadowProperty, EngineIniPath);
	return true;
}

static int32 PerformancePackGetMobileAAValue(const FString& AntiAliasing)
{
	if (AntiAliasing == TEXT("FXAA"))
	{
		return 1;
	}
	if (AntiAliasing == TEXT("TAA"))
	{
		return 2;
	}
	if (AntiAliasing == TEXT("MSAA"))
	{
		return 3;
	}
	return 0;
}

static FString PerformancePackBool01(bool bValue)
{
	return bValue ? TEXT("1") : TEXT("0");
}

bool UPICOOpenXRConfigEditorWidget::WriteDefaultDeviceProfilesIni() const
{
	const FString OutputDir = FPaths::ProjectConfigDir();
	IFileManager::Get().MakeDirectory(*OutputDir, true);
	const FString OutputPath = OutputDir / TEXT("DefaultDeviceProfiles.ini");

	auto GetOption = [](const UComboBoxString* Combo) -> FString
	{
		return Combo ? Combo->GetSelectedOption() : FString();
	};

	auto GetChecked = [](const UCheckBox* CheckBox) -> bool
	{
		return CheckBox && CheckBox->IsChecked();
	};

	struct FDeviceConfig
	{
		FString Resolution;
		FString RefreshRate;
		FString FoveationMode;
		FString FoveationLevel;
		FString AntiAliasing;
		FString MsaaLevel;
		bool bSuperRes = false;
		bool bSharpen = false;
		bool bAdaptive = false;
		bool bTonemapSubpass = false;
		bool bASW = false;
	};

	const FDeviceConfig SwanConfig{
		GetOption(SwanResolutionCombo),
		GetOption(SwanRefreshRateCombo),
		GetOption(SwanFoveationModeCombo),
		GetOption(SwanFoveationLevelCombo),
		GetOption(SwanAntiAliasingCombo),
		GetOption(SwanMsaaLevelCombo),
		GetChecked(SwanSuperResolutionToggle),
		GetChecked(SwanSharpenToggle),
		GetChecked(SwanAdaptiveResolutionToggle),
		GetChecked(SwanTonemapSubpassToggle),
		GetChecked(SwanASWToggle),
	};

	const FDeviceConfig PICO4Config{
		GetOption(PICO4ResolutionCombo),
		GetOption(PICO4RefreshRateCombo),
		GetOption(PICO4FoveationModeCombo),
		GetOption(PICO4FoveationLevelCombo),
		GetOption(PICO4AntiAliasingCombo),
		GetOption(PICO4MsaaLevelCombo),
		GetChecked(PICO4SuperResolutionToggle),
		GetChecked(PICO4SharpenToggle),
		GetChecked(PICO4AdaptiveResolutionToggle),
		GetChecked(PICO4TonemapSubpassToggle),
		GetChecked(PICO4ASWToggle),
	};

	auto AppendDeviceProfile = [&](TArray<FString>& Lines, const FString& ProfileName, const FDeviceConfig& Config)
	{
		if (Lines.Num() > 0)
		{
			Lines.Add(TEXT(""));
		}
		Lines.Add(FString::Printf(TEXT("[%s DeviceProfile]"), *ProfileName));
		Lines.Add(TEXT("DeviceType=Android"));
		if (ProfileName == TEXT("PICO4") || ProfileName == TEXT("Swan"))
		{
			Lines.Add(TEXT("BaseProfileName=Android_OpenXR"));
			Lines.Add(TEXT("+CVars=r.Android.DisableVulkanSupport=0"));
			Lines.Add(TEXT("+CVars=xr.SecondaryScreenPercentage.HMDRenderTarget=100"));
		}
		else
		{
			Lines.Add(TEXT("BaseProfileName=Android"));
		}

		Lines.Add(FString::Printf(TEXT("+CVars=PerformancePack.Resolution=%s"), *Config.Resolution));
		Lines.Add(FString::Printf(TEXT("+CVars=PerformancePack.RefreshRate=%s"), *Config.RefreshRate));
		Lines.Add(FString::Printf(TEXT("+CVars=PerformancePack.FoveationMode=%s"), *Config.FoveationMode));
		Lines.Add(FString::Printf(TEXT("+CVars=PerformancePack.FoveationLevel=%s"), *Config.FoveationLevel));
		Lines.Add(FString::Printf(TEXT("+CVars=PerformancePack.AntiAliasing=%s"), *Config.AntiAliasing));
		Lines.Add(FString::Printf(TEXT("+CVars=PerformancePack.MSAALevel=%s"), *Config.MsaaLevel));
		Lines.Add(FString::Printf(TEXT("+CVars=PerformancePack.SuperResolution=%s"), *PerformancePackBool01(Config.bSuperRes)));
		Lines.Add(FString::Printf(TEXT("+CVars=PerformancePack.Sharpen=%s"), *PerformancePackBool01(Config.bSharpen)));
		Lines.Add(FString::Printf(TEXT("+CVars=PerformancePack.AdaptiveResolution=%s"), *PerformancePackBool01(Config.bAdaptive)));
		Lines.Add(FString::Printf(TEXT("+CVars=PerformancePack.TonemapSubpass=%s"), *PerformancePackBool01(Config.bTonemapSubpass)));
		Lines.Add(FString::Printf(TEXT("+CVars=PerformancePack.ApplicationSpaceWarp=%s"), *PerformancePackBool01(Config.bASW)));

		Lines.Add(FString::Printf(TEXT("+CVars=r.Mobile.AntiAliasing=%d"), PerformancePackGetMobileAAValue(Config.AntiAliasing)));
		if (Config.bTonemapSubpass)
		{
			Lines.Add(TEXT("+CVars=r.Mobile.TonemapSubpass=1"));
		}

		if (Config.AntiAliasing == TEXT("MSAA"))
		{
			Lines.Add(FString::Printf(TEXT("+CVars=r.MSAACount=%s"), *Config.MsaaLevel));
		}
	};

	TArray<FString> Lines;
	AppendDeviceProfile(Lines, TEXT("Swan"), SwanConfig);
	AppendDeviceProfile(Lines, TEXT("PICO4"), PICO4Config);

	const bool bWroteDefault = FFileHelper::SaveStringArrayToFile(Lines, *OutputPath);
	const bool bWroteBase = WriteBaseDeviceProfilesMatchProfiles();
	return bWroteDefault && bWroteBase;
}

static bool PerformancePackEnsureMatchProfileLine(TArray<FString>& Lines, int32 InsertIndex, const FString& NewLine)
{
	for (const FString& Existing : Lines)
	{
		if (Existing == NewLine)
		{
			return false;
		}
	}
	Lines.Insert(NewLine, InsertIndex);
	return true;
}

static void PerformancePackEnsureLineInSection(TArray<FString>& Lines, const FString& SectionHeader, const FString& LineToEnsure)
{
	int32 SectionIndex = Lines.IndexOfByKey(SectionHeader);
	if (SectionIndex == INDEX_NONE)
	{
		Lines.Add(TEXT(""));
		Lines.Add(SectionHeader);
		SectionIndex = Lines.Num() - 1;
	}

	int32 InsertIndex = SectionIndex + 1;
	while (InsertIndex < Lines.Num())
	{
		const FString& Line = Lines[InsertIndex];
		if (Line.StartsWith(TEXT("[")) && Line.EndsWith(TEXT("]")))
		{
			break;
		}
		InsertIndex++;
	}

	for (int32 LineIndex = SectionIndex + 1; LineIndex < InsertIndex; LineIndex++)
	{
		if (Lines[LineIndex] == LineToEnsure)
		{
			return;
		}
	}

	Lines.Insert(LineToEnsure, InsertIndex);
}

bool UPICOOpenXRConfigEditorWidget::WriteBaseDeviceProfilesMatchProfiles() const
{
	const FString EngineConfigDir = FPaths::EngineConfigDir();
	const FString BaseDeviceProfilesPath = EngineConfigDir / TEXT("BaseDeviceProfiles.ini");

	TArray<FString> Lines;
	if (!FFileHelper::LoadFileToStringArray(Lines, *BaseDeviceProfilesPath))
	{
		return false;
	}
	const TArray<FString> OriginalLines = Lines;

	PerformancePackEnsureLineInSection(Lines, TEXT("[DeviceProfiles]"), TEXT("+DeviceProfileNameAndTypes=Swan,Android"));
	PerformancePackEnsureLineInSection(Lines, TEXT("[DeviceProfiles]"), TEXT("+DeviceProfileNameAndTypes=PICO4,Android"));

	const FString SectionHeader = TEXT("[/Script/AndroidDeviceProfileSelector.AndroidDeviceProfileMatchingRules]");
	int32 SectionIndex = Lines.IndexOfByKey(SectionHeader);
	if (SectionIndex == INDEX_NONE)
	{
		Lines.Add(TEXT(""));
		Lines.Add(TEXT("; PerformancePack - auto generated match rules"));
		Lines.Add(SectionHeader);
		SectionIndex = Lines.Num() - 1;
	}

	int32 InsertIndex = SectionIndex + 1;
	while (InsertIndex < Lines.Num())
	{
		const FString& Line = Lines[InsertIndex];
		if (Line.StartsWith(TEXT("[")) && Line.EndsWith(TEXT("]")))
		{
			break;
		}
		InsertIndex++;
	}

	const FString PICO4Match = TEXT("+MatchProfile=(Profile=\"PICO4\",Match=((SourceType=SRC_DeviceMake,CompareType=CMP_Equal,MatchString=\"pico\"), (SourceType=SRC_HMDSystemName,CompareType=CMP_Equal,MatchString=\"PICO 4 Ultra HMD\")))");
	const FString SwanMatch = TEXT("+MatchProfile=(Profile=\"Swan\",Match=((SourceType=SRC_DeviceMake,CompareType=CMP_Equal,MatchString=\"pico\"), (SourceType=SRC_HMDSystemName,CompareType=CMP_Equal,MatchString=\"PICO HMD\")))");

	for (int32 LineIndex = InsertIndex - 1; LineIndex > SectionIndex; LineIndex--)
	{
		if (Lines[LineIndex] == PICO4Match || Lines[LineIndex] == SwanMatch)
		{
			Lines.RemoveAt(LineIndex);
		}
	}

	int32 MagicLeapIndex = INDEX_NONE;
	for (int32 LineIndex = SectionIndex + 1; LineIndex < InsertIndex; LineIndex++)
	{
		if (Lines[LineIndex].Contains(TEXT("Profile=\"MagicLeap_Vulkan\"")))
		{
			MagicLeapIndex = LineIndex;
			break;
		}
	}

	const int32 DesiredInsertIndex = (MagicLeapIndex != INDEX_NONE) ? (MagicLeapIndex + 1) : (SectionIndex + 1);
	Lines.Insert(PICO4Match, DesiredInsertIndex);
	Lines.Insert(SwanMatch, DesiredInsertIndex + 1);

	const FString BackupPath = BaseDeviceProfilesPath + TEXT(".PerformancePack.bak");
	if (!IFileManager::Get().FileExists(*BackupPath))
	{
		FFileHelper::SaveStringArrayToFile(OriginalLines, *BackupPath);
	}

	return FFileHelper::SaveStringArrayToFile(Lines, *BaseDeviceProfilesPath);
}

void UPICOOpenXRConfigEditorWidget::ApplySwanPreset(const FString& PresetName)
{
	if (bUpdatingSwanPreset)
	{
		return;
	}

	bUpdatingSwanPreset = true;

	if (SwanQuickPresetCombo && SwanQuickPresetCombo->GetSelectedOption() != PresetName)
	{
		SwanQuickPresetCombo->SetSelectedOption(PresetName);
	}

	FString Resolution = TEXT("2.5K");
	FString RefreshRate = TEXT("90Hz");
	FString ShadingPath = TEXT("ForwardShading");
	FString MobileHDR = TEXT("Off");
	FString FoveationMode = TEXT("Eye-tracked Foveation");
	FString FoveationLevel = TEXT("Quality");
	FString AntiAliasing = TEXT("MSAA");
	FString MsaaLevel = TEXT("4");
	bool bSuperRes = true;
	bool bSharpen = true;
	bool bAdaptive = true;
	bool bASW = false;

	if (PresetName == TEXT("Performance"))
	{
		Resolution = TEXT("2K");
		RefreshRate = TEXT("72Hz");
		ShadingPath = TEXT("ForwardShading");
		MobileHDR = TEXT("Off");
		FoveationMode = TEXT("Eye-tracked Foveation");
		FoveationLevel = TEXT("Performance");
		AntiAliasing = TEXT("FXAA");
		bSuperRes = false;
		bSharpen = false;
		bAdaptive = true;
		bASW = true;
	}
	else if (PresetName == TEXT("Balanced"))
	{
		Resolution = TEXT("2.5K");
		RefreshRate = TEXT("72Hz");
		ShadingPath = TEXT("ForwardShading");
		MobileHDR = TEXT("Off");
		FoveationMode = TEXT("Eye-tracked Foveation");
		FoveationLevel = TEXT("Balanced");
		AntiAliasing = TEXT("TAA");
		bSuperRes = true;
		bSharpen = true;
		bAdaptive = true;
		bASW = false;
	}
	else if (PresetName == TEXT("Quality"))
	{
		Resolution = TEXT("2.5K");
		RefreshRate = TEXT("90Hz");
		ShadingPath = TEXT("ForwardShading");
		MobileHDR = TEXT("HDR_VulkanSubpass");
		FoveationMode = TEXT("Fixed Foveation");
		FoveationLevel = TEXT("Quality");
		AntiAliasing = TEXT("TAA");
		bSuperRes = true;
		bSharpen = true;
		bAdaptive = false;
		bASW = false;
	}
	else if (PresetName == TEXT("DeferredShading"))
	{
		Resolution = TEXT("4K");
		RefreshRate = TEXT("72Hz");
		ShadingPath = TEXT("DeferredShading");
		MobileHDR = TEXT("HDR_FullRenderPass");
		FoveationMode = TEXT("Fixed Foveation");
		FoveationLevel = TEXT("Quality");
		AntiAliasing = TEXT("TAA");
		bSuperRes = true;
		bSharpen = true;
		bAdaptive = false;
		bASW = false;
	}

	if (SwanResolutionCombo)
	{
		SwanResolutionCombo->SetSelectedOption(Resolution);
	}
	if (SwanRefreshRateCombo)
	{
		SwanRefreshRateCombo->SetSelectedOption(RefreshRate);
	}
	if (SwanShadingPathCombo)
	{
		SwanShadingPathCombo->SetSelectedOption(ShadingPath);
	}
	if (SwanMobileHDRCombo)
	{
		SwanMobileHDRCombo->SetSelectedOption(MobileHDR);
	}
	if (SwanFoveationModeCombo)
	{
		SwanFoveationModeCombo->SetSelectedOption(FoveationMode);
	}
	if (SwanFoveationLevelCombo)
	{
		SwanFoveationLevelCombo->SetSelectedOption(FoveationLevel);
	}
	if (SwanAntiAliasingCombo)
	{
		SwanAntiAliasingCombo->SetSelectedOption(AntiAliasing);
	}
	if (AntiAliasing == TEXT("MSAA") && SwanMsaaLevelCombo)
	{
		SwanMsaaLevelCombo->SetSelectedOption(MsaaLevel);
	}
	if (SwanSuperResolutionToggle)
	{
		SwanSuperResolutionToggle->SetIsChecked(bSuperRes);
	}
	if (SwanSharpenToggle)
	{
		SwanSharpenToggle->SetIsChecked(bSharpen);
	}
	if (SwanAdaptiveResolutionToggle)
	{
		SwanAdaptiveResolutionToggle->SetIsChecked(bAdaptive);
	}
	if (SwanASWToggle)
	{
		SwanASWToggle->SetIsChecked(bASW);
	}

	UpdateSwanStatus();
	bUpdatingSwanPreset = false;
}

void UPICOOpenXRConfigEditorWidget::ApplyPICO4Preset(const FString& PresetName)
{
	if (bUpdatingPICO4Preset)
	{
		return;
	}

	bUpdatingPICO4Preset = true;

	if (PICO4QuickPresetCombo && PICO4QuickPresetCombo->GetSelectedOption() != PresetName)
	{
		PICO4QuickPresetCombo->SetSelectedOption(PresetName);
	}

	FString Resolution = TEXT("2K");
	FString RefreshRate = TEXT("90Hz");
	FString ShadingPath = TEXT("ForwardShading");
	FString MobileHDR = TEXT("Off");
	FString FoveationMode = TEXT("Eye-tracked Foveation");
	FString FoveationLevel = TEXT("Quality");
	FString AntiAliasing = TEXT("MSAA");
	FString MsaaLevel = TEXT("4");
	bool bSuperRes = true;
	bool bSharpen = true;
	bool bAdaptive = true;
	bool bASW = false;

	if (PresetName == TEXT("Performance"))
	{
		Resolution = TEXT("2K");
		RefreshRate = TEXT("72Hz");
		ShadingPath = TEXT("ForwardShading");
		MobileHDR = TEXT("Off");
		FoveationMode = TEXT("Eye-tracked Foveation");
		FoveationLevel = TEXT("Performance");
		AntiAliasing = TEXT("FXAA");
		bSuperRes = false;
		bSharpen = false;
		bAdaptive = true;
		bASW = true;
	}
	else if (PresetName == TEXT("Balanced"))
	{
		Resolution = TEXT("2.5K");
		RefreshRate = TEXT("72Hz");
		ShadingPath = TEXT("ForwardShading");
		MobileHDR = TEXT("Off");
		FoveationMode = TEXT("Eye-tracked Foveation");
		FoveationLevel = TEXT("Balanced");
		AntiAliasing = TEXT("TAA");
		bSuperRes = true;
		bSharpen = true;
		bAdaptive = true;
		bASW = false;
	}
	else if (PresetName == TEXT("Quality"))
	{
		Resolution = TEXT("2K");
		RefreshRate = TEXT("90Hz");
		ShadingPath = TEXT("ForwardShading");
		MobileHDR = TEXT("HDR_VulkanSubpass");
		FoveationMode = TEXT("Fixed Foveation");
		FoveationLevel = TEXT("Quality");
		AntiAliasing = TEXT("TAA");
		bSuperRes = true;
		bSharpen = true;
		bAdaptive = false;
		bASW = false;
	}
	else if (PresetName == TEXT("DeferredShading"))
	{
		Resolution = TEXT("2.5K");
		RefreshRate = TEXT("72Hz");
		ShadingPath = TEXT("DeferredShading");
		MobileHDR = TEXT("HDR_FullRenderPass");
		FoveationMode = TEXT("Fixed Foveation");
		FoveationLevel = TEXT("Quality");
		AntiAliasing = TEXT("TAA");
		bSuperRes = true;
		bSharpen = true;
		bAdaptive = false;
		bASW = false;
	}

	if (PICO4ResolutionCombo)
	{
		PICO4ResolutionCombo->SetSelectedOption(Resolution);
	}
	if (PICO4RefreshRateCombo)
	{
		PICO4RefreshRateCombo->SetSelectedOption(RefreshRate);
	}
	if (PICO4ShadingPathCombo)
	{
		PICO4ShadingPathCombo->SetSelectedOption(ShadingPath);
	}
	if (PICO4MobileHDRCombo)
	{
		PICO4MobileHDRCombo->SetSelectedOption(MobileHDR);
	}
	if (PICO4FoveationModeCombo)
	{
		PICO4FoveationModeCombo->SetSelectedOption(FoveationMode);
	}
	if (PICO4FoveationLevelCombo)
	{
		PICO4FoveationLevelCombo->SetSelectedOption(FoveationLevel);
	}
	if (PICO4AntiAliasingCombo)
	{
		PICO4AntiAliasingCombo->SetSelectedOption(AntiAliasing);
	}
	if (AntiAliasing == TEXT("MSAA") && PICO4MsaaLevelCombo)
	{
		PICO4MsaaLevelCombo->SetSelectedOption(MsaaLevel);
	}
	if (PICO4SuperResolutionToggle)
	{
		PICO4SuperResolutionToggle->SetIsChecked(bSuperRes);
	}
	if (PICO4SharpenToggle)
	{
		PICO4SharpenToggle->SetIsChecked(bSharpen);
	}
	if (PICO4AdaptiveResolutionToggle)
	{
		PICO4AdaptiveResolutionToggle->SetIsChecked(bAdaptive);
	}
	if (PICO4ASWToggle)
	{
		PICO4ASWToggle->SetIsChecked(bASW);
	}

	UpdatePICO4Status();
	bUpdatingPICO4Preset = false;
}
