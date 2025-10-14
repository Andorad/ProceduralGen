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
	bool PointHasEdge(Point* P, Edge* E);
	void AddEdgeToPointNoDup(Point* P, Edge* E);
	
};
