// Copyright Epic Games, Inc. All Rights Reserved.

#include "CPPStudyGameMode.h"
#include "CPPStudyCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACPPStudyGameMode::ACPPStudyGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
