// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvQueryContext_Target.generated.h"

/**
 *  自定义环境查询上下文，返回NPC当前瞄准的Actor
 */
UCLASS()
class ONLINEFPS_API UEnvQueryContext_Target : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:

	/** 为此环境查询提供上下文位置或Actor */
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;

};
