#pragma once
#include "CoreMinimal.h"
#include "EnvironmentData.generated.h"


UENUM(BlueprintType)
enum class ETimeControlMode : uint8
{
	Manual   UMETA(DisplayName = "Manual"),
	System   UMETA(DisplayName = "System"),
	Animated UMETA(DisplayName = "Animated")
};

enum class ENavButton : uint8
{
	TimeofDay,
	Weather,
	Atmpsphere,
	LightSettings
};

USTRUCT(BlueprintType)
struct FEnvPresetId
{
	GENERATED_BODY()

	FString Name;
	int32 Id;

	bool operator==(const FEnvPresetId& Other) const
	{
		return Name == Other.Name && Id == Other.Id;
	}


};

UENUM(BlueprintType)
enum class ETimeSpeedMode : uint8
{
	RealTime UMETA(DisplayName = "Real Time"),
	X2       UMETA(DisplayName = "20x"),
	X3       UMETA(DisplayName = "30x"),
	X5       UMETA(DisplayName = "50x"),
	X8       UMETA(DisplayName = "80x"),
	X10      UMETA(DisplayName = "100x"),

};

UENUM(BlueprintType)
enum class EAOIGeometry : uint8
{
	Box    = 0 UMETA(DisplayName = "Box"),
	Circle = 1 UMETA(DisplayName = "Circle"),
	Rectangle = 2 UMETA(DisplayName = "Rectangle")
};



UENUM(BlueprintType)
enum class EMoonPhase : uint8
{
	NewMoon         UMETA(DisplayName = "New Moon"),
	Wanning  UMETA(DisplayName = "Wanning"),
	Waxing    UMETA(DisplayName = "Waxing"),
	FullMoon        UMETA(DisplayName = "Full Moon"),
};

USTRUCT(BlueprintType)
struct FTime
{
	GENERATED_BODY()

	int32 Hour = 0;
	int32 Min = 0;

};

USTRUCT(BlueprintType)
struct FTimeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category ="TimeSettings")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeSettings")
	ETimeControlMode ControlMode = ETimeControlMode::Manual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeSettings")
	ETimeSpeedMode SpeedMode = ETimeSpeedMode::RealTime;

	// Current effective date
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeSettings")
	FDateTime Date;

	// Current effective time 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeSettings")
	FDateTime Time;

	bool CompleteCycle = false;
	// Used only as persisted manual baseline when mode = Manual,
	// and also as fallback when System/Animated are turned OFF.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeSettings")
	FDateTime ManualDateTime;

	FTime time;
};

USTRUCT(BlueprintType)
struct FLightSettings
{
	GENERATED_BODY()

	FDateTime DawnTime = FDateTime(2000, 1, 1, 6, 00, 0); 
	FDateTime DuskTime = FDateTime(2000, 1, 1, 18, 00, 0); 
	EMoonPhase MoonPhase = EMoonPhase::FullMoon;
	float GetDawnHours() const
	{
		return DawnTime.GetHour() + DawnTime.GetMinute() / 60.0f;
	}

	float GetDuskHours() const
	{
		return DuskTime.GetHour() + DuskTime.GetMinute() / 60.0f;
	}
};


USTRUCT(BlueprintType)
struct FTimeOfDayData
{
	GENERATED_BODY()

	FTimeSettings TimeSettings;
	FLightSettings LightSettings;
	bool bIsNight = false;
};


USTRUCT(BlueprintType)
struct FLonLat
{
	GENERATED_BODY()

	double Lon = 0.0;
	double Lat = 0.0;
};


UENUM(BlueprintType)
enum class EPrecipitationType : uint8
{
	None  UMETA(DisplayName = "None"),
	Rain  UMETA(DisplayName = "Rain"),
	Snow  UMETA(DisplayName = "Snow"),
	Hail  UMETA(DisplayName = "Hail")
};

