// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilites/HeroGameplayAbility_TargetLock.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/WarriorWidgetBase.h"
#include "Controllers/WarriorHeroController.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGamePlayTags.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SizeBox.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "WarriorDebugHelper.h"


void UHeroGameplayAbility_TargetLock::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	TryLockOnTarget();
	InitTargetLockMovement();
	InitTargetLockMappingContext();
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UHeroGameplayAbility_TargetLock::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled
)
{
	ResetTargetLockMovement();
	ResetTargetLockMappingContext();
	CleanUp();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHeroGameplayAbility_TargetLock::OnTargetLockTick(float DeltaTime)
{
	const bool bIsDeadEnemyCharacter = UWarriorFunctionLibrary::NativeDoesActorHaveTag(CurrentLockedActor, WarriorGamePlayTags::Shared_Ability_Death);
	const bool bIsDeadPlayerCharacter = UWarriorFunctionLibrary::NativeDoesActorHaveTag(GetHeroCharacterFromActorInfo(), WarriorGamePlayTags::Shared_Ability_Death);
	
	if (!CurrentLockedActor || bIsDeadEnemyCharacter || bIsDeadPlayerCharacter)
	{
		CancelTargetLockAbility();
		return;
	}
	SetTargetLockWidgetPosition();

	const bool bShouldOverrideRotation =
		!UWarriorFunctionLibrary::NativeDoesActorHaveTag(GetHeroCharacterFromActorInfo(), WarriorGamePlayTags::Player_Status_Rolling)
		&& !UWarriorFunctionLibrary::NativeDoesActorHaveTag(GetHeroCharacterFromActorInfo(), WarriorGamePlayTags::Player_Status_Blocking);

	// 방향 고정
	if (bShouldOverrideRotation)
	{
		const FVector CharacterLocation = GetHeroCharacterFromActorInfo()->GetActorLocation();
		const FVector TargetLocation = CurrentLockedActor->GetActorLocation();
		
		FRotator LookAtRotator = UKismetMathLibrary::FindLookAtRotation(CharacterLocation,TargetLocation);
		LookAtRotator -= FRotator(TargetLockCameraOffsetDistance, 0.f, 0.f);
		
		const FRotator CurrentControlRotator = GetHeroCharacterFromActorInfo()->GetControlRotation();
		const FRotator TargetRotator = FMath::RInterpTo(CurrentControlRotator, LookAtRotator, DeltaTime, TargetLockRotationInterpSpeed);
		
		
		GetHeroControllerFromActorInfo()->SetControlRotation(FRotator(TargetRotator.Pitch, TargetRotator.Yaw, TargetRotator.Roll));
		GetHeroCharacterFromActorInfo()->SetActorRotation(FRotator(0.f, TargetRotator.Yaw, 0.f));
	}
}

void UHeroGameplayAbility_TargetLock::SwitchTarget(const FGameplayTag& InSwitchDirectionTag)
{
	GetAvailableActorsToLock();

	TArray<AActor*> ActorsOnLeft;
	TArray<AActor*> ActorsOnRight;
	AActor* NewTargetToLock = nullptr;
	
	GetAvailableActorsAroundTarget(ActorsOnLeft, ActorsOnRight);

	if (InSwitchDirectionTag == WarriorGamePlayTags::Player_Event_SwitchTarget_Left)
	{
		NewTargetToLock = GetNearestTargetFromAvailableActors(ActorsOnLeft);
	}
	else
	{
		NewTargetToLock = GetNearestTargetFromAvailableActors(ActorsOnRight);
	}

	if (NewTargetToLock)
	{
		CurrentLockedActor = NewTargetToLock;
	}
}

void UHeroGameplayAbility_TargetLock::TryLockOnTarget()
{
	GetAvailableActorsToLock();

	if (AvailableActorsToLock.IsEmpty())
	{
		CancelTargetLockAbility();
		return;
	}
	CurrentLockedActor = GetNearestTargetFromAvailableActors(AvailableActorsToLock);

	if (CurrentLockedActor)
	{
		DrawTargetLockWidget();

		SetTargetLockWidgetPosition();
	}
	else
	{
		CancelTargetLockAbility();
	}
}

void UHeroGameplayAbility_TargetLock::GetAvailableActorsToLock()
{
	AvailableActorsToLock.Empty();
	TArray<FHitResult> BoxTraceHits;
	
	// WorldContextObject: 이 함수를 실행할 월드(레벨)를 식별하기 위한 오브젝트입니다. 보통 자기 자신(this)이나 액터를 넣습니다.
	const UObject* WorldContextObject = GetHeroCharacterFromActorInfo();
	const FVector TraceStart = GetHeroCharacterFromActorInfo()->GetActorLocation();
	const FVector TraceEnd = GetHeroCharacterFromActorInfo()->GetActorLocation() + GetHeroCharacterFromActorInfo()->GetActorForwardVector() * BoxTraceDistance;
	const FVector HalfSize = TraceBoxSize / 2.f;
	const FRotator Orientation = GetHeroCharacterFromActorInfo()->GetActorForwardVector().ToOrientationRotator();
	const bool bTraceComplex = false;
	const TArray<AActor*> ActorsToIgnore = TArray<AActor*>();
	const EDrawDebugTrace::Type DrawDebugType = bShowPersistentDebugShape ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None;
	const bool bIgnoreSelf = true;
	
	UKismetSystemLibrary::BoxTraceMultiForObjects(
		WorldContextObject,
		TraceStart,
		TraceEnd,
		HalfSize,
		Orientation,
		BoxTraceChannel,
		bTraceComplex,
		ActorsToIgnore,
		DrawDebugType,
		BoxTraceHits,
		bIgnoreSelf
	);

	for (const FHitResult& TraceHit : BoxTraceHits)
	{
		if (AActor* HitActor = TraceHit.GetActor())
		{
			if (HitActor != GetHeroCharacterFromActorInfo())
			{
				AvailableActorsToLock.AddUnique(HitActor);
			}
		}
	}
}

AActor* UHeroGameplayAbility_TargetLock::GetNearestTargetFromAvailableActors(const TArray<AActor*>& InAvailableActors)
{
	float ClosestDistance = 0.f;
	return UGameplayStatics::FindNearestActor(GetHeroCharacterFromActorInfo()->GetActorLocation(), InAvailableActors, ClosestDistance);
}

void UHeroGameplayAbility_TargetLock::GetAvailableActorsAroundTarget(TArray<AActor*>& OutActorsOnLeft,
	TArray<AActor*>& OutActorsOnRight)
{
	if (!CurrentLockedActor || AvailableActorsToLock.IsEmpty())
	{
		CancelTargetLockAbility();
		return;
	}

	const FVector PlayerLocation = GetHeroCharacterFromActorInfo()->GetActorLocation();
	const FVector PlayerToCurrentNormalized = (CurrentLockedActor->GetActorLocation() - PlayerLocation).GetSafeNormal();

	for (AActor* AvailableActor : AvailableActorsToLock)
	{
		if (!AvailableActor || AvailableActor == CurrentLockedActor)
		{
			continue;
		}

		const FVector PlayerToAvailableNormalized = (AvailableActor->GetActorLocation() - PlayerLocation).GetSafeNormal();

		const FVector CrossResult = FVector::CrossProduct(PlayerToCurrentNormalized, PlayerToAvailableNormalized);

		if (CrossResult.Z > 0.f)
		{
			OutActorsOnRight.AddUnique(AvailableActor);
		}
		else
		{
			OutActorsOnLeft.AddUnique(AvailableActor);
		}
	}
	
}

void UHeroGameplayAbility_TargetLock::DrawTargetLockWidget()
{
	if (!DrawnTargetLockWidget)
	{
		checkf(TargetLockWidgetClass, TEXT("Blueprint에서 유효한 위젯 클래스를 지정하지 않았습니다."));
		                                                                                                                                       
		DrawnTargetLockWidget = CreateWidget<UWarriorWidgetBase>(GetHeroControllerFromActorInfo(), TargetLockWidgetClass);

		check(DrawnTargetLockWidget)

		DrawnTargetLockWidget->AddToViewport();
	}
	
	
}

void UHeroGameplayAbility_TargetLock::SetTargetLockWidgetPosition()
{
	if (!DrawnTargetLockWidget || !CurrentLockedActor)
	{
		CancelTargetLockAbility();
		return;
	}

	// LockOnTarget
	FVector WorldHeadLocation = CurrentLockedActor->FindComponentByClass<USkeletalMeshComponent>()->GetSocketLocation(TEXT("LockOnTarget")); // 위에 소켓이 설정되어 있을 경우
	FVector2D ScreenPosition;
	
		// CurrentLockedActor->GetActorLocation(),
	UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		GetHeroControllerFromActorInfo(),
		CurrentLockedActor->GetActorLocation(),
		ScreenPosition,
		true
		);

	if (TargetLockWidgetSize == FVector2D::ZeroVector)
	{
		DrawnTargetLockWidget->WidgetTree->ForEachWidget(
			[this](UWidget* FoundWidget)
			{
				if (USizeBox* FoundSizeBox = Cast<USizeBox>(FoundWidget))
					{
					TargetLockWidgetSize.X = FoundSizeBox->GetWidthOverride();
					TargetLockWidgetSize.Y = FoundSizeBox->GetHeightOverride();
					}
			}
		);
	}

	ScreenPosition -= (TargetLockWidgetSize / 2.f);

	DrawnTargetLockWidget->SetPositionInViewport(ScreenPosition, false);
}

