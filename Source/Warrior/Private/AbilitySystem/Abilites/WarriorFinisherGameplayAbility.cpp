// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilites/WarriorFinisherGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AIController.h"
#include "MotionWarpingComponent.h"
#include "WarriorDebugHelper.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGamePlayTags.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/Abilites/HeroGameplayAbility_TargetLock.h" // 락온 어빌리티 클래스 포함
#include "Characters/WarriorHeroCharacter.h"
#include "Kismet/KismetSystemLibrary.h" // 트레이스 함수를 위해 포함
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

/**
 * @brief 생성자입니다. 어빌리티 태그, 활성화 방지 태그, 모션 워핑 키 이름을 설정합니다.
 */
UWarriorFinisherGameplayAbility::UWarriorFinisherGameplayAbility()
{
	FGameplayTagContainer Tags;
	Tags.AddTag(WarriorGamePlayTags::Player_Ability_Finisher);
	SetAssetTags(Tags);
	
	ActivationBlockedTags.AddTag(WarriorGamePlayTags::Player_Status_Rolling);
	ActivationBlockedTags.AddTag(WarriorGamePlayTags::Shared_Status_Dead);
	ActivationBlockedTags.AddTag(WarriorGamePlayTags::Shared_Status_HitReact_Front);
	ActivationBlockedTags.AddTag(WarriorGamePlayTags::Shared_Status_HitReact_Right);
	ActivationBlockedTags.AddTag(WarriorGamePlayTags::Shared_Status_HitReact_Left);
	ActivationBlockedTags.AddTag(WarriorGamePlayTags::Shared_Status_HitReact_Back);

	// 모션 워핑 키 이름 초기화. 이 이름은 몽타주에서 사용되는 워프 타겟의 이름과 일치해야 합니다.
	// 플레이어의 몽타주에서 "FinisherTarget"이라는 이름의 워프 타겟을 찾아 해당 위치로 워프합니다.
	PlayerMotionWarpKeyName = FName("FinisherTarget");
	// 적의 몽타주에서 "ExecutionerTarget"이라는 이름의 워프 타겟을 찾아 해당 위치로 워프합니다.
	EnemyMotionWarpKeyName = FName("ExecutionerTarget");
}

/**
 * @brief 이 어빌리티를 활성화할 수 있는지 확인합니다.
 * @param Handle 어빌리티 스펙 핸들입니다.
 * @param ActorInfo 어빌리티를 활성화하는 액터의 정보입니다.
 * @param SourceTags 소스 액터의 태그입니다.
 * @param TargetTags 타겟 액터의 태그입니다.
 * @param OptionalRelevantTags 어빌리티에 정보를 전달하기 위해 채울 수 있는 선택적 태그입니다.
 * @return 어빌리티를 활성화할 수 있으면 true, 그렇지 않으면 false를 반환합니다.
 * @details 플레이어 전방에서 유효한 피니셔 타겟을 찾고, 해당 타겟이 'Finishable' 태그를 가지고 있는지 확인합니다.
 */
bool UWarriorFinisherGameplayAbility::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags
) const
{
	// 1. 가장 기본적인 활성화 조건을 먼저 확인.
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 2. 유효한 타겟이 있는지 확인.
	const AActor* Target = GetTargetForActivation(ActorInfo);
	if (!Target)
	{
		return false;
	}

	// 3. 타겟이 '처형 가능' 상태인지 확인.
	if (!UWarriorFunctionLibrary::NativeDoesActorHaveTag(const_cast<AActor*>(Target), WarriorGamePlayTags::Enemy_Status_Finishable))
	{
		return false;
	}
    
	// 4. 플레이어와 타겟이 유효한 캐릭터인지 확인.
	const AWarriorHeroCharacter* PlayerCharacter = Cast<AWarriorHeroCharacter>(ActorInfo->AvatarActor.Get());
	const ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	
	if (!PlayerCharacter || !TargetCharacter)
	{
		return false;
	}
    
	// 5. 플레이어와 타겟에 맞는 처형 애니메이션 쌍이 있는지 확인.
	if (!SelectFinisherPair(PlayerCharacter, TargetCharacter))
	{
		return false;
	}

	// 6. 모든 조건을 통과했으므로, 최종적으로 활성화를 허가.
	return true;
}

