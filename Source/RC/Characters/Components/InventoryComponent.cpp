// Fill out your copyright notice in the Description page of Project Settings.
#include "RC/Characters/Components/InventoryComponent.h"

#include "Camera/CameraComponent.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Weapons/Weapons/BasePlayerWeapon.h"
#include "RC/Debug/Debug.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerState.h"

FArchive& operator<<(FArchive& Ar, UInventoryComponent& SObj)
{
	if (Ar.IsLoading() || Ar.IsSaving())
	{
		UClass* Class = UInventoryComponent::StaticClass();
		Class->SerializeTaggedProperties(Ar, (uint8*)&SObj, Class, nullptr);
	}
	return Ar;
}

// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// Equip defaults
	for (const FLoadoutSlotInfo& LoadoutSlotInfo : DefaultLoadout.LoadoutSlotInfos)
	{
		AssignSlot(LoadoutSlotInfo.Slot, LoadoutSlotInfo.Weapon);
	}

	// Either equipped the saved slot or equip the default
	EquipSlot(EquippedSlot != EInventorySlot::NUM_SLOTS ? EquippedSlot : DefaultSlot);
}

// Assign a weapon to the slot. If that slot is currently equipped, it'll swap it out
void UInventoryComponent::AssignSlot(EInventorySlot Slot, TSubclassOf<ABasePlayerWeapon> WeaponClass)
{
	WeaponClasses[static_cast<uint8>(Slot)] = WeaponClass;

	if (Slot == EquippedSlot)
	{
		/****************
		*
		*HACK REMOVE
		*
		****************/
		EquippedSlot = EInventorySlot::NUM_SLOTS;
		EquipSlot(Slot);
	}
}

// Equip a slot as the current weapon. If there's no weapon in that slot, then it won't equip
bool UInventoryComponent::EquipSlot(EInventorySlot NewSlot)
{
	// Equipping invalid slot
	if (NewSlot == EInventorySlot::NUM_SLOTS)
	{
		return false;
	}

	// Equipping the same slot
	if (NewSlot == EquippedSlot)
	{
		return true;
	}

	// New slot has no weapon
	TSubclassOf<ABasePlayerWeapon>& WeaponClass = WeaponClasses[static_cast<uint8>(NewSlot)];
	if (WeaponClass == NULL)
	{
		return false;
	}

	AActor* Owner = GetOwner();
	ASSERT_RETURN_VALUE(Owner != nullptr, false);

	ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(Owner);
	ASSERT_RETURN_VALUE(BaseCharacter != nullptr, false, "Inventory isn't on a base character?");

	// * TODO *
	// delete old weapon
	if (EquippedWeapon != nullptr)
	{
		// Rename so we can name the new weapon
		EquippedWeapon->Rename();
		EquippedWeapon->Destroy();
	}

	EquippedWeapon = SpawnSlot(NewSlot);
	EquippedSlot = NewSlot;

	UCameraComponent* CharacterCamera = BaseCharacter->FindComponentByClass<UCameraComponent>();

	// Tell weapon who the owner is
	EquippedWeapon->SetWielder(BaseCharacter, CharacterCamera);

	ARCCharacter* Player = Cast<ARCCharacter>(BaseCharacter);
	if (Player)
	{
		ARCPlayerState* PlayerState = Player->GetPlayerState<ARCPlayerState>();
		if (PlayerState)
		{
			EquippedWeapon->SetData(PlayerState->FindOrAddWeaponDataForClass(WeaponClass));
		}
		else
		{
			ASSERT(PlayerState != nullptr, "Player with no state");
		}
	}

	// Attach weapon to socket
	FName SocketName = EquippedWeapon->GetSocketName();
	if (!SocketName.IsNone())
	{
		USkeletalMeshComponent* OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
		if (OwnerMesh) {
			FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
			AttachmentRules.bWeldSimulatedBodies = true;

			EquippedWeapon->GetMesh()->AttachToComponent(OwnerMesh, AttachmentRules, SocketName);
		}
	}

	return true;
}

bool UInventoryComponent::EquipQuickSlot(EQuickSlot QuickSlot)
{
	return EquipSlot(QuickSlots[static_cast<uint8>(QuickSlot)]);
}

/** Equip a weapon to a quickslot by mapping the inventory slot */
void UInventoryComponent::EquipToQuickSlot(EQuickSlot QuickSlot, EInventorySlot InventorySlot)
{
	QuickSlots[static_cast<uint8>(QuickSlot)] = InventorySlot;
}

/** Equip the next slot numerically */
void UInventoryComponent::EquipNextSlot()
{
	uint8 InventorySlotIndex = (static_cast<uint8>(EquippedSlot) + 1) % static_cast<uint8>(EInventorySlot::NUM_SLOTS);
	for (; InventorySlotIndex != static_cast<uint8>(EquippedSlot); InventorySlotIndex = (InventorySlotIndex + 1) % static_cast<uint8>(EInventorySlot::NUM_SLOTS))
	{
		if (WeaponClasses[InventorySlotIndex] == NULL)
		{
			continue;
		}

		EquipSlot(static_cast<EInventorySlot>(InventorySlotIndex));
		break;
	}
}

/** Equip the previous slot numerically */
void UInventoryComponent::EquipPreviousSlot()
{
	uint8 InventorySlotIndex = static_cast<uint8>(EquippedSlot) == 0 ? static_cast<uint8>(EInventorySlot::NUM_SLOTS) - 1 : (static_cast<uint8>(EquippedSlot) - 1);
	for (; InventorySlotIndex != static_cast<uint8>(EquippedSlot); InventorySlotIndex = InventorySlotIndex == 0 ? static_cast<uint8>(EInventorySlot::NUM_SLOTS) - 1 : InventorySlotIndex - 1)
	{
		if (InventorySlotIndex == -1)
		{
			InventorySlotIndex = static_cast<uint8>(EInventorySlot::NUM_SLOTS) - 1;
		}

		if (WeaponClasses[InventorySlotIndex] == nullptr)
		{
			continue;
		}

		EquipSlot(static_cast<EInventorySlot>(InventorySlotIndex));
		break;
	}
}


// Spawn the weapon from the slot
ABasePlayerWeapon* UInventoryComponent::SpawnSlot(EInventorySlot NewSlot)
{
	ASSERT_RETURN_VALUE(NewSlot != EInventorySlot::NUM_SLOTS, nullptr, "Trying to equip invalid slot");

	TSubclassOf<ABasePlayerWeapon>& WeaponClass = WeaponClasses[static_cast<uint8>(NewSlot)];
	ASSERT_RETURN_VALUE(WeaponClass != NULL, nullptr, "Trying to equip slot that has no weapon");

	UWorld* World = GetWorld();
	ASSERT_RETURN_VALUE(World != nullptr, nullptr);

	AActor* Owner = GetOwner();
	ASSERT_RETURN_VALUE(Owner != nullptr, nullptr);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = TEXT("CurrentWeapon");
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = Cast<APawn>(Owner);
	SpawnParams.bNoFail = true;

	FTransform SpawnTransform = FTransform::Identity;

	return World->SpawnActor<ABasePlayerWeapon>(WeaponClasses[static_cast<uint8>(NewSlot)], SpawnTransform, SpawnParams);
}