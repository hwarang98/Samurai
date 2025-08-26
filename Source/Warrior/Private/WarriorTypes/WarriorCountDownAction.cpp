// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorTypes/WarriorCountDownAction.h"

/**
 * 매 틱마다 카운트다운 액션의 상태를 갱신하며, 카운트다운을 계속할지, 완료할지, 혹은 취소할지를 결정함.
 *
 * @param Response 카운트다운 과정에서 지연 동작을 처리하고 출력 트리거를 실행하는 데 사용되는 FLatentResponse 객체의 참조
 */
void FWarriorCountDownAction::UpdateOperation(FLatentResponse& Response)
{
	// 1. 외부에서 취소 요청이 들어왔는지 확인
	if (bNeedToCancel)
	{
		// 카운트다운 상태를 "Canceled"로 설정
		CountDownOutput = EWarriorCountDownActionOutput::Canceled;
		
		// Latent Action을 즉시 종료하고 바인딩된 함수 실행
		Response.FinishAndTriggerIf(true,ExecutionFunction,OutputLink,CallbackTarget);
		
		return;
	}

	// 2. 카운트다운이 모두 끝났는지 확인
	if (ElapsedTimeSinceStart >= TotalCountDownTime)
	{
		// 카운트다운 상태를 "Completed"로 설정
		CountDownOutput = EWarriorCountDownActionOutput::Completed;
		
		// Latent Action을 즉시 종료하고 바인딩된 함수 실행
		Response.FinishAndTriggerIf(true,ExecutionFunction,OutputLink,CallbackTarget);
		
		return;
	}

	// 3. 카운트다운이 진행 중이며 취소도 안 된 경우
	if (ElapsedInterval < UpdateInterval)
	{
		// 아직 업데이트 주기가 안 됨 -> 경과 시간만 누적
		ElapsedInterval += Response.ElapsedTime();
	}
	else
	{
		// 업데이트 주기가 지남 -> 실제 경과 시간을 갱신
		ElapsedTimeSinceStart += UpdateInterval > 0.f ? UpdateInterval : Response.ElapsedTime();

		// 남은 시간 계산
		OutRemainingTime = TotalCountDownTime - ElapsedTimeSinceStart;

		// 카운트다운 상태를 "Updated"로 설정
		CountDownOutput = EWarriorCountDownActionOutput::Updated;
		
		// Latent Action을 즉시 트리거 (Blueprint로 이벤트 전달 가능)
		Response.TriggerLink(ExecutionFunction,OutputLink,CallbackTarget);

		// 다음 업데이트 주기를 위해 초기화
		ElapsedInterval = 0.f;
	}
}

void FWarriorCountDownAction::CancelAction()
{
	bNeedToCancel = true;
}
