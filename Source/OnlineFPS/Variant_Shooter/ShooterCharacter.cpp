// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"

AShooterCharacter::AShooterCharacter()
{
	// 创建AI噪声感知组件
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// 配置移动参数
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 把血量回满
	CurrentHP = MaxHP;

	// 更新血条UI
	OnDamaged.Broadcast(1.0f);
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清理重生定时器
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// 父类处理移动、瞄准和跳跃输入
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// 绑定动作输入
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// 开火
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopFiring);

		// 切换武器
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);
	}

}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 已经死了就不再受伤
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	// 记录是谁造成的伤害（用于击杀计分）
	LastDamageInstigator = EventInstigator;

	// 扣血
	CurrentHP -= Damage;

	// 血量归零就死亡
	if (CurrentHP <= 0.0f)
	{
		Die();
	}

	// 更新血条UI
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));

	return Damage;
}

void AShooterCharacter::DoStartFiring()
{
	// 用当前武器开火
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFiring();
	}
}

void AShooterCharacter::DoStopFiring()
{
	// 停止当前武器开火
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFiring();
	}
}

void AShooterCharacter::DoSwitchWeapon()
{
	// 至少要有两把武器才能切换
	if (OwnedWeapons.Num() > 1)
	{
		// 先停用当前武器
		CurrentWeapon->DeactivateWeapon();

		// 找到当前武器在列表中的位置
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// 如果已经是最后一把，就回到第一把
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			WeaponIndex = 0;
		}
		else {
			// 否则切到下一把
			++WeaponIndex;
		}

		// 切换到新武器
		CurrentWeapon = OwnedWeapons[WeaponIndex];

		// 激活新武器
		CurrentWeapon->ActivateWeapon();
	}
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// 把武器Actor挂到角色身上
	Weapon->AttachToActor(this, AttachmentRule);

	// 把武器的第一人称和第三人称网格体分别挂到对应插槽上
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, FirstPersonWeaponSocket);
	
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// 把后坐力转化为俯仰角输入
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	// 从摄像机位置向前做射线检测
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// 命中了就返回命中点，没命中就返回射线终点
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	// 检查是否已经拥有这种武器
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (!OwnedWeapon)
	{
		// 生成新武器
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

		AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

		if (AddedWeapon)
		{
			// 加入武器列表
			OwnedWeapons.Add(AddedWeapon);

			// 如果当前有武器在用，先停用
			if (CurrentWeapon)
			{
				CurrentWeapon->DeactivateWeapon();
			}

			// 切换到新武器
			CurrentWeapon = AddedWeapon;
			CurrentWeapon->ActivateWeapon();
		}
	}
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	// 更新弹药计数UI
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// 设置角色网格体的动画实例
	GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	// 未使用
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// 未使用
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// 遍历已拥有的武器，看有没有同类型的
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// 没找到
	return nullptr;

}

void AShooterCharacter::Die()
{
	UE_LOG(LogTemp, Log, TEXT("玩家死亡！角色: %s"), *GetName());

	// 停用当前武器
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}

	// 通知GameMode处理击杀计分
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->OnCharacterKilled(this, LastDamageInstigator.Get());
	}
		
	// 停止角色移动
	GetCharacterMovement()->StopMovementImmediately();

	// 禁用输入
	DisableInput(nullptr);

	// 重置弹药计数UI
	OnBulletCountUpdated.Broadcast(0, 0);

	// 调用蓝图死亡处理
	BP_OnDeath();

	// 延迟后重生（销毁角色，让玩家控制器重新生成）
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, RespawnTime, false);
}

void AShooterCharacter::OnRespawn()
{
	// 销毁角色，迫使玩家控制器重新生成新角色
	Destroy();
}
