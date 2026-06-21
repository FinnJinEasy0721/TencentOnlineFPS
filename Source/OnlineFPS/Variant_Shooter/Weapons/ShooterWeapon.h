// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterWeaponHolder.h"
#include "Animation/AnimInstance.h"
#include "ShooterWeapon.generated.h"

class IShooterWeaponHolder;
class AShooterProjectile;
class USkeletalMeshComponent;
class UAnimMontage;
class UAnimInstance;

/**
 *  射手游戏武器基类
 *  提供第一人称和第三人称视角的网格体
 *  处理弹药和开火逻辑
 *  通过 ShooterWeaponHolder 接口与武器持有者交互
 */
UCLASS(abstract)
class ONLINEFPS_API AShooterWeapon : public AActor
{
	GENERATED_BODY()
	
	/** 第一人称视角网格体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** 第三人称视角网格体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ThirdPersonMesh;

protected:

	/** 武器持有者的接口指针 */
	IShooterWeaponHolder* WeaponOwner;

	/** 武器发射的投射物类型 */
	UPROPERTY(EditAnywhere, Category="Ammo")
	TSubclassOf<AShooterProjectile> ProjectileClass;

	/** 弹匣容量 */
	UPROPERTY(EditAnywhere, Category="Ammo", meta = (ClampMin = 0, ClampMax = 100))
	int32 MagazineSize = 10;

	/** 当前弹匣中的子弹数 */
	int32 CurrentBullets = 0;
	
	/** 开火时播放的动画蒙太奇 */
	UPROPERTY(EditAnywhere, Category="Animation")
	UAnimMontage* FiringMontage;

	/** 当前武器激活时，第一人称网格体使用的动画实例类 */
	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> FirstPersonAnimInstanceClass;

	/** 当前武器激活时，第三人称网格体使用的动画实例类 */
	UPROPERTY(EditAnywhere, Category="Animation")
	TSubclassOf<UAnimInstance> ThirdPersonAnimInstanceClass;

	/** 瞄准时散射的半锥角 */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 90, Units = "Degrees"))
	float AimVariance = 0.0f;

	/** 给持有者施加的后坐力大小 */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 100))
	float FiringRecoil = 0.0f;

	/** 第一人称枪口插槽名，投射物从这里生成 */
	UPROPERTY(EditAnywhere, Category="Aim")
	FName MuzzleSocketName;

	/** 投射物在枪口前方的生成距离 */
	UPROPERTY(EditAnywhere, Category="Aim", meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
	float MuzzleOffset = 10.0f;

	/** 是否为全自动开火 */
	UPROPERTY(EditAnywhere, Category="Refire")
	bool bFullAuto = false;

	/** 两次射击之间的间隔时间（影响全自动和半自动） */
	UPROPERTY(EditAnywhere, Category="Refire", meta = (ClampMin = 0, ClampMax = 5, Units = "s"))
	float RefireRate = 0.5f;

	/** 上次射击的游戏时间，用于控制半自动射击频率 */
	float TimeOfLastShot = 0.0f;

	/** 是否正在开火 */
	bool bIsFiring = false;

	/** 全自动连发定时器 */
	FTimerHandle RefireTimer;

	/** 转换为Pawn的持有者指针，用于AI感知系统交互 */
	TObjectPtr<APawn> PawnOwner;

	/** 射击的AI感知噪声响度 */
	UPROPERTY(EditAnywhere, Category="Perception", meta = (ClampMin = 0, ClampMax = 100))
	float ShotLoudness = 1.0f;

	/** 射击AI感知噪声的最大范围 */
	UPROPERTY(EditAnywhere, Category="Perception", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float ShotNoiseRange = 3000.0f;

	/** 射击噪声的标签 */
	UPROPERTY(EditAnywhere, Category="Perception")
	FName ShotNoiseTag = FName("Shot");

public:	

	/** 构造函数 */
	AShooterWeapon();

protected:
	
	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 游戏清理 */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

protected:

	/** 当武器持有者被销毁时调用 */
	UFUNCTION()
	void OnOwnerDestroyed(AActor* DestroyedActor);

public:

	/** 激活武器，准备开火 */
	void ActivateWeapon();

	/** 停用武器 */
	void DeactivateWeapon();

	/** 开始开火 */
	void StartFiring();

	/** 停止开火 */
	void StopFiring();

protected:

	/** 开火 */
	virtual void Fire();

	/** 半自动武器的冷却结束回调 */
	void FireCooldownExpired();

	/** 向目标位置发射投射物 */
	virtual void FireProjectile(const FVector& TargetLocation);

	/** 计算投射物的生成变换 */
	FTransform CalculateProjectileSpawnTransform(const FVector& TargetLocation) const;

public:

	/** 返回第一人称网格体 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; };

	/** 返回第三人称网格体 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetThirdPersonMesh() const { return ThirdPersonMesh; };

	/** 返回第一人称动画实例类 */
	const TSubclassOf<UAnimInstance>& GetFirstPersonAnimInstanceClass() const;

	/** 返回第三人称动画实例类 */
	const TSubclassOf<UAnimInstance>& GetThirdPersonAnimInstanceClass() const;

	/** 返回弹匣容量 */
	int32 GetMagazineSize() const { return MagazineSize; };

	/** 返回当前子弹数 */
	int32 GetBulletCount() const { return CurrentBullets; }
};
