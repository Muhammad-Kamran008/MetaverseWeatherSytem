
#include "EnvironmentAdapter.h"
#include "WeatherPrecipitationMapping.h"

FEnvironmentNormalizedData UEnvironmentAdapter::Normalize(
	const FEnvironmentData& InputData) const
{
	FEnvironmentNormalizedData Out1;
	return Out1;
}

FDateTime UEnvironmentAdapter::DeNormalizeTimeDatefromUDS(TTuple<float, float, float, float> DateTime)
{
	const float UdsTime = DateTime.Get<0>();
	const int32 Month = FMath::Max(1,static_cast<int32>(DateTime.Get<1>()));
	const int32 Year = FMath::Max(1, static_cast<int32>(DateTime.Get<2>()));
	const int32 Day = FMath::Max(1, static_cast<int32>(DateTime.Get<3>()));

	float TotalHours = UdsTime / 100.0f;
	int32 Hours = FMath::FloorToInt(TotalHours) % 24;
	//float FractionHour = TotalHours - FMath::FloorToFloat(TotalHours);
	//int32 Minutes = FMath::RoundToInt(FractionHour * 60.0f);
	float MinuteFraction = FMath::Fractional(UdsTime / 100.0f);
	int32 Minutes = FMath::RoundToInt(MinuteFraction * 60.0f);

	if (Minutes >= 60)
	{
		Minutes = 0;
		Hours = (Hours + 1) % 24;
	}

	//Hours = Hours % 24;

	const int32 SafeYear = (Year > 0) ? Year : 2024;
	const int32 SafeMonth = FMath::Clamp(Month, 1, 12);

	const int32 MaxDay = FDateTime::DaysInMonth(SafeYear, SafeMonth);
	const int32 SafeDay = FMath::Clamp(Day, 1, MaxDay);
	Out.SimulateRealSun = true;
	Out.SimulateRealMoon = true;
	return FDateTime(
		SafeYear,
		SafeMonth,
		SafeDay,
		Hours,
		Minutes,
		0
	);
}

void UEnvironmentAdapter::NormalizeTimeDate(FDateTime TimeDate)
{
    const int32 Year = TimeDate.GetYear();
	const int32 Month = TimeDate.GetMonth();
	const int32 Day = TimeDate.GetDay();
	const int32 Hour = TimeDate.GetHour();
	const int32 Minute = TimeDate.GetMinute();
	Out.SimulateRealSun = true;
	Out.SimulateRealMoon = true;
	//const int32 SafeHour = FMath::Clamp(Hour, 0, 23);
	//const int32 SafeMinute = FMath::Clamp(Minute, 0, 59);
	float MinutesFraction = Minute / 60.0f;
	Out.TimeOfDay = (Hour + MinutesFraction) * 100.0f;
//	Out.TimeOfDay = (SafeHour * 100) + SafeMinute;
	UE_LOG(LogTemp, Warning, TEXT("Time of Day NORMALIZE: %f"), Out.TimeOfDay);
	Out.Year = (Year > 0) ? Year : 2000;
	Out.Month = (Month >= 1 && Month <= 12) ? Month : 1;
	Out.Day = (Day >= 1 && Day <= 31) ? Day : 1;
	UE_LOG(LogTemp, Warning, TEXT("Date NORMALIZE: %d, %d, %d"), Out.Year, Out.Month, Out.Day);

}

FEnvironmentNormalizedData UEnvironmentAdapter::GetNormalizedData()
{
	return Out;
}

