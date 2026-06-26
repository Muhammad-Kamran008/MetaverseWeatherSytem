#include "DateTimePicker.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"

void SDateTimePicker::Construct(const FArguments& InArgs)
{
	EnvController = InArgs._EnvironmentController;

	PickerType = InArgs._PickerType;
	OwningWindow = InArgs._OwningWindow;
	TargetField = InArgs._TargetField;
	OwnerDialog = InArgs._OwnerDialog;

	BgBase = InArgs._BgBase;
	PanelFill = InArgs._PanelFill;
	HeaderFill = InArgs._HeaderFill;
	PanelLine = InArgs._PanelLine;
	Accent = InArgs._Accent;

	MonthNames = { TEXT("Jan"),TEXT("Feb"),TEXT("Mar"),TEXT("Apr"),TEXT("May"),TEXT("Jun"),
				   TEXT("Jul"),TEXT("Aug"),TEXT("Sep"),TEXT("Oct"),TEXT("Nov"),TEXT("Dec") };

	// Options
	HourOptions.Reset();
	for (int32 H = 1; H <= 12; ++H)
	{
		HourOptions.Add(MakeShared<FString>(FString::Printf(TEXT("%02d"), H)));
	}

	MinuteOptions.Reset();
	for (int32 M = 0; M <= 59; ++M)
	{
		MinuteOptions.Add(MakeShared<FString>(FString::Printf(TEXT("%02d"), M)));
	}

	AmPmOptions = { MakeShared<FString>(TEXT("AM")), MakeShared<FString>(TEXT("PM")) };

	// Initialization
	if (PickerType == TEXT("Date"))
	{
		if (!TryInitSelectedDateFromTarget())
		{
			InitFallbackToToday();
		}
	}
	else
	{
		if (!TryInitSelectedTimeFromTarget())
		{
			// If no valid target text, keep defaults.
		}
	}

	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	ChildSlot
		[
			SNew(SBorder)
				.BorderImage(WhiteBrush)
				.BorderBackgroundColor(PanelLine)
				.Padding(1)
				[
					SNew(SBorder)
						.BorderImage(WhiteBrush)
						.BorderBackgroundColor(PanelFill)
						.Padding(10)
						[
							SAssignNew(ContentBox, SVerticalBox)
								+ SVerticalBox::Slot().AutoHeight()
								[
									(PickerType == TEXT("Date")) ? BuildDatePicker() : BuildTimePicker()
								]
						]
				]
		];
}

void SDateTimePicker::InitFallbackToToday()
{
	const FDateTime Now = FDateTime::Now();
	CurrentYear = Now.GetYear();
	CurrentMonth = Now.GetMonth() - 1;

	SelectedYear = CurrentYear;
	SelectedMonth = CurrentMonth;
	SelectedDay = Now.GetDay();
}

bool SDateTimePicker::TryInitSelectedDateFromTarget()
{
	TSharedPtr<SEditableTextBox> Field = TargetField.Pin();
	if (!Field.IsValid())
	{
		return false;
	}

	const FString Text = Field->GetText().ToString().TrimStartAndEnd();
	if (Text.IsEmpty())
	{
		return false;
	}

	// Expecting "DD/MM/YYYY"
	TArray<FString> Parts;
	Text.ParseIntoArray(Parts, TEXT("/"), true);
	if (Parts.Num() != 3)
	{
		return false;
	}

	const int32 Day = FCString::Atoi(*Parts[0]);
	const int32 MonthOneBased = FCString::Atoi(*Parts[1]);
	const int32 Year = FCString::Atoi(*Parts[2]);

	if (Day < 1 || Day > 31 || MonthOneBased < 1 || MonthOneBased > 12 || Year < 1)
	{
		return false;
	}

	SelectedDay = Day;
	SelectedMonth = MonthOneBased - 1;
	SelectedYear = Year;

	CurrentMonth = SelectedMonth;
	CurrentYear = SelectedYear;
	return true;
}

