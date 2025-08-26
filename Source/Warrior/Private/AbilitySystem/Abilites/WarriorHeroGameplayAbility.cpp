// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilites/WarriorHeroGameplayAbility.h"

#include "WarriorDebugHelper.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Controllers/WarriorHeroController.h"
#include "WarriorGamePlayTags.h"
#include "Components/UI/HeroUIComponent.h"

AWarriorHeroCharacter* UWarriorHeroGameplayAbility::GetHeroCharacterFromActorInfo() const
{
	if (!CachedWarriorHeroCharacter.IsValid())
	{
		CachedWarriorHeroCharacter = Cast<AWarriorHeroCharacter>(CurrentActorInfo->AvatarActor);
	}

	return CachedWarriorHeroCharacter.IsValid() ? CachedWarriorHeroCharacter.Get() : nullptr;
}

AWarriorHeroController* UWarriorHeroGameplayAbility::GetHeroControllerFromActorInfo() const
{
	// 1. 캐시된 컨트롤러가 유효하지 않은 경우 (최초 접근 or 이미 파괴됨)
	if (!CachedWarriorHeroController.IsValid())
	{
		// 2. 현재 Ability의 ActorInfo에서 PlayerController를 가져와 AWarriorHeroController로 캐스팅하여 캐시에 저장
		CachedWarriorHeroController = Cast<AWarriorHeroController>(CurrentActorInfo->PlayerController);
	}

	// 3. 캐시된 컨트롤러가 여전히 유효하면 해당 컨트롤러 반환, 그렇지 않으면 nullptr 반환
	return CachedWarriorHeroController.IsValid() ? CachedWarriorHeroController.Get() : nullptr;
}

UHeroCombatComponent* UWarriorHeroGameplayAbility::GetHeroCombatComponentFromActorInfo() const
{
	return GetHeroCharacterFromActorInfo()->GetHeroCombatComponent();
}

UHeroUIComponent* UWarriorHeroGameplayAbility::GetHeroUIComponentFromActorInfo()
{
	return GetHeroCharacterFromActorInfo()->GetHeroUIComponent();
}

/**
 * - 지정된 GameplayEffect 클래스와 입력 정보를 바탕으로 FGameplayEffectSpecHandle을 생성합니다.
 * - 이 함수는 무기의 기본 데미지와 현재 공격 상태(공격 태그, 콤보 수 등)를 SetByCaller 값으로 전달합니다.
 * - 생성된 SpecHandle은 대상에게 이펙트를 적용할 때 사용됩니다.
 *
 * @param EffectClass 적용할 GameplayEffect 클래스 (예: 데미지 이펙트)
 * @param InWeaponBaseDamage 무기의 기본 데미지 수치 (SetByCaller로 전달)
 * @param InCurrentAttackTypeTag 현재 공격 타입을 나타내는 태그 (예: 공격1, 차지공격 등)
 * @param InUsedComboCount 현재 콤보 수치 (SetByCaller로 전달)
 * @return 구성된 FGameplayEffectSpecHandle 인스턴스
 */
FGameplayEffectSpecHandle UWarriorHeroGameplayAbility::MakeHeroDamageEffectSpecHandle(
	TSubclassOf<UGameplayEffect> EffectClass,
	float InWeaponBaseDamage,
	FGameplayTag InCurrentAttackTypeTag,
	int32 InUsedComboCount
)
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

	// SetByCaller 방식으로 데미지 값 설정 (이펙트 블루프린트에서 동적으로 받을 수 있음)
	// TMap<FGameplayTag, float> SetByCallerTagMagnitudes;
	EffectSpecHandle.Data->SetSetByCallerMagnitude(
		WarriorGamePlayTags::Shared_SetByCaller_BaseDamage,
		InWeaponBaseDamage
	);

	// 공격 타입 태그가 유효한 경우, 해당 태그를 키로 콤보 카운트를 SetByCaller로 전달
	if (InCurrentAttackTypeTag.IsValid())
	{
		EffectSpecHandle.Data->SetSetByCallerMagnitude(InCurrentAttackTypeTag, InUsedComboCount);
	}

	// 완성된 이펙트 스펙 핸들을 반환 -> 이걸로 ApplyGameplayEffectToTarget() 같은 함수에 넘겨서 실제 적용함
	return EffectSpecHandle;
}

/**
 * 주어진 쿨다운 태그에 해당하는 능력의 남은 쿨다운 시간과 총 쿨다운 시간을 가져옵니다.
 *
 * @param InCooldownTag           확인할 쿨다운 효과의 GameplayTag
 * @param TotalCooldownTime       (출력) 쿨다운의 총 지속 시간
 * @param RemainingCooldownTime   (출력) 현재 남아 있는 쿨다운 시간
 *
 * @return 쿨다운이 아직 남아있으면 true, 그렇지 않으면 false
 */
bool UWarriorHeroGameplayAbility::GetAbilityRemainingCooldownByTag(FGameplayTag InCooldownTag, float& TotalCooldownTime, float& RemainingCooldownTime)
{
	// 입력으로 들어온 쿨다운 태그가 유효한지 검사 (디버그 모드에서 잘못된 값이면 프로그램 중단)
	check(InCooldownTag.IsValid());

	// 특정 태그를 가진 활성화된 GameplayEffect를 찾기 위한 쿼리 생성 (이 캐릭터가 가진 효과 중, 내가 지정한 태그가 포함된 효과가 있는지 찾아줘)
	FGameplayEffectQuery CooldownQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(InCooldownTag.GetSingleTagContainer());

	// 쿼리에 해당하는 활성화된 이펙트들의 남은 시간(Key)과 총 지속 시간(Value)을 가져옴
	TArray<TPair<float,float>> TimeRemainingAndDuration = GetAbilitySystemComponentFromActorInfo()->GetActiveEffectsTimeRemainingAndDuration(CooldownQuery);

	// 만약 하나라도 해당되는 쿨다운 효과가 존재한다면
	if (!TimeRemainingAndDuration.IsEmpty())
	{
		// 첫 번째 쿨다운 효과의 남은 시간과 총 시간을 변수에 할당
		RemainingCooldownTime = TimeRemainingAndDuration[0].Key;   // 남은 시간
		TotalCooldownTime = TimeRemainingAndDuration[0].Value;     // 전체 지속 시간
	}
    
	// 남은 시간이 0보다 크면 아직 쿨다운 중이므로 true 반환
	return RemainingCooldownTime > 0.f;
}