/**
 * @brief 어빌리티의 주 로직을 실행합니다.
 * @param Handle 어빌리티 스펙 핸들입니다.
 * @param ActorInfo 어빌리티를 활성화하는 액터의 정보입니다.
 * @param ActivationInfo 어빌리티 활성화에 대한 정보입니다.
 * @param TriggerEventData 이 어빌리티를 트리거한 이벤트의 데이터입니다(있는 경우).
 * @details 타겟을 캐시하고, 플레이어와 타겟 모두에서 피니셔 몽타주를 재생하며,
 *          데미지 및 모션 워핑을 위한 이벤트 리스너를 설정합니다.
 */
void UWarriorFinisherGameplayAbility::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AWarriorHeroCharacter* Hero = GetHeroCharacterFromActorInfo();
	AActor* TargetActor = GetFinisherTarget(); // 이전에 만든 함수로 타겟 가져오기
	CachedFinisherTarget = TargetActor; // 타겟 저장
	ACharacter* TargetCharacter = Cast<ACharacter>(TargetActor);
	
	if (!Hero || !TargetActor || !DeathEffectClass || !TargetCharacter)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}

	CachedMontagePair = SelectFinisherPair(Hero, TargetCharacter);
	if (!CachedMontagePair)
	{
		CancelAbility(Handle, ActorInfo, ActivationInfo, true);
		return;
	}


	if (AAIController* AIController = Cast<AAIController>(TargetCharacter->GetController()))
	{
		if (UBlackboardComponent* BlackboardComponent = AIController->GetBlackboardComponent())
		{
			BlackboardComponent->SetValueAsBool(ActionLockedKeyName, true);
		}
	}
	

	// 2. 플레이어와 타겟을 무적 상태로 만들기
	UWarriorFunctionLibrary::AddGameplayTagToActorIfNone(Hero, WarriorGamePlayTags::Shared_Status_Invincible);

	// 3. 모션 워핑으로 위치/회전 보정 설정
	if (UMotionWarpingComponent* PlayerMotionWarping = Hero->GetMotionWarpingComponent())
	{
		// 몽타주에 설정된 "FinisherTarget"이라는 이름의 워프 포인트에 타겟의 위치와 회전 정보를 등록합니다.
		PlayerMotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
			PlayerMotionWarpKeyName, 
			TargetActor->GetActorLocation(), 
			TargetActor->GetActorRotation()
		);
	}

	if (UMotionWarpingComponent* EnemyMotionWarping = TargetCharacter->FindComponentByClass<UMotionWarpingComponent>())
	{
		EnemyMotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(
			EnemyMotionWarpKeyName, // 적 몽타주에 설정한 워프 타겟 이름
			Hero->GetActorLocation(),
			Hero->GetActorRotation()
		);
	}
    
	// 4. 플레이어와 적의 피니셔 몽타주 재생
	Hero->PlayAnimMontage(CachedMontagePair->PlayerFinisherMontage);
	TargetCharacter->PlayAnimMontage(CachedMontagePair->EnemyFinisherMontage);

	// 5. 데미지 타이밍을 위한 이벤트 대기 (AbilityTask 사용)
	UAbilityTask_WaitGameplayEvent* WaitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		DamageEventTag, // 이 태그의 이벤트가 발생하기를 기다림
		nullptr, 
		true
	);
	

	UAbilityTask_WaitGameplayEvent* WaitMovementReadyTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		WarriorGamePlayTags::Player_Event_MovementReady, // 몽타주에서 보낸 태그
		nullptr,
		true
	);

	// 이벤트가 수신되면(Received) 실행될 로직을 델리게이트에 바인딩
	WaitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnDamageEventReceived);
	WaitEventTask->ReadyForActivation(); // 태스크 활성화
	WaitMovementReadyTask->EventReceived.AddDynamic(this, &ThisClass::OnMovementReady);
	WaitMovementReadyTask->ReadyForActivation();
}

/**
 * @brief 어빌리티가 종료된 후 정리 작업을 수행합니다.
 * @param Handle 어빌리티 스펙 핸들입니다.
 * @param ActorInfo 어빌리티를 활성화했던 액터의 정보입니다.
 * @param ActivationInfo 어빌리티 활성화에 대한 정보입니다.
 * @param bReplicateEndAbility 어빌리티 종료를 복제할지 여부입니다.
 * @param bWasCancelled 어빌리티가 취소되었는지 여부입니다.
 * @details 플레이어의 무적 상태를 해제하고 캐시된 타겟을 정리합니다.
 */
