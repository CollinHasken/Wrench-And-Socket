// Fill out your copyright notice in the Description page of Project Settings.
#include "RC/Characters/Components/InventoryComponent.h"

#include "Camera/CameraComponent.h"
#include "Engine/StreamableManager.h"

#include "RC/Characters/BaseCharacter.h"
#include "RC/Weapons/Weapons/BasePlayerWeapon.h"
#include "RC/Debug/Debug.h"
#include "RC/Util/RCStatics.h"
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

	// If there wasn't a saved equip slot, set it to the default
	if (EquippedSlot == EInventorySlot::NUM_SLOTS)
	{
		EquippedSlot = DefaultSlot;
	}

	// Equip defaults
	for (const FLoadoutSlotInfo& LoadoutSlotInfo : DefaultLoadout.LoadoutSlotInfos)
	{
		AssignSlot(LoadoutSlotInfo.Slot, LoadoutSlotInfo.WeaponInfoId);
	}
}

// Called before the component is destroyed
void UInventoryComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Destroy the spawned weapon
	if (EquippedWeapon != nullptr)
	{
		EquippedWeapon->Destroy();
	}
}

// Assign a weapon to the slot. If that slot is currently equipped, it'll swap it out
void UInventoryComponent::AssignSlot(EInventorySlot Slot, const FPrimaryAssetId& WeaponInfoId)
{
	ASSERT_RETURN(Slot != EInventorySlot::NUM_SLOTS);

	Weapons[static_cast<uint8>(Slot)].AssetId = WeaponInfoId;
	Weapons[static_cast<uint8>(Slot)].WeaponInfo = nullptr;
	
	Weapons[static_cast<uint8>(Slot)].StreamHandle = URCStatics::LoadPrimaryAsset(WeaponInfoId, FStreamableDelegate::CreateUObject(this, &UInventoryComponent::OnWeaponInfoLoaded, Slot));
}

