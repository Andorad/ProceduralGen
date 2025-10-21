#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Room.h>
#include "DungeonTypes.h"
#include "Generator.generated.h"

/**
 * @class AGenerator
 * @brief Classe principale de génération procédurale du donjon.
 *
 * Gère la génération complète du donjon, incluant :
 * - Le placement aléatoire et la séparation des salles.
 * - La triangulation pour relier les pièces.
 * - L'application de l’algorithme de Prim pour construire un arbre couvrant minimal.
 * - La génération de couloirs, modules de remplissage et nettoyage des entités.
 */
UCLASS()
class PROCEDURALGEN_API AGenerator : public AActor
{
	GENERATED_BODY()
	
public:
	/** @brief Constructeur par défaut. */
	AGenerator();

protected:
	virtual void BeginPlay() override;
	
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ======================================================================
	// -------------------------- DUNGEON CONTROL ---------------------------
	// ======================================================================

	/**
	 * @brief Génère le donjon complet (point d’entrée principal).
	 *
	 * Exécute la séquence complète : spawn des salles, séparation, triangulation,
	 * construction des chemins et couloirs.
	 */
	UFUNCTION(CallInEditor, Category = "Dungeon|Creation")
	void MakeDungeon();

	/**
	 * @brief Fait apparaître les salles dans un rayon aléatoire.
	 */
	UFUNCTION(CallInEditor, Category = "Dungeon|Creation")
	void SpawnRoomsInRadius();

	/**
	 * @brief Sépare les salles qui se chevauchent pour éviter les collisions.
	 */
	UFUNCTION(CallInEditor, Category = "Dungeon|Creation")
	void SeparateRooms();

	/**
	 * @brief Nettoie complètement la scène du donjon (toutes entités).
	 */
	UFUNCTION(CallInEditor, Category = "Dungeon|Destruction")
	void ClearAll();

	/**
	 * @brief Supprime toutes les salles générées.
	 */
	UFUNCTION(CallInEditor, Category = "Dungeon|Destruction")
	void ClearRooms();

	/**
	 * @brief Supprime tous les couloirs générés.
	 */
	UFUNCTION(CallInEditor, Category = "Dungeon|Destruction")
	void ClearCorridors();

	/**
	 * @brief Supprime le super-triangle utilisé pour la triangulation.
	 */
	UFUNCTION(CallInEditor, Category = "Dungeon|Destruction")
	void ClearSuperTriangle();

	/**
	 * @brief Supprime toutes les données de triangulation.
	 */
	UFUNCTION(CallInEditor, Category = "Dungeon|Destruction")
	void ClearTriangles();

	/**
	 * @brief Initialise le super-triangle pour la triangulation Delaunay.
	 */
	UFUNCTION(CallInEditor, Category = "Dungeon|Creation")
	void SetSuperTriangle();

	/**
	 * @brief Exécute la triangulation Delaunay pour connecter les salles.
	 */
	UFUNCTION(CallInEditor, Category = "Dungeon|Creation")
	void Triangulation();

	// ======================================================================
	// -------------------------- CORRIDOR SETTINGS -------------------------
	// ======================================================================

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	float CorridorThickness = 200.f; /**< Épaisseur géométrique des couloirs. */

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	float CorridorZ = 8.f; /**< Hauteur des lignes de debug au-dessus du sol. */

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	bool bRandomHorizontalFirst = true; /**< Si vrai, alterne l’ordre de placement des couloirs en L. */

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	TSubclassOf<ARoom> CorridorToSpawn; /**< Classe utilisée pour les meshes de couloirs. */

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	float CorridorWidth = 600.f; /**< Largeur visuelle des couloirs. */

	UPROPERTY(EditAnywhere, Category="Dungeon|Corridor")
	float CorridorZSpawn = 0.f; /**< Hauteur d’apparition des couloirs. */

	USceneComponent* CorridorsRoot = nullptr; /**< Racine hiérarchique des couloirs. */
	TArray<ARoom*> CorridorRooms; /**< Liste des couloirs instanciés. */

	// ======================================================================
	// ----------------------- TRIANGULATION METHODS ------------------------
	// ======================================================================

	/**
	 * @brief Récupère les triangles invalides pour un point donné.
	 *
	 * @param I Index du point à évaluer.
	 * @return Liste des triangles invalides (à retirer).
	 */
	TArray<Triangle> CollectBadTriangles(int I);

	/**
	 * @brief Extrait les arêtes frontières (bordure) du maillage triangulé.
	 * @return Liste d’arêtes formant la frontière.
	 */
	TArray<Edge> ExtractFrontierEdges();

	/**
	 * @brief Trouve ou crée une arête entre deux points donnés.
	 *
	 * @param A Premier point.
	 * @param B Second point.
	 * @return Pointeur vers l’arête correspondante.
	 */
	Edge* FindOrCreateEdge(Point* A, Point* B);

