// EnvironmentControllerBase.cpp
#include "EnvironmentControllerBase.h"
#include "EnvironmentManager.h"
#include "Engine/World.h"
#include "UIEnvironmentController.h"


FEnvironmentData* UEnvironmentControllerBase::EnvironmentData = nullptr;

bool UEnvironmentControllerBase::isNetwork = false;

void UEnvironmentControllerBase::ResetData() {
	if (EnvironmentData) {
		delete EnvironmentData;
		EnvironmentData = nullptr;
	}
	isNetwork = false;
}

void UEnvironmentControllerBase::Initialize()
{
	if (!EnvironmentData)
	{
		EnvironmentData = new FEnvironmentData();
	}

	if (!Adapter)
	{
		Adapter = NewObject<UEnvironmentAdapter>(this);
	}
	if (!UIEnvironmnetController)
	{
		UIEnvironmnetController = NewObject<UUIEnvironmentController>(this);
	}
	if (UWorld* World = GetWorld())
	{
		Manager = World->GetSubsystem<UEnvironmentManager>();
	}
	//Time from UDS, Date from System
	if (EnvironmentData)
	{
		const FDateTime StartupDateTime = BuildStartupDateTime();

		EnvironmentData->TimeOfDay.TimeSettings.ControlMode = ETimeControlMode::Manual;
		EnvironmentData->TimeOfDay.TimeSettings.ManualDateTime = StartupDateTime;

		ApplyEffectiveDateTime(StartupDateTime);
	}
}

void UEnvironmentControllerBase::UpdateEnvironmentData(const FEnvironmentData& NewData)
{
	if (!Adapter)
	{
		Adapter = NewObject<UEnvironmentAdapter>(this);
	}

	FEnvironmentNormalizedData Normalized = Adapter->Normalize(NewData);

	Manager->ApplyEnvironment(Normalized);


	if (EnvironmentData)
	{
		*EnvironmentData = NewData;
	}
}

void UEnvironmentControllerBase::ApplyShadows(bool state)
{
	Manager->ApplySunCastShadows(state);
	Manager->ApplyMoonCastShadows(state);
	Manager->ApplySkyLightCastShadows(state);
	Manager->ApplyLightningLightCastShadows(state);
}

void UEnvironmentControllerBase::UpdateAmbientLight(FEnvironmentData& NewData)
{
	Manager->SetAmbientCubemapIntensityFromPercent(NewData.Others.AmbientSettings.AmbientLightIntensity);
}

void UEnvironmentControllerBase::UpdateShadowsData(const FEnvironmentData& NewData)
{
}

void UEnvironmentControllerBase::UpdatePrecipitationData(FAreaOfInterest* AOI)
{
	Adapter->NormalizePrecipitation(AOI->CurrentWeatherEffect.Precipitation);
	Manager->SetPrecipitationSettings(Adapter->GetNormalizedData(), AOI);
}

void UEnvironmentControllerBase::UpdateVisibilityData(FAreaOfInterest* AOI)
{
	Adapter->NormalizeVisibility(AOI->CurrentWeatherEffect.Visibility);
	Manager->SetVisibilitySettings(Adapter->GetNormalizedData(), AOI);
}

void UEnvironmentControllerBase::UpdateWindData(FAreaOfInterest* AOI, FWindLayerData& WindLayer)
{
	Adapter->NormalizeWind(WindLayer);
	Manager->SetWindSettings(Adapter->GetNormalizedData(), AOI, WindLayer);
}

void UEnvironmentControllerBase::UpdateCloudsData(FAreaOfInterest* AOI, FCloudLayerData& CloudLayer)
{
	Adapter->NormalizeClouds(CloudLayer);
	Adapter->NormalizeThunderSettings(AOI->CurrentWeatherEffect.Clouds);
	Manager->SetCloudsSettings(Adapter->GetNormalizedData(), AOI, CloudLayer);
	Manager->SetThunderSettings(Adapter->GetNormalizedData(), AOI);
}

void UEnvironmentControllerBase::UpdateCompleteCycleData()
{
	if (EnvironmentData->TimeOfDay.TimeSettings.CompleteCycle)
	{
		Manager->StartAutoPlayback();
	}
	else
	{
		Manager->StopAutoPlayback();
	}
}

