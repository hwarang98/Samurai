// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/WarriorBaseGameMode.h"
#include "WarriorSurvivalGameMode.generated.h"

UENUM(BlueprintType)
enum class EWarriorServivalGameModeState : uint8
{
	WaitSpawnNewWave,
	SpawningNewWave,
	InProgress,
	WaveCompleted,
	AllWavesDone,
	PlayerDied
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnServivalGameModeStateChanged, EWarriorServivalGameModeState, CurrentState);

/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorSurvivalGameMode : public AWarriorBaseGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	void SetCurrentServivalGameModeState(EWarriorServivalGameModeState InState);
	
	UPROPERTY()
	EWarriorServivalGameModeState CurrentServivalGameModeState;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnServivalGameModeStateChanged OnServivalGameModeStateChanged;
};