void UWarriorFinisherGameplayAbility::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled
)
{
	if (AWarriorHeroCharacter* Hero = GetHeroCharacterFromActorInfo())
	{
		// 1. 플레이어의 무적 태그를 제거
		UWarriorFunctionLibrary::RemoveGameplayTagFromActorIfFound(Hero, WarriorGamePlayTags::Shared_Status_Invincible);
	}

	// 캐쉬된 적 제거
	CachedFinisherTarget = nullptr;
	CachedMontagePair = nullptr;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

}

/**
 * @brief 현재 락온된 타겟을 찾습니다.
 * @param LockOnActor [out] 성공적으로 찾은 경우, 락온된 액터입니다.
 * @return 타겟을 찾았으면 true, 그렇지 않으면 false를 반환합니다.
 * @details 활성화된 어빌리티 중 'UHeroGameplayAbility_TargetLock'을 찾아 락온된 타겟을 가져옵니다.
 */
bool UWarriorFinisherGameplayAbility::FindLockOnTarget(AActor*& LockOnActor) const
{
	UWarriorAbilitySystemComponent* ASC = GetWarriorAbilitySystemComponentFromActorInfo();
	if (ASC)
	{
		TArray<FGameplayAbilitySpec> ActivatableAbilities = ASC->GetActivatableAbilities();
		for (const FGameplayAbilitySpec& Spec : ActivatableAbilities)
		{
			// 활성화된 어빌리티 중 TargetLock 어빌리티를 찾음
			if (Spec.IsActive() && Spec.Ability->IsA<UHeroGameplayAbility_TargetLock>())
			{
				UHeroGameplayAbility_TargetLock* TargetLockAbility = Cast<UHeroGameplayAbility_TargetLock>(Spec.Ability);
				if (TargetLockAbility && IsValid(TargetLockAbility->GetLockedTarget()))
				{
					// 락온 중인 타겟이 유효하면 즉시 반환.
					LockOnActor = TargetLockAbility->GetLockedTarget();
					return true;
				}
			}
		}
	}
	return false;
}

/**
 * @brief 피니셔를 적용할 타겟을 찾습니다.
 * 1. 락온 중인 타겟을 우선적으로 반환.
 * 2. 락온 타겟이 없다면, 플레이어 전방을 탐지하여 가장 가까운 적을 반환.
 * @return 피니셔를 적용할 타겟 액터. 찾지 못하면 nullptr를 반환합니다.
 */
AActor* UWarriorFinisherGameplayAbility::GetFinisherTarget () const
{
	// 1. 락온(Lock-On) 타겟 우선 확인
	AActor* LockOnActor;
	if (FindLockOnTarget(LockOnActor)) return LockOnActor;

	// 2. 락온 타겟이 없을 경우, 전방 탐지 실행
	AWarriorHeroCharacter* Hero = GetHeroCharacterFromActorInfo();
	if (!Hero)
	{
		return nullptr;
	}

	const FVector TraceStart = Hero->GetActorLocation();
	const FVector TraceEnd = TraceStart + (Hero->GetActorForwardVector() * FinisherTraceDistance);
	const TArray<AActor*> ActorsToIgnore; // 무시할 액터 없음

	// 탐지할 오브젝트 타입을 Pawn으로 한정
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

	return SphereTraceFirstActor(Hero, TraceStart, TraceEnd, FinisherTraceRadius, ObjectTypes, ActorsToIgnore, Hero);
}

/**
 * @brief 몽타주에서 데미지 이벤트가 수신되었을 때 호출되는 콜백 함수입니다.
 * @param Payload 이벤트와 함께 전달된 데이터입니다.
 * @details 타겟의 체력을 0으로 설정하고, 성공 이벤트를 전송하며, 죽음 효과를 재생하고,
 *          사망 태그를 추가한 후 어빌리티를 종료합니다.
 */
void UWarriorFinisherGameplayAbility::OnDamageEventReceived(FGameplayEventData Payload)
{
	if (!IsValid(CachedFinisherTarget))
	{
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
		return;
	}
	
	if (DeathEffectClass)
	{
		// "누가, 무엇으로" 보냈는지에 대한 정보가 담긴 '송장'을 만든다
		FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
		ContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());

		// '송장'을 포함하여 이펙트 명세서를 만든다.
		FGameplayEffectSpecHandle DeathSpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
			DeathEffectClass,
			GetAbilityLevel(),
			ContextHandle
		);

		if (DeathSpecHandle.IsValid())
		{
			// 데미지 값을 SetByCaller로 추가합니다.
			DeathSpecHandle.Data->SetSetByCallerMagnitude(
				WarriorGamePlayTags::Player_SetByCaller_AttackType_Finisher,
				FinisherDamage.GetValueAtLevel(GetAbilityLevel())
			);

			
			// 시전자의 ASC가 대상의 ASC에게 이펙트를 직접 적용하도록 명령합니다.
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CachedFinisherTarget);
			if (TargetASC)
			{
				GetWarriorAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*DeathSpecHandle.Data, TargetASC);
			}
		}
	}

	// 4. 어빌리티 종료
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

