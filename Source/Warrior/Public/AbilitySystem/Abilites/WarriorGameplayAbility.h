// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "WarriorTypes/WarriorEnumTypes.h"
#include "WarriorGameplayAbility.generated.h"

// class EWarriorSuccessType;
class UPawnCombatComponent;
class UWarriorAbilitySystemComponent;

UENUM(BlueprintType)
enum class EWarriorAbilityActivationPolicy : uint8
{
	/* 입력 또는 게임플레이 이벤트를통해 트리거 될때 */
	EWA_OnTriggered,

	/* 대상에게 주어지면 바로 능력 발동 */
	EWA_OnGiven
};

/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

protected:
	// ~ Begin UGameplayAbility Interface.
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
		bool bWasCancelled
	) override;
	// ~ End UGameplayAbility Interface.

	UPROPERTY(EditAnywhere, Category = "WarriorAbility")
	EWarriorAbilityActivationPolicy AbilityActivationPolicy = EWarriorAbilityActivationPolicy::EWA_OnTriggered;

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	UPawnCombatComponent* GetPawnCombatComponentFromActorInfo() const;

	UFUNCTION(BlueprintPure, Category = "Warrior|Ability")
	UWarriorAbilitySystemComponent* GetWarriorAbilitySystemComponentFromActorInfo() const;

	UFUNCTION(
		BlueprintCallable,
		Category = "Warrior|Ability",
		meta = (DisplayName = "Apply Gameplay Effect Spec Handle To Target Actor", ExpandEnumAsExecs = "OutSuccessType")
	)
	FActiveGameplayEffectHandle BP_ApplyEffectSpecHandleToTarget(
		AActor* TargetActor,
		const FGameplayEffectSpecHandle& InSpecHandle,
		EWarriorSuccessType& OutSuccessType
	);

	FActiveGameplayEffectHandle NativeApplyEffectSpecHandleToTarget(
		AActor* TargetActor,
		const FGameplayEffectSpecHandle& InSpecHandle
	);
};
