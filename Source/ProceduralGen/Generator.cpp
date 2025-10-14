// Fill out your copyright notice in the Description page of Project Settings.


#include "Generator.h"

#include "StaticMeshAttributes.h"
#include "Kismet/KismetMathLibrary.h"

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
	SetSuperTriangle();
	Triangulation();
}

void AGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	ClearTriangles();
	ClearRooms();
	
}

// Called every frame
void AGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGenerator::MakeDungeon()
{
	SpawnRoomsInRadius();
	SeparateRooms();
	SetSuperTriangle();
	Triangulation();
}

// Spawns Rooms Points and Triangles
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
			
			const bool bIsMajor = (rdmX * rdmY > areaLimit);
			
			Room->SetColor(bIsMajor ? MajorMat : SecondaryMat);
			Room->isMajor = bIsMajor;
			Point* point =  DungeonFunction.MakePoint(bIsMajor, Room);
			PointsArray.Add(point);
			roomsArray.Add(Room);
		}
	}
}

void AGenerator::SetSuperTriangle()
{
	float maxX = TNumericLimits<float>::Lowest();
	float minX = TNumericLimits<float>::Max();
	float maxY = TNumericLimits<float>::Lowest();
	float minY = TNumericLimits<float>::Max();
	
	
	for (int i = 0; i < PointsArray.Num(); ++i)
	{
		minX = FMath::Min(minX, PointsArray[i]->Pos.X);
		maxX = FMath::Max(maxX, PointsArray[i]->Pos.X);
		minY = FMath::Min(minY, PointsArray[i]->Pos.Y);
		maxY = FMath::Max(maxY, PointsArray[i]->Pos.Y);
	}

	float CenterX = (minX + maxX) / 2.f;
	float Margin = (maxX - minX + maxY - minY);

	FVector A = FVector(CenterX, maxY + Margin, 0.f);
	FVector B = FVector(minX - Margin, minY - Margin, 0.f);
	FVector C = FVector(maxX + Margin, minY - Margin, 0.f);
	
	ARoom* summitA = GetWorld()->SpawnActor<ARoom>(RoomToSpawn, A, FRotator::ZeroRotator);
	ARoom* summitB = GetWorld()->SpawnActor<ARoom>(RoomToSpawn, B, FRotator::ZeroRotator);
	ARoom* summitC = GetWorld()->SpawnActor<ARoom>(RoomToSpawn, C, FRotator::ZeroRotator);

	// track debug actors
	if (summitA) DebugRooms.Add(summitA);
	if (summitB) DebugRooms.Add(summitB);
	if (summitC) DebugRooms.Add(summitC);

	Point* STA = DungeonFunction.MakePoint(true, summitA);
	Point* STB = DungeonFunction.MakePoint(true, summitB);
	Point* STC = DungeonFunction.MakePoint(true, summitC);
	
	superTriangle.Points.Append({STA, STB, STC});
	validatedTrianglesArray.Add(superTriangle);
	trianglesArray.Add(superTriangle);
}

void AGenerator::DrawEdges()
{
	UWorld* World = GetWorld();
	if (!World) return;
	for (Edge* E : AllEdges)
	{
		if (!E || !E->A || !E->B) continue;

		const FVector A(E->A->Pos.X, E->A->Pos.Y, 5.f);
		const FVector B(E->B->Pos.X, E->B->Pos.Y, 5.f);

		DrawDebugLine(World, A, B, FColor::Green, /*bPersistent*/ true, /*LifeTime*/ 100.f, 0, /*Thickness*/ 200.f);
	}
}

