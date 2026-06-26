// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EnvironmentData.h"
#include "EnvironmentAdapter.generated.h"


UENUM(BlueprintType)
enum class EUDSColorMode : uint8
{
    SkyAtmosphere   UMETA(DisplayName = "Sky Atmosphere"),
    SimplifiedColor UMETA(DisplayName = "Simplified Color"),
};


USTRUCT(BlueprintType)
struct FEnvironmentNormalizedData
{
	GENERATED_BODY()
    //Sky Related Entities
    float TimeOfDay = 960.0f; 
    int32 Day = 15;
    int32 Month = 2;
    int32 Year = 2026;
    float DuskTime = 0.0f;
    float DawnTime = 0.0f; 
    float TimeSpeed= 0.0f; 
    bool SimulateRealSun = false;
    bool SimulateRealMoon = false;
    bool AnimateTimeOfDay= false; 
    float MoonPhase= 0.0f; 
    float AmbientLight = 0.0f;
    float FogShadows = 0.0f; 
    float DustShadows = 0.0f; 
    float BaseHeightFogFalloff = 0.0f; 
    float DustAmount = 0.0f;
    float DustDensityContribution = 0.0f; 
    float DustyHeightFogFalloff = 0.0f; 
    float BaseFogDensity = 0.0f; 
    float VolumetricFogExtincion = 2.5f;
    float FogDaytimeMultiplier = 1.2544f;
    float FogDensityContribution = 0.1f;
    float CloudDensityContribution = 0.002f;
    FLinearColor DustColor = FLinearColor::White;

    //weather related entities
    float Fog = 0.0f; 
    bool FogManualOverride = false;
    bool DustManualOverride = false;
    float CloudCoverage = 0.0f; 
    float WindIntensity = 0.0f; 
    float WindDirection = 1.0f; 
    FVector WindDirectionVector;
    float Dust = 1.0f;
    float Rain = 0.0f; 
    float Snow = 0.0f; 
    float ThunderLightning = 0.0f; 
    float BottomAltitude = 0.0f;
    float VolumetricCloudScale = 0.0f;
    float LayerHeightScale = 0.0f;
    float NoiseScale3D = 0.0f;
    float ErosionIntensity3D = 0.0f;
    float ExtinctionScale = 0.0f;
    float CloudWispsOpacity = 0.0f;
    float CloudWispsOpacityClear = 0.0f;
    FString StaticCloudsTexture;              
    float StaticCloudsRotationSpeed = 0.0f;
    float StaticCloudsColorIntensity = 1.0f;
    float CloudWispsColorIntensity = 1.0f;
    float CloudFormationTextureScale = 0.0f;
    float CloudSpeed = 0.0f;
    float CloudDirection = 0.0f;
    float TracingMaxStartDistance = 0.0f;

    FLinearColor RayleighColor = FLinearColor::White;
    bool PostProcessWindFog = true;
    EUDSColorMode ColorMode = EUDSColorMode::SkyAtmosphere;
    FLinearColor BaseSkyColor = FLinearColor::White;
    FLinearColor NightSkyGlowColor = FLinearColor::White;

    bool SunCastsShadows = true;
    bool MoonCastsShadows = true;
    bool MovableSkyLightCastsShadows = true;
    bool StaticSkyLightCastsShadows = true;
    bool UseCloudShadows = true;
    bool LightningFlashesCastsShadows = true;

};


UCLASS()
class METAVERSEWEATHERSYSTEM_API UEnvironmentAdapter : public UObject
{
	GENERATED_BODY()

public:
    FDateTime DeNormalizeTimeDatefromUDS(TTuple<float, float, float, float>);
    FEnvironmentNormalizedData Out;
	FEnvironmentNormalizedData Normalize(const FEnvironmentData& InputData) const;
    FEnvironmentNormalizedData GetNormalizedData();
public:
    void NormalizeTimeDate(FDateTime TimeDate);
    void NormalizeLightSettings(const FEnvironmentData& Data);
    void NormalizeMoonSettings(const FEnvironmentData& Data);
    void NormalizePrecipitation(const FPrecipitationData& Data);
    void NormalizeVisibility(const FVisibilityData& Data);
    void NormalizeWind(const FWindLayerData& Data);
    void NormalizeClouds(const FCloudLayerData& Data);
    void NormalizeTimeSettings(const FTimeOfDayData& Data);
    void NormalizAmbientLightSettings(const FAmbientCube& Data);
    void NormalizeTimeSpeedSettings(const FEnvironmentData& NewData);
    void NormalizeThunderSettings(const FCloudsData& Data);
};
