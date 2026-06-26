#include "EnvironmentManager.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Components/VolumetricCloudComponent.h"
#include "UObject/Class.h"
#include "CesiumGeoreference.h"
#include "UObject/UnrealType.h"
#include "WeatherDialog.h"
#include "UIEnvironmentController.h"
#include "EnvironmentControllerBase.h"


static FString NormalizePropName(const FString& s) {
	FString out; out.Reserve(s.Len());
	for (TCHAR c : s) if (FChar::IsAlnum(c)) out.AppendChar(FChar::ToLower(c));
	return out;
}

static FString AliasEnumToBPName(const FString& name) {
	if (name == TEXT("TimeofDay"))   return TEXT("Time Of Day");
	if (name == TEXT("FogShadows"))  return TEXT("Fog Shadows");
	if (name == TEXT("DustShadows")) return TEXT("Dust Shadows");
	if (name == TEXT("AnimateTimeOfDay")) return TEXT("Animate Time Of Day");
	if (name == TEXT("TimeSpeed")) return TEXT("Time Speed");
	return name;
}

static void ApplyVis(AActor* A, bool bVisible)
{
	if (!A) return;
	A->SetActorHiddenInGame(!bVisible);
	A->SetActorEnableCollision(bVisible);
	A->SetActorTickEnabled(bVisible);
	if (UPrimitiveComponent* PC = Cast<UPrimitiveComponent>(A->GetRootComponent()))
	{
		PC->SetVisibility(bVisible, true);
		PC->SetHiddenInGame(!bVisible, true);
		PC->MarkRenderStateDirty();
	}
	//setting visibility in the editor
#if WITH_EDITOR
	A->SetIsTemporarilyHiddenInEditor(!bVisible);
#endif
}

