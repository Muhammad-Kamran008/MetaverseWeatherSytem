#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SWindow.h"
#include "UIEnvironmentController.h"
#include "UIStyle.h"
#include "EnvironmentData.h"



class SAreaOfInterestDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAreaOfInterestDialog) {}
		SLATE_ARGUMENT(TWeakPtr<SWindow>, OwningWindow)

		// Theme
		SLATE_ARGUMENT(FLinearColor, BgBase)
		SLATE_ARGUMENT(FLinearColor, HeaderFill)
		SLATE_ARGUMENT(FLinearColor, PanelFill)
		SLATE_ARGUMENT(FLinearColor, PanelLine)
		SLATE_ARGUMENT(FLinearColor, Accent)

		// Styles (must live longer than this widget; pass from SWeatherDialog members)
		SLATE_ARGUMENT(const FEditableTextBoxStyle*, TextBoxStyle)
		SLATE_ARGUMENT(const FButtonStyle*, ButtonStyle)
		SLATE_ARGUMENT(FWeatherStyles, Styles)


		// Callbacks
		SLATE_EVENT(FSimpleDelegate, OnAddConfirmed)
		SLATE_ARGUMENT(UUIEnvironmentController*, EnvironmentController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	SAreaOfInterestDialog();
private:
	// Theme
	FLinearColor BgBase, HeaderFill, PanelFill, PanelLine, Accent;

	// Styles
	const FEditableTextBoxStyle* TextBoxStyle = nullptr;
	FComboBoxStyle ComboStyle ;
	const FButtonStyle* ButtonStyle = nullptr;
	FWeatherStyles Styles;

	UObject* FontAsset;
	FSlateFontInfo FontInfo12;
	FSlateFontInfo FontInfo10;
	FSlateFontInfo FontInfo8;

	// Window + callback
	TWeakPtr<SWindow> OwningWindow;
	FSimpleDelegate OnAddConfirmed;

	// Data
	TArray<TSharedPtr<FString>> GeometryOptions;
	TSharedPtr<FString> SelectedGeometry;


	// Fields
	TSharedPtr<SEditableTextBox> AreaName;
	TSharedPtr<SEditableTextBox> TR_Lat;
	TSharedPtr<SEditableTextBox> TR_Lon;
	TSharedPtr<SEditableTextBox> BL_Lat;
	TSharedPtr<SEditableTextBox> BL_Lon;
	TMap<FString, TSharedPtr<FSlateBrush>> BrushCache;

private:
	TSharedRef<SWidget> HeaderBar();
	TSharedRef<SWidget> SeparatorLine(float Thickness = 1.f) const;

	TSharedRef<SWidget> Label(const FString& Text, bool bUpper = false, int32 Size = 9) const;
	TSharedRef<SWidget> FullWidthField(TSharedPtr<SEditableTextBox>& OutField, const FText& Hint);
	TSharedRef<SWidget> MakeCombo(TArray<TSharedPtr<FString>>* Source, TSharedPtr<FString>& InOutSelected, const FString& DefaultText, float MinWidth, TFunction<void()> OnChanged);

	TSharedRef<SWidget> CoordSection(const FString& Title, TSharedPtr<SEditableTextBox>& Lat, TSharedPtr<SEditableTextBox>& Lon);
	TSharedRef<SWidget> FullWidthReadOnlyField(const FText& Value);

	FReply OnCancel();
	FReply OnAdd();
	FReply OnClose();
	TSharedPtr<FSlateBrush> GetIconBrush(FString IconName, float IconSize);

	TWeakObjectPtr<UUIEnvironmentController> EnvController;

	TSharedRef<SWidget> MakeIconLabel(
		const FString& Text,
		const FString& IconPath = "",
		const FSlateFontInfo* InFont = nullptr,
		const FSlateColor& TextColor = FLinearColor::White,
		float IconSize = 16.0f,
		float IconTextPadding = 6.0f
	);
};