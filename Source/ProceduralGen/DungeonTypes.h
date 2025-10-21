#pragma once

#include "CoreMinimal.h"
#include "Room.h"

/**
 * @struct Point
 * @brief Représente un nœud dans le graphe de génération de donjon.
 *
 * Contient la position, la référence à la salle associée, les arêtes connectées,
 * et un indicateur de visite.
 */
struct Point
{
	FVector Pos;                 /**< Position du point dans le monde. */
	ARoom* Room;                 /**< Salle associée à ce point. */
	TArray<struct Edge*> Edges;  /**< Liste des arêtes connectées à ce point. */
	bool bVisited = false;       /**< Indique si le point a déjà été visité. */
};

/**
 * @struct Edge
 * @brief Représente une arête reliant deux points du graphe.
 *
 * Contient deux extrémités (A et B) et leur distance (poids).
 */
struct Edge
{
	Point* A;            /**< Premier point de l’arête. */
	Point* B;            /**< Second point de l’arête. */
	float weight = 0.f;  /**< Poids de l’arête (distance entre A et B). */

	/**
	 * @brief Met à jour la longueur (poids) de l’arête en 2D.
	 *
	 * Calcule la distance 2D entre les deux points A et B et met à jour le champ `weight`.
	 */
	void UpdateLength()
	{
		weight = FVector::Dist2D(A ? A->Pos : FVector::ZeroVector,
								 B ? B->Pos : FVector::ZeroVector);
	}

	/**
	 * @brief Vérifie si deux arêtes sont équivalentes (sans tenir compte de l’ordre des points).
	 *
	 * @param Other Autre arête à comparer.
	 * @return true si les deux arêtes relient les mêmes points, peu importe l’ordre.
	 */
	bool operator==(const Edge& Other) const
	{
		return (A == Other.A && B == Other.B) ||
			   (A == Other.B && B == Other.A);
	}
};

/**
 * @struct FMstEntry
 * @brief Entrée utilisée pour la construction d’un arbre couvrant minimal (MST).
 *
 * Stocke l’arête, les points de départ/arrivée, et le poids associé.
 */
struct FMstEntry
{
	Edge*  E       = nullptr;  /**< Arête associée. */
	Point* From    = nullptr;  /**< Côté déjà visité. */
	Point* To      = nullptr;  /**< Côté non visité. */
	float  Weight  = 0.f;      /**< Poids (longueur de l’arête). */
};

/**
 * @struct Triangle
 * @brief Représente un triangle dans la triangulation.
 *
 * Permet de tester la présence d’un point dans son cercle circonscrit
 * et de vérifier si une arête donnée appartient au triangle.
 */
struct Triangle
{
	TArray<Point*> Points;  /**< Les trois points formant le triangle. */

	/**
	 * @brief Vérifie si un point donné se trouve dans le cercle circonscrit du triangle.
	 *
	 * @param P Position du point à tester.
	 * @return true si le point P se trouve à l’intérieur du cercle circonscrit, false sinon.
	 */
	bool IsPointInPointCircumCircle(FVector P);

	/**
	 * @brief Vérifie si le triangle contient une arête donnée.
	 *
	 * @param E Arête à tester.
	 * @return true si l’arête fait partie du triangle, false sinon.
	 */
	bool HasEdge(const Edge& E) const
	{
		return Edge{ Points[0], Points[1] } == E ||
			   Edge{ Points[1], Points[2] } == E ||
			   Edge{ Points[2], Points[0] } == E;
	}

	/**
	 * @brief Compare deux triangles pour vérifier s’ils sont identiques.
	 *
	 * @param Other Autre triangle à comparer.
	 * @return true si les trois points sont identiques, false sinon.
	 */
	bool operator==(const Triangle& Other) const
	{
		return Points[0] == Other.Points[0] &&
			   Points[1] == Other.Points[1] &&
			   Points[2] == Other.Points[2];
	}
};

/**
 * @struct FRectToFill
 * @brief Définit une zone rectangulaire à remplir dans le plan XY, avec une hauteur Z spécifique.
 */
struct FRectToFill
{
	FVector2D MinXY;  /**< Coin inférieur gauche du rectangle. */
	FVector2D MaxXY;  /**< Coin supérieur droit du rectangle. */
	float Z;          /**< Hauteur du rectangle. */
};

/**
 * @class DungeonTypes
 * @brief Contient des utilitaires pour la manipulation de points et d’arêtes du donjon.
 *
 * Fournit des fonctions statiques pour créer des points, des entrées MST, et gérer les arêtes sans duplicata.
 */
class PROCEDURALGEN_API DungeonTypes
{
public:
	DungeonTypes() {}

	/**
	 * @brief Vérifie si une arête correspond à deux points donnés, sans tenir compte de l’ordre.
	 *
	 * @param E Arête à tester.
	 * @param A Premier point.
	 * @param B Second point.
	 * @return true si l’arête relie les deux points, peu importe l’ordre.
	 */
	static bool SameUndirected(const Edge* E, const Point* A, const Point* B)
	{
		return (E->A == A && E->B == B) || (E->A == B && E->B == A);
	}

	/**
	 * @brief Crée un point associé à une salle donnée.
	 *
	 * @param room Référence vers la salle.
	 * @return Pointeur vers le point créé.
	 */
	Point* MakePoint(ARoom* room);

	/**
	 * @brief Crée une entrée MST à partir d’une arête et de deux points.
	 *
	 * @param E Arête considérée.
	 * @param From Point déjà visité.
	 * @param To Point non visité.
	 * @return Structure FMstEntry contenant l’entrée construite.
	 */
	FMstEntry MakeEntry(Edge* E, Point* From, Point* To);

	/**
	 * @brief Vérifie si un point possède une arête donnée.
	 *
	 * @param P Point à vérifier.
	 * @param E Arête recherchée.
	 * @return true si l’arête appartient au point, false sinon.
	 */
	bool PointHasEdge(Point* P, Edge* E);

	/**
	 * @brief Ajoute une arête à un point, sans créer de doublon.
	 *
	 * @param P Point cible.
	 * @param E Arête à ajouter.
	 */
	void AddEdgeToPointNoDup(Point* P, Edge* E);
};
