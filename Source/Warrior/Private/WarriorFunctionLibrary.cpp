// Fill out your copyright notice in the Description page of Project Settings.

#include "WarriorFunctionLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "Interfaces/PawnCombatInterface.h"
#include "GenericTeamAgentInterface.h"
#include "WarriorGamePlayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "WarriorTypes/WarriorCountDownAction.h"
#include "WarriorGameInstance.h"

#include "WarriorDebugHelper.h"
#include "Kismet/GameplayStatics.h"
#include "SaveGame/WarriorSaveGame.h"

/**
 * 지정된 액터에서 WarriorAbilitySystemComponent를 검색하고 반환합니다.
 *
 * 이 함수는 액터와 연결된 AbilitySystemComponent를 가져온 뒤,
 * 이를 WarriorAbilitySystemComponent로 안전하게 캐스팅합니다.
 *
 * @param InActor AbilitySystemComponent를 검색할 대상 액터.
 * @return WarriorAbilitySystemComponent. 주어진 액터에 연결된 WarriorAbilitySystemComponent가 없을 경우, 이 함수는 강제 캐스팅 실패로 인해 실행이 중단됩니다.
 *
 * @note 함수 호출 시 유효한 AActor가 전달되어야 합니다. 전달된 액터가 null일 경우 실행은 중단됩니다.
 */
UWarriorAbilitySystemComponent* UWarriorFunctionLibrary::NativeGetWarriorASCFromActor(AActor* InActor)
{
	check(InActor);

	return CastChecked<UWarriorAbilitySystemComponent>(
		UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InActor)
	);
}

/**
 * 지정된 액터에 해당 게임플레이 태그가 없을 경우, 느슨한(loose) 게임플레이 태그를 추가합니다.
 *
 * 이 함수는 중복 태그 추가를 방지하기 위한 헬퍼 함수입니다.
 * 액터의 Warrior Ability System Component(ASC)를 가져와서
 * 주어진 태그가 이미 존재하는지 확인한 뒤, 존재하지 않으면 태그를 추가합니다.
 *
 * @param InActor    게임플레이 태그를 추가할 대상 액터.
 * @param TagToAdd   대상 액터에 추가하려는 게임플레이 태그.
 *
 * @note 대상 액터에는 반드시 유효한 UWarriorAbilitySystemComponent가 존재해야 합니다.
 */
void UWarriorFunctionLibrary::AddGameplayTagToActorIfNone(AActor* InActor, FGameplayTag TagToAdd)
{
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);
	if (!ASC->HasMatchingGameplayTag(TagToAdd))
	{
		ASC->AddLooseGameplayTag(TagToAdd);
	}
}

void UWarriorFunctionLibrary::RemoveGameplayTagFromActorIfFound(AActor* InActor, FGameplayTag TagToRemove)
 {
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);
	if (ASC->HasMatchingGameplayTag(TagToRemove))
	{
		ASC->RemoveLooseGameplayTag(TagToRemove);
	}
}

bool UWarriorFunctionLibrary::NativeDoesActorHaveTag(AActor* InActor, FGameplayTag TagToCheck)
{
	UWarriorAbilitySystemComponent* ASC = NativeGetWarriorASCFromActor(InActor);

	return ASC->HasMatchingGameplayTag(TagToCheck);
}

void UWarriorFunctionLibrary::BP_DoesActorHaveTag(
	AActor* InActor,
	FGameplayTag TagToCheck,
	EWarriorConfirmType& OutConfirmType
)
{
	OutConfirmType = NativeDoesActorHaveTag(InActor, TagToCheck)
		                 ? EWarriorConfirmType::EWC_Yes
		                 : EWarriorConfirmType::EWC_No;
}

UPawnCombatComponent* UWarriorFunctionLibrary::NativeGetPawnCombatComponentFromActor(AActor* InActor)
{
	check(InActor);

	if (const IPawnCombatInterface* PawnCombatInterface = Cast<IPawnCombatInterface>(InActor))
	{
		return PawnCombatInterface->GetPawnCombatComponent();
	}

	return nullptr;
}

UPawnCombatComponent* UWarriorFunctionLibrary::BP_GetPawnCombatComponentFromActor(AActor* InActor,
	EWarriorValidType& OutValidType)
{
	UPawnCombatComponent* CombatComponent = NativeGetPawnCombatComponentFromActor(InActor);
	OutValidType = CombatComponent ? EWarriorValidType::EWT_Valid : EWarriorValidType::EWT_Invalid;
	return CombatComponent;
}

/**
 * 두 Pawn 간의 팀을 비교하여 적대 관계인지 여부를 판단합니다.
 * 
 * @param QueryPawn 판단의 기준이 되는 Pawn (예: AI 또는 플레이어 본인)
 * @param TargetPawn 적대 여부를 판단할 대상 Pawn
 * @return 두 Pawn이 서로 다른 TeamId를 가지고 있으면 true (적대 관계), 그렇지 않으면 false
 *
 * @note 두 Pawn의 컨트롤러는 IGenericTeamAgentInterface를 구현하고 있어야 하며,
 *       구현되어 있지 않거나 null일 경우 false를 반환합니다.
 */
