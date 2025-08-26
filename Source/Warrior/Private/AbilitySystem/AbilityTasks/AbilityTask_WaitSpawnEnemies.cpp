// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AbilityTasks/AbilityTask_WaitSpawnEnemies.h"
#include "AbilitySystemComponent.h"
#include "NavigationSystem.h"
#include "Engine/AssetManager.h"
#include "Characters/WarriorEnemyCharacter.h"

#include "WarriorDebugHelper.h"

UAbilityTask_WaitSpawnEnemies* UAbilityTask_WaitSpawnEnemies::WaitSpawnEnemies(
	UGameplayAbility* OwningAbility,
    FGameplayTag EventTag,
    TSoftClassPtr<AWarriorEnemyCharacter> SoftEnemyClassToSpawn,
    int32 NumToSpawn,
    const FVector& SpawnOrigin,
    float RandomSpawnRadius
)
{
	UAbilityTask_WaitSpawnEnemies* Node = NewAbilityTask<UAbilityTask_WaitSpawnEnemies>(OwningAbility);
	Node->CachedEventTag = EventTag;
	Node->CachedSoftEnemyClassToSpawn = SoftEnemyClassToSpawn;
	Node->CachedNumToSpawn = NumToSpawn;
	Node->CachedSpawnOrigin = SpawnOrigin;
	Node->CachedRandomSpawnRadius = RandomSpawnRadius;

	return Node;
}

void UAbilityTask_WaitSpawnEnemies::OnGameplayEventReceived(const FGameplayEventData* InPayload)
{
	if (ensure(!CachedSoftEnemyClassToSpawn.IsNull()))
	{
		UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
			CachedSoftEnemyClassToSpawn.ToSoftObjectPath(),
			FStreamableDelegate::CreateUObject(this, &ThisClass::OnEnemyClassLoaded)
		);
	}
	else
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		}
		EndTask();
	}
}

void UAbilityTask_WaitSpawnEnemies::OnEnemyClassLoaded()
{
	UClass* LoadedClass = CachedSoftEnemyClassToSpawn.Get();
	UWorld* World = GetWorld();

	if (!LoadedClass || !World)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		}
		EndTask();
		return;
	}

	TArray<AWarriorEnemyCharacter*> SpawnedEnemies;

	FActorSpawnParameters SpawnParam;
	// 가능하면 충돌을 피해서 스폰 위치를 조정하되, 충돌이 있더라도 무조건 스폰한다
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int32 i = 0; i < CachedNumToSpawn; i++)
	{
		FVector RandomLocation;
		UNavigationSystemV1::K2_GetRandomReachablePointInRadius(
			this,                    // 월드 컨텍스트 오브젝트 (보통 UAbilityTask나 AActor)
			CachedSpawnOrigin,      // 중심 위치 (ex. 스폰 기준점)
			RandomLocation,         // 결과로 반환될 랜덤 위치
			CachedRandomSpawnRadius // 반경 (얼마나 떨어진 거리까지 랜덤 지점 허용할 것인지)
		);

		RandomLocation += FVector(0.f, 0.f, 150.f);  // 위로 살짝 띄움, 벽에 갇히는 현상 방지

		const FRotator SpawnFacingRotation = AbilitySystemComponent->GetAvatarActor()->GetActorForwardVector().ToOrientationRotator();
		
		AWarriorEnemyCharacter* SpawnedEnemy = World->SpawnActor<AWarriorEnemyCharacter>(LoadedClass, RandomLocation, SpawnFacingRotation, SpawnParam);

		if (SpawnedEnemy)
		{
			SpawnedEnemies.Add(SpawnedEnemy);
		}
	}

	if (ShouldBroadcastAbilityTaskDelegates())
	{   
		if (!SpawnedEnemies.IsEmpty())
		{
			OnSpawnFinished.Broadcast(SpawnedEnemies);
		}
		else
		{
			DidNotSpawn.Broadcast(TArray<AWarriorEnemyCharacter*>());
		}
	}

	EndTask();
}

void UAbilityTask_WaitSpawnEnemies::Activate()
{
	FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedEventTag);
	DelegateHandle = Delegate.AddUObject(this, &ThisClass::OnGameplayEventReceived);
}

void UAbilityTask_WaitSpawnEnemies::OnDestroy(bool bInOwnerFinished)
{
	FGameplayEventMulticastDelegate& Delegate = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(CachedEventTag);
	Delegate.Remove(DelegateHandle);
	Super::OnDestroy(bInOwnerFinished);
}

