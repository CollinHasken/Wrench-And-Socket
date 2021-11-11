// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseWeapon.h"

#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Debug/Debug.h"
#include "RC/Weapons/RCWeaponTypes.h"
#include "RC/Weapons/Weapons/Components/WeaponComponent.h"

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Empty Root"));

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	WielderAttackMontageEndedDelegate.BindUObject(this, &ABaseWeapon::OnWielderAttackMontageEnded);
	WeaponAttackMontageEndedDelegate.BindUObject(this, &ABaseWeapon::OnWeaponAttackMontageEnded);
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	ASSERT(WeaponInfo != nullptr);
	if (WeaponInfo != nullptr)
	{
		CurrentCooldown = WeaponInfo->Cooldown;
	}

	// Search for a blueprint added weapon component
	if (WeaponComponent == nullptr)
	{
		TInlineComponentArray<UWeaponComponent*> WeaponComponents(this);
		if (WeaponComponents.Num() != 1)
		{
			ASSERT(WeaponComponents.Num() != 0, "No weapon component found on weapon %s", *GetName());
			ASSERT(WeaponComponents.Num() <= 1, "More than one weapon component found on weapon %s. Only one is supported", *GetName());
			return;
		}
		WeaponComponent = WeaponComponents[0];
	}
	ASSERT_RETURN(WeaponComponent != nullptr, "Weapon %s doesn't have a code or blueprint added weapon component", *GetName());

	InitWeaponComponent();
}

// Initialize the weapon component
void ABaseWeapon::InitWeaponComponent()
{
	// Setup the damage
	ASSERT(WeaponInfo != nullptr);
	if (WeaponInfo != nullptr)
	{
		WeaponComponent->Init(*WeaponInfo);
	}
}

// Called when the weapon is being destroyed
void ABaseWeapon::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (Reason == EEndPlayReason::Destroyed)
	{
		// Stop the attack montage if it's playing
		if (bWielderAttackMontagePlaying)
		{
			if (WeaponInfo != nullptr)
			{
				if (Wielder != nullptr)
				{
					UAnimInstance* WielderAnimInstance = Wielder->GetAnimInstance();
					if (WielderAnimInstance != nullptr)
					{
						WielderAnimInstance->Montage_Stop(0.2f, WeaponInfo->WeaponAttackMontage);
					}
				}
			}
		}
	}
}

// Set the new wielder
void ABaseWeapon::SetWielder(ABaseCharacter* NewWielder)
{
	Wielder = NewWielder;

	WeaponComponent->SetWielder(Wielder);

	// Attach weapon to socket
	if (!GetSocketName().IsNone())
	{
		USkeletalMeshComponent* WielderMesh = NewWielder->FindComponentByClass<USkeletalMeshComponent>();
		if (WielderMesh)
		{
			FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
			AttachmentRules.bWeldSimulatedBodies = true;
			ASSERT_RETURN(GetMesh() != nullptr);
			GetMesh()->AttachToComponent(WielderMesh, AttachmentRules, GetSocketName());
		}
	}
}

// Determine if the weapon can attack now
bool ABaseWeapon::CanAttack() const
{
	ASSERT_RETURN_VALUE(WeaponComponent != nullptr, false);

	// Attack playing
	if (bWielderAttackMontagePlaying || bWeaponAttackMontagePlaying)
	{
		return false;
	}

	// Still coolingdown
	if (CooldownTimer.IsActive())
	{
		return false;
	}

	return true;
}

// Attack if this weapon is able to
bool ABaseWeapon::Attack()
{
	if (!CanAttack())
	{
		return false;
	}

	PerformAttack();

	AttackDelegate.Broadcast(this);

	return true;
}