void UHeroGameplayAbility_TargetLock::CancelTargetLockAbility()
{
	CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
}

void UHeroGameplayAbility_TargetLock::CleanUp()
{
	AvailableActorsToLock.Empty();
	
	CurrentLockedActor = nullptr;

	if (DrawnTargetLockWidget)
	{
		DrawnTargetLockWidget->RemoveFromParent();
	}

	DrawnTargetLockWidget = nullptr;

	TargetLockWidgetSize = FVector2D::ZeroVector;
	
	CachedDefaultMaxWalkSpeed = 0.f;
}

void UHeroGameplayAbility_TargetLock::InitTargetLockMovement()
{
	CachedDefaultMaxWalkSpeed = GetHeroCharacterFromActorInfo()->GetCharacterMovement()->MaxWalkSpeed;
	GetHeroCharacterFromActorInfo()->GetCharacterMovement()->MaxWalkSpeed = TargetLockMaxWalkSpeed;
}

void UHeroGameplayAbility_TargetLock::ResetTargetLockMovement()
{
	if (CachedDefaultMaxWalkSpeed > 0.f)
	{
		GetHeroCharacterFromActorInfo()->GetCharacterMovement()->MaxWalkSpeed = CachedDefaultMaxWalkSpeed;
	}
}

void UHeroGameplayAbility_TargetLock::InitializeTargetLockInputSubsystem(UEnhancedInputLocalPlayerSubsystem*& Subsystem)
{
	const ULocalPlayer* LocalPlayer = GetHeroControllerFromActorInfo()->GetLocalPlayer();
	Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	check(Subsystem);
}

