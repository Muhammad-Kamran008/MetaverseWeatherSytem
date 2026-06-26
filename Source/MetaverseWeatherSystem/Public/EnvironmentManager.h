#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "EnvironmentAdapter.h"
//#include "WeatherDialog.h"
#include "Engine/TextureCube.h"
#include "Tickable.h"
#include "Components/DirectionalLightComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "EnvironmentManager.generated.h"

class UUIEnvironmentController;
class UEnvironmentControllerBase;
class SWeatherDialog;

UENUM(BlueprintType)
enum class ESky_Properties : uint8
{
    //Here we are actually mapping the UDS actors
    EUDSColorMode UMETA(DisplayName = "Color Mode"),
    //Time and Light Settings
    TimeofDay UMETA(DisplayName = "Time Of Day"),
    Month UMETA(DisplayName = "Month"),
    Day UMETA(DisplayName = "Day"),
    Year UMETA(DisplayName = "Year"),
    SimulateRealSun UMETA(DisplayName = "Simulate Real Sun"),
    SimulateRealMoon UMETA(DisplayName = "Simulate Real Moon"),
    DawnTime UMETA(DisplayName = "Dawn Time"),
    DuskTime UMETA(DisplayName = "Dusk Time"),
    TimeSpeed UMETA(DisplayName = "Time Speed"),
    AnimateTimeOfDay UMETA(DisplayName = "Animate Time Of Day"),
    MoonPhase UMETA(DisplayName = "Moon Phase"),
    SunLightIntensity UMETA(DisplayName = "Sun Light Intensity"),
    SkyLightIntensity UMETA(DisplayName = "Sky Light Intensity"),
    DayLength UMETA(DisplayName = "Day Length"),
    NightLength UMETA(DisplayName = "Night Length"),
    //Visibility
    FogShadows       UMETA(DisplayName = "FogShadows"),
    DustShadows      UMETA(DisplayName = "DustShadows"),
    BaseHeightFogFalloff UMETA(DisplayName = "Base Height Fog Falloff"),
    DustAmount       UMETA(DisplayName = "Dust Amount"),
    DustDensityContribution UMETA(DisplayName = "Dust Density Contribution"),
    DustyHeightFogFalloff  UMETA(DisplayName = "Dusty Height Fog Falloff"),
    DustColor        UMETA(DisplayName = "Dust Color"),
    FogDensityContribution        UMETA(DisplayName = "Foggy Density Contribution"),
    VolumetricFogExtincion        UMETA(DisplayName = "Volumetric Fog Extinction"),
    FogDaytimeMultiplier        UMETA(DisplayName = "Fog Density Daytime Mulitplier"),

    BottomAltitude    UMETA(DisplayName = "Bottom Altitude"),
    VolumetricCloudScale UMETA(DisplayName = "Volumetric Clouds Scale"),
    LayerHeightScale  UMETA(DisplayName = "Layer Height Scale"),
    NoiseScale3D      UMETA(DisplayName = "3D Noise Scale"),
    ErosionIntensity3D UMETA(DisplayName = "3D Erosion Intensity"),
    ExtinctionScale   UMETA(DisplayName = "Extinction Scale"),
    CloudWispsOpacityCloudy UMETA(DisplayName = "Cloud Wisps Opacity (Cloudy)"),
    CloudWispsOpacityClear UMETA(DisplayName = "Cloud Wisps Opacity (Clear)"),
    CloudDensityContribution UMETA(DisplayName = "Cloud Density Contribution"),

    StaticCloudsTexture         UMETA(DisplayName = "Static Clouds Texture"),
    StaticCloudsRotationSpeed   UMETA(DisplayName = "Static Clouds Rotation Speed"),
    StaticCloudsColorIntensity  UMETA(DisplayName = "Static Clouds Color Intensity"),
    CloudWispsColorIntensity    UMETA(DisplayName = "Cloud Wisps Color Intensity"),

    CloudFormationTextureScale  UMETA(DisplayName = "Cloud Formation Texture Scale"),
    CloudSpeed UMETA(DisplayName = "Cloud Speed"),
    CloudDirection UMETA(DisplayName = "Cloud Direction"),
    TracingMaxStartDistance UMETA(DisplayName = "Tracing Max Start Distance"),
    RayleighColor UMETA(DisplayName = "Rayleigh Scattering Color (Day)"),
    BaseSkyColor UMETA(DisplayName = "Base Sky Color (Night)"),
    NightSkyGlowColor UMETA(DisplayName = "Night Sky Glow Color"),
    BaseFogDensity  UMETA(DisplayName = "Base Fog Density"),

    SunCastsShadows UMETA(DisplayName = "Sun Casts Shadows"),
    MoonCastsShadows UMETA(DisplayName = "Moon Casts Shadows"),
    StaticSkyLightCastsShadows UMETA(DisplayName = "Static/Stationary Sky Light Casts Shadows"),
    MovableSkyLightCastsShadows UMETA(DisplayName = "Movable Sky Light Casts Shadows (Enable DFAO)"),
    UseCloudShadows UMETA(DisplayName = "Use Cloud Shadows"),

    Latitude UMETA(DisplayName = "Latitude"),
    Longitude UMETA(DisplayName = "Longitude")
};

