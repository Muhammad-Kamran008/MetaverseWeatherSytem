#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UIEnvironmentController.h"
#include "WeatherEnvironmentPanel.h"
#include "EnvironmentAdapter.h"
#include "EnvironmentManager.h"
#include "WeatherEffectsPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
//#include "DateTimePicker.h"

#include "UIStyle.h"

class SBox;
class SWindow;
class SEditableTextBox;
class SDateTimePicker;

UENUM(BlueprintType)
enum class ESkySphere : uint8
{
	BlueSkySphere,
	BlackSkySphere,
	UDSSky
};
template<typename ItemType> class SComboBox;

class METAVERSEWEATHERSYSTEM_API SWeatherDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SWeatherDialog) {}
		SLATE_ARGUMENT(UUIEnvironmentController*, EnvironmentController)
	SLATE_END_ARGS()

	SWeatherDialog();
	virtual ~SWeatherDialog();

	void Construct(const FArguments& InArgs);
private:
	FUITheme  Theme;
	FWeatherStyles Styles;

	TSharedPtr<FWeatherEffectsState> WeatherFxState;

	TMap<FString, TSharedPtr<FSlateBrush>> BrushCache;

	UPROPERTY()
	TWeakObjectPtr<UUIEnvironmentController> EnvController;
	// -------------------------
	// Theme
	// -------------------------
	FLinearColor ThemeBgBase;
	FLinearColor ThemeHeaderFill;
	FLinearColor ThemePanelFill;
	FLinearColor ThemePanelLine;
	FLinearColor ThemeAccent;

	UObject* FontAsset;
	FSlateFontInfo FontInfo12;
	FSlateFontInfo FontInfo10;
	FSlateFontInfo FontInfo8;
	// -------------------------
	// Styles 
	// -------------------------
	FEditableTextBoxStyle DarkTextBoxStyle;
	FComboBoxStyle        DarkComboStyle;
	FCheckBoxStyle        DarkCheckBoxStyle;
	FButtonStyle          NavButtonStyle;
	FCheckBoxStyle        NavToggleStyle;

	// -------------------------
	// Brushes
	// -------------------------
	const FSlateBrush* CalendarBrush = nullptr;
	const FSlateBrush* ClockBrush = nullptr;

	// -------------------------
	// Data
	// -------------------------
	TArray<TSharedPtr<FString>> DateOptions;
	TArray<TSharedPtr<FString>> HourOptions;
	TArray<TSharedPtr<FString>> SpeedOptions;
	TArray<TSharedPtr<FString>> MoonPhases;

	TSharedPtr<FString> SelectedMoonPhase;
	TSharedPtr<FString> SelectedSpeedOfTime;
	ENavButton ActiveWindow;
	// Environment
	TArray<TSharedPtr<FString>> EnvironmentOptions;
	TSharedPtr<FString> SelectedEnvironment;

	// -------------------------
	// Layout widgets
	// -------------------------
	TSharedPtr<SWidget> TopBar;
	TSharedPtr<SWidget> LeftPanel;
	TSharedPtr<SBox>    MainContentHost;

	// -------------------------
	// Fields that must persist 
	// -------------------------
	TSharedPtr<SEditableTextBox> DateTextBox;
	TSharedPtr<SEditableTextBox> TimeTextBox;
	TSharedPtr<SEditableTextBox> DawnTextBox;
	TSharedPtr<SEditableTextBox> DuskTextBox;
	TSharedPtr<SEditableTextBox> AmbientLightField;
	TSharedPtr<FAmbientCube> State;

	UPROPERTY()
	FString DateValue;
	UPROPERTY()
	FString TimeValue;
	FString DawnValue = TEXT("06:00 AM");
	FString DuskValue = TEXT("06:00 PM");
	FEnvironmentData* Data;

	bool  bTriggersValid = false;
	// Popups
	TWeakPtr<SWindow> CalendarPopup;
	TWeakPtr<SWindow> ClockPopup;

	bool bIsShadowsChecked;
	bool bDisableNtworkFeatures = false;
	bool bDisableAtmosphereFeatures = true;
	bool bIsShadowsEnable = true;
	ESkySphere SkySphere = ESkySphere::UDSSky;
	// -------------------------
	// Scenario Env controls
	// -------------------------
	TSharedPtr<SComboBox<TSharedPtr<FString>>> EnvironmentCombo;
	TSharedPtr<SEditableTextBox> EnvironmentNameField;
	bool bEnvAddMode = false;
	TSharedPtr<SWidgetSwitcher> EnvironmentSwitcher;

	FDateTime DawnTime;
	FDateTime DuskTime;
	bool bUseCustomDawnDusk = true;
	// Edit/Update mode
	bool bEnvEditingExisting = false;
	FString EnvEditingOriginalName;

	// AOI window
	TSharedPtr<SWindow> AddAreaWindow;
	TMap<FString, FWeatherTemplate> EnvironmentPresets;
	TArray<TSharedPtr<FString>> AreaOfInterestOptions;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> AreaCombo;
	TSharedPtr<FString> SelectedAreaOption;