// Perform an attack with the weapon
void ABaseWeapon::PerformAttack()
{
	// Play the given montage and set the delegate
	auto PlayMontage = [&](bool* PlayingBool, FOnMontageEnded& MontageEndedDelegate, UAnimInstance& AnimInstance, UAnimMontage& Montage)
	{
		// Play montage and listen for end
		if (AnimInstance.Montage_Play(&Montage) == 0)
		{
			// Wasn't played succesfully
			LOG_CHECK(false, LogWeapon, Error, "Unable to play montage %f for weapon %f", *Montage.GetName(), *GetName());
		}
		AnimInstance.Montage_SetEndDelegate(MontageEndedDelegate, &Montage);
		*PlayingBool = true;		
	};

	// If there's an attack montage to play for the wielder
	if (WeaponInfo->WielderAttackMontage != nullptr)
	{
		LOG_RETURN(Wielder != nullptr, LogWeapon, Error, "Unable to play montage %f for weapon %f. Wielder not found.", *WeaponInfo->WielderAttackMontage->GetName(), *GetName());

		UAnimInstance* WielderAnimInstance = Wielder->GetAnimInstance();
		LOG_RETURN(WielderAnimInstance != nullptr, LogWeapon, Error, "Unable to play montage %f for weapon %f.\nWielder %f anim instance not found.", *WeaponInfo->WielderAttackMontage->GetName(), *GetName(), *Wielder->GetName());

		PlayMontage(&bWielderAttackMontagePlaying, WielderAttackMontageEndedDelegate, *WielderAnimInstance, *WeaponInfo->WielderAttackMontage);
	}

	// If there's an attack montage to play for the weapon
	if (WeaponInfo->WeaponAttackMontage != nullptr)
	{
		ASSERT_RETURN(GetMesh() != nullptr);

		UAnimInstance* WeaponAnimInstance = GetMesh()->GetAnimInstance();
		LOG_RETURN(WeaponAnimInstance != nullptr, LogWeapon, Error, "Unable to play montage %f for weapon %f.\nWeapon %f anim instance not found.", *WeaponInfo->WeaponAttackMontage->GetName(), *GetName(), *GetName());

		PlayMontage(&bWeaponAttackMontagePlaying, WeaponAttackMontageEndedDelegate, *WeaponAnimInstance, *WeaponInfo->WeaponAttackMontage);
	}

	// If we don't attack from the montage, then attack
	if (!WeaponInfo->AttackFromMontage)
	{
		WeaponComponent->Attack();
	}

	if (!bWielderAttackMontagePlaying && !bWeaponAttackMontagePlaying)
	{
		// If we don't have a montage, then it's an instant attack
		AttackEnded(false);
	}
}

// Called when the anim notify attack is triggered
void ABaseWeapon::OnAnimNotifyAttack()
{
	ASSERT_RETURN(WeaponComponent != nullptr);
	WeaponComponent->OnAnimNotifyAttack();
}

// Called when the anim notify state attack begins
void ABaseWeapon::OnAnimNotifyStateAttack_Begin()
{
	ASSERT_RETURN(WeaponComponent != nullptr);
	WeaponComponent->OnAnimNotifyStateAttack_Begin();
}

// Called when the anim notify state attack ends
void ABaseWeapon::OnAnimNotifyStateAttack_End()
{
	ASSERT_RETURN(WeaponComponent != nullptr);
	WeaponComponent->OnAnimNotifyStateAttack_End();
}

// Called when the attack montage has ended
void ABaseWeapon::OnWielderAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bWielderAttackMontagePlaying = false;
	if (!bWeaponAttackMontagePlaying)
	{
		AttackEnded(bInterrupted);
	}
}

// Called when the attack montage has ended
void ABaseWeapon::OnWeaponAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bWeaponAttackMontagePlaying = false;
	if (!bWielderAttackMontagePlaying)
	{
		AttackEnded(bInterrupted);
	}
}

// Called when the attack has ended
void ABaseWeapon::AttackEnded(bool bInterrupted)
{
	bWielderAttackMontagePlaying = false;
	bWeaponAttackMontagePlaying = false;
	CooldownTimer.Set(CurrentCooldown);
}
