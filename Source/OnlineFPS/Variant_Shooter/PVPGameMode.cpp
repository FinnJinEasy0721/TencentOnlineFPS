// Copyright Epic Games, Inc. All Rights Reserved.

#include "PVPGameMode.h"
#include "ShooterCharacter.h"
#include "ShooterUI.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

APVPGameMode::APVPGameMode()
{
	// 从蓝图资源中加载并设置默认角色类
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBP(TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterCharacter"));
	if (PlayerPawnBP.Class)
	{
		DefaultPawnClass = PlayerPawnBP.Class;
	}

	// 从蓝图资源中加载并设置默认玩家控制器类
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBP(TEXT("/Game/Variant_Shooter/Blueprints/BP_ShooterPlayerController"));
	if (PlayerControllerBP.Class)
	{
		PlayerControllerClass = PlayerControllerBP.Class;
	}

	// 从蓝图资源中加载并设置计分板UI类
	static ConstructorHelpers::FClassFinder<UShooterUI> ShooterUIClassFinder(TEXT("/Game/Variant_Shooter/UI/UI_Shooter"));
	if (ShooterUIClassFinder.Class)
	{
		ShooterUIClass = ShooterUIClassFinder.Class;
	}
}

void APVPGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 只有服务器才执行游戏逻辑
	if (!HasAuthority())
	{
		return;
	}
}

void APVPGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 只有服务器才执行游戏逻辑
	if (!HasAuthority())
	{
		return;
	}

	// 人数超限就踢出去
	if (GetNumPlayers() > MaxPlayers)
	{
		NewPlayer->ClientReturnToMainMenuWithTextReason(FText::FromString("Server is full."));
		return;
	}

	// 给新加入的玩家分配一个唯一的队伍编号
	if (APlayerState* PS = NewPlayer->GetPlayerState<APlayerState>())
	{
		uint8 AssignedTeamByte = NextTeamByte++;
		PlayerToTeamByte.Add(PS, AssignedTeamByte);
		UE_LOG(LogTemp, Log, TEXT("玩家加入，队伍编号: %d，当前玩家数: %d"), AssignedTeamByte, GetNumPlayers());
	}

	// 人齐了就开始比赛
	if (!bMatchStarted && GetNumPlayers() >= MaxPlayers)
	{
		StartPVPMatch();
	}
}

void APVPGameMode::Logout(AController* Exiting)
{
	// 先把要离开的玩家的 PlayerState 存下来，因为 Super::Logout 会把它清掉
	APlayerState* LeavingPS = Exiting ? Exiting->GetPlayerState<APlayerState>() : nullptr;

	Super::Logout(Exiting);

	// 只有服务器才执行游戏逻辑
	if (!HasAuthority())
	{
		return;
	}

	// 把离开的玩家从队伍编号表里移除
	if (LeavingPS)
	{
		PlayerToTeamByte.Remove(LeavingPS);
	}

	// 人不够了就结束比赛
	if (!bMatchEnded && GetNumPlayers() < 2)
	{
		EndMatch();
	}
}

void APVPGameMode::StartPVPMatch()
{
	// 防止重复开始
	if (bMatchStarted)
	{
		return;
	}

	bMatchStarted = true;

	// 启动比赛倒计时
	GetWorldTimerManager().SetTimer(MatchTimerHandle, this, &APVPGameMode::OnMatchTimeExpired, MatchDuration, false);

	UE_LOG(LogTemp, Log, TEXT("PVP 比赛开始！时长: %.0f 秒，击杀上限: %d"), MatchDuration, KillScoreLimit);
}

void APVPGameMode::OnCharacterKilled(AShooterCharacter* Victim, AController* Killer)
{
	// 比赛已经结束了就不再计分
	if (bMatchEnded)
	{
		return;
	}

	// 只给击杀者加分（自杀和环境致死不计分）
	if (Killer && Killer != Victim->GetController())
	{
		if (APlayerState* KillerPS = Killer->GetPlayerState<APlayerState>())
		{
			if (const uint8* TeamByte = PlayerToTeamByte.Find(KillerPS))
			{
				IncrementTeamScore(*TeamByte);
				UE_LOG(LogTemp, Log, TEXT("队伍 %d 获得一次击杀"), *TeamByte);
			}
		}
	}

	// 检查是否有人赢了
	CheckWinCondition();
}

void APVPGameMode::CheckWinCondition()
{
	if (bMatchEnded)
	{
		return;
	}

	// 遍历所有队伍分数，看有没有人达到上限
	for (const auto& Pair : TeamScores)
	{
		if (Pair.Value >= KillScoreLimit)
		{
			EndMatch();
			return;
		}
	}
}

void APVPGameMode::OnMatchTimeExpired()
{
	if (bMatchEnded)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("比赛时间到！"));
	EndMatch();
}

void APVPGameMode::EndMatch()
{
	// 防止重复结束
	if (bMatchEnded)
	{
		return;
	}

	bMatchEnded = true;

	// 停掉比赛倒计时
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);

	// 找出分数最高的玩家作为胜者
	uint8 WinnerTeamByte = 0;
	int32 WinnerScore = 0;
	bool bFoundWinner = false;

	for (const auto& Pair : TeamScores)
	{
		if (!bFoundWinner || Pair.Value > WinnerScore)
		{
			WinnerTeamByte = Pair.Key;
			WinnerScore = Pair.Value;
			bFoundWinner = true;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("比赛结束！胜者: 队伍 %d，击杀数: %d"), WinnerTeamByte, WinnerScore);

	// 广播比赛结束事件，让蓝图UI可以响应
	OnMatchEnded.Broadcast(WinnerTeamByte, WinnerScore);

	// 延迟5秒后把所有人送回主菜单
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &APVPGameMode::ReturnToMainMenu, 5.0f, false);
}

void APVPGameMode::ReturnToMainMenu()
{
	// 通知所有玩家控制器返回主菜单
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC)
		{
			PC->ClientReturnToMainMenuWithTextReason(FText::FromString("Match Over"));
		}
	}
}
