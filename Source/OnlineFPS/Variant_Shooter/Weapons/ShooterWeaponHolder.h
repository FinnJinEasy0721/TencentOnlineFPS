// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ShooterWeaponHolder.generated.h"

class AShooterWeapon;
class UAnimMontage;


// 这个类不需要修改
UINTERFACE(MinimalAPI)
class UShooterWeaponHolder : public UInterface
{
	GENERATED_BODY()
};

/**
 *  射手游戏武器持有者的通用接口
 */
class ONLINEFPS_API IShooterWeaponHolder
{
	GENERATED_BODY()

public:

	/** 把武器的网格体挂到持有者身上 */
	virtual void AttachWeaponMeshes(AShooterWeapon* Weapon) = 0;

	/** 播放开火动画蒙太奇 */
	virtual void PlayFiringMontage(UAnimMontage* Montage) = 0;

	/** 给持有者施加后坐力 */
	virtual void AddWeaponRecoil(float Recoil) = 0;

	/** 更新HUD上的弹药数量 */
	virtual void UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize) = 0;

	/** 计算并返回武器的瞄准目标位置 */
	virtual FVector GetWeaponTargetLocation() = 0;

	/** 给持有者添加一把指定类型的武器 */
	virtual void AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass) = 0;

	/** 激活指定的武器 */
	virtual void OnWeaponActivated(AShooterWeapon* Weapon) = 0;

	/** 停用指定的武器 */
	virtual void OnWeaponDeactivated(AShooterWeapon* Weapon) = 0;

	/** 通知持有者半自动武器冷却已结束，可以再次射击 */
	virtual void OnSemiWeaponRefire() = 0;
};
