// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonTypes.h"
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

DungeonTypes::DungeonTypes()
{


}

DungeonTypes::~DungeonTypes()
{
}
Point* DungeonTypes::MakePoint(bool isMajor, ARoom* room)
{
	Point* point = new Point();
	point->Room = room;
	point->Pos = room->GetActorLocation();
	return point;
}
FMstEntry DungeonTypes::MakeEntry(Edge* E, Point* From, Point* To,float Weight)
{
	FMstEntry Ent;
	Ent.E    = E;
	Ent.From = From;
	Ent.To   = To;
	Ent.Weight  = (E->weight > 0.f) ? E->weight : FVector::Dist2D(From->Pos, To->Pos);
	return Ent;
}
void DungeonTypes::AddEdgeToPointNoDup(Point* P, Edge* E)
{
	if (!P || !E) return;
	if (!PointHasEdge(P, E))
	{
		P->Edges.Add(E);
	}
}

bool DungeonTypes::PointHasEdge( Point* P, Edge* E)
{
	if (!P || !E) return false;
	for ( Edge* Existing : P->Edges)
	{
		if (SameUndirected(Existing, E->A, E->B))
			return true;
	}
	return false;
}

