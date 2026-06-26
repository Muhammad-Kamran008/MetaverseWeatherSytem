// WeatherEffectsPanel.cpp
#include "WeatherEffectsPanel.h"

#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"

#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "WeatherPrecipitationMapping.h"


SWeatherEffectsPanel::SWeatherEffectsPanel()
	:FontAsset(LoadObject<UObject>(nullptr, TEXT("/Game/Fonts/segoeui_Font.segoeui_Font")))
	, FontInfo12(FontAsset, 12, FName("Default"))
	, FontInfo10(FontAsset, 10, FName("Default"))
	, FontInfo8(FontAsset, 9, FName("Default"))
{}

void SWeatherEffectsPanel::Construct(const FArguments& InArgs)
{
	Theme = InArgs._Theme;
	Styles = InArgs._Styles;
	State = InArgs._State;

	EnvController = InArgs._EnvironmentController;
	//SAssignNew(PresetPanel, SWeatherEnvironmentPanel);

	if (!State.IsValid())
	{
		State = MakeShared<FWeatherEffectsState>();
	}

	// -----------------------------
	// Flatten text box style
	// -----------------------------
	{
		const FEditableTextBoxStyle* Base =
			Styles.TextBoxStyle
			? Styles.TextBoxStyle
			: &FAppStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox");

		FlatTextBoxStyle = *Base;
		FlatTextBoxStyle.BackgroundImageHovered = FlatTextBoxStyle.BackgroundImageNormal;
		FlatTextBoxStyle.BackgroundImageFocused = FlatTextBoxStyle.BackgroundImageNormal;
		FlatTextBoxStyle.BackgroundImageReadOnly = FlatTextBoxStyle.BackgroundImageNormal;

		TableTextBoxStyle = FlatTextBoxStyle;
		TableTextBoxStyle.BackgroundImageNormal = FSlateNoResource();
		TableTextBoxStyle.BackgroundImageHovered = FSlateNoResource();
		TableTextBoxStyle.BackgroundImageFocused = FSlateNoResource();
		TableTextBoxStyle.BackgroundImageReadOnly = FSlateNoResource();
	}

	// -----------------------------
	// Flatten combo style
	// -----------------------------
	{
		const FComboBoxStyle* Base =
			Styles.ComboStyle
			? Styles.ComboStyle
			: &FAppStyle::Get().GetWidgetStyle<FComboBoxStyle>("ComboBox");

		FlatComboStyle = *Base;

		FlatComboStyle.ComboButtonStyle.ButtonStyle.Hovered = FSlateColorBrush(FLinearColor::Gray);
		FlatComboStyle.ComboButtonStyle.ButtonStyle.Pressed = FlatComboStyle.ComboButtonStyle.ButtonStyle.Normal;
	}

	// -----------------------------
	// Stop STableRow painting default backgrounds
	// -----------------------------
	FlatTableViewStyle = FAppStyle::Get().GetWidgetStyle<FTableViewStyle>("TableView");
	{
		FSlateBrush Bg = FlatTableViewStyle.BackgroundBrush;
		Bg.TintColor = FSlateColor(Theme.BgBase);
		FlatTableViewStyle.BackgroundBrush = Bg;
	}
	FlatTableRowStyle = FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("TableView.Row");
	FlatTableRowStyle.SetEvenRowBackgroundBrush(FSlateNoResource());
	FlatTableRowStyle.SetOddRowBackgroundBrush(FSlateNoResource());
	FlatTableRowStyle.SetActiveBrush(FSlateNoResource());
	FlatTableRowStyle.SetActiveHoveredBrush(FSlateColorBrush(FLinearColor::Gray));
	FlatTableRowStyle.SetInactiveBrush(FSlateNoResource());
	FlatTableRowStyle.SetInactiveHoveredBrush(FSlateNoResource());
	FlatTableRowStyle.SetSelectorFocusedBrush(FSlateNoResource());

	// -----------------------------
	// Table colors
	// -----------------------------
	TableFillColor = Theme.Accent;  
	TableCellFillColor = Theme.HeaderFill;     
	TableGridLineColor = Theme.Accent; 

	// -----------------------------
	// Options
	// -----------------------------
	PrecipTypeOptions = { MakeShared<FString>(TEXT("None")), MakeShared<FString>(TEXT("Snow")), MakeShared<FString>(TEXT("Rain")) , MakeShared<FString>(TEXT("Hail")) };
	PrecipIntensityOptions = {  MakeShared<FString>(TEXT("None")), MakeShared<FString>(TEXT("Light")) , MakeShared<FString>(TEXT("Moderate")) , MakeShared<FString>(TEXT("Heavy")), MakeShared<FString>(TEXT("Extreme")) };
	VisibilityObscurantOptions = { MakeShared<FString>(TEXT("None")), MakeShared<FString>(TEXT("Dust")), MakeShared<FString>(TEXT("Fog")), MakeShared<FString>(TEXT("Haze")) };

	CloudsLayersOptions = { MakeShared<FString>(TEXT("Layer 1 - 3000ft")), MakeShared<FString>(TEXT("Layer 2 - 8000ft")) };
	CloudsTypeOptions = { MakeShared<FString>(TEXT("CompleteClearSkies")), MakeShared<FString>(TEXT("Cirrocumulus")), MakeShared<FString>(TEXT("Partly Cloudy")), MakeShared<FString>(TEXT("Cirrostratus")) , MakeShared<FString>(TEXT("CumulusCongestus")) };
	CloudsCoverOptions = { MakeShared<FString>(TEXT("Clear")), MakeShared<FString>(TEXT("Few")), MakeShared<FString>(TEXT("Scattered")), MakeShared<FString>(TEXT("Broken")), MakeShared<FString>(TEXT("Overcast")) };
	ThunderOptions = {
	MakeShared<FString>(TEXT("None")),
	MakeShared<FString>(TEXT("Light")),
	MakeShared<FString>(TEXT("Moderate")),
	MakeShared<FString>(TEXT("Heavy"))
	};

	WindLayersOptions = { MakeShared<FString>(TEXT("Surface - 0ft")), MakeShared<FString>(TEXT("Layer 2 - 5000ft")) };

	/*SelectedPrecipType = PrecipTypeOptions[0];
	SelectedPrecipIntensity = PrecipIntensityOptions[4];
	SelectedVisibilityObscurant = VisibilityObscurantOptions[0];

	SelectedCloudLayer = CloudsLayersOptions[0];
	SelectedCloudType = CloudsTypeOptions[0];
	SelectedCloudCover = CloudsCoverOptions[0];
	SelectedThunder = ThunderOptions[0];

	SelectedWindLayer = WindLayersOptions[0];*/

	SelectedPrecipType = PrecipTypeOptions.Num() > 0 ? PrecipTypeOptions[0] : nullptr;
	SelectedPrecipIntensity = PrecipIntensityOptions.Num() > 0 ? PrecipIntensityOptions[0] : nullptr;
	SelectedVisibilityObscurant = VisibilityObscurantOptions.Num() > 0 ? VisibilityObscurantOptions[0] : nullptr;
	SelectedCloudLayer = CloudsLayersOptions.Num() > 0 ? CloudsLayersOptions[0] : nullptr;
	SelectedCloudType = CloudsTypeOptions.Num() > 0 ? CloudsTypeOptions[0] : nullptr;
	SelectedCloudCover = CloudsCoverOptions.Num() > 0 ? CloudsCoverOptions[0] : nullptr;
	SelectedThunder = ThunderOptions.Num() > 0 ? ThunderOptions[0] : nullptr;
	SelectedWindLayer = WindLayersOptions.Num() > 0 ? WindLayersOptions[0] : nullptr;

	// -----------------------------
	// Seed rows 
	// -----------------------------

	SyncFromEnvironmentData();

	// -----------------------------
	// Table combo tint 
	// -----------------------------
	TableComboStyle = FlatComboStyle;

	auto Tint = [](FSlateBrush& B, const FLinearColor& C)
		{
			B.TintColor = FSlateColor(C);
		};

	Tint(TableComboStyle.ComboButtonStyle.ButtonStyle.Normal, Theme.PanelFill);
	Tint(TableComboStyle.ComboButtonStyle.ButtonStyle.Hovered, Theme.BgBase);
	Tint(TableComboStyle.ComboButtonStyle.ButtonStyle.Pressed, TableCellFillColor);
	Tint(TableComboStyle.ComboButtonStyle.ButtonStyle.Disabled, TableCellFillColor);

	Tint(TableComboStyle.ComboButtonStyle.MenuBorderBrush, TableCellFillColor);
	TableComboStyle.ComboButtonStyle.DownArrowImage.TintColor = Theme.PanelLine;; 
	
	// -----------------------------
	// Root
	// -----------------------------
	ChildSlot
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
				[
					MakeEffectsContainer(
						SNew(SVerticalBox)

						+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
						[
							MakeSubGroupPanel(
								TEXT("PRECIPITATION"),
								BuildPrecipitation(),
								bPrecipEnabled,
								TEXT("Precipitation")
							)
						]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
						[
							MakeSubGroupPanel(
								TEXT("VISIBILITY"),
								BuildVisibility(),
								bVisibilityEnabled,
								TEXT("Visibility")
							)

						]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
						[
							MakeSubGroupPanel(
								TEXT("CLOUDS"),
								BuildClouds(),
								bCloudsEnabled,
								TEXT("Clouds")
							)
						]
					+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
						[
							MakeSubGroupPanel(
								TEXT("WIND"),
								BuildWind(),
								bWindEnabled,
								TEXT("Wind")
							)
						]
						)
				]
		];

	UpdateDownwindFromDir();
}

