// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/EnemyCombatComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WarriorGamePlayTags.h"
#include "WarriorFunctionLibrary.h"

#include "WarriorDebugHelper.h"

void UEnemyCombatComponent::OnHitTargetActor(AActor* HitActor)
{
	// 이미 겹친 액터(같은 프레임에서 중복 피격 방지)에 포함되어 있다면 리턴
	if (OverlappedActors.Contains(HitActor))
	{
		return;
	}

	OverlappedActors.AddUnique(HitActor);

	// 적 캐릭터가 플레이어를 공격할때 공격 차단 하지만 모든 공격을 막기는 x
	bool bIsValidBlock = false;

	const bool bIsPlayerBlocking = UWarriorFunctionLibrary::NativeDoesActorHaveTag(HitActor,WarriorGamePlayTags::Player_Status_Blocking);
	const bool bIsMyAttackUnblockable = UWarriorFunctionLibrary::NativeDoesActorHaveTag(GetOwningPawn(), WarriorGamePlayTags::Enemy_Status_Unblockable);

	// 플레이어가 방어 중이고 && 내 공격이 막을 수 있는 공격일 경우
	if (bIsPlayerBlocking && !bIsMyAttackUnblockable)
	{
		bIsValidBlock = UWarriorFunctionLibrary::IsValidBlock(GetOwningPawn(), HitActor);
	}

	FGameplayEventData EventData;
	EventData.Instigator = GetOwningPawn(); // 공격자 설정
	EventData.Target = HitActor; // 피격 대상 설정

	if (bIsValidBlock)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			HitActor,
			WarriorGamePlayTags::Player_Event_SuccessfulBlock,
			EventData
		);
	}
	else
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			GetOwningPawn(),
			WarriorGamePlayTags::Shared_Event_MeleeHit,
			EventData
		);
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			GetOwningPawn(),
			WarriorGamePlayTags::Shared_Event_SpawnProjectile,
			EventData
		);
	}
}

void UEnemyCombatComponent::TriggerGameplayEvent(const FGameplayTag EventTag, const FGameplayEventData& EventData)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(),
		EventTag,
		EventData
	);
}
