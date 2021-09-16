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
	// Show the weapon select widgets
	UFUNCTION(BlueprintCallable)
	void ShowWeaponSelect();

	// Hide the weapon select widgets
	UFUNCTION(BlueprintCallable)
	void HideWeaponSelect();

private:
	// Weapon select screen class
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UWeaponSelectScreenWidget> WeaponSelectScreenClass;

	// The current weapon select screen
	class UWeaponSelectScreenWidget* WeaponSelectScreen = nullptr;
};
