#pragma once

#include "CoreMinimal.h"

namespace WeatherSim
{
	static constexpr float LightMaxMmPerHour = 2.5f;
	static constexpr float ModerateMaxMmPerHour = 7.6f;                
	static constexpr float HeavyMaxMmPerHour = 50.0f;
	static constexpr float ExtremeMaxMmPerHour = 100.0f;

	FORCEINLINE float MmPerHourToUdsScalar_0_10(float MmPerHour)
	{
		MmPerHour = FMath::Clamp(MmPerHour, 0.0f, ExtremeMaxMmPerHour);
		if (MmPerHour <= 0.0f) return 0.0f;
		const float Num = FMath::Loge(1.0f + MmPerHour);
		const float Den = FMath::Loge(1.0f + ExtremeMaxMmPerHour);
		return 10.0f * (Num / Den);
	}

	FORCEINLINE float UdsScalar_0_10_ToMmPerHour(float Uds)
	{
		Uds = FMath::Clamp(Uds, 0.0f, 10.0f);
		const float LogBase = FMath::Loge(1.0f + ExtremeMaxMmPerHour);
		return FMath::Exp((Uds / 10.0f) * LogBase) - 1.0f;
	}

	FORCEINLINE float RepresentativeMmPerHourForLabel(const FString& Label)
	{
		if (Label == TEXT("Light"))		return 1.2f;
		if (Label == TEXT("Moderate"))		return 5.0f;
		if (Label == TEXT("Heavy"))		return 25.0f;
		if (Label == TEXT("Extreme"))		return 75.0f;
		return 0.0f;
	}

	template<typename TEnumIntensity>
	FORCEINLINE TEnumIntensity ClassifyPerHour(float MmPerHour)
	{
		return (TEnumIntensity)0;
	}
}