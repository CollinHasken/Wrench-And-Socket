// Fill out your copyright notice in the Description page of Project Settings.
#include "Wrench.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerController.h"
#include "RC/Characters/Components/MaskableInputComponent.h"
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

// Called when the weapon is being destroyed
void AWrench::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (bMaskApplied)
	{
		bMaskApplied = false;
		ARCCharacter* Player = Cast<ARCCharacter>(Wielder);
		if (Player != nullptr)
		{
			ARCPlayerController* PlayerController = Player->GetController<ARCPlayerController>();
			if (PlayerController != nullptr)
			{
				UMaskableInputComponent* MaskableInput = PlayerController->GetMaskableInput();
				if (MaskableInput != nullptr)
				{
					MaskableInput->PopMask(MeleeMask, "Wrench::Attack");
				}
			}
		}
	}
}

// Perform an attack with the weapon
void AWrench::PerformAttack()
{
	Super::PerformAttack();

	// Prevent movement
	if (MeleeMask != nullptr)
	{
		ARCCharacter* Player = Cast<ARCCharacter>(Wielder);
		ASSERT_RETURN(Player != nullptr);

		ARCPlayerController* PlayerController = Player->GetController<ARCPlayerController>();
		ASSERT_RETURN(PlayerController != nullptr);

		UMaskableInputComponent* MaskableInput = PlayerController->GetMaskableInput();
		if (MaskableInput != nullptr)
		{
			MaskableInput->PushMask(MeleeMask, "Wrench::Attack");
			bMaskApplied = true;
		}
	}
}

// Called when the attack has ended
void AWrench::AttackEnded(bool bInterrupted)
{
	Super::AttackEnded(bInterrupted);

	// Re-enable movement
	if (MeleeMask != nullptr)
	{
		ARCCharacter* Player = Cast<ARCCharacter>(Wielder);
		ASSERT_RETURN(Player != nullptr);

		ARCPlayerController* PlayerController = Player->GetController<ARCPlayerController>();
		ASSERT_RETURN(PlayerController != nullptr);

		UMaskableInputComponent* MaskableInput = PlayerController->GetMaskableInput();
		if (MaskableInput != nullptr)
		{
			MaskableInput->PopMask(MeleeMask, "Wrench::Attack");
			bMaskApplied = false;
		}
	}
}
