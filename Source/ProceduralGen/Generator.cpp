// Fill out your copyright notice in the Description page of Project Settings.


#include "Generator.h"

#include "Kismet/KismetMathLibrary.h"

//#include "RenderGraphBuilder.inl"

// Sets default values
AGenerator::AGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	SpawnRoomsInRadius();
	SeparateRooms();
}

// Called every frame
void AGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGenerator::SpawnRoomsInRadius()
{
	for (int i = 0; i < roomNumber; i++)
	{
		int rdmX = FMath::RandRange(minSizeX, maxSizeX);
		int rdmY = FMath::RandRange(minSizeY, maxSizeY);
		
		FVector spawnLocation = RandomPointInDisk(initialSpawnRadius);
		FRotator spawnRotation = FRotator(0.f, 0.f, 0.f);
		
		ARoom* Room = GetWorld()->SpawnActor<ARoom>(RoomToSpawn, spawnLocation, spawnRotation);

		if (Room)
		{
			Room->SetActorScale3D(FVector(rdmX, rdmY, 1.f));

			if (rdmX * rdmY > areaLimit) 
			{
				majorRooms.Add(Room);
				Room->SetColor(MajorMat);
			}
			else
			{
				minorRooms.Add(Room);
				Room->SetColor(SecondaryMat);
			}

			roomsArray.Add(Room);
		}
	}
}

void AGenerator::SeparateRooms()
{
	if (roomsArray.Num() <= 1) return;

	for (int32 m = 0; m < maxIteration; ++m)
    {
        bool bAnyMoved = false;

        for (int32 i = 0; i < roomsArray.Num(); ++i)
        {
            AActor* A = roomsArray[i];
            if (!IsValid(A)) continue;

            FVector CenterA, ExtentA;
            A->GetActorBounds(/*bOnlyCollidingComponents*/ false, CenterA, ExtentA);

            for (int32 j = i + 1; j < roomsArray.Num(); ++j)
            {
                AActor* B = roomsArray[j];
                if (!IsValid(B)) continue;

                FVector CenterB, ExtentB;
                B->GetActorBounds(false, CenterB, ExtentB);

                // Overlap test en XY (AABB)
                const float dx = (ExtentA.X + ExtentB.X + padding) - FMath::Abs(CenterA.X - CenterB.X);
                const float dy = (ExtentA.Y + ExtentB.Y + padding) - FMath::Abs(CenterA.Y - CenterB.Y);

                if (dx > 0.f && dy > 0.f) // overlap détecté
                {
                	const FVector Delta = FVector(CenterB.X - CenterA.X, CenterB.Y - CenterA.Y, 0.f);
                	const FVector Dir = Delta.GetSafeNormal(); // A -> B

                	if (!Dir.IsNearlyZero())
                	{
                		float Move = 0.5f * FMath::Min(dx, dy);
                		
                		Move *= FMath::FRandRange(0.95f, 1.05f);
                		
                		A->SetActorLocation(A->GetActorLocation() - Dir * Move, /*bSweep*/ false);
                		B->SetActorLocation(B->GetActorLocation() + Dir * Move, /*bSweep*/ false);
                		
                		A->GetActorBounds(false, CenterA, ExtentA);

                		bAnyMoved = true;
                	}
                }
            }
        }

        if (!bAnyMoved)
           break; // plus d'overlap
    }
}

FVector AGenerator::RandomPointInDisk(float radius)
{
	const float Angle = FMath::FRandRange(0.f, 2.f * PI);
	const float r = radius * FMath::Sqrt(FMath::FRand());
	const float x = r * FMath::Cos(Angle);
	const float y = r * FMath::Sin(Angle);
	return FVector(x, y, 0.f);
}

void AGenerator::ClearRooms()
{
	for (int i = 0; i < roomsArray.Num(); i++)
	{
		roomsArray[i]->Destroy();
	}
}

void AGenerator::ClearMajorRooms()
{
	for (int i = 0; i < majorRooms.Num(); i++)
	{
		majorRooms[i]->Destroy();
	}
}

void AGenerator::ClearMinorRooms()
{
	for (int i = 0; i < minorRooms.Num(); i++)
	{
		minorRooms[i]->Destroy();
	}
}

