// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_RotateFaceTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_RotateFaceTarget::UBTTask_RotateFaceTarget()
{
	NodeName = TEXT("Native Rotate Face Target Actor");

	// "이 서비스 노드에서 재정의한 알림 함수들이 있다면, 해당 이벤트가 발생했을 때 그 함수들을 호출해줘" 라고 비헤이비어 트리 시스템에 자동으로 등록
	INIT_TASK_NODE_NOTIFY_FLAGS();
	AnglePrecision = 10.f;
	RotationInterpSpeed = 5.f;

	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;

	InTargetToFaceKey.AddObjectFilter(
		this,
		GET_MEMBER_NAME_CHECKED(ThisClass, InTargetToFaceKey),
		AActor::StaticClass()
	);
}

void UBTTask_RotateFaceTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InTargetToFaceKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UBTTask_RotateFaceTarget::GetInstanceMemorySize() const
{
	return sizeof(FRotateFaceTargetTaskMemory);
}

FString UBTTask_RotateFaceTarget::GetStaticDescription() const
{
	const FString KeyDescription = InTargetToFaceKey.SelectedKeyName.ToString();

	return FString::Printf(
		TEXT("%s 키에 해당하는 대상을 향해, 지정된 각도 정밀도(%s)에 도달할 때까지 부드럽게 회전합니다."),
		*KeyDescription,
		*FString::SanitizeFloat(AnglePrecision)
	);
}

/**
 * RotateFaceTarget 비헤이비어 트리 태스크를 실행합니다.
 *
 * 이 태스크는 AI가 특정 타겟 액터를 바라보도록 회전하는 역할을 합니다.
 * 타겟 액터는 블랙보드 키에서 가져오며, 태스크 인스턴스 전용 메모리(`FRotateFaceTargetTaskMemory`)에
 * 조종 중인 Pawn과 타겟 액터를 저장합니다.
 *
 * 아래 조건에 따라 실행 결과를 반환합니다:
 * - 이미 타겟을 충분히 바라보고 있다면 → Succeeded
 * - 타겟이나 조종 중인 Pawn이 유효하지 않다면 → Failed
 * - 회전이 아직 완료되지 않았다면 → InProgress (이후 TickTask에서 회전 계속 수행)
 *
 * @param OwnerComp      이 태스크를 소유한 비헤이비어 트리 컴포넌트
 * @param NodeMemory     태스크 인스턴스 전용 메모리 블록 포인터
 * @return               실행 결과 (Succeeded / Failed / InProgress)
 */
EBTNodeResult::Type UBTTask_RotateFaceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 블랙보드에서 타겟 액터(Object 타입으로 저장된 것)를 가져온다.
	UObject* ActorObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(InTargetToFaceKey.SelectedKeyName);

	// 가져온 오브젝트를 AActor 타입으로 캐스팅 (실패하면 nullptr 반환)
	AActor* TargetActor = Cast<AActor>(ActorObject);

	// 현재 AI 컨트롤러가 조종 중인 폰(Pawn)을 가져온다.
	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();

	// BTTask 노드의 인스턴스 전용 메모리 캐스팅 (구조체로 정의된 상태 데이터 접근용)
	FRotateFaceTargetTaskMemory* Memory = CastInstanceNodeMemory<FRotateFaceTargetTaskMemory>(NodeMemory);

	// Memory가 nullptr이 아닌지 확인 (에디터나 개발 빌드에서는 크래시 유발)
	check(Memory);

	// 메모리에 현재 조종 중인 폰을 저장
	Memory->OwningPawn = OwningPawn;

	// 메모리에 타겟 액터도 저장
	Memory->TargetActor = TargetActor;

	// 메모리 구조체 내 유효성 검사 함수 -> 타겟이나 폰이 유효하지 않으면 실패 처리
	if (!Memory->IsValid())
	{
		// 유효하지 않으면 이 태스크 실패로 처리
		return EBTNodeResult::Failed;
	}

	// 이미 타겟을 향한 회전이 완료되어 있는 경우
	if (HasReachedAnglePrecision(OwningPawn, TargetActor))
	{
		// 메모리 상태 초기화
		Memory->Reset();

		// 작업 완료로 성공 반환
		return EBTNodeResult::Succeeded;
	}

	// 아직 회전이 덜 됐으면, 틱에서 계속 처리하게 하기 위해 InProgress 반환
	return EBTNodeResult::InProgress;
}


void UBTTask_RotateFaceTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// 노드 메모리를 사용자 정의 구조체 타입으로 캐스팅하여 사용
	FRotateFaceTargetTaskMemory* Memory = CastInstanceNodeMemory<FRotateFaceTargetTaskMemory>(NodeMemory);

	// 유효하지 않은 메모리일 경우 태스크 실패 처리
	if (!Memory->IsValid())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}

	// 목표 각도 오차 범위 내에 도달했으면 태스크 성공 처리
	if (HasReachedAnglePrecision(Memory->OwningPawn.Get(), Memory->TargetActor.Get()))
	{
		Memory->Reset(); // 상태 초기화
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	else
	{
		// 타겟 방향을 향한 회전값 계산 (현재 위치에서 타겟 위치까지의 방향)
		const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(
			Memory->OwningPawn->GetActorLocation(),
			Memory->TargetActor->GetActorLocation()
		);

		// 현재 회전에서 타겟 회전까지 부드럽게 보간
		const FRotator TargetRotation = FMath::RInterpTo(
			Memory->OwningPawn->GetActorRotation(),
			LookAtRotation,
			DeltaSeconds,
			RotationInterpSpeed
		);

		// 보간된 회전값을 캐릭터에 적용
		Memory->OwningPawn->SetActorRotation(TargetRotation);
	}
}


bool UBTTask_RotateFaceTarget::HasReachedAnglePrecision(APawn* QueryPawn, AActor* TargetActor) const
{
	// 현재 Pawn이 바라보고 있는 방향 벡터를 가져옴
	const FVector OwnerForward = QueryPawn->GetActorForwardVector();

	// 타겟까지의 방향 벡터를 정규화하여 가져옴 (현재 위치 → 타겟 위치)
	const FVector OwnerToTargetNormalized = (TargetActor->GetActorLocation() - QueryPawn->GetActorLocation()).
		GetSafeNormal();

	// 두 벡터(바라보는 방향과 타겟 방향)의 내적 계산 (코사인 값 반환: -1 ~ 1)
	const float DotResult = FVector::DotProduct(OwnerForward, OwnerToTargetNormalized);

	// 내적 결과를 이용해 두 벡터 사이의 각도(라디안)를 계산
	const float AngleDiff = FMath::Acos(DotResult);

	// 현재 회전 각도가 허용 오차(AnglePrecision) 이내인지 확인하여 true/false 반환
	return AngleDiff <= AnglePrecision;
}
