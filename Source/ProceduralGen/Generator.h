// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Room.h>
#include "DungeonTypes.h"
#include "Generator.generated.h"

UCLASS()
class PROCEDURALGEN_API AGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(CallInEditor, Category = "Dungeon|MainFunction")
	void MakeDungeon();

	UFUNCTION(CallInEditor, Category = "Dungeon|Custom")
	void SpawnRoomsInRadius();

	UFUNCTION(CallInEditor, Category = "Dungeon|Custom")
	void SeparateRooms();

	UFUNCTION(CallInEditor, Category = "Dungeon|MainFunction")
	void ClearAll();

	UFUNCTION(CallInEditor, Category = "Dungeon|ClearDetails")
	void ClearRooms();

	UFUNCTION(CallInEditor, Category = "Dungeon|ClearDetails")
	void ClearCorridors();

	UFUNCTION(CallInEditor, Category = "Dungeon|ClearDetails")
	void ClearSuperTriangle();

	UFUNCTION(CallInEditor, Category = "Dungeon|ClearDetails")
	void ClearTriangles();

	UFUNCTION(CallInEditor, Category = "Dungeon|Triangulation")
	void SetSuperTriangle();

	UFUNCTION(CallInEditor, Category = "Dungeon|Triangulation")
	void Triangulation();

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	float CorridorThickness = 200.f; // épaisseur visuelle des lignes debug

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	float CorridorZ = 8.f; // hauteur des lignes debug au-dessus du sol

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	bool bRandomHorizontalFirst = true; // pour varier le L

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	TSubclassOf<ARoom> CorridorToSpawn; // mesh rectangulaire (même BP que RoomToSpawn si tu veux)

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	float CorridorWidth = 600.f;   // largeur visuelle du couloir (axe Y du mesh)

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	float CorridorZSpawn = 0.f;    // hauteur du sol pour les couloirs (0 si tes rooms sont à Z=0)

	
	USceneComponent* CorridorsRoot = nullptr;
	TArray<ARoom*> CorridorRooms;   

	//Triangulation Functions
	TArray<Triangle> CollectBadTriangles(int i);
	TArray<Edge> ExtractFrontierEdges();
	Edge* FindOrCreateEdge(Point* A, Point* B);
	void DeleteBadSuperTriangles();
	void ReasignPointPosition();

	// Prim Algoruithm Functions
	void PrimAlgorithm();
	Point* SelectRandomMajorPoint();
	void PushPathPossibility(Point* current);
	static FVector RandomPointInDisk(float radius);

	// Corridors Functions
	FVector DoorToward(const Point* From, const Point* Toward) const;
	bool TryAlignedDoors(const Point* A, const Point* B, FVector& DoorA, FVector& DoorB) const;
	void BuildCorridorsFromMST_Meshes();                   
	void PlaceCorridorStraightMesh(const FVector& A, const FVector& B);
	void PlaceCorridorLMesh(const FVector& A, const FVector& B, bool HorizontalFirst);
	ARoom* SpawnCorridorSegment(const FVector& A, const FVector& B);
	void EnsureCorridorBaseExtents() const;
	void RemoveMinorRoomsOutOfDungeon(float CorridorPad);

	// Filler Functions 
	bool FillAxisGapBetween(AActor* A, AActor* B);
	ARoom* SpawnRectFill(const FVector2D& MinXY, const FVector2D& MaxXY, float Z);
	void BuildFillers();
	bool ComputeAxisGapBetween(AActor* A, AActor* B, FRectToFill& OutRect);

	
	int roomNumber = 100;
	float minSizeX = 10.f;
	float minSizeY = 10.f;

	float maxSizeX = 100.f;
	float maxSizeY = 100.f;

	UPROPERTY(EditAnywhere, Category="Dungeon|Spawn")
	float areaLimit = 900.f;
	UPROPERTY(EditAnywhere, Category="Dungeon|Spawn")
	TSubclassOf<ARoom> RoomToSpawn;
	UPROPERTY(EditAnywhere, Category = "Dungeon|Spawn")
	UMaterialInterface* MajorMat;
	UPROPERTY(EditAnywhere, Category = "Dungeon|Spawn")
	UMaterialInterface* SecondaryMat;
	
	float initialSpawnRadius = 300.f;
	float padding = 300.f;
	int32 maxIteration = 1000.f;

	mutable float BaseHalfX = -1.f;
	mutable float BaseHalfY = -1.f;

	// Arrays
	TArray<ARoom*> roomsArray;
	TArray<ARoom*> trianglesSummits;
	TArray<ARoom*> DebugRooms;
	TArray<Triangle> trianglesArray;
	TArray<Triangle> BadTriangles;
	TArray<Point*> PointsArray;
	TArray<Point*> MajorPoints;
	TArray<FMstEntry> CandidateEdges;
	TArray<Edge*> MSTEdges;
	TArray<Edge*> AllEdges;
	TArray<ARoom*> FillerModules; 
	
	Triangle superTriangle;
	DungeonTypes DungeonFunction = DungeonTypes();
	
	// Filler modules
    UPROPERTY(EditAnywhere, Category="Dungeon|Filler")
    TSubclassOf<ARoom> FillerToSpawn;       
    UPROPERTY(EditAnywhere, Category="Dungeon|Filler")
    float MaxFillGap = 120.f;               
    UPROPERTY(EditAnywhere, Category="Dungeon|Filler")
    float MinOverlapToFill = 80.f;          
    UPROPERTY(EditAnywhere, Category="Dungeon|Filler")
    float FillerZ = 0.f;
    

             

	//Statics Functions
	static bool Intersect1D(float a0, float a1, float b0, float b1, float& OutMid, float& OutLen)
	{
		const float lo = FMath::Max(a0, b0);
		const float hi = FMath::Min(a1, b1);
		OutLen = hi - lo;
		if (OutLen > 0.f)
		{
			OutMid = 0.5f * (lo + hi);
			return true;
		}
		return false;
	}

	static FBox MakeActorBox(AActor* A, float Pad = 0.f)
	{
		FVector C, E;
		if (!IsValid(A)) return FBox(ForceInit);
		A->GetActorBounds(false, C, E);
		if (Pad > 0.f) E += FVector(Pad);
		return FBox(C - E, C + E);
	}
	
	static void GetBox2D(AActor* Act, FVector2D& MinXY, FVector2D& MaxXY, float& Z)
	{
		FVector C, E; Act->GetActorBounds(false, C, E);
		MinXY = FVector2D(C.X - E.X, C.Y - E.Y);
		MaxXY = FVector2D(C.X + E.X, C.Y + E.Y);
		Z = C.Z;
	}
};




