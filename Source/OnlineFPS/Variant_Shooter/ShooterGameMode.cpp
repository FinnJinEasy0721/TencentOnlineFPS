// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"
#include "ShooterUI.h"
#include "ShooterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 如果没有设置UI类，就不创建，避免空指针崩溃
	if (ShooterUIClass)
	{
		ShooterUI = CreateWidget<UShooterUI>(UGameplayStatics::GetPlayerController(GetWorld(), 0), ShooterUIClass);
		if (ShooterUI)
		{
			ShooterUI->AddToViewport(0);
		}
	}
}

void AShooterGameMode::IncrementTeamScore(uint8 TeamByte)
{
	// 先拿到这个队伍当前的分数，没有就是0
	int32 Score = 0;
	if (int32* FoundScore = TeamScores.Find(TeamByte))
	{
		Score = *FoundScore;
	}

	// 加一分
	++Score;
	TeamScores.Add(TeamByte, Score);

	// 更新UI上的分数显示（如果UI存在的话）
	if (ShooterUI)
	{
		ShooterUI->BP_UpdateScore(TeamByte, Score);
	}
}

void AShooterGameMode::OnCharacterKilled(AShooterCharacter* Victim, AController* Killer)
{
	// 基类默认行为：给被击杀者的队伍加分（保持和原来一样）
	IncrementTeamScore(Victim->GetTeamByte());
}
