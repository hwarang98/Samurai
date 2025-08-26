// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTService_OrientToTargetActor.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTService_OrientToTargetActor::UBTService_OrientToTargetActor()
{
	NodeName = TEXT("Native Orient Rotation To Target Actor"); // 대상 액터를 향해 자연스럽게 바라보도록 회전

	// "이 서비스 노드에서 재정의한 알림 함수들이 있다면, 해당 이벤트가 발생했을 때 그 함수들을 호출해줘" 라고 비헤이비어 트리 시스템에 자동으로 등록
	INIT_SERVICE_NODE_NOTIFY_FLAGS();

	RotationInterpSpeed = 5.f;
	Interval = 0.f;
	RandomDeviation = 0.f;
	InTargetActorKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, InTargetActorKey),
		AActor::StaticClass());
}

void UBTService_OrientToTargetActor::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InTargetActorKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTService_OrientToTargetActor::GetStaticDescription() const
{
	const FString KeyDescription = InTargetActorKey.SelectedKeyName.ToString();

	return FString::Printf(TEXT("%s 키를 기준으로 회전 방향을 정렬합니다. %s"), *KeyDescription, *GetStaticServiceDescription());
	// return FString::Printf(TEXT("Orient rotation to %s key %s"), *KeyDescription, *GetStaticServiceDescription());
}


/**
 * 틱마다 호출되는 함수로, AI가 타겟을 향해 회전하도록 처리
 * @param OwnerComp 
 * @param NodeMemory 
 * @param DeltaSeconds 
 */
void UBTService_OrientToTargetActor::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 부모 클래스의 TickNode 호출 (기본 동작 유지)
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// 블랙보드에서 타겟 액터 가져오기 (지정된 Key로부터)
	UObject* ActorObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(InTargetActorKey.SelectedKeyName);

	// 가져온 객체를 AActor 타입으로 캐스팅
	AActor* TargetActor = Cast<AActor>(ActorObject);

	// 현재 AIController가 소유한 Pawn 가져오기
	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();


	if (OwningPawn && TargetActor)
	{
		// 타겟 액터를 바라보는 방향(회전값)을 계산
		const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
			OwningPawn->GetActorLocation(), // 시작 위치: 나
			TargetActor->GetActorLocation() // 목표 위치: 타겟
		);

		// 현재 회전에서 목표 회전으로 부드럽게 보간
		const FRotator TargetRotation = FMath::RInterpTo(
			OwningPawn->GetActorRotation(), // 현재 회전
			LookAtRotation, // 목표 회전
			DeltaSeconds, // 델타 시간
			RotationInterpSpeed // 회전 보간 속도 (사용자가 지정)
		);

		// 보간된 회전값을 적용하여 타겟을 바라보게 함
		OwningPawn->SetActorRotation(TargetRotation);
	}
}