static FORCEINLINE float Clamp10(float V) { return FMath::Clamp(V, 0.0f, 10.0f); }
void UEnvironmentAdapter::NormalizePrecipitation(const FPrecipitationData& Data)
{
	if (!Data.bEnabled)
	{
		Out.Rain = 0.0f;
		Out.CloudCoverage = 0.0f;
		Out.ThunderLightning = 0.0f;
		Out.WindIntensity = 0.0f;
		//Out.Fog = 0.0f;
		Out.Snow = 0.0f;
		return;
	}
	const float MmPerHour = FMath::Clamp(Data.RateMmPerHr, 0.0f, WeatherSim::ExtremeMaxMmPerHour);
	const float UdsIntensity = WeatherSim::MmPerHourToUdsScalar_0_10(MmPerHour);
	
	float CloudCoverage = 0.0f;
	float RainIntensity = 0.0f;
	float Thunder = 0.0f;
	float WindIntensity = 0.0f;
	float fog = 0.0f;
	float SnowIntensity = 0.0f;

	switch (Data.Type)
	{
	case EPrecipitationType::Rain:
		RainIntensity = UdsIntensity;
		SnowIntensity = 0.0f;
		CloudCoverage = Clamp10(3.0f + 0.95f * UdsIntensity);
		WindIntensity = Clamp10(1.5f + 0.75f * UdsIntensity);
		fog = Clamp10(1.0f + 0.90f * UdsIntensity);
		Thunder = Clamp10((UdsIntensity - 4.5f) * 1.6f);
		/*CloudCoverage = Clamp10(2.0f + 0.85f * UdsIntensity);
		WindIntensity = Clamp10(1.0f + 0.55f * UdsIntensity);
		fog = Clamp10(0.5f + 0.70f * UdsIntensity);
		Thunder = Clamp10((UdsIntensity - 4.5f) * 1.6f);*/
		break;
	case EPrecipitationType::Snow:
		SnowIntensity = UdsIntensity;
		RainIntensity = 0.0f;
		CloudCoverage = Clamp10(2.5f + 0.80f * UdsIntensity);
		WindIntensity = Clamp10(1.0f + 0.60f * UdsIntensity);
		fog = Clamp10(1.5f + 0.65f * UdsIntensity);
		Thunder = Clamp10((UdsIntensity - 8.0f) * 2.0f);
		break;
	case EPrecipitationType::Hail:
		RainIntensity = UdsIntensity;
		SnowIntensity = 0.0f;
		CloudCoverage = Clamp10(4.0f + 0.70f * UdsIntensity);
		WindIntensity = Clamp10(3.0f + 0.70f * UdsIntensity);
		fog = Clamp10(0.5f + 0.50 * UdsIntensity);
		Thunder = Clamp10(2.0f + 0.90f * UdsIntensity);
		break;

	default:
		CloudCoverage = 0.0f;
		SnowIntensity = 0.0f;
		Thunder = 0.0f;
		WindIntensity = 0.0f;
		//fog = 0.0f;
		RainIntensity = 0.0f;
		break;
	}
	Out.Rain = RainIntensity;
	//Out.CloudCoverage = CloudCoverage;
	Out.ThunderLightning = Thunder;
	Out.WindIntensity = WindIntensity;
	Out.Fog = fog;
	Out.Snow = SnowIntensity;
	
}

