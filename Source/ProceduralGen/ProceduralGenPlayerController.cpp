// Copyright Epic Games, Inc. All Rights Reserved.


#include "ProceduralGenPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "ProceduralGenCameraManager.h"

AProceduralGenPlayerController::AProceduralGenPlayerController()
{
	// set the player camera manager class
	PlayerCameraManagerClass = AProceduralGenCameraManager::StaticClass();
}

void AProceduralGenPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Context
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
}
