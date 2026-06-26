#include "RegionalWeather.h"
#include "Engine.h"
#include "Engine/World.h"
#include "Engine/Blueprint.h"
#include "UObject/UnrealType.h"
#include "Components/SplineComponent.h"
#include "DrawDebugHelpers.h"
#include "CesiumGeoreference.h"
#include "CesiumGlobeAnchorComponent.h"

ARegionalWeather::ARegionalWeather()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARegionalWeather::BeginPlay()
{
	Super::BeginPlay();

	if (!Georeference)
	{
		Georeference = ACesiumGeoreference::GetDefaultGeoreference(this);
	}
	Manager = nullptr;

	for (TActorIterator<ARegionalWeather> It(GetWorld()); It; ++It)
	{
		Manager = *It;
		break;
	}
	TArray<FWeatherRegionRequest> Requests;

	// 1) CIRCLE region (Rain Thunderstorm)
	{
		FWeatherRegionRequest R;
		R.Geometry = ERegionGeometry::Circle;
		R.Center = FLonLat(69.70, 24.90);
		R.RadiusMeters = 6000.0;
		R.CircleSegments = 48;
		R.Priority = 10;
		R.TransitionWidthCm = 15000.0f;
		R.VolumeAlpha = 1.0f;

		// Use the ASSET OBJECT path (X.X). The manager will sanitize even if you paste the combined string,
		// but this is the cleanest form.
		R.WeatherPresetReference =
			TEXT("/Game/UltraDynamicSky/Blueprints/Weather_Effects/Weather_Presets/Rain_Thunderstorm.Rain_Thunderstorm");

		Requests.Add(R);
	}

	// 2) BOX region (Sand Dust Storm) - using TopRight + BottomLeft lon/lat
	{
		FWeatherRegionRequest R;
		R.Geometry = ERegionGeometry::Box;
		R.BottomLeft = FLonLat(69.60, 24.80);
		R.TopRight = FLonLat(69.75, 24.95);
		R.Priority = 5;
		R.TransitionWidthCm = 20000.0f;
		R.VolumeAlpha = 1.0f;

		R.WeatherPresetReference =
			TEXT("/Game/UltraDynamicSky/Blueprints/Weather_Effects/Weather_Presets/Sand_Dust_Storm.Sand_Dust_Storm");

		Requests.Add(R);
	}

	// 3) POLYGON region (Rain Thunderstorm) - list of lon/lat points
	{
		FWeatherRegionRequest R;
		R.Geometry = ERegionGeometry::Polygon;
		R.PolygonPoints = {
			FLonLat(69.62, 24.88),
			FLonLat(69.66, 24.86),
			FLonLat(69.70, 24.89),
			FLonLat(69.66, 24.92),
		};
		R.Priority = 20;                 // polygon wins if overlaps
		R.TransitionWidthCm = 12000.0f;
		R.VolumeAlpha = 0.9f;

		// Reuse one of your known presets (you didn't provide Fog/Snow references).
		R.WeatherPresetReference =
			TEXT("/Game/UltraDynamicSky/Blueprints/Weather_Effects/Weather_Presets/Rain_Thunderstorm.Rain_Thunderstorm");

		Requests.Add(R);
	}

	// Submit all regions in one call
	TArray<AActor*> Spawned = Manager->SubmitRegions(Requests);

	UE_LOG(LogTemp, Warning, TEXT("[Caller] Spawned %d weather regions."), Spawned.Num());

}

static void DrawDebugSplineOutline(
	UWorld* World,
	USplineComponent* Spline,
	const FColor& Color,
	float Duration = 30.0f,
	float Thickness = 20.0f
)
{
	if (!World || !Spline) return;

	const int32 NumPts = Spline->GetNumberOfSplinePoints();
	if (NumPts < 2) return;

	for (int32 i = 0; i < NumPts; ++i)
	{
		const int32 Next = (i + 1) % NumPts;

		const FVector P0 = Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		const FVector P1 = Spline->GetLocationAtSplinePoint(Next, ESplineCoordinateSpace::World);

		DrawDebugLine(
			World,
			P0,
			P1,
			Color,
			false,
			Duration,
			0,
			Thickness
		);
	}
}