template<typename TEnum, typename TValue>
void UEnvironmentManager::SetEnvironmentProperty(TEnum EnumKey, const TValue& NewValue)
{
	const FString rawName = StaticEnum<TEnum>()->GetNameStringByValue((int64)EnumKey);
	const FString niceLabel = StaticEnum<TEnum>()->GetDisplayNameTextByValue((int64)EnumKey).ToString();
	FString wanted = AliasEnumToBPName(niceLabel);
	if (wanted == niceLabel) wanted = AliasEnumToBPName(rawName);

	UObject* Target = nullptr;
	if constexpr (std::is_same_v<TEnum, ESky_Properties>)          Target = UltraDynamicSkyActor;
	else if constexpr (std::is_same_v<TEnum, EWeather_Properties>) Target = UltraDynamicWeatherActor;
	if (!Target) { UE_LOG(LogTemp, Warning, TEXT("Target is null for key %s/%s"), *niceLabel, *rawName); return; }

	FProperty* Property = Target->GetClass()->FindPropertyByName(FName(*wanted));

	if (!Property) {
		const FString key = NormalizePropName(wanted);
		for (TFieldIterator<FProperty> It(Target->GetClass()); It; ++It) {
			if (NormalizePropName(It->GetName()) == key) { Property = *It; break; }
		}
	}

	if (!Property) {
		UE_LOG(LogTemp, Warning, TEXT("Property '%s'/'%s' not found on %s"),
			*niceLabel, *rawName, *Target->GetName());
		return;
	}

	const FName PropertyName(*Property->GetName()); // keep your existing type write code below this line
	//FNumericProperty is a subclass of FProperty and only deals with numeric values like int, float, double etc.
	// Handle float
	if constexpr (std::is_same_v<TValue, float>)
	{
		if (FNumericProperty* NumProp = CastField<FNumericProperty>(Property))
		{
			NumProp->SetFloatingPointPropertyValue(NumProp->ContainerPtrToValuePtr<void>(Target), NewValue);
			UE_LOG(LogTemp, Log, TEXT("Set %s to %f on %s"), *PropertyName.ToString(), NewValue, *Target->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Property '%s' is not a float!"), *PropertyName.ToString());
		}
	}
	else if constexpr (std::is_same_v<TValue, FString>)
	{
		if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
		{
			StrProp->SetPropertyValue(StrProp->ContainerPtrToValuePtr<void>(Target), *NewValue);
			UE_LOG(LogTemp, Log, TEXT("Set %s to '%s' on %s"), *PropertyName.ToString(), *NewValue, *Target->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Property '%s' is not a string!"), *PropertyName.ToString());
		}
	}

	else if constexpr (std::is_same_v<TValue, FLinearColor>)
	{
		if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
		{
			if (StructProp->Struct == TBaseStructure<FLinearColor>::Get())
			{
				void* ValuePtr = StructProp->ContainerPtrToValuePtr<void>(Target);
				*reinterpret_cast<FLinearColor*>(ValuePtr) = NewValue;
				UE_LOG(LogTemp, Log, TEXT("Set %s to RGBA (%s) on %s"), *PropertyName.ToString(), *NewValue.ToString(), *Target->GetName());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Property '%s' is a struct but not FLinearColor!"), *PropertyName.ToString());
			}
		}

		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Property '%s' is not a struct (FLinearColor)!"), *PropertyName.ToString());
		}
	}
	else if constexpr (std::is_same_v<TValue, bool>)
	{
		if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
		{
			BoolProp->SetPropertyValue(BoolProp->ContainerPtrToValuePtr<void>(Target), NewValue);
			UE_LOG(LogTemp, Log, TEXT("Set %s to %s on %s"), *PropertyName.ToString(), NewValue ? TEXT("true") : TEXT("false"), *Target->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Property '%s' is not a bool!"), *PropertyName.ToString());
		}
	}
	else if constexpr (std::is_enum<TValue>::value)
	{
		if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
		{
			// Convert the enum value to its underlying byte/int value
			void* ValuePtr = EnumProp->ContainerPtrToValuePtr<void>(Target);
			EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, static_cast<int64>(NewValue));
			UE_LOG(LogTemp, Log, TEXT("Set %s to enum %d on %s"), *PropertyName.ToString(), static_cast<int32>(NewValue), *Target->GetName());
		}
		else if (FByteProperty* ByteProp = CastField<FByteProperty>(Property))
		{
			// Handle byte-backed enums (BP enums like EUDSColorMode)
			ByteProp->SetPropertyValue(ByteProp->ContainerPtrToValuePtr<void>(Target), static_cast<uint8>(NewValue));
			UE_LOG(LogTemp, Log, TEXT("Set %s to enum(byte) %d on %s"), *PropertyName.ToString(), static_cast<uint8>(NewValue), *Target->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Property '%s' is not an enum/byte!"), *PropertyName.ToString());
		}
	}

	//-----IMTEGER----
	else if constexpr (std::is_same_v<TValue, int32>)
	{
		if (FNumericProperty* NumProp = CastField<FNumericProperty>(Property))
		{
			void* ValuePtr = NumProp->ContainerPtrToValuePtr<void>(Target);
			if (NumProp->IsInteger())
			{
				NumProp->SetIntPropertyValue(ValuePtr, static_cast<int64>(NewValue));
			}
			else if (NumProp->IsFloatingPoint())
			{
				NumProp->SetFloatingPointPropertyValue(ValuePtr, static_cast<double>(NewValue));
			}
#if WITH_EDITOR
			Target->Modify();
			FPropertyChangedEvent ChangeEvent(Property, EPropertyChangeType::ValueSet);
			Target->PostEditChangeProperty(ChangeEvent);
			if (AActor* Actor = Cast<AActor>(Target))
			{
				//Actor->RerunConstructionScripts();
				Actor->MarkPackageDirty();
			}
#endif
			UE_LOG(LogTemp, Log, TEXT("Set %s to %d on %s"), *PropertyName.ToString(), NewValue, *Target->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Property '%s' is not numeric!"), *PropertyName.ToString());
		}
	}

	//-----DOUBLE
	else if constexpr (std::is_same_v<TValue, double>)
	{
		if (FNumericProperty* NumProp = CastField<FNumericProperty>(Property))
		{
			void* ValuePtr = NumProp->ContainerPtrToValuePtr<void>(Target);
			if (NumProp->IsFloatingPoint())
			{
				NumProp->SetFloatingPointPropertyValue(ValuePtr, static_cast<double>(NewValue));
			}
			else if (NumProp->IsInteger())
			{
				NumProp->SetIntPropertyValue(ValuePtr, static_cast<int64>(NewValue));
			}
#if WITH_EDITOR
			Target->Modify();
			FPropertyChangedEvent ChangeEvent(Property, EPropertyChangeType::ValueSet);
			Target->PostEditChangeProperty(ChangeEvent);
			if (AActor* Actor = Cast<AActor>(Target))
			{
				//Actor->RerunConstructionScripts();
				Actor->MarkPackageDirty();
			}
#endif
			UE_LOG(LogTemp, Log, TEXT("Set %s to %f on %s"), *PropertyName.ToString(), NewValue, *Target->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Property '%s' is not numeric!"), *PropertyName.ToString());
		}
	}
}

template<typename TEnum, typename TValue>
bool UEnvironmentManager::GetEnvironmentProperty(TEnum EnumKey, TValue& OutValue) const
{
	const FString rawName = StaticEnum<TEnum>()->GetNameStringByValue((int64)EnumKey);
	const FString niceLabel = StaticEnum<TEnum>()->GetDisplayNameTextByValue((int64)EnumKey).ToString();

	FString wanted = AliasEnumToBPName(niceLabel);
	if (wanted == niceLabel) wanted = AliasEnumToBPName(rawName);

	UObject* Target = nullptr;
	if constexpr (std::is_same_v<TEnum, ESky_Properties>)          Target = UltraDynamicSkyActor;
	else if constexpr (std::is_same_v<TEnum, EWeather_Properties>) Target = UltraDynamicWeatherActor;

	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("Get: Target is null for key %s/%s"), *niceLabel, *rawName);
		return false;
	}

	FProperty* Property = Target->GetClass()->FindPropertyByName(FName(*wanted));

	if (!Property)
	{
		const FString key = NormalizePropName(wanted);
		for (TFieldIterator<FProperty> It(Target->GetClass()); It; ++It)
		{
			if (NormalizePropName(It->GetName()) == key)
			{
				Property = *It;
				break;
			}
		}
	}

	if (!Property)
	{
		UE_LOG(LogTemp, Warning, TEXT("Get: Property '%s'/'%s' not found on %s"),
			*niceLabel, *rawName, *Target->GetName());
		return false;
	}

	const FName PropertyName(*Property->GetName());

	// ---- FLOAT ----
	if constexpr (std::is_same_v<TValue, float>)
	{
		if (FNumericProperty* NumProp = CastField<FNumericProperty>(Property))
		{
			void* ValuePtr = NumProp->ContainerPtrToValuePtr<void>(Target);

			// Some numeric props are int; accept both.
			if (NumProp->IsFloatingPoint())
				OutValue = (float)NumProp->GetFloatingPointPropertyValue(ValuePtr);
			else
				OutValue = (float)NumProp->GetSignedIntPropertyValue(ValuePtr);

			UE_LOG(LogTemp, Log, TEXT("Get %s = %f on %s"), *PropertyName.ToString(), OutValue, *Target->GetName());
			return true;
		}

		UE_LOG(LogTemp, Warning, TEXT("Get: Property '%s' is not numeric/float"), *PropertyName.ToString());
		return false;
	}

	// ---- BOOL ----
	else if constexpr (std::is_same_v<TValue, bool>)
	{
		if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Property))
		{
			OutValue = BoolProp->GetPropertyValue(BoolProp->ContainerPtrToValuePtr<void>(Target));
			UE_LOG(LogTemp, Log, TEXT("Get %s = %s on %s"), *PropertyName.ToString(), OutValue ? TEXT("true") : TEXT("false"), *Target->GetName());
			return true;
		}

		UE_LOG(LogTemp, Warning, TEXT("Get: Property '%s' is not bool"), *PropertyName.ToString());
		return false;
	}

	// ---- STRING ----
	else if constexpr (std::is_same_v<TValue, FString>)
	{
		if (FStrProperty* StrProp = CastField<FStrProperty>(Property))
		{
			OutValue = StrProp->GetPropertyValue(StrProp->ContainerPtrToValuePtr<void>(Target));
			UE_LOG(LogTemp, Log, TEXT("Get %s = '%s' on %s"), *PropertyName.ToString(), *OutValue, *Target->GetName());
			return true;
		}

		UE_LOG(LogTemp, Warning, TEXT("Get: Property '%s' is not FString"), *PropertyName.ToString());
		return false;
	}

	// ---- LINEAR COLOR ----
	else if constexpr (std::is_same_v<TValue, FLinearColor>)
	{
		if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
		{
			if (StructProp->Struct == TBaseStructure<FLinearColor>::Get())
			{
				void* ValuePtr = StructProp->ContainerPtrToValuePtr<void>(Target);
				OutValue = *reinterpret_cast<FLinearColor*>(ValuePtr);

				UE_LOG(LogTemp, Log, TEXT("Get %s = RGBA (%s) on %s"),
					*PropertyName.ToString(), *OutValue.ToString(), *Target->GetName());
				return true;
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("Get: Property '%s' is not FLinearColor"), *PropertyName.ToString());
		return false;
	}

	// ---- ENUM (C++ or BP byte enum) ----
	else if constexpr (std::is_enum_v<TValue>)
	{
		// C++ enum stored as EnumProperty
		if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Property))
		{
			void* ValuePtr = EnumProp->ContainerPtrToValuePtr<void>(Target);
			const int64 Raw = EnumProp->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr);
			OutValue = static_cast<TValue>(Raw);

			UE_LOG(LogTemp, Log, TEXT("Get %s = enum %lld on %s"),
				*PropertyName.ToString(), Raw, *Target->GetName());
			return true;
		}

		// BP enums are often stored as ByteProperty
		if (FByteProperty* ByteProp = CastField<FByteProperty>(Property))
		{
			const uint8 Raw = ByteProp->GetPropertyValue(ByteProp->ContainerPtrToValuePtr<void>(Target));
			OutValue = static_cast<TValue>(Raw);

			UE_LOG(LogTemp, Log, TEXT("Get %s = enum(byte) %u on %s"),
				*PropertyName.ToString(), Raw, *Target->GetName());
			return true;
		}

		UE_LOG(LogTemp, Warning, TEXT("Get: Property '%s' is not an enum/byte"), *PropertyName.ToString());
		return false;
	}

	else if constexpr (std::is_same_v<TValue, int32>)
	{
		if (FNumericProperty* NumProp = CastField<FNumericProperty>(Property))
		{
			void* ValuePtr = NumProp->ContainerPtrToValuePtr<void>(Target);
			OutValue = (int32)NumProp->GetSignedIntPropertyValue(ValuePtr);

			UE_LOG(LogTemp, Log, TEXT("Get %s = %d on %s"), *PropertyName.ToString(), OutValue, *Target->GetName());
			return true;
		}

		UE_LOG(LogTemp, Warning, TEXT("Get: Property '%s' is not numeric/int"), *PropertyName.ToString());
		return false;
	}

	//UE_LOG(LogTemp, Warning, TEXT("Get: Unsupported TValue for property '%s'"), *PropertyName.ToString());
	return false;
}