void AGenerator::DeleteBadSuperTriangles()
{
	TArray<FVector> superTrianglePoints;
	superTrianglePoints.Append({superTriangle.Points[0]->Pos, superTriangle.Points[1]->Pos, superTriangle.Points[2]->Pos});
	
	for (int32 i = trianglesArray.Num() - 1; i >= 0; --i)
	{
		const bool isBadTriangle =
			trianglesArray[i].Points.Contains(superTriangle.Points[0]) ||
			trianglesArray[i].Points.Contains(superTriangle.Points[1]) ||
			trianglesArray[i].Points.Contains(superTriangle.Points[2]);
		
		if (isBadTriangle)
		{
			trianglesArray.RemoveAtSwap(i);
		}
		else
		{
			Point* P0 = trianglesArray[i].Points[0];
			Point* P1 = trianglesArray[i].Points[1];
			Point* P2 = trianglesArray[i].Points[2];

			// Crée/récupère les arêtes non orientées du triangle
			Edge* E01 = FindOrCreateEdge(P0, P1);
			Edge* E12 = FindOrCreateEdge(P1, P2);
			Edge* E20 = FindOrCreateEdge(P2, P0);

			// Attache chaque arête aux deux points (sans doublon)
			DungeonFunction.AddEdgeToPointNoDup(P0, E01);
			DungeonFunction.AddEdgeToPointNoDup(P1, E01);

			DungeonFunction.AddEdgeToPointNoDup(P1, E12);
			DungeonFunction.AddEdgeToPointNoDup(P2, E12);

			DungeonFunction.AddEdgeToPointNoDup(P2, E20);
			DungeonFunction.AddEdgeToPointNoDup(P0, E20);
		}
	}
	DrawEdges();
	ClearSuperTriangle();
}


FVector AGenerator::RandomPointInDisk(float radius)
{
	const float Angle = FMath::FRandRange(0.f, 2.f * PI);
	const float r = radius * FMath::Sqrt(FMath::FRand());
	const float x = r * FMath::Cos(Angle);
	const float y = r * FMath::Sin(Angle);
	return FVector(x, y, 0.f);
}

void AGenerator::SeparateRooms()
{
	if (roomsArray.Num() <= 1) return;

	for (int32 m = 0; m < maxIteration; ++m)
    {
        bool bAnyMoved = false;

        for (int32 i = 0; i < roomsArray.Num(); ++i)
        {
            ARoom* A = roomsArray[i];
            if (!IsValid(A)) continue;

            FVector CenterA, ExtentA;
            A->GetActorBounds(false, CenterA, ExtentA);

            for (int32 j = i + 1; j < roomsArray.Num(); ++j)
            {
                ARoom* B = roomsArray[j];
                if (!IsValid(B)) continue;

                FVector CenterB, ExtentB;
                B->GetActorBounds(false, CenterB, ExtentB);

                // Overlap test en XY (AABB)
                const float dx = (ExtentA.X + ExtentB.X + padding) - FMath::Abs(CenterA.X - CenterB.X);
                const float dy = (ExtentA.Y + ExtentB.Y + padding) - FMath::Abs(CenterA.Y - CenterB.Y);

                if (dx > 0.f && dy > 0.f) // overlap détecté
                {
                	const FVector Delta = FVector(CenterB.X - CenterA.X, CenterB.Y - CenterA.Y, 0.f);
                	
                	FVector Dir = Delta.GetSafeNormal();
                	if (Dir.IsNearlyZero())
                	{
                		Dir = FVector(FMath::FRandRange(-1.f, 1.f), FMath::FRandRange(-1.f, 1.f), 0.f).GetSafeNormal();
                	}

                	float Move = 0.5f * FMath::Min(dx, dy) * FMath::FRandRange(0.95f, 1.05f);
                	A->AddActorWorldOffset(- Dir * Move, false);
                	B->AddActorWorldOffset(Dir * Move,  false);

                	A->GetActorBounds(false, CenterA, ExtentA);
                	bAnyMoved = true;
                }
            }
        }
		
        if (!bAnyMoved)
           break; 
    }

	ReasignPointPosition();
}

void AGenerator::ReasignPointPosition()
{
	for (Point* P: PointsArray)
	{
		P->Pos = P->Room->GetActorLocation();
	}
}

