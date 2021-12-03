// Fill out your copyright notice in the Description page of Project Settings.
#include "WeaponMeleeComponent.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Characters/Components/StatusEffectComponent.h"
#include "RC/Debug/Debug.h"
#include "RC/Framework/DamageInterface.h"
#include "RC/Util/RCStatics.h"

// Initialize the weapon component
void UWeaponMeleeComponent::Init(const UWeaponInfo& InWeaponInfo, UCapsuleComponent& CapsuleComponent)
{
	Super::Init(InWeaponInfo);

	Capsule = &CapsuleComponent;
	Capsule->OnComponentBeginOverlap.AddDynamic(this, &UWeaponMeleeComponent::OnWeaponTriggerOverlap);
}

// Called when the anim notify state attack begins
void UWeaponMeleeComponent::OnAnimNotifyStateAttack_Begin()
{
	ASSERT_RETURN(Capsule != nullptr);

	GetOwner()->SetActorEnableCollision(true);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// Called when the anim notify state attack ends
void UWeaponMeleeComponent::OnAnimNotifyStateAttack_End()
{
	ASSERT_RETURN(Capsule != nullptr);

	GetOwner()->SetActorEnableCollision(false);
	Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// On actor overlapping with the weapon's trigger
void UWeaponMeleeComponent::OnWeaponTriggerOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult& HitResult)
{
	IDamageInterface* DamageableActor = Cast<IDamageInterface>(OtherActor);
	if (DamageableActor != nullptr)
	{
		// Request damage on hit
		FDamageRequestParams DamageParams;
		DamageParams.bFromPlayer = URCStatics::IsActorPlayer(Wielder);
		DamageParams.Damage = Damage;
		DamageParams.DamageType = WeaponInfo->DamageType;
		DamageParams.Instigator = Wielder;
		DamageParams.CauseId = WeaponInfoId;
		DamageParams.HitLocation = HitResult.ImpactPoint;
		DamageParams.HitNormal = HitResult.Normal;
		DamageableActor->RequestDamage(DamageParams);

		// Give status effect
		if (WeaponInfo->TimedStatusEffectClass != NULL)
		{
			UStatusEffectComponent* StatusEffectComponent = OtherActor->FindComponentByClass<UStatusEffectComponent>();
			if (StatusEffectComponent != nullptr)
			{
				FStatusEffectTimedRequest TimedRequest(WeaponInfo->TimedStatusEffectClass, WeaponInfo->TimedStatusEffectDuration);
				StatusEffectComponent->AddStatusEffectTimed(TimedRequest);
			}
		}
	}	
}