UENUM(BlueprintType)
enum class EWeather_Properties : uint8
{
    Fog UMETA(DisplayName = "Fog"),
    FogManualOverride UMETA(DisplayName = "Fog-Manual Override"),
    WindDirection UMETA(DisplayName = "Wind Direction"),
    Snow UMETA(DisplayName = "Snow"),
    ThunderLightning UMETA(DisplayName = "Thunder/Lightning"),
    CloudCoverage UMETA(DisplayName = "Cloud Coverage"),
    WindIntensity UMETA(DisplayName = "Wind Intensity"),
    Rain UMETA(DisplayName = "Rain"),
    Dust UMETA(DisplayName = "Dust"),
    DustManualOverride UMETA(DisplayName = "Dust-Manual Override"),
    PostProcessWindFog UMETA(DisplayName = "Enable Post Process Wind Fog"),
    LightningFlashesCastsShadows UMETA(DisplayName = "Lightning Flashes Cast Shadows")
};


UCLASS()
class METAVERSEWEATHERSYSTEM_API UEnvironmentManager : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    void ApplyEnvironment(const FEnvironmentNormalizedData& Data);
    void createEnvironmnetDialog();

    virtual void Tick(float DeltaTime) override;
    virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UEnvironmentManager, STATGROUP_Tickables); }
    virtual bool IsTickable() const override { return bAutoPlayback; }
public:
    void SetTimeSettings(const FEnvironmentNormalizedData& Data);
    void UpdateDateSettings(const FTimeSettings& Data);
    void SetAnimateSettings(const FEnvironmentNormalizedData& Data);
    void SetTimeDateToUDS(const FEnvironmentNormalizedData& Data);
    TTuple<float, float, float, float> GetTimeDatefromUDS();
    float GetSpeedOfTime();
    void SetLightSettings(const FEnvironmentNormalizedData& Data);
    void SetMoonSettings(const FEnvironmentNormalizedData& Data);
    void SetPrecipitationSettings(const FEnvironmentNormalizedData& Data, FAreaOfInterest* AOI);
    void SetCloudsSettings(const FEnvironmentNormalizedData& Data, FAreaOfInterest* AOI, FCloudLayerData& CloudLayer);
    void SetWindSettings(const FEnvironmentNormalizedData& Data, FAreaOfInterest* AOI, FWindLayerData& WindLayer);
    void SetVisibilitySettings(const FEnvironmentNormalizedData& Data, FAreaOfInterest* AOI);
    void ApplyUDSWeatherAsset(const FString& AssetPath);
    void SetAmbientCubemapIntensityFromPercent(float percent);
    void SetPPVVisible(bool bVisible);
    void AddAreaofInterest(FAreaOfInterest* AOI);
    void AddCloudLayer(FAreaOfInterest* AOI, FCloudLayerData& CloudLayer);
    void SetThunderSettings(const FEnvironmentNormalizedData& Data, FAreaOfInterest* AOI);
    void SetSkySphere1Visible(bool bVisible);
    void DirectionalLightVisible(bool bVisible);
    void SetUDSVisible(bool bVisible);
    bool ChangeMaterialOnRunTime(bool value);
//--------------
// Shadows
//--------------
    UDirectionalLightComponent* GetUDSSunComponent() const;
    void ApplySunCastShadows(bool bEnable);
    UDirectionalLightComponent* GetUDSMoonComponent() const;
    void ApplyMoonCastShadows(bool bEnable);
    UDirectionalLightComponent* GetUDSSkyLightComponent() const;
    void ApplySkyLightCastShadows(bool bEnable);
    UDirectionalLightComponent* GetUDSLightningComponent() const;
    void ApplyLightningLightCastShadows(bool bEnable);
    UPROPERTY() APostProcessVolume* PostProcessVolume = nullptr;
    bool getIsNetwork();

    FString AppliedPresetName;
    void CalculatingDayNightLength(const FEnvironmentNormalizedData& Data);
    void StartAutoPlayback();
    void StopAutoPlayback();
    FText GetAppliedPresetName();
private:
    template<typename TEnum, typename TValue>
    void SetEnvironmentProperty(TEnum EnumKey, const TValue& NewValue);
    template<typename TEnum, typename TValue>
    bool GetEnvironmentProperty(TEnum EnumKey, TValue& OutValue) const;

    UPROPERTY() UUIEnvironmentController* EnvController;
    UPROPERTY() UEnvironmentControllerBase* EnvControllerBase;
    UPROPERTY() AActor* UltraDynamicSkyActor = nullptr;
    UPROPERTY() AActor* UltraDynamicWeatherActor = nullptr;
    APawn* PlayerPawn;
    TSharedPtr<SWindow> WeatherDialogWindow;

    UPROPERTY() UTextureCube* Cube = nullptr;
    UMaterialParameterCollection* MPC_Wind = nullptr;
    UMaterialParameterCollectionInstance* MPC_Wind_Inst = nullptr;

    void CreateRectangle_1(FRectangularRecord_1* AOI);
    void CreateBoundingShpere(FBoundingSphere* AOI);  

    bool bAutoPlayback = false;
    int indexToPlay = 0;
    int PrevIndex = 999999;
    FDateTime Date;

    UPROPERTY() AStaticMeshActor* SkySphere1 = nullptr;
    UPROPERTY() AStaticMeshActor* SkySphere2 = nullptr;
    UPROPERTY() UStaticMesh* SkySphereMesh = nullptr;
    UPROPERTY() UMaterialInterface* NewMaterial = nullptr;
    UPROPERTY() UMaterialInterface* NewMaterial2 = nullptr;
    UPROPERTY() ADirectionalLight* MyDirectionalLight = nullptr;
};
