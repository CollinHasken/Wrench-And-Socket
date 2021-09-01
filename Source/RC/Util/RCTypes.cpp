// Fill out your copyright notice in the Description page of Project Settings.


#include "RCTypes.h"

#include "RC/Weapons/Weapons/BasePlayerWeapon.h"

void FWeaponData::GrantDamageXP(float XP)
{
	ABasePlayerWeapon* Weapon = CurrentWeapon.Get();
	if (Weapon != nullptr)
	{
		Weapon->GrantDamageXP(XP);
	}
}
