// Fill out your copyright notice in the Description page of Project Settings.

#include "RCGameInstance.h"

#include "Kismet/GameplayStatics.h"

#include "RC/Debug/Debug.h"
#include "RC/Framework/RCGameMode.h"

// Make a save in the auto save
bool URCGameInstance::AutoSave()
{
	ARCGameMode* GameMode = Cast<ARCGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode == nullptr)
	{
		return false;
	}

	bool bSucceeded = GameMode->SaveLevelTransitionDataToMemory(&LevelTransitionData);
	if (bSucceeded)
	{
		OnAutoSave().Broadcast();
	}

	return bSucceeded;
}

// Request to load a new level
void URCGameInstance::RequestNewLevel(const FName& LevelName)
{
	// Save off level transition data in the game mode
	ARCGameMode* GameMode = Cast<ARCGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->SaveLevelTransitionDataToMemory(&LevelTransitionData);
	}
	UGameplayStatics::OpenLevel(this, LevelName);
}

// Reset the level
void URCGameInstance::ResetLevel()
{
	UWorld* World = GetWorld();
	ASSERT_RETURN(World != nullptr);

	bIsResetting = true;

	static constexpr int FADE_OUT_TIME = 1;

	// Fade out
	APlayerCameraManager* PlayerCamera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (PlayerCamera != nullptr)
	{
		PlayerCamera->StartCameraFade(0, 1, FADE_OUT_TIME, FLinearColor::Black, true, true);
	}

	// Wait until it's faded out
	World->GetTimerManager().SetTimer(ResetFadeTimer, [&]()
		{
			// Reset actors
			// For whatever reason, the name of the map has to start with a / or else it will be invalid
			UGameplayStatics::OpenLevel(this, FName("/"), true, "restart");

			UWorld* World = GetWorld();
			ASSERT_RETURN(World != nullptr);

			// Fade in
			World->GetTimerManager().SetTimer(ResetFadeTimer, [&]()
				{
					APlayerCameraManager* PlayerCamera = UGameplayStatics::GetPlayerCameraManager(this, 0);
					if (PlayerCamera != nullptr)
					{
						PlayerCamera->StopCameraFade();
						PlayerCamera->StartCameraFade(1, 0, 0.5f, FLinearColor::Black, true);
					}
					bIsResetting = false;
				},
				1, false);
		},
		FADE_OUT_TIME, false);
}