	/** @brief Supprime les triangles associés au super-triangle. */
	void DeleteBadSuperTriangles();

	/** @brief Réassigne la position des points après déplacement des salles. */
	void ReasignPointPosition();

	// ======================================================================
	// -------------------------- PRIM ALGORITHM ----------------------------
	// ======================================================================

	/** @brief Exécute l’algorithme de Prim pour construire le graphe des connexions principales. */
	void PrimAlgorithm();

	/** @brief Sélectionne un point majeur aléatoire pour initier l’arbre couvrant. */
	Point* SelectRandomMajorPoint();

	/** @brief Ajoute les arêtes possibles à partir d’un point courant. */
	void PushPathPossibility(Point* Current);

	/**
	 * @brief Génère un point aléatoire dans un disque de rayon donné.
	 * @param Radius Rayon maximal.
	 * @return Position aléatoire dans le disque.
	 */
	static FVector RandomPointInDisk(float Radius);

	// ======================================================================
	// -------------------------- CORRIDOR BUILDING -------------------------
	// ======================================================================

	/**
	 * @brief Calcule la position de la porte d’un point vers un autre.
	 * @param From Point de départ.
	 * @param Toward Point visé.
	 * @return Position 3D de la porte.
	 */
	FVector DoorToward(const Point* From, const Point* Toward) const;

	/**
	 * @brief Essaie d’aligner deux portes entre deux salles adjacentes.
	 * @param A Premier point.
	 * @param B Second point.
	 * @param DoorA Sortie : position de la porte A.
	 * @param DoorB Sortie : position de la porte B.
	 * @return true si un alignement valide est trouvé.
	 */
	bool TryAlignedDoors(const Point* A, const Point* B, FVector& DoorA, FVector& DoorB) const;

	/** @brief Construit les couloirs à partir du graphe MST en instanciant les meshes. */
	void BuildCorridorsFromMST_Meshes();

	/** @brief Place un couloir rectiligne entre deux points. */
	void PlaceCorridorStraightMesh(const FVector& A, const FVector& B);

	/**
	 * @brief Place un couloir en forme de L entre deux points.
	 * @param A Point de départ.
	 * @param B Point d’arrivée.
	 * @param HorizontalFirst Si vrai, commence par le segment horizontal.
	 */
	void PlaceCorridorLMesh(const FVector& A, const FVector& B, bool HorizontalFirst);

	/**
	 * @brief Fait apparaître un segment de couloir entre deux points donnés.
	 * @param A Point de départ.
	 * @param B Point d’arrivée.
	 * @return Référence vers le couloir créé.
	 */
	ARoom* SpawnCorridorSegment(const FVector& A, const FVector& B);

	/** @brief Vérifie et initialise les dimensions de base du couloir. */
	void EnsureCorridorBaseExtents() const;

	/** @brief Supprime les salles mineures situées hors de la zone principale du donjon. */
	void RemoveMinorRoomsOutOfDungeon();

	// ======================================================================
	// ----------------------------- FILLER ---------------------------------
	// ======================================================================

	/**
	 * @brief Remplit un espace vide entre deux acteurs si possible.
	 * @param A Premier acteur.
	 * @param B Second acteur.
	 * @return true si un remplissage a été effectué.
	 */
	bool FillAxisGapBetween(AActor* A, AActor* B);

	/**
	 * @brief Fait apparaître un module rectangulaire de remplissage.
	 * @param MinXY Coordonnée minimale du rectangle.
	 * @param MaxXY Coordonnée maximale du rectangle.
	 * @param Z Hauteur d’apparition.
	 * @return Référence vers le module créé.
	 */
	ARoom* SpawnRectFill(const FVector2D& MinXY, const FVector2D& MaxXY, float Z);

	/**
	 * @brief Calcule un espace vide entre deux acteurs.
	 * @param A Premier acteur.
	 * @param B Second acteur.
	 * @param OutRect Sortie : rectangle à remplir.
	 * @return true si un espace vide est détecté.
	 */
	bool ComputeAxisGapBetween(AActor* A, AActor* B, FRectToFill& OutRect);

	// ======================================================================
	// ------------------------------- SPAWN --------------------------------
	// ======================================================================

	UPROPERTY(EditAnywhere, Category="Dungeon|Spawn")
	int roomNumber = 100; /**< Nombre total de salles à générer. */

	float minSizeX = 10.f; /**< Taille minimale en X. */
	float minSizeY = 10.f; /**< Taille minimale en Y. */
	float maxSizeX = 100.f; /**< Taille maximale en X. */
	float maxSizeY = 100.f; /**< Taille maximale en Y. */
	float CorridorPad = 8.f; /**< Marge entre les couloirs et les pièces. */

	UPROPERTY(EditAnywhere, Category="Dungeon|Spawn")
	float areaLimit = 900.f; /**< Aire maximale du donjon. */

