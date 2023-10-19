// Copyright Epic Games, Inc. All Rights Reserved.

#include "LearningMultiplayerGameMode.h"
#include "LearningMultiplayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALearningMultiplayerGameMode::ALearningMultiplayerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
