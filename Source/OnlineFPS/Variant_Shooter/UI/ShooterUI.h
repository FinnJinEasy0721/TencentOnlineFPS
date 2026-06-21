// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterUI.generated.h"

/**
 *  射手游戏的计分板UI控件
 */
UCLASS(abstract)
class ONLINEFPS_API UShooterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	/** 让蓝图更新分数子控件 */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter", meta = (DisplayName = "Update Score"))
	void BP_UpdateScore(uint8 TeamByte, int32 Score);
};