bool UWarriorFunctionLibrary::IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn)
{
	check(QueryPawn && TargetPawn);

	IGenericTeamAgentInterface* GenericTeamAgent = Cast<IGenericTeamAgentInterface>(QueryPawn->GetController());
	IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(TargetPawn->GetController());

	if (GenericTeamAgent && TargetTeamAgent)
	{
		// 팀 ID가 다르면 적대 관계로 간주하여 true 반환
		return GenericTeamAgent->GetGenericTeamId() != TargetTeamAgent->GetGenericTeamId();
	}

	// 팀 인터페이스를 구현하지 않은 경우 적대 관계로 간주하지 않음
	return false;
}

float UWarriorFunctionLibrary::GetScalableFloatValueAtLevel(const FScalableFloat& InScalableFloat, float InLevel)
{
	return InScalableFloat.GetValueAtLevel(InLevel);
}

FGameplayTag UWarriorFunctionLibrary::ComputeHitReactDirectionTag(
	AActor* InAttacker,
	AActor* InVictim,
	float& OutAngleDifference
)
{
	// InAttacker와 InVictim이 nullptr이 아니어야 함 (디버깅 중 예외 처리)
	check(InAttacker && InVictim);

	// 피격자의 정면 방향 벡터를 가져옴
	const FVector VictimForward = InVictim->GetActorForwardVector();

	// 피격자 → 공격자 방향 벡터를 정규화하여 구함
	const FVector VictimToAttackerNormalized = (InAttacker->GetActorLocation() - InVictim->GetActorLocation()).
		GetSafeNormal();

	// 두 벡터의 내적 결과 (코사인 값)를 구함
	const float DotResult = FVector::DotProduct(VictimForward, VictimToAttackerNormalized);

	// acos 결과를 도(degree) 단위로 변환하여 각도 차이 저장
	OutAngleDifference = UKismetMathLibrary::DegAcos(DotResult);

	// 좌우 방향 판별을 위한 외적 계산
	const FVector CrossResult = FVector::CrossProduct(VictimForward, VictimToAttackerNormalized);

	// 외적 결과의 Z 값이 음수이면 오른쪽에서 공격 -> 각도 부호를 음수로 바꿈
	if (CrossResult.Z < 0.f)
	{
		OutAngleDifference *= -1.f;
	}
	return DetermineHitReactionTag(OutAngleDifference);
}

bool UWarriorFunctionLibrary::IsValidBlock(AActor* InAttacker, AActor* InDefender)
{
	check(InAttacker && InDefender)

	const float DotResult = FVector::DotProduct(InAttacker->GetActorForwardVector(), InDefender->GetActorForwardVector());

	// const FString DebugDotResultString = FString::Printf(TEXT("Dot Result: %f %s"), DotResult, DotResult < 0.f ? TEXT("valid block") : TEXT("InvalidBlock"));
	// Debug::Print(DebugDotResultString, DotResult < 0.f ? FColor::Green : FColor::Red);

	return DotResult < 0.f;
}

/**
 * Instigator가 Target Actor에게 주어진 FGameplayEffectSpecHandle을 기반으로
 * Gameplay Effect를 적용하려 시도합니다.
 *
 * @param InInstigator Effect를 적용하려는 주체 Actor
 * @param InTargetActor Effect를 적용받는 대상 Actor
 * @param InSpecHandle 적용될 FGameplayEffectSpecHandle
 * @return Effect가 성공적으로 적용되었으면 true, 그렇지 않으면 false
 *
 * @note InInstigator와 InTargetActor는 모두 유효한 UWarriorAbilitySystemComponent를 가져야 합니다.
 *       UWarriorAbilitySystemComponent를 얻기 위한 내부 호출에서 nullptr이 반환되거나,
 *       FGameplayEffectSpecHandle이 유효하지 않을 경우 false를 반환합니다.
 */
bool UWarriorFunctionLibrary::ApplyGameplayEffectSpecHandleToTargetActor(AActor* InInstigator, AActor* InTargetActor,
                                                                         const FGameplayEffectSpecHandle& InSpecHandle)
{
	UWarriorAbilitySystemComponent* SourceASC = NativeGetWarriorASCFromActor(InInstigator);
	UWarriorAbilitySystemComponent* TargetASC = NativeGetWarriorASCFromActor(InTargetActor);

	FActiveGameplayEffectHandle ActiveGameplayEffectHandle = SourceASC->ApplyGameplayEffectSpecToTarget(*InSpecHandle.Data, TargetASC);

	return ActiveGameplayEffectHandle.WasSuccessfullyApplied();
}

