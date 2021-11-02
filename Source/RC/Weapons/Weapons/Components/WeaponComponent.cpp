// Fill out your copyright notice in the Description page of Project Settings.
#include "WeaponComponent.h"

#include "Camera/CameraComponent.h"

#include "RC/Characters/BaseCharacter.h"

// Initialize the weapon component
void UWeaponComponent::Init(const UWeaponInfo& WeaponInfo)
{
	SetDamage(WeaponInfo.BaseDamage);
	WeaponInfoId = WeaponInfo.GetPrimaryAssetId();
}

// Set the new wielder for this weapon
void UWeaponComponent::SetWielder(ABaseCharacter* NewWielder)
{
	Wielder = NewWielder;
	WielderCamera = NewWielder->FindComponentByClass<UCameraComponent>();
}
