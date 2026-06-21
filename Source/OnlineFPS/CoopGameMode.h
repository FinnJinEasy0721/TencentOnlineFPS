// CoopGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "CoopGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameWon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameLost);

UCLASS()
class ONLINEFPS_API ACoopGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ACoopGameMode();

protected:
	virtual void BeginPlay() override;
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

public:
	// 玩家加入/离开
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// 比赛开始
	virtual void HandleMatchHasStarted() override;

	// 敌人管理
	void EnemySpawned();
	void EnemyKilled(AController* Killer);
	void PlayerDied(APlayerController* Victim);

	// 胜负条件检查
	void CheckMatchEndCondition();

	// 胜负广播委托（供UI绑定）
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnGameWon OnGameWon;
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnGameLost OnGameLost;

protected:
	// 当前活跃敌人数量
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Stats")
	int32 ActiveEnemyCount;

	// 当前存活玩家数量
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Stats")
	int32 AlivePlayerCount;

	// 最大玩家数量
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Settings")
	int32 MaxPlayers;

	// 比赛已结束标志
	bool bMatchEnded;

	// 返回主菜单
	void ReturnToMainMenu();
};
