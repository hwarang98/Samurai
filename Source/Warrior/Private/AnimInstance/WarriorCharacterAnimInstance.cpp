// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstance/WarriorCharacterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/WarriorBaseCharacter.h"
#include "KismetAnimationLibrary.h"

#include "WarriorDebugHelper.h"

void UWarriorCharacterAnimInstance::NativeInitializeAnimation()
{
	OwningCharacter = Cast<AWarriorBaseCharacter>(TryGetPawnOwner());
	if (OwningCharacter)
	{
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
	}
}

void UWarriorCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (!OwningCharacter || !OwningMovementComponent)
	{
		return;
	}
	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D() > 0.f;

	GroundSpeed = OwningCharacter->GetVelocity().Size2D();

	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(
		OwningCharacter->GetVelocity(),
		OwningCharacter->GetActorRotation()
	);
}