FDateTime UEnvironmentControllerBase::GetTimeDateFromUDS()
{
	TTuple<float, float, float, float> DateTime = Manager->GetTimeDatefromUDS();
	return Adapter->DeNormalizeTimeDatefromUDS(DateTime);
}

void UEnvironmentControllerBase::SetTimeDateToUDS(FDateTime DateTime)
{
	Adapter->NormalizeTimeDate(DateTime);
	Manager->SetTimeDateToUDS(Adapter->GetNormalizedData());
}

float UEnvironmentControllerBase::GetEffectiveTimeSpeed() const
{
	float TimeSpeed = Manager->GetSpeedOfTime();
	return TimeSpeed;
}

void UEnvironmentControllerBase::UpdateLightSettingsData(const FEnvironmentData& NewData)
{
	if (EnvironmentData)
	{
		EnvironmentData->TimeOfDay.LightSettings = NewData.TimeOfDay.LightSettings;
	}

	Adapter->NormalizeLightSettings(NewData);
	Manager->SetLightSettings(Adapter->GetNormalizedData());
	RefreshIsNight();
}

void UEnvironmentControllerBase::UpdateMoonSettingsData(const FEnvironmentData& NewData)
{
	Adapter->NormalizeMoonSettings(NewData);
	Manager->SetMoonSettings(Adapter->GetNormalizedData());
}

void UEnvironmentControllerBase::UpdateAnimateTimeData(const FEnvironmentData& NewData)
{
	Adapter->NormalizeTimeSpeedSettings(NewData);
	Manager->CalculatingDayNightLength(Adapter->GetNormalizedData());
	Manager->SetAnimateSettings(Adapter->GetNormalizedData());
	/*if(EnvironmentData->TimeOfDay.TimeSettings.CompleteCycle)
	{
	}*/
}

void UEnvironmentControllerBase::UpdateThunderData(FAreaOfInterest* AOI)
{
	Adapter->NormalizeThunderSettings(AOI->CurrentWeatherEffect.Clouds);
	Manager->SetThunderSettings(Adapter->GetNormalizedData(), AOI);
}

void UEnvironmentControllerBase::UpdateSkySphere1()
{
	Manager->SetSkySphere1Visible(true);
	Manager->SetPPVVisible(true);
	Manager->ChangeMaterialOnRunTime(false);
	//Manager->DirectionalLightVisible(true);
	Manager->SetUDSVisible(false);
}
void UEnvironmentControllerBase::UpdateSkySphere2()
{
	Manager->SetSkySphere1Visible(true);
	Manager->ChangeMaterialOnRunTime(true);
	Manager->SetPPVVisible(true);
	//Manager->DirectionalLightVisible(true);
	Manager->SetUDSVisible(false);
}
void UEnvironmentControllerBase::UpdateUltraDynamicSky()
{
	Manager->SetSkySphere1Visible(false);
	Manager->DirectionalLightVisible(false);
	Manager->SetPPVVisible(false);
	Manager->SetUDSVisible(true);
}

void UEnvironmentControllerBase::UpdateScenarioEnvData()
{
	/*TMap<FString, FAreaOfInterest*>& AOIS = GetEnvironmentData()->GetCurrentScenarioEnv().GetAreaOfInterestPresets();

	for (TPair<FString, FAreaOfInterest*>& AOIPair : AOIS) {

		Adapter->NormalizePrecipitation(AOIPair.Value->CurrentWeatherEffect.Precipitation);
		Manager->AddAreaofInterest(AOIPair.Value);
		Manager->SetPrecipitationSettings(Adapter->GetNormalizedData(), AOIPair.Value);
		for (FWindData* Wind : AOIPair.Value->CurrentWeatherEffect.Wind.Layers) {
			Adapter->NormalizeWind(AOIPair.Value->CurrentWeatherEffect.Wind.CurrentWindLayer);
			Manager->SetWindSettings(Adapter->GetNormalizedData(), AOIPair.Value, AOIPair.Value->CurrentWeatherEffect.Wind.CurrentWindLayer);
		}

		for (FCloudsData* Cloud : AOIPair.Value->CurrentWeatherEffect.Clouds.Layers.GetData()) {
			Adapter->NormalizeClouds(AOIPair.Value->CurrentWeatherEffect.Clouds.CloudLayer);
			Manager->SetCloudsSettings(Adapter->GetNormalizedData(), AOIPair.Value, AOIPair.Value->CurrentWeatherEffect.Clouds.CloudLayer);

		}


		Adapter->NormalizeVisibility(AOIPair.Value->CurrentWeatherEffect.Visibility);
		Manager->SetVisibilitySettings(Adapter->GetNormalizedData(), AOIPair.Value);


	}*/


}

