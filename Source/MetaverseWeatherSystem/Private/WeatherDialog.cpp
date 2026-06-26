#include "WeatherDialog.h"

#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"

#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"

#include "Widgets/SWindow.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SExpandableArea.h"

#include "EnvironmentData.h"
#include "WeatherEffectsPanel.h"
#include "AreaOfInterestDialog.h"
#include "DateTimePicker.h"


SWeatherDialog::SWeatherDialog()
	:FontAsset(LoadObject<UObject>(nullptr, TEXT("/Game/Fonts/segoeui_Font.segoeui_Font")))
	, FontInfo12(FontAsset, 12, FName("Default"))
	, FontInfo10(FontAsset, 10, FName("Default"))
	, FontInfo8(FontAsset, 8, FName("Default"))
{
}
SWeatherDialog::~SWeatherDialog() {
	if (DialogRefreshTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(DialogRefreshTickerHandle);
		DialogRefreshTickerHandle.Reset();
	}
}
bool SWeatherDialog::OnDialogRefreshTick(float DeltaSeconds)
{
	if (!HasValidDataAndController())
	{
		return true;
	}

	const ETimeControlMode Mode = EnvController->GetTimeControlMode();
	if (Mode == ETimeControlMode::System || Mode == ETimeControlMode::Animated)
	{
		SetCurrentDateTimeEverywhere(EnvController->GetEffectiveDateTime(), false, true);
	}
	return true;
}
// -------------------------
// Helper: popup position under widget
// -------------------------
static FVector2D GetPopupPosUnderWidget(const TSharedRef<SWidget>& AnchorWidget, const FVector2D& PopupSize)
{
	const FSlateApplication& App = FSlateApplication::Get();

	const FGeometry& Geo = AnchorWidget->GetCachedGeometry();
	const FVector2D AbsSize = Geo.GetAbsoluteSize();

	if (AbsSize.X <= 0.f || AbsSize.Y <= 0.f)
	{
		const FVector2D Cursor = App.GetCursorPos();
		return Cursor + FVector2D(10.f, 10.f);
	}

	const FVector2D AbsTopLeft = Geo.GetAbsolutePosition();

	const FSlateRect AnchorRect(
		AbsTopLeft.X,
		AbsTopLeft.Y,
		AbsTopLeft.X + AbsSize.X,
		AbsTopLeft.Y + AbsSize.Y
	);

	return FSlateApplication::Get().CalculatePopupWindowPosition(
		AnchorRect,
		PopupSize,
		true,
		FVector2D::ZeroVector,
		EOrientation::Orient_Vertical
	);
}

TSharedPtr<FSlateBrush> SWeatherDialog::GetIconBrush(FString IconName, float IconSize)
{
	if (BrushCache.Contains(IconName))
	{
		return BrushCache[IconName];
	}
	UTexture2D* Texture = EnvController->GetIcon(IconName);
	if (!Texture) return nullptr;
	TSharedPtr<FSlateBrush> IconSlateBrush = MakeShareable(new FSlateBrush());
	IconSlateBrush = MakeShareable(new FSlateBrush());
	IconSlateBrush->SetResourceObject(Texture);
	IconSlateBrush->ImageSize = FVector2D(IconSize, IconSize);
	BrushCache.Add(IconName, IconSlateBrush);
	return IconSlateBrush;
}

// -------------------------
// Helper: textbox with icon inside
// -------------------------
TSharedRef<SWidget> SWeatherDialog::MakeTextBoxWithIcon(
	TSharedPtr<SEditableTextBox>& OutTextBox,
	const FEditableTextBoxStyle* InStyle,
	const FString& InitialText,
	FOnTextCommitted OnCommitted,
	const FString& IconPath,
	TFunction<FReply()> OnIconClicked,
	float IconSize = 16.0f,
	float RightPaddingForIcon = 26.0f
)
{
	TSharedPtr<FSlateBrush> IconBrush = GetIconBrush(IconPath, IconSize);
	if (!IconBrush.IsValid()) return SNullWidget::NullWidget;

	TSharedRef<SWidget> Widget =
		SNew(SOverlay)

		+ SOverlay::Slot()
		[
			SAssignNew(OutTextBox, SEditableTextBox)
				.Style(InStyle)
				.Text(FText::FromString(InitialText))
				.OnTextCommitted(OnCommitted)
				.Padding(FMargin(8, 4, RightPaddingForIcon, 4))
		]

	+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0, 0, 6, 0))
		[
			SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.ContentPadding(0)
				.OnClicked_Lambda([OnIconClicked]() -> FReply { return OnIconClicked(); })
				[
					SNew(SBox)
						.WidthOverride(IconSize)
						.HeightOverride(IconSize)
						[
							SNew(SImage).Image(IconBrush.Get())
						]
				]
		];

	return Widget;
}

TSharedRef<SWidget> SWeatherDialog::MakeIconLabel(
	const FString& Text,
	const FString& IconPath,
	const FSlateFontInfo* InFont,
	const FSlateColor& TextColor,
	float IconSize,
	float IconTextPadding
)
{
	TSharedPtr<FSlateBrush> IconBrush = GetIconBrush(IconPath, IconSize);
	if (!IconBrush.IsValid()) return SNullWidget::NullWidget;

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0, 0, IconTextPadding, 0)
		[
			SNew(SBox)
				.WidthOverride(IconSize)
				.HeightOverride(IconSize)
				[
					SNew(SImage)
						.Image(IconBrush.Get())
				]
		]

	+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
				.Text(FText::FromString(Text))
				.Font(InFont ? *InFont : FontInfo10)
				.ColorAndOpacity(TextColor)
		];

}

TSharedRef<SWidget> SWeatherDialog::MakeDropDownIcons(
	const FString& IconPath,
	float IconSize
)
{
	TSharedPtr<FSlateBrush> IconBrush = GetIconBrush(IconPath, IconSize);
	if (!IconBrush.IsValid()) return SNullWidget::NullWidget;

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0, 0, 0, 0)
		[
			SNew(SBox)
				.WidthOverride(IconSize)
				.HeightOverride(IconSize)
				[
					SNew(SImage)
						.Image(IconBrush.Get())
				]
		];
}