	UPROPERTY(EditAnywhere, Category="Dungeon|Spawn")
	TSubclassOf<ARoom> RoomToSpawn; /**< Classe utilisée pour instancier les salles. */

	UPROPERTY(EditAnywhere, Category = "Dungeon|Spawn")
	UMaterialInterface* MajorMat; /**< Matériau utilisé pour les salles principales. */

	UPROPERTY(EditAnywhere, Category = "Dungeon|Spawn")
	UMaterialInterface* SecondaryMat; /**< Matériau utilisé pour les salles secondaires. */

	float initialSpawnRadius = 300.f; /**< Rayon initial pour le spawn des salles. */
	float padding = 300.f; /**< Distance de sécurité entre les entités. */
	int32 maxIteration = 1000.f; /**< Nombre maximal d’itérations pour les boucles de génération. */

	mutable float BaseHalfX = -1.f; /**< Demi-largeur de base du couloir. */
	mutable float BaseHalfY = -1.f; /**< Demi-hauteur de base du couloir. */

	// ======================================================================
	// ------------------------------- ARRAYS -------------------------------
	// ======================================================================

	TArray<ARoom*> roomsArray;        /**< Liste de toutes les salles. */
	TArray<ARoom*> trianglesSummits;  /**< Salles formant les sommets de triangulation. */
	TArray<ARoom*> DebugRooms;        /**< Salles utilisées pour le debug. */
	TArray<Triangle> trianglesArray;  /**< Triangles valides. */
	TArray<Triangle> BadTriangles;    /**< Triangles invalides. */
	TArray<Point*> PointsArray;       /**< Points correspondants aux salles. */
	TArray<Point*> MajorPoints;       /**< Points principaux (centres de salles majeures). */
	TArray<FMstEntry> CandidateEdges; /**< Arêtes candidates pour Prim. */
	TArray<Edge*> MSTEdges;           /**< Arêtes de l’arbre couvrant minimal. */
	TArray<Edge*> AllEdges;           /**< Ensemble complet des arêtes. */
	TArray<ARoom*> FillerModules;     /**< Modules de remplissage générés. */

	Triangle superTriangle;           /**< Triangle englobant toutes les salles. */
	DungeonTypes DungeonFunction = DungeonTypes(); /**< Ensemble de fonctions utilitaires. */

	// ======================================================================
	// ---------------------------- FILLER MODULES --------------------------
	// ======================================================================

	UPROPERTY(EditAnywhere, Category="Dungeon|Filler")
	TSubclassOf<ARoom> FillerToSpawn; /**< Classe utilisée pour les modules de remplissage. */

	UPROPERTY(EditAnywhere, Category="Dungeon|Filler")
	float MaxFillGap = 120.f; /**< Distance maximale de remplissage. */

	UPROPERTY(EditAnywhere, Category="Dungeon|Filler")
	float MinOverlapToFill = 80.f; /**< Recouvrement minimal pour remplir un espace. */

	UPROPERTY(EditAnywhere, Category="Dungeon|Filler")
	float FillerZ = 0.f; /**< Hauteur d’apparition des fillers. */

	// ======================================================================
	// ---------------------------- STATIC HELPERS --------------------------
	// ======================================================================

	/**
	 * @brief Teste si deux intervalles 1D se recoupent.
	 * @param a0 Début du premier intervalle.
	 * @param a1 Fin du premier intervalle.
	 * @param b0 Début du second intervalle.
	 * @param b1 Fin du second intervalle.
	 * @param OutMid Sortie : milieu du chevauchement.
	 * @param OutLen Sortie : longueur du chevauchement.
	 * @return true si les intervalles s’intersectent.
	 */
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

	/**
	 * @brief Construit un FBox englobant un acteur.
	 * @param A Acteur cible.
	 * @param Pad Optionnel : marge ajoutée au box.
	 * @return FBox représentant les limites de l’acteur.
	 */
	static FBox MakeActorBox(AActor* A, float Pad = 0.f)
	{
		FVector C, E;
		if (!IsValid(A)) return FBox(ForceInit);
		A->GetActorBounds(false, C, E);
		if (Pad > 0.f) E += FVector(Pad);
		return FBox(C - E, C + E);
	}

	/**
	 * @brief Extrait les coordonnées 2D et hauteur d’un acteur.
	 * @param Act Acteur cible.
	 * @param MinXY Sortie : coordonnées minimales.
	 * @param MaxXY Sortie : coordonnées maximales.
	 * @param Z Sortie : hauteur moyenne.
	 */
	static void GetBox2D(AActor* Act, FVector2D& MinXY, FVector2D& MaxXY, float& Z)
	{
		FVector C, E; 
		Act->GetActorBounds(false, C, E);
		MinXY = FVector2D(C.X - E.X, C.Y - E.Y);
		MaxXY = FVector2D(C.X + E.X, C.Y + E.Y);
		Z = C.Z;
	}
};
