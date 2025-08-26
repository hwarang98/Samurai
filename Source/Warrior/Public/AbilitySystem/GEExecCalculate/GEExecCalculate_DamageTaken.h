// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GEExecCalculate_DamageTaken.generated.h"

/**
 * 
 */
UCLASS()
class WARRIOR_API UGEExecCalculate_DamageTaken : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UGEExecCalculate_DamageTaken();

	// UFUNCTION(BlueprintNativeEvent, Category="Calculation")
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
