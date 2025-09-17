// Fill out your copyright notice in the Description page of Project Settings.

#include "DataAsset/StartUpData/DataAsset_EnemyStartUpData.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/Abilites/WarriorEnemyGameplayAbility.h"

void UDataAsset_EnemyStartUpData::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	Super::GiveToAbilitySystemComponent(InASCToGive, ApplyLevel);

	if (!EnemyGameplayAbility.IsEmpty())
	{
		for(const TSubclassOf<UWarriorEnemyGameplayAbility>& AbilityClass : EnemyGameplayAbility)
		{
			if (!AbilityClass) continue;
			FGameplayAbilitySpec AbilitySpec(AbilityClass);
			AbilitySpec.SourceObject = InASCToGive->GetAvatarActor();
			AbilitySpec.Level = ApplyLevel;
		
			InASCToGive->GiveAbility(AbilitySpec);
		}
	}
}
