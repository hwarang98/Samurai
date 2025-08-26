// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/WarriorEnemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/Combat/EnemyCombatComponent.h"
#include "DataAsset/StartUpData/DataAsset_EnemyStartUpData.h"
#include "Engine/AssetManager.h"
#include "Components/UI/EnemyUIComponent.h"
#include "Components/WidgetComponent.h"
#include "Widgets/WarriorWidgetBase.h"

#include "WarriorDebugHelper.h"

AWarriorEnemyCharacter::AWarriorEnemyCharacter()
{
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	bUseControllerRotationPitch = false; // 컨트롤러의 Pitch(상하 회전)를 캐릭터 본체에 적용하지 않음
	bUseControllerRotationRoll = false; // 컨트롤러의 Roll(좌우 기울기 회전)을 캐릭터 본체에 적용하지 않음
	bUseControllerRotationYaw = false; // 컨트롤러의 Yaw(좌우 회전)를 캐릭터 본체에 적용하지 않음

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	// 컨트롤러의 회전 방향을 이동 방향에 적용하지 않음 (회전은 이동 방향이 아니라 다른 방식으로 처리함)
	GetCharacterMovement()->bOrientRotationToMovement = true; // 캐릭터가 이동하는 방향을 바라보도록 회전하게 설정
	GetCharacterMovement()->RotationRate = FRotator(0.f, 180.f, 0.f); // 회전 속도를 설정 (Yaw = 좌우 회전 속도 180도/초)
	GetCharacterMovement()->MaxWalkSpeed = 300.f; // 걷기 속도를 300으로 설정 (기본 이동 속도)
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.f; // 캐릭터가 걷다가 멈출 때, 적당히 부드럽게 하는 수치

	EnemyCombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("EnemyCombatComponent"));
	EnemyUIComponent = CreateDefaultSubobject<UEnemyUIComponent>(TEXT("EnemyUIComponent"));
	EnemyHealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyHealthWidgetComponent"));

	EnemyHealthWidgetComponent->SetupAttachment(GetMesh());
}


void AWarriorEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (UWarriorWidgetBase* HealthWidget = Cast<UWarriorWidgetBase>(EnemyHealthWidgetComponent->GetUserWidgetObject()))
	{
		HealthWidget->InitEnemyCreatedWidget(this);
	}
}

UPawnCombatComponent* AWarriorEnemyCharacter::GetPawnCombatComponent() const
{
	return EnemyCombatComponent;
}

UPawnUIComponent* AWarriorEnemyCharacter::GetPawnUIComponent() const
{
	return EnemyUIComponent;
}

UEnemyUIComponent* AWarriorEnemyCharacter::GetEnemyUIComponent() const
{
	return EnemyUIComponent;
}

void AWarriorEnemyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitEnemyStartUpData();
}

void AWarriorEnemyCharacter::InitEnemyStartUpData()
{
	// 만약 CharacterStartUpData가 null이라면 (즉, 설정된 에셋이 없다면), 함수 종료
	if (CharacterStartUpData.IsNull())
	{
		return;
	}

	// 에셋 매니저를 통해 소프트 오브젝트 경로를 비동기로 로드
	UAssetManager::GetStreamableManager().RequestAsyncLoad(
		CharacterStartUpData.ToSoftObjectPath(), // 로드할 에셋의 경로 지정
		FStreamableDelegate::CreateLambda( // 로드 완료 시 호출할 콜백 람다 함수 정의
			[this]() // 현재 객체(this)를 캡처하여 멤버에 접근할 수 있도록 설정
			{
				// 로드가 완료되었으므로 실제 데이터 에셋 객체 포인터를 가져옴
				if (UDataAsset_StartUpDataBase* LoadData = CharacterStartUpData.Get())
				{
					// 로드된 스타트업 데이터를 사용해 AbilitySystemComponent에 능력 부여
					LoadData->GiveToAbilitySystemComponent(WarriorAbilitySystemComponent);
				}
			}
		)
	);
}
