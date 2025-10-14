// Fill out your copyright notice in the Description page of Project Settings.


#include "Generator.h"

#include "Kismet/KismetMathLibrary.h"

//#include "RenderGraphBuilder.inl"
bool Triangle::IsPointInPointCircumCircle(FVector P)
{
	double ax = Points[0]->Pos.X - P.X;
	double ay = Points[0]->Pos.Y - P.Y;
	double bx = Points[1]->Pos.X - P.X;
	double by = Points[1]->Pos.Y - P.Y;
	double cx = Points[2]->Pos.X - P.X;
	double cy = Points[2]->Pos.Y - P.Y;

	double det = (ax * ax + ay * ay) * (bx * cy - cx * by)
				- (bx * bx + by * by) * (ax * cy - cx * ay)
				+ (cx * cx + cy * cy) * (ax * by - bx * ay);

	return det > 0.0;
}


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
			
			Point* point = MakePoint(bIsMajor, Room);
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

	Point* STA =MakePoint(true, summitA);
	Point* STB =MakePoint(true, summitB);
	Point* STC =MakePoint(true, summitC);
	
	superTriangle.Points.Append({STA, STB, STC});
	validatedTrianglesArray.Add(superTriangle);
	trianglesArray.Add(superTriangle);

	//Debug
	//summitA->SetActorScale3D(FVector(200, 200, 200));
	//summitB->SetActorScale3D(FVector(200, 200, 200));
	//summitC->SetActorScale3D(FVector(200, 200, 200));
	//Debug
}

void AGenerator::DrawTriangles()
{
	UWorld* World = GetWorld();
	if (!World) return;

	
	for (Triangle& T : trianglesArray)
	{
		ARoom* testA = GetWorld()->SpawnActor<ARoom>(RoomToSpawn, T.Points[0]->Pos, FRotator::ZeroRotator);
		ARoom* testB = GetWorld()->SpawnActor<ARoom>(RoomToSpawn, T.Points[1]->Pos, FRotator::ZeroRotator);
		ARoom* testC = GetWorld()->SpawnActor<ARoom>(RoomToSpawn, T.Points[2]->Pos, FRotator::ZeroRotator);

		trianglesSummits.Append({testA, testB, testC});
		
		testA->SetActorScale3D(FVector(5, 5, 5));
		testB->SetActorScale3D(FVector(5, 5, 5));
		testC->SetActorScale3D(FVector(5, 5, 5));
		
		FVector A = FVector(T.Points[0]->Pos.X,T.Points[0]->Pos.Y, 5.0f);
		FVector B = FVector(T.Points[1]->Pos.X,T.Points[1]->Pos.Y, 5.0f);
		FVector C = FVector(T.Points[2]->Pos.X,T.Points[2]->Pos.Y, 5.0f);
		
		DrawDebugLine(World, A , B, FColor::Green,true, 100 , 0,200.f);
		DrawDebugLine(World, B , C, FColor::Green,true, 100 , 0,200.f);
		DrawDebugLine(World, A , C, FColor::Green,true, 100 , 0,200.f);
		
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
			AddEdgeToPointNoDup(P0, E01);
			AddEdgeToPointNoDup(P1, E01);

			AddEdgeToPointNoDup(P1, E12);
			AddEdgeToPointNoDup(P2, E12);

			AddEdgeToPointNoDup(P2, E20);
			AddEdgeToPointNoDup(P0, E20);
		}
	}
	DrawTriangles();
	ClearSuperTriangle();
}
// Calc Triangulation
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
		if (SameUndirected(E, A, B))
			return E; 
	}
	
	Edge* NewE = new Edge();
	NewE->A = A;
	NewE->B = B;
	AllEdges.Add(NewE);
	return NewE;
}
bool AGenerator::PointHasEdge( Point* P, Edge* E)
{
	if (!P || !E) return false;
	for ( Edge* Existing : P->Edges)
	{
		if (SameUndirected(Existing, E->A, E->B))
			return true;
	}
	return false;
}

void AGenerator::AddEdgeToPointNoDup(Point* P, Edge* E)
{
	if (!P || !E) return;
	if (!PointHasEdge(P, E))
	{
		P->Edges.Add(E);
	}
}
Point* AGenerator::MakePoint(bool isMajor, ARoom* room)
{
	Point* point = new Point();
	point->Room = room;
	point->Pos = room->GetActorLocation();
	return point;
}

void AGenerator::Triangulation()
{
	for (int i = 0; i < PointsArray.Num(); i++)
	{
		if (PointsArray[i]->Room->isMajor)
		{
			// 1) Triangles invalides
			TArray<Triangle> BadTriangles;
			for (Triangle& T : trianglesArray)
			{
				// Cercles circonscrit qui englobent le p courant 
				if (T.IsPointInPointCircumCircle(PointsArray[i]->Pos))
				{
					BadTriangles.Add(T);
				}
			}

			// 2) Arêtes frontières
			TArray<Edge> Polygon;
			for (Triangle& T : BadTriangles)
			{
				// chaque triangle = 3 arêtes
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

					// test si E est partagée par un autre bad triangle
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
						Polygon.Add(E); // E est frontière
				}
			}

			// 3) On supprime les bad triangles
			for (Triangle& T : BadTriangles)
			{
				trianglesArray.RemoveSingle(T);
			}

			// 4) On crée les nouveaux triangles avec P
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