void SWeatherDialog::Construct(const FArguments& InArgs)
{

	EnvController = InArgs._EnvironmentController;
	Data = EnvController->GetEnvironmentData();

	DialogRefreshTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateSP(this, &SWeatherDialog::OnDialogRefreshTick),
		0.1f);

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");


	if (!State.IsValid())
	{
		State = MakeShared<FAmbientCube>();
	}
	InitializeDefaultTimeDateIfNeeded();
	// ----------------------------
	// 1) THEME 
	// ----------------------------
	//ThemeBgBase = FLinearColor::FromSRGBColor(FColor(29, 48, 68, 255));
	//ThemePanelFill = FLinearColor::FromSRGBColor(FColor(18, 26, 46, 255));
	//ThemeHeaderFill = FLinearColor::FromSRGBColor(FColor(18, 26, 46, 255));
	//ThemePanelLine = FLinearColor(0.160f, 0.220f, 0.320f, 1.f);
	//ThemeAccent = FLinearColor(2.f / 255.f, 190.f / 255.f, 118.f / 255.f, 1.f);
	ThemeBgBase = FLinearColor(0.1f, 0.1f, 0.1f, 0.907f);
	ThemePanelFill = FLinearColor(0.022f, 0.022f, 0.022f, 0.907f);
	ThemeHeaderFill = FLinearColor(0.022f, 0.022f, 0.022f, 0.907f);
	ThemeAccent = FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
	ThemePanelLine = FLinearColor(1.0f, 1.0f, 1.0f, 1.f);

	Theme.BgBase = ThemeBgBase;
	Theme.HeaderFill = ThemeHeaderFill;
	Theme.PanelFill = ThemePanelFill;
	Theme.PanelLine = ThemePanelLine;
	Theme.Accent = ThemeAccent;

	// ----------------------------
	// 2) STYLE SETUP 
	// ----------------------------
	auto MakeTintBrush = [&](FSlateBrush& OutBrush, const FLinearColor& Tint)
		{
			OutBrush = *WhiteBrush;
			OutBrush.TintColor = Tint;
		};

	// EditableTextBox style
	DarkTextBoxStyle = FAppStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox");
	MakeTintBrush(DarkTextBoxStyle.BackgroundImageNormal, ThemeBgBase);
	MakeTintBrush(DarkTextBoxStyle.BackgroundImageHovered, ThemeBgBase);
	MakeTintBrush(DarkTextBoxStyle.BackgroundImageFocused, ThemeBgBase);
	MakeTintBrush(DarkTextBoxStyle.BackgroundImageReadOnly, ThemeBgBase);

	// ComboBox style
	DarkComboStyle = FAppStyle::Get().GetWidgetStyle<FComboBoxStyle>("ComboBox");
	{
		FComboButtonStyle& CBS = DarkComboStyle.ComboButtonStyle;
		FButtonStyle& BS = CBS.ButtonStyle;

		MakeTintBrush(BS.Normal, ThemeBgBase);
		MakeTintBrush(BS.Hovered, ThemeBgBase);
		MakeTintBrush(BS.Pressed, ThemeBgBase);
		MakeTintBrush(BS.Disabled, ThemePanelFill);

		MakeTintBrush(CBS.MenuBorderBrush, ThemeBgBase);
		CBS.DownArrowImage.TintColor = FSlateColor(FLinearColor::White);
	}

	// CheckBox style
	DarkCheckBoxStyle = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("Checkbox");
	{
		auto TintSolid = [&](FSlateBrush& B, const FLinearColor& C)
			{
				B = *WhiteBrush;
				B.TintColor = C;
			};

		const FLinearColor CheckFill = ThemeBgBase;
		const FLinearColor CheckFillHover = ThemeBgBase + FLinearColor(0.02f, 0.02f, 0.02f, 0.f);

		TintSolid(DarkCheckBoxStyle.UncheckedImage, CheckFill);
		TintSolid(DarkCheckBoxStyle.UncheckedHoveredImage, CheckFillHover);
		TintSolid(DarkCheckBoxStyle.UncheckedPressedImage, CheckFillHover);

		DarkCheckBoxStyle.Padding = FMargin(2);
	}

	// Nav button style
	{
		const FLinearColor NavBtn_Normal = ThemeHeaderFill;
		const FLinearColor NavBtn_Hovered = ThemeBgBase;
		const FLinearColor NavBtn_Pressed = ThemeHeaderFill;

		NavButtonStyle = FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
		MakeTintBrush(NavButtonStyle.Normal, FLinearColor(0.080f, 0.080f, 0.080f, 0.907f));
		//MakeTintBrush(NavButtonStyle.Normal, NavBtn_Normal);
		MakeTintBrush(NavButtonStyle.Hovered, NavBtn_Hovered);
		MakeTintBrush(NavButtonStyle.Pressed, NavBtn_Pressed);
		MakeTintBrush(NavButtonStyle.Disabled, NavBtn_Normal);
		NavButtonStyle.NormalPadding = FMargin(4, 4, 4, 4);
		NavButtonStyle.PressedPadding = FMargin(8, 8, 8, 8);
		/*NavButtonStyle.NormalPadding = FMargin(10, 6);
		NavButtonStyle.PressedPadding = FMargin(10, 7, 10, 5);*/
	}

	Styles.TextBoxStyle = &DarkTextBoxStyle;
	Styles.ComboStyle = &DarkComboStyle;
	Styles.ButtonStyle = &NavButtonStyle;
	Styles.CheckStyle = &DarkCheckBoxStyle;

	// ----------------------------
	// 3) STATE
	// ----------------------------
	if (EnvController.IsValid())
	{
		WeatherFxState = EnvController->GetWeatherEffectsState();
	}
	if (!WeatherFxState.IsValid())
	{
		WeatherFxState = MakeShared<FWeatherEffectsState>();
	}

	// ----------------------------
	// 4) DATA
	// ----------------------------
	DateOptions.Reset();
	HourOptions.Reset();
	SpeedOptions.Reset();
	MoonPhases.Reset();

	for (int32 Day = 1; Day <= 31; ++Day)
	{
		DateOptions.Add(MakeShared<FString>(FString::Printf(TEXT("%02d"), Day)));
	}

	for (int32 Hour = 1; Hour <= 12; ++Hour)
	{
		HourOptions.Add(MakeShared<FString>(FString::Printf(TEXT("%02d"), Hour)));
	}

	SpeedOptions.Add(MakeShared<FString>(TEXT("Real Time")));
	SpeedOptions.Add(MakeShared<FString>(TEXT("2X")));
	SpeedOptions.Add(MakeShared<FString>(TEXT("3X")));
	SpeedOptions.Add(MakeShared<FString>(TEXT("5X")));
	SpeedOptions.Add(MakeShared<FString>(TEXT("8X")));
	SpeedOptions.Add(MakeShared<FString>(TEXT("10X")));

	MoonPhases.Add(MakeShared<FString>(TEXT("New Moon")));
	MoonPhases.Add(MakeShared<FString>(TEXT("Wanning")));
	MoonPhases.Add(MakeShared<FString>(TEXT("Waxing")));
	MoonPhases.Add(MakeShared<FString>(TEXT("Full Moon")));

	HydrateTimeStateOnOpen();
	LoadFromEnvironmentData();

	// ----------------------------
	// 5) NAV BUTTON BUILDER
	// ----------------------------
	//auto MakeNavButton =
	//	[&](const FString& Text, TFunction<FReply()> OnClick) -> TSharedRef<SWidget>
	//	{
	//		//return SNew(SBorder)
	//		////	.BorderImage(WhiteBrush)
	//		//	//.BorderBackgroundColor(ThemeBgBase)
	//		//	.Padding(1)
	//		//	[
	//	/*return	SNew(SButton)
	//			.ButtonStyle(&NavButtonStyle)
	//			.HAlign(HAlign_Left)
	//			.OnClicked_Lambda([OnClick]() { return OnClick(); })
	//			[
	//				SNew(STextBlock)
	//					.Text(FText::FromString(Text))
	//					.ColorAndOpacity(Theme.Accent)
	//			];*/
	//			//];
	//	return SNew(SOverlay)
	//		+ SOverlay::Slot()
	//		.Padding(FMargin(8, 8,8, 8))
	//		[
	//			SNew(SBorder)
	//				.BorderBackgroundColor(FLinearColor(0.02f, 0.10f, 0.25f))
	//		]
	//		+ SOverlay::Slot()
	//		[
	//			SNew(SButton)
	//				.ButtonStyle(&NavButtonStyle)
	//				.OnClicked_Lambda([OnClick]() { return OnClick(); })
	//				[
	//					SNew(STextBlock)
	//						.Text(FText::FromString(Text))
	//						.ColorAndOpacity(Theme.Accent)
	//				]
	//		];
	//	};


	auto MakeNavButton = [&](const FString& Text, ENavButton ButtonId, TFunction<void()>OnClick)->TSharedRef<SWidget>
		{
			TSharedPtr<SCheckBox> Checkbox;
			NavToggleStyle = FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("ToggleButtonCheckbox");
			NavToggleStyle.SetCheckedImage(FSlateColorBrush(FLinearColor(0.1f, 0.1f, 0.1f, 1.f)));
			NavToggleStyle.SetCheckedHoveredImage(FSlateColorBrush(FLinearColor(0.10f, 0.10f, 0.10f, 1.f)));
			NavToggleStyle.SetCheckedPressedImage(FSlateColorBrush(FLinearColor(0.6f, 0.6f, 0.6f,1.f)));
			Checkbox = SNew(SCheckBox)
				.Type(ESlateCheckBoxType::ToggleButton)
				.Style(&NavToggleStyle)
				.BorderBackgroundColor_Lambda([Checkbox]()
					{
						if (Checkbox.IsValid() && Checkbox->IsPressed())
						{
							return FLinearColor(0.10f, 0.10f, 0.10f, 1.f);
						}
						return FLinearColor::Gray;
					})

				.IsChecked_Lambda([this, ButtonId]()
					{
						return Data->SelectedNavButton == ButtonId ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
					})
				.OnCheckStateChanged_Lambda([this, ButtonId, OnClick](ECheckBoxState NewState)
					{
						if (NewState == ECheckBoxState::Checked)
						{
							Data->SelectedNavButton = ButtonId;
							OnClick();
						}
					})
				[
					SNew(STextBlock)
						.Text(FText::FromString(Text))
						.ColorAndOpacity(Theme.Accent)
				];
			return Checkbox.ToSharedRef();
		};

	auto HSeparator = [&]() -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemeBgBase)
				.Padding(FMargin(0, 1));
		};

	// ----------------------------
	// 6) TOP + LEFT PANEL
	// ----------------------------
	TopBar = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.f)
		[
			SNew(SSpacer)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0, 0, 10, 0)
		[
			MakeIconLabel(
				TEXT("Use Network provided settings"),
				TEXT("Network"),
				&FontInfo8,
				FLinearColor::White
			)
		]

	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SNew(SCheckBox)

				.Style(Styles.CheckStyle ? Styles.CheckStyle : &FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("Checkbox"))
				.IsChecked_Lambda([this]() { return bDisableNtworkFeatures ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
				.OnCheckStateChanged_Lambda([this](ECheckBoxState S) {bDisableNtworkFeatures = (S == ECheckBoxState::Checked);
			if (MainContentHost.IsValid())
			{
				MainContentHost->Invalidate(EInvalidateWidget::LayoutAndVolatility);
			}
			EnvController->setIsNetwork(bDisableNtworkFeatures);
					})
		];

	LeftPanel = SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(6, 6, 6, 0)
		[
			MakeNavButton(TEXT("Time of Day"),ENavButton::TimeofDay,
				[this]() -> FReply { return OnModuleButtonClicked("Time of Day"); })
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)
		[
			HSeparator()
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(6, 6, 6, 0)
		[
			MakeNavButton(TEXT("Weather    "), ENavButton::Weather,
				[this]() -> FReply { return OnModuleButtonClicked("Weather"); })
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)
		[
			HSeparator()
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(6, 6, 6, 0)
		[
			MakeNavButton(TEXT("Atmosphere"), ENavButton::Atmpsphere,
				[this]() -> FReply { return OnModuleButtonClicked("Atmosphere"); })
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)
		[
			HSeparator()
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(6, 6, 6, 0)
		[
			MakeNavButton(TEXT("Light Setting"), ENavButton::LightSettings,
				[this]() -> FReply { return OnModuleButtonClicked("Light Settings"); })
		];

	// ----------------------------
	// 7) ROOT LAYOUT
	// ----------------------------
	ChildSlot
		[
			SNew(SOverlay)

				+ SOverlay::Slot()
				[
					SNew(SBorder)
						.Visibility(EVisibility::HitTestInvisible)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(ThemeBgBase)
				]

				+ SOverlay::Slot()
				[
					SNew(SVerticalBox)

						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBorder)
								.Padding(FMargin(12, 10))
								.BorderImage(WhiteBrush)
								.BorderBackgroundColor(ThemeHeaderFill)
								[
									TopBar.ToSharedRef()
								]
						]

					+ SVerticalBox::Slot().FillHeight(1.f).Padding(10)
						[
							SNew(SHorizontalBox)

								+ SHorizontalBox::Slot().AutoWidth()
								[
									SNew(SBorder)
										.Padding(6)
										.BorderImage(WhiteBrush)
										.BorderBackgroundColor(ThemeHeaderFill)
										[
											LeftPanel.ToSharedRef()
										]
								]

							+ SHorizontalBox::Slot().FillWidth(1.f).Padding(FMargin(12, 0, 0, 0))
								[
									SAssignNew(MainContentHost, SBox)
										[
											BuildTimeOfDayContent()
										]
								]
						]
				]
		];
}

FReply SWeatherDialog::OnModuleButtonClicked(FString ModuleName)
{
	if (!MainContentHost.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("MainContentHost invalid - not assigned in Construct."));
		return FReply::Handled();
	}

	if (ModuleName == "Time of Day")
	{
		MainContentHost->SetContent(BuildTimeOfDayContent());
	}
	else if (ModuleName == "Weather")
	{
		MainContentHost->SetContent(BuildScenarioEnvContent());
	}
	else if (ModuleName == "Atmosphere")
	{
		MainContentHost->SetContent(BuildAtmpshereContent());
	}
	else if (ModuleName == "Light Settings")
	{
		MainContentHost->SetContent(BuildOthersScreenContent());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unknown module: %s"), *ModuleName);
	}

	MainContentHost->Invalidate(EInvalidateWidget::LayoutAndVolatility);
	return FReply::Handled();
}

