// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineFPSCharacter.h"
#include "ShooterWeaponHolder.h"
#include "ShooterNPC.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPawnDeathDelegate);

class AShooterWeapon;

/**
 *  简单的AI控制射击游戏NPC
 *  通过AI控制器管理的状态树执行行为
 *  持有并管理武器
 */
UCLASS(abstract)
class ONLINEFPS_API AShooterNPC : public AOnlineFPSCharacter, public IShooterWeaponHolder
{
	GENERATED_BODY()

public:

	/** 此角色的当前血量。受到伤害降为零时死亡 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	float CurrentHP = 100.0f;

protected:

	/** 死亡布娃娃状态使用的碰撞配置名 */
	UPROPERTY(EditAnywhere, Category="Damage")
	FName RagdollCollisionProfile = FName("Ragdoll");

	/** 死亡后销毁此Actor前的等待时间 */
	UPROPERTY(EditAnywhere, Category="Damage")
	float DeferredDestructionTime = 5.0f;

	/** 此角色的队伍编号 */
	UPROPERTY(EditAnywhere, Category="Team")
	uint8 TeamByte = 1;

	/** 已装备武器的指针 */
	TObjectPtr<AShooterWeapon> Weapon;

	/** 为此角色生成的武器类型 */
	UPROPERTY(EditAnywhere, Category="Weapon")
	TSubclassOf<AShooterWeapon> WeaponClass;

	/** 第一人称网格体武器插槽名 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** 第三人称网格体武器插槽名 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** 瞄准计算的最大距离 */
	UPROPERTY(EditAnywhere, Category="Aim")
	float AimRange = 10000.0f;

	/** 瞄准时施加的锥形偏差 */
	UPROPERTY(EditAnywhere, Category="Aim")
	float AimVarianceHalfAngle = 10.0f;

	/** 瞄准时目标中心的最小垂直偏移 */
	UPROPERTY(EditAnywhere, Category="Aim")
	float MinAimOffsetZ = -35.0f;

	/** 瞄准时目标中心的最大垂直偏移 */
	UPROPERTY(EditAnywhere, Category="Aim")
	float MaxAimOffsetZ = -60.0f;

	/** 当前瞄准的Actor */
	TObjectPtr<AActor> CurrentAimTarget;

	/** 为true时，此角色正在射击 */
	bool bIsShooting = false;

	/** 为true时，此角色已经死亡 */
	bool bIsDead = false;

	/** 死亡延迟销毁定时器 */
	FTimerHandle DeathTimer;

public:

	/** 此NPC死亡时调用的委托 */
	FPawnDeathDelegate OnPawnDeath;

protected:

	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 游戏清理 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:

	/** 处理受到的伤害 */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:

	//~Begin IShooterWeaponHolder interface

	/** 将武器网格体挂载到持有者身上 */
	virtual void AttachWeaponMeshes(AShooterWeapon* Weapon) override;

	/** 播放武器开火动画蒙太奇 */
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;

	/** 给持有者施加后坐力 */
	virtual void AddWeaponRecoil(float Recoil) override;

	/** 用当前弹药数更新武器HUD */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;

	/** 计算并返回武器的瞄准位置 */
	virtual FVector GetWeaponTargetLocation() override;

	/** 将此类型的武器给予持有者 */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass) override;

	/** 激活指定的武器 */
	virtual void OnWeaponActivated(AShooterWeapon* Weapon) override;

	/** 停用指定的武器 */
	virtual void OnWeaponDeactivated(AShooterWeapon* Weapon) override;

	/** 通知持有者武器冷却已结束，可以再次射击 */
	virtual void OnSemiWeaponRefire() override;

	//~End IShooterWeaponHolder interface

protected:

	/** 血量耗尽且角色应死亡时调用 */
	void Die();

	/** 死亡后销毁Actor时调用 */
	void DeferredDestruction();

public:

	/** 通知此角色开始向指定Actor射击 */
	void StartShooting(AActor* ActorToShoot);

	/** 通知此角色停止射击 */
	void StopShooting();
};
