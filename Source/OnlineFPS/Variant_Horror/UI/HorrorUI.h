// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HorrorUI.generated.h"

class AHorrorCharacter;

/**
 *  第一人称恐怖游戏的简单UI
 *  管理角色冲刺体力条显示
 */
UCLASS(abstract)
class ONLINEFPS_API UHorrorUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** 为传入的角色设置委托监听器 */
	void SetupCharacter(AHorrorCharacter* HorrorCharacter);

	/** 角色冲刺体力值更新时调用 */
	UFUNCTION()
	void OnSprintMeterUpdated(float Percent);

	/** 角色冲刺状态变化时调用 */
	UFUNCTION()
	void OnSprintStateChanged(bool bSprinting);

protected:

	/** 将控制权交给蓝图以更新冲刺体力条控件 */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror", meta = (DisplayName = "Sprint Meter Updated"))
	void BP_SprintMeterUpdated(float Percent);

	/** 将控制权交给蓝图以更新冲刺体力状态 */
	UFUNCTION(BlueprintImplementableEvent, Category="Horror", meta = (DisplayName = "Sprint State Changed"))
	void BP_SprintStateChanged(bool bSprinting);
};
