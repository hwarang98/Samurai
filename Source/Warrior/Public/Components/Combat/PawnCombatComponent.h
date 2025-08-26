// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/PawnExtensionComponentBase.h"
#include "PawnCombatComponent.generated.h"

// class FGameplayTag;
class AWarriorWeaponBase;

UENUM(BlueprintType)
enum class EToggleDamageType : uint8
{
	EED_CurrentEquippedWeapon,
	EED_LeftHand,
	EED_RightHand
};
/**
 * 
 */
UCLASS()
class WARRIOR_API UPawnCombatComponent : public UPawnExtensionComponentBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void RegisterSpawnedWeapon(FGameplayTag InWeaponTagToResister, AWarriorWeaponBase* InWeaponToResister, bool bResisterAsEquippedWeapon = false);

	/**
	 * 특정 무기를 검색할 수 있다.
	 * @param InWeaponTagToGet FGameplayTag
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	AWarriorWeaponBase* GetCharacterCarriedWeaponByTag(FGameplayTag InWeaponTagToGet) const;

	/**
	 * 장착한 무기를 추적하는데 사용하는 태그
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Warrior|Combat")
	FGameplayTag CurrentEquippedWeaponTag;
 
	/**
	 * 현재 장착된 무기 주소값
	 */
	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	AWarriorWeaponBase* GetCharacterCurrentEquippedWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "Warrior|Combat")
	void ToggleWeaponCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType = EToggleDamageType::EED_CurrentEquippedWeapon);

	virtual void OnHitTargetActor(AActor* HitActor);
	virtual void OnWeaponPulledFromTargetActor(AActor* InteractingActor);

protected:
	TArray<AActor*> OverlappedActors;

private:
	TMap<FGameplayTag, AWarriorWeaponBase*> CharacterCarriedWeaponMap;
};
