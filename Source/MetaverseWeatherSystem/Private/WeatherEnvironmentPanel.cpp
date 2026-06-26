#include "WeatherEnvironmentPanel.h"

#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

SWeatherEnvironmentPanel::SWeatherEnvironmentPanel()
	: FontAsset(LoadObject<UObject>(nullptr, TEXT("/Game/Fonts/segoeui_Font.segoeui_Font")))
	, FontInfo12(FontAsset, 12, FName("Default"))
	, FontInfo10(FontAsset, 10, FName("Default"))
	, FontInfo8(FontAsset, 9, FName("Default"))
{
}

TSharedPtr<FSlateBrush> SWeatherEnvironmentPanel::GetIconBrush(FString IconName)
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
	BrushCache.Add(IconName, IconSlateBrush);
	return IconSlateBrush;
}

TSharedRef<SWidget> SWeatherEnvironmentPanel::MakeDropDownIcons(
	const FString& IconPath,
	float IconSize = 16.0f
)
{
	TSharedPtr<FSlateBrush> IconBrush = GetIconBrush(IconPath);
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

void SWeatherEnvironmentPanel::Construct(const FArguments& InArgs)
{
	Theme = InArgs._Theme;
	Styles = InArgs._Styles;
	EnvController = InArgs._EnvironmentController;
	OnPresetAppliedOrReset = InArgs._OnPresetAppliedOrReset;
	if (EnvController.IsValid())
	{
		EnvController.Get()->OnPresetChanged.BindLambda([this](bool bPresetDirty) {
			IsPresetDirty(bPresetDirty);
			});
	}
	FString InitialPresetName = TEXT("None");
	if (EnvController.IsValid())
	{
		const FEnvironmentData* Data = EnvController->GetEnvironmentData();
		if (Data)
		{
			const FAreaOfInterest* Area = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
			if (Area)
			{
				const FString& AppliedName = Area->CurrentEnvironmentPreset.PresetId.Name;
				if (!AppliedName.IsEmpty())
				{
					InitialPresetName = AppliedName;
				}
			}
		}
	}
	RefreshPresetOptions(InitialPresetName);

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	auto GenerateItem = [](FStringItem Item)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
				.ColorAndOpacity(FLinearColor::White);
		};

	auto ThemedFieldShell = [&](TSharedRef<SWidget> Inner) -> TSharedRef<SWidget>
		{
			return SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(Theme.BgBase)
				.Padding(1)
				[
					SNew(SBorder)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(Theme.BgBase)
						.Padding(FMargin(6, 2))
						[
							Inner
						]
				];
		};

	auto IconBtn = [&](const FSlateBrush* Icon, FReply(SWeatherEnvironmentPanel::* Fn)())
		{
			return SNew(SButton)
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.ContentPadding(4)
				.OnClicked(this, Fn)
				[
					SNew(SImage).Image(Icon)
				];
		};

	TSharedRef<SWidget> NormalRow =
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().FillWidth(1.f)
		[
			ThemedFieldShell(
				SAssignNew(PresetCombo, SComboBox<FStringItem>)
				.ComboBoxStyle(Styles.ComboStyle ? Styles.ComboStyle : &FAppStyle::Get().GetWidgetStyle<FComboBoxStyle>("ComboBox"))
				.OptionsSource(&PresetOptions)
				.OnGenerateWidget_Lambda(GenerateItem)
				.OnSelectionChanged(this, &SWeatherEnvironmentPanel::OnPresetChanged)
				.InitiallySelectedItem(SelectedPreset)
				[
					/*SNew(STextBlock)
						.Text(this, &SWeatherEnvironmentPanel::GetSelectedPresetText)*/
					SAssignNew(PresetTextBlock, STextBlock)
						.Text(this, &SWeatherEnvironmentPanel::GetSelectedPresetText)
						.ColorAndOpacity(FLinearColor::White)
				]
			)
		]

	+ SHorizontalBox::Slot().AutoWidth().Padding(6, 0, 0, 0)
		[
			//	MakeDropDownIcons(TEXT("compass"), 9)
			IconBtn(FAppStyle::Get().GetBrush("Icons.Plus"), &SWeatherEnvironmentPanel::OnPlusClicked)
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
		[
			//	MakeDropDownIcons(TEXT("compass"), 9)
			IconBtn(FAppStyle::Get().GetBrush("Icons.Edit"), &SWeatherEnvironmentPanel::OnEditClicked)
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
		[
			//MakeDropDownIcons(TEXT("compass"), 9)
			IconBtn(FAppStyle::Get().GetBrush("Icons.Delete"), &SWeatherEnvironmentPanel::OnDeleteClicked)
		];

	TSharedRef<SWidget> EditRow =
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().FillWidth(1.f)
		[
			ThemedFieldShell(
				SAssignNew(NameField, SEditableTextBox)
				.Style(Styles.TextBoxStyle ? Styles.TextBoxStyle : &FAppStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
				.HintText(FText::FromString(TEXT("Enter preset name")))
			)
		]

	+ SHorizontalBox::Slot().AutoWidth().Padding(6, 0, 0, 0)
		[
			SNew(SButton)
				.ButtonStyle(Styles.ButtonStyle ? Styles.ButtonStyle : &FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
				.OnClicked(this, &SWeatherEnvironmentPanel::OnConfirmClicked)
				[
					SNew(STextBlock)
						.Text(this, &SWeatherEnvironmentPanel::GetConfirmText)
						.ColorAndOpacity(FLinearColor::White)
				]
		]

	+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
		[
			SNew(SButton)
				.ButtonStyle(Styles.ButtonStyle ? Styles.ButtonStyle : &FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
				.OnClicked(this, &SWeatherEnvironmentPanel::OnCancelClicked)
				[
					SNew(STextBlock)
						.Text(FText::FromString(TEXT("Cancel")))
						.ColorAndOpacity(FLinearColor::White)
				]
		];


	ChildSlot
		[
			SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(Theme.HeaderFill)
				.Padding(2)
				[
					SNew(SBorder)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(Theme.HeaderFill)
						.Padding(FMargin(10, 10))
						[
							SNew(SVerticalBox)

								+ SVerticalBox::Slot().AutoHeight()
								[
									MakeHeader(
										TEXT("ENVIRONMENT"),
										TEXT("Environment")
									)
								]

								+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 8)
								[
									MakeSeparator(1.0f)
								]

								+ SVerticalBox::Slot().AutoHeight()
								[
									SAssignNew(Switcher, SWidgetSwitcher)
										+ SWidgetSwitcher::Slot()[NormalRow]
										+ SWidgetSwitcher::Slot()[EditRow]
								]
						]
				]
		];

	Switcher->SetActiveWidgetIndex(0);
}

void SWeatherEnvironmentPanel::RefreshPresetOptions(const FString& SelectName)
{
	PresetOptions.Reset();
	PresetOptions.Add(MakeShared<FString>(TEXT("None")));

	if (EnvController.IsValid())
	{
		for (const FScenarioEnvironmentPreset& P : EnvController->GetEnvironmentPresets())
		{
			PresetOptions.Add(MakeShared<FString>(P.PresetId.Name));
		}
	}

	SelectedPreset.Reset();
	for (const FStringItem& Item : PresetOptions)
	{
		if (Item.IsValid() && Item->Equals(SelectName, ESearchCase::IgnoreCase))
		{
			SelectedPreset = Item;
			break;
		}
	}
	if (!SelectedPreset.IsValid() && PresetOptions.Num() > 0)
	{
		SelectedPreset = PresetOptions[0];
	}

	if (PresetCombo.IsValid())
	{
		bIgnorePresetSelectionChanged = true;
		PresetCombo->RefreshOptions();
		PresetCombo->SetSelectedItem(SelectedPreset);
		bIgnorePresetSelectionChanged = false;
	}
}

FText SWeatherEnvironmentPanel::GetSelectedPresetText() const
{
	FEnvironmentData* Data = EnvController->GetEnvironmentData();
	if (Data->TimeOfDay.TimeSettings.CompleteCycle)
	{
		if (PresetTextBlock.IsValid()) PresetTextBlock->Invalidate(EInvalidateWidget::Paint);
		FText AppliedPresetName = EnvController->GetAppliedPresetName();
		return AppliedPresetName.IsEmpty() ? FText::FromString(TEXT("None")) : AppliedPresetName;
	}
	return FText::FromString(SelectedPreset.IsValid() ? *SelectedPreset : TEXT("None"));
}

FText SWeatherEnvironmentPanel::GetConfirmText() const
{
	return FText::FromString(bEditMode ? TEXT("Update") : TEXT("Add"));
}

void SWeatherEnvironmentPanel::IsPresetDirty(bool bPresetDirty)
{
	if (bPresetDirty)
	{
		bEditMode = false;
		EditingOldName.Empty();
		if (NameField.IsValid())
		{
			NameField->SetText(FText::GetEmpty());
			FSlateApplication::Get().SetKeyboardFocus(NameField, EFocusCause::SetDirectly);
		}

		GetConfirmText();

		if (Switcher.IsValid())
		{
			Switcher->SetActiveWidgetIndex(1);
		}
		if (OnPresetAppliedOrReset.IsBound())
		{
			OnPresetAppliedOrReset.Execute();
		}
		OnConfirmClicked();
	}
}

FReply SWeatherEnvironmentPanel::OnPlusClicked()
{
	bEditMode = false;
	EditingOldName.Empty();

	if (EnvController.IsValid())
	{
		EnvController->ResetCurrentWeatherEffectToDefaults();
	}

	if (NameField.IsValid())
	{
		NameField->SetText(FText::GetEmpty());
		FSlateApplication::Get().SetKeyboardFocus(NameField, EFocusCause::SetDirectly);
	}

	if (Switcher.IsValid())
	{
		Switcher->SetActiveWidgetIndex(1);
	}
	if (OnPresetAppliedOrReset.IsBound())
	{
		OnPresetAppliedOrReset.Execute();
	}
	return FReply::Handled();
}

FReply SWeatherEnvironmentPanel::OnEditClicked()
{
	if (!SelectedPreset.IsValid() || SelectedPreset->Equals(TEXT("None"), ESearchCase::IgnoreCase) || !EnvController.IsValid())
	{
		return FReply::Handled();
	}

	EditingOldName = *SelectedPreset;
	bEditMode = true;

	EnvController->ApplyEnvironmentPresetByName(EditingOldName);
	EnvController->BeginEditingPreset();
	if (NameField.IsValid())
	{
		NameField->SetText(FText::FromString(EditingOldName));
		FSlateApplication::Get().SetKeyboardFocus(NameField, EFocusCause::SetDirectly);
	}

	if (Switcher.IsValid())
	{
		Switcher->SetActiveWidgetIndex(1);
	}

	return FReply::Handled();
}

FReply SWeatherEnvironmentPanel::OnDeleteClicked()
{
	if (!SelectedPreset.IsValid() || SelectedPreset->Equals(TEXT("None"), ESearchCase::IgnoreCase) || !EnvController.IsValid())
	{
		return FReply::Handled();
	}

	EnvController->RemoveEnvironmentPreset(*SelectedPreset);
	EnvController->ResetCurrentWeatherEffectToDefaults();

	bEditMode = false;
	EditingOldName.Empty();

	if (Switcher.IsValid())
	{
		Switcher->SetActiveWidgetIndex(0);
	}

	RefreshPresetOptions(TEXT("None"));

	if (OnPresetAppliedOrReset.IsBound())
	{
		OnPresetAppliedOrReset.Execute();
	}
	return FReply::Handled();
}

FReply SWeatherEnvironmentPanel::OnConfirmClicked()
{
	if (!EnvController.IsValid() || !NameField.IsValid())
	{
		return FReply::Handled();
	}

	const FString Name = NameField->GetText().ToString().TrimStartAndEnd();
	if (Name.IsEmpty())
	{
		return FReply::Handled();
	}

	bool bOk = false;
	if (bEditMode)
	{
		bOk = EnvController->UpdateEnvironmentPreset(EditingOldName, Name);
	}
	else
	{
		bOk = EnvController->AddEnvironmentPreset(Name);
	}

	if (!bOk)
	{
		return FReply::Handled();
	}

	//EnvController->ApplyEnvironmentPresetByName(Name);

	bEditMode = false;
	EditingOldName.Empty();

	if (Switcher.IsValid())
	{
		Switcher->SetActiveWidgetIndex(0);
	}

	RefreshPresetOptions(Name);
	/*if (OnPresetAppliedOrReset.IsBound())
	{
		OnPresetAppliedOrReset.Execute();
	}*/
	return FReply::Handled();
}

FReply SWeatherEnvironmentPanel::OnCancelClicked()
{
	bEditMode = false;
	EditingOldName.Empty();

	if (Switcher.IsValid())
	{
		Switcher->SetActiveWidgetIndex(0);
	}

	if (SelectedPreset.IsValid())
	{
		if (SelectedPreset->Equals(TEXT("None"), ESearchCase::IgnoreCase))
		{
			if (EnvController.IsValid())
			{
				EnvController->ResetCurrentWeatherEffectToDefaults();
			}
		}
		else if (EnvController.IsValid())
		{
			EnvController->ApplyEnvironmentPresetByName(*SelectedPreset);
		}
	}
	if (OnPresetAppliedOrReset.IsBound())
	{
		OnPresetAppliedOrReset.Execute();
	}
	return FReply::Handled();
}

void SWeatherEnvironmentPanel::OnPresetChanged(FStringItem Item, ESelectInfo::Type)
{
	if (bIgnorePresetSelectionChanged)
	{
		return;
	}

	if (!Item.IsValid() || !EnvController.IsValid())
	{
		return;
	}

	const FString NewPresetName = *Item;
	const FString CurrentPresetName = SelectedPreset.IsValid() ? *SelectedPreset : TEXT("");

	if (NewPresetName.Equals(CurrentPresetName, ESearchCase::IgnoreCase))
	{
		return;
	}

	SelectedPreset = Item;

	if (SelectedPreset->Equals(TEXT("None"), ESearchCase::IgnoreCase))
	{
		EnvController->ResetCurrentWeatherEffectToDefaults();
	}
	else
	{
		EnvController->ApplyEnvironmentPresetByName(*SelectedPreset);
	}

	if (OnPresetAppliedOrReset.IsBound())
	{
		OnPresetAppliedOrReset.Execute();
	}
	FEnvironmentData* Data = EnvController->GetEnvironmentData();
	if (Data->TimeOfDay.TimeSettings.CompleteCycle)
	{
		GetSelectedPresetText();
	}
}

TSharedRef<SWidget> SWeatherEnvironmentPanel::MakeHeader(const FString& Title, const FString& IconPath)
{
	TSharedPtr<FSlateBrush> IconBrush = GetIconBrush(IconPath);
	if (!IconBrush.IsValid()) return SNullWidget::NullWidget;

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(Theme.HeaderFill)
		.Padding(FMargin(5, 5))
		[
			SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 6, 0)
				[
					IconBrush
						? StaticCastSharedRef<SWidget>(
							SNew(SBox)
							.WidthOverride(16.f)
							.HeightOverride(16.f)
							[
								SNew(SImage)
									.Image(IconBrush.Get())
							]
						)
						: StaticCastSharedRef<SWidget>(SNew(SSpacer))
				]

			+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Title))
						.Font(FontInfo10)
						.ColorAndOpacity(FLinearColor::White)
				]
		];
}

TSharedRef<SWidget> SWeatherEnvironmentPanel::MakeSeparator(float Thickness)
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(Theme.PanelLine)
		.Padding(FMargin(0, Thickness));
}