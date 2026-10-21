#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "PICOOpenXRConfigEditorWidget.generated.h"

class UButton;
class UCheckBox;
class UComboBoxString;
class UHorizontalBox;
class UTextBlock;
class UWidgetSwitcher;
class UWidget;

UCLASS()
class PICOOPENXREDITOR_API UPICOOpenXRConfigEditorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

protected:
	void CreateLayout();

	UFUNCTION()
	void OnTabSwan();

	UFUNCTION()
	void OnTabPICO4();

	UFUNCTION()
	void OnTabAdvanced();

	UFUNCTION()
	void OnSwanResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSwanRefreshRateChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSwanShadingPathChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSwanMobileHDRChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSwanFoveationModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSwanFoveationLevelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSwanAntiAliasingChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSwanMsaaLevelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnPICO4ResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnPICO4RefreshRateChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnPICO4ShadingPathChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnPICO4MobileHDRChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnPICO4FoveationModeChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnPICO4FoveationLevelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnPICO4AntiAliasingChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnPICO4MsaaLevelChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnSwanQuickPresetChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnPICO4QuickPresetChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnGlobalShadingPathChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnGlobalMobileHDRChanged(bool bIsChecked);

	UFUNCTION()
	void OnGlobalMovableSpotlightShadowsChanged(bool bIsChecked);

	UFUNCTION()
	void OnRestartNow();

	UFUNCTION()
	void OnAnyToggleChanged(bool bIsChecked);

	UFUNCTION()
	void OnResetSwan();

	UFUNCTION()
	void OnResetPICO4();

