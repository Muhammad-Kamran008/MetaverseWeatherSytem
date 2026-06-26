// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UIStyle.h"
#include "Engine/Texture2D.h"
#include "Styling/SlateBrush.h"
#include "Containers/Ticker.h"
#include "EnvironmentControllerBase.h"
#include "Delegates/Delegate.h"
#include "UIEnvironmentController.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EPresetMode : uint8
{
	Idle,
	ViewingPreset,
	EditingPreset,
	CreatingPreset
};
DECLARE_DELEGATE_OneParam(FOnPresetChanged, bool);

UCLASS()
class METAVERSEWEATHERSYSTEM_API UUIEnvironmentController : public UEnvironmentControllerBase
{
	GENERATED_BODY()

public:
	void Initialize();
	void AddAreaOfInterest(FAreaOfInterest* AOI);
	void SetCurrentAOI(const FString& key);
	void AddCloudLayer(const FCloudLayerData& CloudLayerData);
	void AddWindLayer(const FWindLayerData& WindLayerData);
	const TMap<FString, FAreaOfInterest*>& GetListOFAreaofInterest() const;
	void AddCloudLayer(FCloudLayerData& CloudLayer);
	void AddWindLayer(FWindLayerData& WindLayer);
	void setIsNetwork(bool bisNetwork);
	TSharedPtr<FWeatherEffectsState> GetWeatherEffectsState();
	FOnPresetChanged OnPresetChanged;
	//Environment
	const TArray<FScenarioEnvironmentPreset>& GetEnvironmentPresets() const;
	bool AddEnvironmentPreset(const FString& PresetName);
	bool UpdateEnvironmentPreset(const FString& OldName, const FString& NewName);
	bool RemoveEnvironmentPreset(const FString& PresetName);
	bool ApplyEnvironmentPresetByName(const FString& PresetName);
	void BeginEditingPreset();
	void OnAnyFeatureChanged();
	void AddNewPreset();
	FText GetAppliedPresetName();
	bool ResetCurrentWeatherEffectToDefaults();
	UTexture2D* GetIcon(FString Name);
	UPROPERTY()
	TMap<FString, UTexture2D*> Icons;

private:
	FText AppliedPresetName;
	bool bHasAppliedPreset = false;
	bool bPresetDirty = false;
	FScenarioEnvironmentPreset AppliedPresetSnapshot;
	FScenarioEnvironmentPreset CurrentFeatureData;
	EPresetMode PresetMode = EPresetMode::Idle;
	TSharedPtr<FWeatherEffectsState> PersistentWeatherEffectsState;
	bool LoadEnvironmentPresetsFromXml();
	bool SaveEnvironmentPresetToXml();
	FString GetPresetXmlPath();
	FString GetPathForIcon(FString& Name)
	{
		return FString::Printf(TEXT("/Game/Blueprints/WeatherIcons/%s.%s"), *Name, *Name);
	}
};