UENUM(BlueprintType)
enum class EPrecipitationIntensity : uint8
{
	None = 0 UMETA(DisplayName = "None"),
	Light    UMETA(DisplayName = "Light"),
	Moderate UMETA(DisplayName = "Moderate"),
	Heavy    UMETA(DisplayName = "Heavy"),
	Extreme    UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct FPrecipitationData
{
	GENERATED_BODY()

	bool bEnabled = true;

	EPrecipitationType Type = EPrecipitationType::None;
	EPrecipitationIntensity Intensity = EPrecipitationIntensity::Light;

	float RateMmPerHr = 0.0f;

	bool operator==(const FPrecipitationData& Other) const
	{
		return bEnabled == Other.bEnabled &&
			Type == Other.Type &&
			Intensity == Other.Intensity &&
			RateMmPerHr == Other.RateMmPerHr;
	}

};

UENUM(BlueprintType)
enum class EVisibilityObscurant : uint8
{
	None  UMETA(DisplayName = "None"),
	Fog   UMETA(DisplayName = "Fog"),
	Haze   UMETA(DisplayName = "Haze"),
	Dust  UMETA(DisplayName = "Dust")
};

USTRUCT(BlueprintType)
struct FVisibilityData
{
	GENERATED_BODY()

	bool bEnabled = true;
	EVisibilityObscurant Obscurant = EVisibilityObscurant::None;
	float DistanceKm = 0.0f;

	bool operator==(const FVisibilityData& Other) const
	{
		return bEnabled == Other.bEnabled &&
			Obscurant == Other.Obscurant &&
			DistanceKm == Other.DistanceKm;
	}
};

UENUM(BlueprintType)
enum class ECloudType : uint8
{
	Cirrocumulus  UMETA(DisplayName = "Cirrocumulus"),
	Cirrostratus UMETA(DisplayName = "Cirrostratus"),
	CumulusCongestus UMETA(DisplayName = "CumulusCongestus"),
	CompleteClearSkies  UMETA(DisplayName = "CompleteClearSkies"),
	PartlyCloudy  UMETA(DisplayName = "PartlyCloudy")
};

UENUM(BlueprintType)
enum class ECloudCover : uint8
{
	Clear     UMETA(DisplayName = "Clear"),
	Few       UMETA(DisplayName = "Few"),
	Scattered UMETA(DisplayName = "Scattered"),
	Broken    UMETA(DisplayName = "Broken"),
	Overcast  UMETA(DisplayName = "Overcast")
};

UENUM(BlueprintType)
enum class EThunderOption : uint8
{
	None     UMETA(DisplayName = "None"),
	Light    UMETA(DisplayName = "Light"),
	Moderate UMETA(DisplayName = "Moderate"),
	Heavy    UMETA(DisplayName = "Heavy")
};

USTRUCT(BlueprintType)
struct FCloudLayerData
{
	GENERATED_BODY()

	int32 LayerIndex = 1;

	float BaseFt = 3000.0f;
	float TopFt = 8000.0f;

	ECloudType Type = ECloudType::CompleteClearSkies;
	ECloudCover Cover = ECloudCover::Clear;

	bool operator == (const FCloudLayerData& Other) const {
		return LayerIndex == Other.LayerIndex &&
			FMath::IsNearlyEqual(BaseFt, Other.BaseFt) &&
			FMath::IsNearlyEqual(TopFt, Other.TopFt) &&
			Type == Other.Type &&
			Cover == Other.Cover;
	}


};

USTRUCT(BlueprintType)
struct FCloudsData
{
	GENERATED_BODY()

	bool bEnabled = true;

	bool bShadows = false;

	int32 SelectedCloudLayerIndex = INDEX_NONE;

	EThunderOption Thunder = EThunderOption::None;

	TArray<FCloudLayerData> Layers;

	FCloudLayerData CloudLayer;

	bool AddCloudLayer(const FCloudLayerData& Layer)
	{
		Layers.Add(Layer);

		// Auto-select first layer if none selected
		if (SelectedCloudLayerIndex == INDEX_NONE)
		{
			SelectedCloudLayerIndex = 0;
		}
		return true;
	}

	int32 GetCloudLayerCount() const
	{
		return Layers.Num();
	}

	FCloudLayerData* GetCloudLayer(int32 Index) 
	{
		return Layers.IsValidIndex(Index) ? &Layers[Index] : nullptr;
	}

	bool UpdateCloudLayer(int32 Index, const FCloudLayerData& UpdatedLayer)
	{
		if (!Layers.IsValidIndex(Index))
			return false;

		Layers[Index] = UpdatedLayer;
		return true;
	}

	bool RemoveCloudLayer(int32 Index)
	{
		if (!Layers.IsValidIndex(Index))
			return false;

		Layers.RemoveAt(Index);

		// Fix selection
		if (SelectedCloudLayerIndex == Index)
		{
			SelectedCloudLayerIndex = Layers.Num() > 0 ? 0 : INDEX_NONE;
		}
		else if (SelectedCloudLayerIndex > Index)
		{
			--SelectedCloudLayerIndex;
		}

		return true;
	}


