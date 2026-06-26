// Fill out your copyright notice in the Description page of Project Settings.

#include "UIEnvironmentController.h"
#include "EnvironmentManager.h"
#include "WeatherPrecipitationMapping.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "XmlFile.h"
#include "XmlNode.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UUIEnvironmentController::Initialize()
{
	Super::Initialize();
	LoadEnvironmentPresetsFromXml();
}

//void UUIEnvironmentController::UpdateEnvironmentData(const FEnvironmentData& NewData)
//{
//	//Super::UpdateEnvironmentData(NewData);
//	if (NewData = ScenarioEnvironment.CurrentWeatherEffect.Precipitation.Type = EPrecipitationType::Rain)
//	{
//		Super::UpdatePrecipitationData(NewData);
//	}
//}

UTexture2D* UUIEnvironmentController::GetIcon(FString Name)
{
	if (Icons.Contains(Name))
	{
		return Icons[Name];
	}
	FString Path = GetPathForIcon(Name);
	UTexture2D* Tex = LoadObject<UTexture2D>(nullptr, *Path);
	if (Tex)
	{
		Icons.Add(Name, Tex);
	}
	return Tex;
}

void UUIEnvironmentController::AddAreaOfInterest(FAreaOfInterest* AOI)
{
	FEnvironmentData* Data = GetEnvironmentData();
	if (Data)
	{
		Data->LocalScenarioEnvironment.AddAreaOfInterest(AOI);
		Manager->AddAreaofInterest(AOI);
	}
}
void UUIEnvironmentController::AddCloudLayer(FCloudLayerData& CloudLayer) {


}

void  UUIEnvironmentController::AddWindLayer(FWindLayerData& WindLayer) {



}

void UUIEnvironmentController::SetCurrentAOI(const FString& key)
{
	FEnvironmentData* Data = GetEnvironmentData();
	if (Data)
	{
		Data->LocalScenarioEnvironment.SetSelectedAreaOfInterestByKey(key);
	}
}

void UUIEnvironmentController::AddCloudLayer(const FCloudLayerData& CloudLayerData)
{
	FEnvironmentData* Data = GetEnvironmentData();
	if (Data)
	{
		Data->LocalScenarioEnvironment.SelectedEnvironmentPreset.WeatherEffect.Clouds.AddCloudLayer(CloudLayerData);
	}
}

void UUIEnvironmentController::setIsNetwork(bool bisNetwork) {
	SetFlag(bisNetwork);
}

TSharedPtr<FWeatherEffectsState> UUIEnvironmentController::GetWeatherEffectsState()
{
	if (!PersistentWeatherEffectsState.IsValid())
	{
		PersistentWeatherEffectsState = MakeShared<FWeatherEffectsState>();
	}
	return PersistentWeatherEffectsState;
}

void UUIEnvironmentController::AddWindLayer(const FWindLayerData& WindLayerData)
{
	FEnvironmentData* Data = GetEnvironmentData();
	if (Data)
	{
		Data->LocalScenarioEnvironment.SelectedEnvironmentPreset.WeatherEffect.Wind.AddWindLayer(WindLayerData);
	}
}

const TMap<FString, FAreaOfInterest*>& UUIEnvironmentController::GetListOFAreaofInterest() const
{
	FEnvironmentData* Data = GetEnvironmentData();
	return Data->LocalScenarioEnvironment.GetAreaOfInterestPresets();
}

//------------------------
//--------Environment-----
//------------------------
const TArray<FScenarioEnvironmentPreset>& UUIEnvironmentController::GetEnvironmentPresets() const
{
	static const TArray<FScenarioEnvironmentPreset> Empty;
	const FEnvironmentData* Data = GetEnvironmentData();
	return Data ? Data->LocalScenarioEnvironment.EnvironmentPresets : Empty;
}

