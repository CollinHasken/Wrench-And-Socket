// Fill out your copyright notice in the Description page of Project Settings.
#include "Enforcer.h"

#include "RC/Weapons/Weapons/Components/WeaponProjectileComponent.h"


AEnforcer::AEnforcer()
{
	WeaponComponent = CreateDefaultSubobject<UWeaponProjectileComponent>(TEXT("Weapon"));
}
