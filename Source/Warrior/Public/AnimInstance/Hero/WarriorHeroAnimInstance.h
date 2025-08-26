// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimInstance/WarriorCharacterAnimInstance.h"
#include "WarriorHeroAnimInstance.generated.h"

class AWarriorHeroCharacter;

/**
 * 
 */
UCLASS()
class WARRIOR_API UWarriorHeroAnimInstance : public UWarriorCharacterAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	FORCEINLINE bool IsMovingBackward() const { return bIsMovingBackward; }

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Refrences")
	AWarriorHeroCharacter* OwningHeroCharacter;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Locomotion")
	bool bShouldEnterRelaxState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Locomotion")
	float EnterRelaxStateThreshold = 5.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData|Locomotion")
	bool IsJump;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData|Locomotion")
	bool IsCrouch;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData|Locomotion")
	float Angle;

	float IdleElpasedTime;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimData|Locomotion", meta=(AllowPrivateAccess = "true"))
	bool bIsMovingBackward = false;
};
