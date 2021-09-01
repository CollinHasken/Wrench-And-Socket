// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseWeapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"

#include "RC/Weapons/Bullets/BaseBullet.h"
#include "RC/Characters/BaseCharacter.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Debug/Debug.h"
#include "RC/Util/RCTypes.h"

// Sets default values
ABaseWeapon::ABaseWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Empty Root"));

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
}

void ABaseWeapon::PostInitProperties()
{
	Super::PostInitProperties();

	if (Mesh != nullptr) 
	{
		BulletOffsetSocket = Mesh->GetSocketByName(GetSocketName());
	} 
	else 
	{
		ASSERT(Mesh != nullptr, "Mesh wasn't created?")
	}
}

void ABaseWeapon::ApplyConfig(const FWeaponConfig& Config)
{
	WeaponConfig = Config;
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);	
}

// Set the new wielder
void ABaseWeapon::SetWielder(ABaseCharacter* NewWielder)
{
	Wielder = NewWielder;
}

bool ABaseWeapon::CanStartShooting()
{
	FTimerManager& TimerManager = GetWorldTimerManager();
	// Still coolingdown
	if (CooldownTimer.IsActive())
	{
		return false;
	}

	// Already have a shot queueing up
	if (TimerManager.IsTimerActive(ShootTimer))
	{
		return false;
	}

	return true;
}

bool ABaseWeapon::CanShoot()
{
	FTimerManager& TimerManager = GetWorldTimerManager();
	// Still coolingdown
	if (CooldownTimer.IsActive())
	{
		return false;
	}

	return true;
}

bool ABaseWeapon::Shoot()
{
	CooldownTimer.Set(WeaponConfig.CooldownDelay);
	return true;
}

void ABaseWeapon::CooldownExpired()
{

}