//------------------
//------HELPERS-----
//------------------
static FString FormatDateDDMMYYYY(const FDateTime& InDate)
{
	return FString::Printf(
		TEXT("%02d/%02d/%04d"),
		InDate.GetDay(),
		InDate.GetMonth(),
		InDate.GetYear());
}
static FString FormatDateDDMMYYYY(int32 Day, int32 Month, int32 Year)
{
	return FString::Printf(
		TEXT("%02d/%02d/%04d"),
		Day,
		Month,
		Year);
}
static FString FormatTime12h(const FDateTime& InTime)
{
	const int32 Hour24 = InTime.GetHour();
	const int32 Minute = InTime.GetMinute();
	const bool bPM = Hour24 >= 12;

	int32 Hour12 = Hour24 % 12;
	if (Hour12 == 0)
	{
		Hour12 = 12;
	}

	return FString::Printf(
		TEXT("%02d:%02d %s"),
		Hour12,
		Minute,
		bPM ? TEXT("PM") : TEXT("AM"));
}
static FString FormatTime12hFrom24(int32 Hour24, int32 Minute)
{
	const bool bPM = Hour24 >= 12;

	int32 Hour12 = Hour24 % 12;
	if (Hour12 == 0)
	{
		Hour12 = 12;
	}

	return FString::Printf(
		TEXT("%02d:%02d %s"),
		Hour12,
		Minute,
		bPM ? TEXT("PM") : TEXT("AM"));
}
bool SWeatherDialog::HasValidDataAndController() const
{
	return Data && EnvController.IsValid();
}

void SWeatherDialog::InitializeDefaultTimeDateIfNeeded()
{
	if (!HasValidDataAndController())
	{
		return;
	}

	const FDateTime Effective = EnvController->GetEffectiveDateTime();
	SetCurrentDateTimeEverywhere(Effective, false, true);

	Data->TimeOfDay.TimeSettings.ManualDateTime = Effective;
}
void SWeatherDialog::HydrateTimeStateOnOpen()
{
	if (!HasValidDataAndController())
	{
		return;
	}

	const FDateTime Effective = EnvController->GetEffectiveDateTime();
	SetCurrentDateTimeEverywhere(Effective, false, true);
}
void SWeatherDialog::SetCurrentDateTimeEverywhere(const FDateTime& InDateTime, bool bWriteToUDS, bool bUpdateStoredDate)
{
	if (!HasValidDataAndController())
	{
		return;
	}

	Data->TimeOfDay.TimeSettings.Date = FDateTime(
		InDateTime.GetYear(),
		InDateTime.GetMonth(),
		InDateTime.GetDay());

	Data->TimeOfDay.TimeSettings.Time = FDateTime(
		InDateTime.GetYear(),
		InDateTime.GetMonth(),
		InDateTime.GetDay(),
		InDateTime.GetHour(),
		InDateTime.GetMinute(),
		InDateTime.GetSecond());

	DateValue = FormatDateDDMMYYYY(Data->TimeOfDay.TimeSettings.Date);
	TimeValue = FormatTime12h(InDateTime);

	if (DateTextBox.IsValid())
	{
		DateTextBox->SetText(FText::FromString(DateValue));
	}

	if (TimeTextBox.IsValid())
	{
		TimeTextBox->SetText(FText::FromString(TimeValue));
	}

}
static bool ParseUIText12h_ToHM(const FString& In, int32& OutH24, int32& OutM)
{
	FString S = In.TrimStartAndEnd();
	FString TimePart, AmPmPart;
	if (!S.Split(TEXT(" "), &TimePart, &AmPmPart)) return false;

	FString HStr, MStr;
	if (!TimePart.Split(TEXT(":"), &HStr, &MStr)) return false;

	int32 H = FCString::Atoi(*HStr);
	int32 M = FCString::Atoi(*MStr);

	AmPmPart = AmPmPart.TrimStartAndEnd().ToUpper();
	if (H < 1 || H > 12 || M < 0 || M > 59) return false;
	if (AmPmPart != TEXT("AM") && AmPmPart != TEXT("PM")) return false;

	int32 H24 = H;
	if (AmPmPart == TEXT("PM") && H24 != 12) H24 += 12;
	if (AmPmPart == TEXT("AM") && H24 == 12) H24 = 0;

	OutH24 = H24;
	OutM = M;
	return true;
}
FDateTime SWeatherDialog::CombineDateAndTime(const FDateTime& DateOnly, const FDateTime& TimeOnly)
{
	return FDateTime(
		DateOnly.GetYear(),
		DateOnly.GetMonth(),
		DateOnly.GetDay(),
		TimeOnly.GetHour(),
		TimeOnly.GetMinute(),
		TimeOnly.GetSecond()
	);
}
bool SWeatherDialog::ParseDateTime()
{
	TSharedPtr<SEditableTextBox> TimeField = ActiveTimeTarget.Pin();
	TSharedPtr<SEditableTextBox> DateField = ActiveDateTarget.Pin();

	if (!TimeField.IsValid())
	{
		TimeField = TimeTextBox;
		ActiveTimeTarget = TimeField;
	}

	if (!DateField.IsValid())
	{
		DateField = DateTextBox;
		ActiveDateTarget = DateField;
	}

	if (!TimeField.IsValid() || !DateField.IsValid() || !HasValidDataAndController())
	{
		return false;
	}

	const FString DateStr = DateField->GetText().ToString().TrimStartAndEnd();

	int32 Day = 0;
	int32 Month = 0;
	int32 Year = 0;

	TArray<FString> Parts;
	DateStr.ParseIntoArray(Parts, TEXT("/"));

	if (Parts.Num() != 3)
	{
		return false;
	}

	Day = FCString::Atoi(*Parts[0]);
	Month = FCString::Atoi(*Parts[1]);
	Year = FCString::Atoi(*Parts[2]);

	if (Day < 1 || Day > 31 || Month < 1 || Month > 12 || Year < 1)
	{
		return false;
	}

	const FString InStr = TimeField->GetText().ToString().TrimStartAndEnd();

	int32 HH24 = 0;
	int32 MM = 0;
	if (!ParseUIText12h_ToHM(InStr, HH24, MM))
	{
		return false;
	}

	const FDateTime ParsedDateTime(Year, Month, Day, HH24, MM, 0);

	EnvController->SetManualDateTime(ParsedDateTime);
	State->AmbientLightIntensity = EnvController->SetAmbientLight();
	EnvController->SetTimeDateToUDS(ParsedDateTime);
	SetCurrentDateTimeEverywhere(ParsedDateTime, false, true);

	return true;
}

