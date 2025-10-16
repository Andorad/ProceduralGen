// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Room.generated.h"

UCLASS()
class PROCEDURALGEN_API ARoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoom();

public:
	void SetColor(UMaterialInterface* _mat);
	

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PlaneMesh;

	UMaterialInstanceDynamic* DynamicMaterial;

	bool isMajor = false;

};