bool UUIEnvironmentController::ResetCurrentWeatherEffectToDefaults()
{
	FEnvironmentData* Data = GetEnvironmentData();
	if (!Data) return false;

	FAreaOfInterest* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
	if (!AOI) return false;

	FWeatherEffectsData DefaultWeather;

	// -------------------------
	// Precipitation
	// -------------------------
	DefaultWeather.Precipitation.bEnabled = true;
	DefaultWeather.Precipitation.Type = EPrecipitationType::None;
	DefaultWeather.Precipitation.Intensity = EPrecipitationIntensity::None;
	DefaultWeather.Precipitation.RateMmPerHr = 0.0f;

	// -------------------------
	// Visibility
	// -------------------------
	DefaultWeather.Visibility.bEnabled = true;
	DefaultWeather.Visibility.Obscurant = EVisibilityObscurant::None;
	DefaultWeather.Visibility.DistanceKm = 0.0f;

	// -------------------------
	// Clouds
	// -------------------------
	DefaultWeather.Clouds.bEnabled = true;
	DefaultWeather.Clouds.bShadows = false;
	DefaultWeather.Clouds.SelectedCloudLayerIndex = INDEX_NONE;
	DefaultWeather.Clouds.Thunder = EThunderOption::None;
	DefaultWeather.Clouds.Layers.Reset();

	DefaultWeather.Clouds.CloudLayer = FCloudLayerData();
	DefaultWeather.Clouds.CloudLayer.Type = ECloudType::CompleteClearSkies;
	DefaultWeather.Clouds.CloudLayer.Cover = ECloudCover::Clear;
	DefaultWeather.Clouds.CloudLayer.BaseFt = 0.0f;
	DefaultWeather.Clouds.CloudLayer.TopFt = 0.0f;

	// -------------------------
	// Wind
	// -------------------------
	DefaultWeather.Wind.bEnabled = true;
	DefaultWeather.Wind.SelectedLayerIndex = INDEX_NONE;
	DefaultWeather.Wind.Layers.Reset();

	DefaultWeather.Wind.CurrentWindLayer = FWindLayerData();
	DefaultWeather.Wind.CurrentWindLayer.SpeedKt = 0.0f;
	DefaultWeather.Wind.CurrentWindLayer.DirectionDeg = 0.0f;
	DefaultWeather.Wind.CurrentWindLayer.AltitudeFt = 0.0f;
	DefaultWeather.Wind.CurrentWindLayer.DownwindDirectionDeg = 0.0f;

	AOI->CurrentWeatherEffect = DefaultWeather;
	AOI->CurrentEnvironmentPreset = FScenarioEnvironmentPreset();
	Data->LocalScenarioEnvironment.SelectedEnvironmentPreset = FScenarioEnvironmentPreset();

	UpdatePrecipitationData(AOI);
	UpdateVisibilityData(AOI);

	FCloudLayerData CloudLayer = AOI->CurrentWeatherEffect.Clouds.CloudLayer;
	UpdateCloudsData(AOI, CloudLayer);

	FWindLayerData WindLayer = AOI->CurrentWeatherEffect.Wind.CurrentWindLayer;
	UpdateWindData(AOI, WindLayer);


	CurrentFeatureData = FScenarioEnvironmentPreset();
	AppliedPresetSnapshot = FScenarioEnvironmentPreset();
	bHasAppliedPreset = false;
	bPresetDirty = false;

	PresetMode = EPresetMode::Idle;
	return true;
}

bool UUIEnvironmentController::AddEnvironmentPreset(const FString& PresetName)
{
	FEnvironmentData* Data = GetEnvironmentData();
	if (!Data) return false;

	FAreaOfInterest* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
	if (!AOI) return false;

	const FString CleanName = PresetName.TrimStartAndEnd();
	if (CleanName.IsEmpty() || CleanName.Equals(TEXT("None"), ESearchCase::IgnoreCase))
	{
		return false;
	}

	for (const FScenarioEnvironmentPreset& P : Data->LocalScenarioEnvironment.EnvironmentPresets)
	{
		if (P.PresetId.Name.Equals(CleanName, ESearchCase::IgnoreCase))
		{
			return false;
		}
	}

	/*int32 MaxId = 0;
	for (const FScenarioEnvironmentPreset& P : Data->LocalScenarioEnvironment.EnvironmentPresets)
	{
		MaxId = FMath::Max(MaxId, P.PresetId.Id);
	}*/

	// Normalize precipitation state before saving preset
	auto& Precip = AOI->CurrentWeatherEffect.Precipitation;

	if (Precip.Type == EPrecipitationType::None)
	{
		Precip.RateMmPerHr = 0.0f;
		Precip.Intensity = EPrecipitationIntensity::None;
	}
	else
	{
		const float Rate = FMath::Clamp(Precip.RateMmPerHr, 0.0f, WeatherSim::ExtremeMaxMmPerHour);

		if (Rate <= 0.0f)
		{
			Precip.Intensity = EPrecipitationIntensity::None;
		}
		else if (Rate < WeatherSim::LightMaxMmPerHour)
		{
			Precip.Intensity = EPrecipitationIntensity::Light;
		}
		else if (Rate < WeatherSim::ModerateMaxMmPerHour)
		{
			Precip.Intensity = EPrecipitationIntensity::Moderate;
		}
		else if (Rate < WeatherSim::HeavyMaxMmPerHour)
		{
			Precip.Intensity = EPrecipitationIntensity::Heavy;
		}
		else
		{
			Precip.Intensity = EPrecipitationIntensity::Extreme;
		}
	}

	FScenarioEnvironmentPreset NewPreset;
	NewPreset.PresetId.Id = Data->LocalScenarioEnvironment.EnvironmentPresets.Num() + 1;
	NewPreset.PresetId.Name = CleanName;
	//NewPreset.PresetId.Id = MaxId + 1;
	NewPreset.WeatherEffect = AOI->CurrentWeatherEffect;
	PresetMode = EPresetMode::CreatingPreset;

	Data->LocalScenarioEnvironment.EnvironmentPresets.Add(NewPreset);
	Data->LocalScenarioEnvironment.SelectedEnvironmentPreset = NewPreset;
	AOI->CurrentEnvironmentPreset = NewPreset;

	return SaveEnvironmentPresetToXml();
}

