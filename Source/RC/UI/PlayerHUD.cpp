// Fill out your copyright notice in the Description page of Project Settings.
#include "PlayerHUD.h"

#include "Blueprint/UserWidget.h"

#include "RC/Debug/Debug.h"
#include "RC/UI/Widgets/WeaponSelectScreenWidget.h"

// Show the weapon select widget
void APlayerHUD::ShowWeaponSelect()
{
	WeaponSelectScreen = CreateWidget<UWeaponSelectScreenWidget>(PlayerOwner, WeaponSelectScreenClass);
	ASSERT_RETURN(WeaponSelectScreen != nullptr);

	WeaponSelectScreen->AddToViewport();
}

// Hide the weapon select widget
void APlayerHUD::HideWeaponSelect()
{
	ASSERT_RETURN(WeaponSelectScreen != nullptr);
	WeaponSelectScreen->ConditionalBeginDestroy();
	WeaponSelectScreen = nullptr;
}

// Show the pause settings widget
void APlayerHUD::ShowPauseSettings()
{
	PauseSettingsScreen = CreateWidget<UUserWidget>(PlayerOwner, PauseSettingsScreenClass);
	ASSERT_RETURN(PauseSettingsScreen != nullptr);

	PauseSettingsScreen->AddToViewport();
}

// Hide the pause settings widget
void APlayerHUD::HidePauseSettings()
{
	ASSERT_RETURN(PauseSettingsScreen != nullptr);
	PauseSettingsScreen->ConditionalBeginDestroy();
	PauseSettingsScreen = nullptr;
}