void UEnvironmentAdapter::NormalizeVisibility(const FVisibilityData& Data)
{
	if (!Data.bEnabled)
	{
		Out.Fog = 0.0f;
		Out.Dust = 0.0f;
		return;
	}
	float FogValue = 0.0f;
	float DustValue = 0.0f;
	float HazeValue = 0.0f;
	float BaseFog = 0.0f;
	float Normlaized = 0.0f;
	float Visbility = 0.0f;
	float Extinction = 0.0f;
	Out.FogManualOverride = false;
	Out.DustManualOverride = false;
	switch (Data.Obscurant)
	{
	case EVisibilityObscurant::Fog:
		Out. FogManualOverride = true;
		Out.ThunderLightning = 0;
		//FogValue = 2.2f + (20.0f - 2.2f) * FMath::Exp(-0.2f * Data.DistanceKm);
		Normlaized = FMath::Clamp(Data.DistanceKm / 10.0f, 0.0f, 1.0f);
		FogValue = 10.0f * FMath::Pow(1.0f - Normlaized, 2.0f);
		Out.WindIntensity = 1.0f;
		break;

	case EVisibilityObscurant::Dust:
		Out.DustManualOverride = true;
		Out.ThunderLightning = 0;
		DustValue=1.08f+(10.0f-1.08f)*FMath::Exp(-0.45f*Data.DistanceKm);
		//Out.WindIntensity = 10.0f;
		FogValue = 1.0f;
		break;
		
	case EVisibilityObscurant::Haze:
		Out.FogManualOverride = true;
		Visbility = FMath::Clamp(Data.DistanceKm, 0.0f, 10.0f);
		Visbility = Visbility / 10.0f;
		HazeValue = 1.0f - Visbility;
		HazeValue = FMath::Pow(HazeValue,0.45f);
		FogValue = FMath::Lerp(0.0f,10.0f,HazeValue);
		DustValue = FMath::Lerp(0.0f,7.0f, HazeValue);
		BaseFog = FMath::Lerp(0.35f, 0.05f, HazeValue);
		
		break;

	case EVisibilityObscurant::None:
		FogValue = 0.0f;
		DustValue = 0.0f;
		Out.FogManualOverride = false;
		Out.DustManualOverride = false;
		Out.BaseHeightFogFalloff = 0.0f;
		break;

	default:
		FogValue = 0.0f;
		DustValue = 0.0f;
		break;
	}

	Out.Fog = FogValue;
	Out.Dust = DustValue;
	Out.BaseHeightFogFalloff = BaseFog;
}

void UEnvironmentAdapter::NormalizeWind(const FWindLayerData& Data)
{
	float UDSWind = 10.0f * FMath::Pow(FMath::Clamp(Data.SpeedKt / 407.0f, 0.0f, 1.0f), 0.6f);
	Out.WindIntensity = FMath::Clamp(UDSWind, 0.0f, 10.0f);

	Out.WindDirection = Data.DirectionDeg+270.0f;
	float RadianAngle = FMath::DegreesToRadians(Data.DirectionDeg);
	Out.WindDirectionVector = FVector(FMath::Cos(RadianAngle), FMath::Sin(RadianAngle), 0.0f);
}

