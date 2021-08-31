// Fill out your copyright notice in the Description page of Project Settings.


#include "RCTypes.h"

#include "RC/Weapons/Weapons/BaseWeapon.h"

void FWeaponData::GrantDamageXP(float XP)
{
	ABaseWeapon* Weapon = CurrentWeapon.Get();
	if (Weapon != nullptr)
	{
		Weapon->GrantDamageXP(XP);
	}
}
