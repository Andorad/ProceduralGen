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
	//TArray<Edge> Edges;
};

struct Edge
{
	Point* A;
	Point* B;
	bool operator==(const Edge& Other)  const
	{
		return (A->Pos == Other.A->Pos && B->Pos == Other.B->Pos || (A->Pos == Other.B->Pos && B->Pos == Other.A->Pos));
	}
};



struct Triangle
{
	TArray<Point*> Points;
	//FVector pointA, pointB, pointC;
	bool IsPointInPointCircumCircle(FVector P) ;
	bool HasEdge(const Edge& E) const
	{
		return (Edge{Points[0],Points[1]} == E) || (Edge{Points[1],Points[2]} == E) || (Edge{Points[2],Points[0]} == E);
	}

	bool operator== (const Triangle& Other) const
	{
		return Points[0]->Pos == Other.Points[0]->Pos && Points[1]->Pos == Other.Points[1]->Pos && Points[2]->Pos == Other.Points[2]->Pos;
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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(CallInEditor, Category = "Custom")
	void SpawnRoomsInRadius();

	UFUNCTION(CallInEditor, Category = "Custom")
	void SeparateRooms();

	UFUNCTION(CallInEditor, Category = "Clear")
	void ClearRooms();

	UFUNCTION(CallInEditor, Category = "Clear")
	void ClearMajorRooms();

	UFUNCTION(CallInEditor, Category = "Clear")
	void ClearMinorRooms();

	UFUNCTION(CallInEditor, Category = "Triangulation")
	void SetSuperTriangle();

	UFUNCTION(CallInEditor, Category = "Triangulation")
	void Triangulation();

	UFUNCTION(CallInEditor, Category = "Clear")
	void ClearTriangles();

	void ClearBadSuperTriangles();
	
	void DrawTriangles();

	//void ConstructPointArray();

	

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

	TArray<ARoom*> majorRooms;
	TArray<ARoom*> minorRooms;
	TArray<ARoom*> roomsArray;

	UPROPERTY(EditAnywhere, Category="Spawn")
	TSubclassOf<ARoom> RoomToSpawn;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	UMaterialInterface* MajorMat;
	UPROPERTY(EditAnywhere, Category = "Spawn")
	UMaterialInterface* SecondaryMat;

	TArray<Triangle> validatedTrianglesArray;
	TArray<Triangle> trianglesArray;

	TArray<Point*> PointsArray;

	TArray<ARoom*> trianglesSummits;

	Triangle superTriangle;

};