void UEnvironmentManager::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	for (TActorIterator<AActor> It(InWorld.GetWorld()); It; ++It)
	{
		if (It->ActorHasTag("Ultra_Dynamic_Sky"))
		{
			UltraDynamicSkyActor = *It;
			UE_LOG(LogTemp, Warning, TEXT("Found Ultra Dynamic Sky Actor: %s"), *UltraDynamicSkyActor->GetName());
			break;
		}

	}
	for (TActorIterator<AActor> It(InWorld.GetWorld()); It; ++It)
	{
		if (It->ActorHasTag("Ultra_Dynamic_Weather"))
		{
			UltraDynamicWeatherActor = *It;
			UE_LOG(LogTemp, Warning, TEXT("Found Ultra Dynamic Weather Actor: %s"), *UltraDynamicWeatherActor->GetName());
			break;
		}
	}

	EnvController = NewObject<UUIEnvironmentController>(this/*InWorld.GetWorld()*/);
	EnvController->Initialize();
	EnvControllerBase = NewObject<UEnvironmentControllerBase>(this);
	EnvControllerBase->Initialize();

	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	UltraDynamicSkyActor->SetActorLocation(PlayerPawn->GetActorLocation());
	UltraDynamicWeatherActor->SetActorLocation(PlayerPawn->GetActorLocation());

	MPC_Wind = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Materials/Collections/MPC_Wind.MPC_Wind"));
	if (MPC_Wind)
	{
		MPC_Wind_Inst = GetWorld()->GetParameterCollectionInstance(MPC_Wind);
	}
	FVector SpawnLocation(0.f, 0.f, 0.f);   // where to place it@
	FRotator SpawnRotation(0.f, 0.f, 0.f); // rotation
	PostProcessVolume = InWorld.GetWorld()->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), SpawnLocation, SpawnRotation);