// -------------------------
// Time of Day
// -------------------------
TSharedRef<SWidget> SWeatherDialog::BuildTimeOfDayContent()
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	const float LabelW = 105.f;
	const float FieldW = 185.f;
	const float IconW = 22.f;
	const float RowPadY = 4.f;

	const TAttribute<bool> ManualControlsEnabled =
		TAttribute<bool>::CreateLambda([this]()
			{
				return EnvController.IsValid() &&
					EnvController->GetTimeControlMode() != ETimeControlMode::System;
			});

	auto HSeparator = [&]() -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemePanelLine)
				.Padding(FMargin(0, 1));
		};

	auto MakeLabel = [&](const FString& Text) -> TSharedRef<SWidget>
		{
			return SNew(SBox).WidthOverride(LabelW)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Text))
						.Font(FontInfo10)
						.ColorAndOpacity(FLinearColor::White)
				];
		};

	auto MakePanel = [&](TSharedRef<SWidget> Inner) -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemeHeaderFill)
				.Padding(1)
				[
					SNew(SBorder)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(ThemeHeaderFill)
						.Padding(FMargin(10))
						.IsEnabled_Lambda([this]() { return !bDisableNtworkFeatures; })
						[
							Inner
						]
				];
		};

	auto GenStringItem = [](TSharedPtr<FString> Item)
		{
			return SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::White)
				.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")));
		};

	auto WrapAsField = [&](TSharedRef<SWidget> Inner, TAttribute<bool> IsEnabledAttr = true) -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemeBgBase)
				.Padding(FMargin(6, 2))
				.IsEnabled(IsEnabledAttr)
				[
					SNew(SBox).WidthOverride(FieldW)
						[
							Inner
						]
				];
		};

	auto MakeCheckRow = [&](const FString& Label, TFunction<ECheckBoxState(void)> GetState, TFunction<void(ECheckBoxState)> SetState, TAttribute<bool> IsEnabledAttr = false) -> TSharedRef<SWidget>
		{
			return SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth()
				[
					MakeLabel(Label)
				]

				+ SHorizontalBox::Slot().AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SCheckBox)
						.Style(&DarkCheckBoxStyle)
						.IsEnabled(IsEnabledAttr)
						.IsChecked_Lambda([GetState]() {return GetState();  })
						.OnCheckStateChanged_Lambda([SetState](ECheckBoxState NewState) {SetState(NewState); })
				]

			+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(FieldW)[SNew(SSpacer)]

				]

				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(IconW)[SNew(SSpacer)]
				];
		};

	auto MakeFieldRow = [&](const FString& Label, TSharedRef<SWidget> FieldWidget) -> TSharedRef<SWidget>
		{
			return SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					MakeLabel(Label)
				]

				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(FieldW)[FieldWidget]
						.IsEnabled(ManualControlsEnabled)
				]

				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SBox).WidthOverride(IconW)[SNew(SSpacer)]
				];
		};


	auto onDateCommitted = FOnTextCommitted::CreateLambda([this](const FText& NewText, ETextCommit::Type)
		{
			ParseDateTime();
		});
	auto onTimeCommitted = FOnTextCommitted::CreateLambda([this](const FText& NewText, ETextCommit::Type)
		{
			ParseDateTime();
		});
	auto onDawnCommitted = FOnTextCommitted::CreateLambda([this](const FText& NewText, ETextCommit::Type)
		{
			DawnValue = NewText.ToString();
			if (DawnTextBox.IsValid())
				DawnTextBox->SetText(FText::FromString(DawnValue));

			int32 H24, M;
			if (ParseUIText12h_ToHM(DawnValue, H24, M))
			{
				Data->TimeOfDay.LightSettings.DawnTime = FDateTime(2000, 1, 1, H24, M, 0);
				EnvController->UpdateLightSettingsData(*Data);
			}
		});

	auto onDuskCommitted = FOnTextCommitted::CreateLambda([this](const FText& NewText, ETextCommit::Type)
		{
			DuskValue = NewText.ToString();
			if (DuskTextBox.IsValid())
				DuskTextBox->SetText(FText::FromString(DuskValue));

			int32 H24, M;
			if (ParseUIText12h_ToHM(DuskValue, H24, M))
			{
				Data->TimeOfDay.LightSettings.DuskTime = FDateTime(2000, 1, 1, H24, M, 0);
				EnvController->UpdateLightSettingsData(*Data);
			}
		});

	auto DateField = MakeTextBoxWithIcon(
		DateTextBox, &DarkTextBoxStyle,
		DateValue,
		onDateCommitted,
		TEXT("calendar"),
		[this]() -> FReply { ActiveDateTarget = DateTextBox; ShowCalendarPopup(); return FReply::Handled(); }
	);

	auto TimeField = MakeTextBoxWithIcon(
		TimeTextBox, &DarkTextBoxStyle,
		TimeValue,
		onTimeCommitted,
		TEXT("clock"),
		[this]() -> FReply
		{
			ActiveTimeTarget = TimeTextBox;
			ShowClockPopup(TimeTextBox);
			return FReply::Handled();
		}
	);

	auto DawnField = MakeTextBoxWithIcon(
		DawnTextBox, &DarkTextBoxStyle,
		DawnValue,
		onDawnCommitted,
		TEXT("clock"),
		[this]() -> FReply
		{
			ActiveTimeTarget = DawnTextBox;
			ShowClockPopup(DawnTextBox);
			return FReply::Handled();
		}
	);

	auto DuskField = MakeTextBoxWithIcon(
		DuskTextBox, &DarkTextBoxStyle,
		DuskValue,
		onDuskCommitted,
		TEXT("clock"),
		[this]() -> FReply
		{
			ActiveTimeTarget = DuskTextBox;
			ShowClockPopup(DuskTextBox);
			return FReply::Handled();
		}
	);
	if (!SelectedSpeedOfTime.IsValid() && SpeedOptions.Num() > 0)
	{
		SelectedSpeedOfTime = SpeedOptions[0];
	}

	TSharedRef<SWidget> SpeedComboField = WrapAsField(
		SNew(SComboBox<TSharedPtr<FString>>)
		.ComboBoxStyle(&DarkComboStyle)
		.OptionsSource(&SpeedOptions)
		.InitiallySelectedItem(SelectedSpeedOfTime)
		.OnGenerateWidget_Lambda(GenStringItem)
		.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Item, ESelectInfo::Type)
			{
				if (!Item.IsValid() || !Data || !EnvController.IsValid())
				{
					return;
				}

				SelectedSpeedOfTime = Item;
				const FString& S = *Item;

				if (S == TEXT("Real Time"))       Data->TimeOfDay.TimeSettings.SpeedMode = ETimeSpeedMode::RealTime;
				else if (S == TEXT("2X"))         Data->TimeOfDay.TimeSettings.SpeedMode = ETimeSpeedMode::X2;
				else if (S == TEXT("3X"))         Data->TimeOfDay.TimeSettings.SpeedMode = ETimeSpeedMode::X3;
				else if (S == TEXT("5X"))         Data->TimeOfDay.TimeSettings.SpeedMode = ETimeSpeedMode::X5;
				else if (S == TEXT("8X"))         Data->TimeOfDay.TimeSettings.SpeedMode = ETimeSpeedMode::X8;
				else if (S == TEXT("10X"))        Data->TimeOfDay.TimeSettings.SpeedMode = ETimeSpeedMode::X10;
				else                              Data->TimeOfDay.TimeSettings.SpeedMode = ETimeSpeedMode::RealTime;

				EnvController->UpdateTimeSpeedSettingsData(*Data);
				//if (Data->TimeOfDay.TimeSettings.CompleteCycle) { EnvController->UpdateCompleteCycleData(); }
			})
		[
			SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::White)
				.Text_Lambda([this]()
					{
						return SelectedSpeedOfTime.IsValid()
							? FText::FromString(*SelectedSpeedOfTime)
							: FText::FromString(TEXT("Real Time"));
					})
		],

		TAttribute<bool>::CreateLambda([this]()
			{
				return EnvController.IsValid() &&
					EnvController->GetTimeControlMode() == ETimeControlMode::Animated;
			})
	);


	if (!SelectedMoonPhase.IsValid() && MoonPhases.Num() > 0)
	{
		SelectedMoonPhase = MoonPhases[3];
	}

	TSharedRef<SWidget> MoonComboField = WrapAsField(
		SNew(SComboBox<TSharedPtr<FString>>)
		.ComboBoxStyle(&DarkComboStyle)
		.OptionsSource(&MoonPhases)
		.InitiallySelectedItem(SelectedMoonPhase)
		.OnGenerateWidget_Lambda(GenStringItem)
		.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Item, ESelectInfo::Type)
			{
				if (!Item.IsValid() || !Data || !EnvController.IsValid())
					return;

				SelectedMoonPhase = Item;
				const FString& S = *Item;

				if (S == TEXT("Full Moon"))       Data->TimeOfDay.LightSettings.MoonPhase = EMoonPhase::FullMoon;
				else if (S == TEXT("Wanning"))  Data->TimeOfDay.LightSettings.MoonPhase = EMoonPhase::Wanning;
				else if (S == TEXT("Waxing"))   Data->TimeOfDay.LightSettings.MoonPhase = EMoonPhase::Waxing;
				else if (S == TEXT("New Moon")) Data->TimeOfDay.LightSettings.MoonPhase = EMoonPhase::NewMoon;
				else                             Data->TimeOfDay.LightSettings.MoonPhase = EMoonPhase::FullMoon;

				EnvController->UpdateMoonSettingsData(*Data);
			})
		[
			SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::White)
				.Text_Lambda([this]()
					{
						return SelectedMoonPhase.IsValid()
							? FText::FromString(*SelectedMoonPhase)
							: FText::FromString(TEXT("Full Moon"));
					})
		]
	);


	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
				.Padding(FMargin(10, 5))

				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemeHeaderFill)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
						[
							MakeIconLabel(
								TEXT("TIME OF DAY"),
								TEXT("Time"),
								&FontInfo12,
								FLinearColor::White
							)
						]
				]
		]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
		[
			MakePanel(
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeIconLabel(
						TEXT("TIME SETTINGS"),
						TEXT("TimeSettings"),
						&FontInfo10,
						ThemeAccent
					)
				]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)
				[
					HSeparator()
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
				[
					SNew(SGridPanel)
						.FillColumn(0, 1.f)
						.FillColumn(1, 1.f)

						+ SGridPanel::Slot(0, 0).Padding(0, RowPadY)[MakeCheckRow("Use System Time", [this]()
							{
								return EnvController->GetTimeControlMode() == ETimeControlMode::System
									? ECheckBoxState::Checked
									: ECheckBoxState::Unchecked;
							},
							[this](ECheckBoxState State)
							{
								const bool bUse = (State == ECheckBoxState::Checked);

								if (bUse)
								{
									EnvController->SetTimeControlMode(ETimeControlMode::System);
								}
								else
								{
									EnvController->SetTimeControlMode(ETimeControlMode::Manual);
								}

								const FDateTime Effective = EnvController->GetEffectiveDateTime();
								SetCurrentDateTimeEverywhere(Effective, false, true);
							}, true)
						]

						+ SGridPanel::Slot(1, 0).Padding(0, RowPadY)[
							MakeCheckRow(
								"Animate Time",
								[this]()
								{
									return EnvController->GetTimeControlMode() == ETimeControlMode::Animated
										? ECheckBoxState::Checked
										: ECheckBoxState::Unchecked;
								},
								[this](ECheckBoxState State)
								{
									const bool bAnimate = (State == ECheckBoxState::Checked);

									if (bAnimate)
									{
										ActiveTimeTarget = TimeTextBox;
										ActiveDateTarget = DateTextBox;

										if (!ParseDateTime())
										{
											const FDateTime Effective = EnvController->GetEffectiveDateTime();
											SetCurrentDateTimeEverywhere(Effective, false, true);
										}

										EnvController->SetTimeControlMode(ETimeControlMode::Animated);
									}
									else
									{
										EnvController->SetTimeControlMode(ETimeControlMode::Manual);
									}

									const FDateTime Effective = EnvController->GetEffectiveDateTime();
									SetCurrentDateTimeEverywhere(Effective, false, true);
								},
								ManualControlsEnabled
							)
						]
							+ SGridPanel::Slot(1, 2).Padding(0, RowPadY)[MakeCheckRow("Complete Cycle", [this]()
								{
									return Data->TimeOfDay.TimeSettings.CompleteCycle ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
								},
								[this](ECheckBoxState State)
								{
									const bool bUse = (State == ECheckBoxState::Checked);

									if (bUse)
									{
										ActiveTimeTarget = TimeTextBox;
										ActiveDateTarget = DateTextBox;
										Data->TimeOfDay.TimeSettings.CompleteCycle = true;
										EnvController->SetTimeControlMode(ETimeControlMode::Animated);
									}
									else
									{
										Data->TimeOfDay.TimeSettings.CompleteCycle = false;
										EnvController->SetTimeControlMode(ETimeControlMode::Manual);
									}
									EnvController->UpdateCompleteCycleData();
								}, ManualControlsEnabled)
							]


								+ SGridPanel::Slot(0, 1).Padding(0, RowPadY)[MakeFieldRow("Date", DateField)]
									+ SGridPanel::Slot(1, 1).Padding(0, RowPadY)[MakeFieldRow("Speed of Time", SpeedComboField)]

									+ SGridPanel::Slot(0, 2).Padding(0, RowPadY)[MakeFieldRow("Time", TimeField)]
									+ SGridPanel::Slot(1, 2).Padding(0, RowPadY)[SNew(SSpacer)]
				]
				)
		]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[
			MakePanel(
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeIconLabel(
						TEXT("LIGHT SETTINGS"),
						TEXT("LightSetting"),
						&FontInfo10,
						ThemeAccent
					)
				]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)
				[
					HSeparator()
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
				[
					SNew(SGridPanel)
						.FillColumn(0, 1.f)
						.FillColumn(1, 1.f)

						+ SGridPanel::Slot(0, 0).Padding(0, RowPadY)[MakeFieldRow("Dawn Time", DawnField)]
						+ SGridPanel::Slot(1, 0).Padding(0, RowPadY)[MakeFieldRow("Dusk Time", DuskField)]

						+ SGridPanel::Slot(0, 1).Padding(0, RowPadY)[MakeFieldRow("Moon Phases", MoonComboField)]
						+ SGridPanel::Slot(1, 1).Padding(0, RowPadY)[SNew(SSpacer)]
				]
				)
		];
}