TSharedPtr<FSlateBrush> SWeatherEffectsPanel::GetIconBrush(FString IconName, float IconSize)
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


TSharedRef<SWidget> SWeatherEffectsPanel::MakeIconLabel(
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
TSharedRef<SWidget> SWeatherEffectsPanel::MakeDropDownIcons(
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
// -------------------------
// Common helpers
// -------------------------
TSharedRef<SWidget> SWeatherEffectsPanel::MakeSeparator(float Thickness)
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(Theme.PanelLine)
		.Padding(FMargin(0, Thickness));
}

TSharedRef<SWidget> SWeatherEffectsPanel::MakeEffectsContainer(const TSharedRef<SWidget>& Inner)
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(Theme.HeaderFill) 
		.Padding(1)
		[
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Theme.HeaderFill)
				.Padding(FMargin(5, 5)) 
				[
					SNew(SVerticalBox)

						+ SVerticalBox::Slot().AutoHeight()
						[
							MakeIconLabel(
								TEXT("WEATHER EFFECTS"),
								TEXT("WeatherEffects"),
								&FontInfo10,
								FLinearColor::White
							)
						]

						+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 10)
						[
							MakeSeparator(1.0)
						]

						+ SVerticalBox::Slot().AutoHeight()
						[
							Inner
						]
				]
		];
}

TSharedRef<SWidget> SWeatherEffectsPanel::MakeSubGroupPanel(
	const FString& Title,
	const TSharedRef<SWidget>& Body,
	bool& EnableFlag,
	const FString& IconPath
)
{
	TSharedPtr<FSlateBrush> IconBrush = GetIconBrush(IconPath, 16.0f);
	if (!IconBrush.IsValid()) return SNullWidget::NullWidget;

	return SNew(SBorder)
		.Padding(1)
		[
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Theme.HeaderFill)
				.Padding(FMargin(10, 8))
				[
					SNew(SVerticalBox)

						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SHorizontalBox)

								+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
								[
									SNew(SHorizontalBox)

										+ SHorizontalBox::Slot()
										.AutoWidth()
										.VAlign(VAlign_Center)
										.Padding(0, 0, 6, 0)
										[
											SNew(SBox)
												.WidthOverride(16.f)
												.HeightOverride(16.f)
												[
													SNew(SImage)
														.Image(IconBrush.Get())
														.ColorAndOpacity(FLinearColor::White)
												]
										]

									+ SHorizontalBox::Slot()
										.FillWidth(1.f)
										.VAlign(VAlign_Center)
										[
											SNew(STextBlock)
												.Text(FText::FromString(Title))
												.Font(FontInfo8)
												.ColorAndOpacity(FLinearColor::White)
										]
								]

							+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
								[
									SNew(SCheckBox)
										.Style(Styles.CheckStyle ? Styles.CheckStyle : &FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("Checkbox"))
										.IsChecked_Lambda([&EnableFlag]() { return EnableFlag ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
										.OnCheckStateChanged_Lambda([&EnableFlag](ECheckBoxState S) { EnableFlag = (S == ECheckBoxState::Checked); })
								]
						]

					+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 6)
						[
							MakeSeparator(0.1)
						]

						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBox)
								.IsEnabled_Lambda([&EnableFlag]() { return EnableFlag; })
								[
									Body
								]
						]
				]
		];
}

TSharedRef<SWidget> SWeatherEffectsPanel::MakeCombo(
	TArray<TSharedPtr<FString>>* Source,
	TSharedPtr<FString>& InOutSelected,
	const FString& DefaultText,
	float MinWidth,
	TFunction<void()> OnChanged,
	TSharedPtr<SComboBox<TSharedPtr<FString>>>* OutComboBox
)
{
	auto GenItem = [](TSharedPtr<FString> Item)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
				.ColorAndOpacity(FLinearColor::White)
				.Font(FCoreStyle::GetDefaultFontStyle("Light", 9));
		};
	const bool bIsPrecipIntensityCombo = (Source == &PrecipIntensityOptions); 
	TSharedPtr<SComboBox<TSharedPtr<FString>>> ComboBoxWidget = SNew(SComboBox<TSharedPtr<FString>>)
		.ComboBoxStyle(&FlatComboStyle)
		.OptionsSource(Source)
		.InitiallySelectedItem(InOutSelected)
		.OnGenerateWidget_Lambda(GenItem)
		.OnSelectionChanged_Lambda(
			[this, &InOutSelected, OnChanged, bIsPrecipIntensityCombo](TSharedPtr<FString> Item, ESelectInfo::Type)
			{
				if (!Item.IsValid())
					return;

				/*if (bIsPrecipIntensityCombo && bUpdatingPrecipFromUI)
				{
					InOutSelected = Item;
					return;
				}*/
				
				InOutSelected = Item;
				if (!bUpdatingPrecipFromUI && OnChanged)
				{
					OnChanged();
				}
			}
		)
		[
			SNew(STextBlock)
				.Text_Lambda([&InOutSelected, DefaultText]()
					{
						return FText::FromString(
							InOutSelected.IsValid()
							? *InOutSelected
							: DefaultText
						);
					})
				.ColorAndOpacity(FLinearColor::White)
				.Font(FontInfo8)
		];
	if (OutComboBox)
	{
		*OutComboBox = ComboBoxWidget;
	}
	/*if (bIsPrecipIntensityCombo) 
	{
		PrecipIntensityComboBox = ComboBoxWidget;
	}*/

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(Theme.BgBase)
		.Padding(1)
		[
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Theme.BgBase)
				.Padding(FMargin(6, 2))
				[
					SNew(SBox)
						.MinDesiredWidth(static_cast<float>(MinWidth))
						[
							ComboBoxWidget.ToSharedRef()
						]
				]
		];
	
}

bool SWeatherEffectsPanel::TryParseFloat(const FString& In, float& Out)
{
	FString S = In.TrimStartAndEnd();
	if (S.IsEmpty()) return false;

	S.ReplaceInline(TEXT("km/h"), TEXT(""));
	S.ReplaceInline(TEXT("ft"), TEXT(""));
	S.ReplaceInline(TEXT("km"), TEXT(""));
	S.ReplaceInline(TEXT("mm/h"), TEXT(""));
	S = S.TrimStartAndEnd();

	bool bHasDigit = false;
	for (TCHAR C : S)
	{
		if (FChar::IsDigit(C)) { bHasDigit = true; break; }
	}
	if (!bHasDigit) return false;

	Out = FCString::Atof(*S);
	return true;
}

