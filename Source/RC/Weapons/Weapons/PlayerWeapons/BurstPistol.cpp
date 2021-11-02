// Fill out your copyright notice in the Description page of Project Settings.
#include "BurstPistol.h"

#include "RC/Weapons/Weapons/Components/WeaponProjectileComponent.h"

ABurstPistol::ABurstPistol()
{
	WeaponComponent = CreateDefaultSubobject<UWeaponProjectileComponent>(TEXT("Weapon"));
}
