// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HorrorGameMode.generated.h"

/**
 *  第一人称恐怖游戏的简单游戏模式
 */
UCLASS(abstract)
class ONLINEFPS_API AHorrorGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	/** 构造函数 */
	AHorrorGameMode();
};