	const FCloudLayerData* GetSelectedCloudLayer() const
	{
		return Layers.IsValidIndex(SelectedCloudLayerIndex)
			? &Layers[SelectedCloudLayerIndex]
			: nullptr;
	}

	bool SetSelectedCloudLayerByIndex(int32 Index)
	{
		if (!Layers.IsValidIndex(Index))
			return false;

		SelectedCloudLayerIndex = Index;
		return true;
	}

	bool HasSelectedCloudLayer() const
	{
		return Layers.IsValidIndex(SelectedCloudLayerIndex);
	}

	void ClearAllCloudLayers()
	{
		Layers.Reset();
		SelectedCloudLayerIndex = INDEX_NONE;
	}

	bool operator==(const FCloudsData& Other) const
	{
		if (bEnabled != Other.bEnabled) return false;
		if (bShadows != Other.bShadows) return false;
		if (SelectedCloudLayerIndex != Other.SelectedCloudLayerIndex) return false;
		if (Thunder != Other.Thunder) return false;
		if (!(CloudLayer == Other.CloudLayer)) return false;

		if (Layers.Num() != Other.Layers.Num()) return false;

		for (int32 i = 0; i < Layers.Num(); ++i)
		{
			if (!(Layers[i] == Other.Layers[i])) return false;
		}

		return true;
	}

};


USTRUCT(BlueprintType)
struct FWindLayerData
{
	GENERATED_BODY()

	int32 LayerIndex = 1;
	float AltitudeFt = 0.0f;
	float DirectionDeg = 270.0f;
	float SpeedKt = 12.0f;
	bool bWhistling = false;
	float DownwindDirectionDeg = 270.0f;


	bool operator == (const FWindLayerData& Other) const {
		return LayerIndex == Other.LayerIndex &&
			FMath::IsNearlyEqual(AltitudeFt, Other.AltitudeFt) &&
			FMath::IsNearlyEqual(DirectionDeg, Other.DirectionDeg) &&
			FMath::IsNearlyEqual(SpeedKt, Other.SpeedKt) &&
			bWhistling == Other.bWhistling &&
			FMath::IsNearlyEqual(DownwindDirectionDeg, Other.DownwindDirectionDeg);
	}

};



USTRUCT(BlueprintType)
struct FWindData
{
	GENERATED_BODY()

	bool bEnabled = true;

	int32 SelectedLayerIndex = INDEX_NONE;

	TArray<FWindLayerData> Layers;
	FWindLayerData CurrentWindLayer;



	bool AddWindLayer(const FWindLayerData& Layer)
	{
		Layers.Add(Layer);

		// Auto-select first layer
		if (SelectedLayerIndex == INDEX_NONE)
		{
			SelectedLayerIndex = 0;
		}
		return true;
	}

	int32 GetWindLayerCount() const
	{
		return Layers.Num();
	}

	FWindLayerData* GetWindLayer(int32 Index)
	{
		return Layers.IsValidIndex(Index) ? &Layers[Index] : nullptr;
	}

	bool UpdateWindLayer(int32 Index, const FWindLayerData& UpdatedLayer)
	{
		if (!Layers.IsValidIndex(Index))
			return false;

		Layers[Index] = UpdatedLayer;
		return true;
	}

	bool RemoveWindLayer(int32 Index)
	{
		if (!Layers.IsValidIndex(Index))
			return false;

		Layers.RemoveAt(Index);

		// Fix selection
		if (SelectedLayerIndex == Index)
		{
			SelectedLayerIndex = Layers.Num() > 0 ? 0 : INDEX_NONE;
		}
		else if (SelectedLayerIndex > Index)
		{
			--SelectedLayerIndex;
		}

		return true;
	}

	const FWindLayerData* GetSelectedWindLayer() const
	{
		return Layers.IsValidIndex(SelectedLayerIndex)
			? &Layers[SelectedLayerIndex]
			: nullptr;
	}

	bool SetSelectedWindLayerByIndex(int32 Index)
	{
		if (!Layers.IsValidIndex(Index))
			return false;

		SelectedLayerIndex = Index;
		return true;
	}

