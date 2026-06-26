#include "AreaOfInterestDialog.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "UIStyle.h"
#include "LocalGeometryFactory.h"

SAreaOfInterestDialog::SAreaOfInterestDialog()
	:FontAsset(LoadObject<UObject>(nullptr, TEXT("/Game/Fonts/segoeui_Font.segoeui_Font")))
	, FontInfo12(FontAsset, 12, FName("Default"))
	, FontInfo10(FontAsset, 10, FName("Default"))
	, FontInfo8(FontAsset, 9, FName("Default"))
{}

TSharedRef<SWidget> SAreaOfInterestDialog::FullWidthReadOnlyField(const FText& Value)
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	return SNew(SBorder)
		.BorderImage(WhiteBrush)
		.BorderBackgroundColor(PanelLine)
		.Padding(1)
		[
			SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(BgBase) 
				.Padding(FMargin(10, 6))
				[
					SNew(STextBlock)
						.Text(Value)
						.ColorAndOpacity(FLinearColor::White)
				]
		];
}

TSharedPtr<FSlateBrush> SAreaOfInterestDialog::GetIconBrush(FString IconName, float IconSize)
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


TSharedRef<SWidget> SAreaOfInterestDialog::MakeIconLabel(
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

void SAreaOfInterestDialog::Construct(const FArguments& InArgs)
{

	Styles = InArgs._Styles;

	OwningWindow = InArgs._OwningWindow;
	BgBase = InArgs._BgBase;
	HeaderFill = InArgs._HeaderFill;
	PanelFill = InArgs._PanelFill;
	PanelLine = InArgs._PanelLine;
	Accent = InArgs._Accent;
	TextBoxStyle = InArgs._TextBoxStyle;
	ButtonStyle = InArgs._ButtonStyle;

	EnvController = InArgs._EnvironmentController;

	OnAddConfirmed = InArgs._OnAddConfirmed;

	{
		const FComboBoxStyle* Base =
			Styles.ComboStyle
			? Styles.ComboStyle
			: &FAppStyle::Get().GetWidgetStyle<FComboBoxStyle>("ComboBox");

		ComboStyle = *Base;

		ComboStyle.ComboButtonStyle.ButtonStyle.Hovered = ComboStyle.ComboButtonStyle.ButtonStyle.Normal;
		ComboStyle.ComboButtonStyle.ButtonStyle.Pressed = ComboStyle.ComboButtonStyle.ButtonStyle.Normal;
	}





	GeometryOptions =
	{
		MakeShared<FString>(TEXT("Rectangle")),
		MakeShared<FString>(TEXT("Circle")),
	};
	SelectedGeometry = GeometryOptions[0];

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	ChildSlot
		[
			SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(BgBase)
				[
					SNew(SVerticalBox)

						+ SVerticalBox::Slot().AutoHeight()
						[
							HeaderBar()
						]

						+ SVerticalBox::Slot().FillHeight(1.f)
						[
							SNew(SScrollBox)

								+ SScrollBox::Slot()
								.Padding(FMargin(18, 14, 18, 14))
								[
									SNew(SVerticalBox)

										+ SVerticalBox::Slot().AutoHeight()
										[
											Label(TEXT("Geometry Type"), false, 9)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 14)
										[
											//FullWidthReadOnlyField(FText::FromString(TEXT("Rectangle")))
											MakeCombo(
												&GeometryOptions,
												SelectedGeometry,
												TEXT("Select Shape"),
												460.f,
												[this]()
												{


												}
											)
										]

									// Area Name
									+ SVerticalBox::Slot().AutoHeight()
										[
											Label(TEXT("Area Name"), false, 9)
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 16)
										[
											FullWidthField(AreaName, FText::FromString(TEXT("Enter area name")))
										]
										+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 16)
										[
											SNew(SHorizontalBox)
												+ SHorizontalBox::Slot()
												.AutoWidth()
												.Padding(0, 0, 10, 0)
												[
													SNew(STextBlock)
														.Text(FText::FromString("Pick From Map"))
														.Font(FontInfo8)
														.ColorAndOpacity(FLinearColor::White)
												]

												+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
												[
													SNew(SCheckBox)

														.Style(&FAppStyle::Get().GetWidgetStyle<FCheckBoxStyle>("Checkbox"))
												]
										]

									+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
										[
											CoordSection(TEXT("TOP RIGHT"), TR_Lat, TR_Lon)
										]

										+ SVerticalBox::Slot().AutoHeight().Padding(0, 18, 0, 0)
										[
											CoordSection(TEXT("BOTTOM LEFT"), BL_Lat, BL_Lon)
										]
								]
						]

					// ===== Footer separator =====
					+ SVerticalBox::Slot().AutoHeight()
						[
							SeparatorLine(1.f)
						]

						// ===== Footer buttons bar (matches prototype) =====
						+ SVerticalBox::Slot().AutoHeight()
						[
							SNew(SBorder)
								.BorderImage(WhiteBrush)
								.BorderBackgroundColor(BgBase)
								.Padding(FMargin(18, 12))
								[
									SNew(SHorizontalBox)

										// Cancel (left)
										+ SHorizontalBox::Slot().AutoWidth()
										[
											SNew(SButton)
												.ButtonStyle(ButtonStyle ? ButtonStyle : &FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
												.OnClicked(this, &SAreaOfInterestDialog::OnCancel)
												[
													SNew(STextBlock)
														.Text(FText::FromString(TEXT("Cancel")))
														.ColorAndOpacity(FLinearColor::White)
												]
										]

									+ SHorizontalBox::Slot().FillWidth(1.f)
										[
											SNew(SSpacer)
										]

										// Add Area (right) — tinted to Accent similar to prototype green button
										+ SHorizontalBox::Slot().AutoWidth()
										[
											SNew(SButton)
												.ButtonStyle(ButtonStyle ? ButtonStyle : &FAppStyle::Get().GetWidgetStyle<FButtonStyle>("Button"))
												.ButtonColorAndOpacity(Accent)
												.OnClicked(this, &SAreaOfInterestDialog::OnAdd)
												[
													SNew(STextBlock)
														.Text(FText::FromString(TEXT("Add Area")))
														.ColorAndOpacity(FLinearColor::White)
												]
										]
								]
						]
				]
		];
}

TSharedRef<SWidget> SAreaOfInterestDialog::HeaderBar()
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
	const FSlateBrush* HeaderIcon = FAppStyle::Get().GetBrush("Icons.Circle");  

	return SNew(SBorder)
		.BorderImage(WhiteBrush)
		.BorderBackgroundColor(HeaderFill)
		.Padding(FMargin(18, 10))
		[
			SNew(SHorizontalBox)

				// Left: icon + title (uppercase)
				+ SHorizontalBox::Slot().FillWidth(1.f).VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 10, 0)
						[
							SNew(SImage)
								.Image(HeaderIcon)
								.ColorAndOpacity(Accent)
						]

						+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
						[
								MakeIconLabel(
									TEXT("ADD AREA OF INTEREST"),
									TEXT("Area"),
									&FontInfo10,
									Accent
								)
						]
				]

			// Right: close X
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[
					SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
						.ContentPadding(2)
						.OnClicked(this, &SAreaOfInterestDialog::OnClose)
						[
							SNew(SImage)
								.Image(FAppStyle::Get().GetBrush("Icons.X"))
								.ColorAndOpacity(FLinearColor::White)
						]
				]
		];
}

