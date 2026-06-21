// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineFPSCharacter.h"
#include "ShooterWeaponHolder.h"
#include "ShooterCharacter.generated.h"

class AShooterWeapon;
class UInputAction;
class UInputComponent;
class UPawnNoiseEmitterComponent;

// 弹药数量变化事件（参数：弹匣容量、当前子弹数）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBulletCountUpdatedDelegate, int32, MagazineSize, int32, Bullets);
// 受伤事件（参数：剩余血量百分比）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDamagedDelegate, float, LifePercent);

/**
 *  可操控的第一人称射击角色
 *  通过 IShooterWeaponHolder 接口管理武器库存
 *  管理血量和死亡逻辑
 */
UCLASS(abstract)
class ONLINEFPS_API AShooterCharacter : public AOnlineFPSCharacter, public IShooterWeaponHolder
{
	GENERATED_BODY()
	
	/** AI 噪声感知组件，用于让AI听到角色的声音 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UPawnNoiseEmitterComponent* PawnNoiseEmitter;

protected:

	/** 开火输入动作 */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* FireAction;

	/** 切换武器输入动作 */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SwitchWeaponAction;

	/** 第一人称网格体上挂载武器的插槽名 */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName FirstPersonWeaponSocket = FName("HandGrip_R");

	/** 第三人称网格体上挂载武器的插槽名 */
	UPROPERTY(EditAnywhere, Category ="Weapons")
	FName ThirdPersonWeaponSocket = FName("HandGrip_R");

	/** 瞄准射线检测的最大距离 */
	UPROPERTY(EditAnywhere, Category ="Aim", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float MaxAimDistance = 10000.0f;

	/** 最大血量 */
	UPROPERTY(EditAnywhere, Category="Health")
	float MaxHP = 500.0f;

	/** 当前血量 */
	float CurrentHP = 0.0f;

	/** 队伍编号 */
	UPROPERTY(EditAnywhere, Category="Team")
	uint8 TeamByte = 0;

	/** 角色已拾取的武器列表 */
	TArray<AShooterWeapon*> OwnedWeapons;

	/** 当前装备中的武器 */
	TObjectPtr<AShooterWeapon> CurrentWeapon;

	/** 死亡后多久重生（秒） */
	UPROPERTY(EditAnywhere, Category ="Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RespawnTime = 5.0f;

	/** 最后对角色造成伤害的控制器（用弱指针，避免对方退出后悬空） */
	TWeakObjectPtr<AController> LastDamageInstigator;

	FTimerHandle RespawnTimer;

public:

	/** 弹药数量变化事件 */
	FBulletCountUpdatedDelegate OnBulletCountUpdated;

	/** 受伤事件 */
	FDamagedDelegate OnDamaged;

public:

	/** 构造函数 */
	AShooterCharacter();

protected:

	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 游戏清理 */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** 设置输入绑定 */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:

	/** 处理受到伤害 */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** 获取队伍编号 */
	UFUNCTION(BlueprintPure, Category="Team")
	uint8 GetTeamByte() const { return TeamByte; }

public:

	/** 开始开火 */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStartFiring();

	/** 停止开火 */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoStopFiring();

	/** 切换武器 */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoSwitchWeapon();

public:

	//~开始 IShooterWeaponHolder 接口

	/** 把武器的网格体挂到角色身上 */
	virtual void AttachWeaponMeshes(AShooterWeapon* Weapon) override;

	/** 播放开火动画蒙太奇 */
	virtual void PlayFiringMontage(UAnimMontage* Montage) override;

	/** 给角色施加后坐力 */
	virtual void AddWeaponRecoil(float Recoil) override;

	/** 更新HUD上的弹药数量 */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) override;

	/** 计算并返回武器的瞄准目标位置 */
	virtual FVector GetWeaponTargetLocation() override;

	/** 给角色添加一把指定类型的武器 */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass) override;

	/** 激活某把武器 */
	virtual void OnWeaponActivated(AShooterWeapon* Weapon) override;

	/** 停用某把武器 */
	virtual void OnWeaponDeactivated(AShooterWeapon* Weapon) override;

	/** 通知角色半自动武器的冷却已结束，可以再次射击 */
	virtual void OnSemiWeaponRefire() override;

	//~结束 IShooterWeaponHolder 接口

protected:

	/** 检查角色是否已经拥有指定类型的武器 */
	AShooterWeapon* FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const;

	/** 血量归零时调用 */
	void Die();

	/** 让蓝图代码响应角色死亡 */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "On Death"))
	void BP_OnDeath();

	/** 重生定时器回调，销毁角色让玩家控制器重新生成 */
	void OnRespawn();
};
