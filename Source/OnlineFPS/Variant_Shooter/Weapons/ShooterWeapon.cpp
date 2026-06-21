// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "ShooterProjectile.h"
#include "ShooterWeaponHolder.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"

AShooterWeapon::AShooterWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 创建第一人称网格体
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));
	FirstPersonMesh->SetupAttachment(RootComponent);

	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
	FirstPersonMesh->bOnlyOwnerSee = true;

	// 创建第三人称网格体
	ThirdPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Mesh"));
	ThirdPersonMesh->SetupAttachment(RootComponent);

	ThirdPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	ThirdPersonMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
	ThirdPersonMesh->bOwnerNoSee = true;
}

void AShooterWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 如果没有Owner（比如直接放在场景里），就不要继续初始化
	AActor* WeaponOwnerActor = GetOwner();
	if (!WeaponOwnerActor)
	{
		return;
	}

	// 订阅Owner的销毁事件
	WeaponOwnerActor->OnDestroyed.AddDynamic(this, &AShooterWeapon::OnOwnerDestroyed);

	// 获取武器持有者接口和Pawn
	WeaponOwner = Cast<IShooterWeaponHolder>(WeaponOwnerActor);
	PawnOwner = Cast<APawn>(WeaponOwnerActor);

	// 填满弹匣
	CurrentBullets = MagazineSize;

	// 把武器网格体挂到持有者身上
	WeaponOwner->AttachWeaponMeshes(this);
}

void AShooterWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清理连发定时器
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

void AShooterWeapon::OnOwnerDestroyed(AActor* DestroyedActor)
{
	// 持有者销毁时，武器也一起销毁
	Destroy();
}

void AShooterWeapon::ActivateWeapon()
{
	// 显示武器
	SetActorHiddenInGame(false);

	// 通知持有者
	WeaponOwner->OnWeaponActivated(this);
}

void AShooterWeapon::DeactivateWeapon()
{
	// 停用武器时先停止开火
	StopFiring();

	// 隐藏武器
	SetActorHiddenInGame(true);

	// 通知持有者
	WeaponOwner->OnWeaponDeactivated(this);
}

void AShooterWeapon::StartFiring()
{
	// 设置开火标志
	bIsFiring = true;

	// 检查距离上次射击过了多久
	// 如果射击间隔较短且玩家连续按扳机，可能还没到射击间隔
	const float TimeSinceLastShot = GetWorld()->GetTimeSeconds() - TimeOfLastShot;

	if (TimeSinceLastShot > RefireRate)
	{
		// 直接收击
		Fire();

	} else {

		// 全自动模式下，安排下一次射击
		if (bFullAuto)
		{
			GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, TimeSinceLastShot, false);
		}

	}
}

void AShooterWeapon::StopFiring()
{
	// 取消开火标志
	bIsFiring = false;

	// 清理连发定时器
	GetWorld()->GetTimerManager().ClearTimer(RefireTimer);
}

void AShooterWeapon::Fire()
{
	// 确认玩家还在按扳机，可能已经松手了
	if (!bIsFiring)
	{
		return;
	}
	
	// 向目标位置发射投射物
	FireProjectile(WeaponOwner->GetWeaponTargetLocation());

	// 更新上次射击时间
	TimeOfLastShot = GetWorld()->GetTimeSeconds();

	// 制造噪声，让AI感知系统能听到
	MakeNoise(ShotLoudness, PawnOwner, PawnOwner->GetActorLocation(), ShotNoiseRange, ShotNoiseTag);

	// 全自动？
	if (bFullAuto)
	{
		// 安排下一次射击
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::Fire, RefireRate, false);
	} else {

		// 半自动武器，安排冷却结束通知
		GetWorld()->GetTimerManager().SetTimer(RefireTimer, this, &AShooterWeapon::FireCooldownExpired, RefireRate, false);

	}
}

void AShooterWeapon::FireCooldownExpired()
{
	// 通知持有者可以再次射击了
	WeaponOwner->OnSemiWeaponRefire();
}

void AShooterWeapon::FireProjectile(const FVector& TargetLocation)
{
	// 获取投射物的生成变换
	FTransform ProjectileTransform = CalculateProjectileSpawnTransform(TargetLocation);
	
	// 生成投射物
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = PawnOwner;

	AShooterProjectile* Projectile = GetWorld()->SpawnActor<AShooterProjectile>(ProjectileClass, ProjectileTransform, SpawnParams);

	// 播放开火动画
	WeaponOwner->PlayFiringMontage(FiringMontage);

	// 施加后坐力
	WeaponOwner->AddWeaponRecoil(FiringRecoil);

	// 消耗一发子弹
	--CurrentBullets;

	// 弹匣空了就换弹
	if (CurrentBullets <= 0)
	{
		CurrentBullets = MagazineSize;
	}

	// 更新弹药HUD
	WeaponOwner->UpdateWeaponHUD(CurrentBullets, MagazineSize);
}

FTransform AShooterWeapon::CalculateProjectileSpawnTransform(const FVector& TargetLocation) const
{
	// 获取枪口位置
	const FVector MuzzleLoc = FirstPersonMesh->GetSocketLocation(MuzzleSocketName);

	// 在枪口前方计算生成位置
	const FVector SpawnLoc = MuzzleLoc + ((TargetLocation - MuzzleLoc).GetSafeNormal() * MuzzleOffset);

	// 计算瞄准旋转，同时施加散射偏差
	const FRotator AimRot = UKismetMathLibrary::FindLookAtRotation(SpawnLoc, TargetLocation + (UKismetMathLibrary::RandomUnitVector() * AimVariance));

	// 返回构建好的变换
	return FTransform(AimRot, SpawnLoc, FVector::OneVector);
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetFirstPersonAnimInstanceClass() const
{
	return FirstPersonAnimInstanceClass;
}

const TSubclassOf<UAnimInstance>& AShooterWeapon::GetThirdPersonAnimInstanceClass() const
{
	return ThirdPersonAnimInstanceClass;
}
