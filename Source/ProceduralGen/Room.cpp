// Fill out your copyright notice in the Description page of Project Settings.


#include "Room.h"

#include "Generator.h"

// Sets default values
ARoom::ARoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

	

    PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
    RootComponent = PlaneMesh;

    PlaneMesh->SetMobility(EComponentMobility::Movable);
    PlaneMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    PlaneMesh->SetCollisionProfileName(TEXT("PhysicsActor"));

    PlaneMesh->SetSimulatePhysics(false);
    PlaneMesh->SetEnableGravity(false);

}

void ARoom::SetColor(UMaterialInterface* mat)
{
    PlaneMesh->SetMaterial(0, mat);
}