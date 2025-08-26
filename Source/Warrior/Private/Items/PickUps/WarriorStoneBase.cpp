// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/PickUps/WarriorStoneBase.h"
#include "WarriorGamePlayTags.h"
#include "Characters/WarriorHeroCharacter.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"

void AWarriorStoneBase::Consume(UWarriorAbilitySystemComponent* AbilitySystemComponent, int32 ApplyLevel)
{
	check(StoneGameplayEffectClass);

	UGameplayEffect* EffectCDO = StoneGameplayEffectClass->GetDefaultObject<UGameplayEffect>();
	
	AbilitySystemComponent->ApplyGameplayEffectToSelf(
		EffectCDO,
		ApplyLevel,
		AbilitySystemComponent->MakeEffectContext()
	);

	BP_OnStoneConsumed();
}

void AWarriorStoneBase::OnPickUpCollisionSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (const AWarriorHeroCharacter* OverlappedHeroCharacter = Cast<AWarriorHeroCharacter>(OtherActor))
	{
		OverlappedHeroCharacter->GetWarriorAbilitySystemComponent()->TryActivateAbilityByTag(WarriorGamePlayTags::Player_Ability_PickUp_Stones);
	}
}
