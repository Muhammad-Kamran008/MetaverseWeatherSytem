#pragma once

#include "CoreMinimal.h"
#include "UIStyle.h"
#include "Widgets/SCompoundWidget.h"
#include "UIEnvironmentController.h"

//class SComboBox;
class SEditableTextBox;
class SWidgetSwitcher;

class METAVERSEWEATHERSYSTEM_API SWeatherEnvironmentPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SWeatherEnvironmentPanel) {}
		SLATE_ARGUMENT(FUITheme, Theme)
		SLATE_ARGUMENT(FWeatherStyles, Styles)
		SLATE_ARGUMENT(UUIEnvironmentController*, EnvironmentController)
		SLATE_EVENT(FSimpleDelegate, OnPresetAppliedOrReset)
	SLATE_END_ARGS()

	SWeatherEnvironmentPanel();

	void Construct(const FArguments& InArgs);
		
private:
	using FStringItem = TSharedPtr<FString>;
	FSimpleDelegate OnPresetAppliedOrReset;
	FUITheme Theme;
	FWeatherStyles Styles;
	TWeakObjectPtr<UUIEnvironmentController> EnvController;
	TMap<FString, TSharedPtr<FSlateBrush>> BrushCache;

	UObject* FontAsset;
	FSlateFontInfo FontInfo12;
	FSlateFontInfo FontInfo10;
	FSlateFontInfo FontInfo8;

	bool bIgnorePresetSelectionChanged = false;
	bool bEditMode = false;
	FString EditingOldName;

	TArray<FStringItem> PresetOptions;
	FStringItem SelectedPreset;
	TSharedPtr<STextBlock> PresetTextBlock;

	TSharedPtr<SComboBox<FStringItem>> PresetCombo;
	TSharedPtr<SEditableTextBox> NameField; 
	TSharedPtr<SWidgetSwitcher> Switcher;
public:
	void IsPresetDirty(bool bPresetDirty);
private:
	void RefreshPresetOptions(const FString& SelectName = TEXT("None"));
	FText GetSelectedPresetText() const;
	FText GetConfirmText() const;


	FReply OnPlusClicked();
	FReply OnEditClicked();
	FReply OnDeleteClicked();
	FReply OnConfirmClicked();
	FReply OnCancelClicked();

	void OnPresetChanged(FStringItem Item, ESelectInfo::Type);
	TSharedPtr<FSlateBrush>GetIconBrush(FString IconName);
	TSharedRef<SWidget> MakeDropDownIcons(
		const FString& IconPath,
		float IconSize
	);
	TSharedRef<SWidget> MakeHeader(const FString& Title, const FString& IconPath = TEXT(""));
	TSharedRef<SWidget> MakeSeparator(float Thickness);
};