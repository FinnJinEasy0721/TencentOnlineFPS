// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterPickup.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ShooterWeaponHolder.h"
#include "ShooterWeapon.h"
#include "Engine/World.h"
#include "TimerManager.h"

AShooterPickup::AShooterPickup()
{
 	PrimaryActorTick.bCanEverTick = true;

	// 创建根组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 创建碰撞球体
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	SphereCollision->SetupAttachment(RootComponent);

	SphereCollision->SetRelativeLocation(FVector(0.0f, 0.0f, 84.0f));
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionObjectType(ECC_WorldStatic);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollision->bFillCollisionUnderneathForNavmesh = true;

	// 绑定碰撞重叠事件
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AShooterPickup::OnOverlap);

	// 创建网格体
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SphereCollision);

	Mesh->SetCollisionProfileName(FName("NoCollision"));
}

void AShooterPickup::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (FWeaponTableRow* WeaponData = WeaponType.GetRow<FWeaponTableRow>(FString()))
	{
		// 从数据表设置网格体
		Mesh->SetStaticMesh(WeaponData->StaticMesh.LoadSynchronous());
	}
}

void AShooterPickup::BeginPlay()
{
	Super::BeginPlay();

	if (FWeaponTableRow* WeaponData = WeaponType.GetRow<FWeaponTableRow>(FString()))
	{
		// 从数据表复制武器类
		WeaponClass = WeaponData->WeaponToSpawn;
	}
}

void AShooterPickup::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// 清理重生定时器
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
}

void AShooterPickup::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 碰到的是武器持有者？
	if (IShooterWeaponHolder* WeaponHolder = Cast<IShooterWeaponHolder>(OtherActor))
	{
		// 给持有者添加武器
		WeaponHolder->AddWeaponClass(WeaponClass);

		// 隐藏拾取物
		SetActorHiddenInGame(true);

		// 关闭碰撞
		SetActorEnableCollision(false);

		// 关闭Tick
		SetActorTickEnabled(false);

		// 安排重生
		GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterPickup::RespawnPickup, RespawnTime, false);
	}
}

void AShooterPickup::RespawnPickup()
{
	// 显示拾取物
	SetActorHiddenInGame(false);

	// 调用蓝图处理重生效果
	BP_OnRespawn();
}

void AShooterPickup::FinishRespawn()
{
	// 开启碰撞
	SetActorEnableCollision(true);

	// 开启Tick
	SetActorTickEnabled(true);
}
