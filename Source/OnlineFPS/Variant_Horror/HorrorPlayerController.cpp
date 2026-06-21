// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Horror/HorrorPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "OnlineFPSCameraManager.h"
#include "HorrorCharacter.h"
#include "HorrorUI.h"
#include "OnlineFPS.h"
#include "Widgets/Input/SVirtualJoystick.h"

AHorrorPlayerController::AHorrorPlayerController()
{
	// 设置玩家摄像机管理器类
	PlayerCameraManagerClass = AOnlineFPSCameraManager::StaticClass();
}

void AHorrorPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 仅在本机玩家控制器上生成触控控件
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		// 生成移动端控件
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// 将控件添加到玩家屏幕
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogOnlineFPS, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AHorrorPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	// 仅在本机玩家控制器上生成UI
	if (IsLocalPlayerController())
	{
		// 为角色设置UI
		if (AHorrorCharacter* HorrorCharacter = Cast<AHorrorCharacter>(aPawn))
		{
			// 创建UI
			if (!HorrorUI)
			{
				HorrorUI = CreateWidget<UHorrorUI>(this, HorrorUIClass);
				HorrorUI->AddToViewport(0);
			}

			HorrorUI->SetupCharacter(HorrorCharacter);
		}
	}
	
}

void AHorrorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// 仅在本机玩家控制器上添加输入映射上下文
	if (IsLocalPlayerController())
	{
		// 添加输入映射上下文
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// 仅在不使用移动端触控输入时添加这些映射
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