bool SDateTimePicker::TryInitSelectedTimeFromTarget()
{
	TSharedPtr<SEditableTextBox> Field = TargetField.Pin();
	if (!Field.IsValid())
	{
		return false;
	}

	const FString Text = Field->GetText().ToString().TrimStartAndEnd();
	if (Text.IsEmpty())
	{
		return false;
	}

	// Expected format: "HH:MM AM" or "HH:MM PM"
	FString TimePart, AmPmPart;
	if (!Text.Split(TEXT(" "), &TimePart, &AmPmPart))
	{
		return false;
	}

	FString HourPart, MinutePart;
	if (!TimePart.Split(TEXT(":"), &HourPart, &MinutePart))
	{
		return false;
	}

	HourPart = HourPart.TrimStartAndEnd();
	MinutePart = MinutePart.TrimStartAndEnd();
	AmPmPart = AmPmPart.TrimStartAndEnd();

	if (HourPart.Len() == 1)   HourPart = FString::Printf(TEXT("0%s"), *HourPart);
	if (MinutePart.Len() == 1) MinutePart = FString::Printf(TEXT("0%s"), *MinutePart);

	// Basic validation
	const int32 HH = FCString::Atoi(*HourPart);
	const int32 MM = FCString::Atoi(*MinutePart);
	if (HH < 1 || HH > 12 || MM < 0 || MM > 59)
	{
		return false;
	}
	if (!AmPmPart.Equals(TEXT("AM"), ESearchCase::IgnoreCase) && !AmPmPart.Equals(TEXT("PM"), ESearchCase::IgnoreCase))
	{
		return false;
	}

	SelectedHour = HourPart;
	SelectedMinute = MinutePart;
	SelectedAmPm = AmPmPart.ToUpper();
	return true;
}

TSharedRef<SWidget> SDateTimePicker::BuildDatePicker()
{
	const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");

	TSharedRef<SGridPanel> Grid = SNew(SGridPanel);

	static const TCHAR* WeekDays[] =
	{
		TEXT("Sun"), TEXT("Mon"), TEXT("Tue"), TEXT("Wed"),
		TEXT("Thu"), TEXT("Fri"), TEXT("Sat")
	};

	for (int32 i = 0; i < 7; ++i)
	{
		Grid->AddSlot(i, 0)
			[
				SNew(STextBlock)
					.Text(FText::FromString(WeekDays[i]))
					.ColorAndOpacity(FLinearColor::White)
			];
	}

	const int32 DaysInThisMonth = GetDaysInMonth(CurrentYear, CurrentMonth);
	const int32 FirstWeekday = GetFirstWeekdayOfMonth(CurrentYear, CurrentMonth);

	int32 Row = 1;
	int32 Col = FirstWeekday;

	// Empty cells before day 1
	for (int32 i = 0; i < FirstWeekday; ++i)
	{
		Grid->AddSlot(i, Row)
			[
				SNew(STextBlock).Text(FText::GetEmpty())
			];
	}

	for (int32 Day = 1; Day <= DaysInThisMonth; ++Day)
	{
		const bool bIsSelected =
			(Day == SelectedDay && CurrentMonth == SelectedMonth && CurrentYear == SelectedYear);

		Grid->AddSlot(Col, Row)
			.Padding(2)
			[
				SNew(SButton)
					.ButtonColorAndOpacity(bIsSelected ? Accent : BgBase)
					.ContentPadding(4)
					.OnClicked_Lambda([this, Day]()
						{
							SelectedDay = Day;
							SelectedMonth = CurrentMonth;
							SelectedYear = CurrentYear;

							const int32 MonthOneBased = CurrentMonth + 1;
							const FString Picked = FString::Printf(TEXT("%02d/%02d/%d"), Day, MonthOneBased, CurrentYear);

							CommitToTargetAndClose(Picked);
							return FReply::Handled();
						})
					[
						SNew(STextBlock)
							.Text(FText::AsNumber(Day))
							.ColorAndOpacity(FLinearColor::White)
					]
			];

		Col++;
		if (Col > 6)
		{
			Col = 0;
			Row++;
		}
	}

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
		[
			SNew(SHorizontalBox)

				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
						.Text(FText::FromString(TEXT("<")))
						.OnClicked(this, &SDateTimePicker::PrevMonth)
				]

				+ SHorizontalBox::Slot().FillWidth(1.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString(MonthNames[CurrentMonth] + TEXT(" ") + FString::FromInt(CurrentYear)))
						.ColorAndOpacity(FLinearColor::White)
				]

				+ SHorizontalBox::Slot().AutoWidth()
				[
					SNew(SButton)
						.Text(FText::FromString(TEXT(">")))
						.OnClicked(this, &SDateTimePicker::NextMonth)
				]
		]

	+ SVerticalBox::Slot().AutoHeight()
		[
			Grid
		];
}

void SDateTimePicker::RefreshDatePicker()
{
	if (!ContentBox.IsValid())
	{
		return;
	}

	ContentBox->ClearChildren();
	ContentBox->AddSlot()
		.AutoHeight()
		[
			BuildDatePicker()
		];
}

