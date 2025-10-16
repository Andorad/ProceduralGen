// Fill out your copyright notice in the Description page of Project Settings.


#include "Generator.h"

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
	
	ClearAll();
	
}

// Called every frame
void AGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGenerator::MakeDungeon()
{
	ClearAll();
	SpawnRoomsInRadius();
	SeparateRooms();
	SetSuperTriangle();
	Triangulation();
	PrimAlgorithm();
}

// Create Room and Points
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
			Room->SetActorScale3D(FVector(rdmX, rdmY, 0.1f));
			
			const bool bIsMajor = (rdmX * rdmY > areaLimit);
			
			Room->SetColor(bIsMajor ? MajorMat : SecondaryMat);
			Room->isMajor = bIsMajor;
			Point* point =  DungeonFunction.MakePoint(bIsMajor, Room);
			
			if (bIsMajor)MajorPoints.Add(point);
			
			PointsArray.Add(point);
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

                
                const float dx = (ExtentA.X + ExtentB.X + padding) - FMath::Abs(CenterA.X - CenterB.X);
                const float dy = (ExtentA.Y + ExtentB.Y + padding) - FMath::Abs(CenterA.Y - CenterB.Y);

                if (dx > 0.f && dy > 0.f)
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
	
	if (summitA) DebugRooms.Add(summitA);
	if (summitB) DebugRooms.Add(summitB);
	if (summitC) DebugRooms.Add(summitC);

	Point* STA = DungeonFunction.MakePoint(true, summitA);
	Point* STB = DungeonFunction.MakePoint(true, summitB);
	Point* STC = DungeonFunction.MakePoint(true, summitC);
	
	superTriangle.Points.Append({STA, STB, STC});
	trianglesArray.Add(superTriangle);
}

FVector AGenerator::RandomPointInDisk(float radius)
{
	const float Angle = FMath::FRandRange(0.f, 2.f * PI);
	const float r = radius * FMath::Sqrt(FMath::FRand());
	const float x = r * FMath::Cos(Angle);
	const float y = r * FMath::Sin(Angle);
	return FVector(x, y, 0.f);
}



void AGenerator::ReasignPointPosition()
 {
 	for (Point* P: PointsArray)
 	{
 		P->Pos = P->Room->GetActorLocation();
 	}
 }

//Triangulation
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
	ClearSuperTriangle();
}

TArray<Triangle> AGenerator::CollectBadTriangles(int i)
{
	TArray<Triangle> badTriangles;
	for (Triangle& T : trianglesArray)
	{
		if (T.IsPointInPointCircumCircle(PointsArray[i]->Pos))
		{
			badTriangles.Add(T);
		}
	}
	return badTriangles;
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
			
			Edge* E01 = FindOrCreateEdge(P0, P1);
			Edge* E12 = FindOrCreateEdge(P1, P2);
			Edge* E20 = FindOrCreateEdge(P2, P0);
			
			DungeonFunction.AddEdgeToPointNoDup(P0, E01);
			DungeonFunction.AddEdgeToPointNoDup(P1, E01);

			DungeonFunction.AddEdgeToPointNoDup(P1, E12);
			DungeonFunction.AddEdgeToPointNoDup(P2, E12);

			DungeonFunction.AddEdgeToPointNoDup(P2, E20);
			DungeonFunction.AddEdgeToPointNoDup(P0, E20);
		}
	}
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
	NewE->UpdateLength(); 
	AllEdges.Add(NewE);
	return NewE;
}

void AGenerator::PushPathPossibility(Point* current)
{
	for (Edge* E : current->Edges)
	{
		if (!E) continue;
		Point* To = (E->A == current) ? E->B : (E->B == current ? E->A : nullptr);
		if (!To || To->bVisited) continue;
    
		FMstEntry Ent = DungeonFunction.MakeEntry(E, current,To, E->weight);
		CandidateEdges.Add(Ent);
    	}
}

