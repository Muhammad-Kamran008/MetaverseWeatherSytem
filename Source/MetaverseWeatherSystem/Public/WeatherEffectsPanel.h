#pragma once

#include "CoreMinimal.h"
#include "UIStyle.h"
#include "Widgets/SCompoundWidget.h"
#include "WeatherEnvironmentPanel.h"
#include "UIEnvironmentController.h"

class SEditableTextBox;
template<typename ItemType> class SListView;

class METAVERSEWEATHERSYSTEM_API SWeatherEffectsPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SWeatherEffectsPanel) {}
		SLATE_ARGUMENT(FUITheme, Theme)
		SLATE_ARGUMENT(FWeatherStyles, Styles)
		SLATE_ARGUMENT(TSharedPtr<FWeatherEffectsState>, State)
		SLATE_ARGUMENT(UUIEnvironmentController*, EnvironmentController)
	SLATE_END_ARGS()


	void Construct(const FArguments& InArgs);
	SWeatherEffectsPanel();
	void RefreshFromEnvironmentData();
private:
	// -------------------------
	// Types: editable table rows
	// -------------------------

	using FCloudRowPtr = TSharedPtr<FCloudLayerRow>;
	using FWindRowPtr = TSharedPtr<FWindLayerRow>;

	// -------------------------
	// Inputs
	// -------------------------
	FUITheme  Theme;
	FWeatherStyles Styles;
	TSharedPtr<FWeatherEffectsState> State;

	// -------------------------
	// Styles (flattened)
	// -------------------------
	FTableViewStyle FlatTableViewStyle;
	FEditableTextBoxStyle FlatTextBoxStyle;
	FEditableTextBoxStyle TableTextBoxStyle;  
	FComboBoxStyle        FlatComboStyle;     
	FComboBoxStyle        TableComboStyle;     
	FTableRowStyle        FlatTableRowStyle;   

	UObject* FontAsset;
	FSlateFontInfo FontInfo12;
	FSlateFontInfo FontInfo10;
	FSlateFontInfo FontInfo8;

	// Subgroup enable flags
	bool bPrecipEnabled = true;
	bool bVisibilityEnabled = true;
	bool bCloudsEnabled = true;
	bool bWindEnabled = true;
	bool bRefreshingFromEnvironment = false;
	// -------------------------
	// Table theming
	// -------------------------
	FLinearColor TableFillColor;
	FLinearColor TableCellFillColor;
	FLinearColor TableGridLineColor;

	// -------------------------
	// Dropdown sources + selections 
	// -------------------------
	TArray<TSharedPtr<FString>> PrecipTypeOptions;
	TArray<TSharedPtr<FString>> PrecipIntensityOptions;
	TArray<TSharedPtr<FString>> VisibilityObscurantOptions;

	TArray<TSharedPtr<FString>> CloudsLayersOptions;
	TArray<TSharedPtr<FString>> CloudsTypeOptions;
	TArray<TSharedPtr<FString>> CloudsCoverOptions;
	TArray<TSharedPtr<FString>> ThunderOptions;

	TArray<TSharedPtr<FString>> WindLayersOptions;

	TSharedPtr<FString> SelectedPrecipType;
	TSharedPtr<FString> SelectedPrecipIntensity;
	TSharedPtr<FString> SelectedVisibilityObscurant;

	TSharedPtr<FString> SelectedCloudLayer;
	TSharedPtr<FString> SelectedCloudType;
	TSharedPtr<FString> SelectedCloudCover;
	TSharedPtr<FString> SelectedThunder;

	TSharedPtr<SComboBox<TSharedPtr<FString>>> PrecipIntensityComboBox;
	//new
	TSharedPtr<SComboBox<TSharedPtr<FString>>> PrecipTypeComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> VisibilityComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> CloudLayerComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> CloudTypeComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> CloudCoverComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ThunderComboBox;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> WindLayerComboBox;
	TSharedPtr<FString> SelectedWindLayer;

	TArray<TSharedPtr<FString>> WeatherOptions;
	// -------------------------
	// Rows + views
	// -------------------------
	TArray<FCloudRowPtr> CloudRows;
	TArray<FWindRowPtr>  WindRows;

	TSharedPtr<SListView<FCloudRowPtr>> CloudListView;
	TSharedPtr<SListView<FWindRowPtr>>  WindListView;

	// -------------------------
	// Cached fields
	// -------------------------
	TSharedPtr<SEditableTextBox> PrecipField;
	TSharedPtr<SEditableTextBox> VisibilityField;
	TSharedPtr<SEditableTextBox> CloudsTopField;
	TSharedPtr<SEditableTextBox> CloudsBaseField;

	TSharedPtr<SEditableTextBox> WindDirField;
	TSharedPtr<SEditableTextBox> WindSpeedField;
	TSharedPtr<SEditableTextBox> DownwindField;

	bool bUpdatingPrecipFromUI = false;
	bool bUpdatingFromSlider = false;
	bool bUpdatingFromDropdown = false;
	TMap<FString, TSharedPtr<FSlateBrush>> BrushCache;
	TSharedPtr<SWeatherEnvironmentPanel> PresetPanel;
	// -------------------------
	// Layout builders
	// -------------------------
	TSharedRef<SWidget> BuildPrecipitation();
	void UpdateIntensityBasedOnSlider();
	void UpdateSliderBasedOnIntensity();
	void ApplyPrecipitationToEnvironment();
	TSharedRef<SWidget> BuildVisibility();
	TSharedRef<SWidget> BuildClouds();
	void RefreshCloudUiFromEnvironment();
	TSharedRef<SWidget> BuildWind();

	// -------------------------
	// Reusable UI helpers 
	// -------------------------
	TSharedRef<SWidget> MakeEffectsContainer(const TSharedRef<SWidget>& Inner);
	TSharedRef<SWidget> MakeSubGroupPanel(
		const FString& Title,
		const TSharedRef<SWidget>& Body,
		bool& EnableFlag,
		const FString& IconPath = TEXT("")
	);

	TSharedRef<SWidget> MakeSeparator(float Thickness = 1.f);

	// -------------------------
	// Table helpers 
	// -------------------------
	TSharedRef<SWidget> WrapThemedTable(TSharedRef<SWidget> TableWidget);

	TSharedRef<SWidget> TableCell(
		TSharedRef<SWidget> Inner,
		bool bRightBorder,
		bool bBottomBorder) const;

	TSharedRef<SWidget> MakeHeaderCell(const FString& Text, bool bRightBorder) const;

	TSharedRef<SWidget> MakeEditableTextCell(
		const FString& Initial,
		TFunction<void(const FString&)> OnCommit,
		bool bRightBorder,
		bool bBottomBorder) const;

	TSharedRef<SWidget> MakeTableComboCell(
		TArray<TSharedPtr<FString>>* Source,
		const FString& CurrentValue,
		TFunction<void(const FString&)> OnPick,
		const FString& DefaultText,
		float MinWidth,
		bool bRightBorder,
		bool bBottomBorder);

	TSharedRef<SWidget> MakeCombo(
		TArray<TSharedPtr<FString>>* Source,
		TSharedPtr<FString>& InOutSelected,
		const FString& DefaultText,
		float MinWidth,
		TFunction<void()> OnChanged = nullptr,
		TSharedPtr<SComboBox<TSharedPtr<FString>>>* OutComboBox = nullptr
	);

	TSharedRef<SWidget> MakeIconLabel(
		const FString& Text,
		const FString& IconPath = "",
		const FSlateFontInfo* InFont = nullptr,
		const FSlateColor& TextColor = FLinearColor::White,
		float IconSize = 16.0f,
		float IconTextPadding = 6.0f
	);
	TSharedRef<SWidget> MakeDropDownIcons(const FString& IconPath, float IconSize);
	// -------------------------
	// Parsing / misc
	// -------------------------
	static bool TryParseFloat(const FString& In, float& Out);
	TSharedRef<SWidget> MakeSliderWithField(const FString& Label, float Min, float Max, TSharedPtr<SEditableTextBox>& OutField, float LabelWidth, TFunction<float()> GetValue, TFunction<void(float)> SetValue);
	void UpdateDownwindFromDir();

	void SyncFromEnvironmentData();
	TWeakObjectPtr<UUIEnvironmentController> EnvController;
	TSharedPtr<FSlateBrush> GetIconBrush(FString IconName, float IconSize);

};