//void UEnvironmentAdapter::NormalizeClouds(const FCloudLayerData& Data)
//{
//	float CloudCoverage=0.0f;
//
//	switch (Data.Type)
//	{
//	case ECloudType::Cirrocumulus:
//		CloudCoverage = 0.0f;
//		CloudCoverage = (Data.Cover == ECloudCover::Clear) ? 5.76f * 0.0f :
//			(Data.Cover == ECloudCover::Few) ? 5.76f * 0.35f :
//			(Data.Cover == ECloudCover::Scattered) ? 5.76f * 0.6f :
//			(Data.Cover == ECloudCover::Broken) ? 5.76f * 0.85f :
//			(Data.Cover == ECloudCover::Overcast) ? 5.76f * 1.15f : 0.0f;
//		Out.ThunderLightning = (Data.Cover == ECloudCover::Clear) ? 0.0f :
//			(Data.Cover == ECloudCover::Few) ? 0.0f :
//			(Data.Cover == ECloudCover::Scattered) ? 0.0f :
//			(Data.Cover == ECloudCover::Broken) ? 0.0f :
//			(Data.Cover == ECloudCover::Overcast) ? 3.0f : 0.0f;
//		Out.BottomAltitude = 1.589f;
//		Out.VolumetricCloudScale = 2.21f;
//		Out.LayerHeightScale = 0.521f;
//		Out.NoiseScale3D = 1.43f;
//		Out.ErosionIntensity3D = 0.94f;
//		Out.ExtinctionScale = 13.2472f;
//		Out.CloudWispsOpacity = 0.704f;
//		Out.RayleighColor = FLinearColor(0.073413f, 0.172646f, 0.317708f, 1.0f); 
//		break;
//
//	case ECloudType::Cirrostratus:
//		Out.CloudWispsOpacityClear = 0.0f;
//		Out.StaticCloudsRotationSpeed = 0.128f;
//		Out.CloudWispsOpacityClear = (Data.Cover == ECloudCover::Clear) ? 0.0f :
//			(Data.Cover == ECloudCover::Few) ? 0.3f :
//			(Data.Cover == ECloudCover::Scattered) ? 0.5f :
//			(Data.Cover == ECloudCover::Broken) ? 0.75f :
//			(Data.Cover == ECloudCover::Overcast) ? 1.0f : 0.0f;
//		Out.CloudWispsColorIntensity = (Data.Cover == ECloudCover::Clear) ? 0.0f :
//			(Data.Cover == ECloudCover::Few) ? 1.0f :
//			(Data.Cover == ECloudCover::Scattered) ? 2.0f :
//			(Data.Cover == ECloudCover::Broken) ? 3.0f :
//			(Data.Cover == ECloudCover::Overcast) ? 4.0f : 0.0f;
//		Out.BottomAltitude = -0.6f;
//		Out.VolumetricCloudScale = 2.21f;
//		Out.LayerHeightScale = 0.2f;
//		Out.NoiseScale3D = 0.4f;
//		Out.ErosionIntensity3D = 0.3f;
//		Out.ExtinctionScale = (Data.Cover == ECloudCover::Clear) ? 0.0f :
//			(Data.Cover == ECloudCover::Few) ? 11.5f :
//			(Data.Cover == ECloudCover::Scattered) ? 12.5f :
//			(Data.Cover == ECloudCover::Broken) ? 13.9f :
//			(Data.Cover == ECloudCover::Overcast) ? 15.2f : 0.0f;
//		Out.RayleighColor = FLinearColor(0.073413f, 0.172646f, 0.317708f, 1.0f);
//		Out.ThunderLightning = 0.0f;
//		break;
//
//	case ECloudType::CumulusCongestus:
//		CloudCoverage = 0.0f;
//		CloudCoverage = (Data.Cover == ECloudCover::Clear) ? 2.5f * 0.0f :
//			(Data.Cover == ECloudCover::Few) ? 2.5f * 1.25f :
//			(Data.Cover == ECloudCover::Scattered) ? 2.5f * 1.45f :
//			(Data.Cover == ECloudCover::Broken) ? 2.5f * 1.85f :
//			(Data.Cover == ECloudCover::Overcast) ? 2.5f * 2.15f : 0.0f;
//		Out.ThunderLightning = (Data.Cover == ECloudCover::Clear) ? 0.0f :
//			(Data.Cover == ECloudCover::Few) ? 0.0f :
//			(Data.Cover == ECloudCover::Scattered) ? 0.0f :
//			(Data.Cover == ECloudCover::Broken) ? 0.0f :
//			(Data.Cover == ECloudCover::Overcast) ? 3.0f : 0.0f;
//		Out.CloudWispsOpacity = 1.0f;
//		Out.CloudWispsColorIntensity = 2.04f;
//		Out.BottomAltitude = 30.0f;
//		Out.VolumetricCloudScale = 5.0f;
//		Out.LayerHeightScale = 2.0f;
//		Out.CloudFormationTextureScale = 1.8f;
//		Out.NoiseScale3D = 4.0f;
//		Out.ErosionIntensity3D = 1.0f;
//		Out.ExtinctionScale = 5.0f;
//		Out.CloudWispsOpacity = 0.0f;
//		Out.CloudWispsOpacityClear = 0.0f;
//		Out.CloudSpeed = 0.35f;
//		Out.CloudDirection = 180.0f;
//		Out.TracingMaxStartDistance = 17.0f;
//		Out.RayleighColor = FLinearColor(0.073413f, 0.172646f, 0.317708f, 1.0f);
//		break;
//
//	case ECloudType::PartlyCloudy:
//		CloudCoverage = 0.0f;
//		
//		CloudCoverage = (Data.Cover == ECloudCover::Clear) ? 0.0f :
//			(Data.Cover == ECloudCover::Few) ? 2.5f :
//			(Data.Cover == ECloudCover::Scattered) ? 3.0f :
//			(Data.Cover == ECloudCover::Broken) ? 3.5f :
//			(Data.Cover == ECloudCover::Overcast) ? 4.0f : 0.0f;
//		Out.ThunderLightning = (Data.Cover == ECloudCover::Clear) ? 0.0f :
//			(Data.Cover == ECloudCover::Few) ? 0.0f :
//			(Data.Cover == ECloudCover::Scattered) ? 0.0f :
//			(Data.Cover == ECloudCover::Broken) ? 0.0f :
//			(Data.Cover == ECloudCover::Overcast) ? 3.0f : 0.0f;
//		Out.CloudWispsOpacity = 1.0f;
//		Out.CloudWispsColorIntensity = 2.04f;
//		Out.BottomAltitude = 30.0f;
//		Out.VolumetricCloudScale = 5.0f;
//		Out.LayerHeightScale = 2.0f;
//		Out.CloudFormationTextureScale = 1.8f;
//		Out.NoiseScale3D = 4.0f;
//		Out.ErosionIntensity3D = 1.0f;
//		Out.ExtinctionScale = 5.0f;
//		Out.CloudWispsOpacity = 0.0f;
//		Out.CloudWispsOpacityClear = 0.0f;
//		Out.CloudSpeed = 0.35f;
//		Out.CloudDirection = 180.0f;
//		Out.TracingMaxStartDistance = 17.0f;
//		Out.WindIntensity = 2.0f;
//		Out.VolumetricCloudScale = 5.0f;
//		Out.Fog = 1.0f;
//		Out.RayleighColor = FLinearColor(0.073413f, 0.172646f, 0.317708f, 1.0f); 
//		break;
//
//	case ECloudType::CompleteClearSkies:
//		CloudCoverage = 0.0f;
//		Out.Rain = 0.0f;
//		Out.Snow = 0.0f;
//		Out.WindIntensity = 2.0f;
//		Out.ThunderLightning = 0.0f;
//		Out.Fog = 0.0f;
//		Out.Dust = 0.0f;
//		Out.BaseHeightFogFalloff = 0.08f;
//		Out.CloudWispsOpacityClear = 0.0f;
//		Out.BaseFogDensity = 0.0f;
//		Out.RayleighColor = FLinearColor(0.168627f, 0.407843f, 1.0f, 1.0f);
//		break;
//
//	default:
//		Out.CloudCoverage = 0.0f;
//		break;
//	}
//
//	Out.CloudCoverage = CloudCoverage;
//	
//}