TSharedRef<SWidget> SWeatherEffectsPanel::MakeSliderWithField(
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
						.Font(FontInfo8)
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


void SWeatherEffectsPanel::UpdateDownwindFromDir()
{
	if (DownwindField.IsValid() && State.IsValid())
	{
		DownwindField->SetText(FText::AsNumber(FMath::RoundToInt(State->WindDirDeg)));
	}
}

void SWeatherEffectsPanel::SyncFromEnvironmentData()
{
	if (!EnvController.IsValid()) return;

	FEnvironmentData* Data = EnvController->GetEnvironmentData();
	if (!Data || !State.IsValid()) return;

	const auto* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
	if (!AOI) return;

	const auto& Weather = AOI->CurrentWeatherEffect;

	bPrecipEnabled = Weather.Precipitation.bEnabled;
	bVisibilityEnabled = Weather.Visibility.bEnabled;
	bCloudsEnabled = Weather.Clouds.bEnabled;
	bWindEnabled = Weather.Wind.bEnabled;

	State->PrecipMmPerHr = Weather.Precipitation.RateMmPerHr;
	State->VisibilityKm = Weather.Visibility.DistanceKm;
	State->CloudsBaseFt = Weather.Clouds.CloudLayer.BaseFt;
	State->CloudsTopFt = Weather.Clouds.CloudLayer.TopFt;
	State->WindDirDeg = Weather.Wind.CurrentWindLayer.DirectionDeg;
	State->WindSpeedKt = Weather.Wind.CurrentWindLayer.SpeedKt;

	if (Weather.Precipitation.Type == EPrecipitationType::Rain)         SelectedPrecipType = PrecipTypeOptions[2];
	else if (Weather.Precipitation.Type == EPrecipitationType::Snow)    SelectedPrecipType = PrecipTypeOptions[1];
	else if (Weather.Precipitation.Type == EPrecipitationType::Hail)    SelectedPrecipType = PrecipTypeOptions[3];
	else                                                                SelectedPrecipType = PrecipTypeOptions[0];

	switch (Weather.Precipitation.Intensity)
	{
	case EPrecipitationIntensity::Light: SelectedPrecipIntensity = PrecipIntensityOptions[1]; break;
	case EPrecipitationIntensity::Moderate:       SelectedPrecipIntensity = PrecipIntensityOptions[2]; break;
	case EPrecipitationIntensity::Heavy:       SelectedPrecipIntensity = PrecipIntensityOptions[3]; break;
	case EPrecipitationIntensity::Extreme:       SelectedPrecipIntensity = PrecipIntensityOptions[4]; break;
	default:                             SelectedPrecipIntensity = PrecipIntensityOptions[0]; break;
	}

	switch (Weather.Visibility.Obscurant)
	{
	case EVisibilityObscurant::Dust:  SelectedVisibilityObscurant = VisibilityObscurantOptions[1]; break;
	case EVisibilityObscurant::Fog:       SelectedVisibilityObscurant = VisibilityObscurantOptions[2]; break;
	default:                             SelectedVisibilityObscurant = VisibilityObscurantOptions[0]; break;
	}
	switch (Weather.Clouds.CloudLayer.Type)
	{
	case ECloudType::CompleteClearSkies: SelectedCloudType = CloudsTypeOptions[0]; break;
	case ECloudType::Cirrocumulus:       SelectedCloudType = CloudsTypeOptions[1]; break;
	case ECloudType::PartlyCloudy:       SelectedCloudType = CloudsTypeOptions[2]; break;
	case ECloudType::Cirrostratus:       SelectedCloudType = CloudsTypeOptions[3]; break;
	case ECloudType::CumulusCongestus:   SelectedCloudType = CloudsTypeOptions[4]; break;
	default:                             SelectedCloudType = CloudsTypeOptions[0]; break;
	}

	switch (Weather.Clouds.CloudLayer.Cover)
	{
	case ECloudCover::Clear:      SelectedCloudCover = CloudsCoverOptions[0]; break;
	case ECloudCover::Few:        SelectedCloudCover = CloudsCoverOptions[1]; break;
	case ECloudCover::Scattered:  SelectedCloudCover = CloudsCoverOptions[2]; break;
	case ECloudCover::Broken:     SelectedCloudCover = CloudsCoverOptions[3]; break;
	case ECloudCover::Overcast:   SelectedCloudCover = CloudsCoverOptions[4]; break;
	default:                      SelectedCloudCover = CloudsCoverOptions[0]; break;
	}

	switch (Weather.Clouds.Thunder)
	{
	case EThunderOption::None:      SelectedThunder = ThunderOptions[0]; break;
	case EThunderOption::Light:     SelectedThunder = ThunderOptions[1]; break;
	case EThunderOption::Moderate:  SelectedThunder = ThunderOptions[2]; break;
	case EThunderOption::Heavy:    SelectedThunder = ThunderOptions[3]; break;
	default:                       SelectedThunder = ThunderOptions[0]; break;
	}

	CloudRows.Reset();
	{
		FCloudRowPtr CloudRow = MakeShared<FCloudLayerRow>();
		CloudRow->Layer = 1;
		CloudRow->BaseFt = FMath::RoundToInt(Weather.Clouds.CloudLayer.BaseFt);
		CloudRow->TopFt = FMath::RoundToInt(Weather.Clouds.CloudLayer.TopFt);
		CloudRow->Type = SelectedCloudType.IsValid() ? *SelectedCloudType : TEXT("CompleteClearSkies");
		CloudRow->Cover = SelectedCloudCover.IsValid() ? *SelectedCloudCover : TEXT("Clear");
		CloudRows.Add(CloudRow);
	}

	WindRows.Reset();
	{
		FWindRowPtr WindRow = MakeShared<FWindLayerRow>();
		WindRow->Layer = 1;
		WindRow->AltFt = FMath::RoundToInt(Weather.Wind.CurrentWindLayer.AltitudeFt);
		WindRow->DirDeg = FMath::RoundToInt(Weather.Wind.CurrentWindLayer.DirectionDeg);
		WindRow->SpdKt = FMath::RoundToInt(Weather.Wind.CurrentWindLayer.SpeedKt);
		WindRows.Add(WindRow);
	}

	UpdateDownwindFromDir();
}

// -------------------------
// Table helpers
// -------------------------
TSharedRef<SWidget> SWeatherEffectsPanel::TableCell(
	TSharedRef<SWidget> Inner,
	bool bRightBorder,
	bool bBottomBorder) const
{
	const FMargin Inset(0.f, 0.f, bRightBorder ? 1.f : 0.f, bBottomBorder ? 1.f : 0.f);

	return SNew(SOverlay)

		+ SOverlay::Slot()
		[
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(TableGridLineColor)
				.Padding(0)
		]

		+ SOverlay::Slot()
		[
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(TableCellFillColor)
				.Padding(Inset)
				[
					SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(TableCellFillColor)
						.Padding(FMargin(6, 4))
						[
							Inner
						]
				]
		];
}

TSharedRef<SWidget> SWeatherEffectsPanel::MakeHeaderCell(const FString& Text, bool bRightBorder) const
{
	return TableCell(
		SNew(STextBlock)
		.Text(FText::FromString(Text))
		.ColorAndOpacity(FLinearColor::White)
		.Font(FontInfo8),
		bRightBorder,
		true);
}

TSharedRef<SWidget> SWeatherEffectsPanel::MakeEditableTextCell(
	const FString& Initial,
	TFunction<void(const FString&)> OnCommit,
	bool bRightBorder,
	bool bBottomBorder) const
{
	TSharedRef<SWidget> Inner =
		SNew(SEditableTextBox)
		.Style(&TableTextBoxStyle)
		.Text(FText::FromString(Initial))
		.SelectAllTextWhenFocused(true)
		.RevertTextOnEscape(true)
		.OnTextCommitted_Lambda([OnCommit](const FText& T, ETextCommit::Type)
			{
				OnCommit(T.ToString());
			});

	return TableCell(Inner, bRightBorder, bBottomBorder);
}

TSharedRef<SWidget> SWeatherEffectsPanel::MakeTableComboCell(
	TArray<TSharedPtr<FString>>* Source,
	const FString& CurrentValue,
	TFunction<void(const FString&)> OnPick,
	const FString& DefaultText,
	float MinWidth,
	bool bRightBorder,
	bool bBottomBorder)
{
	TSharedPtr<FString> LocalSelected;
	if (Source)
	{
		for (const TSharedPtr<FString>& Opt : *Source)
		{
			if (Opt.IsValid() && Opt->Equals(CurrentValue, ESearchCase::IgnoreCase))
			{
				LocalSelected = Opt;
				break;
			}
		}
	}

	auto GenItem = [](TSharedPtr<FString> Item)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
				.ColorAndOpacity(FLinearColor::White)
				.Font(FCoreStyle::GetDefaultFontStyle("Light", 9));
		};

	TSharedRef<SWidget> Combo =
		SNew(SComboBox<TSharedPtr<FString>>)
		.ComboBoxStyle(&TableComboStyle)
		.ContentPadding(FMargin(0))
		.OptionsSource(Source)
		.InitiallySelectedItem(LocalSelected)
		.OnGenerateWidget_Lambda(GenItem)
		.OnSelectionChanged_Lambda([OnPick](TSharedPtr<FString> Item, ESelectInfo::Type)
			{
				if (Item.IsValid()) OnPick(*Item);
			})
		[
			SNew(STextBlock)
				.Text_Lambda([CurrentValue, DefaultText]()
					{
						return FText::FromString(!CurrentValue.IsEmpty() ? CurrentValue : DefaultText);
					})
				.ColorAndOpacity(FLinearColor::White)
				.Font(FontInfo8)
		];

	return TableCell(SNew(SBox).MinDesiredWidth(MinWidth)[Combo], bRightBorder, bBottomBorder);
}

