// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GEExecCalculate/GEExecCalculate_DamageTaken.h"
#include "AbilitySystem/WarriorAttributeSet.h"
#include "WarriorGamePlayTags.h"

#include "WarriorDebugHelper.h"

struct FWarriorDamageCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageTaken)
	
	FWarriorDamageCapture()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet, AttackPower, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet, DefensePower, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet, DamageTaken, Target, false);
	}
};

static FWarriorDamageCapture& GetWarriorDamageCapture()
{
	static FWarriorDamageCapture WarriorDamageCapture;
	return WarriorDamageCapture;
}

UGEExecCalculate_DamageTaken::UGEExecCalculate_DamageTaken()
{
	RelevantAttributesToCapture.Add(GetWarriorDamageCapture().AttackPowerDef);
	RelevantAttributesToCapture.Add(GetWarriorDamageCapture().DefensePowerDef);
	RelevantAttributesToCapture.Add(GetWarriorDamageCapture().DamageTakenDef);
}

void UGEExecCalculate_DamageTaken::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput
) const
{
	const FGameplayEffectSpec& EffectSpec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

	float SourceAttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetWarriorDamageCapture().AttackPowerDef,
		EvaluateParameters,
		SourceAttackPower
	);

	float BaseDamage = 0.f;
	bool bIsFinisherAttack = false; 
	int32 UsedLightAttackComboCount = 0;
	int32 UsedHeavyAttackComboCount = 0;

	for (const TPair<FGameplayTag, float>& TagMagnitube : EffectSpec.SetByCallerTagMagnitudes)
	{
		const float MagnitudeValue = TagMagnitube.Value;
		if (TagMagnitube.Key.MatchesTagExact(WarriorGamePlayTags::Shared_SetByCaller_BaseDamage) || TagMagnitube.Key.MatchesTagExact(WarriorGamePlayTags::Player_SetByCaller_AttackType_Finisher))
		{
			BaseDamage = MagnitudeValue;
			bIsFinisherAttack = true;
		}
		if (TagMagnitube.Key.MatchesTagExact(WarriorGamePlayTags::Player_SetByCaller_AttackType_Light))
		{
			UsedLightAttackComboCount = MagnitudeValue;
		}

		if (TagMagnitube.Key.MatchesTagExact(WarriorGamePlayTags::Player_SetByCaller_AttackType_Heavy))
		{
			UsedHeavyAttackComboCount = MagnitudeValue;
		}
	}

	float TargetDefensePower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		GetWarriorDamageCapture().DefensePowerDef,
		EvaluateParameters,
		TargetDefensePower
	);

	if (UsedLightAttackComboCount != 0)
	{
		const float DamageIncreasePercentLightAttack = (UsedLightAttackComboCount - 1) * 0.05f + 1.f;

		BaseDamage *= DamageIncreasePercentLightAttack;
	}

	if (UsedHeavyAttackComboCount != 0)
	{
		const float DamageIncreasePercentHeavyAttack = UsedHeavyAttackComboCount * 0.15f + 1.f;
		BaseDamage *= DamageIncreasePercentHeavyAttack;
	}
	
	float FinalDamageDone;
	if (bIsFinisherAttack)
	{
		// 처형 공격일 경우, 공격력/방어력 계산을 모두 무시하고
		// 블루프린트에서 설정한 BaseDamage를 최종 데미지로 사용
		FinalDamageDone = BaseDamage;
	}
	else
	{
		// 일반 공격일 경우, 기존의 콤보 및 공/방 계산 로직을 그대로 사용.
		// ... (기존 Light/Heavy 콤보 데미지 계산 로직) ...
		FinalDamageDone = BaseDamage * SourceAttackPower / TargetDefensePower;
	}

	if (FinalDamageDone > 0.f)
	{
		FGameplayModifierEvaluatedData ModifierEvaluatedData = FGameplayModifierEvaluatedData(
			GetWarriorDamageCapture().DamageTakenProperty,
			EGameplayModOp::Override,
			FinalDamageDone
		);

		OutExecutionOutput.AddOutputModifier(ModifierEvaluatedData);
	}
}
