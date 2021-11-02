// Fill out your copyright notice in the Description page of Project Settings.
#include "Wrench.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Weapons/Weapons/Components/WeaponMeleeComponent.h"
#include "RC/Util/RCStatics.h"
#include "RC/Debug/Debug.h"

AWrench::AWrench()
{
	// Set the default weapon class to projectile
	WeaponComponent = CreateDefaultSubobject<UWeaponMeleeComponent>(TEXT("Weapon"));

	/* MOVE TO MELEE COMPONENT */
	// Create the trigger to detect hitting
	HitTrigger = CreateDefaultSubobject<UCapsuleComponent>(TEXT("HitTrigger"));
	HitTrigger->SetCollisionProfileName(URCStatics::OverlapOnlyActor_ProfileName);
	HitTrigger->SetGenerateOverlapEvents(true);
	HitTrigger->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	HitTrigger->SetupAttachment(Mesh);
}

// Initialize the weapon component
void AWrench::InitWeaponComponent()
{
	UWeaponMeleeComponent* WeaponMeleeComponent = Cast<UWeaponMeleeComponent>(WeaponComponent);
	ASSERT_RETURN(WeaponMeleeComponent != nullptr);
	
	WeaponMeleeComponent->Init(*WeaponInfo, *HitTrigger);
}

// Perform an attack with the weapon
void AWrench::PerformAttack()
{
	Super::PerformAttack();

	// Prevent movement
	Wielder->DisableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
}

// Called when the attack has ended
void AWrench::AttackEnded(bool bInterrupted)
{
	Super::AttackEnded(bInterrupted);

	// Re-enable movement
	Wielder->EnableInput(UGameplayStatics::GetPlayerController(GetWorld(), 0));
}