#if WITH_EDITOR
	PostProcessVolume->SetActorLabel("Light POST PROCESS");
#endif
	// settings for it 
	PostProcessVolume->bUnbound = true;
	PostProcessVolume->Priority = 100.f;
	PostProcessVolume->BlendWeight = 1.f;

	//New settigs done for flickering issue
	PostProcessVolume->Settings.bOverride_LumenFinalGatherQuality = true;
	PostProcessVolume->Settings.LumenFinalGatherQuality = true;
	PostProcessVolume->Settings.bOverride_LumenFinalGatherScreenTraces = true;
	PostProcessVolume->Settings.LumenFinalGatherScreenTraces = false;

	Cube = LoadObject<UTextureCube>(nullptr, TEXT("/Engine/EngineResources/DefaultTextureCube.DefaultTextureCube"));

	if (Cube)
	{
		FPostProcessSettings& S = PostProcessVolume->Settings;
		S.bOverride_AmbientCubemapTint = true;
		S.bOverride_AmbientCubemapIntensity = true;

		// --- Exposure: Manual + Compensation +7 EV ---
		S.bOverride_AutoExposureMethod = true;                 // enable override
		S.bOverride_AutoExposureBias = true; //it's display name is "Exposure Compensation"

		S.AmbientCubemap = Cube;
		S.AmbientCubemapTint = FLinearColor::White;
		S.AmbientCubemapIntensity = 1.0f;
		S.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
		S.AutoExposureBias = 10.0f;
	}

	SkySphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/MapTemplates/Sky/SM_SkySphere.SM_SkySphere"));
	NewMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/SkyBox/BlueSky_Inst.BlueSky_Inst"));
	NewMaterial2 = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/SkyBox/BlackSky_Inst.BlackSky_Inst"));
	UE_LOG(LogTemp, Warning, TEXT("SkySphereMesh=%p  Blue=%p  Black=%p"),SkySphereMesh, NewMaterial, NewMaterial2);
	SkySphere1 = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, SpawnRotation);
	if (SkySphere1 && SkySphereMesh)
	{
		UStaticMeshComponent* Comp = SkySphere1->GetStaticMeshComponent();
		Comp->SetMobility(EComponentMobility::Movable);
		SkySphere1->GetStaticMeshComponent()->SetStaticMesh(SkySphereMesh);
		FVector SkyScale(500000.f, 500000.f, 500000.f);
		SkySphere1->SetActorScale3D(SkyScale);
	}

	if (SkySphere1)
	{
		SkySphere1->GetStaticMeshComponent()->SetMaterial(0, NewMaterial);  
		UE_LOG(LogTemp, Warning, TEXT("Applied new material to %s"), *SkySphere1->GetName());
	}
	SetSkySphere1Visible(false);
	DirectionalLightVisible(false);
	SetPPVVisible(false);
	SetUDSVisible(true);
	Date = FDateTime::UtcNow();
	SetEnvironmentProperty(ESky_Properties::Day, Date.GetDay());
	SetEnvironmentProperty(ESky_Properties::Month, Date.GetMonth());
	SetEnvironmentProperty(ESky_Properties::Year, Date.GetYear());
	UE_LOG(LogTemp, Warning, TEXT("Applied date %d %d %d"), Date.GetDay(), Date.GetMonth(), Date.GetYear());

	SetEnvironmentProperty(ESky_Properties::SimulateRealSun,true);
	//SetEnvironmentProperty(ESky_Properties::SimulateRealMoon,true);
	ACesiumGeoreference* Georef = ACesiumGeoreference::GetDefaultGeoreference(this);
	double Lon = Georef->GetOriginLongitude();
	double Lat = Georef->GetOriginLatitude();
	SetEnvironmentProperty(ESky_Properties::Latitude, Lat);
	SetEnvironmentProperty(ESky_Properties::Longitude, Lon);

}

void UEnvironmentManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	if (!EnvController) {
		EnvController = NewObject<UUIEnvironmentController>(this);
		EnvController->Initialize();
	}
	if (!EnvControllerBase) {
		EnvControllerBase = NewObject<UEnvironmentControllerBase>(this);
		EnvControllerBase->Initialize();
	}



}

