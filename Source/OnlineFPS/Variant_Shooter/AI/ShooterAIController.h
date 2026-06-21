// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ShooterAIController.generated.h"

class UStateTreeAIComponent;
class UAIPerceptionComponent;
struct FAIStimulus;

DECLARE_DELEGATE_TwoParams(FShooterPerceptionUpdatedDelegate, AActor*, const FAIStimulus&);
DECLARE_DELEGATE_OneParam(FShooterPerceptionForgottenDelegate, AActor*);

/**
 *  第一人称射击游戏敌人的简单AI控制器
 */
UCLASS(abstract)
class ONLINEFPS_API AShooterAIController : public AAIController
{
	GENERATED_BODY()
	
	/** 运行此NPC的行为状态树 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStateTreeAIComponent* StateTreeAI;

	/** 通过视觉、听觉等感官检测其他Actor */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* AIPerception;

protected:

	/** 用于识别敌我的队伍标签 */
	UPROPERTY(EditAnywhere, Category="Shooter")
	FName TeamTag = FName("Enemy");

	/** 当前瞄准的敌人 */
	TObjectPtr<AActor> TargetEnemy;

public:

	/** AI感知更新时调用。状态树任务委托钩子 */
	FShooterPerceptionUpdatedDelegate OnShooterPerceptionUpdated;

	/** AI感知遗忘时调用。状态树任务委托钩子 */
	FShooterPerceptionForgottenDelegate OnShooterPerceptionForgotten;

public:

	/** 构造函数 */
	AShooterAIController();

protected:

	/** Pawn初始化 */
	virtual void OnPossess(APawn* InPawn) override;

protected:

	/** 被操控的Pawn死亡时调用 */
	UFUNCTION()
	void OnPawnDeath();

public:

	/** 设置瞄准的敌人 */
	void SetCurrentTarget(AActor* Target);

	/** 清除瞄准的敌人 */
	void ClearCurrentTarget();

	/** 返回瞄准的敌人 */
	AActor* GetCurrentTarget() const { return TargetEnemy; };

protected:

	/** AI感知组件更新某Actor感知时调用 */
	UFUNCTION()
	void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/** AI感知组件遗忘某Actor时调用 */
	UFUNCTION()
	void OnPerceptionForgotten(AActor* Actor);
};