	bool HasSelectedWindLayer() const
	{
		return Layers.IsValidIndex(SelectedLayerIndex);
	}

	void ClearAllWindLayers()
	{
		Layers.Reset();
		SelectedLayerIndex = INDEX_NONE;
	}

	bool operator==(const FWindData& Other) const
	{
		if (bEnabled != Other.bEnabled) return false;
		if (SelectedLayerIndex != Other.SelectedLayerIndex) return false;
		if (!(CurrentWindLayer == Other.CurrentWindLayer)) return false;

		if (Layers.Num() != Other.Layers.Num()) return false;

		for (int32 i = 0; i < Layers.Num(); ++i)
		{
			if (!(Layers[i] == Other.Layers[i])) return false;
		}

		return true;
	}

};




USTRUCT(BlueprintType)
struct FWeatherEffectsData
{
	GENERATED_BODY()

	FPrecipitationData Precipitation;
	FVisibilityData Visibility;
	FCloudsData Clouds;
	FWindData Wind;

	bool operator==(const FWeatherEffectsData& Other) const
	{
		return Precipitation == Other.Precipitation &&
			Visibility == Other.Visibility &&
			Clouds == Other.Clouds &&
			Wind == Other.Wind;
	}

};


USTRUCT(BlueprintType)
struct FAtmosphereData
{
	GENERATED_BODY()

	bool bEnabled = true;
	float Humidity = 0.0f;
	float PressureHpa = 1013.0f;
	float TemperatureC = 18.5f;
	float TempGradientCPerKm = -6.5f;
	float InversionLayerCode = 1.0f;

	bool operator==(const FAtmosphereData& Other) const
	{
		return bEnabled == Other.bEnabled &&
			Humidity == Other.Humidity &&
			PressureHpa == Other.PressureHpa &&
			TemperatureC == Other.TemperatureC &&
			TempGradientCPerKm == Other.TempGradientCPerKm &&
			InversionLayerCode == Other.InversionLayerCode;
	}

};

UENUM(BlueprintType)
enum class EIcingType : uint8
{
	None  UMETA(DisplayName = "None"),
	Rime  UMETA(DisplayName = "Rime"),
	Clear UMETA(DisplayName = "Clear"),
	Mixed UMETA(DisplayName = "Mixed")
};

UENUM(BlueprintType)
enum class EIcingSeverity : uint8
{
	None     UMETA(DisplayName = "None"),
	Light    UMETA(DisplayName = "Light"),
	Moderate UMETA(DisplayName = "Moderate"),
	Severe   UMETA(DisplayName = "Severe")
};

USTRUCT(BlueprintType)
struct FIcingData
{
	GENERATED_BODY()

	bool bEnabled = true;
	EIcingType Type = EIcingType::None;
	EIcingSeverity Severity = EIcingSeverity::None;

	bool operator==(const FIcingData& Other) const
	{
		return bEnabled == Other.bEnabled &&
			Type == Other.Type &&
			Severity == Other.Severity;
	}
};

USTRUCT(BlueprintType)
struct FAtmosphereAndIcingData
{
	GENERATED_BODY()

	FAtmosphereData Atmosphere;
	FIcingData Icing;
	bool operator==(const FAtmosphereAndIcingData& Other) const
	{
		return Atmosphere == Other.Atmosphere && Icing == Other.Icing;
	}
};

USTRUCT(BlueprintType)
struct FScenarioEnvironmentPreset
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ScenarioEnvironmentPreset")
	FEnvPresetId PresetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "ScenarioEnvironmentPreset")
	FWeatherEffectsData WeatherEffect;

	bool operator==(const FScenarioEnvironmentPreset& Other) const
	{
		return PresetId == Other.PresetId && WeatherEffect == Other.WeatherEffect;
	}
};

USTRUCT(BlueprintType)
struct FAreaOfInterest
{
	GENERATED_BODY()

	FString Key;
	EAOIGeometry AreaShape;
	FScenarioEnvironmentPreset CurrentEnvironmentPreset;
	FWeatherEffectsData CurrentWeatherEffect;
	FAtmosphereAndIcingData AtmosphereAndIcingData;

	virtual FString GetAreaType() const PURE_VIRTUAL(FAreaOfInterest::GetAreaType, return TEXT(""););
	virtual FString GetLabel() const PURE_VIRTUAL(FAreaOfInterest::GetLabel, return TEXT(""););