void UEnvironmentManager::Deinitialize()
{
	if (WeatherDialogWindow.IsValid()) {
		//  FSlateApplication::Get().RequestDestroyWindow(WeatherDialogWindow.Pin().ToSharedRef());
		FSlateApplication::Get().RequestDestroyWindow(WeatherDialogWindow.ToSharedRef());
	}
	UEnvironmentControllerBase::ResetData();
	Super::Deinitialize();
}

void UEnvironmentManager::createEnvironmnetDialog()
{
	if (TSharedPtr<SWindow> Existing = WeatherDialogWindow)
	{
		Existing->BringToFront();
		return;
	}
	else
	{
		WeatherDialogWindow = SNew(SWindow)
			.Title(FText::FromString("Environment Settings"))
			.ClientSize(FVector2D(800, 400))
			.FocusWhenFirstShown(false)
			.IsTopmostWindow(false)
			.Content()
			[
				SNew(SWeatherDialog).EnvironmentController(EnvController)
			];

		FSlateApplication::Get().AddWindow(WeatherDialogWindow.ToSharedRef());
		FSlateApplication::Get().SetAllUserFocusToGameViewport(); FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
		WeatherDialogWindow->SetOnWindowClosed(FOnWindowClosed::CreateLambda([this](const TSharedRef<SWindow>&) {
			WeatherDialogWindow.Reset();
			})
		);
		FEnvironmentData* Data = EnvController->GetEnvironmentData();
		Data->SelectedNavButton = ENavButton::TimeofDay;

		// FOR TESTING ONLY
		FRectangularRecord_1* AOI = new FRectangularRecord_1();
		AOI->Key = FString(TEXT("Global"));
		EnvController->AddAreaOfInterest(AOI);
		EnvController->SetCurrentAOI(AOI->Key);
		EnvController->GetEnvironmentData()->SetCurrentScenarioEnv(EnvController->GetEnvironmentData()->LocalScenarioEnvironment);
	}

}

void UEnvironmentManager::ApplyEnvironment(const FEnvironmentNormalizedData& Data)
{
	SetTimeSettings(Data);
	SetLightSettings(Data);
	//SetPrecipitationSettings(Data);
	//SetCloudsSettings(Data);
	//SetWindSettings(Data);
}

void UEnvironmentManager::SetTimeSettings(const FEnvironmentNormalizedData& Data)
{
	SetEnvironmentProperty(ESky_Properties::TimeofDay, Data.TimeOfDay);
	SetEnvironmentProperty(ESky_Properties::Day, Data.Day);
	SetEnvironmentProperty(ESky_Properties::Month, Data.Month);
	SetEnvironmentProperty(ESky_Properties::Year, Data.Year);
	SetEnvironmentProperty(ESky_Properties::SimulateRealSun, Data.SimulateRealSun);
	//SetEnvironmentProperty(ESky_Properties::SimulateRealMoon, Data.SimulateRealMoon);
}

void UEnvironmentManager::SetAnimateSettings(const FEnvironmentNormalizedData& Data)
{
	SetEnvironmentProperty(ESky_Properties::AnimateTimeOfDay, Data.AnimateTimeOfDay);
	SetEnvironmentProperty(ESky_Properties::TimeSpeed, Data.TimeSpeed);
}

void UEnvironmentManager::SetTimeDateToUDS(const FEnvironmentNormalizedData& Data)
{
	SetEnvironmentProperty(ESky_Properties::Month, Data.Month);
	SetEnvironmentProperty(ESky_Properties::Year, Data.Year);
	SetEnvironmentProperty(ESky_Properties::Day, Data.Day);

	SetTimeSettings(Data);
}

TTuple<float, float, float, float> UEnvironmentManager::GetTimeDatefromUDS()
{
	float Month = 0.0f;
	float Year = 0.0f;
	float Day = 0.0f;
	float TimeOfDay = 0.0f;
	GetEnvironmentProperty(ESky_Properties::TimeofDay, TimeOfDay);
	GetEnvironmentProperty(ESky_Properties::Month, Month);
	GetEnvironmentProperty(ESky_Properties::Year, Year);
	GetEnvironmentProperty(ESky_Properties::Day, Day);
	return { TimeOfDay, Month, Year, Day };
}

float UEnvironmentManager::GetSpeedOfTime()
{
	float TimeSpeed = 0.0f;
	GetEnvironmentProperty(ESky_Properties::TimeSpeed, TimeSpeed);
	return TimeSpeed;
}

void UEnvironmentManager::SetLightSettings(const FEnvironmentNormalizedData& Data)
{
	SetEnvironmentProperty(ESky_Properties::DawnTime, Data.DawnTime);
	SetEnvironmentProperty(ESky_Properties::DuskTime, Data.DuskTime);
}

void UEnvironmentManager::SetMoonSettings(const FEnvironmentNormalizedData& Data)
{
	SetEnvironmentProperty(ESky_Properties::MoonPhase, Data.MoonPhase);
}

void UEnvironmentManager::SetPrecipitationSettings(const FEnvironmentNormalizedData& Data, FAreaOfInterest* AOI)
{
	SetEnvironmentProperty(EWeather_Properties::Rain, Data.Rain);
	SetEnvironmentProperty(EWeather_Properties::WindIntensity, Data.WindIntensity);
	SetEnvironmentProperty(EWeather_Properties::ThunderLightning, Data.ThunderLightning);
	SetEnvironmentProperty(EWeather_Properties::Snow, Data.Snow);
	SetEnvironmentProperty(EWeather_Properties::Dust, Data.Dust);
}