void UEnvironmentAdapter::NormalizeClouds(const FCloudLayerData& Data)
{
	Out.CloudCoverage = 0.0f;
	Out.ThunderLightning = 0.0f;

	Out.BottomAltitude = 0.0f;
	Out.VolumetricCloudScale = 0.0f;
	Out.LayerHeightScale = 0.0f;
	Out.NoiseScale3D = 0.0f;
	Out.ErosionIntensity3D = 0.0f;
	Out.ExtinctionScale = 0.0f;

	Out.CloudWispsOpacity = 0.0f;
	Out.CloudWispsOpacityClear = 0.0f;
	Out.CloudWispsColorIntensity = 0.0f;

	Out.CloudFormationTextureScale = 0.0f;
	Out.CloudSpeed = 0.0f;
	Out.CloudDirection = 0.0f;
	Out.TracingMaxStartDistance = 0.0f;

	Out.StaticCloudsRotationSpeed = 0.0f;
	Out.StaticCloudsColorIntensity = 0.0f;

	Out.BaseHeightFogFalloff = 0.08f;
	Out.BaseFogDensity = 0.0f;

	auto CoverWeight = [](ECloudCover Cover) -> float
		{
			switch (Cover)
			{
			case ECloudCover::Clear:      return 0.0f;
			case ECloudCover::Few:        return 0.25f;
			case ECloudCover::Scattered:  return 0.50f;
			case ECloudCover::Broken:     return 0.75f;
			case ECloudCover::Overcast:   return 1.00f;
			default:                      return 0.0f;
			}
		};

	const float W = CoverWeight(Data.Cover);

	switch (Data.Type)
	{
	case ECloudType::Cirrocumulus:
	{
		Out.CloudCoverage = 9.0f * W;
		//Out.ThunderLightning = (Data.Cover == ECloudCover::Overcast) ? 3.0f : 0.0f;

		Out.BottomAltitude = 1.589f;
		Out.VolumetricCloudScale = 2.21f;
		Out.LayerHeightScale = 0.521f;
		Out.NoiseScale3D = 1.43f;
		Out.ErosionIntensity3D = 0.94f;
		Out.ExtinctionScale = 13.2472f;
		Out.CloudWispsOpacity = 0.704f;
		Out.RayleighColor = FLinearColor(0.073413f, 0.172646f, 0.317708f, 1.0f);
		break;
	}

	case ECloudType::Cirrostratus:
	{
		//Out.CloudCoverage = 6.5f * W;
		//Out.ThunderLightning = 0.0f;

		Out.StaticCloudsRotationSpeed = 0.128f;
		Out.CloudWispsOpacityClear = W;
		Out.CloudWispsColorIntensity = 4.0f * W;

		Out.BottomAltitude = -0.6f;
		Out.VolumetricCloudScale = 2.21f;
		Out.LayerHeightScale = 0.2f;
		Out.NoiseScale3D = 0.4f;
		Out.ErosionIntensity3D = 0.3f;
		Out.ExtinctionScale = 10.0f + (5.2f * W);
		Out.RayleighColor = FLinearColor(0.073413f, 0.172646f, 0.317708f, 1.0f);
		break;
	}

	case ECloudType::CumulusCongestus:
	{
		Out.CloudCoverage = 7.0f * W;
		//Out.ThunderLightning = (Data.Cover == ECloudCover::Overcast) ? 3.0f : 0.0f;

		Out.CloudWispsOpacity = 0.0f;
		Out.CloudWispsOpacityClear = 0.0f;
		Out.CloudWispsColorIntensity = 2.04f;
		Out.BottomAltitude = 30.0f;
		Out.VolumetricCloudScale = 5.0f;
		Out.LayerHeightScale = 2.0f;
		Out.CloudFormationTextureScale = 1.8f;
		Out.NoiseScale3D = 4.0f;
		Out.ErosionIntensity3D = 1.0f;
		Out.ExtinctionScale = 5.0f;
		Out.CloudSpeed = 0.35f;
		Out.CloudDirection = 180.0f;
		Out.TracingMaxStartDistance = 17.0f;
		Out.RayleighColor = FLinearColor(0.073413f, 0.172646f, 0.317708f, 1.0f);
		break;
	}

	case ECloudType::PartlyCloudy:
	{
		Out.CloudCoverage = 4.0f * W;
		//Out.ThunderLightning = (Data.Cover == ECloudCover::Overcast) ? 2.0f : 0.0f;

		Out.CloudWispsOpacity = 0.0f;
		Out.CloudWispsOpacityClear = 0.0f;
		Out.CloudWispsColorIntensity = 2.04f;
		Out.BottomAltitude = 30.0f;
		Out.VolumetricCloudScale = 5.0f;
		Out.LayerHeightScale = 2.0f;
		Out.CloudFormationTextureScale = 1.8f;
		Out.NoiseScale3D = 4.0f;
		Out.ErosionIntensity3D = 1.0f;
		Out.ExtinctionScale = 5.0f;
		Out.CloudSpeed = 0.35f;
		Out.CloudDirection = 180.0f;
		Out.TracingMaxStartDistance = 17.0f;
		//Out.Fog = FMath::Max(Out.Fog, 1.0f * W);
		Out.RayleighColor = FLinearColor(0.073413f, 0.172646f, 0.317708f, 1.0f);
		break;
	}

	case ECloudType::CompleteClearSkies:
	{
		Out.CloudCoverage = 0.0f;
		Out.Rain = 0.0f;
		Out.Snow = 0.0f;
		Out.ThunderLightning = 0.0f;
		Out.Dust = 0.0f;
		Out.BaseHeightFogFalloff = 0.08f;
		Out.CloudWispsOpacityClear = 0.0f;
		Out.BaseFogDensity = 0.0f;
		Out.RayleighColor = FLinearColor(0.168627f, 0.407843f, 1.0f, 1.0f);
		break;
	}

	default:
	{
		Out.CloudCoverage = 0.0f;
		break;
	}
	}

	Out.CloudCoverage = FMath::Clamp(Out.CloudCoverage, 0.0f, 10.0f);
	//Out.ThunderLightning = FMath::Clamp(Out.ThunderLightning, 0.0f, 10.0f);
}