/**
 * @brief 몽타주에서 이동 준비 이벤트가 수신되었을 때 호출되는 콜백 함수입니다.
 * @param Payload 이벤트와 함께 전달된 데이터입니다.
 * @details 플레이어가 다시 움직일 수 있도록 어빌리티를 종료합니다.
 */
void UWarriorFinisherGameplayAbility::OnMovementReady(FGameplayEventData Payload)
{
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
}

/**
 * @brief 어빌리티 활성화 가능 여부를 판단하기 위한 타겟을 찾습니다.
 * @param ActorInfo 어빌리티를 활성화하는 액터의 정보입니다.
 * @return 잠재적인 타겟 액터. 찾지 못하면 nullptr를 반환합니다.
 * @details GetFinisherTarget과 유사하게 락온 타겟을 우선 확인하고, 없으면 전방 탐지를 수행합니다.
 */
AActor* UWarriorFinisherGameplayAbility::GetTargetForActivation(const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo)
	{
		return nullptr;
	}

	// 1. 락온 타겟 확인
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		TArray<FGameplayAbilitySpec> ActivatableAbilities = ASC->GetActivatableAbilities();
		for (const FGameplayAbilitySpec& Spec : ActivatableAbilities)
		{
			if (Spec.IsActive() && Spec.Ability->IsA<UHeroGameplayAbility_TargetLock>())
			{
				UHeroGameplayAbility_TargetLock* TargetLockAbility = Cast<UHeroGameplayAbility_TargetLock>(Spec.Ability);
				if (TargetLockAbility && IsValid(TargetLockAbility->GetLockedTarget()))
				{
					return TargetLockAbility->GetLockedTarget();
				}
			}
		}
	}

	// 2. 전방 탐지
	// ActorInfo에서 아바타를 가져옴
	AWarriorHeroCharacter* Hero = Cast<AWarriorHeroCharacter>(ActorInfo->AvatarActor.Get());
	if (!IsValid(Hero))
	{
		return nullptr;
	}
	
	const FVector TraceStart = Hero->GetActorLocation();
	const FVector TraceEnd = TraceStart + (Hero->GetActorForwardVector() * FinisherTraceDistance);
	const TArray<AActor*> ActorsToIgnore;
	
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	// return SphereTraceFirstActor(Hero, TraceStart, TraceEnd, FinisherTraceRadius, ObjectTypes, ActorsToIgnore);
	return SphereTraceFirstActor(Hero, TraceStart, TraceEnd, FinisherTraceRadius, ObjectTypes, ActorsToIgnore, Hero);
}


/**
 * @brief 지정된 범위와 각도 내에서 첫 번째 유효한 액터를 찾기 위해 구(Sphere) 트레이스를 수행합니다.  
 *        피격된 액터가 플레이어의 전방 방향에 대해 지정된 각도 임계값 안에 있는지도 확인합니다.
 *
 * @param WorldContextObject 구 트레이스를 위한 컨텍스트 객체로, 일반적으로 UObject를 상속한 객체입니다.
 * @param TraceStart 구 트레이스의 시작 위치.
 * @param TraceEnd 구 트레이스의 끝 위치.
 * @param Radius 구 트레이스의 반경.
 * @param ObjectTypes 구 트레이스에 포함할 오브젝트 타입(충돌 오브젝트 채널 기반).
 * @param ActorsToIgnore 구 트레이스에서 제외할 액터 목록.
 * @param Hero 트레이스를 수행하는 영웅 캐릭터로, 방향과 위치 계산에 사용됩니다.
 * @return 트레이스에 의해 맞은 액터 중 각도 임계값 내에 있는 첫 번째 유효한 액터.  
 *         유효한 액터가 없을 경우 nullptr을 반환합니다.
 */