AActor* ARegionalWeather::SubmitRegion(const FWeatherRegionRequest& Request)
{
	if (!GetWorld())
	{
		return nullptr;
	}

	if (!Georeference)
	{
		UE_LOG(LogTemp, Error, TEXT("[RegionalWeather] Cesium Georeference is null."));
		return nullptr;
	}

	TSubclassOf<AActor> SpawnClass = ResolveSpawnClass();
	if (!SpawnClass)
	{
		UE_LOG(LogTemp, Error, TEXT("[RegionalWeather] Weather Override Volume spawn class could not be resolved. Set WeatherOverrideVolumeBlueprint."));
		return nullptr;
	}

	AActor* Vol = SpawnAndConfigureVolume(Request);
	if (Vol)
	{
		SpawnedVolumes.Add(Vol);
	}
	return Vol;
}

TArray<AActor*> ARegionalWeather::SubmitRegions(const TArray<FWeatherRegionRequest>& Requests)
{
	TArray<AActor*> Out;
	Out.Reserve(Requests.Num());
	for (const FWeatherRegionRequest& R : Requests)
	{
		if (AActor* V = SubmitRegion(R))
		{
			Out.Add(V);
		}
	}
	return Out;
}

void ARegionalWeather::DestroyAllSpawnedRegions()
{
	for (AActor* A : SpawnedVolumes)
	{
		if (IsValid(A))
		{
			A->Destroy();
		}
	}
	SpawnedVolumes.Reset();
}

TSubclassOf<AActor> ARegionalWeather::ResolveSpawnClass() const
{
	if (WeatherOverrideVolumeClassOverride)
	{
		return WeatherOverrideVolumeClassOverride;
	}

	UBlueprint* BP = WeatherOverrideVolumeBlueprint.LoadSynchronous();
	if (!BP)
	{
		return nullptr;
	}
	UClass* Gen = BP->GeneratedClass;
	if (!Gen)
	{
		return nullptr;
	}

	return Gen;
}

