// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterGameMode.h"
#include "PVPGameMode.generated.h"

class AShooterCharacter;
class APlayerState;
class APlayerController;
class AController;

// 比赛结束事件广播（参数：胜者的队伍编号、胜者的击杀分数）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPVPMatchEnded, uint8, WinnerTeamByte, int32, WinnerScore);

/**
 *  自由死斗（FFA）模式的 GameMode
 *  所有玩家各自为战，先达到击杀分数上限者获胜
 *  如果比赛时间耗尽，则分数最高者获胜
 */
UCLASS()
class ONLINEFPS_API APVPGameMode : public AShooterGameMode
{
	GENERATED_BODY()

public:
	APVPGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	//~ 开始 ShooterGameMode 接口
	virtual void OnCharacterKilled(AShooterCharacter* Victim, AController* Killer) override;
	//~ 结束 ShooterGameMode 接口

public:
	/** 比赛结束时触发，蓝图可绑定此事件来更新UI */
	UPROPERTY(BlueprintAssignable, Category = "PVP Events")
	FOnPVPMatchEnded OnMatchEnded;

protected:
	/** 赢得比赛所需的击杀分数 */
	UPROPERTY(EditAnywhere, Category = "PVP", meta = (ClampMin = 1))
	int32 KillScoreLimit = 30;

	/** 比赛时长（秒） */
	UPROPERTY(EditAnywhere, Category = "PVP", meta = (ClampMin = 0, Units = "s"))
	float MatchDuration = 600.0f;

	/** 最大玩家数量 */
	UPROPERTY(EditAnywhere, Category = "PVP", meta = (ClampMin = 2))
	int32 MaxPlayers = 2;

	/** 比赛是否已开始 */
	bool bMatchStarted = false;

	/** 比赛是否已结束 */
	bool bMatchEnded = false;

	/** 比赛倒计时定时器 */
	FTimerHandle MatchTimerHandle;

	/** 每个玩家对应的队伍编号（用弱指针避免悬空引用） */
	TMap<TWeakObjectPtr<APlayerState>, uint8> PlayerToTeamByte;

	/** 下一个分配给新玩家的队伍编号 */
	uint8 NextTeamByte = 0;

	/** 开始PVP比赛（玩家数量达到上限时调用） */
	void StartPVPMatch();

	/** 检查是否有人达到了分数上限 */
	void CheckWinCondition();

	/** 比赛时间耗尽时的回调 */
	void OnMatchTimeExpired();

	/** 结束比赛，决出胜者 */
	void EndMatch();

	/** 把所有玩家送回主菜单 */
	void ReturnToMainMenu();
};