bool UUIEnvironmentController::UpdateEnvironmentPreset(const FString& OldName, const FString& NewName)
{
	FEnvironmentData* Data = GetEnvironmentData();
	if (!Data) return false;

	FAreaOfInterest* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
	if (!AOI) return false;

	const FString CleanOld = OldName.TrimStartAndEnd();
	const FString CleanNew = NewName.TrimStartAndEnd();

	if (CleanOld.IsEmpty() || CleanNew.IsEmpty() || CleanNew.Equals(TEXT("None"), ESearchCase::IgnoreCase))
	{
		return false;
	}

	FScenarioEnvironmentPreset* Found = nullptr;
	for (FScenarioEnvironmentPreset& P : Data->LocalScenarioEnvironment.EnvironmentPresets)
	{
		if (P.PresetId.Name.Equals(CleanOld, ESearchCase::IgnoreCase))
		{
			Found = &P;
			break;
		}
	}
	if (!Found) return false;

	for (const FScenarioEnvironmentPreset& P : Data->LocalScenarioEnvironment.EnvironmentPresets)
	{
		if (&P != Found && P.PresetId.Name.Equals(CleanNew, ESearchCase::IgnoreCase))
		{
			return false;
		}
	}

	Found->PresetId.Name = CleanNew;
	Found->WeatherEffect = AOI->CurrentWeatherEffect;
	PresetMode = EPresetMode::EditingPreset;

	Data->LocalScenarioEnvironment.SelectedEnvironmentPreset = *Found;
	AOI->CurrentEnvironmentPreset = *Found;

	return SaveEnvironmentPresetToXml();
}

bool UUIEnvironmentController::RemoveEnvironmentPreset(const FString& PresetName)
{
	FEnvironmentData* Data = GetEnvironmentData();
	if (!Data) return false;

	TArray<FScenarioEnvironmentPreset>& Presets = Data->LocalScenarioEnvironment.EnvironmentPresets;
	int32 IndexToRemove = INDEX_NONE;

	for (int32 i = 0; i < Presets.Num(); ++i)
	{
		if (Presets[i].PresetId.Name.Equals(PresetName, ESearchCase::IgnoreCase))
		{
			IndexToRemove = i;
			break;
		}
	}

	if (IndexToRemove == INDEX_NONE)
	{
		return false;
	}

	Presets.RemoveAt(IndexToRemove);
	for (int32 i = 0; i < Presets.Num(); ++i)
	{
		Presets[i].PresetId.Id = i + 1;
	}

	Data->LocalScenarioEnvironment.SelectedEnvironmentPreset = FScenarioEnvironmentPreset();

	FAreaOfInterest* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
	if (AOI)
	{
		AOI->CurrentEnvironmentPreset = FScenarioEnvironmentPreset();
	}
	PresetMode = EPresetMode::Idle;

	return SaveEnvironmentPresetToXml();
}

