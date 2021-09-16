// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "RC/Util/RCTypes.h"

#include "WeaponSelectScreenWidget.generated.h"

/**
 * Screen to show when selecting a weapon
 * The weapon wheel and quick select
 */
UCLASS()
class RC_API UWeaponSelectScreenWidget : public UUserWidget
{
	GENERATED_BODY()

private:

	// The weapon wheel to choose the weapon from
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
	class UWeaponWheelWidget* WeaponWheel;
};