FGameplayTag UWarriorFunctionLibrary::DetermineHitReactionTag(const float& OutAngleDifference)
{
	// -45 ~ 45도 사이면 정면
	if (OutAngleDifference >= -45.f && OutAngleDifference <= 45.f)
	{
		return WarriorGamePlayTags::Shared_Status_HitReact_Front;
	}
	// -135 ~ -45도 사이면 왼쪽
	if (OutAngleDifference < -45.f && OutAngleDifference >= -135.f)
	{
		return WarriorGamePlayTags::Shared_Status_HitReact_Left;
	}
	// - 135보다 작거나 135보다 크면 오른쪽
	if (OutAngleDifference < -135.f || OutAngleDifference > 135.f)
	{
		return WarriorGamePlayTags::Shared_Status_HitReact_Back;
	}
	// 45 ~ 135도 사이면 뒤
	if (OutAngleDifference > 45.f && OutAngleDifference <= 135.f)
	{
		return WarriorGamePlayTags::Shared_Status_HitReact_Right;
	}
	return WarriorGamePlayTags::Shared_Status_HitReact_Front;
}

void UWarriorFunctionLibrary::CountDown(
	const UObject* WorldContextObject,
	float TotalTime,
	float UpdateInterval,
	float& OutRemainingTime,
	EWarriorCountDownActionInput CountDownInput,
	UPARAM(DisplayName = "Output") EWarriorCountDownActionOutput& CountDownOutput,
	FLatentActionInfo LatentInfo
)
{
	UWorld* World = nullptr;

	if (GEngine)
	{
		World = GEngine->GetWorldFromContextObject(WorldContextObject,EGetWorldErrorMode::LogAndReturnNull);
	}

	if (!World)
	{
		return;
	}

	FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

	FWarriorCountDownAction* FoundAction = LatentActionManager.FindExistingAction<FWarriorCountDownAction>(LatentInfo.CallbackTarget,LatentInfo.UUID);

	if (!FoundAction && CountDownInput == EWarriorCountDownActionInput::Start)
	{
		LatentActionManager.AddNewAction(
			LatentInfo.CallbackTarget,
			LatentInfo.UUID,
			new FWarriorCountDownAction(TotalTime,UpdateInterval,OutRemainingTime,CountDownOutput,LatentInfo)
		);
	}

	if (FoundAction && CountDownInput == EWarriorCountDownActionInput::Cancel)
	{
		FoundAction->CancelAction();
	}
}

UWarriorGameInstance* UWarriorFunctionLibrary::GetWarriorGameInstance(const UObject* WorldContextObject)
{
	if (GEngine)
	{
		// 로그를 남기고 nullptr 반환
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			return World->GetGameInstance<UWarriorGameInstance>();
		}
	}

	return nullptr;
}

void UWarriorFunctionLibrary::ToggleInputMode(const UObject* WorldContextObject, EWarriorInputMode InputMode)
{
	APlayerController* PlayerController = nullptr;

	if (GEngine)
	{
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			PlayerController = World->GetFirstPlayerController();
		}
	}

	if (!PlayerController)
	{
		return;
	}

	FInputModeGameOnly GameOnlyMode;
	FInputModeUIOnly UIOnlyMOde;

	switch (InputMode)
	{
	case EWarriorInputMode::GameOnly:
		PlayerController->SetInputMode(GameOnlyMode);
		PlayerController->bShowMouseCursor = false;
		break;
		
	case EWarriorInputMode::UIOnly:
		PlayerController->SetInputMode(UIOnlyMOde);
		PlayerController->bShowMouseCursor = true;
		break;

	default:
		break;
	}
}

void UWarriorFunctionLibrary::SaveCurrentGameDifficulty(EWarriorGameDifficulty InDifficultyToSave)
{
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(UWarriorSaveGame::StaticClass());

	if (UWarriorSaveGame* WarriorSaveGameObject = Cast<UWarriorSaveGame>(SaveGameObject))
	{
		WarriorSaveGameObject->SavedCurrentGameDifficulty = InDifficultyToSave;

		const bool bWasSaved = UGameplayStatics::SaveGameToSlot(WarriorSaveGameObject, WarriorGamePlayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0);

		Debug::Print(bWasSaved ? TEXT("난이도 저장") : TEXT("난이도 저장 안됨"));
	}
}

bool UWarriorFunctionLibrary::TryLoadSavedGameDifficulty(EWarriorGameDifficulty& OutSavedDifficulty)
{
	if (UGameplayStatics::DoesSaveGameExist(WarriorGamePlayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0))
	{
		USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(WarriorGamePlayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0);
		if (const UWarriorSaveGame* WarriorSaveGameObject = Cast<UWarriorSaveGame>(SaveGameObject))
		{
			OutSavedDifficulty = WarriorSaveGameObject->SavedCurrentGameDifficulty;

			Debug::Print(TEXT("로딩 성공"), FColor::Green);

			return true;
		}
	}

	return false;
}
