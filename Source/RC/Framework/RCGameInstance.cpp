// Fill out your copyright notice in the Description page of Project Settings.

#include "RCGameInstance.h"

#include "Kismet/GameplayStatics.h"

#include "RCGameMode.h"

void URCGameInstance::RequestNewLevel(const FName& LevelName)
{
	ARCGameMode* GameMode = Cast<ARCGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->SaveLevelTransitionDataToMemory(&LevelTransitionData);
	}
	UGameplayStatics::OpenLevel(this, LevelName);
}
