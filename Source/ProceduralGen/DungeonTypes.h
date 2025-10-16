// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Room.h"

/**
 * 
 */
 struct Edge; 
 
 struct Point
 {
 	FVector Pos;
 	ARoom* Room;
 	TArray<Edge*> Edges;
 	bool bVisited = false;
 };
 
 struct Edge
 {
 	Point* A;
 	Point* B;
 	float weight = 0.f;
 	void UpdateLength()
 	{
 		weight = FVector::Dist2D(A ? A->Pos : FVector::ZeroVector,
								  B ? B->Pos : FVector::ZeroVector);
 	}
 	bool operator==(const Edge& Other)  const
 	{
 		return (A == Other.A && B == Other.B || (A == Other.B && B == Other.A));
 	}
 };

struct FMstEntry
{
	Edge*  E   = nullptr;
	Point* From = nullptr;   // côté déjà visité
	Point* To   = nullptr;   // côté non visité
	float  Weight    = 0.f;       // poids (longueur)
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

struct FRectToFill { FVector2D MinXY, MaxXY; float Z; };
 
class PROCEDURALGEN_API DungeonTypes
{
public:
	DungeonTypes();
	~DungeonTypes();

	static bool SameUndirected(const Edge* E, const Point* A, const Point* B)
	{
		return ( (E->A == A && E->B == B) || (E->A == B && E->B == A) );
	}

	Point* MakePoint(bool isMajor, ARoom* room);
	FMstEntry MakeEntry(Edge* E, Point* From, Point* To,float Weight);
	bool PointHasEdge(Point* P, Edge* E);
	void AddEdgeToPointNoDup(Point* P, Edge* E);
	
};
