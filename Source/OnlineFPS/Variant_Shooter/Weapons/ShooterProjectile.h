// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShooterProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class ACharacter;
class UPrimitiveComponent;

/**
 *  射手游戏的投射物类
 */
UCLASS(abstract)
class ONLINEFPS_API AShooterProjectile : public AActor
{
	GENERATED_BODY()
	
	/** 投射物的碰撞组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* CollisionComponent;

	/** 投射物的移动组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

protected:

	/** 命中时的AI感知噪声响度 */
	UPROPERTY(EditAnywhere, Category="Projectile|Noise", meta = (ClampMin = 0, ClampMax = 100))
	float NoiseLoudness = 3.0f;

	/** 命中时的AI感知噪声范围 */
	UPROPERTY(EditAnywhere, Category="Projectile|Noise", meta = (ClampMin = 0, ClampMax = 100000, Units = "cm"))
	float NoiseRange = 3000.0f;

	/** 命中噪声的标签 */
	UPROPERTY(EditAnywhere, Category="Noise")
	FName NoiseTag = FName("Projectile");

	/** 命中时施加的物理力 */
	UPROPERTY(EditAnywhere, Category="Projectile|Hit", meta = (ClampMin = 0, ClampMax = 50000))
	float PhysicsForce = 100.0f;

	/** 命中时造成的伤害值 */
	UPROPERTY(EditAnywhere, Category="Projectile|Hit", meta = (ClampMin = 0, ClampMax = 100))
	float HitDamage = 25.0f;

	/** 伤害类型。可用于区分火焰、爆炸等特定伤害类型 */
	UPROPERTY(EditAnywhere, Category="Projectile|Hit")
	TSubclassOf<UDamageType> HitDamageType;

	/** 是否可以伤害射击者本人 */
	UPROPERTY(EditAnywhere, Category="Projectile|Hit")
	bool bDamageOwner = false;

	/** 是否在命中时爆炸并对范围内所有Actor造成范围伤害 */
	UPROPERTY(EditAnywhere, Category="Projectile|Explosion")
	bool bExplodeOnHit = false;

	/** 爆炸伤害的最大影响范围 */
	UPROPERTY(EditAnywhere, Category="Projectile|Explosion", meta = (ClampMin = 0, ClampMax = 5000, Units = "cm"))
	float ExplosionRadius = 500.0f;	

	/** 是否已经命中过某个物体 */
	bool bHit = false;

	/** 命中后多久才销毁投射物 */
	UPROPERTY(EditAnywhere, Category="Projectile|Destruction", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float DeferredDestructionTime = 5.0f;

	/** 延迟销毁定时器 */
	FTimerHandle DestructionTimer;

public:	

	/** 构造函数 */
	AShooterProjectile();

protected:
	
	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 游戏清理 */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** 处理碰撞命中 */
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;

protected:

	/** 在爆炸范围内查找并伤害附近的Actor */
	void ExplosionCheck(const FVector& ExplosionCenter);

	/** 处理投射物命中某个Actor的逻辑 */
	void ProcessHit(AActor* HitActor, UPrimitiveComponent* HitComp, const FVector& HitLocation, const FVector& HitDirection);

	/** 交给蓝图实现命中后的额外效果 */
	UFUNCTION(BlueprintImplementableEvent, Category="Projectile", meta = (DisplayName = "On Projectile Hit"))
	void BP_OnProjectileHit(const FHitResult& Hit);

	/** 延迟销毁定时器回调 */
	void OnDeferredDestruction();

};