	bool operator == (const FAreaOfInterest& Other) const {
		return Key == Other.Key &&
			AreaShape == Other.AreaShape &&
			CurrentEnvironmentPreset == Other.CurrentEnvironmentPreset &&
			CurrentWeatherEffect == Other.CurrentWeatherEffect &&
			AtmosphereAndIcingData == Other.AtmosphereAndIcingData;
	}


};

USTRUCT(BlueprintType)
struct FBoundingSphere: public FAreaOfInterest
{
	GENERATED_BODY()

	double radius;
	FVector Center;

	FBoundingSphere() {
		AreaShape = EAOIGeometry::Circle;
	}

	virtual FString GetAreaType() const override {
		return TEXT("BoundingSphere");
	}

	virtual FString GetLabel() const override {
		return FString::Printf(TEXT("Bounding Sphere: %s (%.2f) "), *Key, radius);
	}

	bool operator == (const FBoundingSphere& Other) const {
		return FAreaOfInterest::operator == (Other) &&
			radius == Other.radius &&
			Center == Other.Center;
	}
};

USTRUCT(BlueprintType)
struct FRectangularRecord_1 : public FAreaOfInterest
{
	GENERATED_BODY()


	FLonLat TopRight;
	FLonLat BottomLeft;

	FRectangularRecord_1() {
		AreaShape = EAOIGeometry::Rectangle;
	}

	virtual FString GetAreaType() const override {
		return TEXT("RectangularRecord_1");
	}
	virtual FString GetLabel() const override {
		return FString::Printf(TEXT("Rectangle: %s (%.2f,%.2f ->%.2f, %.2f "), *Key, TopRight.Lat, TopRight.Lon, BottomLeft.Lat,BottomLeft.Lon);
	}
	bool operator == (const FRectangularRecord_1& Other) const {
		return FAreaOfInterest::operator == (Other) &&
			TopRight.Lon == Other.TopRight.Lon &&
			TopRight.Lat == Other.TopRight.Lat &&
			BottomLeft.Lat == Other.BottomLeft.Lat &&
			BottomLeft.Lon == Other.BottomLeft.Lon;
	}

};





USTRUCT(BlueprintType)
struct FScenarioEnvironmentData
{
	GENERATED_BODY()

	//TArray<FAreaOfInterest> AreaOfInterestPresets;

	TMap<FString, FAreaOfInterest*> AreaOfInterestPresets;
	FAreaOfInterest* SelectedAreaOfInterest;


	TArray<FScenarioEnvironmentPreset> EnvironmentPresets;
	FScenarioEnvironmentPreset SelectedEnvironmentPreset;

	bool AddAreaOfInterest(FAreaOfInterest* AOI)
	{
		if (AreaOfInterestPresets.Contains(AOI->Key)) {
			return false;
		}
		AreaOfInterestPresets.Add(AOI->Key, AOI);
		return true;
	}

	FAreaOfInterest* GetAreaOfInterest(FString key)
	{
		if (AreaOfInterestPresets.Contains(key)) {
			return AreaOfInterestPresets[key];
		}
		return nullptr;
	}

	bool RemoveAreaOfInterest(FString key)
	{
		if (!AreaOfInterestPresets.Contains(key)) return false;
		AreaOfInterestPresets.Remove(key);
		return true;
	}

	bool UpdateAreaOfInterest(FAreaOfInterest* AOI) {
		if (AreaOfInterestPresets.Contains(AOI->Key)) {
			AreaOfInterestPresets[AOI->Key] = AOI;
			return true;
		}
		return false;
	}

	int32 GetAreaOfInterestCount() const { return AreaOfInterestPresets.Num(); }

	FAreaOfInterest* GetSelectedAreaOfInterest() const { return SelectedAreaOfInterest; }

	const TMap<FString, FAreaOfInterest*>& GetAreaOfInterestPresets() const
	{
		return AreaOfInterestPresets;
	}


	bool SetSelectedAreaOfInterestByKey(FString key)
	{
		if (FAreaOfInterest* FoundAOI = GetAreaOfInterest(key)) {
			SelectedAreaOfInterest = FoundAOI;
			return true;
		}
		return false;
	}

	bool SetEnvPresetForSelectedAreaOfInterestByKey(int PresetID)
	{
		if (FAreaOfInterest* AOI = SelectedAreaOfInterest) {
			if (FScenarioEnvironmentPreset* FoundPreset = GetEnvironmentPreset(PresetID)) {
				AOI->CurrentEnvironmentPreset = *FoundPreset;
				return true;
			}

		}
		return false;
	}

