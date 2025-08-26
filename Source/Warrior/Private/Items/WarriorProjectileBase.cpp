 // Fill out your copyright notice in the Description page of Project Settings.


#include "Items/WarriorProjectileBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "WarriorDebugHelper.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGamePlayTags.h"
#include "GameFramework/ProjectileMovementComponent.h"

AWarriorProjectileBase::AWarriorProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectileCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ProjectileCollisionBox"));
	SetRootComponent(ProjectileCollisionBox);
	ProjectileCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);// 충돌 감지를 위한 설정 (물리 충돌 없이 오버랩/라인 트레이스 등만 허용)
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block); // 플레이어나 AI 캐릭터(Pawn)와 충돌 시 반응 (예: 데미지 처리)
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block); // 움직이는 오브젝트(예: 떨어지는 박스, 애니메이션으로 움직이는 문 등)와 충돌 시 반응
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block); // 고정된 맵 지형이나 벽 등 정적인 오브젝트와 충돌 시 반응
	ProjectileCollisionBox->OnComponentHit.AddUniqueDynamic(this, &ThisClass::AWarriorProjectileBase::OnProjectileHit); // 다른 액터 또는 컴포넌트와 'Block' 충돌이 발생했을 때(OnHit 이벤트)
	ProjectileCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::AWarriorProjectileBase::OnProjectileBeginOverlap); // 다른 액터 또는 컴포넌트와 'Overlap' 겹침이 시작될 때(OnBeginOverlap 이벤트)
	
	ProjectileNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraComponent"));
	ProjectileNiagaraComponent->SetupAttachment(GetRootComponent());
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 700.f; // 투사체의 초기 속도 설정 (시작 시 700 유닛/초로 발사됨)
	ProjectileMovementComponent->MaxSpeed = 1000.f; // 투사체가 도달할 수 있는 최대 속도 설정 (속도가 가속되더라도 1000 유닛/초를 넘지 않음)
	ProjectileMovementComponent->Velocity = FVector(1.f, 0.f, 0.f); // 초기 방향 및 속도 벡터 설정 (X축 방향으로 이동하도록 설정됨)
	ProjectileMovementComponent->ProjectileGravityScale = 0.f; // 중력 영향을 받지 않도록 설정 (투사체가 곡선이 아닌 직선으로 날아감)
	
	InitialLifeSpan = 4.f; // 투사체의 생존 시간 설정 (4초 후 자동으로 파괴됨)

}

void AWarriorProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// 발사체가 대상 폰 과 충돌시 콜리전을 오버랩으로 변경
	if (ProjectileDamagePolicy == EProjectileDamagePolicy::EPP_OnBeginOverlap)
	{
		ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
}

void AWarriorProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	BP_OnSpawnProjectileHiFX(Hit.ImpactPoint);
	
	APawn* HitPawn = Cast<APawn>(OtherActor);
	
	if (!HitPawn || !UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitPawn))
	{
		Destroy();
		return;
	}

	bool bisValidBLock = false;
	const bool bIsPlayerBlocking = UWarriorFunctionLibrary::NativeDoesActorHaveTag(HitPawn, WarriorGamePlayTags::Player_Status_Blocking);
	const bool bIsMyAttackUnblockable = UWarriorFunctionLibrary::NativeDoesActorHaveTag(GetInstigator(), WarriorGamePlayTags::Enemy_Status_Unblockable);

	if (bIsPlayerBlocking && !bIsMyAttackUnblockable)
	{
		bisValidBLock = UWarriorFunctionLibrary::IsValidBlock(this, HitPawn);
	}

	FGameplayEventData Data;
	Data.Instigator = this;
	Data.Target = HitPawn;
	
	if (bisValidBLock)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			HitPawn,
			WarriorGamePlayTags::Player_Event_SuccessfulBlock,
			Data
		);
	}
	else
	{
		HandleApplyProjectileDamage(HitPawn, Data);
	}

	Destroy();
}

void AWarriorProjectileBase::OnProjectileBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!IsValid(OtherActor) || OtherActor == this)
	{
		return;
	}

	// 이미 같은 액터가 들어있으면 스킵 (WeakPtr 비교는 Get()으로 실제 포인터 비교)
	const bool bAlreadyAdded = OverlapActors.ContainsByPredicate(
	[OtherActor](const TWeakObjectPtr<AActor>& Ptr)
	{
		return Ptr.Get() == OtherActor;
	});
	
	if (bAlreadyAdded)
	{
		return;
	}
	
	OverlapActors.Add(TWeakObjectPtr<AActor>(OtherActor));

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		FGameplayEventData Data;
		Data.Instigator = GetInstigator();
		Data.Target     = HitPawn;

		if (UWarriorFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitPawn))
		{
			HandleApplyProjectileDamage(HitPawn, Data);
		}
	}

	
	// 가끔 무효 포인터 청소
	OverlapActors.RemoveAllSwap([](const TWeakObjectPtr<AActor>& Pointer){ return !Pointer.IsValid(); });
}

void AWarriorProjectileBase::HandleApplyProjectileDamage(APawn* InHitPawn, const FGameplayEventData& InPayload)
{
	checkf(ProjectileDamageEffectSpecHandle.IsValid(), TEXT("유효한 spec handle이 Projectile에 할당되지 않았습니다: %s"), *GetActorNameOrLabel())
	const bool bWasApplied = UWarriorFunctionLibrary::ApplyGameplayEffectSpecHandleToTargetActor(GetInstigator(), InHitPawn, ProjectileDamageEffectSpecHandle);

	if (bWasApplied)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			InHitPawn,
			WarriorGamePlayTags::Shared_Event_HitReact,
			InPayload
		);
	}
}

