// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilites/WarriorGameplayAbility.h"
#include "WarriorHeroGameplayAbility.generated.h"

class UHeroUIComponent;
class AWarriorHeroCharacter;
class AWarriorHeroController;
class UHeroCombatComponent;
/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorHeroGameplayAbility : public UWarriorGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	AWarriorHeroCharacter* GetHeroCharacterFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	AWarriorHeroController* GetHeroControllerFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	UHeroCombatComponent* GetHeroCombatComponentFromActorInfo() const;
	
	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	UHeroUIComponent* GetHeroUIComponentFromActorInfo();

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	FGameplayEffectSpecHandle MakeHeroDamageEffectSpecHandle(
		TSubclassOf<UGameplayEffect> EffectClass,
		float InWeaponBaseDamage,
		FGameplayTag InCurrentAttackTypeTag,
		int32 InUsedComboCount
	);
	
	UFUNCTION(BlueprintCallable, Category = "Warrior|Ability")
	bool GetAbilityRemainingCooldownByTag(FGameplayTag InCooldownTag, float& TotalCooldownTime, float& RemainingCooldownTime);

private:
	mutable TWeakObjectPtr<AWarriorHeroCharacter> CachedWarriorHeroCharacter;
	mutable TWeakObjectPtr<AWarriorHeroController> CachedWarriorHeroController;
};
