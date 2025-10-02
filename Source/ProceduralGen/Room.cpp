// Fill out your copyright notice in the Description page of Project Settings.


#include "Room.h"

#include "Generator.h"

// Sets default values
ARoom::ARoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
    RootComponent = CubeMesh;

    CubeMesh->SetMobility(EComponentMobility::Movable);
    CubeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CubeMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

    CubeMesh->SetSimulatePhysics(false);
    CubeMesh->SetEnableGravity(false);

}

void ARoom::SetColor(UMaterialInterface* mat)
{
    CubeMesh->SetMaterial(0, mat);
}