bool UUIEnvironmentController::ApplyEnvironmentPresetByName(const FString& PresetName)
{
	FEnvironmentData* Data = GetEnvironmentData();
	if (!Data) return false;

	FAreaOfInterest* AOI = Data->LocalScenarioEnvironment.GetSelectedAreaOfInterest();
	if (!AOI) return false;

	FScenarioEnvironmentPreset* Found = nullptr;
	if (Data->LocalScenarioEnvironment.EnvironmentPresets.Num() == 0) return false;
	for (FScenarioEnvironmentPreset& P : Data->LocalScenarioEnvironment.EnvironmentPresets)
	{
		if (P.PresetId.Name.Equals(PresetName, ESearchCase::IgnoreCase))
		{
			Found = &P;
			break;
		}
	}

	if (!Found) return false;

	AppliedPresetName = Manager->GetAppliedPresetName();

	Data->LocalScenarioEnvironment.SelectedEnvironmentPreset = *Found;
	AOI->CurrentEnvironmentPreset = *Found;
	AOI->CurrentWeatherEffect = Found->WeatherEffect;

	UpdatePrecipitationData(AOI);
	UpdateVisibilityData(AOI);

	FCloudLayerData CloudLayer = AOI->CurrentWeatherEffect.Clouds.CloudLayer;
	UpdateCloudsData(AOI, CloudLayer);

	FWindLayerData WindLayer = AOI->CurrentWeatherEffect.Wind.CurrentWindLayer;
	UpdateWindData(AOI, WindLayer);
	
	CurrentFeatureData = *Found;
	AppliedPresetSnapshot = *Found;
//	bHasAppliedPreset = true;
	bPresetDirty = false;
	PresetMode = EPresetMode::ViewingPreset;

	return true;
}

void UUIEnvironmentController::BeginEditingPreset()
{
	PresetMode = EPresetMode::EditingPreset;
}

void UUIEnvironmentController::OnAnyFeatureChanged()
{
	/*if (bHasAppliedPreset)
	{
		FEnvironmentData* Data = GetEnvironmentData();
		CurrentFeatureData.WeatherEffect = Data->LocalScenarioEnvironment.SelectedAreaOfInterest->CurrentWeatherEffect;
		this->bPresetDirty = !(CurrentFeatureData == AppliedPresetSnapshot);
		 
		if(bPresetDirty)
		{
			AddNewPreset();
		}
	}*/

	if (PresetMode == EPresetMode::EditingPreset) return;
	if (PresetMode == EPresetMode::ViewingPreset)
	{
		FEnvironmentData* Data = GetEnvironmentData();
		CurrentFeatureData.WeatherEffect = Data->LocalScenarioEnvironment.SelectedAreaOfInterest->CurrentWeatherEffect;
		this->bPresetDirty = !(CurrentFeatureData == AppliedPresetSnapshot);

		if (bPresetDirty)
		{
			AddNewPreset();
		}
	}
}

void UUIEnvironmentController::AddNewPreset()
{
	if (OnPresetChanged.IsBound())
	{
		OnPresetChanged.Execute(bPresetDirty);
	}
}

FText UUIEnvironmentController::GetAppliedPresetName()
{
	//AppliedPresetName = Manager->GetAppliedPresetName();
	UE_LOG(LogTemp, Warning, TEXT("Applied Preset Name is %s"), *AppliedPresetName.ToString());
	return AppliedPresetName;
}
 
//---------------
//-----XML-------
//---------------

FString UUIEnvironmentController::GetPresetXmlPath()
{
	return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Archive/WeatherPresets.xml"));
}

