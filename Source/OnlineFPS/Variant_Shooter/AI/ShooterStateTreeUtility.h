// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"

#include "ShooterStateTreeUtility.generated.h"

class AShooterNPC;
class AAIController;
class AShooterAIController;

/**
 *  FStateTreeLineOfSightToTargetCondition 条件的实例数据结构
 */
USTRUCT()
struct FStateTreeLineOfSightToTargetConditionInstanceData
{
	GENERATED_BODY()
	
	/** 瞄准角色 */
	UPROPERTY(EditAnywhere, Category = "Context")
	AShooterNPC* Character;

	/** 要检查视线的目标 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	AActor* Target;

	/** 最大允许的视线锥角（度） */
	UPROPERTY(EditAnywhere, Category = "Condition")
	float LineOfSightConeAngle = 35.0f;

	/** 为绕过低矮障碍物而执行的垂直视线检测次数 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	int32 NumberOfVerticalLineOfSightChecks = 5;

	/** 为true时，角色有视线则条件通过 */
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bMustHaveLineOfSight = true;
};
STATETREE_POD_INSTANCEDATA(FStateTreeLineOfSightToTargetConditionInstanceData);

/**
 *  检查角色是否着地的状态树条件
 */
USTRUCT(DisplayName = "Has Line of Sight to Target", Category="Shooter")
struct FStateTreeLineOfSightToTargetCondition : public FStateTreeConditionCommonBase
{
	GENERATED_BODY()

	/** 设置实例数据类型 */
	using FInstanceDataType = FStateTreeLineOfSightToTargetConditionInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 默认构造函数 */
	FStateTreeLineOfSightToTargetCondition() = default;
	
	/** 测试状态树条件 */
	virtual bool TestCondition(FStateTreeExecutionContext& Context) const override;

#if WITH_EDITOR
	/** 提供描述字符串 */
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

};

////////////////////////////////////////////////////////////////////

/**
 *  面向Actor状态树任务的实例数据结构
 */
USTRUCT()
struct FStateTreeFaceActorInstanceData
{
	GENERATED_BODY()

	/** 决定聚焦目标的AI控制器 */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AAIController> Controller;

	/** 要面向的Actor */
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<AActor> ActorToFaceTowards;
};

/**
 *  使AI控制的Pawn面向某Actor的状态树任务
 */
USTRUCT(meta=(DisplayName="Face Towards Actor", Category="Shooter"))
struct FStateTreeFaceActorTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* 确保使用正确的实例数据结构 */
	using FInstanceDataType = FStateTreeFaceActorInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 进入所属状态时运行 */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	/** 退出所属状态时运行 */
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

////////////////////////////////////////////////////////////////////

/**
 *  面向位置状态树任务的实例数据结构
 */
USTRUCT()
struct FStateTreeFaceLocationInstanceData
{
	GENERATED_BODY()

	/** 决定聚焦位置的AI控制器 */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AAIController> Controller;

	/** 要面向的位置 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FVector FaceLocation = FVector::ZeroVector;
};

/**
 *  使AI控制的Pawn面向世界位置的状态树任务
 */
USTRUCT(meta=(DisplayName="Face Towards Location", Category="Shooter"))
struct FStateTreeFaceLocationTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* 确保使用正确的实例数据结构 */
	using FInstanceDataType = FStateTreeFaceLocationInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 进入所属状态时运行 */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	/** 退出所属状态时运行 */
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

////////////////////////////////////////////////////////////////////

/**
 *  设置随机浮点数状态树任务的实例数据结构
 */
USTRUCT()
struct FStateTreeSetRandomFloatData
{
	GENERATED_BODY()

	/** 最小随机值 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float MinValue = 0.0f;

	/** 最大随机值 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float MaxValue = 0.0f;

	/** 输出的计算值 */
	UPROPERTY(EditAnywhere, Category = Output)
	float OutValue = 0.0f;
};

/**
 *  在指定范围内计算随机浮点数的状态树任务
 */
USTRUCT(meta=(DisplayName="Set Random Float", Category="Shooter"))
struct FStateTreeSetRandomFloatTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* 确保使用正确的实例数据结构 */
	using FInstanceDataType = FStateTreeSetRandomFloatData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 进入所属状态时运行 */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

////////////////////////////////////////////////////////////////////

/**
 *  向目标射击状态树任务的实例数据结构
 */
USTRUCT()
struct FStateTreeShootAtTargetInstanceData
{
	GENERATED_BODY()

	/** 执行射击的NPC */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AShooterNPC> Character;

	/** 射击目标 */
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<AActor> Target;
};

/**
 *  让NPC向某Actor射击的状态树任务
 */
USTRUCT(meta=(DisplayName="Shoot at Target", Category="Shooter"))
struct FStateTreeShootAtTargetTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* 确保使用正确的实例数据结构 */
	using FInstanceDataType = FStateTreeShootAtTargetInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 进入所属状态时运行 */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	/** 退出所属状态时运行 */
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

////////////////////////////////////////////////////////////////////

/**
 *  感知敌人状态树任务的实例数据结构
 */
USTRUCT()
struct FStateTreeSenseEnemiesInstanceData
{
	GENERATED_BODY()

	/** 感知AI控制器 */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AShooterAIController> Controller;

	/** 感知NPC */
	UPROPERTY(EditAnywhere, Category = Context)
	TObjectPtr<AShooterNPC> Character;

	/** 感知到的目标Actor */
	UPROPERTY(EditAnywhere, Category = Output)
	TObjectPtr<AActor> TargetActor;

	/** 感知到的调查位置 */
	UPROPERTY(EditAnywhere, Category = Output)
	FVector InvestigateLocation = FVector::ZeroVector;

	/** 成功感知到目标时为true */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bHasTarget = false;

	/** 成功感知到调查位置时为true */
	UPROPERTY(EditAnywhere, Category = Output)
	bool bHasInvestigateLocation = false;

	/** 被感知Actor需要具有的标签 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	FName SenseTag = FName("Player");

	/** 视为完全感知的视线锥半角 */
	UPROPERTY(EditAnywhere, Category = Parameter)
	float DirectLineOfSightCone = 85.0f;

	/** 上次处理的感知信号强度 */
	UPROPERTY(EditAnywhere)
	float LastStimulusStrength = 0.0f;
};

/**
 *  让NPC处理AI感知并感知附近敌人的状态树任务
 */
USTRUCT(meta=(DisplayName="Sense Enemies", Category="Shooter"))
struct FStateTreeSenseEnemiesTask : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()

	/* 确保使用正确的实例数据结构 */
	using FInstanceDataType = FStateTreeSenseEnemiesInstanceData;
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	/** 进入所属状态时运行 */
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

	/** 退出所属状态时运行 */
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
	virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif // WITH_EDITOR
};

////////////////////////////////////////////////////////////////////