TSharedRef<SWidget> SWeatherEffectsPanel::WrapThemedTable(TSharedRef<SWidget> TableWidget)
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(TableGridLineColor)
		.Padding(1)
		[
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(TableCellFillColor)
				.Padding(0)
				[
					TableWidget
				]
		];
}

// -------------------------
// Subgroup: PRECIPITATION
// -------------------------

static FString MmPerHourToIntensityLabel(float MmPerHour)
{
	if (MmPerHour <= 0.0f)	return TEXT("None");
	if (MmPerHour < WeatherSim::LightMaxMmPerHour)	return TEXT("Light");
	if (MmPerHour < WeatherSim::ModerateMaxMmPerHour)	return TEXT("Moderate");
	if (MmPerHour < WeatherSim::HeavyMaxMmPerHour)	return TEXT("Heavy");
	return TEXT("Extreme");
}
static EPrecipitationIntensity MmPerHrToIntensityEnum(float MmPerHour)
{
	if (MmPerHour < WeatherSim::LightMaxMmPerHour)	return EPrecipitationIntensity::Light;
	if (MmPerHour < WeatherSim::ModerateMaxMmPerHour)	return EPrecipitationIntensity::Moderate;
	if (MmPerHour < WeatherSim::HeavyMaxMmPerHour)	return EPrecipitationIntensity::Heavy;
	return EPrecipitationIntensity::Extreme;
}

TSharedRef<SWidget> SWeatherEffectsPanel::BuildPrecipitation()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SGridPanel)
				.FillColumn(0, 1.f)
				.FillColumn(1, 1.f)

				+ SGridPanel::Slot(0, 0).Padding(0, 0, 10, 0)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Type")))
								.ColorAndOpacity(FLinearColor::White)
								.Font(FontInfo8)
						]
						+ SHorizontalBox::Slot().FillWidth(1.f)
						[

							MakeCombo(
								&PrecipTypeOptions,
								SelectedPrecipType,
								TEXT("None"),
								160.f,
								[this]()
								{
									if (bRefreshingFromEnvironment)
									{
										return;
									}
									ApplyPrecipitationToEnvironment();
									EnvController->OnAnyFeatureChanged();
									//PresetPanel->IsPresetDirty();
								},
								&PrecipTypeComboBox
							)
						]
				]

			+ SGridPanel::Slot(1, 0)
				[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Intensity mm/h")))
								.ColorAndOpacity(FLinearColor::White)
								.Font(FontInfo8)
						]
					
						+ SHorizontalBox::Slot().FillWidth(1.f)
						[
							SNew(SBox)
								.IsEnabled_Lambda([this]()
									{
										return SelectedPrecipType.IsValid() &&
											!SelectedPrecipType->Equals(TEXT("None"), ESearchCase::IgnoreCase);
									})
								[
									MakeCombo(
										&PrecipIntensityOptions,
										SelectedPrecipIntensity,
										TEXT("None"),
										160.f,
										[this]()
										{
											if (bRefreshingFromEnvironment)
											{
												return;
											}

											if (!SelectedPrecipType.IsValid() ||
												SelectedPrecipType->Equals(TEXT("None"), ESearchCase::IgnoreCase))
											{
												return;
											}

											UpdateSliderBasedOnIntensity();
											ApplyPrecipitationToEnvironment();
											EnvController->OnAnyFeatureChanged();
										},
										&PrecipIntensityComboBox
									)
								]
						]
				]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
			[
				SNew(SBox)
					.IsEnabled_Lambda([this]()
						{
							return SelectedPrecipType.IsValid() &&
								!SelectedPrecipType->Equals(TEXT("None"), ESearchCase::IgnoreCase);
						})
					[
						MakeSliderWithField(
							TEXT("mm/h"),
							0.f,
							100.f,
							PrecipField,
							60.f,
							[this]() { return State.IsValid() ? State->PrecipMmPerHr : 0.f; },
							[this](float NewValue)
							{
								if (bRefreshingFromEnvironment) return;
								if (!State.IsValid()) return;

								if (!SelectedPrecipType.IsValid() ||
									SelectedPrecipType->Equals(TEXT("None"), ESearchCase::IgnoreCase))
								{
									State->PrecipMmPerHr = 0.f;

									if (PrecipField.IsValid())
									{
										PrecipField->SetText(FText::AsNumber(0.f));
									}
									return;
								}

								State->PrecipMmPerHr = FMath::Clamp(NewValue, 0.0f, WeatherSim::ExtremeMaxMmPerHour);
								UpdateIntensityBasedOnSlider();
								ApplyPrecipitationToEnvironment();
								EnvController->OnAnyFeatureChanged();

							}
						)
					]
			];
}

void SWeatherEffectsPanel::UpdateIntensityBasedOnSlider()
{
	if (State.IsValid())
	{
		const FString TargetLabel = MmPerHourToIntensityLabel(State->PrecipMmPerHr);
		for (const TSharedPtr<FString>& Opt : PrecipIntensityOptions)
		{
			if (Opt.IsValid() && *Opt == TargetLabel)
			{
				SelectedPrecipIntensity = Opt;
				if (PrecipIntensityComboBox.IsValid())
				{
					PrecipIntensityComboBox->SetSelectedItem(SelectedPrecipIntensity);
				}

				return;
			}
		}
	}
}

void SWeatherEffectsPanel::UpdateSliderBasedOnIntensity()
{
	if (bUpdatingFromSlider) return;
	if (!State.IsValid()) return;

	if (!SelectedPrecipType.IsValid() ||
		SelectedPrecipType->Equals(TEXT("None"), ESearchCase::IgnoreCase))
	{
		State->PrecipMmPerHr = 0.f;

		if (PrecipField.IsValid())
		{
			PrecipField->SetText(FText::AsNumber(State->PrecipMmPerHr));
		}
		return;
	}

	bUpdatingFromDropdown = true;

	const FString Label = *SelectedPrecipIntensity;
	State->PrecipMmPerHr = WeatherSim::RepresentativeMmPerHourForLabel(Label);

	if (PrecipField.IsValid())
	{
		PrecipField->SetText(FText::AsNumber(State->PrecipMmPerHr));
	}

	bUpdatingFromDropdown = false;
}

