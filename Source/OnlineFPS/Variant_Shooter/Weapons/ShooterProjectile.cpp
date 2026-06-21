// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "TimerManager.h"

AShooterProjectile::AShooterProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建碰撞组件并设为根组件
	RootComponent = CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision Component"));

	CollisionComponent->SetSphereRadius(16.0f);
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionComponent->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;

	// 创建投射物移动组件，不需要挂载因为不是场景组件
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));

	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bShouldBounce = true;

	// 设置默认伤害类型
	HitDamageType = UDamageType::StaticClass();
}

void AShooterProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// 忽略发射这个投射物的Pawn
	CollisionComponent->IgnoreActorWhenMoving(GetInstigator(), true);
}

void AShooterProjectile::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清理延迟销毁定时器
	GetWorld()->GetTimerManager().ClearTimer(DestructionTimer);
}

void AShooterProjectile::NotifyHit(class UPrimitiveComponent* MyComp, AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	// 已经命中过其他物体就忽略
	if (bHit)
	{
		return;
	}

	bHit = true;

	// 关闭投射物碰撞
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 制造AI感知噪声
	MakeNoise(NoiseLoudness, GetInstigator(), GetActorLocation(), NoiseRange, NoiseTag);

	if (bExplodeOnHit)
	{
		// 爆炸模式：以投射物为中心施加范围伤害
		ExplosionCheck(GetActorLocation());

	} else {

		// 单体命中：处理碰撞到的Actor
		ProcessHit(Other, OtherComp, Hit.ImpactPoint, -Hit.ImpactNormal);

	}

	// 交给蓝图处理额外效果
	BP_OnProjectileHit(Hit);

	// 检查是否需要延迟销毁
	if (DeferredDestructionTime > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(DestructionTimer, this, &AShooterProjectile::OnDeferredDestruction, DeferredDestructionTime, false);

	} else {

		// 立即销毁投射物
		Destroy();
	}
}

void AShooterProjectile::ExplosionCheck(const FVector& ExplosionCenter)
{
	// 用球形重叠检测查找附近需要伤害的Actor
	TArray<FOverlapResult> Overlaps;

	FCollisionShape OverlapShape;
	OverlapShape.SetSphere(ExplosionRadius);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	if (!bDamageOwner)
	{
		QueryParams.AddIgnoredActor(GetInstigator());
	}

	GetWorld()->OverlapMultiByObjectType(Overlaps, ExplosionCenter, FQuat::Identity, ObjectParams, OverlapShape, QueryParams);

	TArray<AActor*> DamagedActors;

	// 处理重叠结果
	for (const FOverlapResult& CurrentOverlap : Overlaps)
	{
		// 重叠可能对同一个Actor返回多次结果
		// 确保每个Actor只被伤害一次
		if (DamagedActors.Find(CurrentOverlap.GetActor()) == INDEX_NONE)
		{
			DamagedActors.Add(CurrentOverlap.GetActor());

			// 计算爆炸推开方向
			const FVector& ExplosionDir = CurrentOverlap.GetActor()->GetActorLocation() - GetActorLocation();

			// 推开并/或伤害被波及的Actor
			ProcessHit(CurrentOverlap.GetActor(), CurrentOverlap.GetComponent(), GetActorLocation(), ExplosionDir.GetSafeNormal());
		}
			
	}
}

void AShooterProjectile::ProcessHit(AActor* HitActor, UPrimitiveComponent* HitComp, const FVector& HitLocation, const FVector& HitDirection)
{
	// 命中的是角色？
	if (ACharacter* HitCharacter = Cast<ACharacter>(HitActor))
	{
		// 忽略投射物的主人（除非允许自伤）
		if (HitCharacter != GetOwner() || bDamageOwner)
		{
			// 对角色造成伤害
			UGameplayStatics::ApplyDamage(HitCharacter, HitDamage, GetInstigator()->GetController(), this, HitDamageType);

			UE_LOG(LogTemp, Log, TEXT("玩家命中！对 %s 造成 %.1f 伤害"), *HitCharacter->GetName(), HitDamage);
		}
	}

	// 命中的是物理对象？
	if (HitComp->IsSimulatingPhysics())
	{
		// 施加物理冲量
		HitComp->AddImpulseAtLocation(HitDirection * PhysicsForce, HitLocation);
	}
}

void AShooterProjectile::OnDeferredDestruction()
{
	// 销毁投射物
	Destroy();
}