// -------------------------
// Scenario Env
// -------------------------
TSharedRef<SWidget> SWeatherDialog::BuildScenarioEnvContent()
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	auto MakePanel = [&](TSharedRef<SWidget> Inner) -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemeHeaderFill)
				.Padding(1)
				[
					SNew(SBorder)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(ThemeHeaderFill)
						.Padding(FMargin(10))
						[
							Inner
						]
				];
		};
	auto HSeparator = [&]() -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemePanelLine)
				.Padding(FMargin(0, 1));
		};
	auto MakeGroup = [&](const FString& Title) -> TSharedRef<SWidget>
		{
			return MakePanel(
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(STextBlock)
						.Text(FText::FromString(Title))
						.Font(FontInfo10)
						.ColorAndOpacity(FLinearColor::White)
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
				[
					SNew(STextBlock)
						.Text(FText::FromString("Add your controls here..."))
						.ColorAndOpacity(FLinearColor::White)
				]
			);
		};

	TSharedRef<SWidget> AreaOfInterestGroup =
		MakePanel(
			SNew(SVerticalBox)

			+ SVerticalBox::Slot().AutoHeight()
			[
				MakeIconLabel(
					TEXT("Area of Interest"),
					TEXT("Area"),
					&FontInfo10,
					FLinearColor::White
				)
			]
	+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)
		[
			HSeparator()
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SBorder)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(ThemeBgBase)
						.Padding(FMargin(6, 2))
						[
							SAssignNew(AreaCombo, SComboBox<TSharedPtr<FString>>)
								.ComboBoxStyle(&DarkComboStyle)
								.OptionsSource(&AreaOfInterestOptions)
								.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
									{
										return SNew(STextBlock)
											.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
											.ColorAndOpacity(FLinearColor::White);
									})
								.OnSelectionChanged(this, &SWeatherDialog::OnAOISelected)
								[
									SNew(STextBlock)
										.Text(this, &SWeatherDialog::GetSelectedAOIText)
										.ColorAndOpacity(FLinearColor::White)
								]

						]
				]

			+ SHorizontalBox::Slot().AutoWidth().Padding(6, 0, 0, 0)
				[
					SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
						.ContentPadding(4)
						.OnClicked(this, &SWeatherDialog::OnAreaAddClicked)
						[
							//MakeDropDownIcons(TEXT("compass"), 9)
							SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Plus"))
						]
				]

			+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
				[
					SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
						.ContentPadding(4)
						.OnClicked_Lambda([]()
							{
								return FReply::Handled();
							})
						[
							//	MakeDropDownIcons(TEXT("compass"), 9)
							SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Delete"))
						]
				]

			+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
				[
					SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
						.ContentPadding(4)
						.OnClicked_Lambda([]()
							{
								return FReply::Handled();
							})
						[
							//MakeDropDownIcons(TEXT("compass"), 9)
							SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Edit"))
						]
				]
		]
		);

	return SNew(SBorder)
		.BorderImage(WhiteBrush)
		.BorderBackgroundColor(ThemeBgBase)
		.Padding(0)
		[
			SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBorder)
						.Padding(FMargin(10, 5))
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(ThemeHeaderFill)
						[
							SNew(SHorizontalBox)

								+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
								[
									MakeIconLabel(
										TEXT("WEATHER"),
										TEXT("Weather"),
										&FontInfo12,
										Theme.Accent
									)
								]
						]
				]

			+ SVerticalBox::Slot().FillHeight(1.f).Padding(0, 8, 0, 0)
				[
					SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(ThemeHeaderFill)
						[
							SNew(SScrollBox)

								+ SScrollBox::Slot()
								.Padding(FMargin(0, 0, 0, 12))

								[
									SNew(SVerticalBox)
										.IsEnabled_Lambda([this]() { return !bDisableNtworkFeatures;  })
										+ SVerticalBox::Slot().AutoHeight()
										[
											AreaOfInterestGroup
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
										[
											SAssignNew(WeatherEnvironmentPanelWidget, SWeatherEnvironmentPanel)
												.Theme(Theme)
												.Styles(Styles)
												.EnvironmentController(EnvController.Get())
												.OnPresetAppliedOrReset(FSimpleDelegate::CreateLambda([this]()
													{
														if (WeatherEffectsPanelWidget.IsValid())
														{
															WeatherEffectsPanelWidget->RefreshFromEnvironmentData();
														}
													}))
										]
									+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
										[
											SAssignNew(WeatherEffectsPanelWidget, SWeatherEffectsPanel)
												.Theme(Theme)
												.Styles(Styles)
												.State(WeatherFxState)
												.EnvironmentController(EnvController.Get())
										]


								]
						]
				]

		];
}