void SWeatherEffectsPanel::ApplyPrecipitationToEnvironment()
{
	if (!EnvController.IsValid() || !State.IsValid())
		return;

	FEnvironmentData* Data = EnvController->GetEnvironmentData();
	if (!Data)
		return;

	auto* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
	if (!AOI)
		return;

	auto& Precip = AOI->CurrentWeatherEffect.Precipitation;
	auto& Clouds = AOI->CurrentWeatherEffect.Clouds;

	if (SelectedPrecipType.IsValid())
	{
		const FString& TypeStr = *SelectedPrecipType;

		if (TypeStr == TEXT("Rain"))       Precip.Type = EPrecipitationType::Rain;
		else if (TypeStr == TEXT("Snow"))  Precip.Type = EPrecipitationType::Snow;
		else if (TypeStr == TEXT("Hail"))  Precip.Type = EPrecipitationType::Hail;
		else                               Precip.Type = EPrecipitationType::None;
	}
	else
	{
		Precip.Type = EPrecipitationType::None;
	}

	if (Precip.Type == EPrecipitationType::None)
	{
		State->PrecipMmPerHr = 0.f;
		Precip.RateMmPerHr = 0.f;
		Precip.Intensity = EPrecipitationIntensity::None;

		Clouds.CloudLayer.Type = ECloudType::CompleteClearSkies;
		Clouds.CloudLayer.Cover = ECloudCover::Clear;
	}
	else
	{
		Precip.RateMmPerHr = FMath::Clamp(State->PrecipMmPerHr, 0.0f, WeatherSim::ExtremeMaxMmPerHour);

		if (Precip.RateMmPerHr <= 0.0f)
		{
			Precip.Intensity = EPrecipitationIntensity::None;
		}
		else
		{
			Precip.Intensity = MmPerHrToIntensityEnum(Precip.RateMmPerHr);
			UpdateIntensityBasedOnSlider();
		}

		if (Clouds.CloudLayer.Type == ECloudType::CompleteClearSkies)
		{
			Clouds.CloudLayer.Type = ECloudType::Cirrocumulus;
		}

		if (Precip.Intensity == EPrecipitationIntensity::Light)
		{
			Clouds.CloudLayer.Cover = ECloudCover::Few;
		}
		else if (Precip.Intensity == EPrecipitationIntensity::Moderate)
		{
			Clouds.CloudLayer.Cover = ECloudCover::Scattered;
		}
		else if (Precip.Intensity == EPrecipitationIntensity::Heavy)
		{
			Clouds.CloudLayer.Cover = ECloudCover::Broken;
		}
		else if (Precip.Intensity == EPrecipitationIntensity::Extreme)
		{
			Clouds.CloudLayer.Cover = ECloudCover::Overcast;
		}
		else
		{
			Clouds.CloudLayer.Cover = ECloudCover::Clear;
		}
	}

	Precip.bEnabled = bPrecipEnabled;

	EnvController->UpdatePrecipitationData(AOI);
	EnvController->UpdateCloudsData(AOI, Clouds.CloudLayer);

	RefreshCloudUiFromEnvironment();

	if (PrecipField.IsValid())
	{
		PrecipField->SetText(FText::AsNumber(State->PrecipMmPerHr));
	}
}
// -------------------------
// Subgroup: VISIBILITY
// -------------------------
TSharedRef<SWidget> SWeatherEffectsPanel::BuildVisibility()
{
	return SNew(SGridPanel)
		.FillColumn(0, 1.f)
		.FillColumn(1, 1.f)

		+ SGridPanel::Slot(0, 0).Padding(0, 0, 12, 0)
		[
			SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Obscurant")))
						.ColorAndOpacity(FLinearColor::White)
						.Font(FontInfo8)
				]

				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					MakeCombo(
						&VisibilityObscurantOptions,
						SelectedVisibilityObscurant,
						TEXT("Fog"),
						160.f,
						[this]()
						{
							if (bRefreshingFromEnvironment) return;
							if (!SelectedVisibilityObscurant.IsValid() || !EnvController.IsValid())
							{
								return;
							}

							FEnvironmentData* Data = EnvController->GetEnvironmentData();
							if (!Data)
							{
								return;
							}

							auto* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
							if (!AOI)
							{
								return;
							}

							auto& Visibility = AOI->CurrentWeatherEffect.Visibility;
							const FString& TypeStr = *SelectedVisibilityObscurant;

							if (TypeStr == TEXT("Dust"))
							{
								Visibility.Obscurant = EVisibilityObscurant::Dust;
							}
							else if (TypeStr == TEXT("Fog"))
							{
								Visibility.Obscurant = EVisibilityObscurant::Fog;
							}
							else if (TypeStr == TEXT("Haze"))
							{
								Visibility.Obscurant = EVisibilityObscurant::Haze;
							}
							else
							{
								Visibility.Obscurant = EVisibilityObscurant::None;

								// Reset slider/backend value to 0 when obscurant is None
								Visibility.DistanceKm = 0.f;

								if (State.IsValid())
								{
									State->VisibilityKm = 0.f;
								}

								if (VisibilityField.IsValid())
								{
									VisibilityField->SetText(FText::AsNumber(0.f));
								}
							}

							Visibility.bEnabled = bVisibilityEnabled;
							EnvController->UpdateVisibilityData(AOI);
							EnvController->OnAnyFeatureChanged();

						},
						&VisibilityComboBox
					)
				]
		]

		+ SGridPanel::Slot(1, 0).Padding(12, 0, 0, 0)
			[
				SNew(SBox)
					.IsEnabled_Lambda([this]()
						{
							return SelectedVisibilityObscurant.IsValid() &&
								!SelectedVisibilityObscurant->Equals(TEXT("None"), ESearchCase::IgnoreCase);
						})
					[
						MakeSliderWithField(
							TEXT("Distance km"),
							0.f,
							10.f,
							VisibilityField,
							80.f,
							[this]()
							{
								return State.IsValid() ? State->VisibilityKm : 0.f;
							},
							[this](float NewValue)
							{
								if (bRefreshingFromEnvironment) return;
								if (State.IsValid())
								{
									State->VisibilityKm = NewValue;
								}
								if (VisibilityField.IsValid())
								{
									VisibilityField->SetText(FText::AsNumber(NewValue));
								}
								if (EnvController.IsValid())
								{
									FEnvironmentData* Data = EnvController->GetEnvironmentData();
									if (Data)
									{
										auto* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
										if (!AOI)
										{
											return;
										}

										auto& Visibility = AOI->CurrentWeatherEffect.Visibility;
										Visibility.DistanceKm = NewValue;
										Visibility.bEnabled = bVisibilityEnabled;

										EnvController->UpdateVisibilityData(AOI);
										EnvController->OnAnyFeatureChanged();

									}
								}
							}
						)
					]
			];
}
// -------------------------
// Subgroup: CLOUDS
// -------------------------
TSharedRef<SWidget> SWeatherEffectsPanel::BuildClouds()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
						MakeCombo(
							&CloudsLayersOptions,
							SelectedCloudLayer,
							TEXT("Layer 1"),
							200.f,
							nullptr,
							&CloudLayerComboBox
						)
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0, 0, 0)
				[
					SNew(SButton).ButtonStyle(FAppStyle::Get(), "HoverHintOnly").ContentPadding(4)
						[
						//	MakeDropDownIcons(TEXT("compass"), 9)
              SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Plus"))
						]
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
				[
					SNew(SButton).ButtonStyle(FAppStyle::Get(), "HoverHintOnly").ContentPadding(4)
						[
							//MakeDropDownIcons(TEXT("compass"), 9)
              SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Edit"))
						]
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
				[
					SNew(SButton).ButtonStyle(FAppStyle::Get(), "HoverHintOnly").ContentPadding(4)
						[
							//MakeDropDownIcons(TEXT("compass"), 9)
							SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Delete"))
						]
				]
		]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[
			SNew(SGridPanel)
				.FillColumn(0, 1.f)
				.FillColumn(1, 1.f)

				+ SGridPanel::Slot(0, 0).Padding(0, 0, 10, 0)
				[
					MakeSliderWithField(TEXT("Top Elevation m"), 0.f, 60000.f, CloudsTopField, 100.f, [this]() { return State.IsValid() ? State->CloudsTopFt : 0.f; },   
						[this](float NewValue)
						{
							if (bRefreshingFromEnvironment)
							{
								return;
							}
							if (State.IsValid())
								State->CloudsTopFt = NewValue;

							if (EnvController.IsValid())
							{
								FEnvironmentData* Data = EnvController->GetEnvironmentData();
								if (Data)
								{
									FCloudsData& Clouds = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest()->CurrentWeatherEffect.Clouds;
									if (Clouds.HasSelectedCloudLayer())
									{
										FCloudLayerData Layer = *Clouds.GetSelectedCloudLayer();
										Layer.TopFt = NewValue;
										Clouds.UpdateCloudLayer(Clouds.SelectedCloudLayerIndex, Layer);
									}
								}
							}
						}
					)
				]

			+ SGridPanel::Slot(1, 0)
				[
					MakeSliderWithField(TEXT("Base Elevation m"), 0.f, 60000.f, CloudsBaseField, 100.f, [this]() { return State.IsValid() ? State->CloudsBaseFt : 0.f; },
						[this](float NewValue)
						{
							if (bRefreshingFromEnvironment)
							{
								return;
							}
							if (State.IsValid())
								State->CloudsBaseFt = NewValue;

							if (EnvController.IsValid())
							{
								FEnvironmentData* Data = EnvController->GetEnvironmentData();
								if (Data)
								{
									FCloudsData& Clouds = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest()->CurrentWeatherEffect.Clouds;
									if (Clouds.HasSelectedCloudLayer())
									{
										FCloudLayerData Layer = *Clouds.GetSelectedCloudLayer();
										Layer.BaseFt = NewValue;        
										Clouds.UpdateCloudLayer(Clouds.SelectedCloudLayerIndex, Layer);
									}
								}
							}
						}
					)
		]]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[
			SNew(SGridPanel)
				.FillColumn(0, 1.f)
				.FillColumn(1, 1.f)

				+ SGridPanel::Slot(0, 0).Padding(0, 0, 10, 0)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Category")))
								.ColorAndOpacity(FLinearColor::White)
								.Font(FontInfo8)
						]

						+ SHorizontalBox::Slot().FillWidth(1.f)
						[
							MakeCombo(&CloudsTypeOptions, SelectedCloudType, TEXT("CompleteClearSkies"), 160.f, [this]()
								{
									if (bRefreshingFromEnvironment)
									{
										return;
									}
									if (SelectedCloudType.IsValid() && EnvController.IsValid())
									{
										const FString& TypeStr1 = *SelectedCloudType;

										FEnvironmentData* Data = EnvController->GetEnvironmentData();
										if (Data)
										{
											auto* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
											if (!AOI)
											{
												return;
											}

											auto& Clouds = AOI->CurrentWeatherEffect.Clouds;
											auto& CloudsType = Clouds.CloudLayer;

											if (TypeStr1 == TEXT("Cirrocumulus"))             CloudsType.Type = ECloudType::Cirrocumulus;
											else if (TypeStr1 == TEXT("Cirrostratus"))        CloudsType.Type = ECloudType::Cirrostratus;
											else if (TypeStr1 == TEXT("CumulusCongestus"))    CloudsType.Type = ECloudType::CumulusCongestus;
											else if (TypeStr1 == TEXT("Partly Cloudy"))       CloudsType.Type = ECloudType::PartlyCloudy;
											else if (TypeStr1 == TEXT("CompleteClearSkies"))  CloudsType.Type = ECloudType::CompleteClearSkies;

											// If clear skies selected, force Oktas/Cover to Clear
											if (TypeStr1 == TEXT("CompleteClearSkies"))
											{
												Clouds.CloudLayer.Cover = ECloudCover::Clear;

												for (const TSharedPtr<FString>& Opt : CloudsCoverOptions)
												{
													if (Opt.IsValid() && Opt->Equals(TEXT("Clear"), ESearchCase::IgnoreCase))
													{
														SelectedCloudCover = Opt;
														break;
													}
												}

												if (CloudCoverComboBox.IsValid())
												{
													CloudCoverComboBox->SetSelectedItem(SelectedCloudCover);
												}
											}

											EnvController->UpdateCloudsData(AOI, Clouds.CloudLayer);
											EnvController->OnAnyFeatureChanged();

										}
									}
								}
								, &CloudTypeComboBox)
						]
				]

			+ SGridPanel::Slot(1, 0)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Oktas     ")))
								.ColorAndOpacity(FLinearColor::White)
								.Font(FontInfo8)
						]

						+ SHorizontalBox::Slot().FillWidth(1.f)
						[
							MakeCombo(&CloudsCoverOptions, SelectedCloudCover, TEXT("Few"), 160.f, [this]()
								{
									if (bRefreshingFromEnvironment)
									{
										return;
									}
									if (SelectedCloudCover.IsValid() && EnvController.IsValid())
									{
										const FString& TypeStr1 = *SelectedCloudCover;
										if (EnvController.IsValid())
										{
											FEnvironmentData* Data = EnvController->GetEnvironmentData();
											if (Data)
											{
												auto& CloudsCover = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest()->CurrentWeatherEffect.Clouds.CloudLayer;
												if (SelectedCloudCover.IsValid())
												{
													if (TypeStr1 == TEXT("Clear"))      CloudsCover.Cover = ECloudCover::Clear;
													else if (TypeStr1 == TEXT("Few")) CloudsCover.Cover = ECloudCover::Few;
													else if (TypeStr1 == TEXT("Scattered")) CloudsCover.Cover = ECloudCover::Scattered;
													else if (TypeStr1 == TEXT("Broken")) CloudsCover.Cover = ECloudCover::Broken;
													else if (TypeStr1 == TEXT("Overcast")) CloudsCover.Cover = ECloudCover::Overcast;
												}
												EnvController->UpdateCloudsData(Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest(), CloudsCover);
												EnvController->OnAnyFeatureChanged();

											}
										}
									}
								}, &CloudCoverComboBox)
						]
				]

		]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[
			SNew(SGridPanel)
				.FillColumn(0, 1.f)
				.FillColumn(1, 1.f)

				+ SGridPanel::Slot(0, 0).Padding(0, 0, 10, 0)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
							SNew(SCheckBox)
								.Style(Styles.CheckStyle ? Styles.CheckStyle : &FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("Checkbox"))
								.IsChecked_Lambda([this]() { return State->bCloudShadows ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
								.OnCheckStateChanged_Lambda([this](ECheckBoxState S) { State->bCloudShadows = (S == ECheckBoxState::Checked); })
						]

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0, 0, 0)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Shadows")))
								.ColorAndOpacity(FLinearColor::White)
								.Font(FontInfo8)
						]
				]

			+ SGridPanel::Slot(1, 0)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Thunder  ")))
								.ColorAndOpacity(FLinearColor::White)
								.Font(FontInfo8)
						]

						+ SHorizontalBox::Slot().FillWidth(1.f)
						[
							MakeCombo(&ThunderOptions, SelectedThunder, TEXT("None"), 160.f, [this]()
								{
									if (bRefreshingFromEnvironment) return;
									if (!SelectedThunder.IsValid() || !EnvController.IsValid())
									{
										return;
									}

									FEnvironmentData* Data = EnvController->GetEnvironmentData();
									if (!Data)
									{
										return;
									}

									auto* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
									if (!AOI)
									{
										return;
									}

									auto& Clouds = AOI->CurrentWeatherEffect.Clouds;
									const FString& TypeStr = *SelectedThunder;

									if (TypeStr == TEXT("Light"))
									{
										Clouds.Thunder = EThunderOption::Light;
									}
									else if (TypeStr == TEXT("Moderate"))
									{
										Clouds.Thunder = EThunderOption::Moderate;
									}
									else if (TypeStr == TEXT("Heavy"))
									{
										Clouds.Thunder = EThunderOption::Heavy;
									}
									else
									{
										Clouds.Thunder = EThunderOption::None;
									}

									Clouds.bEnabled = bCloudsEnabled;
									EnvController->UpdateThunderData(AOI);
									EnvController->OnAnyFeatureChanged();

								}, &ThunderComboBox)
						]
				]
		]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[
			SNew(SExpandableArea)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Theme.PanelFill)
				.HeaderPadding(FMargin(8, 6))
				.HeaderContent()
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Layers Data")))
						.ColorAndOpacity(FLinearColor::White)
						.Font(FontInfo8)
				]
				.BodyContent()
				[
					SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(TableCellFillColor)
						.Padding(FMargin(8))
						[
							SNew(SVerticalBox)

								+ SVerticalBox::Slot().AutoHeight()
								[
									SNew(SGridPanel)
										.FillColumn(0, 0.10f)
										.FillColumn(1, 0.22f)
										.FillColumn(2, 0.22f)
										.FillColumn(3, 0.23f)
										.FillColumn(4, 0.23f)

										+ SGridPanel::Slot(0, 0)[MakeHeaderCell(TEXT("Layers"), true)]
										+ SGridPanel::Slot(1, 0)[MakeHeaderCell(TEXT("Top (ft)"), true)]
										+ SGridPanel::Slot(2, 0)[MakeHeaderCell(TEXT("Base (ft)"), true)]
										+ SGridPanel::Slot(3, 0)[MakeHeaderCell(TEXT("Cloud Type"), true)]
										+ SGridPanel::Slot(4, 0)[MakeHeaderCell(TEXT("Cloud Cover"), false)]
								]

							+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
								[
									WrapThemedTable(
										SNew(SBox)
										.MinDesiredHeight(95.f)
										[
											SAssignNew(CloudListView, SListView<FCloudRowPtr>)
												.ListViewStyle(&FlatTableViewStyle)
												.ListItemsSource(&CloudRows)
												.SelectionMode(ESelectionMode::None)
												.OnGenerateRow_Lambda([this](FCloudRowPtr Item, const TSharedRef<STableViewBase>& Owner)
													{
														return SNew(STableRow<FCloudRowPtr>, Owner)
															.Style(&FlatTableRowStyle)
															[
																SNew(SGridPanel)
																	.FillColumn(0, 0.10f)
																	.FillColumn(1, 0.22f)
																	.FillColumn(2, 0.22f)
																	.FillColumn(3, 0.23f)
																	.FillColumn(4, 0.23f)

																	+ SGridPanel::Slot(0, 0)
																	[
																		MakeEditableTextCell(FString::FromInt(Item->Layer),
																			[Item](const FString& V) { Item->Layer = FCString::Atoi(*V); },
																			true, true)
																	]
																	+ SGridPanel::Slot(1, 0)
																	[
																		MakeEditableTextCell(FString::FromInt(Item->BaseFt),
																			[Item](const FString& V) { Item->BaseFt = FCString::Atoi(*V); },
																			true, true)
																	]
																	+ SGridPanel::Slot(2, 0)
																	[
																		MakeEditableTextCell(FString::FromInt(Item->TopFt),
																			[Item](const FString& V) { Item->TopFt = FCString::Atoi(*V); },
																			true, true)
																	]
																	+ SGridPanel::Slot(3, 0)
																	[
																		MakeTableComboCell(&CloudsTypeOptions, Item->Type,
																			[Item](const FString& Pick) { Item->Type = Pick; },
																			TEXT("Cirrus"), 140.f, true, true)
																	]
																	+ SGridPanel::Slot(4, 0)
																	[
																		MakeTableComboCell(&CloudsCoverOptions, Item->Cover,
																			[Item](const FString& Pick) { Item->Cover = Pick; },
																			TEXT("Clear"), 140.f, false, true)
																	]
															];
													})
										]
									)
								]
						]
				]
		];
}

