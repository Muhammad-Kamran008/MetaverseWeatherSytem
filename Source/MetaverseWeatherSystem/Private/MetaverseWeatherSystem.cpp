// Copyright Epic Games, Inc. All Rights Reserved.

#include "MetaverseWeatherSystem.h"

#define LOCTEXT_NAMESPACE "FMetaverseWeatherSystemModule"

void FMetaverseWeatherSystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FMetaverseWeatherSystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMetaverseWeatherSystemModule, MetaverseWeatherSystem)