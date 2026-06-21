// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "OnlineFPSCharacter.h"
#include "HorrorCharacter.generated.h"

class USpotLightComponent;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateSprintMeterDelegate, float, Percentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSprintStateChangedDelegate, bool, bSprinting);

/**
 *  第一人称恐怖角色
 *  提供基于体力的冲刺机制
 */
UCLASS(abstract)
class ONLINEFPS_API AHorrorCharacter : public AOnlineFPSCharacter
{
	GENERATED_BODY()

	/** 玩家光源 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpotLightComponent* SpotLight;
	
protected:

	/** 冲刺输入动作 */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* SprintAction;

	/** 是否正在冲刺 */
	bool bSprinting = false;

	/** 是否正在恢复体力 */
	bool bRecovering = false;

	/** 非冲刺和非恢复时的默认行走速度 */
	UPROPERTY(EditAnywhere, Category="Walk")
	float WalkSpeed = 250.0f;

	/** 冲刺体力计时的间隔时间 */
	UPROPERTY(EditAnywhere, Category="Sprint", meta = (ClampMin = 0, ClampMax = 1, Units = "s"))
	float SprintFixedTickTime = 0.03333f;

	/** 冲刺体力值，最大值为 SprintTime */
	float SprintMeter = 0.0f;

	/** 可冲刺的持续时间（秒） */
	UPROPERTY(EditAnywhere, Category="Sprint", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float SprintTime = 3.0f;

	/** 冲刺时的行走速度 */
	UPROPERTY(EditAnywhere, Category="Sprint", meta = (ClampMin = 0, ClampMax = 10, Units = "cm/s"))
	float SprintSpeed = 600.0f;

	/** 恢复体力时的行走速度 */
	UPROPERTY(EditAnywhere, Category="Recovery", meta = (ClampMin = 0, ClampMax = 10, Units = "cm/s"))
	float RecoveringWalkSpeed = 150.0f;

	/** 体力恢复所需时间 */
	UPROPERTY(EditAnywhere, Category="Recovery", meta = (ClampMin = 0, ClampMax = 10, Units = "s"))
	float RecoveryTime = 0.0f;

	/** 冲刺计时定时器 */
	FTimerHandle SprintTimer;

public:

	/** 冲刺体力值更新时调用的委托 */
	FUpdateSprintMeterDelegate OnSprintMeterUpdated;

	/** 开始和停止冲刺时调用的委托 */
	FSprintStateChangedDelegate OnSprintStateChanged;

protected:

	/** 构造函数 */
	AHorrorCharacter();

	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 游戏清理 */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** 设置输入动作绑定 */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

protected:

	/** 开始冲刺行为 */
	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoStartSprint();

	/** 停止冲刺行为 */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoEndSprint();

	/** 冲刺时以固定时间间隔调用 */
	void SprintFixedTick();
};