// -------------------------
// Atmosphere
// -------------------------
TSharedRef<SWidget> SWeatherDialog::BuildAtmpshereContent()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(ThemeBgBase)
		.Padding(0)
		[
			SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBorder)
						.Padding(FMargin(10, 5))
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(ThemeHeaderFill)
						[
							SNew(SHorizontalBox)

								+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
								[
									SNew(STextBlock)
										.Text(FText::FromString("ATMOSPHERE"))
										.Font(FontInfo12)
										.ColorAndOpacity(Theme.Accent)
								]

								+ SHorizontalBox::Slot()
								.AutoWidth()
								.Padding(0, 0, 10, 0)
								[
									MakeIconLabel(
										TEXT("Network"),
										TEXT("Network"),
										&FontInfo8,
										FLinearColor::White
									)
								]

							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(SCheckBox)

										.Style(Styles.CheckStyle ? Styles.CheckStyle : &FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("Checkbox"))
										.IsChecked_Lambda([this]() {return ECheckBoxState::Checked; })
								]
						]
				]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					BuildAtmosphereGroup()
				]
		];
}

// -------------------------
// Others
// -------------------------
TSharedRef<SWidget> SWeatherDialog::BuildOthersScreenContent()
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	auto MakePanel = [&](TSharedRef<SWidget> Inner) -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemeHeaderFill)
				.Padding(1)
				[
					SNew(SBorder)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(ThemeHeaderFill)
						.Padding(FMargin(10))
						[
							Inner
						]
				];
		};

	auto HSeparator = [&]() -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemePanelLine)
				.Padding(FMargin(0, 1));
		};

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SBorder)
				.Padding(FMargin(10, 5))
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemeHeaderFill)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
						[
							MakeIconLabel(
								TEXT("Light Settings"),
								TEXT("LightSetting"),
								&FontInfo12,
								Theme.Accent
							)
						]
				]
		]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 8, 0, 0)
		[
			MakePanel(
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight()
				[
					MakeIconLabel(
						TEXT("Ambient Light Settings"),
						TEXT("AmbientLight"),
						&FontInfo10,
						FLinearColor::White
					)
				]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)
				[
					HSeparator()
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(5)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
						[
							MakeSliderWithField(
								TEXT("Ambient Cubemap "),
								0.f,
								100.f,
								AmbientLightField,
								130.f,
								[this]()
								{
									return State.IsValid() ? State->AmbientLightIntensity : 0.f;

								},
								[this](float NewValue)
								{
									if (!State.IsValid()) return;
									State->AmbientLightIntensity = NewValue;
									Data->Others.AmbientSettings.AmbientLightIntensity = NewValue;
									EnvController->UpdateAmbientLight(*Data);
								}
							)
						]
				]

			+ SVerticalBox::Slot().AutoHeight()
				[
					MakeIconLabel(
						TEXT("Shadows"),
						TEXT("shadow"),
						&FontInfo10,
						FLinearColor::White
					)
				]

			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)
				[
					HSeparator()
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					SNew(SHorizontalBox)
						//Disable Shadows
						+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(0, 0, 10, 0)
						[
							SNew(SButton)
								.ButtonColorAndOpacity_Lambda([this]()
									{
										return bIsShadowsEnable ? ThemeHeaderFill : FLinearColor(0.95f, 0.95f, 0.95f, 1.f);
									})
								//.ButtonColorAndOpacity(ThemeHeaderFill)
								.Text(FText::FromString("Disable Shadows"))
								.HAlign(HAlign_Center)
								.OnClicked_Lambda([this]()
									{
										if (EnvController.IsValid())
										{
											EnvController->ApplyShadows(false);
										}
										bIsShadowsEnable = false;

										return FReply::Handled();
									})
						]

					// Enable Shadows
					+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
						[
							SNew(SButton)
								.ButtonColorAndOpacity_Lambda([this]()
									{
										return bIsShadowsEnable ? FLinearColor(0.95f, 0.95f, 0.95f, 1.f) : ThemeHeaderFill;
									})
								//.ButtonColorAndOpacity(ThemeHeaderFill)
								.Text(FText::FromString("Enable Shadows"))
								.HAlign(HAlign_Center)
								.OnClicked_Lambda([this]()
									{
										if (EnvController.IsValid())
										{
											EnvController->ApplyShadows(true);
										}
										bIsShadowsEnable = true;

										return FReply::Handled();

									})
						]
				]

			+ SVerticalBox::Slot().AutoHeight()
				[
					MakeIconLabel(
						TEXT("SkySphere"),
						TEXT("shadow"),
						&FontInfo10,
						FLinearColor::White
					)
				]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 6)
				[
					HSeparator()
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					SNew(SHorizontalBox)
						//SkySphere1
						+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center).Padding(0, 0, 10, 0)
						[
							SNew(SButton)
							//	.ButtonColorAndOpacity(ThemeHeaderFill)
								.ButtonColorAndOpacity_Lambda([this]()
									{
										return SkySphere == ESkySphere::BlueSkySphere ? FLinearColor(0.95f, 0.95f, 0.95f, 1.f) : ThemeHeaderFill;
									})
								.Text(FText::FromString("Use Blue SkySphere"))
								.HAlign(HAlign_Center)
								.OnClicked_Lambda([this]()
									{
										if (EnvController.IsValid())
										{
											EnvController->UpdateSkySphere1();
										}
										SkySphere = ESkySphere::BlueSkySphere;
										return FReply::Handled();
									})
						]

					// SkySphere2
					+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)
						[
							SNew(SButton)
								//.ButtonColorAndOpacity(ThemeHeaderFill)
								.ButtonColorAndOpacity_Lambda([this]()
									{
										return SkySphere == ESkySphere::BlackSkySphere ? FLinearColor(0.95f, 0.95f, 0.95f, 1.f) : ThemeHeaderFill;
									})
								.Text(FText::FromString("Use Black SkySphere"))
								.HAlign(HAlign_Center)
								.OnClicked_Lambda([this]()
									{
										if (EnvController.IsValid())
										{
											EnvController->UpdateSkySphere2();
										}
										SkySphere = ESkySphere::BlackSkySphere;
										return FReply::Handled();

									})
						]
				]
			+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
				[
					SNew(SButton)
						//.ButtonColorAndOpacity(ThemeHeaderFill)
						.ButtonColorAndOpacity_Lambda([this]()
							{
								return SkySphere == ESkySphere::UDSSky ? FLinearColor(0.95f, 0.95f, 0.95f, 1.f) : ThemeHeaderFill;
							})
						.Text(FText::FromString("Use UltraDynamicSky"))
						.HAlign(HAlign_Center)
						.OnClicked_Lambda([this]()
							{
								if (EnvController.IsValid())
								{
									EnvController->UpdateUltraDynamicSky();
								}
								SkySphere = ESkySphere::UDSSky;
								return FReply::Handled();

							})
				]
				)
		];

}

TSharedRef<SWidget> SWeatherDialog::MakeSliderWithField(
	const FString& Label,
	float Min,
	float Max,
	TSharedPtr<SEditableTextBox>& OutField,
	float LabelWidth,
	TFunction<float()> GetValue,
	TFunction<void(float)> SetValue
)
{
	auto Clamp = [Min, Max](float V) { return FMath::Clamp(V, Min, Max); };

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
		[
			SNew(SBox).WidthOverride(LabelWidth)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Label))
						.ColorAndOpacity(FLinearColor::White)
						.Font(FontInfo10)
				]
		]

	+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center).Padding(0, 0, 10, 0)
		[
			SNew(SSlider)
				.MinValue(Min)
				.MaxValue(Max)
				.Value_Lambda([GetValue]() { return GetValue(); })
				.OnValueChanged_Lambda([SetValue, Clamp](float V)
					{
						SetValue(Clamp(V));
					})
		]

	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
		[
			SAssignNew(OutField, SEditableTextBox)
				.Text_Lambda([GetValue]() { return FText::AsNumber(GetValue()); })
				.OnTextCommitted_Lambda([SetValue, Clamp](const FText& T, ETextCommit::Type)
					{
						float Parsed = FCString::Atof(*T.ToString());
						SetValue(Clamp(Parsed));
					})
		];
}

