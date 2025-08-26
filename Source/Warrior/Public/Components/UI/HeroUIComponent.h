// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/UI/PawnUIComponent.h"
#include "HeroUIComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnEquippedWeaponChangedDelegate,
	TSoftObjectPtr<UTexture2D>,
	SoftWeaponIcon
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnAbilityIconUpdatedDelegate,
	FGameplayTag, AbilityInputTag,
	TSoftObjectPtr<UMaterialInterface>, SoftAbilityIconMaterial
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnAbilityCooldownBeginDelegate,
	FGameplayTag, AbilityInputTag,
	float, totalCooldownTime, // 총 재사용 시간
	float, RemainingCooldownTime // 남은 재사용 시간
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnStoneInteractedDelegate,
	bool, bShouldDisplayInputKey
);
/**
 * 
 */
UCLASS()
class WARRIOR_API UHeroUIComponent : public UPawnUIComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnPercentChangedDelegate OnCurrentRageChanged;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnEquippedWeaponChangedDelegate OnEquippedWeaponChanged;

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnAbilityIconUpdatedDelegate OnAbilityIconUpdated;
	
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnAbilityCooldownBeginDelegate OnAbilityCooldownBegin;
	
	UPROPERTY(BlueprintCallable, BlueprintAssignable)
	FOnStoneInteractedDelegate OnStoneInteracted;
};
