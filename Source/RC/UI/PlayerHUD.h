// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "RC/Util/RCTypes.h"

#include "PlayerHUD.generated.h"

/**
 * Player's HUD during normal gameplay
 */
UCLASS()
class RC_API APlayerHUD : public AHUD
{
	GENERATED_BODY()

public:
	// Show the weapon select widget
	UFUNCTION(BlueprintCallable)
	void ShowWeaponSelect();

	// Hide the weapon select widget
	UFUNCTION(BlueprintCallable)
	void HideWeaponSelect();

	// Show the pause settings widget
	UFUNCTION(BlueprintCallable)
	void ShowPauseSettings();

	// Hide the pause settings widget
	UFUNCTION(BlueprintCallable)
	void HidePauseSettings();

private:
	// Weapon select screen class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UWeaponSelectScreenWidget> WeaponSelectScreenClass;

	// Pause settings screen class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UUserWidget> PauseSettingsScreenClass;

	// The current weapon select screen
	class UWeaponSelectScreenWidget* WeaponSelectScreen = nullptr;

	// The current pause settings screen
	class UUserWidget* PauseSettingsScreen = nullptr;
};