void SWeatherEffectsPanel::RefreshCloudUiFromEnvironment()
{
	if (!EnvController.IsValid() || !State.IsValid())
	{
		return;
	}

	FEnvironmentData* Data = EnvController->GetEnvironmentData();
	if (!Data)
	{
		return;
	}

	auto* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
	if (!AOI)
	{
		return;
	}

	bRefreshingFromEnvironment = true;

	const FCloudLayerData& CloudLayer = AOI->CurrentWeatherEffect.Clouds.CloudLayer;

	auto FindOption = [](const TArray<TSharedPtr<FString>>& Options, const FString& Value) -> TSharedPtr<FString>
		{
			for (const TSharedPtr<FString>& Opt : Options)
			{
				if (Opt.IsValid() && Opt->Equals(Value, ESearchCase::IgnoreCase))
				{
					return Opt;
				}
			}
			return nullptr;
		};

	FString CloudTypeLabel;
	switch (CloudLayer.Type)
	{
	case ECloudType::Cirrocumulus:      CloudTypeLabel = TEXT("Cirrocumulus"); break;
	case ECloudType::Cirrostratus:      CloudTypeLabel = TEXT("Cirrostratus"); break;
	case ECloudType::CumulusCongestus:  CloudTypeLabel = TEXT("CumulusCongestus"); break;
	case ECloudType::PartlyCloudy:      CloudTypeLabel = TEXT("Partly Cloudy"); break;
	case ECloudType::CompleteClearSkies:CloudTypeLabel = TEXT("CompleteClearSkies"); break;
	default:                            CloudTypeLabel = TEXT("CompleteClearSkies"); break;
	}

	FString CloudCoverLabel;
	switch (CloudLayer.Cover)
	{
	case ECloudCover::Clear:      CloudCoverLabel = TEXT("Clear"); break;
	case ECloudCover::Few:        CloudCoverLabel = TEXT("Few"); break;
	case ECloudCover::Scattered:  CloudCoverLabel = TEXT("Scattered"); break;
	case ECloudCover::Broken:     CloudCoverLabel = TEXT("Broken"); break;
	case ECloudCover::Overcast:   CloudCoverLabel = TEXT("Overcast"); break;
	default:                      CloudCoverLabel = TEXT("Clear"); break;
	}

	SelectedCloudType = FindOption(CloudsTypeOptions, CloudTypeLabel);
	SelectedCloudCover = FindOption(CloudsCoverOptions, CloudCoverLabel);

	if (CloudTypeComboBox.IsValid())
	{
		CloudTypeComboBox->SetSelectedItem(SelectedCloudType);
	}

	if (CloudCoverComboBox.IsValid())
	{
		CloudCoverComboBox->SetSelectedItem(SelectedCloudCover);
	}

	if (CloudRows.Num() > 0 && CloudRows[0].IsValid())
	{
		CloudRows[0]->Type = CloudTypeLabel;
		CloudRows[0]->Cover = CloudCoverLabel;
		CloudRows[0]->BaseFt = CloudLayer.BaseFt;
		CloudRows[0]->TopFt = CloudLayer.TopFt;
	}

	if (CloudListView.IsValid())
	{
		CloudListView->RequestListRefresh();
	}

	bRefreshingFromEnvironment = false;
}
// -------------------------
// Subgroup: WIND
// -------------------------
TSharedRef<SWidget> SWeatherEffectsPanel::BuildWind()
{
	auto MakeOutlinedReadOnly = [&](TSharedPtr<SEditableTextBox>& FieldRef)
		{
			return SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Theme.BgBase)
				.Padding(1)
				[
					SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(Theme.BgBase)
						.Padding(FMargin(4, 2))
						[
							SAssignNew(FieldRef, SEditableTextBox)
								.Style(&FlatTextBoxStyle)
								.IsReadOnly(true)
								.Text_Lambda([this]()
									{
										return FText::AsNumber(FMath::RoundToInt(State->WindDirDeg));
									})
						]
				];
		};

	TSharedRef<SWidget> Compass =
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(Theme.BgBase)
		.Padding(4)
		[
			SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.Compass"))
				//MakeDropDownIcons(TEXT("compass"), 9)
				.ColorAndOpacity(FLinearColor::White)
		];

	TSharedRef<SWidget> Body =
		SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().FillWidth(1.f)
				[
					MakeCombo(&WindLayersOptions, SelectedWindLayer, TEXT("Surface - 0ft"), 240.f, nullptr, &WindLayerComboBox)
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(8, 0, 0, 0)
				[
					SNew(SButton).ButtonStyle(FAppStyle::Get(), "HoverHintOnly").ContentPadding(4)
						[
							//MakeDropDownIcons(TEXT("compass"), 9)
              SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Plus"))
						]
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
				[
					SNew(SButton).ButtonStyle(FAppStyle::Get(), "HoverHintOnly").ContentPadding(4)
						[
							//MakeDropDownIcons(TEXT("compass"), 9)
              SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Edit"))
						]
				]

				+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
				[
					SNew(SButton).ButtonStyle(FAppStyle::Get(), "HoverHintOnly").ContentPadding(4)
						[
							//MakeDropDownIcons(TEXT("compass"), 9)
	            SNew(SImage).Image(FAppStyle::Get().GetBrush("Icons.Delete"))
						]
				]
		]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[
			SNew(SGridPanel)
				.FillColumn(0, 1.f)
				.FillColumn(1, 1.f)

				+ SGridPanel::Slot(0, 0).Padding(0, 0, 10, 0)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(10, 0, 30, 0)
						[
							MakeIconLabel(
								TEXT("Direction °"),
								TEXT("compass"),
								&FontInfo8,
								FLinearColor::White
							)
						]

						+ SHorizontalBox::Slot().FillWidth(1.f)
						[
							SNew(SBorder)
								.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
								.BorderBackgroundColor(Theme.BgBase)
								.Padding(1)
								[
									SNew(SBorder)
										.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
										.BorderBackgroundColor(Theme.BgBase)
										.Padding(FMargin(4, 2))
										[
											SAssignNew(WindDirField, SEditableTextBox)
												.Style(&FlatTextBoxStyle)
												.Text_Lambda([this]() { return FText::AsNumber(FMath::RoundToInt(State->WindDirDeg)); })
												.OnTextCommitted_Lambda([this](const FText& T, ETextCommit::Type)
													{
														if (bRefreshingFromEnvironment)
														{
															return;
														}
														float Parsed = 0.f;
														TryParseFloat(T.ToString(), Parsed);

														if (State.IsValid())
															State->WindDirDeg = Parsed;

														if (EnvController.IsValid())
														{
															FEnvironmentData* Data = EnvController->GetEnvironmentData();
															if (Data)
															{
																FWindData& Wind = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest()->CurrentWeatherEffect.Wind;
																Wind.CurrentWindLayer.DirectionDeg = Parsed;
																Wind.bEnabled = bWindEnabled;
																EnvController->UpdateWindData(Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest(), Wind.CurrentWindLayer);
																EnvController->OnAnyFeatureChanged();

															}
														}
													})
										]
								]
						]
				]

			+ SGridPanel::Slot(1, 0)
				[
					MakeSliderWithField(
						TEXT("Speed km/h"),
						0.f,
						407.f,
						WindSpeedField,
						70.f,
						/*[this]() { return State.IsValid() ? State->WindSpeedKt : 0.f; },  
						[this](float NewValue)
						{
							if (bRefreshingFromEnvironment)
							{
								return;
							}
							if (State.IsValid())
								State->WindSpeedKt = NewValue;

							if (EnvController.IsValid())
							{
								FEnvironmentData* Data = EnvController->GetEnvironmentData();
								if (Data)
								{
									FWindData& Wind = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest()->CurrentWeatherEffect.Wind;
									Wind.CurrentWindLayer.SpeedKt = NewValue;
									EnvController->UpdateWindData(Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest(), Wind.CurrentWindLayer);
									EnvController->OnAnyFeatureChanged();

								}
							}
						}*/
						[this]()
						{
							if (!EnvController.IsValid()) return 0.f;
							FEnvironmentData* Data = EnvController->GetEnvironmentData();
							if (!Data) return 0.f;
							FAreaOfInterest* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
							if (!AOI) return 0.f;
							return AOI->CurrentWeatherEffect.Wind.CurrentWindLayer.SpeedKt;
						},
						[this](float NewValue)
						{
							if (!EnvController.IsValid()) return;
							FEnvironmentData* Data = EnvController->GetEnvironmentData();
							if (!Data) return;
							FAreaOfInterest* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
							if (!AOI) return;
							AOI->CurrentWeatherEffect.Wind.CurrentWindLayer.SpeedKt = NewValue;
							EnvController->UpdateWindData(AOI, AOI->CurrentWeatherEffect.Wind.CurrentWindLayer);
							EnvController->OnAnyFeatureChanged();
						}
					)
				]
		]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[
			SNew(SGridPanel)
				.FillColumn(0, 1.f)
				.FillColumn(1, 1.f)

				+ SGridPanel::Slot(0, 0).Padding(0, 0, 10, 0)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("Downwind Direction°")))
								.ColorAndOpacity(FLinearColor::White)
								.Font(FontInfo8)
						]

						+ SHorizontalBox::Slot().FillWidth(1.f)
						[
							MakeOutlinedReadOnly(DownwindField)
						]
				]
		]

	+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[
			SNew(SExpandableArea)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(Theme.PanelFill)
				.HeaderPadding(FMargin(8, 6))
				.HeaderContent()
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Layers Data")))
						.ColorAndOpacity(FLinearColor::White)
						.Font(FontInfo8)
				]
				.BodyContent()
				[
					SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
						.BorderBackgroundColor(TableCellFillColor)
						.Padding(FMargin(6))
						[
							SNew(SVerticalBox)

								+ SVerticalBox::Slot().AutoHeight()
								[
									SNew(SGridPanel)
										.FillColumn(0, 0.25f)
										.FillColumn(1, 0.25f)
										.FillColumn(2, 0.25f)
										.FillColumn(3, 0.25f)

										+ SGridPanel::Slot(0, 0)[MakeHeaderCell(TEXT("Layers"), true)]
										+ SGridPanel::Slot(1, 0)[MakeHeaderCell(TEXT("Altitude (ft)"), true)]
										+ SGridPanel::Slot(2, 0)[MakeHeaderCell(TEXT("Direction (°)"), true)]
										+ SGridPanel::Slot(3, 0)[MakeHeaderCell(TEXT("Speed (kt)"), false)]
								]

							+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
								[
									WrapThemedTable(
										SNew(SBox)
										.MinDesiredHeight(85.f)
										[
											SAssignNew(WindListView, SListView<FWindRowPtr>)
												.ListViewStyle(&FlatTableViewStyle)
												.ListItemsSource(&WindRows)
												.SelectionMode(ESelectionMode::None)
												.OnGenerateRow_Lambda([this](FWindRowPtr Item, const TSharedRef<STableViewBase>& Owner)
													{
														return SNew(STableRow<FWindRowPtr>, Owner)
															.Style(&FlatTableRowStyle)
															[
																SNew(SGridPanel)
																	.FillColumn(0, 0.25f)
																	.FillColumn(1, 0.25f)
																	.FillColumn(2, 0.25f)
																	.FillColumn(3, 0.25f)

																	+ SGridPanel::Slot(0, 0)
																	[
																		MakeEditableTextCell(FString::FromInt(Item->Layer),
																			[Item](const FString& V) { Item->Layer = FCString::Atoi(*V); },
																			true, true)
																	]
																	+ SGridPanel::Slot(1, 0)
																	[
																		MakeEditableTextCell(FString::FromInt(Item->AltFt),
																			[Item](const FString& V) { Item->AltFt = FCString::Atoi(*V); },
																			true, true)
																	]
																	+ SGridPanel::Slot(2, 0)
																	[
																		MakeEditableTextCell(FString::FromInt(Item->DirDeg),
																			[Item](const FString& V) { Item->DirDeg = FCString::Atoi(*V); },
																			true, true)
																	]
																	+ SGridPanel::Slot(3, 0)
																	[
																		MakeEditableTextCell(FString::FromInt(Item->SpdKt),
																			[Item](const FString& V) { Item->SpdKt = FCString::Atoi(*V); },
																			false, true)
																	]
															];
													})
										]
									)
								]
						]
				]
		];

	return Body;
}

