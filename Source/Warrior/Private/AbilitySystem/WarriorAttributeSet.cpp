// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/WarriorAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGamePlayTags.h"
#include "Interfaces/PawnUIInterface.h"
#include "Components/UI/PawnUIComponent.h"
#include "Components/UI/HeroUIComponent.h"

#include "WarriorDebugHelper.h"

UWarriorAttributeSet::UWarriorAttributeSet()
{
	InitCurrentHealth(1.f);
	InitMaxHealth(1.f);
	InitCurrentRage(1.f);
	InitMaxRage(1.f);
	InitAttackPower(1.f);
	InitDefensePower(1.f);
}

void UWarriorAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	if (!CachedPawnUIInterface.IsValid())
	{
		CachedPawnUIInterface = TWeakInterfacePtr<IPawnUIInterface>(Data.Target.GetAvatarActor());
	}

	checkf(
		CachedPawnUIInterface.IsValid(),
		TEXT("%s IPawnUIInterface를 구현하지 않았습니다. "),
		*Data.Target.GetAvatarActor()->GetActorNameOrLabel()
	);

	UPawnUIComponent* PawnUIComponent = CachedPawnUIInterface->GetPawnUIComponent();

	checkf(
		PawnUIComponent,
		TEXT("PawnUIComponent를 %s 로 부터 추출 할 수 없습니다."),
		*Data.Target.GetAvatarActor()->GetActorNameOrLabel()
	);

	// 현재체력 상태 변경
	if (Data.EvaluatedData.Attribute == GetCurrentHealthAttribute())
	{
		const float NewCurrentHealth = FMath::Clamp(GetCurrentHealth(), 0.f, GetMaxHealth());

		SetCurrentHealth(NewCurrentHealth);

		PawnUIComponent->OnCurrentHealthChanged.Broadcast(GetCurrentHealth() / GetMaxHealth());
	}

	// 분노 상태 변경
	if (Data.EvaluatedData.Attribute == GetCurrentRageAttribute())
	{
		const float NewCurrentRage = FMath::Clamp(GetCurrentRage(), 0.f, GetMaxRage());
		SetCurrentRage(NewCurrentRage);

		if (GetCurrentRage() == GetMaxRage())
		{
			UWarriorFunctionLibrary::AddGameplayTagToActorIfNone(Data.Target.GetAvatarActor(), WarriorGamePlayTags::Player_Status_Rage_Full);
		}
		else if (GetCurrentRage() == 0.f)
		{
			UWarriorFunctionLibrary::AddGameplayTagToActorIfNone(Data.Target.GetAvatarActor(), WarriorGamePlayTags::Player_Status_Rage_None);
		}
		else
		{
			UWarriorFunctionLibrary::RemoveGameplayTagFromActorIfFound(Data.Target.GetAvatarActor(), WarriorGamePlayTags::Player_Status_Rage_Full);
			UWarriorFunctionLibrary::RemoveGameplayTagFromActorIfFound(Data.Target.GetAvatarActor(), WarriorGamePlayTags::Player_Status_Rage_None);
		}

		if (UHeroUIComponent* HeroUIComponent = CachedPawnUIInterface->GetHeroUIComponent())
		{
			HeroUIComponent->OnCurrentRageChanged.Broadcast(GetCurrentRage() / GetMaxRage());
		}
	}

	// 데미지 받았을때 상태 변경
	if (Data.EvaluatedData.Attribute == GetDamageTakenAttribute())
	{
		const float PreviousHealth = GetCurrentHealth();
		const float DamageDone = GetDamageTaken();

		const float NewCurrentHealth = FMath::Clamp(PreviousHealth - DamageDone, 0.f, GetMaxHealth());
		SetCurrentHealth(NewCurrentHealth);

		// const FString DebugString = FString::Printf(
		// 	TEXT("이전체력: %f, 데미지 완료: %f"),
		// 	PreviousHealth,
		// 	DamageDone
		// );
		//
		// Debug::Print(DebugString, FColor::Green);

		PawnUIComponent->OnCurrentHealthChanged.Broadcast(GetCurrentHealth() / GetMaxHealth());

		// 피니셔 가능 상태인지 확인
		if (GetCurrentHealth() > 0.f && (GetCurrentHealth() / GetMaxHealth() <= 0.30f)) // 체력이 20% 이하일 때
		{
			UWarriorFunctionLibrary::AddGameplayTagToActorIfNone(
				Data.Target.GetAvatarActor(),
				WarriorGamePlayTags::Enemy_Status_Finishable
			);
		}

		if (GetCurrentHealth() <= 0.f)
		{
			UWarriorFunctionLibrary::AddGameplayTagToActorIfNone(
				Data.Target.GetAvatarActor(),
				WarriorGamePlayTags::Shared_Status_Dead
			);

			// 죽었을 때는 피니셔 태그 제거
			UWarriorFunctionLibrary::RemoveGameplayTagFromActorIfFound(
				Data.Target.GetAvatarActor(),
				WarriorGamePlayTags::Enemy_Status_Finishable
			);
		}
	}
}
