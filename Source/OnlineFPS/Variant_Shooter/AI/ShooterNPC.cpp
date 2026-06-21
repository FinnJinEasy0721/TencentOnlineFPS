// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/AI/ShooterNPC.h"
#include "ShooterWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "ShooterGameMode.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

void AShooterNPC::BeginPlay()
{
	Super::BeginPlay();

	// 生成武器
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	Weapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);
}

void AShooterNPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清除死亡定时器
	GetWorld()->GetTimerManager().ClearTimer(DeathTimer);
}

float AShooterNPC::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 如果已死亡则忽略
	if (bIsDead)
	{
		return 0.0f;
	}

	// 扣减血量
	CurrentHP -= Damage;

	// 血量是否耗尽？
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	return Damage;
}

void AShooterNPC::AttachWeaponMeshes(AShooterWeapon* WeaponToAttach)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// 挂载武器Actor
	WeaponToAttach->AttachToActor(this, AttachmentRule);

	// 挂载武器网格体
	WeaponToAttach->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	WeaponToAttach->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
}

void AShooterNPC::PlayFiringMontage(UAnimMontage* Montage)
{
	// 未使用
}

void AShooterNPC::AddWeaponRecoil(float Recoil)
{
	// 未使用
}

void AShooterNPC::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	// 未使用
}

FVector AShooterNPC::GetWeaponTargetLocation()
{
	// 从摄像机位置开始瞄准
	const FVector AimSource = GetFirstPersonCameraComponent()->GetComponentLocation();

	FVector AimDir, AimTarget = FVector::ZeroVector;

	// 是否有瞄准目标？
	if (CurrentAimTarget)
	{
		// 瞄准Actor位置
		AimTarget = CurrentAimTarget->GetActorLocation();

		// 对目标头部/脚部施加垂直偏移
		AimTarget.Z += FMath::RandRange(MinAimOffsetZ, MaxAimOffsetZ);

		// 获取瞄准方向并施加锥形随机偏差
		AimDir = (AimTarget - AimSource).GetSafeNormal();
		AimDir = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(AimDir, AimVarianceHalfAngle);

		
	} else {

		// 无瞄准目标，直接使用摄像机朝向
		AimDir = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(GetFirstPersonCameraComponent()->GetForwardVector(), AimVarianceHalfAngle);

	}

	// 计算无遮挡的瞄准目标位置
	AimTarget = AimSource + (AimDir * AimRange);

	// 执行可见性射线检测是否有遮挡
	FHitResult OutHit;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, AimSource, AimTarget, ECC_Visibility, QueryParams);

	// 返回命中点或射线终点
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterNPC::AddWeaponClass(const TSubclassOf<AShooterWeapon>& InWeaponClass)
{
	// 未使用
}

void AShooterNPC::OnWeaponActivated(AShooterWeapon* InWeapon)
{
	// 未使用
}

void AShooterNPC::OnWeaponDeactivated(AShooterWeapon* InWeapon)
{
	// 未使用
}

void AShooterNPC::OnSemiWeaponRefire()
{
	// 是否仍在射击？
	if (bIsShooting)
	{
		// 开火
		Weapon->StartFiring();
	}
}

void AShooterNPC::Die()
{
	// 如果已死亡则忽略
	if (bIsDead)
	{
		return;
	}

	// 设置死亡标志
	bIsDead = true;

	// 增加队伍分数
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->IncrementTeamScore(TeamByte);
	}

	// 禁用胶囊体碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 停止移动
	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->StopActiveMovement();

	// 在第三人称网格体上启用布娃娃物理
	GetMesh()->SetCollisionProfileName(RagdollCollisionProfile);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetPhysicsBlendWeight(1.0f);

	// 安排Actor销毁
	GetWorld()->GetTimerManager().SetTimer(DeathTimer, this, &AShooterNPC::DeferredDestruction, DeferredDestructionTime, false);
}

void AShooterNPC::DeferredDestruction()
{
	Destroy();
}

void AShooterNPC::StartShooting(AActor* ActorToShoot)
{
	// 保存瞄准目标
	CurrentAimTarget = ActorToShoot;

	// 设置标志
	bIsShooting = true;

	// 通知武器
	Weapon->StartFiring();
}

void AShooterNPC::StopShooting()
{
	// 清除标志
	bIsShooting = false;

	// 通知武器
	Weapon->StopFiring();
}
