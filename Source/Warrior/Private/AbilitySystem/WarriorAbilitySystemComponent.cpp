// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WarriorAbilitySystemComponent.h"

#include "WarriorDebugHelper.h"
#include "WarriorGamePlayTags.h"
#include "AbilitySystem/Abilites/WarriorHeroGameplayAbility.h"


void UWarriorAbilitySystemComponent::OnAbilityInputPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			continue;
		}

		if (InputTag.MatchesTag(WarriorGamePlayTags::InputTag_Toggleable) && AbilitySpec.IsActive())
		{
			CancelAbilityHandle(AbilitySpec.Handle);
		}
		else
		{
			TryActivateAbility(AbilitySpec.Handle);
		}
	}
}

void UWarriorAbilitySystemComponent::OnAbilityInputReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid() || !InputTag.MatchesTag(WarriorGamePlayTags::InputTag_MustBeHeld))
	{
		return;
	}

	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			CancelAbilityHandle(AbilitySpec.Handle);
		}
	}
}

void UWarriorAbilitySystemComponent::GrantHeroWeaponAbilities(
	const TArray<FWarriorHeroAbilitySet>& InDefaultWeaponAbilities,
	const TArray<FWarriorHeroSpecialAbilitySet>& InSpecialWeaponAbilities,
	int32 ApplyLevel,
	TArray<FGameplayAbilitySpecHandle>& OutGrantedAbilitySpecHandles)
{
	if (InDefaultWeaponAbilities.IsEmpty())
	{
		return;
	}
	
	for (const FWarriorHeroAbilitySet& AbilitySet : InDefaultWeaponAbilities)
	{
		if (!AbilitySet.IsValid())
		{
			continue;
		}
		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
		AbilitySpec.SourceObject = GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilitySet.InputTag);

		OutGrantedAbilitySpecHandles.AddUnique(GiveAbility(AbilitySpec));
	}

	for (const FWarriorHeroSpecialAbilitySet& AbilitySet : InSpecialWeaponAbilities)
	{
		if (!AbilitySet.IsValid())
		{
			continue;
		}
		FGameplayAbilitySpec AbilitySpec(AbilitySet.AbilityToGrant);
		AbilitySpec.SourceObject = GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(AbilitySet.InputTag);

		OutGrantedAbilitySpecHandles.AddUnique(GiveAbility(AbilitySpec));
	}
}

void UWarriorAbilitySystemComponent::RemoveGrantHeroWeaponAbilities(UPARAM(ref)
	TArray<FGameplayAbilitySpecHandle>& InSpecHandlesRemove)
{
	if (InSpecHandlesRemove.IsEmpty())
	{
		return;
	}

	for (const FGameplayAbilitySpecHandle& SpecHandle : InSpecHandlesRemove)
	{
		if (SpecHandle.IsValid())
		{
			ClearAbility(SpecHandle);
		}
	}

	// 혹시모를 안전장치
	InSpecHandlesRemove.Empty();
}

bool UWarriorAbilitySystemComponent::TryActivateAbilityByTag(FGameplayTag AbilityTagToActivate)
{
	check(AbilityTagToActivate.IsValid());

	// 해당 태그와 일치하는 활성화 가능한 어빌리티들을 담을 배열
	TArray<FGameplayAbilitySpec*> FoundAbilitiesSpecs;

	// AbilityTagToActivate와 일치하는 어빌리티들을 검색하여 FoundAbilitiesSpecs에 채운다
	GetActivatableGameplayAbilitySpecsByAllMatchingTags(
		AbilityTagToActivate.GetSingleTagContainer(), // 단일 태그 컨테이너로 변환
		FoundAbilitiesSpecs
	);

	// 조건: 태그에 해당하는 어빌리티가 하나라도 있다면
	if (!FoundAbilitiesSpecs.IsEmpty())
	{
		// 어빌리티가 여러 개일 수 있으므로, 무작위로 하나 선택
		const int32 RandomAbilityIndex = FMath::RandRange(0, FoundAbilitiesSpecs.Num() - 1);
		const FGameplayAbilitySpec* SpecToActivate = FoundAbilitiesSpecs[RandomAbilityIndex];

		// 무작위로 선택된 어빌리티가 null이 아닌지 확인
		check(SpecToActivate);

		// 이미 활성화된 어빌리티가 아니라면
		if (!SpecToActivate->IsActive())
		{
			// 해당 어빌리티를 활성화 시도하고 결과 반환
			return TryActivateAbility(SpecToActivate->Handle);
		}
	}

	// 어빌리티를 찾지 못했거나 이미 활성화된 경우 false 반환
	return false;
}
