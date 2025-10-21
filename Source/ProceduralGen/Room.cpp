#include "Room.h"

ARoom::ARoom()
{
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