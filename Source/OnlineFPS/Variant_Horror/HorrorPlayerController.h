// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HorrorPlayerController.generated.h"

class UInputMappingContext;
class UHorrorUI;

/**
 *  第一人称恐怖游戏的玩家控制器
 *  管理输入映射
 *  管理UI
 */
UCLASS(abstract)
class ONLINEFPS_API AHorrorPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** 要生成的UI控件类型 */
	UPROPERTY(EditAnywhere, Category="Horror|UI")
	TSubclassOf<UHorrorUI> HorrorUIClass;

	/** UI控件指针 */
	TObjectPtr<UHorrorUI> HorrorUI;

public:

	/** 构造函数 */
	AHorrorPlayerController();

protected:

	/** 输入映射上下文 */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** 输入映射上下文 */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** 要生成的移动端控件 */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** 移动端控件指针 */
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** 游戏初始化 */
	virtual void BeginPlay() override;

	/** 被操控Pawn初始化 */
	virtual void OnPossess(APawn* aPawn) override;

	/** 输入映射上下文设置 */
	virtual void SetupInputComponent() override;

};