void UHeroGameplayAbility_TargetLock::InitTargetLockMappingContext()
{
	UCharacterMovementComponent* MoveComponent = GetHeroCharacterFromActorInfo()->GetCharacterMovement();
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem;
	InitializeTargetLockInputSubsystem(Subsystem);
	Subsystem->AddMappingContext(TargetLockMappingContext, 3);
	
	if (MoveComponent && bIsFlipFlopStateA)
	{
		MoveComponent->bOrientRotationToMovement = false;
		bIsFlipFlopStateA = !bIsFlipFlopStateA; // 다음 호출을 위해 상태를 반전 (A -> B, B -> A)
	}
}


void UHeroGameplayAbility_TargetLock::ResetTargetLockMappingContext()
{
	if (!GetHeroControllerFromActorInfo())
	{
		return;
	}

	UCharacterMovementComponent* MoveComponent = GetHeroCharacterFromActorInfo()->GetCharacterMovement();
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem;
	InitializeTargetLockInputSubsystem(Subsystem);
	Subsystem->RemoveMappingContext(TargetLockMappingContext);
	
	if (MoveComponent && !bIsFlipFlopStateA)
	{
		
		MoveComponent->bOrientRotationToMovement = true;
		bIsFlipFlopStateA = !bIsFlipFlopStateA; // 다음 호출을 위해 상태를 반전 (A -> B, B -> A)
	}
}