void UEnvironmentManager::SetCloudsSettings(const FEnvironmentNormalizedData& Data, FAreaOfInterest* AOI, FCloudLayerData& CloudLayer)
{
	SetEnvironmentProperty(EWeather_Properties::CloudCoverage, Data.CloudCoverage);
	SetEnvironmentProperty(ESky_Properties::BottomAltitude, Data.BottomAltitude);
	SetEnvironmentProperty(ESky_Properties::VolumetricCloudScale, Data.VolumetricCloudScale);
	SetEnvironmentProperty(ESky_Properties::LayerHeightScale, Data.LayerHeightScale);
	SetEnvironmentProperty(ESky_Properties::NoiseScale3D, Data.NoiseScale3D);
	SetEnvironmentProperty(ESky_Properties::ErosionIntensity3D, Data.ErosionIntensity3D);
	SetEnvironmentProperty(ESky_Properties::ExtinctionScale, Data.ExtinctionScale);
	SetEnvironmentProperty(ESky_Properties::CloudWispsOpacityCloudy, Data.CloudWispsOpacity);
	SetEnvironmentProperty(ESky_Properties::CloudWispsOpacityClear, Data.CloudWispsOpacityClear);
	SetEnvironmentProperty(ESky_Properties::RayleighColor, Data.RayleighColor);
	SetEnvironmentProperty(ESky_Properties::StaticCloudsTexture, Data.StaticCloudsTexture);
	SetEnvironmentProperty(ESky_Properties::StaticCloudsRotationSpeed, Data.StaticCloudsRotationSpeed);
	SetEnvironmentProperty(ESky_Properties::StaticCloudsColorIntensity, Data.StaticCloudsColorIntensity);
	SetEnvironmentProperty(ESky_Properties::CloudWispsColorIntensity, Data.CloudWispsColorIntensity);
	SetEnvironmentProperty(ESky_Properties::BaseHeightFogFalloff, Data.BaseHeightFogFalloff);
	//SetEnvironmentProperty(EWeather_Properties::ThunderLightning, Data.ThunderLightning);
	SetEnvironmentProperty(ESky_Properties::CloudFormationTextureScale, Data.CloudFormationTextureScale);
	SetEnvironmentProperty(ESky_Properties::CloudSpeed, Data.CloudSpeed);
	SetEnvironmentProperty(ESky_Properties::CloudDirection, Data.CloudDirection);
	SetEnvironmentProperty(ESky_Properties::TracingMaxStartDistance, Data.TracingMaxStartDistance);
}

void UEnvironmentManager::SetWindSettings(const FEnvironmentNormalizedData& Data, FAreaOfInterest* AOI, FWindLayerData& WindLayer)
{
	SetEnvironmentProperty(EWeather_Properties::WindIntensity, Data.WindIntensity);
	SetEnvironmentProperty(EWeather_Properties::WindDirection, Data.WindDirection);

	if (MPC_Wind_Inst) {
		MPC_Wind_Inst->SetScalarParameterValue(FName("Intensity"), Data.WindIntensity);
		MPC_Wind_Inst->SetVectorParameterValue(FName("Direction"), Data.WindDirectionVector);
	}
}

void UEnvironmentManager::SetVisibilitySettings(const FEnvironmentNormalizedData& Data, FAreaOfInterest* AOI)
{
	SetEnvironmentProperty(EWeather_Properties::Dust, Data.Dust);
	SetEnvironmentProperty(EWeather_Properties::Fog, Data.Fog);
	SetEnvironmentProperty(EWeather_Properties::FogManualOverride, Data.FogManualOverride);
	SetEnvironmentProperty(EWeather_Properties::DustManualOverride, Data.DustManualOverride);
	SetEnvironmentProperty(EWeather_Properties::WindIntensity, Data.WindIntensity);
	SetEnvironmentProperty(ESky_Properties::DustAmount, Data.DustAmount);
	//SetEnvironmentProperty(ESky_Properties::BaseHeightFogFalloff, Data.BaseHeightFogFalloff);
	SetEnvironmentProperty(ESky_Properties::FogDensityContribution, Data.FogDensityContribution);
	SetEnvironmentProperty(ESky_Properties::VolumetricFogExtincion, Data.VolumetricFogExtincion);
	SetEnvironmentProperty(ESky_Properties::FogDaytimeMultiplier, Data.FogDaytimeMultiplier);
	SetEnvironmentProperty(ESky_Properties::CloudDensityContribution, Data.CloudDensityContribution);
}

void UEnvironmentManager::SetThunderSettings(const FEnvironmentNormalizedData& Data, FAreaOfInterest* AOI)
{
	SetEnvironmentProperty(EWeather_Properties::ThunderLightning, Data.ThunderLightning);
}


