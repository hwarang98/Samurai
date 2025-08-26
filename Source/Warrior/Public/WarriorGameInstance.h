// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/GameInstance.h"
#include "WarriorGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FWarriorGameLevelSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "GameData.Level"))
	FGameplayTag LevelTag;
	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> Level;

	bool IsValid() const
	{
		return LevelTag.IsValid() && !Level.IsNull();
	}
};

/**
 * @class UWarriorGameInstance
 * @brief "Warrior" 게임 프로젝트에 특화된 커스텀 게임 인스턴스 클래스.
 *
 * UWarriorGameInstance는 기본 UGameInstance의 기능을 확장하여,
 * 프로젝트 전용 초기화, 데이터 처리, 그리고 "Warrior" 게임에 맞춘
 * 게임 로직을 구현할 수 있도록 합니다.
 *
 * 이 클래스는 게임의 전체 수명 주기 동안 접근 가능한
 * 전역 데이터 및 상태를 관리·저장하는 중앙 관리 지점으로 설계되었습니다.
 *
 * 또한 UWarriorFunctionLibrary와 같은 다른 클래스와 상호작용하여
 * Warrior 전용 게임플레이 기능과 관련된 유틸리티와 기능을 제공합니다.
 */
UCLASS()
class WARRIOR_API UWarriorGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	
	UFUNCTION(BlueprintPure,  meta = (GameplayTagFilter = "GameData.Level"))
	TSoftObjectPtr<UWorld> GetGameLevelByTag(FGameplayTag InTag) const;
	
protected:
	virtual void OnPreLoadMap(const FString& MapName);
	virtual void OnDestinationWorldLoaded(UWorld* LoadedWorld);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FWarriorGameLevelSet> GameLevelSets;
	
};
