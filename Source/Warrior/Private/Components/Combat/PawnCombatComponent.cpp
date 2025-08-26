// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PawnCombatComponent.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include "Components/BoxComponent.h"

#include "WarriorDebugHelper.h"

void UPawnCombatComponent::RegisterSpawnedWeapon(FGameplayTag InWeaponTagToResister, AWarriorWeaponBase* InWeaponToResister, bool bResisterAsEquippedWeapon)
{
	// 1. 이미 동일한 태그로 등록된 무기가 있는지 확인 (있으면 에러 발생)
	checkf(!CharacterCarriedWeaponMap.Contains(InWeaponTagToResister), TEXT("%s는(은) 이미 장착된 무기 목록에 포함되어 있습니다"),
	       *InWeaponTagToResister.ToString());
	// 2. 전달받은 무기 객체가 유효한지 확인 (nullptr이면 에러 발생)
	check(InWeaponToResister);

	// 3. 무기 태그와 무기 객체를 맵에 등록 (무기 보유 목록에 추가)
	CharacterCarriedWeaponMap.Emplace(InWeaponTagToResister, InWeaponToResister);

	// ~델리게이트 구독 (이 무기로 누군가를 맞추면 OnHitTargetActor 함수가 호출됨)
	InWeaponToResister->OnWeaponHitTarget.BindUObject(this, &ThisClass::OnHitTargetActor);
	InWeaponToResister->OnWeaponPulledFromTarget.BindUObject(this, &ThisClass::OnWeaponPulledFromTargetActor);
	// ~델리게이트 구독

	if (bResisterAsEquippedWeapon)
	{
		// 4. 현재 장착 무기 태그를 이번에 등록한 무기 태그로 갱신
		CurrentEquippedWeaponTag = InWeaponTagToResister;
	}
}

/**
 * 이 함수는 CharacterCarriedWeaponMap (예: TMap<FGameplayTag, AWarriorWeaponBase*>)에서
 * 특정 GameplayTag로 등록된 무기를 찾아 반환하는 헬퍼 함수입니다.
 * @param InWeaponTagToGet 
 * @return 
 */
AWarriorWeaponBase* UPawnCombatComponent::GetCharacterCarriedWeaponByTag(FGameplayTag InWeaponTagToGet) const
{
	// // 1. 입력한 태그에 해당하는 무기가 무기 맵에 존재하는지 확인
	if (CharacterCarriedWeaponMap.Contains(InWeaponTagToGet))
	{
		// 2. 해당 태그에 대응되는 무기 포인터를 맵에서 찾아봄
		if (AWarriorWeaponBase* const* FoundWeapon = CharacterCarriedWeaponMap.Find(InWeaponTagToGet))
		{
			// 3. 찾은 무기 포인터 반환 (더블 포인터 → 역참조)
			{
				return *FoundWeapon;
			}
		}
	}

	return nullptr;
}

AWarriorWeaponBase* UPawnCombatComponent::GetCharacterCurrentEquippedWeapon() const
{
	// 1. 현재 장착된 무기 태그가 유효하지 않다면 (즉, 설정되지 않았다면)
	if (!CurrentEquippedWeaponTag.IsValid())
	{
		return nullptr;
	}
	// 3. 유효한 태그가 있을 경우, 해당 태그로 등록된 무기를 찾아 반환
	return GetCharacterCarriedWeaponByTag(CurrentEquippedWeaponTag);
}

void UPawnCombatComponent::ToggleWeaponCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType)
{
	if (ToggleDamageType == EToggleDamageType::EED_CurrentEquippedWeapon)
	{
		AWarriorWeaponBase* WeaponToToggle = GetCharacterCurrentEquippedWeapon();

		check(WeaponToToggle);

		WeaponToToggle->GetWeaponCollisionBox()->SetCollisionEnabled(
			bShouldEnable ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);

		if (!bShouldEnable)
		{
			OverlappedActors.Empty();
		}
	}


}

void UPawnCombatComponent::OnHitTargetActor(AActor* HitActor)
{
}

void UPawnCombatComponent::OnWeaponPulledFromTargetActor(AActor* InteractingActor)
{
}
