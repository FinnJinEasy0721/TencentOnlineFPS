// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class UShooterUI;
class AShooterCharacter;
class AController;

/**
 *  射手模式的基础 GameMode
 *  负责创建游戏UI和维护队伍分数
 */
UCLASS(abstract)
class ONLINEFPS_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	/** 要创建的UI控件类型 */
	UPROPERTY(EditAnywhere, Category="Shooter")
	TSubclassOf<UShooterUI> ShooterUIClass;

	/** UI控件实例 */
	TObjectPtr<UShooterUI> ShooterUI;

	/** 各队伍的分数表（队伍编号 -> 击杀数） */
	TMap<uint8, int32> TeamScores;

protected:

	/** 游戏初始化 */
	virtual void BeginPlay() override;

public:

	/** 给指定队伍加一分 */
	void IncrementTeamScore(uint8 TeamByte);

	/** 角色被击杀时调用。基类默认给被击杀者的队伍加分（保持原有行为） */
	virtual void OnCharacterKilled(AShooterCharacter* Victim, AController* Killer);
};
