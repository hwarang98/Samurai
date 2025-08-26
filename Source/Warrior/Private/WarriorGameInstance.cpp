// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorGameInstance.h"
#include "MoviePlayer.h"

void UWarriorGameInstance::Init()
{
	Super::Init();
	
	// 맵이 로드되기 전에 호출되는 델리게이트에 OnPreLoadMap 함수를 등록
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &ThisClass::OnPreLoadMap);
	// 맵이 로드된 후, World 객체가 준비되었을 때 호출되는 델리게이트에 OnDestinationWorldLoaded 함수를 등록
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::OnDestinationWorldLoaded);
}

void UWarriorGameInstance::OnPreLoadMap(const FString& MapName)
{
	// 맵이 로드되기 "직전"에 실행되는 함수
	// MapName: 로드될 맵의 이름을 전달받음
	// 예시: 로딩 화면 표시, 현재 상태 정리, 데이터 저장 등에 활용 가능

	FLoadingScreenAttributes LoadingScreenAttributes;
	LoadingScreenAttributes.bAutoCompleteWhenLoadingCompletes = true;
	LoadingScreenAttributes.MinimumLoadingScreenDisplayTime = 2.f;
	LoadingScreenAttributes.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreenAttributes);
}

void UWarriorGameInstance::OnDestinationWorldLoaded(UWorld* LoadedWorld)
{
	// 맵이 완전히 로드된 뒤 실행되는 함수
	// LoadedWorld: 로드 완료된 World 객체를 전달받음
	// 예시: 캐릭터 스폰, UI 초기화, 게임 로직 재시작 등에 활용 가능

	GetMoviePlayer()->StopMovie();
}


TSoftObjectPtr<UWorld> UWarriorGameInstance::GetGameLevelByTag(FGameplayTag InTag) const
{
	for (const FWarriorGameLevelSet& GameLevelSet : GameLevelSets)
	{
		if (!GameLevelSet.IsValid())
		{
			continue;
		}

		if (GameLevelSet.LevelTag == InTag)
		{
			return GameLevelSet.Level;
		}
	}

	return TSoftObjectPtr<UWorld>();
}