void UEnvironmentAdapter::NormalizeTimeSettings(const FTimeOfDayData& Data)
{
	float TimeOfDay = 960.0f;     
	

	if (!Data.TimeSettings.bEnabled)
	{
		
		return;
	}

	Out.TimeOfDay = (Data.TimeSettings.Time.GetHour() * 100) + Data.TimeSettings.Time.GetMinute();
	//Out.isNight = Data.bIsNight;
}

void UEnvironmentAdapter::NormalizeLightSettings(const FEnvironmentData& Data)
{
	float DawnTimeOfDay = 0.0f;
	float DuskTimeOfDay = 0.0f;
	
	Out.DawnTime = (Data.TimeOfDay.LightSettings.DawnTime.GetHour() * 100) + Data.TimeOfDay.LightSettings.DawnTime.GetMinute();
	Out.DuskTime = (Data.TimeOfDay.LightSettings.DuskTime.GetHour() * 100) + Data.TimeOfDay.LightSettings.DuskTime.GetMinute();

	
}

void UEnvironmentAdapter::NormalizeMoonSettings(const FEnvironmentData& Data)
{
	switch (Data.TimeOfDay.LightSettings.MoonPhase)
	{
	case EMoonPhase::NewMoon:     Out.MoonPhase = 14.8f;  break;
	case EMoonPhase::Waxing:	  Out.MoonPhase = 7.4f;   break;
	case EMoonPhase::FullMoon:    Out.MoonPhase = 0.0f;   break;
	case EMoonPhase::Wanning:	  Out.MoonPhase = 22.1f;  break;
	default:					  Out.MoonPhase = 0.0f;   break;
	}
}

