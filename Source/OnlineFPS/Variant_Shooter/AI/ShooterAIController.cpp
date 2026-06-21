// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/AI/ShooterAIController.h"
#include "ShooterNPC.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/Navigation/PathFollowingAgentInterface.h"

AShooterAIController::AShooterAIController()
{
	// 创建状态树组件
	StateTreeAI = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAI"));

	// 创建AI感知组件，将在蓝图中配置
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	// 订阅AI感知委托
	AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AShooterAIController::OnPerceptionUpdated);
	AIPerception->OnTargetPerceptionForgotten.AddDynamic(this, &AShooterAIController::OnPerceptionForgotten);
}

void AShooterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 确保正在操控的是NPC
	if (AShooterNPC* NPC = Cast<AShooterNPC>(InPawn))
	{
		// 为Pawn添加队伍标签
		NPC->Tags.Add(TeamTag);

		// 订阅Pawn的死亡委托
		NPC->OnPawnDeath.AddDynamic(this, &AShooterAIController::OnPawnDeath);
	}
}

void AShooterAIController::OnPawnDeath()
{
	// 停止移动
	GetPathFollowingComponent()->AbortMove(*this, FPathFollowingResultFlags::UserAbort);

	// 停止状态树逻辑
	StateTreeAI->StopLogic(FString(""));

	// 取消操控Pawn
	UnPossess();

	// 销毁此控制器
	Destroy();
}

void AShooterAIController::SetCurrentTarget(AActor* Target)
{
	TargetEnemy = Target;
}

void AShooterAIController::ClearCurrentTarget()
{
	TargetEnemy = nullptr;
}

void AShooterAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 将数据传递给状态树委托钩子
	OnShooterPerceptionUpdated.ExecuteIfBound(Actor, Stimulus);
}

void AShooterAIController::OnPerceptionForgotten(AActor* Actor)
{
	// 将数据传递给状态树委托钩子
	OnShooterPerceptionForgotten.ExecuteIfBound(Actor);
}
