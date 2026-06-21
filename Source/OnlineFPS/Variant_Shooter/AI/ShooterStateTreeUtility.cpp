// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/AI/ShooterStateTreeUtility.h"
#include "StateTreeExecutionContext.h"
#include "ShooterNPC.h"
#include "Camera/CameraComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "ShooterAIController.h"
#include "StateTreeAsyncExecutionContext.h"

bool FStateTreeLineOfSightToTargetCondition::TestCondition(FStateTreeExecutionContext& Context) const
{
	const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// 确保目标有效
	if (!IsValid(InstanceData.Target))
	{
		return !InstanceData.bMustHaveLineOfSight;
	}
	
	// 检查角色是否朝向目标
	const FVector TargetDir = (InstanceData.Target->GetActorLocation() - InstanceData.Character->GetActorLocation()).GetSafeNormal();

	const float FacingDot = FVector::DotProduct(TargetDir, InstanceData.Character->GetActorForwardVector());
	const float MaxDot = FMath::Cos(FMath::DegreesToRadians(InstanceData.LineOfSightConeAngle));

	// 朝向是否超出锥形半角范围？
	if (FacingDot <= MaxDot)
	{
		return !InstanceData.bMustHaveLineOfSight;
	}

	// 获取目标的包围盒
	FVector CenterOfMass, Extent;
	InstanceData.Target->GetActorBounds(true, CenterOfMass, Extent, false);

	// 将垂直范围除以视线检测次数
	const float ExtentZOffset = Extent.Z * 2.0f / InstanceData.NumberOfVerticalLineOfSightChecks;

	// 获取角色摄像机位置作为射线检测起点
	const FVector Start = InstanceData.Character->GetFirstPersonCameraComponent()->GetComponentLocation();

	// 忽略角色和目标，确保射线检测不计入它们
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(InstanceData.Character);
	QueryParams.AddIgnoredActor(InstanceData.Target);

	FHitResult OutHit;

	// 对目标位置执行多次垂直偏移的射线检测
	for (int32 i = 0; i < InstanceData.NumberOfVerticalLineOfSightChecks - 1; ++i)
	{
		// 计算射线终点
		const FVector End = CenterOfMass + FVector(0.0f, 0.0f, Extent.Z - ExtentZOffset * i);

		InstanceData.Character->GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

		// 射线是否无遮挡？
		if (!OutHit.bBlockingHit)
		{
			// 只需一次无遮挡即可提前返回
			return InstanceData.bMustHaveLineOfSight;
		}
	}

	// 未找到视线
	return !InstanceData.bMustHaveLineOfSight;
}

#if WITH_EDITOR
FText FStateTreeLineOfSightToTargetCondition::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Has Line of Sight</b>");
}
#endif

////////////////////////////////////////////////////////////////////

EStateTreeRunStatus FStateTreeFaceActorTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 是否从其他状态切换过来？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 获取实例数据
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 设置AI控制器的聚焦目标
		InstanceData.Controller->SetFocus(InstanceData.ActorToFaceTowards);
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeFaceActorTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 是否切换到了其他状态？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 获取实例数据
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 清除AI控制器的聚焦目标
		InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

#if WITH_EDITOR
FText FStateTreeFaceActorTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Face Towards Actor</b>");
}
#endif // WITH_EDITOR

////////////////////////////////////////////////////////////////////

EStateTreeRunStatus FStateTreeFaceLocationTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 是否从其他状态切换过来？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 获取实例数据
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 设置AI控制器的聚焦目标
		InstanceData.Controller->SetFocalPoint(InstanceData.FaceLocation);
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeFaceLocationTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 是否切换到了其他状态？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 获取实例数据
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 清除AI控制器的聚焦目标
		InstanceData.Controller->ClearFocus(EAIFocusPriority::Gameplay);
	}
}

#if WITH_EDITOR
FText FStateTreeFaceLocationTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Face Towards Location</b>");
}
#endif // WITH_EDITOR

////////////////////////////////////////////////////////////////////

EStateTreeRunStatus FStateTreeSetRandomFloatTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 是否切换到了其他状态？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 获取实例数据
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 计算输出值
		InstanceData.OutValue = FMath::RandRange(InstanceData.MinValue, InstanceData.MaxValue);
	}

	return EStateTreeRunStatus::Running;
}

#if WITH_EDITOR
FText FStateTreeSetRandomFloatTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Set Random Float</b>");
}
#endif // WITH_EDITOR

////////////////////////////////////////////////////////////////////

EStateTreeRunStatus FStateTreeShootAtTargetTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 是否从其他状态切换过来？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 获取实例数据
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 通知角色向目标射击
		InstanceData.Character->StartShooting(InstanceData.Target);
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeShootAtTargetTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 是否切换到了其他状态？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 获取实例数据
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 通知角色停止射击
		InstanceData.Character->StopShooting();
	}
}

#if WITH_EDITOR
FText FStateTreeShootAtTargetTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Shoot at Target</b>");
}
#endif // WITH_EDITOR

