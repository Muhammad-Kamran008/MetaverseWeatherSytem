// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "EnvironmentData.h"
#include "EnvironmentAdapter.h"
#include "EnvironmentControllerBase.generated.h"

class UEnvironmentManager;
class UUIEnvironmentController;
/**
 *
 */
UCLASS()
class METAVERSEWEATHERSYSTEM_API UEnvironmentControllerBase : public UObject
{
    GENERATED_BODY()
private:
    static bool isNetwork ;
public:
    void Initialize();
    virtual void UpdateEnvironmentData(const FEnvironmentData& NewData);
    void UpdateAmbientLight(FEnvironmentData& NewData);
    void UpdatePrecipitationData(FAreaOfInterest* AOI);
    void UpdateVisibilityData(FAreaOfInterest* AOI);
    void UpdateWindData(FAreaOfInterest* AOI, FWindLayerData& WindLayer);
    void UpdateCloudsData(FAreaOfInterest* AOI, FCloudLayerData& CloudLayer);
    void UpdateTimeSettingsData(const FEnvironmentData& NewData);
    void UpdateCompleteCycleData();
    FText GetAppliedPresetName();
    FDateTime GetTimeDateFromUDS();
    void SetTimeDateToUDS(FDateTime DateTime);
    void UpdateLightSettingsData(const FEnvironmentData& NewData);
    void UpdateMoonSettingsData(const FEnvironmentData& NewData);
    void UpdateAnimateTimeData(const FEnvironmentData& NewData);
    void UpdateScenarioEnvData();
    void UpdateTimeSpeedSettingsData(FEnvironmentData& NewData);
    void UpdateThunderData(FAreaOfInterest* AOI);
    void UpdateSkySphere1();
    void UpdateSkySphere2();
    void UpdateUltraDynamicSky();
    static FEnvironmentData* GetEnvironmentData() { return EnvironmentData; }
    void UpdateShadowsData(const FEnvironmentData& NewData);
    void ApplyShadows(bool state);
    float GetEffectiveTimeSpeed() const;
    float SetAmbientLight() { return AmbientLight; }

    // ---- New minimal time API ----
    FDateTime GetEffectiveDateTime() const;
    void SetManualDateTime(const FDateTime& InDateTime);
    void SetTimeControlMode(ETimeControlMode NewMode);
    ETimeControlMode GetTimeControlMode() const;
    UPROPERTY()
    UEnvironmentManager* Manager;
    UEnvironmentManager* GetEnvironmentManager() { return Manager; }

    static void SetFlag(bool flag) {
        isNetwork = flag;
    }
    static bool getFlag() {
        return isNetwork;
    }
    static void ResetData();

public:
    static FEnvironmentData* EnvironmentData;

    UPROPERTY()
    UEnvironmentAdapter* Adapter;
    UPROPERTY()
    UUIEnvironmentController* UIEnvironmnetController;
private:
    FTSTicker::FDelegateHandle TimeTickerHandle;
    FDateTime AnimationStartSimTime;
    double AnimationStartRealSeconds = 0.0;
    float AmbientLight = 25.0f;
    FTimerHandle UdsTimerHandle;

    bool OnTimeTicker(float DeltaSeconds);
    void ApplyEffectiveDateTime(const FDateTime& InDateTime);
    FDateTime BuildStartupDateTime() const;
    FDateTime BuildAnimatedDateTime();
    void RefreshIsNight();
    void EnsureTimeTicker();
    void StopTimeTicker();
};