//------------------
//-------TIME-------
//------------------
FDateTime UEnvironmentControllerBase::BuildStartupDateTime() const
{
	if (!Manager || !Adapter)
	{
		return FDateTime::Now();
	}

	const FDateTime UdsDateTime = const_cast<UEnvironmentControllerBase*>(this)->GetTimeDateFromUDS();
	const FDateTime SysNow = FDateTime::UtcNow();

	if (UdsDateTime.GetYear() < 100) return SysNow;

	return FDateTime(
		SysNow.GetYear(),
		SysNow.GetMonth(),
		SysNow.GetDay(),
		UdsDateTime.GetHour(),
		UdsDateTime.GetMinute(),
		UdsDateTime.GetSecond()
	);
}

void UEnvironmentControllerBase::ApplyEffectiveDateTime(const FDateTime& InDateTime)
{
	if (!EnvironmentData)
	{
		return;
	}

	EnvironmentData->TimeOfDay.TimeSettings.Date = FDateTime(
		InDateTime.GetYear(),
		InDateTime.GetMonth(),
		InDateTime.GetDay());

	EnvironmentData->TimeOfDay.TimeSettings.Time = FDateTime(
		InDateTime.GetYear(),
		InDateTime.GetMonth(),
		InDateTime.GetDay(),
		InDateTime.GetHour(),
		InDateTime.GetMinute(),
		InDateTime.GetSecond());
	RefreshIsNight();
}

void UEnvironmentControllerBase::RefreshIsNight()
{
	if (!EnvironmentData)
	{
		return;
	}

	const FDateTime& CurrentTime = EnvironmentData->TimeOfDay.TimeSettings.Time;
	const FDateTime& Dawn = EnvironmentData->TimeOfDay.LightSettings.DawnTime;
	const FDateTime& Dusk = EnvironmentData->TimeOfDay.LightSettings.DuskTime;

	const int32 CurrentMinutes = CurrentTime.GetHour() * 60 + CurrentTime.GetMinute();
	const int32 DawnMinutes = Dawn.GetHour() * 60 + Dawn.GetMinute();
	const int32 DuskMinutes = Dusk.GetHour() * 60 + Dusk.GetMinute();

	EnvironmentData->TimeOfDay.bIsNight =
		(CurrentMinutes < DawnMinutes || CurrentMinutes >= DuskMinutes);

	if (EnvironmentData->TimeOfDay.bIsNight)
	{
		EnvironmentData->Others.AmbientSettings.AmbientLightIntensity = 0.0f;
	}
	else
	{
		EnvironmentData->Others.AmbientSettings.AmbientLightIntensity = 25.0f;
	}
	AmbientLight = EnvironmentData->Others.AmbientSettings.AmbientLightIntensity;
	Manager->SetAmbientCubemapIntensityFromPercent(AmbientLight);
}

FDateTime UEnvironmentControllerBase::GetEffectiveDateTime() const
{
	if (!EnvironmentData)
	{
		return FDateTime::Now();
	}

	return EnvironmentData->TimeOfDay.TimeSettings.Time;
}

void UEnvironmentControllerBase::SetManualDateTime(const FDateTime& InDateTime)
{
	if (!EnvironmentData)
	{
		return;
	}

	EnvironmentData->TimeOfDay.TimeSettings.ControlMode = ETimeControlMode::Manual;
	EnvironmentData->TimeOfDay.TimeSettings.ManualDateTime = InDateTime;

	StopTimeTicker();
	ApplyEffectiveDateTime(InDateTime);
}

ETimeControlMode UEnvironmentControllerBase::GetTimeControlMode() const
{
	if (!EnvironmentData)
	{
		return ETimeControlMode::Manual;
	}

	return EnvironmentData->TimeOfDay.TimeSettings.ControlMode;
}

