// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RotateFaceTarget.generated.h"

struct FRotateFaceTargetTaskMemory
{
	TWeakObjectPtr<APawn> OwningPawn;

	TWeakObjectPtr<AActor> TargetActor;

	bool IsValid() const
	{
		return OwningPawn.IsValid() && TargetActor.IsValid();
	}

	void Reset()
	{
		OwningPawn.Reset();
		TargetActor.Reset();
	}
};

/**
 * 
 */
UCLASS()
class WARRIOR_API UBTTask_RotateFaceTarget : public UBTTaskNode
{
	GENERATED_BODY()

	UBTTask_RotateFaceTarget();

	// ~ Begin UBTNode interface
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;
	// ~ End UBTNode interface

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	bool HasReachedAnglePrecision(APawn* QueryPawn, AActor* TargetActor) const;

	UPROPERTY(EditAnywhere, Category = "Face Target")
	float AnglePrecision;

	UPROPERTY(EditAnywhere, Category = "Face Target")
	float RotationInterpSpeed;

	UPROPERTY(EditAnywhere, Category = "Face Target")
	FBlackboardKeySelector InTargetToFaceKey;
};
