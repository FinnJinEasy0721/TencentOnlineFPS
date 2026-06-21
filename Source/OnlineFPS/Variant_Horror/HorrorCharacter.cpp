// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Horror/HorrorCharacter.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"

AHorrorCharacter::AHorrorCharacter()
{
	// 创建聚光灯
	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(GetFirstPersonCameraComponent());

	SpotLight->SetRelativeLocationAndRotation(FVector(30.0f, 17.5f, -5.0f), FRotator(-18.6f, -1.3f, 5.26f));
	SpotLight->Intensity = 0.5;
	SpotLight->SetIntensityUnits(ELightUnits::Lumens);
	SpotLight->AttenuationRadius = 1050.0f;
	SpotLight->InnerConeAngle = 18.7f;
	SpotLight->OuterConeAngle = 45.24f;
}

void AHorrorCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 将冲刺体力值初始化为最大值
	SprintMeter = SprintTime;

	// 初始化行走速度
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// 启动冲刺计时定时器
	GetWorld()->GetTimerManager().SetTimer(SprintTimer, this, &AHorrorCharacter::SprintFixedTick, SprintFixedTickTime, true);
}

void AHorrorCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清除冲刺定时器
	GetWorld()->GetTimerManager().ClearTimer(SprintTimer);
}

void AHorrorCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	{
		// 设置动作绑定
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
		{
			// 冲刺
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AHorrorCharacter::DoStartSprint);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AHorrorCharacter::DoEndSprint);

		}
	}
}

void AHorrorCharacter::DoStartSprint()
{
	// 设置冲刺标志
	bSprinting = true;

	// 是否已脱离恢复状态？
	if (!bRecovering)
	{
		// 设置冲刺行走速度
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;

		// 调用冲刺状态变化委托
		OnSprintStateChanged.Broadcast(true);
	}

}

void AHorrorCharacter::DoEndSprint()
{
	// 设置冲刺标志
	bSprinting = false;

	// 是否已脱离恢复状态？
	if (!bRecovering)
	{
		// 设置默认行走速度
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

		// 调用冲刺状态变化委托
		OnSprintStateChanged.Broadcast(false);
	}
}

void AHorrorCharacter::SprintFixedTick()
{
	// 是否已脱离恢复、仍有体力且移动速度高于行走速度？
	if (bSprinting && !bRecovering && GetVelocity().Length() > WalkSpeed)
	{

		// 是否还有体力可消耗？
		if (SprintMeter > 0.0f)
		{
			// 更新冲刺体力值
			SprintMeter = FMath::Max(SprintMeter - SprintFixedTickTime, 0.0f);

			// 体力是否已耗尽？
			if (SprintMeter <= 0.0f)
			{
				// 设置恢复标志
				bRecovering = true;

				// 设置恢复时的行走速度
				GetCharacterMovement()->MaxWalkSpeed = RecoveringWalkSpeed;
			}
		}
		
	} else {

		// 恢复体力
		SprintMeter = FMath::Min(SprintMeter + SprintFixedTickTime, SprintTime);

		if (SprintMeter >= SprintTime)
		{
			// 清除恢复标志
			bRecovering = false;

			// 根据冲刺按键是否按下设置行走或冲刺速度
			GetCharacterMovement()->MaxWalkSpeed = bSprinting ? SprintSpeed : WalkSpeed;

			// 根据按键状态更新冲刺状态
			OnSprintStateChanged.Broadcast(bSprinting);
		}

	}

	// 广播冲刺体力值更新委托
	OnSprintMeterUpdated.Broadcast(SprintMeter / SprintTime);

}
