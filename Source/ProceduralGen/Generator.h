// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Room.h>
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
};
