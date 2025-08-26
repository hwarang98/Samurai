// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset/StartUpData/DataAsset_StartUpDataBase.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/Abilites/WarriorGameplayAbility.h"

/**
 * 어빌리티를 지정된 워리어 어빌리티 시스템 컴포넌트에 부여합니다.
 * 
 * 이 함수는 UGameplayAbility를 기반으로 한 능력을 InWarriorASCToGive에 추가하며,
 * 추가된 어빌리티는 지정된 레벨(ApplyLevel)을 기준으로 적용됩니다.
 * 
 * @param InWarriorASCToGive 어빌리티를 부여할 대상 AbilitySystemComponent. nullptr이면 아무 작업도 하지 않습니다.
 * @param ApplyLevel 부여할 어빌리티의 레벨. 기본값은 1입니다.
 */
void UDataAsset_StartUpDataBase::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASCToGive,
	int32 ApplyLevel)
{
	check(InASCToGive);
	
	GrantAbilities(ActivateOnGivenAbilities, InASCToGive, ApplyLevel);
	GrantAbilities(ReactiveAbilities, InASCToGive, ApplyLevel);

	if (!StartUpGameplayEffects.IsEmpty())
	{
		for (const TSubclassOf<UGameplayEffect>& EffectsClass : StartUpGameplayEffects)
		{
			if (!EffectsClass) continue;
			UGameplayEffect* EffectCDO = EffectsClass->GetDefaultObject<UGameplayEffect>();
			InASCToGive->ApplyGameplayEffectToSelf(EffectCDO, ApplyLevel, InASCToGive->MakeEffectContext());
			
		}
	}
}

/**
 * 지정된 어빌리티 배열을 대상 AbilitySystemComponent에 부여합니다.
 * 각 어빌리티는 TSubclassOf<UWarriorGameplayAbility> 형식으로 전달되며,
 * 주어진 ApplyLevel을 기준으로 WarriorAbilitySystemComponent에 등록됩니다.
 * @param InAbilitiesToGive 부여할 어빌리티 클래스 배열. 각 요소는 UWarriorGameplayAbility의 서브클래스여야 합니다.
 * @param InASCToGive 어빌리티를 부여할 대상 AbilitySystemComponent. nullptr이면 아무 작업도 하지 않습니다.
 * @param ApplyLevel 부여할 어빌리티의 레벨. 기본값은 1입니다.
 */
void UDataAsset_StartUpDataBase::GrantAbilities(const TArray<TSubclassOf<UWarriorGameplayAbility>>& InAbilitiesToGive,
	UWarriorAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	if (InAbilitiesToGive.IsEmpty())
	{
		return;
	}
	for (const TSubclassOf<UWarriorGameplayAbility>& Ability : InAbilitiesToGive)
	{
		if (!Ability)
		{
			continue;
		}

		FGameplayAbilitySpec AbilitySpec(Ability);
		AbilitySpec.SourceObject = InASCToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		
		InASCToGive->GiveAbility(AbilitySpec);
	}
}
