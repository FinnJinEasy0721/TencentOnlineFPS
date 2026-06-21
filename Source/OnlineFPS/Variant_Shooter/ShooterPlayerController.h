// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class AShooterCharacter;
class UShooterBulletCounterUI;

/**
 *  玩家控制器
 *  管理输入映射
 *  当玩家Pawn被销毁时重生
 */
UCLASS(abstract)
class ONLINEFPS_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** 默认输入映射上下文 */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** 移动端排除的输入映射上下文 */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** 移动端触控控件 */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** 移动端控件实例 */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** 重生时使用的角色类 */
	UPROPERTY(EditAnywhere, Category="Shooter|Respawn")
	TSubclassOf<AShooterCharacter> CharacterClass;

	/** 弹药计数器UI控件类 */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UShooterBulletCounterUI> BulletCounterUIClass;

	/** 给被操控的Pawn添加的玩家标签 */
	UPROPERTY(EditAnywhere, Category="Shooter|Player")
	FName PlayerPawnTag = FName("Player");

	/** 弹药计数器UI实例 */
	TObjectPtr<UShooterBulletCounterUI> BulletCounterUI;

protected:

	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 初始化输入绑定 */
	virtual void SetupInputComponent() override;

	/** Pawn初始化 */
	virtual void OnPossess(APawn* InPawn) override;

	/** 被操控的Pawn被销毁时调用 */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	/** 当被操控Pawn的弹药数量变化时调用 */
	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	/** 当被操控的Pawn受伤时调用 */
	UFUNCTION()
	void OnPawnDamaged(float LifePercent);
};