// Prim Algorithm
void AGenerator::PrimAlgorithm()
{
	MSTEdges.Empty();
	if (MajorPoints.Num() == 0) return;
	Point* current = SelectRandomMajorPoint();
	

	if (!current) return;
	current->bVisited = true;

	PushPathPossibility(current);
	
	while (true)
    {
        // DeadEnd
        if (CandidateEdges.Num() == 0)
        {
            Point* Seed = nullptr;
            for (Point* P : MajorPoints)
            {
                if (P && !P->bVisited)
                {
	                Seed = P;
                	break;
                }
            }
            if (!Seed) break; // All points connected

            Seed->bVisited = true;
            
        	PushPathPossibility(Seed);
            continue;
        }

        // Lightest Edge
        int32 BestIdx = INDEX_NONE;
        float BestW   = FLT_MAX;

        for (int32 i = 0; i < CandidateEdges.Num(); ++i)
        {
            if (!CandidateEdges[i].To || CandidateEdges[i].To->bVisited) continue;
            if (CandidateEdges[i].Weight < BestW)
            {
                BestW = CandidateEdges[i].Weight;
                BestIdx = i;
            }
        }

        // No valid entries
        if (BestIdx == INDEX_NONE)
        {
            CandidateEdges.Empty();
            continue;
        }

        // Validate Edge
        FMstEntry Best = CandidateEdges[BestIdx];
        CandidateEdges.RemoveAtSwap(BestIdx);

        MSTEdges.Add(Best.E);
        Best.To->bVisited = true;

        // Push Path
        current = Best.To;
		PushPathPossibility(current);
    }

	BuildCorridorsFromMST_Meshes();
	RemoveMinorRoomsOutOfDungeon(/*CorridorPad=*/8.f);
	BuildFillers();
}

Point* AGenerator::SelectRandomMajorPoint()
{
	const int32 N = MajorPoints.Num();
	if (N == 0) return nullptr;
	int32 start = FMath::RandRange(0, N - 1);
	
	return MajorPoints[start];

}
//Corridor Generation

FVector AGenerator::DoorToward(const Point* From, const Point* Toward) const
{
	check(From && From->Room && Toward && Toward->Room);

	FVector CenterA, ExtentA;
	From->Room->GetActorBounds(false, CenterA, ExtentA);

	FVector CenterB, ExtentB;
	Toward->Room->GetActorBounds(false, CenterB, ExtentB);

	const FVector Delta = CenterB - CenterA;
	FVector Out = CenterA;

	const bool AlignedX = FMath::Abs(Delta.Y) <= (ExtentA.Y + ExtentB.Y) * 0.25f;
	const bool AlignedY = FMath::Abs(Delta.X) <= (ExtentA.X + ExtentB.X) * 0.25f;

	if (AlignedX)
	{
		// Aligned horizontally exit along X
		Out.X = CenterA.X + FMath::Sign(Delta.X) * ExtentA.X;
		Out.Y = CenterA.Y; // keep centered vertically
	}
	else if (AlignedY)
	{
		// Aligned vertically exit along Y
		Out.Y = CenterA.Y + FMath::Sign(Delta.Y) * ExtentA.Y;
		Out.X = CenterA.X; // keep centered horizontally
	}
	else
	{
		// Not aligned use dominant axis (classic behaviour)
		if (FMath::Abs(Delta.X) >= FMath::Abs(Delta.Y))
		{
			// Exit through the +X or -X face, and clamp Y within the room’s horizontal bounds.
			Out.X = CenterA.X + FMath::Sign(Delta.X) * ExtentA.X;
			Out.Y = FMath::Clamp(CenterB.Y, CenterA.Y - ExtentA.Y + 5.f, CenterA.Y + ExtentA.Y - 5.f);
		}
		else
		{
			// Exit through the +Y or -Y face, and clamp X within the room’s vertical bounds.
			Out.Y = CenterA.Y + FMath::Sign(Delta.Y) * ExtentA.Y;
			Out.X = FMath::Clamp(CenterB.X, CenterA.X - ExtentA.X + 5.f, CenterA.X + ExtentA.X - 5.f);
		}
	}

	Out.Z = CenterA.Z + CorridorZ;
	return Out;
}