bool UUIEnvironmentController::SaveEnvironmentPresetToXml()
{
	const FEnvironmentData* Data = GetEnvironmentData();
	if (!Data) return false;
	const TArray<FScenarioEnvironmentPreset>& Presets = Data->LocalScenarioEnvironment.EnvironmentPresets;
	auto B = [](bool bValue) {return bValue ? TEXT("true") : TEXT("false");};
	auto PrecipTypeToString = [](EPrecipitationType V)
		{
			switch (V)
			{
			case EPrecipitationType::Rain: return TEXT("Rain");
			case EPrecipitationType::Snow: return TEXT("Snow");
			case EPrecipitationType::Hail: return TEXT("Hail");
			default:					   return TEXT("None");
			}
		};
	auto PrecipIntensityToString = [](EPrecipitationIntensity V)
		{
			switch (V)
			{
			case EPrecipitationIntensity::Light:		return TEXT("Light");
			case EPrecipitationIntensity::Moderate:		return TEXT("Moderate");
			case EPrecipitationIntensity::Heavy:		return TEXT("Heavy");
			case EPrecipitationIntensity::Extreme:		return TEXT("Extreme");
			default:									return TEXT("None");
			}
		};
	auto VisibilityToString = [](EVisibilityObscurant V)
		{
			switch (V)
			{
			case EVisibilityObscurant::Fog:			return TEXT("Fog");
			case EVisibilityObscurant::Dust:		return TEXT("Dust");
			default:								return TEXT("None");
			}
		};
	auto CloudTypeToString = [](ECloudType V)
		{
			switch (V)
			{
			case ECloudType::Cirrocumulus:			return TEXT("Cirrocumulus");
			case ECloudType::Cirrostratus:			return TEXT("Cirrostratus");
			case ECloudType::CumulusCongestus:		return TEXT("CumulusCongestus");
			case ECloudType::PartlyCloudy:			return TEXT("PartlyCloudy");
			default:								return TEXT("CompleteClearSkies");
			}
		};
	auto CloudCoverToString = [](ECloudCover V)
		{
			switch (V)
			{
			case ECloudCover::Few:			return TEXT("Few");
			case ECloudCover::Scattered:	return TEXT("Scattered");
			case ECloudCover::Broken:		return TEXT("Broken");
			case ECloudCover::Overcast:		return TEXT("Overcast");
			default:						return TEXT("Clear");
			}
		};
	auto ThunderToString = [](EThunderOption V)
		{
			switch (V)
			{
			case EThunderOption::Light:			return TEXT("Light");
			case EThunderOption::Moderate:		return TEXT("Moderate");
			case EThunderOption::Heavy:			return TEXT("Heavy");
			default:							return TEXT("None");
			}
		};
	FString Xml;
	Xml.Reserve(8192);
	Xml += TEXT("<?xml version=\"1.0\"encoding=\"UTF-8\"?>\n");
	Xml += TEXT("<EnvironmentPresets>\n");
	for (const FScenarioEnvironmentPreset& P : Presets)
	{
		const FPrecipitationData& Precip = P.WeatherEffect.Precipitation;
		const FVisibilityData& Visibility = P.WeatherEffect.Visibility;
		const FCloudsData& Clouds = P.WeatherEffect.Clouds;
		const FCloudLayerData& CloudLayer = Clouds.CloudLayer;
		const FWindData& Wind = P.WeatherEffect.Wind;
		const FWindLayerData& CurrentWind = Wind.CurrentWindLayer;

		Xml += TEXT("\t<Preset>\n");
		Xml += FString::Printf(TEXT("\t\t<Id>%d</Id>\n"), P.PresetId.Id);
		Xml += TEXT("\t\t<Name>") + P.PresetId.Name + TEXT("</Name>\n");
		Xml += TEXT("\t\t<WeatherEffect>\n");
		Xml += TEXT("\t\t\t<Precipitation>\n");
		Xml += FString::Printf(TEXT("\t\t\t\t<Enabled>%s</Enabled>\n"), B(Precip.bEnabled));
		Xml += FString::Printf(TEXT("\t\t\t\t<Type>%s</Type>\n"), PrecipTypeToString(Precip.Type));
		Xml += FString::Printf(TEXT("\t\t\t\t<Intensity>%s</Intensity>\n"), PrecipIntensityToString(Precip.Intensity));
		Xml += FString::Printf(TEXT("\t\t\t\t<RateMmPerHr>%.6f</RateMmPerHr>\n"), Precip.RateMmPerHr);
		Xml += TEXT("\t\t\t</Precipitation>\n");
		Xml += TEXT("\t\t\t<Visibility>\n");
		Xml += FString::Printf(TEXT("\t\t\t\t<Enabled>%s</Enabled>\n"), B(Visibility.bEnabled));
		Xml += FString::Printf(TEXT("\t\t\t\t<Obscurant>%s</Obscurant>\n"), VisibilityToString(Visibility.Obscurant));
		Xml += FString::Printf(TEXT("\t\t\t\t<DistanceKm>%.6f</DistanceKm>\n"), Visibility.DistanceKm);
		Xml += TEXT("\t\t\t</Visibility>\n");
		Xml += TEXT("\t\t\t<Clouds>\n");
		Xml += FString::Printf(TEXT("\t\t\t\t<Enabled>%s</Enabled>\n"), B(Clouds.bEnabled));
		Xml += FString::Printf(TEXT("\t\t\t\t<Shadows>%s</Shadows>\n"), B(Clouds.bShadows));
		Xml += TEXT("\t\t\t\t<SelectedCloudLayerIndex>")+LexToString(Clouds.SelectedCloudLayerIndex)+("</SelectedCloudLayerIndex>\n");
		Xml += FString::Printf(TEXT("\t\t\t\t<Thunder>%s</Thunder>\n"), ThunderToString(Clouds.Thunder));
		Xml += TEXT("\t\t\t\t<CloudLayer>\n");
		Xml += FString::Printf(TEXT("\t\t\t\t\t<LayerIndex>%d</LayerIndex>\n"),CloudLayer.LayerIndex);
		Xml += FString::Printf(TEXT("\t\t\t\t\t<BaseFt>%.6f</BaseFt>\n"), CloudLayer.BaseFt);
		Xml += FString::Printf(TEXT("\t\t\t\t\t<TopFt>%.6f</TopFt>\n"), CloudLayer.TopFt);
		Xml += FString::Printf(TEXT("\t\t\t\t\t<Type>%s</Type>\n"), CloudTypeToString(CloudLayer.Type));
		Xml += FString::Printf(TEXT("\t\t\t\t\t<Cover>%s</Cover>\n"), CloudCoverToString(CloudLayer.Cover));
		Xml += TEXT("\t\t\t\t</CloudLayer>\n");
		Xml += TEXT("\t\t\t</Clouds>\n");
		Xml += TEXT("\t\t\t<Wind>\n");
		Xml += FString::Printf(TEXT("\t\t\t\t<Enabled>%s</Enabled>\n"), B(Wind.bEnabled));
		Xml += TEXT("\t\t\t\t<SelectedLayerIndex>")+LexToString(Wind.SelectedLayerIndex)+("</SelectedLayerIndex>\n");
		Xml += TEXT("\t\t\t\t<CurrentWindLayer>\n");
		Xml += FString::Printf(TEXT("\t\t\t\t\t<LayerIndex>%d</LayerIndex>\n"),CurrentWind.LayerIndex);
		Xml += FString::Printf(TEXT("\t\t\t\t\t<AltitudeFt>%.6f</AltitudeFt>\n"), CurrentWind.AltitudeFt);
		Xml += FString::Printf(TEXT("\t\t\t\t\t<DirectionDeg>%.6f</DirectionDeg>\n"), CurrentWind.DirectionDeg);
		Xml += FString::Printf(TEXT("\t\t\t\t\t<SpeedKt>%.6f</SpeedKt>\n"), CurrentWind.SpeedKt);
		Xml += FString::Printf(TEXT("\t\t\t\t\t<Whistling>%s</Whistling>\n"), B(CurrentWind.bWhistling));
		Xml += FString::Printf(TEXT("\t\t\t\t\t<DownWindDirectionDeg>%.6f</DownWindDirectionDeg>\n"), CurrentWind.DownwindDirectionDeg);
		Xml += TEXT("\t\t\t\t</CurrentWindLayer>\n");
		Xml += TEXT("\t\t\t</Wind>\n");
		Xml += TEXT("\t\t</WeatherEffect>\n");
		Xml += TEXT("\t</Preset>\n");
	}
	Xml += TEXT("</EnvironmentPresets>\n");
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(GetPresetXmlPath()), true);
	return FFileHelper::SaveStringToFile(Xml, *GetPresetXmlPath());
}