// Called once the weapon info has been loaded
void UInventoryComponent::OnWeaponInfoLoaded(EInventorySlot Slot)
{
	FStreamableHandle* StreamHandle = Weapons[static_cast<uint8>(Slot)].StreamHandle.Get();
	if (StreamHandle != nullptr)
	{
		Weapons[static_cast<uint8>(Slot)].WeaponInfo = Cast<UPlayerWeaponInfo>(StreamHandle->GetLoadedAsset());
	}
	else
	{
		// If the stream handle is invalid, then it should be already loaded
		Weapons[static_cast<uint8>(Slot)].WeaponInfo = URCStatics::GetPrimaryAssetObject<UPlayerWeaponInfo>(Weapons[static_cast<uint8>(Slot)].AssetId);
	}
	ASSERT(Weapons[static_cast<uint8>(Slot)].WeaponInfo != nullptr);


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
ABasePlayerWeapon* UInventoryComponent::EquipSlot(EInventorySlot NewSlot)
{
	// Equipping the same slot
	if (NewSlot == EquippedSlot)
	{
		return GetEquippedWeapon();
	}

	// Slot doesn't have weapon info
	if (!IsSlotOccupied(NewSlot))
	{
		return nullptr;
	}

	AActor* Owner = GetOwner();
	ASSERT_RETURN_VALUE(Owner != nullptr, nullptr);

	ARCCharacter* Player = Cast<ARCCharacter>(Owner);
	ASSERT_RETURN_VALUE(Player != nullptr, nullptr, "Inventory isn't on the player?");

	// * TODO *
	// delete old weapon
	if (EquippedWeapon != nullptr)
	{
		// Rename so we can name the new weapon
		EquippedWeapon->Rename();
		EquippedWeapon->Destroy();
	}

	ABasePlayerWeapon* NewWeapon = SpawnSlot(NewSlot);
	ASSERT_RETURN_VALUE(NewWeapon != nullptr, nullptr, "New weapon wasn't able to be spawned");

	EquippedWeapon = NewWeapon;
	PreviouslyEquippedSlot = EquippedSlot;
	EquippedSlot = NewSlot;

	UCameraComponent* CharacterCamera = Player->FindComponentByClass<UCameraComponent>();

	// Tell weapon who the owner is
	EquippedWeapon->SetWielder(Player, CharacterCamera);

	WeaponEquippedDelegate.Broadcast(EquippedWeapon);

	return EquippedWeapon;
}

// Equip a slot as the current weapon through the quick slot.
ABasePlayerWeapon* UInventoryComponent::EquipQuickSlot(EQuickSlot QuickSlot)
{
	return EquipSlot(QuickSlots[static_cast<uint8>(QuickSlot)]);
}

/** Equip a weapon to a quickslot by mapping the inventory slot */
void UInventoryComponent::EquipToQuickSlot(EQuickSlot QuickSlot, EInventorySlot InventorySlot)
{
	QuickSlots[static_cast<uint8>(QuickSlot)] = InventorySlot;
}

/** Equip the next slot numerically */
ABasePlayerWeapon* UInventoryComponent::EquipNextSlot()
{
	uint8 InventorySlotIndex = (static_cast<uint8>(EquippedSlot) + 1) % static_cast<uint8>(EInventorySlot::NUM_SLOTS);
	for (; InventorySlotIndex != static_cast<uint8>(EquippedSlot); InventorySlotIndex = (InventorySlotIndex + 1) % static_cast<uint8>(EInventorySlot::NUM_SLOTS))
	{
		if (!IsSlotOccupied(static_cast<EInventorySlot>(InventorySlotIndex)))
		{
			continue;
		}

		return EquipSlot(static_cast<EInventorySlot>(InventorySlotIndex));
	}
	return nullptr;
}

/** Equip the previous slot numerically */
ABasePlayerWeapon* UInventoryComponent::EquipPreviousSlot()
{
	uint8 InventorySlotIndex = static_cast<uint8>(EquippedSlot) == 0 ? static_cast<uint8>(EInventorySlot::NUM_SLOTS) - 1 : (static_cast<uint8>(EquippedSlot) - 1);
	for (; InventorySlotIndex != static_cast<uint8>(EquippedSlot); InventorySlotIndex = InventorySlotIndex == 0 ? static_cast<uint8>(EInventorySlot::NUM_SLOTS) - 1 : InventorySlotIndex - 1)
	{
		if (InventorySlotIndex == -1)
		{
			InventorySlotIndex = static_cast<uint8>(EInventorySlot::NUM_SLOTS) - 1;
		}

		if (!IsSlotOccupied(static_cast<EInventorySlot>(InventorySlotIndex)))
		{
			continue;
		}

		return EquipSlot(static_cast<EInventorySlot>(InventorySlotIndex));
	}
	return nullptr;
}

// Equip the previous weapon that was equipped
ABasePlayerWeapon* UInventoryComponent::EquipPreviousWeapon()
{
	return EquipSlot(PreviouslyEquippedSlot);
}

// Get weapon info for the slot
const UPlayerWeaponInfo* UInventoryComponent::GetWeaponInfo(EInventorySlot InventorySlot)
{
	if (InventorySlot == EInventorySlot::NUM_SLOTS)
	{
		return nullptr;
	}

	return Weapons[static_cast<uint8>(InventorySlot)].WeaponInfo;
}

// Get the weapon data for the inventory slot
bool UInventoryComponent::GetWeaponData(FWeaponData& WeaponData, EInventorySlot InventorySlot)
{
	if (InventorySlot == EInventorySlot::NUM_SLOTS)
	{
		return false;
	}

	// No weapon in that slot
	if (!IsSlotOccupied(InventorySlot))
	{
		return false;
	}

	// Inventory not on player
	ARCCharacter* Player = GetOwner<ARCCharacter>();
	if (Player == nullptr)
	{
		return false;
	}

	ARCPlayerState* PlayerState = Player->GetPlayerState<ARCPlayerState>();
	ASSERT_RETURN_VALUE(PlayerState != nullptr, false, "Player doesn't have RCPlayerState");

	FWeaponData* FoundWeaponData = PlayerState->FindOrAddDataForAsset<FWeaponData>(Weapons[static_cast<uint8>(InventorySlot)].AssetId);
	ASSERT_RETURN_VALUE(FoundWeaponData != nullptr, false, "Weapon Data not able to be added");
	
	WeaponData = *FoundWeaponData;

	return true;
}

// Get weapon class for the slot
bool UInventoryComponent::GetWeaponClass(TSubclassOf<class ABaseWeapon>& WeaponClass, EInventorySlot InventorySlot)
{
	if (!IsSlotOccupied(InventorySlot))
	{
		return false;
	}

	WeaponClass = Weapons[static_cast<uint8>(InventorySlot)].WeaponInfo->WeaponClass;
	return WeaponClass != NULL;
}

// Is there a weapon in the given slot
bool UInventoryComponent::IsSlotOccupied(EInventorySlot Slot)
{
	return (Slot != EInventorySlot::NUM_SLOTS) && (Weapons[static_cast<uint8>(Slot)].IsValid());
}

// Spawn the weapon from the slot
ABasePlayerWeapon* UInventoryComponent::SpawnSlot(EInventorySlot NewSlot)
{
	TSubclassOf<class ABaseWeapon> WeaponClass = NULL;
	if (!GetWeaponClass(WeaponClass, NewSlot))
	{
		return nullptr;
	}

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

	return World->SpawnActor<ABasePlayerWeapon>(WeaponClass, SpawnTransform, SpawnParams);
}