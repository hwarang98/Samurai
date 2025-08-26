// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/WarriorDetourCrowdAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/CrowdFollowingComponent.h"

#include "WarriorDebugHelper.h"

/**
 * @brief AWarriorDetourCrowdAIController 클래스의 생성자입니다.
 * Detour Crowd를 사용하는 AI 컨트롤러를 초기화하고, 시야 감지 및 인지 컴포넌트를 설정합니다.
 * @param ObjectInitializer FObjectInitializer 객체입니다.
 */
AWarriorDetourCrowdAIController::AWarriorDetourCrowdAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
	AISenseConfig_Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("EnemySenseConfig_Sight"));
	if (AISenseConfig_Sight)
	{
		// '적'으로 설정된 대상을 감지하도록 설정
		AISenseConfig_Sight->DetectionByAffiliation.bDetectEnemies = true;
		// '아군'으로 설정된 대상은 감지하지 않도록 설정
		AISenseConfig_Sight->DetectionByAffiliation.bDetectFriendlies = false;
		// '중립'으로 설정된 대상은 감지하지 않도록 설정
		AISenseConfig_Sight->DetectionByAffiliation.bDetectNeutrals = false;

		AISenseConfig_Sight->SightRadius = 3200.f;
		// 대상을 한 번 놓쳤을 때, 시야를 완전히 잃게 되는 추가 반경을 0으로 설정 (즉, 기본 시야 반경을 벗어나면 바로 잃는다.)
		AISenseConfig_Sight->LoseSightRadius = 3500.f;
		AISenseConfig_Sight->PeripheralVisionAngleDegrees = 90.f;
	}

	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerceptionComponent"));
	if (PerceptionComponent)
	{
		PerceptionComponent->ConfigureSense(*AISenseConfig_Sight);
		PerceptionComponent->SetDominantSense(UAISenseConfig_Sight::StaticClass());
		// UAISense_Sight
		// 인지 시스템이 대상을 감지하거나 잃을 때마다 'OnEnemyPerceptionUpdated' 함수를 호출하도록 연결(바인딩)
		PerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &ThisClass::OnEnemyPerceptionUpdated);
	}

	// 팀 ID 설정
	SetGenericTeamId(FGenericTeamId(1));
}

/**
 * @brief 게임이 시작될 때 호출되는 메서드입니다.
 * Crowd Following Component의 시뮬레이션 상태 및 회피 관련 설정을 초기화합니다.
 */
void AWarriorDetourCrowdAIController::BeginPlay()
{
	Super::BeginPlay();

	if (UCrowdFollowingComponent* CrowdComponent = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent()))
	{
		//  Crowd AI 시스템을 사용할지 여부를 켜거나 끄는 데 사용
		const ECrowdSimulationState CrowdSimulationState =
			bEnableDetourCrowdAvoidance
				? ECrowdSimulationState::Enabled
				: ECrowdSimulationState::Disabled;

		CrowdComponent->SetCrowdSimulationState(CrowdSimulationState);

		SetCrowdAvoidanceQualityByLevel(CrowdComponent);

		CrowdComponent->SetAvoidanceGroup(1);
		CrowdComponent->SetGroupsToAvoid(1);
		CrowdComponent->SetCrowdCollisionQueryRange(CollisionQueryRange);
	}
}

/**
 * @brief 다른 액터에 대한 이 AI의 팀 태도를 결정합니다.
 * @param Other 태도를 확인할 다른 액터입니다.
 * @return ETeamAttitude::Type 팀 태도를 반환합니다. (Hostile, Friendly, Neutral)
 */
ETeamAttitude::Type AWarriorDetourCrowdAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	const APawn* PawnToCheck = Cast<const APawn>(&Other);

	const IGenericTeamAgentInterface* OtherTeamAgent = Cast<IGenericTeamAgentInterface>(PawnToCheck->GetController());

	// 적 인지
	if (OtherTeamAgent && OtherTeamAgent->GetGenericTeamId() < GetGenericTeamId())
	{
		return ETeamAttitude::Hostile;
	}

	return ETeamAttitude::Friendly;
}

/**
 * @brief AI 인지 시스템에 의해 타겟의 인지 정보가 업데이트될 때 호출되는 콜백 함수입니다.
 * @param Actor 인지된 액터입니다.
 * @param Stimulus 인지 자극에 대한 정보입니다.
 */
void AWarriorDetourCrowdAIController::OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// AI가 적을 인지한 경우
	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		if (!BlackboardComponent->GetValueAsObject(FName("TargetActor")))
		{
			// 적을 Blackboard에 저장
			if (Stimulus.WasSuccessfullySensed() && Actor)
			{
				BlackboardComponent->SetValueAsObject(FName("TargetActor"), Actor);
			}
		}
	}
}

/**
 * @brief 설정된 DetourCrowdAvoidanceQuality 레벨에 따라 Crowd 회피 품질을 설정합니다.
 * @param CrowdComponent 품질을 설정할 UCrowdFollowingComponent입니다.
 */
void AWarriorDetourCrowdAIController::SetCrowdAvoidanceQualityByLevel(UCrowdFollowingComponent* CrowdComponent)
{
	switch (DetourCrowdAvoidanceQuality)
	{
	case 1:
		CrowdComponent->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Low);
		break;

	case 2:
		CrowdComponent->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Medium);
		break;

	case 3:
		CrowdComponent->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::Good);

	case 4:
		CrowdComponent->SetCrowdAvoidanceQuality(ECrowdAvoidanceQuality::High);

	default:
		break;
	}
}