void UEnvironmentManager::ApplyUDSWeatherAsset(const FString& AssetPath)
{
	//StaticLoadObject, This function is loading our pre - defined assets from UDS
	UObject* AssetObj = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
	if (!AssetObj)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load weather asset: %s"), *AssetPath);
		return;
	}

	if (!UltraDynamicWeatherActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("UltraDynamicWeatherActor not set."));
		return;
	}

	// Correct function name based on tooltip
	UFunction* Func = UltraDynamicWeatherActor->FindFunction(TEXT("Change Weather"));
	if (!Func)
	{
		UE_LOG(LogTemp, Warning, TEXT("Function 'ChangeWeather' not found on UltraDynamicWeatherActor."));
		return;
	}

	struct FChangeWeatherParams
	{
		UObject* Preset;
	};

	FChangeWeatherParams Params;
	Params.Preset = AssetObj;

	UltraDynamicWeatherActor->ProcessEvent(Func, &Params);

	UE_LOG(LogTemp, Log, TEXT("Called ChangeWeather with asset: %s"), *AssetPath);

	AVolumetricCloud* CloudActor = Cast<AVolumetricCloud>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AVolumetricCloud::StaticClass())
	);

	if (CloudActor)
	{
		CloudActor->SetActorHiddenInGame(false);

		if (UVolumetricCloudComponent* CloudComp = CloudActor->FindComponentByClass<UVolumetricCloudComponent>())
		{
			CloudComp->SetVisibility(true);
		}

		UE_LOG(LogTemp, Log, TEXT("Re-enabled volumetric clouds after weather asset change."));
	}
}

void UEnvironmentManager::SetAmbientCubemapIntensityFromPercent(float percent)
{
	if (!PostProcessVolume) { UE_LOG(LogTemp, Warning, TEXT("PPV not valid")); return; }

	const float ClampedPct = FMath::Clamp(percent, 0.f, 100.f);
	const float Mapped = FMath::GetMappedRangeValueClamped(
		FVector2D(0.f, 100.f), FVector2D(0.f, 4.f), ClampedPct);
	FPostProcessSettings& S = PostProcessVolume->Settings;
	S.bOverride_AmbientCubemapIntensity = true;
	S.AmbientCubemapIntensity = Mapped;
}

void UEnvironmentManager::SetPPVVisible(bool bVisible) {
	ApplyVis(PostProcessVolume, bVisible);
}

void UEnvironmentManager::AddAreaofInterest(FAreaOfInterest* AOI)
{
	switch (AOI->AreaShape) {
	case EAOIGeometry::Circle:
		CreateBoundingShpere(static_cast<FBoundingSphere*> (AOI));
		return;
	case EAOIGeometry::Rectangle:
		CreateRectangle_1(static_cast<FRectangularRecord_1*> (AOI));
		return;
	default:
		CreateRectangle_1(static_cast<FRectangularRecord_1*> (AOI));
		return;
	}
}

void UEnvironmentManager::CreateRectangle_1(FRectangularRecord_1* AOI)
{




}

void UEnvironmentManager::CreateBoundingShpere(FBoundingSphere* AOI)
{



}

void UEnvironmentManager::AddCloudLayer(FAreaOfInterest* AOI, FCloudLayerData& CloudLayer) {

}

void UEnvironmentManager::SetSkySphere1Visible(bool bVisible) { ApplyVis(SkySphere1, bVisible); }

void UEnvironmentManager::DirectionalLightVisible(bool bVisible) { ApplyVis(MyDirectionalLight, bVisible); }

void UEnvironmentManager::SetUDSVisible(bool bVisible)
{
	ApplyVis(UltraDynamicSkyActor, bVisible);
	ApplyVis(UltraDynamicWeatherActor, bVisible);
}

bool UEnvironmentManager::ChangeMaterialOnRunTime(bool value)
{
	if (value == true)
	{
		SkySphere1->GetStaticMeshComponent()->SetMaterial(0, NewMaterial2);
		UE_LOG(LogTemp, Warning, TEXT("Applied black material to %s"), *SkySphere1->GetName());
		return 1;
	}

	else
	{
		SkySphere1->GetStaticMeshComponent()->SetMaterial(0, NewMaterial);
		UE_LOG(LogTemp, Warning, TEXT("Applied blue material to %s"), *SkySphere1->GetName());
		return 0;
	}

}

//--------------
// Shadows
//--------------

UDirectionalLightComponent* UEnvironmentManager::GetUDSSunComponent() const
{
	if (!UltraDynamicSkyActor) return nullptr;

	if (FObjectProperty* P = FindFProperty<FObjectProperty>(
		UltraDynamicSkyActor->GetClass(), TEXT("Sun_LightComponent")))
	{
		if (UObject* Obj = P->GetObjectPropertyValue_InContainer(UltraDynamicSkyActor))
			if (auto* Dir = Cast<UDirectionalLightComponent>(Obj))
				return Dir;
	}
	return UltraDynamicSkyActor->FindComponentByClass<UDirectionalLightComponent>();
}

void UEnvironmentManager::ApplySunCastShadows(bool bEnable)
{
	SetEnvironmentProperty(ESky_Properties::SunCastsShadows, bEnable);
	if (UDirectionalLightComponent* Sun = GetUDSSunComponent())
	{
		Sun->SetCastShadows(bEnable);

		Sun->MarkRenderStateDirty();
	}
}

UDirectionalLightComponent* UEnvironmentManager::GetUDSMoonComponent() const
{
	if (!UltraDynamicSkyActor) return nullptr;

	if (FObjectProperty* P = FindFProperty<FObjectProperty>(
		UltraDynamicSkyActor->GetClass(), TEXT("Moon_LightComponent")))
	{
		if (UObject* Obj = P->GetObjectPropertyValue_InContainer(UltraDynamicSkyActor))
			if (auto* Dir = Cast<UDirectionalLightComponent>(Obj))
				return Dir;
	}
	return UltraDynamicSkyActor->FindComponentByClass<UDirectionalLightComponent>();


}

