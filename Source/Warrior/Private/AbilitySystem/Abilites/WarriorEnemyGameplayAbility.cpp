// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilites/WarriorEnemyGameplayAbility.h"

#include "WarriorGamePlayTags.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Characters/WarriorEnemyCharacter.h"

AWarriorEnemyCharacter* UWarriorEnemyGameplayAbility::GetEnemyCharacterFromActorInfo()
{
	if (!CachedWarriorEnemyCharacter.IsValid())
	{
		CachedWarriorEnemyCharacter = Cast<AWarriorEnemyCharacter>(CurrentActorInfo->AvatarActor);
	}

	return CachedWarriorEnemyCharacter.IsValid() ? CachedWarriorEnemyCharacter.Get() : nullptr;
}

UEnemyCombatComponent* UWarriorEnemyGameplayAbility::GetEnemyCombatComponentFromActorInfo()
{
	return GetEnemyCharacterFromActorInfo()->GetEnemyCombatComponent();
}

FGameplayEffectSpecHandle UWarriorEnemyGameplayAbility::MakeEnemyDamageEffectSpecHandle(
	TSubclassOf<UGameplayEffect> EffectClass, const FScalableFloat& InDamageScalableFloat)
{
	check(EffectClass);

	// 이펙트에 사용될 컨텍스트 핸들 생성 (누가 시전했는지, 어떤 스킬인지 등 정보를 담음)
	FGameplayEffectContextHandle ContextHandle = GetWarriorAbilitySystemComponentFromActorInfo()->MakeEffectContext();
	// Ability 정보 추가 (누가 발동했는지)
	ContextHandle.SetAbility(this);
	// SourceObject 설정: 일반적으로 무기, 액터 등 이펙트의 "출처" 객체
	ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
	// Instigator 설정: 이펙트를 유발한 주체 (보통 캐릭터 자신)
	ContextHandle.AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());

	// !!실제 GameplayEffect 사양을 생성함 실제 적용은 안되는단계 (어떤 효과를 어떤 레벨로 누구에게 적용할지)
	FGameplayEffectSpecHandle EffectSpecHandle = GetWarriorAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
		EffectClass, // 사용할 이펙트 클래스
		GetAbilityLevel(), // 현재 어빌리티 레벨
		ContextHandle // 위에서 만든 컨텍스트 정보
	);

	EffectSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGamePlayTags::Shared_SetByCaller_BaseDamage,
		InDamageScalableFloat.GetValueAtLevel(GetAbilityLevel())
	);

	return EffectSpecHandle;
}
