// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DetourCrowdAIController.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionTypes.h"
#include "WarriorDetourCrowdAIController.generated.h"

struct FAIStimulus;
class UAISenseConfig_Sight;
class UAIPerceptionComponent;
/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorDetourCrowdAIController : public ADetourCrowdAIController
{
	GENERATED_BODY()

public:
	AWarriorDetourCrowdAIController(const FObjectInitializer& ObjectInitializer);

	//~ Begin IGenericTeamAgentInterface Interface.
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	//~ End IGenericTeamAgentInterface Interface.

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* AISenseConfig_Sight;

	UFUNCTION()
	virtual void OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Detour Crowd Avoidance Config")
	bool bEnableDetourCrowdAvoidance = true;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Detour Crowd Avoidance Config",
		meta = (EditCondition = "bEnableDetourCrowdAvoidance", UIMin = "1", UIMax = "4")
	)
	int32 DetourCrowdAvoidanceQuality = 4;

	UPROPERTY(
		EditDefaultsOnly,
		Category = "Detour Crowd Avoidance Config",
		meta = (EditCondition = "bEnableDetourCrowdAvoidance")
	)
	float CollisionQueryRange = 600.0f;

	void SetCrowdAvoidanceQualityByLevel(UCrowdFollowingComponent* CrowdComponent);
};