AActor* UWarriorFinisherGameplayAbility::SphereTraceFirstActor(
	UObject* WorldContextObject,
	const FVector& TraceStart,
	const FVector& TraceEnd,
	float Radius,
	const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
	const TArray<AActor*>& ActorsToIgnore,
	AWarriorHeroCharacter* Hero
) const
{
	FHitResult HitResult;

	const EDrawDebugTrace::Type DebugDraw = bDrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	const bool bHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
		WorldContextObject,
		TraceStart,
		TraceEnd,
		Radius,
		ObjectTypes,
		false, // bTraceComplex
		ActorsToIgnore,
		DebugDraw,
		HitResult,
		true // bIgnoreSelf
	);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (IsValid(HitActor))
		{
			// 플레이어의 정면 벡터
			const FVector PlayerForward = Hero->GetActorForwardVector();
			// 플레이어에서 타겟으로 향하는 방향 벡터 (정규화)
			const FVector PlayerToTarget = (HitActor->GetActorLocation() - Hero->GetActorLocation()).GetSafeNormal();

			// 두 벡터를 내적하여 각도를 확인
			const float DotProduct = FVector::DotProduct(PlayerForward, PlayerToTarget);
			const float AngleDifference = UKismetMathLibrary::DegAcos(DotProduct);

			// 계산된 각도가 설정된 값(예: 45도)의 절반보다 작거나 같은지 확인
			// (45도 값 = 정면 기준 좌/우 22.5도씩, 총 45도 범위)
			if (AngleDifference <= FinisherAngleThreshold / 2.f)
			{
				return HitActor;
			}
		}
	}
	
	return nullptr;
}

/**
 * @brief 주어진 플레이어 캐릭터와 타겟 캐릭터에 적합한 처형 애니메이션 쌍을 선택합니다.
 *
 * @param PlayerCharacter 처형을 실행할 플레이어 캐릭터입니다.
 * @param TargetCharacter 처형 대상이 되는 캐릭터입니다.
 * @return FFinisherMontagePair* 선택된 애니메이션 쌍에 대한 포인터. 적합한 쌍이 없으면 nullptr을 반환합니다.
 *
 * 
 * 1. PlayerCharacter와 TargetCharacter가 유효한지 확인하고, 애니메이션 쌍 데이터(FinisherMontagePairs)가 비어 있는 경우 nullptr을 반환합니다.
 * 2. FinisherMontagePairs 배열을 순회하며, 각 애니메이션 쌍의 조건(PlayerClass 및 TargetClass)에 맞는지 확인합니다.
 *    - 조건에 부합하는 애니메이션 쌍의 인덱스를 UsableIndices 배열에 저장합니다.
 * 3. 조건에 맞는 애니메이션 쌍이 없을 경우 nullptr을 반환합니다.
 * 4. 조건을 만족하는 애니메이션 쌍 중 무작위로 하나를 선택하여 반환합니다.
 */
const FFinisherMontagePair* UWarriorFinisherGameplayAbility::SelectFinisherPair(
	const AWarriorHeroCharacter* PlayerCharacter, const ACharacter* TargetCharacter) const
{
	if (!PlayerCharacter || !TargetCharacter || FinisherMontagePairs.IsEmpty())
	{
		return nullptr;
	}

	// 1. 현재 상황에 사용 가능한 애니메이션들의 인덱스를 저장할 배열
	TArray<int32> UsableIndices;
	for (int32 i = 0; i < FinisherMontagePairs.Num(); ++i)
	{
		const FFinisherMontagePair& Pair = FinisherMontagePairs[i];

		// 2. 플레이어와 타겟 클래스가 이 애니메이션 쌍의 조건과 맞는지 확인
		// (IsA()는 상속 관계도 확인해줍니다.)
		bool bPlayerMatch = !Pair.AllowedPlayerClass || PlayerCharacter->IsA(Pair.AllowedPlayerClass);
		bool bTargetMatch = !Pair.AllowedTargetClass || TargetCharacter->IsA(Pair.AllowedTargetClass);

		if (bPlayerMatch && bTargetMatch)
		{
			UsableIndices.Add(i);
		}
	}

	// 3. 사용 가능한 애니메이션이 하나도 없으면 nullptr 반환
	if (UsableIndices.IsEmpty())
	{
		return nullptr;
	}

	// 4. 사용 가능한 애니메이션 중에서만 무작위로 하나를 선택
	const int32 PickedIndexInUsableArray = FMath::RandRange(0, UsableIndices.Num() - 1);
	const int32 PickedIndexInMainArray = UsableIndices[PickedIndexInUsableArray];
    
	return &FinisherMontagePairs[PickedIndexInMainArray];
}