TSharedRef<SWidget> SDateTimePicker::BuildTimePicker()
{
	auto GenItem = [](TSharedPtr<FString> Item)
		{
			return SNew(STextBlock)
				.Text(FText::FromString(Item.IsValid() ? *Item : TEXT("")))
				.ColorAndOpacity(FLinearColor::White);
		};

	auto MakeCombo =
		[this, &GenItem](TArray<TSharedPtr<FString>>* Source,
			TFunction<void(const FString&)> Setter,
			TFunction<FText()> GetText)
		{
			return SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(Source)
				.OnGenerateWidget_Lambda(GenItem)
				.OnSelectionChanged_Lambda([Setter](TSharedPtr<FString> Item, ESelectInfo::Type)
					{
						if (Item.IsValid())
						{
							Setter(*Item);
						}
					})
				[
					SNew(STextBlock)
						.Text_Lambda([GetText]() { return GetText(); })
						.ColorAndOpacity(FLinearColor::White)
				];
		};

	TSharedRef<SWidget> HourCombo =
		MakeCombo(&HourOptions,
			[this](const FString& V) { SelectedHour = V; },
			[this]() { return FText::FromString(SelectedHour); });

	TSharedRef<SWidget> MinCombo =
		MakeCombo(&MinuteOptions,
			[this](const FString& V) { SelectedMinute = V; },
			[this]() { return FText::FromString(SelectedMinute); });

	TSharedRef<SWidget> AmPmCombo =
		MakeCombo(&AmPmOptions,
			[this](const FString& V) { SelectedAmPm = V; },
			[this]() { return FText::FromString(SelectedAmPm); });

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 10)
		[
			SNew(STextBlock)
				.Text(FText::FromString(TEXT("Select Time")))
				.ColorAndOpacity(FLinearColor::White)
		]

		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4)[HourCombo]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4)[MinCombo]
				+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(4)[AmPmCombo]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 10, 0, 0)
		[
			SNew(SButton)
				.ButtonColorAndOpacity(Accent)
				.Text(FText::FromString(TEXT("OK")))
				.OnClicked_Lambda([this]()
					{
						const FString Picked = SelectedHour + TEXT(":") + SelectedMinute + TEXT(" ") + SelectedAmPm;
						CommitToTargetAndClose(Picked);
						return FReply::Handled();
					})
		];
}

FReply SDateTimePicker::PrevMonth()
{
	CurrentMonth--;
	if (CurrentMonth < 0)
	{
		CurrentMonth = 11;
		CurrentYear--;
	}

	RefreshDatePicker();
	return FReply::Handled();
}

FReply SDateTimePicker::NextMonth()
{
	CurrentMonth++;
	if (CurrentMonth > 11)
	{
		CurrentMonth = 0;
		CurrentYear++;
	}

	RefreshDatePicker();
	return FReply::Handled();
}

bool SDateTimePicker::IsLeapYear(int32 Year) const
{
	return (Year % 4 == 0 && Year % 100 != 0) || (Year % 400 == 0);
}

int32 SDateTimePicker::GetDaysInMonth(int32 Year, int32 Month) const
{
	switch (Month)
	{
	case 1:  return IsLeapYear(Year) ? 29 : 28; // Feb
	case 3:  return 30; // Apr
	case 5:  return 30; // Jun
	case 8:  return 30; // Sep
	case 10: return 30; // Nov
	default: return 31;
	}
}

int32 SDateTimePicker::GetFirstWeekdayOfMonth(int32 Year, int32 Month) const
{
	// Zeller's Congruence (Gregorian)
	int32 m = Month + 1;
	int32 y = Year;

	if (m < 3)
	{
		m += 12;
		y -= 1;
	}

	const int32 K = y % 100;
	const int32 J = y / 100;

	const int32 h = (1 + (13 * (m + 1)) / 5 + K + (K / 4) + (J / 4) + (5 * J)) % 7;
	return (h + 6) % 7; 
}

void SDateTimePicker::CommitToTargetAndClose(const FString& Value)
{
	if(TSharedPtr<SEditableTextBox> Field = TargetField.Pin())
	{
		Field->SetText(FText::FromString(Value));
	}
	if (TSharedPtr<SWeatherDialog> Owner = OwnerDialog.Pin())
	{
		Owner->OnPickerCommitted(PickerType, Value);
	}
	CloseOwningWindow();
}

void SDateTimePicker::CloseOwningWindow()
{
	if (TSharedPtr<SWindow> Win = OwningWindow.Pin())
	{
		Win->RequestDestroyWindow();
	}
}