EStateTreeRunStatus FStateTreeSenseEnemiesTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 是否从其他状态切换过来？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 获取实例数据
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 在控制器上绑定感知更新委托
		InstanceData.Controller->OnShooterPerceptionUpdated.BindLambda(
			[WeakContext = Context.MakeWeakExecutionContext()](AActor* SensedActor, const FAIStimulus& Stimulus)
			{
				// 在lambda中获取实例数据
				const FStateTreeStrongExecutionContext StrongContext = WeakContext.MakeStrongExecutionContext();

				if (FInstanceDataType* LambdaInstanceData = StrongContext.GetInstanceDataPtr<FInstanceDataType>())
				{
					if (SensedActor->ActorHasTag(LambdaInstanceData->SenseTag))
					{
						bool bDirectLOS = false;

						// 计算感知信号方向
						const FVector StimulusDir = (Stimulus.StimulusLocation - LambdaInstanceData->Character->GetActorLocation()).GetSafeNormal();

						// 通过角色朝向和感知信号方向的点积推断角度
						const float DirDot = FVector::DotProduct(StimulusDir, LambdaInstanceData->Character->GetActorForwardVector());
						const float MaxDot = FMath::Cos(FMath::DegreesToRadians(LambdaInstanceData->DirectLineOfSightCone));

						// 方向是否在感知锥范围内？
						if (DirDot >= MaxDot)
						{
							// 在角色和被感知Actor之间执行射线检测
							FCollisionQueryParams QueryParams;
							QueryParams.AddIgnoredActor(LambdaInstanceData->Character);
							QueryParams.AddIgnoredActor(SensedActor);

							FHitResult OutHit;

							// 如果射线无遮挡则有直接视线
							bDirectLOS = !LambdaInstanceData->Character->GetWorld()->LineTraceSingleByChannel(OutHit, LambdaInstanceData->Character->GetActorLocation(), SensedActor->GetActorLocation(), ECC_Visibility, QueryParams);

						}

						// 检查是否对感知信号有直接视线
						if (bDirectLOS)
						{
							// 设置控制器的目标
							LambdaInstanceData->Controller->SetCurrentTarget(SensedActor);

							// 设置任务输出
							LambdaInstanceData->TargetActor = SensedActor;

							// 设置标志
							LambdaInstanceData->bHasTarget = true;
							LambdaInstanceData->bHasInvestigateLocation = false;

						// 对目标没有直接视线
						} else {

							// 如果已有目标，忽略部分感知并继续追踪
							if (!IsValid(LambdaInstanceData->TargetActor))
							{
								// 此感知信号是否比上次更强？
								if (Stimulus.Strength > LambdaInstanceData->LastStimulusStrength)
								{
									// 更新感知信号强度
									LambdaInstanceData->LastStimulusStrength = Stimulus.Strength;

									// 设置调查位置
									LambdaInstanceData->InvestigateLocation = Stimulus.StimulusLocation;

									// 设置调查标志
									LambdaInstanceData->bHasInvestigateLocation = true;
								}
							}
						}
					}
				}
			}
		);

		// 在控制器上绑定感知遗忘委托
		InstanceData.Controller->OnShooterPerceptionForgotten.BindLambda(
			[WeakContext = Context.MakeWeakExecutionContext()](AActor* SensedActor)
			{
				// 在lambda中获取实例数据
				FInstanceDataType* LambdaInstanceData = WeakContext.MakeStrongExecutionContext().GetInstanceDataPtr<FInstanceDataType>();

				if (!LambdaInstanceData)
				{
					return;
				}

				bool bForget = false;

				// 是否正在遗忘当前目标？
				if (SensedActor == LambdaInstanceData->TargetActor)
				{
					bForget = true;

				} else {

					// 是否正在遗忘部分感知？
					if (!IsValid(LambdaInstanceData->TargetActor))
					{
						bForget = true;
					}
				}

				if (bForget)
				{
					// 清除目标
					LambdaInstanceData->TargetActor = nullptr;

					// 清除标志
					LambdaInstanceData->bHasInvestigateLocation = false;
					LambdaInstanceData->bHasTarget = false;

					// 重置感知信号强度
					LambdaInstanceData->LastStimulusStrength = 0.0f;

					// 清除控制器上的目标
					LambdaInstanceData->Controller->ClearCurrentTarget();
					LambdaInstanceData->Controller->ClearFocus(EAIFocusPriority::Gameplay);
				}

			}
		);
	}

	return EStateTreeRunStatus::Running;
}

void FStateTreeSenseEnemiesTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	// 是否切换到了其他状态？
	if (Transition.ChangeType == EStateTreeStateChangeType::Changed)
	{
		// 获取实例数据
		FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

		// 解绑感知委托
		InstanceData.Controller->OnShooterPerceptionUpdated.Unbind();
		InstanceData.Controller->OnShooterPerceptionForgotten.Unbind();
	}
}

#if WITH_EDITOR
FText FStateTreeSenseEnemiesTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting /*= EStateTreeNodeFormatting::Text*/) const
{
	return FText::FromString("<b>Sense Enemies</b>");
}
#endif // WITH_EDITOR
