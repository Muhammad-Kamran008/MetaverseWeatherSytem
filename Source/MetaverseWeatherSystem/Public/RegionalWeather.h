// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnvironmentData.h"
#include "UObject/SoftObjectPtr.h"
#include "RegionalWeather.generated.h"

class ACesiumGeoreference;
class USplineComponent;
class UBlueprint;
class UCesiumGlobeAnchorComponent;

UENUM(BlueprintType)
enum class ERegionGeometry : uint8
{
	Box     UMETA(DisplayName = "Box (TopRight + BottomLeft)"),
	Circle  UMETA(DisplayName = "Circle (Center + Radius)"),
	Polygon UMETA(DisplayName = "Polygon (LonLat Points)")
};


//USTRUCT(BlueprintType)
//struct FLonLat
//{
//	GENERATED_BODY()
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//	double Lon = 0.0;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite)
//	double Lat = 0.0;
//
//	FLonLat() = default;
//	FLonLat(double InLon, double InLat) : Lon(InLon), Lat(InLat) {}
//};

USTRUCT(BlueprintType)
struct FWeatherRegionRequest
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	ERegionGeometry Geometry = ERegionGeometry::Circle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	FString WeatherPresetReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	int32 Priority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float TransitionWidthCm = 20000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	float VolumeAlpha = 1.0f;

	/** Height in meters (Cesium height). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cesium")
	double HeightMeters = 0.0;

	// ---- Box ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (EditCondition = "Geometry==ERegionGeometry::Box"))
	FLonLat TopRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (EditCondition = "Geometry==ERegionGeometry::Box"))
	FLonLat BottomLeft;

	// ---- Circle ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (EditCondition = "Geometry==ERegionGeometry::Circle"))
	FLonLat Center;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (EditCondition = "Geometry==ERegionGeometry::Circle"))
	double RadiusMeters = 6000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (EditCondition = "Geometry==ERegionGeometry::Circle", ClampMin = "8", ClampMax = "256"))
	int32 CircleSegments = 48;

	// ---- Polygon ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (EditCondition = "Geometry==ERegionGeometry::Polygon"))
	TArray<FLonLat> PolygonPoints;
};

UCLASS()
class METAVERSEWEATHERSYSTEM_API ARegionalWeather : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARegionalWeather();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	UPROPERTY(EditAnywhere, Category = "RegionalWeather")
	TObjectPtr<ARegionalWeather> Manager;
	UFUNCTION(BlueprintCallable, Category = "RegionalWeather")
	AActor* SubmitRegion(const FWeatherRegionRequest& Request);
	UFUNCTION(BlueprintCallable, Category = "RegionalWeather")
	TArray<AActor*> SubmitRegions(const TArray<FWeatherRegionRequest>& Requests);
	UFUNCTION(BlueprintCallable, Category = "RegionalWeather")
	void DestroyAllSpawnedRegions();	
public:
	UPROPERTY(EditAnywhere, Category = "UDS|Spawn")
	TSoftObjectPtr<UBlueprint> WeatherOverrideVolumeBlueprint;
	UPROPERTY(EditAnywhere, Category = "UDS|Spawn")
	TSubclassOf<AActor> WeatherOverrideVolumeClassOverride;
	UPROPERTY(EditAnywhere, Category = "Cesium|Refs")
	TObjectPtr<ACesiumGeoreference> Georeference;
	UPROPERTY(EditAnywhere, Category = "Cesium|Stability")
	bool bUseGlobeAnchor = true;
	UPROPERTY(EditAnywhere, Category = "Cesium|Stability")
	bool bSplinePointsAreLocalOffsets = true;
private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> SpawnedVolumes;
private:
	AActor* SpawnAndConfigureVolume(const FWeatherRegionRequest& Request);
	void ApplyUDSProperties(AActor* VolumeActor, const FWeatherRegionRequest& Request);
	bool BuildSpline(AActor* VolumeActor, const FWeatherRegionRequest& Request);
	TSubclassOf<AActor> ResolveSpawnClass() const;
	static USplineComponent* FindSpline(AActor* VolumeActor);

	bool LonLatHeightToUnreal(double LonDeg, double LatDeg, double HeightM, FVector& OutUnreal) const;

	FVector EnsureAnchor(AActor* VolumeActor, const FLonLat& AnchorLonLat, double HeightM) const;

	static FLonLat GetAnchorPointForRequest(const FWeatherRegionRequest& Request);
	static FString ExtractObjectPathFromAnyReference(const FString& InRef);
	static UObject* LoadPresetObjectFromReference(const FString& AnyRef);
	static bool SetObjectPropertyByName(AActor* Target, const FName PropertyName, UObject* Value);
	static bool SetFloatPropertyByName(AActor* Target, const FName PropertyName, float Value);
	static bool SetIntPropertyByName(AActor* Target, const FName PropertyName, int32 Value);
	static FLonLat DestinationPoint(const FLonLat& Start, double BearingRadians, double DistanceMeters);
};