void SWeatherDialog::OnPickerCommitted(const FName PickerType, const FString& Value)
{
	if (!Data)
		return;
	if (PickerType == TEXT("Date"))
	{
		DateValue = Value;
		if (DateTextBox.IsValid())
		{
			DateTextBox->SetText(FText::FromString(DateValue));
		}
		ParseDateTime();
		ActiveDateTarget.Reset();
	}
	else if (PickerType == TEXT("Time"))
	{
		if (TSharedPtr<SEditableTextBox> Target = ActiveTimeTarget.Pin())
		{
			Target->SetText(FText::FromString(Value));

			if (Target == TimeTextBox)
			{
				TimeValue = Value;
				ParseDateTime();
			}
			else if (Target == DawnTextBox)
			{
				DawnValue = Value;

				int32 H24, M;
				if (Data && ParseUIText12h_ToHM(DawnValue, H24, M))
				{
					Data->TimeOfDay.LightSettings.DawnTime = FDateTime(2000, 1, 1, H24, M, 0);
					EnvController->UpdateLightSettingsData(*Data);
				}
			}
			else if (Target == DuskTextBox)
			{
				DuskValue = Value;

				int32 H24, M;
				if (Data && ParseUIText12h_ToHM(DuskValue, H24, M))
				{
					Data->TimeOfDay.LightSettings.DuskTime = FDateTime(2000, 1, 1, H24, M, 0);
					EnvController->UpdateLightSettingsData(*Data);
				}
			}
		}
		ActiveTimeTarget = TimeTextBox;
	}
}

// -------------------------
// Popups
// -------------------------
void SWeatherDialog::ShowCalendarPopup()
{
	if (!DateTextBox.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ShowCalendarPopup: DateTextBox invalid."));
		return;
	}

	if (TSharedPtr<SWindow> Existing = CalendarPopup.Pin())
	{
		Existing->BringToFront();
		return;
	}

	const FVector2D PopupSize(380, 360);
	const FVector2D PopupPos = GetPopupPosUnderWidget(DateTextBox.ToSharedRef(), PopupSize);

	TSharedRef<SWindow> Win = SNew(SWindow)
		.Title(FText::FromString("Select Date"))
		.ClientSize(PopupSize)
		.AutoCenter(EAutoCenter::None)
		.ScreenPosition(PopupPos)
		.SizingRule(ESizingRule::Autosized);

	CalendarPopup = Win;
	Win->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>&)
		{
			CalendarPopup.Reset();
		}));

	Win->SetContent(
		SNew(SDateTimePicker)
		.PickerType(TEXT("Date"))
		.OwningWindow(Win)
		.OwnerDialog(SharedThis(this))
		.TargetField(DateTextBox)
		.BgBase(ThemeBgBase)
		.PanelFill(ThemePanelFill)
		.HeaderFill(ThemeHeaderFill)
		.PanelLine(ThemePanelLine)
		.Accent(ThemeAccent)

	);

	FSlateApplication::Get().AddWindow(Win);
}

void SWeatherDialog::ShowClockPopup(TSharedPtr<SEditableTextBox> Target)
{
	if (!Target.IsValid())
	{
		Target = ActiveTimeTarget.Pin();
	}

	if (!Target.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ShowClockPopup: Target invalid."));
		return;
	}
	if (TSharedPtr<SWindow> Existing = ClockPopup.Pin())
	{
		Existing->BringToFront();
		Existing->RequestDestroyWindow();
		ClockPopup.Reset();
		return;
	}

	const FVector2D PopupSize(380, 220);
	const FVector2D PopupPos = GetPopupPosUnderWidget(Target.ToSharedRef(), PopupSize);

	TSharedRef<SWindow> Win = SNew(SWindow)
		.Title(FText::FromString("Select Time"))
		.ClientSize(PopupSize)
		.AutoCenter(EAutoCenter::None)
		.ScreenPosition(PopupPos)
		.SizingRule(ESizingRule::Autosized);

	ClockPopup = Win;

	Win->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>&)
		{
			ClockPopup.Reset();
		}));

	Win->SetContent(
		SNew(SDateTimePicker)
		.PickerType(TEXT("Time"))
		.OwningWindow(Win)
		.TargetField(Target)
		.OwnerDialog(SharedThis(this))
		.BgBase(ThemeBgBase)
		.PanelFill(ThemePanelFill)
		.HeaderFill(ThemeHeaderFill)
		.PanelLine(ThemePanelLine)
		.Accent(ThemeAccent)
	);

	FSlateApplication::Get().AddWindow(Win);
}

FReply SWeatherDialog::OnAreaAddClicked()
{
	if (AddAreaWindow.IsValid())
	{
		AddAreaWindow->BringToFront();
		return FReply::Handled();
	}

	const FVector2D PopupSize(740, 560);

	AddAreaWindow =
		SNew(SWindow)
		.Title(FText::FromString("Area of Interest"))
		.ClientSize(PopupSize)
		.SizingRule(ESizingRule::FixedSize)
		.AutoCenter(EAutoCenter::PreferredWorkArea);

	AddAreaWindow->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>&)
		{
			AddAreaWindow.Reset();
		}));

	AddAreaWindow->SetContent(
		SNew(SAreaOfInterestDialog)
		.OwningWindow(AddAreaWindow)
		.BgBase(ThemeBgBase)
		.HeaderFill(ThemeHeaderFill)
		.PanelFill(ThemePanelFill)
		.PanelLine(ThemePanelLine)
		.Accent(ThemeAccent)
		.TextBoxStyle(&DarkTextBoxStyle)
		.ButtonStyle(&NavButtonStyle)
		.Styles(Styles)
		.EnvironmentController(EnvController.Get())
		.OnAddConfirmed(FSimpleDelegate::CreateSP(
			this, &SWeatherDialog::RefreshAreaOfInterestCombo))



	);

	FSlateApplication::Get().AddWindow(AddAreaWindow.ToSharedRef());
	return FReply::Handled();
}

void SWeatherDialog::RefreshAreaOfInterestCombo()
{
	AreaOfInterestOptions.Reset();

	if (!EnvController.IsValid())
	{
		return;
	}

	TMap<FString, FAreaOfInterest*> AOIs =
		EnvController->GetEnvironmentData()
		->LocalScenarioEnvironment
		.GetAreaOfInterestPresets();

	for (TPair<FString, FAreaOfInterest*>& AOIPair : AOIs)
	{
		FAreaOfInterest* AOI = AOIPair.Value;

		AreaOfInterestOptions.Add(MakeShared<FString>(AOI->GetLabel()));
	}

	if (AreaOfInterestOptions.Num() > 0)
	{
		SelectedAreaOption = AreaOfInterestOptions.Last();
	}

	if (AreaCombo.IsValid())
	{
		AreaCombo->RefreshOptions();
		AreaCombo->SetSelectedItem(SelectedAreaOption);
	}
}

void SWeatherDialog::RefreshAreaOfInterestOptions()
{
	AreaOfInterestOptions.Empty();

	TMap<FString, FAreaOfInterest*> AOIs =
		EnvController->GetEnvironmentData()
		->LocalScenarioEnvironment
		.GetAreaOfInterestPresets();

	int32 index = 0;
	for (TPair<FString, FAreaOfInterest*> AOIPair : AOIs)
	{
		FAreaOfInterest* AOI = AOIPair.Value;
		FString Label = FString::Printf(
			TEXT("AOI %d "),
			++index);

		AreaOfInterestOptions.Add(MakeShared<FString>(Label));
	}

	if (AreaCombo.IsValid())
	{
		AreaCombo->RefreshOptions();

		if (AreaOfInterestOptions.Num() > 0)
		{
			SelectedAreaOption = AreaOfInterestOptions.Last();
			AreaCombo->SetSelectedItem(SelectedAreaOption);
		}
	}
}

