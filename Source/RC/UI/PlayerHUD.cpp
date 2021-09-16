// Fill out your copyright notice in the Description page of Project Settings.
#include "PlayerHUD.h"

#include "Blueprint/UserWidget.h"

#include "RC/Debug/Debug.h"
#include "RC/UI/Widgets/WeaponSelectScreenWidget.h"

void APlayerHUD::ShowWeaponSelect()
{
	WeaponSelectScreen = CreateWidget<UWeaponSelectScreenWidget>(PlayerOwner, WeaponSelectScreenClass);
	ASSERT_RETURN(WeaponSelectScreen != nullptr);

	WeaponSelectScreen->AddToViewport();
}

void APlayerHUD::HideWeaponSelect()
{
	ASSERT_RETURN(WeaponSelectScreen != nullptr);
	WeaponSelectScreen->ConditionalBeginDestroy();
	WeaponSelectScreen = nullptr;
}