private:

	FTSTicker::FDelegateHandle DialogRefreshTickerHandle;
	bool OnDialogRefreshTick(float DeltaSeconds);

	// Builders
	TSharedRef<SWidget> BuildTimeOfDayContent();
	TSharedRef<SWidget> BuildScenarioEnvContent();
	TSharedRef<SWidget> BuildAtmpshereContent();
	TSharedRef<SWidget> MakeSliderWithField(const FString& Label, float Min, float Max, TSharedPtr<SEditableTextBox>& OutField, float LabelWidth, TFunction<float()> GetValue, TFunction<void(float)> SetValue);
	TSharedRef<SWidget> BuildOthersScreenContent();
	TSharedRef<SWidget> BuildAtmosphereGroup();

	// Navigation handler
	FReply OnModuleButtonClicked(FString ModuleName);

	// Popups
	void ShowCalendarPopup();
	void ShowClockPopup(TSharedPtr<SEditableTextBox> Target);

	// AOI
	FReply OnAreaAddClicked();
	void RefreshAreaOfInterestCombo();
	FText GetSelectedAOIText() const;
	void OnAOISelected(TSharedPtr<FString> Selected, ESelectInfo::Type);

	void HydrateTimeStateOnOpen();
	void SetCurrentDateTimeEverywhere(const FDateTime& InDateTime, bool bWriteToUDS, bool bUpdateStoredDate);
	bool HasValidDataAndController() const;
	void InitializeDefaultTimeDateIfNeeded();

	void RefreshAreaOfInterestOptions();

	//Core Helpers
	static FDateTime CombineDateAndTime(const FDateTime& DateOnly, const FDateTime& TimeOnly);
	TSharedPtr<FString> FindOptionByText(const TArray<TSharedPtr<FString>>& Options, const FString& Value);
	void LoadFromEnvironmentData();

	TSharedRef<SWidget> MakeTextBoxWithIcon(TSharedPtr<SEditableTextBox>& OutTextBox, const FEditableTextBoxStyle* InStyle, const FString& InitialText, FOnTextCommitted OnCommitted, const FString& IconPath, TFunction<FReply()> OnIconClicked, float IconSize, float RightPaddingForIcon);

	TSharedRef<SWidget> MakeIconLabel(
		const FString& Text,
		const FString& IconPath = "",
		const FSlateFontInfo* InFont = nullptr,
		const FSlateColor& TextColor = FLinearColor::White,
		float IconSize = 16.0f,
		float IconTextPadding = 6.0f
	);

	TSharedRef<SWidget> MakeDropDownIcons(const FString& IconPath, float IconSize);
	TSharedPtr<FSlateBrush> GetIconBrush(FString IconName, float IconSize);
public:
	bool ParseDateTime();
	TWeakPtr<SEditableTextBox> ActiveTimeTarget;
	TWeakPtr<SEditableTextBox> ActiveDateTarget;
	void OnPickerCommitted(const FName PickerType, const FString& Value);
	TSharedPtr<SWeatherEffectsPanel> WeatherEffectsPanelWidget;
	TSharedPtr<SWeatherEnvironmentPanel> WeatherEnvironmentPanelWidget;
};
