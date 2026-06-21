// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterBulletCounterUI.h"
#include "OnlineFPS.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 只在本地玩家控制器上生成触控控件
	if (IsLocalPlayerController())
	{
		if (SVirtualJoystick::ShouldDisplayTouchInterface())
		{
			// 生成移动端控件
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// 添加到玩家屏幕上
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogOnlineFPS, Error, TEXT("无法生成移动端控件。"));

			}
		}

		// 创建弹药计数器控件并添加到屏幕上
		BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogOnlineFPS, Error, TEXT("无法生成弹药计数器控件。"));

		}
		
	}
}

void AShooterPlayerController::SetupInputComponent()
{
	// 只在本地玩家控制器上添加输入映射
	if (IsLocalPlayerController())
	{
		// 添加输入映射上下文
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// 如果不是移动端触控输入，才添加这些映射
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 订阅Pawn的销毁事件
	InPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

	// 是射手角色？
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		// 添加玩家标签
		ShooterCharacter->Tags.Add(PlayerPawnTag);

		// 订阅Pawn的委托事件
		ShooterCharacter->OnBulletCountUpdated.AddDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.AddDynamic(this, &AShooterPlayerController::OnPawnDamaged);

		// 强制更新一次血条
		ShooterCharacter->OnDamaged.Broadcast(1.0f);
	}
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// 重置弹药计数UI（先检查指针是否有效，避免退出时崩溃）
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	// 如果正在退出，就不要重生了
	if (!IsValid(GetWorld()) || GetWorld()->bIsTearingDown)
	{
		return;
	}

	// 找到所有出生点
	TArray<AActor*> ActorList;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);

	if (ActorList.Num() > 0)
	{
		// 随机选一个出生点
		AActor* RandomPlayerStart = ActorList[FMath::RandRange(0, ActorList.Num() - 1)];

		// 在出生点生成角色
		const FTransform SpawnTransform = RandomPlayerStart->GetActorTransform();

		if (AShooterCharacter* RespawnedCharacter = GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
		{
			// 操控新生成的角色
			Possess(RespawnedCharacter);
		}
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// 更新UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}
