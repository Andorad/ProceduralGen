// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Room.h>
#include "Generator.generated.h"

struct Edge; 

struct Point
{
	FVector Pos;
	ARoom* Room;
	TArray<Edge*> Edges;
};

struct Edge
{
	Point* A;
	Point* B;
	bool operator==(const Edge& Other)  const
	{
		return (A == Other.A && B == Other.B || (A == Other.B && B == Other.A));
	}
};



struct Triangle
{
	TArray<Point*> Points;
	bool IsPointInPointCircumCircle(FVector P) ;
	bool HasEdge(const Edge& E) const
	{
		return (Edge{Points[0],Points[1]} == E) || (Edge{Points[1],Points[2]} == E) || (Edge{Points[2],Points[0]} == E);
	}

	bool operator== (const Triangle& Other) const
	{
		return Points[0] == Other.Points[0] && Points[1] == Other.Points[1] && Points[2] == Other.Points[2];
	}
	
};
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
	
	UFUNCTION(CallInEditor, Category = "Dungeon")
	void MakeDungeon();

	UFUNCTION(CallInEditor, Category = "Custom")
	void SpawnRoomsInRadius();

	UFUNCTION(CallInEditor, Category = "Custom")
	void SeparateRooms();

	UFUNCTION(CallInEditor, Category = "Clear")
	void ClearAll();

	UFUNCTION(CallInEditor, Category = "Clear")
	void ClearRooms();

	UFUNCTION(CallInEditor, Category = "Clear")
	void ClearSuperTriangle();

	UFUNCTION(CallInEditor, Category = "Clear")
	void ClearTriangles();

	UFUNCTION(CallInEditor, Category = "Triangulation")
	void SetSuperTriangle();

	UFUNCTION(CallInEditor, Category = "Triangulation")
	void Triangulation();



	void DeleteBadSuperTriangles();
	
	void DrawTriangles();

	void ReasignPointPosition();

	void AddEdgeToPointNoDup(Point* P, Edge* E);

	Edge* FindOrCreateEdge(Point* A, Point* B);

	bool PointHasEdge(Point* P, Edge* E);

	static bool SameUndirected(const Edge* E, const Point* A, const Point* B)
	{
		return ( (E->A == A && E->B == B) || (E->A == B && E->B == A) );
	}

	Point* MakePoint(bool isMajor, ARoom* room);

	static FVector RandomPointInDisk(float radius);

	int roomNumber = 100;

	float minSizeX = 10.f;
	float minSizeY = 10.f;

	float maxSizeX = 50.f;
	float maxSizeY = 50.f;

	float areaLimit = 900.f;

	float initialSpawnRadius = 300.f;

	float padding = 300.f;
	int32 maxIteration = 1000.f;

	TArray<ARoom*> roomsArray;

	UPROPERTY(EditAnywhere, Category="Spawn")
	TSubclassOf<ARoom> RoomToSpawn;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	UMaterialInterface* MajorMat;
	UPROPERTY(EditAnywhere, Category = "Spawn")
	UMaterialInterface* SecondaryMat;

	TArray<Triangle> validatedTrianglesArray;
	TArray<Triangle> trianglesArray;

	TArray<Edge*> AllEdges;

	TArray<Point*> PointsArray;

	TArray<ARoom*> trianglesSummits;
	TArray<ARoom*> DebugRooms;

	Triangle superTriangle;

};