TSharedRef<SWidget> SAreaOfInterestDialog::SeparatorLine(float Thickness) const
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	return SNew(SBorder)
		.BorderImage(WhiteBrush)
		.BorderBackgroundColor(PanelLine)
		.Padding(FMargin(0, Thickness));
}

TSharedRef<SWidget> SAreaOfInterestDialog::Label(const FString& Text, bool bUpper, int32 Size) const
{
	FString Out = bUpper ? Text.ToUpper() : Text;

	return SNew(STextBlock)
		.Text(FText::FromString(Out))
		.Font(FontInfo10)
		.ColorAndOpacity(FLinearColor(0.65f, 0.72f, 0.80f, 1.f)); 
}

TSharedRef<SWidget> SAreaOfInterestDialog::FullWidthField(TSharedPtr<SEditableTextBox>& OutField, const FText& Hint)
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	const FLinearColor FieldFill = BgBase;   
	const FLinearColor Outline = PanelLine; 

	return SNew(SBorder)
		.BorderImage(WhiteBrush)
		.BorderBackgroundColor(PanelLine)
		.Padding(1)
		[
			SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(BgBase)
				.Padding(FMargin(8, 4))
				[
					SAssignNew(OutField, SEditableTextBox)
						.Style(TextBoxStyle ? TextBoxStyle : &FAppStyle::Get().GetWidgetStyle<FEditableTextBoxStyle>("NormalEditableTextBox"))
						.HintText(Hint)
				]
		];
}


