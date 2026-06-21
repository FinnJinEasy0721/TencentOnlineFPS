// CoopGameMode.cpp
#include "CoopGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

ACoopGameMode::ACoopGameMode()
{
	// Enable replication
	bReplicates = true;

	// Set default Pawn class (first person blueprint character)
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBP(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	if (PlayerPawnBP.Class)
	{
		DefaultPawnClass = PlayerPawnBP.Class;
	}

	// Max 2 players for co-op
	MaxPlayers = 2;

	ActiveEnemyCount = 0;
	AlivePlayerCount = 0;
	bMatchEnded = false;
}

void ACoopGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Initialize state
	ActiveEnemyCount = 0;
	AlivePlayerCount = 0;
	bMatchEnded = false;
}

void ACoopGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	// Delayed start: wait for players to join
	bDelayedStart = true;
}

void ACoopGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!HasAuthority()) return;

	// Enforce max player count
	if (GetNumPlayers() > MaxPlayers)
	{
		NewPlayer->ClientReturnToMainMenuWithTextReason(FText::FromString("Server is full (max 2 players)."));
		return;
	}

	AlivePlayerCount = GetNumPlayers();
	UE_LOG(LogTemp, Log, TEXT("Player joined. Total players: %d"), AlivePlayerCount);

	// Start match when 2 players are connected
	if (AlivePlayerCount == MaxPlayers)
	{
		StartMatch();
	}
}

void ACoopGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (!HasAuthority()) return;

	AlivePlayerCount = GetNumPlayers();
	UE_LOG(LogTemp, Log, TEXT("Player left. Remaining: %d"), AlivePlayerCount);

	// In co-op mode, any player leaving ends the match
	if (!bMatchEnded)
	{
		bMatchEnded = true;
		OnGameLost.Broadcast();
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ACoopGameMode::ReturnToMainMenu, 3.0f, false);
	}
}

void ACoopGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	UE_LOG(LogTemp, Log, TEXT("Match Started! Begin cooperative play."));
}

void ACoopGameMode::EnemySpawned()
{
	if (!HasAuthority()) return;
	ActiveEnemyCount++;
	UE_LOG(LogTemp, Log, TEXT("Enemy spawned. Active enemies: %d"), ActiveEnemyCount);
}

void ACoopGameMode::EnemyKilled(AController* Killer)
{
	if (!HasAuthority()) return;
	ActiveEnemyCount = FMath::Max(0, ActiveEnemyCount - 1);
	UE_LOG(LogTemp, Log, TEXT("Enemy killed. Remaining: %d"), ActiveEnemyCount);

	CheckMatchEndCondition();
}

void ACoopGameMode::PlayerDied(APlayerController* Victim)
{
	if (!HasAuthority()) return;
	AlivePlayerCount = FMath::Max(0, AlivePlayerCount - 1);
	UE_LOG(LogTemp, Log, TEXT("Player died. Alive players: %d"), AlivePlayerCount);

	CheckMatchEndCondition();
}

void ACoopGameMode::CheckMatchEndCondition()
{
	if (bMatchEnded) return;

	// Win condition: all enemies eliminated
	if (ActiveEnemyCount <= 0)
	{
		bMatchEnded = true;
		OnGameWon.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("All enemies killed - YOU WIN!"));
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ACoopGameMode::ReturnToMainMenu, 5.0f, false);
		return;
	}

	// Lose condition: all players dead
	if (AlivePlayerCount <= 0)
	{
		bMatchEnded = true;
		OnGameLost.Broadcast();
		UE_LOG(LogTemp, Log, TEXT("All players dead - MISSION FAILED!"));
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &ACoopGameMode::ReturnToMainMenu, 5.0f, false);
		return;
	}
}

void ACoopGameMode::ReturnToMainMenu()
{
	// Send all players back to main menu
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			PC->ClientReturnToMainMenuWithTextReason(FText::FromString("Game Over"));
		}
	}
}
