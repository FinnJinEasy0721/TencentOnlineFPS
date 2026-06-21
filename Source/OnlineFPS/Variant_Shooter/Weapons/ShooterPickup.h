// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "ShooterPickup.generated.h"

class USphereComponent;
class UPrimitiveComponent;
class AShooterWeapon;

/**
 *  武器拾取物的数据结构
 */
USTRUCT(BlueprintType)
struct FWeaponTableRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 拾取物上显示的网格体 */
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UStaticMesh> StaticMesh;

	/** 拾取时给予的武器类 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<AShooterWeapon> WeaponToSpawn;
};

/**
 *  射手游戏的武器拾取物
 */
UCLASS(abstract)
class ONLINEFPS_API AShooterPickup : public AActor
{
	GENERATED_BODY()

	/** 碰撞球体 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* SphereCollision;

	/** 拾取物的网格体，从数据表中设置 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Mesh;
	
protected:

	/** 武器类型数据（从数据表读取） */
	UPROPERTY(EditAnywhere, Category="Pickup")
	FDataTableRowHandle WeaponType;

	/** 拾取时给予的武器类，从数据表设置 */
	TSubclassOf<AShooterWeapon> WeaponClass;
	
	/** 拾取后重新生成的时间 */
	UPROPERTY(EditAnywhere, Category="Pickup", meta = (ClampMin = 0, ClampMax = 120, Units = "s"))
	float RespawnTime = 4.0f;

	/** 重新生成定时器 */
	FTimerHandle RespawnTimer;

public:	
	
	/** 构造函数 */
	AShooterPickup();

protected:

	/** 蓝图构造脚本，根据数据表设置网格体 */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 游戏清理 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 处理碰撞重叠 */
	UFUNCTION()
	virtual void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:

	/** 重新生成拾取物 */
	void RespawnPickup();

	/** 交给蓝图实现拾取物重生的动画效果，结束后调用 FinishRespawn */
	UFUNCTION(BlueprintImplementableEvent, Category="Pickup", meta = (DisplayName = "OnRespawn"))
	void BP_OnRespawn();

	/** 重生完成后启用拾取物 */
	UFUNCTION(BlueprintCallable, Category="Pickup")
	void FinishRespawn();
};
