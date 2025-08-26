// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/WarriorSurvivalGameMode.h"

#include "WarriorFunctionLibrary.h"

void AWarriorSurvivalGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AWarriorSurvivalGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWarriorSurvivalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	EWarriorGameDifficulty SavedGameDifficulty;

	if (UWarriorFunctionLibrary::TryLoadSavedGameDifficulty(SavedGameDifficulty))
	{
		CurrentGameDifficulty = SavedGameDifficulty;
	}
}

void AWarriorSurvivalGameMode::SetCurrentServivalGameModeState(const EWarriorServivalGameModeState InState)
{
	CurrentServivalGameModeState = InState;
	OnServivalGameModeStateChanged.Broadcast(CurrentServivalGameModeState);
}
