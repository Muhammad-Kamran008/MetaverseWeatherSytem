// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentData.h"
/**
 * 
 */

struct FieldsData {
	FString AreaName;
};


struct FRectangle_1FieldsData : public FieldsData {


	double TopRightLat;
	double TopRightLon;
	double BottomLeftLat;
	double BottomLeftLon;
};

struct FBoundingSphereFieldsData : public FieldsData {

	double Radius;
	FVector Center;

};






class METAVERSEWEATHERSYSTEM_API LocalGeometryFactory
{
private:
	static LocalGeometryFactory* m_LocalGeometryFactoryInstance;
	LocalGeometryFactory();

public:
	static LocalGeometryFactory* GetLocalGeometryFactoryInstance();
	FAreaOfInterest* GetGeomtery(EAOIGeometry Type, FEnvironmentData* EnvData, FieldsData* FiledsData);
	~LocalGeometryFactory();

	
};
