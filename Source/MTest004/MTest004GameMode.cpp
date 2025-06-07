// Copyright Epic Games, Inc. All Rights Reserved.

#include "MTest004GameMode.h"
#include "MTest004Character.h"
#include "UObject/ConstructorHelpers.h"

AMTest004GameMode::AMTest004GameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
