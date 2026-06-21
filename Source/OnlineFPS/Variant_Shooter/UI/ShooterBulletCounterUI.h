// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterBulletCounterUI.generated.h"

/**
 *  射手游戏的弹药计数器UI控件
 */
UCLASS(abstract)
class ONLINEFPS_API UShooterBulletCounterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** 让蓝图用新的弹药数量更新子控件 */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "UpdateBulletCounter"))
	void BP_UpdateBulletCounter(int32 MagazineSize, int32 BulletCount);

	/** 让蓝图更新血条并播放受击效果 */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta=(DisplayName = "Damaged"))
	void BP_Damaged(float LifePercent);
};
