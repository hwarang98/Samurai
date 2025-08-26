// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WarriorEnumTypes.h"

class FWarriorCountDownAction : public FPendingLatentAction
{
public:
	FWarriorCountDownAction(float InTotalCountDownTime, float InUpdateInterval, float& InOutRemainingTime, EWarriorCountDownActionOutput& InCountDownOutput, const FLatentActionInfo& LatentInfo)
	: bNeedToCancel(false)
	, TotalCountDownTime(InTotalCountDownTime)
	, UpdateInterval(InUpdateInterval)
	, OutRemainingTime(InOutRemainingTime)
	, CountDownOutput(InCountDownOutput)
	, ExecutionFunction(LatentInfo.ExecutionFunction)
	, OutputLink(LatentInfo.Linkage)
	, CallbackTarget(LatentInfo.CallbackTarget)
	, ElapsedInterval(0.f)
	, ElapsedTimeSinceStart(0.f)
	{}

	// 매 틱마다 호출
	virtual void UpdateOperation(FLatentResponse& Response) override;
	
	void CancelAction();

private:
	bool bNeedToCancel; /* 액션을 취소하는데 사용 */
	float TotalCountDownTime; /* 총 카운트 시간 */
	float UpdateInterval; /* 업데이트 주기 */
	float& OutRemainingTime; /* 블루프린트에 남은 시간을 알리는데 사용 */
	EWarriorCountDownActionOutput& CountDownOutput; /* 카운트다운 노드에서 어떤 출력핀을 실행할지 결정 */
	FName ExecutionFunction; /* 실행함수 */
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
	float ElapsedInterval; /* 경과시간 */
	float ElapsedTimeSinceStart; /* 시작 후 결과된 시간 */
};
