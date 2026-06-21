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
	// Player join/leave
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// Match start
	virtual void HandleMatchHasStarted() override;

	// Enemy management
	void EnemySpawned();
	void EnemyKilled(AController* Killer);
	void PlayerDied(APlayerController* Victim);

	// Win/lose condition check
	void CheckMatchEndCondition();

	// Win/lose broadcast delegates (for UI binding)
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnGameWon OnGameWon;
	UPROPERTY(BlueprintAssignable, Category = "Game Events")
	FOnGameLost OnGameLost;

protected:
	// Current active enemy count
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Stats")
	int32 ActiveEnemyCount;

	// Current alive player count
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Stats")
	int32 AlivePlayerCount;

	// Max player count
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game Settings")
	int32 MaxPlayers;

	// Match ended flag
	bool bMatchEnded;

	// Return to main menu
	void ReturnToMainMenu();
};