private:
	void SetActiveTabIndex(int32 Index);
	void UpdateSwanStatus();
	void UpdatePICO4Status();
	void UpdateStatusLabel(UTextBlock* StatusLabel, const FString& StatusText, const FLinearColor& StatusColor);
	void ApplySwanPreset(const FString& PresetName);
	void ApplyPICO4Preset(const FString& PresetName);
	void ApplyConfig(bool bShowDialog);
	void ScheduleAutoApply();
	bool LoadFromDefaultDeviceProfilesIni();
	bool WriteDefaultEngineIni() const;
	bool WriteDefaultDeviceProfilesIni() const;
	bool WriteBaseDeviceProfilesMatchProfiles() const;
	void ApplyMobileHDRConstraints(bool bMarkRestart);

	UPROPERTY()
	UWidgetSwitcher* MainSwitcher = nullptr;

	UPROPERTY()
	UComboBoxString* GlobalShadingPathCombo = nullptr;

	UPROPERTY()
	UCheckBox* GlobalMobileHDRToggle = nullptr;

	UPROPERTY()
	UCheckBox* GlobalMovableSpotlightShadowsToggle = nullptr;

	UPROPERTY()
	UWidget* RestartNoticeWidget = nullptr;

	UPROPERTY()
	UButton* RestartNowButton = nullptr;

	bool bNeedRestartEditor = false;
	bool bUpdatingMobileHDRConstraint = false;
	bool bSuppressAutoApply = false;
	bool bAutoApplyPending = false;
	double AutoApplyTriggerTime = 0.0;

	UPROPERTY()
	UButton* TabSwanButton = nullptr;

	UPROPERTY()
	UButton* TabSwanButtonActive = nullptr;

	UPROPERTY()
	UButton* TabPICO4Button = nullptr;

	UPROPERTY()
	UButton* TabPICO4ButtonActive = nullptr;

	UPROPERTY()
	UButton* TabAdvancedButton = nullptr;

	UPROPERTY()
	UButton* TabAdvancedButtonActive = nullptr;

	UPROPERTY()
	UWidgetSwitcher* TabSwanSwitcher = nullptr;

	UPROPERTY()
	UWidgetSwitcher* TabPICO4Switcher = nullptr;

	UPROPERTY()
	UWidgetSwitcher* TabAdvancedSwitcher = nullptr;

	UPROPERTY()
	UComboBoxString* SwanQuickPresetCombo = nullptr;

	UPROPERTY()
	UTextBlock* SwanQuickPresetStatus = nullptr;

	UPROPERTY()
	UComboBoxString* SwanResolutionCombo = nullptr;

	UPROPERTY()
	UComboBoxString* SwanRefreshRateCombo = nullptr;

	UPROPERTY()
	UComboBoxString* SwanShadingPathCombo = nullptr;

	UPROPERTY()
	UComboBoxString* SwanMobileHDRCombo = nullptr;

	UPROPERTY()
	UComboBoxString* SwanFoveationModeCombo = nullptr;

	UPROPERTY()
	UWidget* SwanFoveationLevelLabel = nullptr;

	UPROPERTY()
	UComboBoxString* SwanFoveationLevelCombo = nullptr;

	UPROPERTY()
	UComboBoxString* SwanAntiAliasingCombo = nullptr;

	UPROPERTY()
	UTextBlock* SwanResolutionStatus = nullptr;

	UPROPERTY()
	UTextBlock* SwanRefreshRateStatus = nullptr;

	UPROPERTY()
	UTextBlock* SwanShadingPathStatus = nullptr;

	UPROPERTY()
	UTextBlock* SwanMobileHDRStatus = nullptr;

	UPROPERTY()
	UTextBlock* SwanFoveationModeStatus = nullptr;

	UPROPERTY()
	UTextBlock* SwanFoveationLevelStatus = nullptr;

	UPROPERTY()
	UTextBlock* SwanAntiAliasingStatus = nullptr;

	UPROPERTY()
	UWidget* SwanMsaaLevelLabel = nullptr;

	UPROPERTY()
	UComboBoxString* SwanMsaaLevelCombo = nullptr;

	UPROPERTY()
	UTextBlock* SwanMsaaLevelStatus = nullptr;

	UPROPERTY()
	UCheckBox* SwanSuperResolutionToggle = nullptr;

	UPROPERTY()
	UCheckBox* SwanSharpenToggle = nullptr;

	UPROPERTY()
	UCheckBox* SwanAdaptiveResolutionToggle = nullptr;

	UPROPERTY()
	UCheckBox* SwanASWToggle = nullptr;

	UPROPERTY()
	UCheckBox* SwanTonemapSubpassToggle = nullptr;

	UPROPERTY()
	UComboBoxString* PICO4QuickPresetCombo = nullptr;

	UPROPERTY()
	UTextBlock* PICO4QuickPresetStatus = nullptr;

	UPROPERTY()
	UComboBoxString* PICO4ResolutionCombo = nullptr;

	UPROPERTY()
	UComboBoxString* PICO4RefreshRateCombo = nullptr;

	UPROPERTY()
	UComboBoxString* PICO4ShadingPathCombo = nullptr;

	UPROPERTY()
	UComboBoxString* PICO4MobileHDRCombo = nullptr;

	UPROPERTY()
	UComboBoxString* PICO4FoveationModeCombo = nullptr;

	UPROPERTY()
	UWidget* PICO4FoveationLevelLabel = nullptr;

	UPROPERTY()
	UComboBoxString* PICO4FoveationLevelCombo = nullptr;

	UPROPERTY()
	UComboBoxString* PICO4AntiAliasingCombo = nullptr;

	UPROPERTY()
	UTextBlock* PICO4ResolutionStatus = nullptr;

	UPROPERTY()
	UTextBlock* PICO4RefreshRateStatus = nullptr;

	UPROPERTY()
	UTextBlock* PICO4ShadingPathStatus = nullptr;

	UPROPERTY()
	UTextBlock* PICO4MobileHDRStatus = nullptr;

	UPROPERTY()
	UTextBlock* PICO4FoveationModeStatus = nullptr;

	UPROPERTY()
	UTextBlock* PICO4FoveationLevelStatus = nullptr;

	UPROPERTY()
	UTextBlock* PICO4AntiAliasingStatus = nullptr;

	UPROPERTY()
	UWidget* PICO4MsaaLevelLabel = nullptr;

	UPROPERTY()
	UComboBoxString* PICO4MsaaLevelCombo = nullptr;

	UPROPERTY()
	UTextBlock* PICO4MsaaLevelStatus = nullptr;

	UPROPERTY()
	UCheckBox* PICO4SuperResolutionToggle = nullptr;

	UPROPERTY()
	UCheckBox* PICO4SharpenToggle = nullptr;

	UPROPERTY()
	UCheckBox* PICO4AdaptiveResolutionToggle = nullptr;

	UPROPERTY()
	UCheckBox* PICO4ASWToggle = nullptr;

	UPROPERTY()
	UCheckBox* PICO4TonemapSubpassToggle = nullptr;

	UPROPERTY()
	UButton* SwanResetButton = nullptr;

	UPROPERTY()
	UButton* PICO4ResetButton = nullptr;

	bool bUpdatingSwanPreset = false;
	bool bUpdatingPICO4Preset = false;
};
