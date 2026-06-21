// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/AI/EnvQueryContext_Target.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "ShooterAIController.h"

void UEnvQueryContext_Target::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	// 从查询实例中获取控制器
	if (AShooterAIController* Controller = Cast<AShooterAIController>(QueryInstance.Owner))
	{
		// 确保目标有效
		if (IsValid(Controller->GetCurrentTarget()))
		{
			// 将控制器的目标Actor添加到上下文中
			UEnvQueryItemType_Actor::SetContextHelper(ContextData, Controller->GetCurrentTarget());

		} else {

			// 如果没有目标，则默认使用控制器本身
			UEnvQueryItemType_Actor::SetContextHelper(ContextData, Controller);
		}
	}

}
