// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnExtensionComponentBase.generated.h"

/**
 * UPawnExtensionComponentBase는 Pawn의 확장을 위한 베이스 컴포넌트입니다.
 * 이 컴포넌트는 소유자 Pawn 및 해당 Controller에 안전하게 접근할 수 있는 유틸리티 함수들을 제공합니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class WARRIOR_API UPawnExtensionComponentBase : public UActorComponent
{
	GENERATED_BODY()

protected:
	/**
	 * 현재 이 컴포넌트를 포함하는 액터를 T 타입(APawn 또는 그 자식)으로 캐스팅하여 반환합니다.
	 * 
	 * @tparam T APawn 또는 그 자식 클래스여야 하며, 컴파일 타임에 static_assert로 검사됩니다.
	 * @return 성공 시 T* 타입의 포인터. 실패 시 런타임 크래시(CastChecked).
	 *
	 * @note 내부적으로 CastChecked를 사용하므로, T가 실제로 Owner의 타입과 일치해야 합니다.
	 *
	 * @example
	 * AMyCharacter* Character = GetOwningPawn<AMyCharacter>();
	 */
	template <class T>
	T* GetOwningPawn() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "'T' 템플릿 매개 변수는 APawn에서 파생되어야합니다");
		return CastChecked<T>(GetOwner());
	}

	/**
	 * 현재 이 컴포넌트를 포함하는 액터를 APawn* 타입으로 반환합니다.
	 * @return APawn* 타입의 오너 액터
	 * @note 내부적으로 GetOwningPawn<APawn>()을 호출합니다.
	 */
	APawn* GetOwningPawn() const
	{
		return GetOwningPawn<APawn>();
	}

	/**
	 * 이 컴포넌트의 소유자(Pawn)가 가지고 있는 Controller를 T 타입으로 반환합니다.
	 * @tparam T AController 또는 그 자식 클래스여야 하며, 컴파일 타임에 static_assert로 검사됩니다.
	 * @return 성공 시 T* 타입의 컨트롤러 포인터. 실패 시 런타임 크래시 발생 가능성.
	 * @note 반드시 Pawn 오브젝트가 존재하고, GetController<T>()가 유효해야 합니다.
	 * @example
	 * AMyAIController* Controller = GetOwningController<AMyAIController>();
	 */
	template <class T>
	T* GetOwningController() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AController>::Value, "'T' 템플릿 매개 변수는 AController에서 파생되어야합니다");
		return GetOwningPawn<APawn>()->GetController<T>();
	}
};