bool AGenerator::TryAlignedDoors(const Point* A, const Point* B, FVector& DoorA, FVector& DoorB) const
{
	check(A && A->Room && B && B->Room);

	FVector CA, EA; A->Room->GetActorBounds(false, CA, EA);
	FVector CB, EB; B->Room->GetActorBounds(false, CB, EB);

	const float Eps = 5.f; 

	// HTZ Corridors
	float midY = 0.f, lenY = 0.f;
	const bool overlapY = Intersect1D(CA.Y - EA.Y, CA.Y + EA.Y, CB.Y - EB.Y, CB.Y + EB.Y, midY, lenY);
	if (overlapY && lenY >= CorridorThickness)
	{
		const bool AisLeft = (CB.X - CA.X) > 0.f;
		DoorA = FVector(CA.X + (AisLeft ? +EA.X + Eps : -EA.X - Eps), midY, CA.Z + CorridorZ);
		DoorB = FVector(CB.X + (AisLeft ? -EB.X - Eps : +EB.X + Eps), midY, CB.Z + CorridorZ);
		return true; 
	}

	// Vertical Corridors 
	float midX = 0.f, lenX = 0.f;
	const bool overlapX = Intersect1D(CA.X - EA.X, CA.X + EA.X, CB.X - EB.X, CB.X + EB.X, midX, lenX);
	if (overlapX && lenX >= CorridorThickness)
	{
		const bool AisBelow = (CB.Y - CA.Y) > 0.f;
		DoorA = FVector(midX, CA.Y + (AisBelow ? +EA.Y + Eps : -EA.Y - Eps), CA.Z + CorridorZ);
		DoorB = FVector(midX, CB.Y + (AisBelow ? -EB.Y - Eps : +EB.Y + Eps), CB.Z + CorridorZ);
		return true; 
	}

	return false; 
}

