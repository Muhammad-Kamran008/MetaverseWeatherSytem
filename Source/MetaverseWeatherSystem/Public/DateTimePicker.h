#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"
#include "UIEnvironmentController.h"
#include "WeatherDialog.h"
#include "Widgets/Input/SEditableTextBox.h"

class METAVERSEWEATHERSYSTEM_API SDateTimePicker : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDateTimePicker)
		: _PickerType(TEXT("Date"))
		, _BgBase(FLinearColor(0.040f, 0.060f, 0.090f, 1.f))
		, _PanelFill(FLinearColor(0.015f, 0.025f, 0.045f, 1.f))
		, _HeaderFill(FLinearColor(0.015f, 0.025f, 0.045f, 1.f))
		, _PanelLine(FLinearColor(0.14f, 0.20f, 0.30f, 1.f))
		, _Accent(FLinearColor(0.12f, 0.38f, 0.70f, 1.f))
		{}
		SLATE_ARGUMENT(FName, PickerType)
		SLATE_ARGUMENT(TWeakPtr<SWindow>, OwningWindow)
		SLATE_ARGUMENT(TSharedPtr<SEditableTextBox>, TargetField)
		SLATE_ARGUMENT(TWeakPtr<SWeatherDialog>, OwnerDialog)

		SLATE_ARGUMENT(FLinearColor, BgBase)
		SLATE_ARGUMENT(FLinearColor, PanelFill)
		SLATE_ARGUMENT(FLinearColor, HeaderFill)
		SLATE_ARGUMENT(FLinearColor, PanelLine)
		SLATE_ARGUMENT(FLinearColor, Accent)

		SLATE_ARGUMENT(UUIEnvironmentController*, EnvironmentController)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// Args
	UPROPERTY()
	FName PickerType;
	TWeakPtr<SWindow> OwningWindow;
	TWeakPtr<SEditableTextBox> TargetField;
	TWeakPtr<SWeatherDialog> OwnerDialog;
	// Date data
	TArray<FString> MonthNames;
	int32 CurrentMonth = 0;
	int32 CurrentYear = 2021;

	// Time data
	TArray<TSharedPtr<FString>> HourOptions;
	TArray<TSharedPtr<FString>> MinuteOptions; 
	TArray<TSharedPtr<FString>> AmPmOptions;

	// Theme colors
	FLinearColor BgBase, PanelFill, HeaderFill, PanelLine, Accent;

	// UI containers 
	TSharedPtr<class SBorder> RootBorder;
	TSharedPtr<class SVerticalBox> ContentBox;

private:
	TSharedRef<SWidget> BuildDatePicker();
	TSharedRef<SWidget> BuildTimePicker();

	void RefreshDatePicker();

	FReply PrevMonth();
	FReply NextMonth();

	void CommitToTargetAndClose(const FString& Value);
	void CloseOwningWindow();

	int32 GetDaysInMonth(int32 Year, int32 Month) const;
	int32 GetFirstWeekdayOfMonth(int32 Year, int32 Month) const;
	bool  IsLeapYear(int32 Year) const;
	void InitFallbackToToday();

	UPROPERTY() TWeakObjectPtr<UUIEnvironmentController> EnvController;
public:

	int32 SelectedDay = -1;
	int32 SelectedMonth = -1; 
	int32 SelectedYear = -1;
	FString SelectedHour = TEXT("09");
	FString SelectedMinute = TEXT("36");
	FString SelectedAmPm = TEXT("AM");
	bool TryInitSelectedDateFromTarget();
	bool TryInitSelectedTimeFromTarget();
};
