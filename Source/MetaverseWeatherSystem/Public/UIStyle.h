#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"

// --------------------
// Theme + Style pack
// --------------------
struct FUITheme
{
	//FLinearColor BgBase = FLinearColor::FromSRGBColor(FColor(29, 48, 68, 255));//FLinearColor(0.020f, 0.030f, 0.055f, 1.f);
	//FLinearColor HeaderFill = FLinearColor::FromSRGBColor(FColor(18, 26, 46, 255));//FLinearColor(0.030f, 0.045f, 0.080f, 1.f);
	//FLinearColor PanelFill = FLinearColor::FromSRGBColor(FColor(18, 26, 46, 255)); //FLinearColor(0.045f, 0.065f, 0.105f, 1.f);
	//FLinearColor PanelLine = FLinearColor(0.160f, 0.220f, 0.320f, 1.f);
	//FLinearColor Accent = FLinearColor(2.f / 255.f, 190.f / 255.f, 118.f / 255.f, 1.f);//FLinearColor(0.00f, 0.85f, 0.62f, 1.f);
	

	FLinearColor BgBase = FLinearColor(0.1f, 0.1f, 0.1f, 0.907f);
	FLinearColor HeaderFill = FLinearColor(0.022f, 0.022f, 0.022f, 0.907f);
	FLinearColor PanelFill = FLinearColor(0.022f, 0.022f, 0.022f, 0.907f); 
	FLinearColor PanelLine = FLinearColor(1.0f, 1.0f, 1.0f, 1.f);
	FLinearColor Accent = FLinearColor(0.5f, 0.5f, 0.5f, 1.f);



	float HeadingIconSize = 14.f;
	float GroupIconSize = 12.f;
};

struct FWeatherStyles
{
	const FEditableTextBoxStyle* TextBoxStyle = nullptr;
	const FComboBoxStyle* ComboStyle = nullptr;
	const FButtonStyle* ButtonStyle = nullptr;
	const FCheckBoxStyle* CheckStyle = nullptr;
};

// Header icon source type
enum class EHeaderIconSource : uint8
{
	None,
	Slate,     // FAppStyle/FCoreStyle brush by name
	Custom     // Brush loaded from content path
};

struct FHeaderIcon
{
	EHeaderIconSource Source = EHeaderIconSource::None;
	FName SlateBrushName;          // e.g. "Icons.Clock"
	FString ContentBrushPath;      // e.g. "/Game/UI/Icons/Ico_TimeOfDay.Ico_TimeOfDay"
	float Size = 14.f;
};

// --------------------
// Shared state for Weather Effects UI 
// --------------------
struct FWeatherEffectsState : public TSharedFromThis<FWeatherEffectsState>
{
	// Precipitation
	float PrecipMmPerHr = 0.0f;

	// Visibility
	float VisibilityKm = 10.f;

	// Clouds
	float CloudsTopFt = 8000.f;
	float CloudsBaseFt = 3000.f;
	bool  bCloudShadows = false;

	// Wind
	float WindDirDeg = 270.f;
	float WindSpeedKt = 12.f;
	bool  bWindWhistling = false;
};

// --------------------
// Row models used by WeatherEffectsPanel (backend-friendly)
// --------------------
struct FCloudLayerRow
{
	int32  Layer = 1;
	int32  BaseFt = 3000;
	int32  TopFt = 5000;
	FString Type = TEXT("Cumulus");
	FString Cover = TEXT("Scattered");
};

struct FWindLayerRow
{
	int32 Layer = 1;
	int32 AltFt = 0;
	int32 DirDeg = 270;
	int32 SpdKt = 12;
};

// --------------------
// Environment preset model 
// --------------------
struct FWeatherTemplate
{
	FString Name = TEXT("Preset");

	// Toggles (sub-groups)
	bool bPrecipEnabled = true;
	bool bVisibilityEnabled = true;
	bool bCloudsEnabled = true;
	bool bWindEnabled = true;

	// Top-level combo selections 
	FString PrecipType = TEXT("Rain");
	FString PrecipIntensity = TEXT("Light");
	FString VisibilityObsc = TEXT("Fog");
	FString Thunder = TEXT("None");

	// Values + boolean flags (shared state)
	FWeatherEffectsState State;

	// Layer tables
	TArray<FCloudLayerRow> CloudLayers;
	TArray<FWindLayerRow>  WindLayers;
};

