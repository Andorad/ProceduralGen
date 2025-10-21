#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Room.generated.h"

/**
 * @class ARoom
 * @brief Représente une salle dans le générateur procédural.
 * 
 * Cette classe définit un acteur représentant une salle dans le monde.
 * Chaque salle possède un maillage de base (PlaneMesh) et un matériau dynamique
 * qui peut être modifié à l’exécution. Elle peut également être marquée comme
 * « majeure ».
 */
UCLASS()
class PROCEDURALGEN_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	/** 
	 * @brief Constructeur par défaut de la classe ARoom.
	 * 
	 * Initialise les composants et prépare la salle pour l’ajout dans le monde.
	 */
	ARoom();

	/**
	 * @brief Définit la couleur ou le matériau de la salle.
	 * 
	 * Crée ou met à jour un matériau dynamique à partir du matériau fourni
	 * afin de changer la couleur de la salle dans le monde.
	 * 
	 * @param _mat Matériau de base utilisé pour générer le matériau dynamique.
	 */
	void SetColor(UMaterialInterface* _mat);

	/** 
	 * @brief Maillage statique représentant la surface de la salle (un simple plan par défaut).
	 */
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PlaneMesh;

	/** 
	 * @brief Instance dynamique du matériau appliqué à la salle.
	 * Permet de modifier ses paramètres (couleur, opacité, etc.) à l’exécution.
	 */
	UMaterialInstanceDynamic* DynamicMaterial;

	/** 
	 * @brief Indique si la salle est une salle majeure (par exemple une pièce principale ou centrale).
	 */
	bool isMajor = false;
};