//Clear
void AGenerator::ClearAll()
{
	ClearTriangles();
	ClearRooms();
	ClearCorridors();
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
void AGenerator::ClearCorridors()
{
	// 1) Détruire tout ce que nous avons explicitement tracké
	auto DestroyArray = [](TArray<ARoom*>& Arr)
	{
		for (int32 i = Arr.Num() - 1; i >= 0; --i)
		{
			ARoom* R = Arr[i];
			if (IsValid(R)) R->Destroy();
			Arr.RemoveAtSwap(i);
		}
	};

	DestroyArray(CorridorRooms);   // couloirs posés (segments droits + morceaux de L)
	DestroyArray(FillerModules);   // modules de comblement, connecteurs, etc. si tu en as

	// 2) Par sécurité : si des couloirs/fillers ont été spawnés sans être trackés,
	//    on détruit les acteurs attachés au CorridorsRoot portant nos tags.
	if (CorridorsRoot)
	{
		TArray<AActor*> Attached;
		GetAttachedActors(Attached); // tous les enfants de l'Actor courant

		for (AActor* A : Attached)
		{
			if (!IsValid(A)) continue;

			const bool bIsCorridorLike =
				A->Tags.Contains(TEXT("Corridor")) || A->Tags.Contains(TEXT("Filler"));

			if (bIsCorridorLike)
			{
				A->Destroy();
			}
		}
	}

	// 3) Optionnel : si, par mégarde, des couloirs se trouvent dans roomsArray, on les enlève aussi.
	for (int32 i = roomsArray.Num() - 1; i >= 0; --i)
	{
		ARoom* R = roomsArray[i];
		if (!IsValid(R)) { roomsArray.RemoveAtSwap(i); continue; }

		if (R->Tags.Contains(TEXT("Corridor")) || R->Tags.Contains(TEXT("Filler")))
		{
			R->Destroy();
			roomsArray.RemoveAtSwap(i);
		}
	}
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

void AGenerator::EnsureCorridorBaseExtents() const
{
	if (BaseHalfX > 0.f && BaseHalfY > 0.f) return;
	if (!CorridorToSpawn) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorSpawnParameters P; 
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	P.bDeferConstruction = true;  // pas d’init visuelle
	ARoom* Proto = World->SpawnActor<ARoom>(CorridorToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, P);
	if (!Proto) return;

	FVector Center, Extent;
	Proto->GetActorBounds(false, Center, Extent);

	BaseHalfX = FMath::Max(1.f, Extent.X);
	BaseHalfY = FMath::Max(1.f, Extent.Y);

	Proto->Destroy();
}


void AGenerator::BuildCorridorsFromMST_Meshes()
{
	for (Edge* E : MSTEdges)
	{
		if (!E || !E->A || !E->B) continue;

		FVector Adoor, Bdoor;

		// 1) Essayer couloir droit parfait
		if (TryAlignedDoors(E->A, E->B, Adoor, Bdoor))
		{
			PlaceCorridorStraightMesh(Adoor, Bdoor);
			continue;
		}

		// 2) Sinon, fallback porte-bord + L si nécessaire
		const FVector DA = DoorToward(E->A, E->B);
		const FVector DB = DoorToward(E->B, E->A);

		const bool SameX = FMath::IsNearlyEqual(DA.X, DB.X, 1.f);
		const bool SameY = FMath::IsNearlyEqual(DA.Y, DB.Y, 1.f);

		if (SameX || SameY)
		{
			PlaceCorridorStraightMesh(DA, DB);
		}
		else
		{
			const bool HorizontalFirst = bRandomHorizontalFirst ? FMath::RandBool() : true;
			PlaceCorridorLMesh(DA, DB, HorizontalFirst);
		}
	}
}

void AGenerator::PlaceCorridorStraightMesh(const FVector& A, const FVector& B)
{
	SpawnCorridorSegment(A, B);
}

void AGenerator::PlaceCorridorLMesh(const FVector& A, const FVector& B, bool HorizontalFirst)
{
	const FVector Pivot = HorizontalFirst ? FVector(B.X, A.Y, A.Z) : FVector(A.X, B.Y, A.Z);

	SpawnCorridorSegment(A, Pivot);
	SpawnCorridorSegment(Pivot, B);

	// Optionnel : petit “cap” carré au pivot (agrémente les coins)
	// SpawnCorridorSegment(Pivot + FVector(-20,0,0), Pivot + FVector(+20,0,0)); etc.
}

ARoom* AGenerator::SpawnCorridorSegment(const FVector& A, const FVector& B)
{
	if (!CorridorToSpawn) return nullptr;
	EnsureCorridorBaseExtents();

	const FVector Mid = 0.5f * (A + B);
	FVector Dir = (B - A); Dir.Z = 0.f;
	const float Len = Dir.Size();
	if (Len < KINDA_SMALL_NUMBER) return nullptr;

	Dir.Normalize();
	const FRotator Yaw = FRotationMatrix::MakeFromX(Dir).Rotator();

	// scale (X = longueur, Y = largeur)
	const float TargetHalfX = 0.5f * Len;
	const float TargetHalfY = 0.5f * CorridorWidth;
	const float SX = TargetHalfX / FMath::Max(1.f, BaseHalfX);
	const float SY = TargetHalfY / FMath::Max(1.f, BaseHalfY);

	FTransform T(Yaw, FVector(Mid.X, Mid.Y, CorridorZSpawn));
	T.SetScale3D(FVector(SX, SY, 0.1f)); // Z fin comme tes rooms

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARoom* Seg = GetWorld()->SpawnActor<ARoom>(CorridorToSpawn, T, P);
	if (!Seg) return nullptr;


	// Parent dans le dossier Corridors + track dans la liste
	if (CorridorsRoot) Seg->AttachToComponent(CorridorsRoot, FAttachmentTransformRules::KeepWorldTransform);
	CorridorRooms.Add(Seg);
	roomsArray.Add(Seg);

	// (Facultatif) tag pour filtrer plus tard
	Seg->Tags.Add(TEXT("Corridor"));

	return Seg;
}

void AGenerator::RemoveMinorRoomsOutOfDungeon(float CorridorPad /*=8.f*/)
{
	TArray<FBox> CorridorBoxes;
	CorridorBoxes.Reserve(CorridorRooms.Num());
	for (ARoom* C : CorridorRooms)
	{
		if (!IsValid(C)) continue;
		CorridorBoxes.Add(MakeActorBox(C, CorridorPad));
	}

	// Parcours des minor rooms (en arrière pour pouvoir remove)
	for (int32 i = roomsArray.Num() - 1; i >= 0; --i)
	{
		ARoom* R = roomsArray[i];
		if (!IsValid(R)) { roomsArray.RemoveAtSwap(i); continue; }

		// On ne touche pas aux majors, ni aux couloirs déjà listés
		if (R->isMajor || CorridorRooms.Contains(R) || R->Tags.Contains(TEXT("Corridor")))
			continue;

		// => c’est une MINOR room : garde-la seulement si elle intersecte un couloir
		const FBox RB = MakeActorBox(R, /*Pad*/ 0.f);
		bool bTouchesCorridor = false;

		for (const FBox& CB : CorridorBoxes)
		{
			if (RB.Intersect(CB)) { bTouchesCorridor = true; break; }
		}

		if (!bTouchesCorridor)
		{
			R->Destroy();
			roomsArray.RemoveAtSwap(i);
		}
	}
}

bool AGenerator::FillAxisGapBetween(AActor* A, AActor* B)
{
    if (!IsValid(A) || !IsValid(B)) return false;

    FVector2D Amin, Amax, Bmin, Bmax;
    float ZA, ZB;
    GetBox2D(A, Amin, Amax, ZA);
    GetBox2D(B, Bmin, Bmax, ZB);

    // --- Cas 1 : faces verticales qui se font face (gap sur X, chevauchement sur Y)
    if (Amax.X <= Bmin.X) // A à gauche, B à droite
    {
        const float gap = Bmin.X - Amax.X;
        const float overlapY = FMath::Min(Amax.Y, Bmax.Y) - FMath::Max(Amin.Y, Bmin.Y);
        if (gap > 0.f && gap <= MaxFillGap && overlapY >= MinOverlapToFill)
        {
            const float y0 = FMath::Max(Amin.Y, Bmin.Y);
            const float y1 = FMath::Min(Amax.Y, Bmax.Y);
            return SpawnRectFill(FVector2D(Amax.X, y0), FVector2D(Bmin.X, y1), FMath::Min(ZA,ZB)) != nullptr;
        }
    }
    else if (Bmax.X <= Amin.X) // B à gauche, A à droite
    {
        const float gap = Amin.X - Bmax.X;
        const float overlapY = FMath::Min(Amax.Y, Bmax.Y) - FMath::Max(Amin.Y, Bmin.Y);
        if (gap > 0.f && gap <= MaxFillGap && overlapY >= MinOverlapToFill)
        {
            const float y0 = FMath::Max(Amin.Y, Bmin.Y);
            const float y1 = FMath::Min(Amax.Y, Bmax.Y);
            return SpawnRectFill(FVector2D(Bmax.X, y0), FVector2D(Amin.X, y1), FMath::Min(ZA,ZB)) != nullptr;
        }
    }

    // --- Cas 2 : faces horizontales qui se font face (gap sur Y, chevauchement sur X)
    if (Amax.Y <= Bmin.Y) // A en bas, B en haut
    {
        const float gap = Bmin.Y - Amax.Y;
        const float overlapX = FMath::Min(Amax.X, Bmax.X) - FMath::Max(Amin.X, Bmin.X);
        if (gap > 0.f && gap <= MaxFillGap && overlapX >= MinOverlapToFill)
        {
            const float x0 = FMath::Max(Amin.X, Bmin.X);
            const float x1 = FMath::Min(Amax.X, Bmax.X);
            return SpawnRectFill(FVector2D(x0, Amax.Y), FVector2D(x1, Bmin.Y), FMath::Min(ZA,ZB)) != nullptr;
        }
    }
    else if (Bmax.Y <= Amin.Y) // B en bas, A en haut
    {
        const float gap = Amin.Y - Bmax.Y;
        const float overlapX = FMath::Min(Amax.X, Bmax.X) - FMath::Max(Amin.X, Bmin.X);
        if (gap > 0.f && gap <= MaxFillGap && overlapX >= MinOverlapToFill)
        {
            const float x0 = FMath::Max(Amin.X, Bmin.X);
            const float x1 = FMath::Min(Amax.X, Bmax.X);
            return SpawnRectFill(FVector2D(x0, Bmax.Y), FVector2D(x1, Amin.Y), FMath::Min(ZA,ZB)) != nullptr;
        }
    }

    return false;
}

void AGenerator::BuildFillers()
{
	// --- Snapshots pour ne PAS modifier les arrays pendant qu’on itère
	const TArray<ARoom*> RoomsSnap = roomsArray;          // si tu ne modifies pas roomsArray ici, ce n'est pas obligé
	const TArray<ARoom*> CorrSnap  = CorridorRooms;       // important !

	TArray<FRectToFill> ToCreate;
	ToCreate.Reserve(128);

	// 1) Room ↔ Corridor
	for (ARoom* R : RoomsSnap)
	{
		if (!IsValid(R)) continue;
		for (ARoom* C : CorrSnap)
		{
			if (!IsValid(C)) continue;
			FRectToFill Rect{};
			if (ComputeAxisGapBetween(R, C, Rect))
				ToCreate.Add(Rect);
		}
	}

	// 2) Corridor ↔ Corridor
	for (int32 i = 0; i < CorrSnap.Num(); ++i)
	{
		ARoom* A = CorrSnap[i]; if (!IsValid(A)) continue;
		for (int32 j = i + 1; j < CorrSnap.Num(); ++j)
		{
			ARoom* B = CorrSnap[j]; if (!IsValid(B)) continue;
			FRectToFill Rect{};
			if (ComputeAxisGapBetween(A, B, Rect))
				ToCreate.Add(Rect);
		}
	}

	// 3) On spawn MAINTENANT (hors itérations) → pas d’invalidation d’itérateur
	for (const FRectToFill& R : ToCreate)
	{
		SpawnRectFill(R.MinXY, R.MaxXY, R.Z);
	}
}

bool AGenerator::ComputeAxisGapBetween(AActor* A, AActor* B, FRectToFill& OutRect)
{
    if (!IsValid(A) || !IsValid(B)) return false;

    auto GetBox2D = [](AActor* Act, FVector2D& MinXY, FVector2D& MaxXY, float& Z)
    {
        FVector C, E; Act->GetActorBounds(false, C, E);
        MinXY = { C.X - E.X, C.Y - E.Y };
        MaxXY = { C.X + E.X, C.Y + E.Y };
        Z = C.Z;
    };

    FVector2D Amin, Amax, Bmin, Bmax;
    float ZA, ZB;
    GetBox2D(A, Amin, Amax, ZA);
    GetBox2D(B, Bmin, Bmax, ZB);

    // Faces verticales qui se font face (gap sur X, chevauchement sur Y)
    if (Amax.X <= Bmin.X)
    {
        const float gap = Bmin.X - Amax.X;
        const float overlapY = FMath::Min(Amax.Y, Bmax.Y) - FMath::Max(Amin.Y, Bmin.Y);
        if (gap > 0.f && gap <= MaxFillGap && overlapY >= MinOverlapToFill)
        {
            OutRect = { FVector2D(Amax.X, FMath::Max(Amin.Y, Bmin.Y)),
                        FVector2D(Bmin.X, FMath::Min(Amax.Y, Bmax.Y)),
                        FMath::Min(ZA, ZB) };
            return true;
        }
    }
    else if (Bmax.X <= Amin.X)
    {
        const float gap = Amin.X - Bmax.X;
        const float overlapY = FMath::Min(Amax.Y, Bmax.Y) - FMath::Max(Amin.Y, Bmin.Y);
        if (gap > 0.f && gap <= MaxFillGap && overlapY >= MinOverlapToFill)
        {
            OutRect = { FVector2D(Bmax.X, FMath::Max(Amin.Y, Bmin.Y)),
                        FVector2D(Amin.X, FMath::Min(Amax.Y, Bmax.Y)),
                        FMath::Min(ZA, ZB) };
            return true;
        }
    }

    // Faces horizontales qui se font face (gap sur Y, chevauchement sur X)
    if (Amax.Y <= Bmin.Y)
    {
        const float gap = Bmin.Y - Amax.Y;
        const float overlapX = FMath::Min(Amax.X, Bmax.X) - FMath::Max(Amin.X, Bmin.X);
        if (gap > 0.f && gap <= MaxFillGap && overlapX >= MinOverlapToFill)
        {
            OutRect = { FVector2D(FMath::Max(Amin.X, Bmin.X), Amax.Y),
                        FVector2D(FMath::Min(Amax.X, Bmax.X), Bmin.Y),
                        FMath::Min(ZA, ZB) };
            return true;
        }
    }
    else if (Bmax.Y <= Amin.Y)
    {
        const float gap = Amin.Y - Bmax.Y;
        const float overlapX = FMath::Min(Amax.X, Bmax.X) - FMath::Max(Amin.X, Bmin.X);
        if (gap > 0.f && gap <= MaxFillGap && overlapX >= MinOverlapToFill)
        {
            OutRect = { FVector2D(FMath::Max(Amin.X, Bmin.X), Bmax.Y),
                        FVector2D(FMath::Min(Amax.X, Bmax.X), Amin.Y),
                        FMath::Min(ZA, ZB) };
            return true;
        }
    }

    return false;
}

ARoom* AGenerator::SpawnRectFill(const FVector2D& MinXY, const FVector2D& MaxXY, float Z)
{
	if (!FillerToSpawn) return nullptr;

	const FVector2D Size = MaxXY - MinXY;
	if (Size.X <= 1.f || Size.Y <= 1.f) return nullptr;

	const FVector Mid( (MinXY.X + MaxXY.X) * 0.5f, (MinXY.Y + MaxXY.Y) * 0.5f, FillerZ );

	FActorSpawnParameters P;
	P.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ARoom* R = GetWorld()->SpawnActor<ARoom>(FillerToSpawn, Mid, FRotator::ZeroRotator, P);
	if (!R) return nullptr;

	FVector C, E; R->GetActorBounds(false, C, E);
	const float SX = (Size.X * 0.5f) / FMath::Max(1.f, E.X);
	const float SY = (Size.Y * 0.5f) / FMath::Max(1.f, E.Y);
	R->SetActorScale3D(FVector(SX, SY, 0.1f));

	if (CorridorsRoot) R->AttachToComponent(CorridorsRoot, FAttachmentTransformRules::KeepWorldTransform);
	R->Tags.Add(TEXT("Filler"));

	// On enregistre APRÈS, en-dehors des boucles d’itération
	CorridorRooms.Add(R);
	FillerModules.Add(R);
	return R;
}