Edge* AGenerator::FindOrCreateEdge(Point* A, Point* B)
{
	if (!A || !B || A == B) return nullptr;
	for (Edge* E : AllEdges)
	{
		if (DungeonFunction.SameUndirected(E, A, B))
			return E; 
	}
	
	Edge* NewE = new Edge();
	NewE->A = A;
	NewE->B = B;
	AllEdges.Add(NewE);
	return NewE;
}

void AGenerator::Triangulation()
{
	for (int i = 0; i < PointsArray.Num(); i++)
	{
		if (PointsArray[i]->Room->isMajor)
		{
			BadTriangles = CollectBadTriangles(i) ;
			TArray<Edge> Polygon = ExtractFrontierEdges();
			
			for (Triangle& T : BadTriangles)
			{
				trianglesArray.RemoveSingle(T);
			}
			
			for (Edge& E : Polygon)
			{
				Triangle triangleToAdd;
				triangleToAdd.Points.Append({E.A, E.B, PointsArray[i]});
				trianglesArray.Add(triangleToAdd);
			}
		}
	}
	DeleteBadSuperTriangles();
}

TArray<Triangle> AGenerator::CollectBadTriangles(int i)
{
	TArray<Triangle> badTriangles;
	for (Triangle& T : trianglesArray)
	{
		// Cercles circonscrit qui englobent le p courant 
		if (T.IsPointInPointCircumCircle(PointsArray[i]->Pos))
		{
			badTriangles.Add(T);
		}
	}
	return badTriangles;
}

TArray<Edge> AGenerator::ExtractFrontierEdges()
{
	TArray<Edge> Polygon;
	for (Triangle& T : BadTriangles)
	{
		Edge E1, E2, E3 ;
		E1.A = T.Points[0];
		E1.B = T.Points[1];
		E2.A = T.Points[1];
		E2.B = T.Points[2];
		E3.A = T.Points[2];
		E3.B = T.Points[0];
			
		TArray<Edge> Edges;
		Edges.Append({E1,E2,E3});

		for (int k = 0; k < 3; k++)
		{
			Edge E = Edges[k];
			
			bool bShared = false;
			for (Triangle& Other : BadTriangles)
			{
				if (&T == &Other) continue;
				if (Other.HasEdge(E)) 
				{
					bShared = true;
					break;
				}
			}

			if (!bShared)
				Polygon.Add(E);
		}
	}
	return Polygon;
}

// Lane Prim Algorithm



//Clear

void AGenerator::ClearAll()
{
	ClearTriangles();
	ClearRooms();
}

void AGenerator::ClearRooms()
{
	for (int i = 0; i < roomsArray.Num(); i++)
	{
		roomsArray[i]->Destroy();
	}
	for (int i = PointsArray.Num() - 1; i >= 0; i--)
	{
		PointsArray[i]->Room->Destroy();
		PointsArray.RemoveAt(i);
	}
	for (Point* P : superTriangle.Points) { delete P; }
	superTriangle.Points.Empty();
}

void AGenerator::ClearTriangles()
{
	ClearSuperTriangle();
	
	for (int i = 0; i < trianglesSummits.Num(); ++i)
	{
		trianglesSummits[i]->Destroy();
	}
	for (Edge* E : AllEdges) { delete E; }
	AllEdges.Empty();
	for (Point* P : PointsArray) { if (P) P->Edges.Empty(); }
	trianglesArray.Empty();
	trianglesSummits.Empty();
	
	FlushPersistentDebugLines(GetWorld());
}

void AGenerator::ClearSuperTriangle()
{
	for (Point* P : superTriangle.Points)
	{
		delete P; 
	}
	superTriangle.Points.Empty();
	for (ARoom* R : DebugRooms)
	{
		if (IsValid(R)) R->Destroy();
	}
	DebugRooms.Empty();
	superTriangle.Points.Reset();
}