void UEnvironmentManager::ApplyMoonCastShadows(bool bEnable)
{
	SetEnvironmentProperty(ESky_Properties::MoonCastsShadows, bEnable);

	if (UDirectionalLightComponent* Moon = GetUDSMoonComponent())
	{
		Moon->SetCastShadows(bEnable);
		Moon->MarkRenderStateDirty();
	}
}

UDirectionalLightComponent* UEnvironmentManager::GetUDSSkyLightComponent() const
{
	if (!UltraDynamicSkyActor) return nullptr;

	if (FObjectProperty* P = FindFProperty<FObjectProperty>(
		UltraDynamicSkyActor->GetClass(), TEXT("SkyLightComponent")))
	{
		if (UObject* Obj = P->GetObjectPropertyValue_InContainer(UltraDynamicSkyActor))
			if (auto* Dir = Cast<UDirectionalLightComponent>(Obj))
				return Dir;
	}
	return UltraDynamicSkyActor->FindComponentByClass<UDirectionalLightComponent>();
}

void UEnvironmentManager::ApplySkyLightCastShadows(bool bEnable)
{
	SetEnvironmentProperty(ESky_Properties::StaticSkyLightCastsShadows, bEnable);
	SetEnvironmentProperty(ESky_Properties::MovableSkyLightCastsShadows, bEnable);

	if (UDirectionalLightComponent* SkyLight = GetUDSSkyLightComponent())
	{
		SkyLight->SetCastShadows(bEnable);
		SkyLight->MarkRenderStateDirty();
	}
}

UDirectionalLightComponent* UEnvironmentManager::GetUDSLightningComponent() const
{
	if (!UltraDynamicWeatherActor) return nullptr;

	if (FObjectProperty* P = FindFProperty<FObjectProperty>(
		UltraDynamicWeatherActor->GetClass(), TEXT("Lightning Light")))
	{
		if (UObject* Obj = P->GetObjectPropertyValue_InContainer(UltraDynamicWeatherActor))
			if (auto* Dir = Cast<UDirectionalLightComponent>(Obj))
				return Dir;
	}
	return UltraDynamicWeatherActor->FindComponentByClass<UDirectionalLightComponent>();
}

void UEnvironmentManager::ApplyLightningLightCastShadows(bool bEnable)
{
	SetEnvironmentProperty(EWeather_Properties::LightningFlashesCastsShadows, bEnable);

	if (UDirectionalLightComponent* LightningLight = GetUDSLightningComponent())
	{
		LightningLight->SetCastShadows(bEnable);
		LightningLight->MarkRenderStateDirty();
	}
}

bool UEnvironmentManager::getIsNetwork()
{
	if (EnvController->getFlag()) {
		EnvController->GetEnvironmentData()->SetCurrentScenarioEnv(EnvController->GetEnvironmentData()->NetworkScenarioEnvironment);
	}
	else {
		EnvController->GetEnvironmentData()->SetCurrentScenarioEnv(EnvController->GetEnvironmentData()->LocalScenarioEnvironment);
	}
	return EnvController->getFlag();
}

//-----------------------------------
//--------Complete Simulation Cycle
FText UEnvironmentManager::GetAppliedPresetName()
{
	return FText::FromString(AppliedPresetName);
}

void UEnvironmentManager::CalculatingDayNightLength(const FEnvironmentNormalizedData& Data)
{
	float DayLength = 0.0f;
	float NightLength = 0.0f;
	float DawnTime = 0.0f;
	float DuskTime = 0.0f;
	float TimeofDay = 0.0f;
	GetEnvironmentProperty(ESky_Properties::DawnTime, DawnTime);
	GetEnvironmentProperty(ESky_Properties::DuskTime, DuskTime);
	float DawnMins = (DawnTime / 100 * 60) + FMath::Fmod(DawnTime, 100.0f);
	float DuskMins = (DuskTime / 100 * 60) + FMath::Fmod(DuskTime, 100.0f);
	DayLength = DuskMins - DawnMins;
	NightLength = 1440 - DayLength;
	SetEnvironmentProperty(ESky_Properties::DayLength, DayLength);
	SetEnvironmentProperty(ESky_Properties::NightLength, NightLength);
	return;
}

void UEnvironmentManager::StartAutoPlayback()
{
	bAutoPlayback = true;
}

void UEnvironmentManager::Tick(float DeltaTime)
{
	if (bAutoPlayback)
	{
		const TArray<FScenarioEnvironmentPreset>& Preset = EnvController->GetEnvironmentPresets();
		if (Preset.Num() == 0) return;
		float TimeofDay = 0.0f;
		GetEnvironmentProperty(ESky_Properties::TimeofDay, TimeofDay);
		int32 Time = FMath::Fmod(TimeofDay, 300.0f);
		int32 PresetSlot = 300.0f / Preset.Num();
		indexToPlay = Time / PresetSlot;
		if (indexToPlay == Preset.Num()) return;
		if (indexToPlay != PrevIndex) {

			FString PresetToApply = Preset[indexToPlay].PresetId.Name;
			AppliedPresetName = PresetToApply;
			EnvController->ApplyEnvironmentPresetByName(PresetToApply);
			PrevIndex = indexToPlay;
		}
	}
}

void UEnvironmentManager::StopAutoPlayback()
{
	bAutoPlayback = false;
}