AActor* ARegionalWeather::SpawnAndConfigureVolume(const FWeatherRegionRequest& Request)
{
	const TSubclassOf<AActor> SpawnClass = ResolveSpawnClass();
	if (!SpawnClass)
	{
		return nullptr;
	}

	// Spawn/anchor point for the actor
	const FLonLat AnchorLonLat = GetAnchorPointForRequest(Request);

	FVector SpawnWorld;
	if (!LonLatHeightToUnreal(AnchorLonLat.Lon, AnchorLonLat.Lat, Request.HeightMeters, SpawnWorld))
	{
		UE_LOG(LogTemp, Error, TEXT("[RegionalWeather] Failed lon/lat->Unreal conversion for anchor."));
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* VolumeActor = GetWorld()->SpawnActor<AActor>(SpawnClass, SpawnWorld, FRotator::ZeroRotator, Params);
	if (!VolumeActor)
	{
		return nullptr;
	}

	if (bUseGlobeAnchor)
	{
		EnsureAnchor(VolumeActor, AnchorLonLat, Request.HeightMeters);
	}
	UE_LOG(LogTemp, Warning,
		TEXT("[RegionalWeather] Spawning volume for %s at anchor Lon=%.8f Lat=%.8f H=%.2f -> Unreal (%.2f, %.2f, %.2f)"),
		*UEnum::GetValueAsString(Request.Geometry),
		AnchorLonLat.Lon, AnchorLonLat.Lat, Request.HeightMeters,
		SpawnWorld.X, SpawnWorld.Y, SpawnWorld.Z
	);
	ApplyUDSProperties(VolumeActor, Request);
	BuildSpline(VolumeActor, Request);
#if WITH_EDITOR
	VolumeActor->RerunConstructionScripts();
#endif
	UE_LOG(LogTemp, Warning,
		TEXT("[RegionalWeather] Spawned VolumeActor=%s at WorldLocation (%.2f, %.2f, %.2f)"),
		*GetNameSafe(VolumeActor),
		VolumeActor->GetActorLocation().X, VolumeActor->GetActorLocation().Y, VolumeActor->GetActorLocation().Z
	);
	return VolumeActor;
}

void ARegionalWeather::ApplyUDSProperties(AActor* VolumeActor, const FWeatherRegionRequest& Request)
{
	if (!VolumeActor)
	{
		return;
	}

	UObject* PresetObj = LoadPresetObjectFromReference(Request.WeatherPresetReference);
	if (!PresetObj)
	{
		UE_LOG(LogTemp, Error, TEXT("[RegionalWeather] Could not load WeatherPresetReference: %s"), *Request.WeatherPresetReference);
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[RegionalWeather] Preset loaded: %s | class: %s"),
		*PresetObj->GetPathName(),
		*PresetObj->GetClass()->GetPathName()
	);

	const bool bWeatherOK = SetObjectPropertyByName(VolumeActor, TEXT("Weather"), PresetObj);
	if (!bWeatherOK)
	{
		UE_LOG(LogTemp, Error, TEXT("[RegionalWeather] Failed to set 'Weather' property on %s."), *VolumeActor->GetClass()->GetName());
	}

	SetFloatPropertyByName(VolumeActor, TEXT("TransitionWidth"), Request.TransitionWidthCm);
	SetIntPropertyByName(VolumeActor, TEXT("Priority"), Request.Priority);
	SetFloatPropertyByName(VolumeActor, TEXT("VolumeAlpha"), Request.VolumeAlpha);
}

bool ARegionalWeather::BuildSpline(AActor* VolumeActor, const FWeatherRegionRequest& Request)
{
	USplineComponent* Spline = FindSpline(VolumeActor);
	if (!Spline)
	{
		UE_LOG(LogTemp, Error, TEXT("[RegionalWeather] No Spline component found on spawned volume."));
		return false;
	}

	Spline->ClearSplinePoints(true);
	Spline->SetClosedLoop(true, true);

	const FVector ActorWorld = VolumeActor->GetActorLocation();

	auto AddPoint = [&](const FVector& WorldPoint)
		{
			if (bUseGlobeAnchor && bSplinePointsAreLocalOffsets)
			{
				Spline->AddSplinePoint(WorldPoint - ActorWorld, ESplineCoordinateSpace::Local, true);
			}
			else
			{
				Spline->AddSplinePoint(WorldPoint, ESplineCoordinateSpace::World, true);
			}
		};

	if (Request.Geometry == ERegionGeometry::Box)
	{
		const double MinLon = FMath::Min(Request.BottomLeft.Lon, Request.TopRight.Lon);
		const double MaxLon = FMath::Max(Request.BottomLeft.Lon, Request.TopRight.Lon);
		const double MinLat = FMath::Min(Request.BottomLeft.Lat, Request.TopRight.Lat);
		const double MaxLat = FMath::Max(Request.BottomLeft.Lat, Request.TopRight.Lat);

		const TArray<FLonLat> Corners = {
			FLonLat(MinLon, MinLat), // bottom-left
			FLonLat(MaxLon, MinLat), // bottom-right
			FLonLat(MaxLon, MaxLat), // top-right
			FLonLat(MinLon, MaxLat)  // top-left
		};

		for (const FLonLat& P : Corners)
		{
			FVector W;
			if (!LonLatHeightToUnreal(P.Lon, P.Lat, Request.HeightMeters, W))
			{
				return false;
			}
			AddPoint(W);
		}
	}
	else if (Request.Geometry == ERegionGeometry::Polygon)
	{
		if (Request.PolygonPoints.Num() < 3)
		{
			UE_LOG(LogTemp, Error, TEXT("[RegionalWeather] Polygon requires at least 3 points."));
			return false;
		}

		for (const FLonLat& P : Request.PolygonPoints)
		{
			FVector W;
			if (!LonLatHeightToUnreal(P.Lon, P.Lat, Request.HeightMeters, W))
			{
				return false;
			}
			AddPoint(W);
		}
	}
	else if (Request.Geometry == ERegionGeometry::Circle)
	{
		const int32 Segs = FMath::Clamp(Request.CircleSegments, 8, 256);
		for (int32 i = 0; i < Segs; ++i)
		{
			const double t = (2.0 * PI * double(i)) / double(Segs);
			const FLonLat Pt = DestinationPoint(Request.Center, t, Request.RadiusMeters);

			FVector W;
			if (!LonLatHeightToUnreal(Pt.Lon, Pt.Lat, Request.HeightMeters, W))
			{
				return false;
			}
			AddPoint(W);
		}
	}
	else
	{
		return false;
	}

	Spline->UpdateSpline();
	DrawDebugSplineOutline(
		GetWorld(),
		Spline,
		FColor::Green,  
		1000.0f,         
		100.0f           
	);

	return true;
}

USplineComponent* ARegionalWeather::FindSpline(AActor* VolumeActor)
{
	return VolumeActor ? VolumeActor->FindComponentByClass<USplineComponent>() : nullptr;
}

bool ARegionalWeather::LonLatHeightToUnreal(double LonDeg, double LatDeg, double HeightM, FVector& OutUnreal) const
{
	if (!Georeference)
	{
		return false;
	}
	const FVector Llh((float)LonDeg, (float)LatDeg, (float)HeightM);
	OutUnreal = Georeference->TransformLongitudeLatitudeHeightPositionToUnreal(Llh);
	UE_LOG(LogTemp, Warning,
		TEXT("[RegionalWeather] LonLatHeightToUnreal: Lon=%.8f Lat=%.8f H=%.2f -> Unreal X=%.2f Y=%.2f Z=%.2f"),
		LonDeg, LatDeg, HeightM,
		OutUnreal.X, OutUnreal.Y, OutUnreal.Z
	);
	return true;
}

static bool TryCallLonLatHeightSetter(UActorComponent* AnchorComp, double Lon, double Lat, double Height)
{
	if (!AnchorComp) return false;

	static const FName Candidates[] = {
		TEXT("SetLongitudeLatitudeHeight"),
		TEXT("SetLonLatHeight"),
		TEXT("SetGeodeticPosition"),
		TEXT("SetGeodeticCoordinates"),
		TEXT("MoveToLongitudeLatitudeHeight"),
		TEXT("K2_SetLongitudeLatitudeHeight"),
	};

	for (const FName FnName : Candidates)
	{
		if (UFunction* Fn = AnchorComp->FindFunction(FnName))
		{
			struct FParams
			{
				double Longitude;
				double Latitude;
				double Height;
			};

			FParams Params;
			Params.Longitude = Lon;
			Params.Latitude = Lat;
			Params.Height = Height;

			AnchorComp->ProcessEvent(Fn, &Params);
			return true;
		}
	}

	auto SetDoubleProp = [&](const TCHAR* PropName, double Value) -> bool
		{
			if (FProperty* P = AnchorComp->GetClass()->FindPropertyByName(FName(PropName)))
			{
				if (FDoubleProperty* DP = CastField<FDoubleProperty>(P))
				{
					DP->SetPropertyValue_InContainer(AnchorComp, Value);
					return true;
				}
				if (FFloatProperty* FP = CastField<FFloatProperty>(P))
				{
					FP->SetPropertyValue_InContainer(AnchorComp, (float)Value);
					return true;
				}
			}
			return false;
		};

	bool bAny = false;
	bAny |= SetDoubleProp(TEXT("Longitude"), Lon);
	bAny |= SetDoubleProp(TEXT("Latitude"), Lat);
	bAny |= SetDoubleProp(TEXT("Height"), Height);

	return bAny;
}

FVector ARegionalWeather::EnsureAnchor(AActor* VolumeActor, const FLonLat& AnchorLonLat, double HeightM) const
{
	if (!VolumeActor)
	{
		return FVector::ZeroVector;
	}
	UActorComponent* AnchorComp = nullptr;
	if (UCesiumGlobeAnchorComponent* Typed = VolumeActor->FindComponentByClass<UCesiumGlobeAnchorComponent>())
	{
		AnchorComp = Typed;
	}
	else
	{
		UCesiumGlobeAnchorComponent* NewAnchor = NewObject<UCesiumGlobeAnchorComponent>(
			VolumeActor,
			UCesiumGlobeAnchorComponent::StaticClass(),
			TEXT("CesiumGlobeAnchor")
		);

		if (NewAnchor)
		{
			NewAnchor->RegisterComponent();
			AnchorComp = NewAnchor;
		}
	}
	if (AnchorComp)
	{
		const bool bSetOK = TryCallLonLatHeightSetter(AnchorComp, AnchorLonLat.Lon, AnchorLonLat.Lat, HeightM);
		if (bSetOK)
		{
			return VolumeActor->GetActorLocation();
		}

		UE_LOG(LogTemp, Warning, TEXT("[RegionalWeather] GlobeAnchor exists but no supported lon/lat/height setter found. Falling back to direct placement."));
	}

	if (Georeference)
	{
		FVector World;
		if (LonLatHeightToUnreal(AnchorLonLat.Lon, AnchorLonLat.Lat, HeightM, World))
		{
			VolumeActor->SetActorLocation(World);
			return World;
		}
	}

	return VolumeActor->GetActorLocation();
}


FLonLat ARegionalWeather::GetAnchorPointForRequest(const FWeatherRegionRequest& Request)
{
	switch (Request.Geometry)
	{
	case ERegionGeometry::Circle:
		return Request.Center;

	case ERegionGeometry::Box:
	{
		const double MinLon = FMath::Min(Request.BottomLeft.Lon, Request.TopRight.Lon);
		const double MaxLon = FMath::Max(Request.BottomLeft.Lon, Request.TopRight.Lon);
		const double MinLat = FMath::Min(Request.BottomLeft.Lat, Request.TopRight.Lat);
		const double MaxLat = FMath::Max(Request.BottomLeft.Lat, Request.TopRight.Lat);
		return FLonLat((MinLon + MaxLon) * 0.5, (MinLat + MaxLat) * 0.5);
	}

	case ERegionGeometry::Polygon:
	{
		if (Request.PolygonPoints.Num() == 0)
		{
			return FLonLat(0.0, 0.0);
		}
		double SumLon = 0.0, SumLat = 0.0;
		for (const FLonLat& P : Request.PolygonPoints)
		{
			SumLon += P.Lon;
			SumLat += P.Lat;
		}
		return FLonLat(SumLon / Request.PolygonPoints.Num(), SumLat / Request.PolygonPoints.Num());
	}
	default:
		return FLonLat(0.0, 0.0);
	}
}

FString ARegionalWeather::ExtractObjectPathFromAnyReference(const FString& InRef)
{
	int32 GameIdx = InRef.Find(TEXT("/Game/"), ESearchCase::IgnoreCase, ESearchDir::FromStart);
	if (GameIdx == INDEX_NONE)
	{
		return FString();
	}
	FString Tail = InRef.Mid(GameIdx);

	int32 QuoteIdx = Tail.Find(TEXT("'"), ESearchCase::IgnoreCase, ESearchDir::FromStart);
	if (QuoteIdx != INDEX_NONE)
	{
		Tail = Tail.Left(QuoteIdx);
	}
	Tail.TrimStartAndEndInline();

	return Tail;
}

UObject* ARegionalWeather::LoadPresetObjectFromReference(const FString& AnyRef)
{
	const FString ObjPath = ExtractObjectPathFromAnyReference(AnyRef);
	if (ObjPath.IsEmpty())
	{
		return nullptr;
	}
	return LoadObject<UObject>(nullptr, *ObjPath);
}

bool ARegionalWeather::SetObjectPropertyByName(AActor* Target, const FName PropertyName, UObject* Value)
{
	if (!Target)
	{
		return false;
	}

	FProperty* Prop = Target->GetClass()->FindPropertyByName(PropertyName);
	if (!Prop)
	{
		return false;
	}

	FObjectProperty* ObjProp = CastField<FObjectProperty>(Prop);
	if (!ObjProp)
	{
		return false;
	}

	ObjProp->SetObjectPropertyValue_InContainer(Target, Value);
	return true;
}

bool ARegionalWeather::SetFloatPropertyByName(AActor* Target, const FName PropertyName, float Value)
{
	if (!Target)
	{
		return false;
	}

	FProperty* Prop = Target->GetClass()->FindPropertyByName(PropertyName);
	if (!Prop)
	{
		return false;
	}

	FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop);
	if (!FloatProp)
	{
		return false;
	}

	FloatProp->SetPropertyValue_InContainer(Target, Value);
	return true;
}