bool UUIEnvironmentController::LoadEnvironmentPresetsFromXml()
{
	FEnvironmentData* Data = GetEnvironmentData();
	if (!Data) return false;
	Data->LocalScenarioEnvironment.EnvironmentPresets.Reset();
	const FString FilePath = GetPresetXmlPath();
	UE_LOG(LogTemp, Warning, TEXT("Presets XML Path = %s"), *GetPresetXmlPath());
	if (!FPaths::FileExists(FilePath)) return true;
	FXmlFile XmlFile(FilePath, EConstructMethod::ConstructFromFile);
	if (!XmlFile.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("XML Invalid = %s"), *XmlFile.GetLastError());
		return false;
	}
	const FXmlNode* Root = XmlFile.GetRootNode();
	if (!Root || Root->GetTag() != TEXT("EnvironmentPresets")) return false;
	auto GetChildText = [](const FXmlNode* Parent, const TCHAR* Tag, const FString& Default = TEXT(""))->FString
		{
			if (!Parent) return Default;
			const FXmlNode* Child = Parent->FindChildNode(Tag);
			return Child ? Child->GetContent() : Default;
		};
	auto ToBool = [](const FString& S)-> bool
		{
			return S.Equals(TEXT("true"), ESearchCase::IgnoreCase) || S == TEXT("1");
		};
	auto ToInt = [](const FString& S, float Default = 0.0f)->float
		{
			return S.IsEmpty() ? Default : FCString::Atoi(*S);
		};
	auto ToFloat = [](const FString& S, float Default = 0.0f)->float
		{
			return S.IsEmpty() ? Default : FCString::Atof(*S);
		};
	auto StringToPrecipType = [](const FString& S)
		{
			if (S.Equals(TEXT("Rain"), ESearchCase::IgnoreCase))	return EPrecipitationType::Rain;
			if (S.Equals(TEXT("Snow"), ESearchCase::IgnoreCase))	return EPrecipitationType::Snow;
			if (S.Equals(TEXT("Hail"), ESearchCase::IgnoreCase))	return EPrecipitationType::Hail;
			return EPrecipitationType::None;
		};
	auto StringToPrecipIntensity = [](const FString& S)
		{
			if (S.Equals(TEXT("Light"), ESearchCase::IgnoreCase))		return EPrecipitationIntensity::Light;
			if (S.Equals(TEXT("Moderate"), ESearchCase::IgnoreCase))	return EPrecipitationIntensity::Moderate;
			if (S.Equals(TEXT("Heavy"), ESearchCase::IgnoreCase))		return EPrecipitationIntensity::Heavy;
			if (S.Equals(TEXT("Extreme"), ESearchCase::IgnoreCase))		return EPrecipitationIntensity::Extreme;
			return EPrecipitationIntensity::None;
		};
	auto StringToVisbility = [](const FString& S)
		{
			if (S.Equals(TEXT("Fog"), ESearchCase::IgnoreCase))		return EVisibilityObscurant::Fog;
			if (S.Equals(TEXT("Dust"), ESearchCase::IgnoreCase))	return EVisibilityObscurant::Dust;
			return EVisibilityObscurant::None;
		};
	auto StringToCloudType = [](const FString& S)
		{
			if (S.Equals(TEXT("Cirrocumulus"), ESearchCase::IgnoreCase))		return ECloudType::Cirrocumulus;
			if (S.Equals(TEXT("Cirrostratus"), ESearchCase::IgnoreCase))		return ECloudType::Cirrostratus;
			if (S.Equals(TEXT("CumulusCongestus"), ESearchCase::IgnoreCase))	return ECloudType::CumulusCongestus;
			if (S.Equals(TEXT("PartlyCloudy"), ESearchCase::IgnoreCase))		return ECloudType::PartlyCloudy;
			return ECloudType::CompleteClearSkies;
		};
	auto StringToCloudCover = [](const FString& S)
		{
			if (S.Equals(TEXT("Few"), ESearchCase::IgnoreCase))				return ECloudCover::Few;
			if (S.Equals(TEXT("Scattered"), ESearchCase::IgnoreCase))		return ECloudCover::Scattered;
			if (S.Equals(TEXT("Broken"), ESearchCase::IgnoreCase))			return ECloudCover::Broken;
			if (S.Equals(TEXT("Overcast"), ESearchCase::IgnoreCase))		return ECloudCover::Overcast;
			return ECloudCover::Clear;
		};
	auto StringToThunder = [](const FString& S)
		{
			if (S.Equals(TEXT("Light"), ESearchCase::IgnoreCase))		return EThunderOption::Light;
			if (S.Equals(TEXT("Moderate"), ESearchCase::IgnoreCase))	return EThunderOption::Moderate;
			if (S.Equals(TEXT("Heavy"), ESearchCase::IgnoreCase))		return EThunderOption::Heavy;
			return EThunderOption::None;
		};
	for (const FXmlNode* PresetNode : Root->GetChildrenNodes())
	{
		if (!PresetNode || PresetNode->GetTag() != TEXT("Preset")) continue;
		FScenarioEnvironmentPreset Preset;
		Preset.PresetId.Id = ToInt(GetChildText(PresetNode, TEXT("Id")));
		Preset.PresetId.Name = GetChildText(PresetNode, TEXT("Name"));
		const FXmlNode* WeatherNode = PresetNode->FindChildNode(TEXT("WeatherEffect"));
		if (WeatherNode)
		{
			if (const FXmlNode* PrecipNode = WeatherNode->FindChildNode(TEXT("Precipitation")))
			{
				Preset.WeatherEffect.Precipitation.bEnabled = ToBool(GetChildText(PrecipNode, TEXT("Enabled"), TEXT("true")));
				Preset.WeatherEffect.Precipitation.Type = StringToPrecipType(GetChildText(PrecipNode, TEXT("Type"), TEXT("None")));
				Preset.WeatherEffect.Precipitation.Intensity = StringToPrecipIntensity(GetChildText(PrecipNode, TEXT("Intensity"), TEXT("None")));
				Preset.WeatherEffect.Precipitation.RateMmPerHr = ToFloat(GetChildText(PrecipNode, TEXT("RateMmPerHr")));
			}
			if (const FXmlNode* VisbilityNode = WeatherNode->FindChildNode(TEXT("Visibility")))
			{
				Preset.WeatherEffect.Visibility.bEnabled = ToBool(GetChildText(VisbilityNode, TEXT("Enabled"), TEXT("true")));
				Preset.WeatherEffect.Visibility.Obscurant = StringToVisbility(GetChildText(VisbilityNode, TEXT("Obscurant"), TEXT("None")));
				Preset.WeatherEffect.Visibility.DistanceKm = ToFloat(GetChildText(VisbilityNode, TEXT("DistanceKm")));
			}
			if (const FXmlNode* CloudNode = WeatherNode->FindChildNode(TEXT("Clouds")))
			{
				Preset.WeatherEffect.Clouds.bEnabled = ToBool(GetChildText(CloudNode, TEXT("Enabled"), TEXT("true")));
				Preset.WeatherEffect.Clouds.bShadows = ToBool(GetChildText(CloudNode, TEXT("Shadows"), TEXT("false")));
				Preset.WeatherEffect.Clouds.SelectedCloudLayerIndex = ToInt(GetChildText(CloudNode, TEXT("SelectedCloudLayerIndex"), TEXT("-1")));
				Preset.WeatherEffect.Clouds.Thunder = StringToThunder(GetChildText(CloudNode, TEXT("Thunder"), TEXT("None")));
				if (const FXmlNode* CloudLayerNode = CloudNode->FindChildNode(TEXT("CloudLayer")))
				{
					FCloudLayerData& CloudLayer = Preset.WeatherEffect.Clouds.CloudLayer;
					CloudLayer.LayerIndex = ToInt(GetChildText(CloudLayerNode, TEXT("LayerIndex"), TEXT("true")));
					CloudLayer.BaseFt = ToFloat(GetChildText(CloudLayerNode, TEXT("BaseFt"), TEXT("3000")));
					CloudLayer.TopFt = ToFloat(GetChildText(CloudLayerNode, TEXT("TopFt"), TEXT("8000")));
					CloudLayer.Type = StringToCloudType(GetChildText(CloudLayerNode, TEXT("Type"), TEXT("CompleteClearSkies")));
					CloudLayer.Cover = StringToCloudCover(GetChildText(CloudLayerNode, TEXT("Cover"), TEXT("Clear")));
				}
			}
			if (const FXmlNode* WindNode = WeatherNode->FindChildNode(TEXT("Wind")))
			{
				Preset.WeatherEffect.Wind.bEnabled = ToBool(GetChildText(WindNode, TEXT("Enabled"), TEXT("true")));
				Preset.WeatherEffect.Wind.SelectedLayerIndex = ToInt(GetChildText(WindNode, TEXT("SelectedLayerIndex"), TEXT("-1")));
				if (const FXmlNode* CurrentWindNode = WindNode->FindChildNode(TEXT("CurrentWindLayer")))
				{
					FWindLayerData& WindLayer = Preset.WeatherEffect.Wind.CurrentWindLayer;
					WindLayer.LayerIndex = ToInt(GetChildText(CurrentWindNode, TEXT("LayerIndex"), TEXT("1")));
					WindLayer.AltitudeFt = ToFloat(GetChildText(CurrentWindNode, TEXT("AltitudeFt"), TEXT("0")));
					WindLayer.DirectionDeg = ToFloat(GetChildText(CurrentWindNode, TEXT("DirectionDeg"), TEXT("270")));
					WindLayer.SpeedKt = ToFloat(GetChildText(CurrentWindNode, TEXT("SpeedKt"), TEXT("0")));
					WindLayer.bWhistling = ToBool(GetChildText(CurrentWindNode, TEXT("Whistling"), TEXT("false")));
					WindLayer.DownwindDirectionDeg = ToFloat(GetChildText(CurrentWindNode, TEXT("DownWindDirectionDeg"), TEXT("270")));
				}
			}
		}
		if (!Preset.PresetId.Name.IsEmpty() && !Preset.PresetId.Name.Equals(TEXT("None"), ESearchCase::IgnoreCase))
		{
			Data->LocalScenarioEnvironment.EnvironmentPresets.Add(Preset);
		}
	}
	return true;
}