TSharedRef<SWidget> SAreaOfInterestDialog::MakeCombo(
	TArray<TSharedPtr<FString>>* Source,
	TSharedPtr<FString>& InOutSelected,
	const FString& DefaultText,
	float MinWidth,
	TFunction<void()> OnChanged
)
{
	auto GenItem = [](TSharedPtr<FString> Item)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
				.ColorAndOpacity(FLinearColor::White)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9));
		};

	TSharedPtr<SComboBox<TSharedPtr<FString>>> ComboBoxWidget = SNew(SComboBox<TSharedPtr<FString>>)
		.ComboBoxStyle(&ComboStyle)
		.OptionsSource(Source)
		.InitiallySelectedItem(InOutSelected)
		.OnGenerateWidget_Lambda(GenItem)
		.OnSelectionChanged_Lambda(
			[&InOutSelected, OnChanged](TSharedPtr<FString> Item, ESelectInfo::Type)
			{
				if (!Item.IsValid())
					return;
				InOutSelected = Item;
				if (OnChanged)
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


	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(PanelLine)
		.Padding(1)
		[
			SNew(SBorder)
				.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
				.BorderBackgroundColor(BgBase)
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




//TSharedRef<SWidget> SAreaOfInterestDialog::FullWidthCombo()
//{
//	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
//
//	return SNew(SBorder)
//		.BorderImage(WhiteBrush)
//		.BorderBackgroundColor(BgBase)
//		.Padding(FMargin(8, 4))
//		[
//			SNew(SComboBox<TSharedPtr<FString>>)
//				.ComboBoxStyle(ComboStyle ? ComboStyle : &FAppStyle::Get().GetWidgetStyle<FComboBoxStyle>("ComboBox"))
//				.OptionsSource(&GeometryOptions)
//				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
//					{
//						return SNew(STextBlock)
//							.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
//							.ColorAndOpacity(FLinearColor::White);
//					})
//				.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Item, ESelectInfo::Type)
//					{
//						if (Item.IsValid()) SelectedGeometry = Item;
//					})
//				[
//					SNew(STextBlock)
//						.Text_Lambda([this]()
//							{
//								return FText::FromString(SelectedGeometry.IsValid() ? *SelectedGeometry : TEXT("Select"));
//							})
//						.ColorAndOpacity(FLinearColor::White)
//				]
//		];
//}

TSharedRef<SWidget> SAreaOfInterestDialog::CoordSection(const FString& Title, TSharedPtr<SEditableTextBox>& Lat, TSharedPtr<SEditableTextBox>& Lon)
{
	return SNew(SVerticalBox)

		// Section title
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(STextBlock)
				.Text(FText::FromString(Title))
				.Font(FontInfo10)
				.ColorAndOpacity(FLinearColor::White)
		]

		// Latitude label + field
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[
			Label(TEXT("Latitude"), false, 9)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 12)
		[
			FullWidthField(Lat, FText::FromString(TEXT("e.g., 51.5074° N")))
		]

		// Longitude label + field
		+ SVerticalBox::Slot().AutoHeight()
		[
			Label(TEXT("Longitude"), false, 9)
		]
		+ SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 0)
		[
			FullWidthField(Lon, FText::FromString(TEXT("e.g., 0.1278° W")))
		];
}

FReply SAreaOfInterestDialog::OnClose()
{
	return OnCancel();
}

FReply SAreaOfInterestDialog::OnCancel()
{
	if (TSharedPtr<SWindow> Win = OwningWindow.Pin())
	{
		Win->RequestDestroyWindow();
	}
	return FReply::Handled();
}

FReply SAreaOfInterestDialog::OnAdd()
{
	if (!EnvController.IsValid())
	{
		return FReply::Handled();
	}

	const FString& IntStr = *SelectedGeometry;
	EAOIGeometry Type;
	FieldsData* fieldsData = nullptr;
	if (IntStr == TEXT("Rectangle")) {
		Type = EAOIGeometry::Rectangle;
		FRectangle_1FieldsData* Data = new FRectangle_1FieldsData();
		Data->AreaName = AreaName->GetText().ToString();
		Data->TopRightLat = FCString::Atod(*TR_Lat->GetText().ToString());
		Data->TopRightLon= FCString::Atod(*TR_Lon->GetText().ToString());
		Data->BottomLeftLat = FCString::Atod(*BL_Lat->GetText().ToString());
		Data->BottomLeftLon = FCString::Atod(*BL_Lon->GetText().ToString());
		fieldsData = Data;
	}
	else if (IntStr == TEXT("Circle")) {
		Type = EAOIGeometry::Circle;
		FBoundingSphereFieldsData* Data = new FBoundingSphereFieldsData();
		Data->AreaName = AreaName->GetText().ToString();

		// TODOS:
		//Data->Radius = ;
		//Data->Center = ;
		fieldsData = Data;
	}

	FAreaOfInterest* Geometery = LocalGeometryFactory::GetLocalGeometryFactoryInstance()->GetGeomtery(Type, EnvController->GetEnvironmentData(), fieldsData);
	EnvController->AddAreaOfInterest(Geometery);

	OnAddConfirmed.ExecuteIfBound();

	if (TSharedPtr<SWindow> Win = OwningWindow.Pin())
	{
		Win->RequestDestroyWindow();
	}


	return FReply::Handled();
}