//---------------
//   Atmosphere
//---------------
TSharedRef<SWidget> SWeatherDialog::BuildAtmosphereGroup()
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	auto MakePanel = [&](TSharedRef<SWidget> Inner) -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemeHeaderFill)
				.Padding(1)
				[
					SNew(SBorder)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(ThemeHeaderFill)
						.Padding(FMargin(10))
						[
							Inner
						]
				];
		};

	auto MakeSubGroupBox = [&](TSharedRef<SWidget> Inner) -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.Padding(1)
				[
					SNew(SBorder)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(ThemeHeaderFill)
						.Padding(FMargin(10, 8))
						[
							Inner
						]
				];
		};

	auto HSeparator = [&]() -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(ThemePanelLine)
				.Padding(FMargin(0, 1));
		};

	auto SubHeader = [&](const FString& Text, const FString& IconPath) -> TSharedRef<SWidget>
		{
			TSharedPtr<FSlateBrush> IconBrush = GetIconBrush(IconPath, 16.0f);
			if (!IconBrush.IsValid()) return SNullWidget::NullWidget;

			return SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 6, 0)
				[
					SNew(SBox)
						.WidthOverride(16.f)
						.HeightOverride(16.f)
						[
							SNew(SImage)
								.Image(IconBrush.Get())
								.ColorAndOpacity(ThemeAccent)
						]
				]

			+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Text))
						.Font(FontInfo10)
						.ColorAndOpacity(FLinearColor::White)
				];
		};

	auto FieldShell = [&](TSharedRef<SWidget> Inner) -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.Padding(1)
				[
					SNew(SBorder)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(ThemeBgBase)
						.Padding(FMargin(6, 3))
						[
							Inner
						]
				];
		};

	auto ReadOnlyBox = [&](const FString& Value) -> TSharedRef<SWidget>
		{
			return FieldShell(
				SNew(SEditableTextBox)
				.Style(&DarkTextBoxStyle)
				.IsReadOnly(true)
				.Text(FText::FromString(Value))
				.Padding(FMargin(6, 3))
			);
		};

	const float LabelW = 90.f;
	const float FieldW_Left = 240.f;
	const float FieldW_Right = 240.f;
	const float ColGap = 20.f;

	auto Label = [&](const FString& T) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.WidthOverride(LabelW)
				[
					SNew(STextBlock)
						.Text(FText::FromString(T))
						.Font(FontInfo10)
						.ColorAndOpacity(FLinearColor::White)
				];
		};

	auto TwoColRow = [&](const FString& L1, const FString& V1, const FString& L2, const FString& V2) -> TSharedRef<SWidget>
		{
			return SNew(SBox)
				.Padding(FMargin(0, 0, 2, 0))
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							Label(L1)
						]

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SBox).WidthOverride(FieldW_Left)
								[
									ReadOnlyBox(V1)
								]
						]

						+ SHorizontalBox::Slot().AutoWidth()
						[
							SNew(SBox).WidthOverride(ColGap)[SNew(SSpacer)]
						]

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							Label(L2)
						]

						+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
						[
							SNew(SBox)
								.MinDesiredWidth(FieldW_Right)
								[
									ReadOnlyBox(V2)
								]
						]
				];
		};

	auto OneColRow = [&](const FString& L1, const FString& V1) -> TSharedRef<SWidget>
		{
			return SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					Label(L1)
				]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SBox).WidthOverride(FieldW_Left)
						[
							ReadOnlyBox(V1)
						]
				]
				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					SNew(SSpacer)
				];
		};

	/*const FSlateBrush* AtmosIcon = FAppStyle::Get().GetBrush("Icons.Warning");
	const FSlateBrush* IcingIcon = FAppStyle::Get().GetBrush("Icons.Warning");*/
	/*const FSlateBrush* AtmosIcon = MakeDropDownIcons(TEXT("/MetaverseWeatherSystem/compass.compass"), 9);
	const FSlateBrush* IcingIcon = MakeDropDownIcons(TEXT("/MetaverseWeatherSystem/compass.compass"), 9);*/


	return MakePanel(
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[
			MakeIconLabel(
				TEXT("ATMOSPHERE"),
				TEXT("Atmosphere"),
				&FontInfo10,
				FLinearColor(.92f, 0.96f, 1.0f, 1.f)
			)
		]

	// =========================
	// SUBGROUP: ATMOSPHERE DATA 
	// =========================
	+SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
		[
			MakeSubGroupBox(
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight()
				[
					SubHeader(
						TEXT("ATMOSPHERE DATA"),
						TEXT("AtmosphereData1")
					)
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
				[
					HSeparator()
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
				[
					SNew(SVerticalBox)

						+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
						[
							TwoColRow(TEXT("Humidity"), TEXT("0.0085"), TEXT("Inversion Layer m"), TEXT("IL-03"))
						]

						+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
						[
							TwoColRow(TEXT("Pressure N/m2"), TEXT("1013"), TEXT("Temp °C"), TEXT("18.5"))
						]

						+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
						[
							OneColRow(TEXT("Temp Grad"), TEXT("Stable"))
						]
				]
			)
		]

	// =========================
	// SUBGROUP: ICING 
	// =========================
	+SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
		[
			MakeSubGroupBox(
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight()
				[
					SubHeader(
						TEXT("ICING"),
						TEXT("Icing")
					)
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 0)
				[
					HSeparator()
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
				[
					SNew(SVerticalBox)

						+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
						[
							TwoColRow(TEXT("Type"), TEXT("Rime"), TEXT("Severity"), TEXT("Light"))
						]
				]
			)
		]
		);
}

void SWeatherDialog::OnAOISelected(
	TSharedPtr<FString> Selected,
	ESelectInfo::Type)
{
	if (!Selected.IsValid())
		return;

	SelectedAreaOption = Selected;

	const TMap<FString, FAreaOfInterest*>& AOIs =
		EnvController->GetEnvironmentData()
		->LocalScenarioEnvironment
		.GetAreaOfInterestPresets();

	FString SelectedKey = *Selected;
	FAreaOfInterest* AOI = AOIs.FindRef(SelectedKey);
	EnvController->GetEnvironmentData()
		->LocalScenarioEnvironment
		.SetSelectedAreaOfInterestByKey(SelectedKey);
}

FText SWeatherDialog::GetSelectedAOIText() const
{
	return SelectedAreaOption.IsValid()
		? FText::FromString(*SelectedAreaOption)
		: FText::FromString(TEXT("Select Area of Interest"));
}

TSharedPtr<FString> SWeatherDialog::FindOptionByText(const TArray<TSharedPtr<FString>>& Options, const FString& Value)
{
	for (const TSharedPtr<FString>& Item : Options)
	{
		if (Item.IsValid() && Item->Equals(Value, ESearchCase::IgnoreCase))
		{
			return Item;
		}
	}

	return Options.Num() > 0 ? Options[0] : nullptr;
}

void SWeatherDialog::LoadFromEnvironmentData()
{
	if (!Data)
	{
		return;
	}
	
	const FDateTime& Date = Data->TimeOfDay.TimeSettings.Date;
	const FDateTime& Time = Data->TimeOfDay.TimeSettings.Time;

	if (Date.GetYear() > 1)
	{
		DateValue = FormatDateDDMMYYYY(Date);
	}

	if (Time.GetYear() > 1)
	{
		TimeValue = FormatTime12h(Time);
	}

	const FDateTime& Dawn = Data->TimeOfDay.LightSettings.DawnTime;
	const FDateTime& Dusk = Data->TimeOfDay.LightSettings.DuskTime;

	if (Dawn.GetYear() > 1)
	{
		DawnValue = FormatTime12h(Dawn);
	}

	if (Dusk.GetYear() > 1)
	{
		DuskValue = FormatTime12h(Dusk);
	}

	switch (Data->TimeOfDay.TimeSettings.SpeedMode)
	{
	case ETimeSpeedMode::RealTime:
		SelectedSpeedOfTime = FindOptionByText(SpeedOptions, TEXT("Real Time"));
		break;

	case ETimeSpeedMode::X2:
		SelectedSpeedOfTime = FindOptionByText(SpeedOptions, TEXT("20X"));
		break;

	case ETimeSpeedMode::X3:
		SelectedSpeedOfTime = FindOptionByText(SpeedOptions, TEXT("30X"));
		break;

	case ETimeSpeedMode::X5:
		SelectedSpeedOfTime = FindOptionByText(SpeedOptions, TEXT("50X"));
		break;

	case ETimeSpeedMode::X8:
		SelectedSpeedOfTime = FindOptionByText(SpeedOptions, TEXT("80X"));
		break;

	case ETimeSpeedMode::X10:
		SelectedSpeedOfTime = FindOptionByText(SpeedOptions, TEXT("100X"));
		break;

	default:
		SelectedSpeedOfTime = FindOptionByText(SpeedOptions, TEXT("Real Time"));
		break;
	}

	switch (Data->TimeOfDay.LightSettings.MoonPhase)
	{
	case EMoonPhase::FullMoon:
		SelectedMoonPhase = FindOptionByText(MoonPhases, TEXT("Full Moon"));
		break;

	case EMoonPhase::NewMoon:
		SelectedMoonPhase = FindOptionByText(MoonPhases, TEXT("New Moon"));
		break;

	case EMoonPhase::Wanning:
		SelectedMoonPhase = FindOptionByText(MoonPhases, TEXT("Wanning"));
		break;

	case EMoonPhase::Waxing:
		SelectedMoonPhase = FindOptionByText(MoonPhases, TEXT("Waxing"));
		break;

	default:
		SelectedMoonPhase = FindOptionByText(MoonPhases, TEXT("Full Moon"));
		break;
	}

	if (State.IsValid())
	{
		State->AmbientLightIntensity = Data->Others.AmbientSettings.AmbientLightIntensity;
	}

}