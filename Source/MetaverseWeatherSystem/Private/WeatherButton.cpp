//// Fill out your copyright notice in the Description page of Project Settings.
//
//
//#include "WeatherButton.h"
//#include "Components/Button.h"
//#include "Framework/Application/SlateApplication.h"
//
//void UWeatherButton::NativeConstruct()
//{
//    Super::NativeConstruct();
//
//    if (WeatherButton)
//    {
//        WeatherButton->OnClicked.AddDynamic(this, &UWeatherButton::OnWeatherButtonClick);
//    }
//}
//
//void UWeatherButton::OnWeatherButtonClick()
//{
//    if (FSlateApplication::IsInitialized())
//    {
//        // Check if WeatherDialog is already initialized
//        if (!WeatherDialog.IsValid())
//        {
//            // Initialize the WeatherDialog widget
//            WeatherDialog = SNew(SWeatherDialog);
//        }
//
//        // Create and show the weather settings window
//        if (!WeatherDialog.IsValid())
//        {
//            UE_LOG(LogTemp, Error, TEXT("WeatherDialog failed to initialize."));
//            return;
//        }
//
//        TSharedRef<SWindow> WeatherWindow = SNew(SWindow)
//            .Title(FText::FromString(TEXT("Weather Settings")))
//            .ClientSize(FVector2D(980, 720)) // Adjust this size as needed
//            .Content()
//            [
//                WeatherDialog.ToSharedRef()  // Add the weather dialog widget to the window
//            ];
//
//        FSlateApplication::Get().AddWindow(WeatherWindow);
//        UE_LOG(LogTemp, Warning, TEXT("Weather dialog window added successfully."));
//    }
//}