bool ARegionalWeather::SetIntPropertyByName(AActor* Target, const FName PropertyName, int32 Value)
{
	if (!Target)
	{
		return false;
	}

	FProperty* Prop = Target->GetClass()->FindPropertyByName(PropertyName);
	if (!Prop)
	{
		return false;
	}

	FIntProperty* IntProp = CastField<FIntProperty>(Prop);
	if (!IntProp)
	{
		return false;
	}

	IntProp->SetPropertyValue_InContainer(Target, Value);
	return true;
}

FLonLat ARegionalWeather::DestinationPoint(const FLonLat& Start, double BearingRadians, double DistanceMeters)
{
	const double R = 6378137.0;

	const double lat1 = FMath::DegreesToRadians(Start.Lat);
	const double lon1 = FMath::DegreesToRadians(Start.Lon);
	const double dByR = DistanceMeters / R;

	const double sinLat1 = FMath::Sin(lat1);
	const double cosLat1 = FMath::Cos(lat1);
	const double sinD = FMath::Sin(dByR);
	const double cosD = FMath::Cos(dByR);

	const double sinLat2 = sinLat1 * cosD + cosLat1 * sinD * FMath::Cos(BearingRadians);
	const double lat2 = FMath::Asin(sinLat2);

	const double y = FMath::Sin(BearingRadians) * sinD * cosLat1;
	const double x = cosD - sinLat1 * sinLat2;
	const double lon2 = lon1 + FMath::Atan2(y, x);

	FLonLat Out;
	Out.Lat = FMath::RadiansToDegrees(lat2);
	Out.Lon = FMath::RadiansToDegrees(lon2);
	if (Out.Lon > 180.0) Out.Lon -= 360.0;
	if (Out.Lon < -180.0) Out.Lon += 360.0;

	return Out;
}