void UEnvironmentAdapter::NormalizAmbientLightSettings(const FAmbientCube& Data)
{
	Out.AmbientLight = Data.AmbientLightIntensity * 100.f;

}

void UEnvironmentAdapter::NormalizeTimeSpeedSettings(const FEnvironmentData& Data)
{
	const bool bAnimated = (Data.TimeOfDay.TimeSettings.ControlMode == ETimeControlMode::Animated);

	Out.AnimateTimeOfDay = bAnimated;

	if (!bAnimated)
	{
		Out.TimeSpeed = 0.0f;
		return;
	}

	switch (Data.TimeOfDay.TimeSettings.SpeedMode)
	{
	case ETimeSpeedMode::RealTime: Out.TimeSpeed = 1.0f;   break;
	case ETimeSpeedMode::X2:       Out.TimeSpeed = 20.0f;  break;
	case ETimeSpeedMode::X3:       Out.TimeSpeed = 30.0f;  break;
	case ETimeSpeedMode::X5:       Out.TimeSpeed = 50.0f;  break;
	case ETimeSpeedMode::X8:       Out.TimeSpeed = 80.0f;  break;
	case ETimeSpeedMode::X10:      Out.TimeSpeed = 100.0f; break;
	default:                       Out.TimeSpeed = 0.0f;   break;
	}
}

void UEnvironmentAdapter::NormalizeThunderSettings(const FCloudsData& Data)
{
	switch (Data.Thunder)
	{
	case EThunderOption::Light:     Out.ThunderLightning = 5.0f;  break;
	case EThunderOption::Moderate:  Out.ThunderLightning = 8.0f;  break;
	case EThunderOption::Heavy:     Out.ThunderLightning = 10.0f; break;
	default:						Out.ThunderLightning = 0.0f;  break;
	}
}