	bool AddEnvironmentPreset(const FScenarioEnvironmentPreset& NewPreset)
	{

		EnvironmentPresets.Add(NewPreset);
		SelectedEnvironmentPreset = NewPreset;

		return true;
	}

	bool EditSelectedEnvironmentPreset()
	{
		for (FScenarioEnvironmentPreset& Preset : EnvironmentPresets)
		{
			if (Preset.PresetId.Id == SelectedEnvironmentPreset.PresetId.Id)
			{
				Preset.WeatherEffect = SelectedAreaOfInterest->CurrentWeatherEffect;
				SelectedEnvironmentPreset.WeatherEffect = SelectedAreaOfInterest->CurrentWeatherEffect;
				return true;
			}
		}
		return false;
	}

	FScenarioEnvironmentPreset* GetEnvironmentPreset(int32 Index)
	{
		return EnvironmentPresets.IsValidIndex(Index) ? &EnvironmentPresets[Index] : nullptr;
	}

	FScenarioEnvironmentPreset* FindEnvironmentPresetById(int32 PresetId)
	{
		return EnvironmentPresets.FindByPredicate([&](const FScenarioEnvironmentPreset& P) { return P.PresetId.Id == PresetId; });
	}

	const FScenarioEnvironmentPreset* GetSelectedEnvironmentPreset() const
	{
		return &SelectedEnvironmentPreset;
	}

	bool SetSelectedEnvironmentPresetById(int32 PresetId)
	{
		if (FScenarioEnvironmentPreset* Found = FindEnvironmentPresetById(PresetId))
		{
			SelectedEnvironmentPreset = *Found;
			SelectedAreaOfInterest->CurrentWeatherEffect = Found->WeatherEffect;
			return true;
		}
		return false;
	}

	bool SetSelectedEnvironmentPresetByIndex(int32 Index)
	{
		if (!EnvironmentPresets.IsValidIndex(Index)) return false;
		SelectedEnvironmentPreset = EnvironmentPresets[Index];
		SelectedAreaOfInterest->CurrentWeatherEffect = EnvironmentPresets[Index].WeatherEffect;
		return true;
	}

	bool RemoveEnvironmentPresetById(int32 PresetId)
	{
		const int32 Removed = EnvironmentPresets.RemoveAll([&](const FScenarioEnvironmentPreset& P) { return P.PresetId.Id == PresetId; });
		return Removed > 0;
	}
};


USTRUCT(BlueprintType)
struct FAmbientCube
{
	GENERATED_BODY()

	float AmbientLightIntensity = 25.0f;
};

USTRUCT(BlueprintType)
struct FShadows
{
	GENERATED_BODY()

	bool bIsShadows = true;
};

USTRUCT(BlueprintType)
struct FOthersData
{
	GENERATED_BODY()

	FAmbientCube AmbientSettings;
	FShadows Shadows;
};

USTRUCT(BlueprintType)
struct FEnvironmentData
{
	GENERATED_BODY()

	FTimeOfDayData TimeOfDay;
	//FScenarioEnvironmentData ScenarioEnvironment;
	FScenarioEnvironmentData LocalScenarioEnvironment;
	FScenarioEnvironmentData NetworkScenarioEnvironment;
	FScenarioEnvironmentData* CurrentScenarioEnvironment;

	FScenarioEnvironmentData GetCurrentScenarioEnv() { return *CurrentScenarioEnvironment; };
	void SetCurrentScenarioEnv(FScenarioEnvironmentData& Env) {
		CurrentScenarioEnvironment = &Env;
	};

	FOthersData Others;
	ENavButton SelectedNavButton = ENavButton::TimeofDay;
	//const FAreaOfInterest* GetSelectedAreaOfInterest() const
	//{
	//	return ScenarioEnvironment.GetSelectedAreaOfInterest();
	//}

	//const FScenarioEnvironmentPreset* GetSelectedEnvironmentPreset() const
	//{
	//	return ScenarioEnvironment.GetSelectedEnvironmentPreset();
	//}

	//const FWeatherEffectsData* GetCurrentWeatherEffect() 
	//{
	//	return &ScenarioEnvironment.GetSelectedAreaOfInterest()->CurrentWeatherEffect;
	//}
};
