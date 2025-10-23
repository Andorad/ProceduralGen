#  Procedural Dungeon Generator – Unreal Engine 5

### Collaborators
Mathilde Grillon & Théo Lopez

##  Présentation

Ce projet implémente un **générateur procédural de donjons**, basé sur la **triangulation de Delaunay** et l’**algorithme de Prim**.  
Il permet de créer dynamiquement des réseaux de salles reliées entre elles par des couloirs, garantissant une structure cohérente.

Pour tester facilement, il est possible de cliquer sur l'objet Generator puis d'écrire **"Dungeon"** dans la barre de recherche pour
avoir accès à toutes les fonctions utiles à la création/destruction du donjon depuis l'éditeur.

---

##  Fondements théoriques

###  1. Triangulation de Delaunay


L’algorithme utilisé ici est **Bowyer-Watson (1981)** :
1. Créer un **super-triangle** englobant tous les points.
2. Ajouter les points un à un :
   - Supprimer les triangles dont le cercle circonscrit contient le point.
   - Identifier la frontière du "trou" ainsi créé.
   - Relier le nouveau point à toutes les arêtes de la frontière.
3. Supprimer les triangles connectés au super-triangle.

---

###  2. Algorithme de Prim – Minimum Spanning Tree (MST)


Étapes principales :
1. Sélection d’un point de départ aléatoire.
2. Ajout de l’arête de poids minimal connectant un point visité à un point non visité.
3. Répétition jusqu’à ce que tous les points soient connectés.

---

###  3. Génération des couloirs

Les couloirs sont générés à partir du MST :
- S’ils sont **alignés** (horizontalement ou verticalement) → couloir **rectiligne**.
- Sinon → couloir en **forme de L** avec alternance horizontale/verticale.

Chaque couloir est instancié comme un `ARoom` secondaire, à l’aide de la classe `CorridorToSpawn`.

---

##  Architecture du code

### **`AGenerator`**
Classe principale dérivée d’`AActor` responsable de toute la génération.

Fonctions principales :
- `MakeDungeon()` : exécution complète de la génération.
- `SpawnRoomsInRadius()` : placement aléatoire des salles.
- `SeparateRooms()` : élimine les chevauchements.
- `Triangulation()` : construction du graphe via Delaunay.
- `PrimAlgorithm()` : création du MST.
- `BuildCorridorsFromMST_Meshes()` : placement des couloirs.
- `FillAxisGapBetween()` : remplissage des espaces vides avec des modules (“fillers”).

Paramètres ajustables dans l’éditeur :
- Nombre de salles (`roomNumber`)
- Largeur et hauteur des couloirs
- Rayon de spawn initial
- Matériaux (`MajorMat`, `SecondaryMat`)

---

### **`ARoom`**
Représente une salle ou un couloir dans le monde.

- Contient un `UStaticMeshComponent` (souvent un plan).
- Possède un matériau dynamique (`UMaterialInstanceDynamic`).
- Peut être marquée comme **majeure** (`isMajor = true`), ce qui influence la triangulation et le MST.

Fonction :
```cpp
void SetColor(UMaterialInterface* _mat);

