// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstance/Hero/WarriorHeroAnimInstance.h"
#include "Characters/WarriorHeroCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UWarriorHeroAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (OwningCharacter)
	{
		OwningHeroCharacter = Cast<AWarriorHeroCharacter>(OwningCharacter);
	}
}

void UWarriorHeroAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	if (!OwningCharacter || !OwningMovementComponent)
	{
		return;
	}

	IsJump = OwningHeroCharacter->IsJumping();
	IsCrouch = OwningMovementComponent->IsCrouching();
	// GroundSpeed = OwningCharacter->GetVelocity().Size2D();

	if (bHasAcceleration)
	{
		IdleElpasedTime = 0.f;
		bShouldEnterRelaxState = false;
	}
	else
	{
		IdleElpasedTime += DeltaSeconds;
		bShouldEnterRelaxState = (IdleElpasedTime >= EnterRelaxStateThreshold);
	}

	if (GroundSpeed > KINDA_SMALL_NUMBER)
	{
		const FVector Velocity = OwningCharacter->GetVelocity();
		const FRotator MovementRot = Velocity.ToOrientationRotator(); // 이동 방향
		const FRotator ActorRot = OwningCharacter->GetActorRotation(); // 캐릭터가 보고 있는 방향

		// Yaw 차이를 계산
		float YawDiff = FMath::FindDeltaAngleDegrees(ActorRot.Yaw, MovementRot.Yaw);

		Angle = YawDiff;

		bIsMovingBackward = FMath::Abs(Angle) > 90.f;
	}
	else
	{
		Angle = 0.f;
		bIsMovingBackward = false;
	}
}
