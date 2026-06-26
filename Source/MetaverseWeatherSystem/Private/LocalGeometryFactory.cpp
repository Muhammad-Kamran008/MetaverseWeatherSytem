// Fill out your copyright notice in the Description page of Project Settings.


#include "LocalGeometryFactory.h"

LocalGeometryFactory::LocalGeometryFactory()
{
}

LocalGeometryFactory* LocalGeometryFactory::m_LocalGeometryFactoryInstance = nullptr;
LocalGeometryFactory* LocalGeometryFactory::GetLocalGeometryFactoryInstance()
{
    if (m_LocalGeometryFactoryInstance == nullptr)
    {
        m_LocalGeometryFactoryInstance = new LocalGeometryFactory();
    }
    return m_LocalGeometryFactoryInstance;
}

FAreaOfInterest* LocalGeometryFactory::GetGeomtery(EAOIGeometry Type,FEnvironmentData* EnvData, FieldsData* FieldsData)
{

    FAreaOfInterest* Geometery = nullptr;
    switch (Type)
    {
    case EAOIGeometry::Rectangle:
    {
        FRectangularRecord_1* AOI = new FRectangularRecord_1();
        int32 key = EnvData->LocalScenarioEnvironment.GetAreaOfInterestPresets().Num() + 1;
        AOI->TopRight.Lat = static_cast<FRectangle_1FieldsData*>(FieldsData)->TopRightLat;
        AOI->TopRight.Lon = static_cast<FRectangle_1FieldsData*>(FieldsData)->TopRightLon;
        AOI->BottomLeft.Lat = static_cast<FRectangle_1FieldsData*>(FieldsData)->BottomLeftLat;
        AOI->BottomLeft.Lon = static_cast<FRectangle_1FieldsData*>(FieldsData)->BottomLeftLon;
        AOI->AreaShape = Type;
        Geometery = AOI;
        break;
    }
    case EAOIGeometry::Circle :
    {
        FBoundingSphere* AOI = new FBoundingSphere();
        int32 key = EnvData->LocalScenarioEnvironment.GetAreaOfInterestPresets().Num() + 1;
        AOI->AreaShape = Type;
        AOI->Center = static_cast<FBoundingSphereFieldsData*>(FieldsData)->Center;
        AOI->radius = static_cast<FBoundingSphereFieldsData*>(FieldsData)->Radius;
        Geometery = AOI;
        break;
    }
    default:
        break;
    }

    return Geometery;
}



LocalGeometryFactory::~LocalGeometryFactory()
{
}