void UEnvironmentControllerBase::SetTimeControlMode(ETimeControlMode NewMode)
{
	if (!EnvironmentData)
	{
		return;
	}

	const ETimeControlMode OldMode = EnvironmentData->TimeOfDay.TimeSettings.ControlMode;
	const FDateTime CurrentEffective = GetEffectiveDateTime();

	if ((OldMode == ETimeControlMode::Animated || OldMode == ETimeControlMode::System) &&
		NewMode == ETimeControlMode::Manual)
	{
		EnvironmentData->TimeOfDay.TimeSettings.ManualDateTime = CurrentEffective;
	}

	EnvironmentData->TimeOfDay.TimeSettings.ControlMode = NewMode;

	if (NewMode == ETimeControlMode::Manual)
	{
		StopTimeTicker();

		ApplyEffectiveDateTime(EnvironmentData->TimeOfDay.TimeSettings.ManualDateTime);

		UpdateAnimateTimeData(*EnvironmentData);

		return;
	}

	if (NewMode == ETimeControlMode::System)
	{
		const FDateTime Now = FDateTime::Now();
		SetTimeDateToUDS(Now);
		ApplyEffectiveDateTime(Now);

		UpdateAnimateTimeData(*EnvironmentData);

		EnsureTimeTicker();
		return;
	}

	if (NewMode == ETimeControlMode::Animated)
	{
		//  	AnimationStartSimTime = CurrentEffective;
		//		AnimationStartRealSeconds = FPlatformTime::Seconds();
		UpdateAnimateTimeData(*EnvironmentData);

		EnsureTimeTicker();
		return;
	}
}

FDateTime UEnvironmentControllerBase::BuildAnimatedDateTime()
{
	//	if (!EnvironmentData)
	//	{
	//		return FDateTime::Now();
	//	}
	//
	//	const double RealNow = FPlatformTime::Seconds();
	//	const double ElapsedReal = RealNow - AnimationStartRealSeconds;
	//
	//	double SpeedMultiplier = 1.0;
	//	switch (EnvironmentData->TimeOfDay.TimeSettings.SpeedMode)
	//	{
	//	case ETimeSpeedMode::RealTime: SpeedMultiplier = 1.0;  break;
	//	case ETimeSpeedMode::X2:       SpeedMultiplier = 2.0;  break;
	//	case ETimeSpeedMode::X3:       SpeedMultiplier = 3.0;  break;
	//	case ETimeSpeedMode::X10:      SpeedMultiplier = 10.0; break;
	//	case ETimeSpeedMode::X20:      SpeedMultiplier = 20.0; break;
	//	default:                       SpeedMultiplier = 1.0;  break;
	//	}
	//
	//	return AnimationStartSimTime + FTimespan::FromSeconds(ElapsedReal * SpeedMultiplier);
	const FDateTime UdsDateTime = const_cast<UEnvironmentControllerBase*>(this)->GetTimeDateFromUDS();
	const FDateTime SysNow = FDateTime::Now();

	return FDateTime(
		SysNow.GetYear(),
		SysNow.GetMonth(),
		SysNow.GetDay(),
		UdsDateTime.GetHour(),
		UdsDateTime.GetMinute(),
		UdsDateTime.GetSecond()
	);
}

bool UEnvironmentControllerBase::OnTimeTicker(float DeltaSeconds)
{
	if (!EnvironmentData)
	{
		return false;
	}

	switch (EnvironmentData->TimeOfDay.TimeSettings.ControlMode)
	{
	case ETimeControlMode::System:
		ApplyEffectiveDateTime(FDateTime::Now());
		return true;

	case ETimeControlMode::Animated:
		ApplyEffectiveDateTime(BuildAnimatedDateTime());
		return true;

	case ETimeControlMode::Manual:
	default:
		return true;
	}
}

void UEnvironmentControllerBase::EnsureTimeTicker()
{
	if (!TimeTickerHandle.IsValid())
	{
		TimeTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(this, &UEnvironmentControllerBase::OnTimeTicker),
			0.1f
		);
	}
}

void UEnvironmentControllerBase::StopTimeTicker()
{
	if (TimeTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TimeTickerHandle);
		TimeTickerHandle.Reset();
	}
}

void UEnvironmentControllerBase::UpdateTimeSpeedSettingsData(FEnvironmentData& NewData)
{
	if (!EnvironmentData)
	{
		return;
	}

	EnvironmentData->TimeOfDay.TimeSettings.SpeedMode = NewData.TimeOfDay.TimeSettings.SpeedMode;
	UpdateAnimateTimeData(*EnvironmentData);
}

