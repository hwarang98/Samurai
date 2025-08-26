// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WarriorHeroGameplayAbility.h"
#include "AbilitySystem/Abilites/WarriorGameplayAbility.h"
#include "WarriorFinisherGameplayAbility.generated.h"

struct FFinisherMontagePair;
/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorFinisherGameplayAbility : public UWarriorHeroGameplayAbility
{
	GENERATED_BODY()

public:
	UWarriorFinisherGameplayAbility();

protected:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// ~Begin UGameplayAbility Interface
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// ~End UGameplayAbility Interface
	bool FindLockOnTarget(AActor*& LockOnActor) const;
	
	UFUNCTION(BlueprintCallable, Category = "Finisher")
	AActor* GetFinisherTarget() const;

	UPROPERTY(EditDefaultsOnly, Category = "Finisher")
	TSubclassOf<UGameplayEffect> DeathEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Finisher", meta = (Categories = "Event"))
	FGameplayTag DamageEventTag; // Player.Event.FinisherDamage
	
	/** 피니셔 공격이 가하는 기본 데미지 */
	UPROPERTY(EditDefaultsOnly, Category = "Finisher|Damage")
	FScalableFloat FinisherDamage; 

	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnMovementReady(FGameplayEventData Payload);

	UPROPERTY(EditDefaultsOnly, Category = "Finisher|Montage")
	TArray<FFinisherMontagePair> FinisherMontagePairs;

private:
	/** 전방 탐지에 사용할 트레이스 거리 */
	UPROPERTY(EditDefaultsOnly, Category = "Finisher|Trace")
	float FinisherTraceDistance = 250.f;

	/** 전방 탐지에 사용할 트레이스 반경 */
	UPROPERTY(EditDefaultsOnly, Category = "Finisher|Trace")
	float FinisherTraceRadius = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Finisher|MotionWarpKeyName")
	FName PlayerMotionWarpKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "Finisher|MotionWarpKeyName")
	FName EnemyMotionWarpKeyName;

	UPROPERTY(EditDefaultsOnly, Category = "Finisher|BlackboardStopKeyName")
	FName ActionLockedKeyName = FName("bIsActionLock");

	UPROPERTY(EditDefaultsOnly, Category = "Finisher|Trace")
	bool bDrawDebug = false;

	AActor* GetTargetForActivation(const FGameplayAbilityActorInfo* ActorInfo) const;

	UPROPERTY()
	AActor* CurrentLockedActor;

	/** 처형할 대상을 저장하기 위한 멤버 변수 */
	UPROPERTY()
	TObjectPtr<AActor> CachedFinisherTarget;

	const FFinisherMontagePair* CachedMontagePair = nullptr;

	const FFinisherMontagePair* SelectFinisherPair(
		const AWarriorHeroCharacter* PlayerCharacter,
		const ACharacter* TargetCharacter
	) const;
	
	AActor* SphereTraceFirstActor(
		UObject* WorldContextObject,
		const FVector& TraceStart,
		const FVector& TraceEnd,
		float Radius,
		const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes,
		const TArray<AActor*>& ActorsToIgnore,
		AWarriorHeroCharacter* Hero
	) const;

	UPROPERTY(EditDefaultsOnly, Category = "Finisher|Trace")
	float FinisherAngleThreshold = 45.f;
};