void SWeatherEffectsPanel::RefreshFromEnvironmentData()
{
	bRefreshingFromEnvironment = true;

	SyncFromEnvironmentData();

	if (PrecipTypeComboBox.IsValid())
	{
		PrecipTypeComboBox->SetSelectedItem(SelectedPrecipType);
	}

	if (PrecipIntensityComboBox.IsValid())
	{
		PrecipIntensityComboBox->SetSelectedItem(SelectedPrecipIntensity);
	}

	if (VisibilityComboBox.IsValid())
	{
		VisibilityComboBox->SetSelectedItem(SelectedVisibilityObscurant);
	}

	if (CloudLayerComboBox.IsValid())
	{
		CloudLayerComboBox->SetSelectedItem(SelectedCloudLayer);
	}

	if (CloudTypeComboBox.IsValid())
	{
		CloudTypeComboBox->SetSelectedItem(SelectedCloudType);
	}

	if (CloudCoverComboBox.IsValid())
	{
		CloudCoverComboBox->SetSelectedItem(SelectedCloudCover);
	}

	if (ThunderComboBox.IsValid())
	{
		ThunderComboBox->SetSelectedItem(SelectedThunder);
	}

	if (WindLayerComboBox.IsValid())
	{
		WindLayerComboBox->SetSelectedItem(SelectedWindLayer);
	}

	if (PrecipField.IsValid())
	{
		PrecipField->SetText(FText::AsNumber(State.IsValid() ? State->PrecipMmPerHr : 0.f));
	}

	if (VisibilityField.IsValid())
	{
		VisibilityField->SetText(FText::AsNumber(State.IsValid() ? State->VisibilityKm : 0.f));
	}

	if (CloudsTopField.IsValid())
	{
		CloudsTopField->SetText(FText::AsNumber(State.IsValid() ? State->CloudsTopFt : 0.f));
	}

	if (CloudsBaseField.IsValid())
	{
		CloudsBaseField->SetText(FText::AsNumber(State.IsValid() ? State->CloudsBaseFt : 0.f));
	}

	if (WindDirField.IsValid())
	{
		WindDirField->SetText(FText::AsNumber(State.IsValid() ? FMath::RoundToInt(State->WindDirDeg) : 0));
	}

	/*if (WindSpeedField.IsValid())
	{
		WindSpeedField->SetText(FText::AsNumber(State.IsValid() ? State->WindSpeedKt : 0.f));
	}*/

	UpdateDownwindFromDir();

	if (CloudListView.IsValid())
	{
		CloudListView->RequestListRefresh();
	}

	if (WindListView.IsValid())
	{
		WindListView->RequestListRefresh();
	}

	bRefreshingFromEnvironment = false;
}