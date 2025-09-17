// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/WarriorHeroCharacter.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "WarriorGamePlayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DataAsset/Input/DataAsset_InputConfig.h"
#include "DataAsset/StartUpData/DataAsset_HeroStartUpData.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
#include "Components/Input/WarriorInputComponent.h"
#include "Components/Combat/HeroCombatComponent.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Components/UI/HeroUIComponent.h"

#include "WarriorDebugHelper.h"
#include "GameModes/WarriorBaseGameMode.h"
#include "WarriorTypes/WarriorEnumTypes.h"

AWarriorHeroCharacter::AWarriorHeroCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 200.f;
	CameraBoom->SocketOffset = FVector(0.f, 55.f, 65.f);
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

	HeroCombatComponent = CreateDefaultSubobject<UHeroCombatComponent>(TEXT("HeroCombatComponent"));
	HeroUIComponent = CreateDefaultSubobject<UHeroUIComponent>(TEXT("HeroUIComponent"));

	bIsFlipFlopStateA = true;

	LandedDelegate.AddDynamic(this, &ThisClass::OnLandedHandler);
}

UPawnCombatComponent* AWarriorHeroCharacter::GetPawnCombatComponent() const
{
	return HeroCombatComponent;
}

UPawnUIComponent* AWarriorHeroCharacter::GetPawnUIComponent() const
{
	return HeroUIComponent;
}

UHeroUIComponent* AWarriorHeroCharacter::GetHeroUIComponent() const
{
	return HeroUIComponent;
}

void AWarriorHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	checkf(PlayerInputComponent, TEXT("유효한 데이터 자산을 입력 구성으로 할당하는 것을 잊었습니다!"));
	ULocalPlayer* LocalPlayer = GetController<APlayerController>()->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	check(Subsystem);

	Subsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext, 0);

	UWarriorInputComponent* WarriorInputComponent = CastChecked<UWarriorInputComponent>(PlayerInputComponent);

	WarriorInputComponent->BindNativeInputAction(
		InputConfigDataAsset, WarriorGamePlayTags::InputTag_Move,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_Move
	);

	WarriorInputComponent->BindNativeInputAction(
		InputConfigDataAsset,
		WarriorGamePlayTags::InputTag_Mouse_Lock,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_Look
	);

	WarriorInputComponent->BindNativeInputAction(
		InputConfigDataAsset,
		WarriorGamePlayTags::InputTag_Crouch,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_Crouch
	);

	WarriorInputComponent->BindNativeInputAction(
		InputConfigDataAsset,
		WarriorGamePlayTags::InputTag_Walk,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_Walk
	);

	WarriorInputComponent->BindNativeInputAction(
		InputConfigDataAsset,
		WarriorGamePlayTags::InputTag_Sprint,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_Sprint
	);

	WarriorInputComponent->BindNativeInputAction(
		InputConfigDataAsset,
		WarriorGamePlayTags::InputTag_Jump,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_Jump
	);

	WarriorInputComponent->BindNativeInputAction(
		InputConfigDataAsset,
		WarriorGamePlayTags::InputTag_OrientRotation,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_OrientRotation
	);

	WarriorInputComponent->BindNativeInputAction(
		InputConfigDataAsset,
		WarriorGamePlayTags::InputTag_SwitchTarget,
		ETriggerEvent::Triggered,
		this,
		&ThisClass::Input_SwitchTargetTriggered
	);

	WarriorInputComponent->BindNativeInputAction(
		InputConfigDataAsset,
		WarriorGamePlayTags::InputTag_SwitchTarget,
		ETriggerEvent::Completed,
		this,
		&ThisClass::Input_SwitchTargetCompleted
	);

	WarriorInputComponent->BindNativeInputAction(
		InputConfigDataAsset,
		WarriorGamePlayTags::InputTag_PickUp_Stones,
		ETriggerEvent::Started,
		this,
		&ThisClass::Input_PickUpStonesStarted
	);
	
	WarriorInputComponent->BindAbilityInputAction(
		InputConfigDataAsset,
		this,
		&ThisClass::Input_AbilityInputPressed,
		&ThisClass::Input_AbilityInputReleased
	);
}

void AWarriorHeroCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AWarriorHeroCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);


	// 현재 이동 모드를 가져옴
	const EMovementMode CurrentMode = GetCharacterMovement()->MovementMode;

	// 이전 모드가 '떨어지는 중'이 아니었는데, 현재 모드가 '떨어지는 중'이라면
	// (즉, 막 떨어지기 시작했다면)
	if (PrevMovementMode != MOVE_Falling && CurrentMode == MOVE_Falling)
	{
		// 점프/낙하 상태로 설정
		bIsJump = true;
	}
}

void AWarriorHeroCharacter::OnLandedHandler(const FHitResult& Hit)
{
	bIsJump = false;
}

void AWarriorHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!CharacterStartUpData.IsNull())
	{
		if (UDataAsset_StartUpDataBase* LoadedData = CharacterStartUpData.LoadSynchronous())
		{	
			int32 AbilityApplyLevel = 1;

			if (AWarriorBaseGameMode* BaseGameMode = GetWorld()->GetAuthGameMode<AWarriorBaseGameMode>())
			{
				switch (BaseGameMode->GetCurrentGameDifficulty())
				{
				case EWarriorGameDifficulty::Easy:
					AbilityApplyLevel = 4;
					Debug::Print(TEXT("현재 난이도: Easy"));
					break;

				case EWarriorGameDifficulty::Normal:
					AbilityApplyLevel = 3;
					Debug::Print(TEXT("현재 난이도: Normal"));
					break;

				case EWarriorGameDifficulty::Hard:
					AbilityApplyLevel = 2;
					Debug::Print(TEXT("현재 난이도: Hard"));
					break;

				case EWarriorGameDifficulty::Hell:
					AbilityApplyLevel = 1;
					Debug::Print(TEXT("현재 난이도: Hell"));
					break;

				default:
					break;
				}
			}
			
			LoadedData->GiveToAbilitySystemComponent(WarriorAbilitySystemComponent,AbilityApplyLevel);
		}
	}
}

void AWarriorHeroCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	const FVector2D MovementVector = InputActionValue.Get<FVector2D>();

	const FRotator MovementRotation = FRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);

	// 사용자 입력이 앞/뒤(Y축)일 경우
	if (MovementVector.Y != 0.f)
	{
		// 카메라 기준의 '앞 방향'을 계산하고
		const FVector ForwardDirection = MovementRotation.RotateVector(FVector::ForwardVector);

		// 그 방향으로 입력 세기를 적용해 이동시킴
		AddMovementInput(ForwardDirection, MovementVector.Y);
	}

	if (MovementVector.X != 0.f)
	{
		const FVector RightDirection = MovementRotation.RotateVector(FVector::RightVector);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AWarriorHeroCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
	const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

	if (LookAxisVector.X != 0.f)
	{
		AddControllerYawInput(LookAxisVector.X);
	}

	if (LookAxisVector.Y != 0.f)
	{
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AWarriorHeroCharacter::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}
	else
	{
		{
			Crouch();
		}
	}
}

void AWarriorHeroCharacter::Input_Walk(const FInputActionValue& InputActionValue)
{
	if (IsInputPressed(InputActionValue))
	{
		bIsWalking = !bIsWalking; // 토글

		if (bIsWalking)
		{
			SetMaxWalkSpeed(200.f); // 걷기
		}
		else
		{
			SetMaxWalkSpeed(550.f); // 달리기
		}
	}
}

void AWarriorHeroCharacter::Input_Sprint(const FInputActionValue& InputActionValue)
{
	if (IsInputPressed(InputActionValue))
	{
		bIsSprint = !bIsSprint;

		if (bIsSprint)
		{
			SetMaxWalkSpeed(600.f); // 스프린트
		}
		else
		{
			SetMaxWalkSpeed(550.f); // 달리기
		}
	}
}

void AWarriorHeroCharacter::Input_Jump(const FInputActionValue& InputActionValue)
{
	if (IsInputPressed(InputActionValue))
	{
		Jump(); // ACharacter의 내장 함수
	}
}

void AWarriorHeroCharacter::Input_OrientRotation(const FInputActionValue& InputActionValue)
{
	UCharacterMovementComponent* MoveComponent = GetCharacterMovement();

	// Orient Rotation To Movement
	// 켜면 (True): 캐릭터가 키보드로 입력한 이동 방향으로 즉시 몸을 돌림
	// 끄면 (False): 캐릭터가 마우스로 조종하는 카메라 방향을 보면서, 이동은 자유롭게 이동함
	if (MoveComponent)
	{
		if (bIsFlipFlopStateA)
		{
			// A 경로: Orient Rotation To Movement off
			MoveComponent->bOrientRotationToMovement = false;
		}
		else
		{
			// B 경로: Orient Rotation To Movement on
			MoveComponent->bOrientRotationToMovement = true;
		}

		// 다음 호출을 위해 상태를 반전 (A -> B, B -> A)
		bIsFlipFlopStateA = !bIsFlipFlopStateA;
	}
}

void AWarriorHeroCharacter::Input_SwitchTargetTriggered(const FInputActionValue& InputActionValue)
{
	SwitchDirection = InputActionValue.Get<FVector2D>();
}

void AWarriorHeroCharacter::Input_SwitchTargetCompleted(const FInputActionValue& InputActionValue)
{
	FGameplayEventData Data;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this,
		SwitchDirection.X > 0.f ? WarriorGamePlayTags::Player_Event_SwitchTarget_Right : WarriorGamePlayTags::Player_Event_SwitchTarget_Left,
		Data
	);
}

void AWarriorHeroCharacter::Input_AbilityInputPressed(FGameplayTag InInputTag)
{
	WarriorAbilitySystemComponent->OnAbilityInputPressed(InInputTag);
}

void AWarriorHeroCharacter::Input_AbilityInputReleased(FGameplayTag InInputTag)
{
	WarriorAbilitySystemComponent->OnAbilityInputReleased(InInputTag);
}

void AWarriorHeroCharacter::Input_PickUpStonesStarted(const FInputActionValue& InputActionValue)
{
	FGameplayEventData Data;
	
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this,
		WarriorGamePlayTags::Player_Event_ConsumeStones,
		Data
	);
}

bool AWarriorHeroCharacter::IsInputPressed(const FInputActionValue& InputActionValue)
{
	return InputActionValue.Get<bool>();
}

void AWarriorHeroCharacter::SetMaxWalkSpeed(const float NewMaxWalkSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewMaxWalkSpeed;
}
