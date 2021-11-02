// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h" 	

#include "RC/Framework/AssetDataInterface.h"
#include "RC/Util/TimeStamp.h"
#include "RC/Util/RCTypes.h"
#include "RC/Weapons/RCWeaponTypes.h"

#include "BaseWeapon.generated.h"

// Broadcasted when the weapon attacks
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponAttack, class ABaseWeapon*, Weapon);

/**
 * Basic weapon
 */
UCLASS(Abstract, Blueprintable)
class RC_API ABaseWeapon : public AActor, public IAssetDataInterface
{
	GENERATED_BODY()
	
public:	
	ABaseWeapon();

	/**
	 * Set the new wielder for this weapon
	 * @param NewWielder		The pawn that is now wielding this weapon
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWielder(class ABaseCharacter* NewWielder);

	/**
	 * Attack if this weapon is able to
	 * Returns true if an attack was performed
	 */
	bool Attack();

	// Get the info for this weapon
	UFUNCTION(BlueprintPure, Category = "Weapon|Info")
	const UWeaponInfo* GetWeaponInfo() const { return WeaponInfo; }

	/* IAssetDataInterface interface */
	// Get the Primary Data Asset Id associated with this data actor's data.
	UFUNCTION(BlueprintPure, Category = "Weapon|Info")
	FPrimaryAssetId GetInfoId() const override { return WeaponInfo != nullptr ? WeaponInfo->GetPrimaryAssetId() : FPrimaryAssetId(); }
	/* End of IAssetDataInterface interface */

	FORCEINLINE class UWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

	// Returns Mesh subobject
	FORCEINLINE class USkeletalMeshComponent* GetMesh() const { return Mesh; }

	// Returns the Socket Name
	FORCEINLINE const FName& GetSocketName() const { return WeaponInfo != nullptr ? WeaponInfo->SocketName : NAME_NONE; }

	// Get the Shot delegate
	FOnWeaponAttack& OnShot() { return AttackDelegate; }
protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

	// Initialize the weapon component
	virtual void InitWeaponComponent();

	// Called when the weapon is being destroyed
	void EndPlay(const EEndPlayReason::Type Reason) override;
	
	// Determine if the weapon can attack now
	virtual bool CanAttack() const;

	// Perform an attack with the weapon
	virtual void PerformAttack();

	/**
	 * Called when the attack has ended
	 *
	 * @param bInterrupted	Whether the attack was interrupted
	 */
	virtual void AttackEnded(bool bInterrupted);

	// The current weapon config
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Config, meta = (AllowPrivateAccess = "true"))
	UWeaponInfo* WeaponInfo;

	// The weapon component, created dynamically with the set class
	UPROPERTY(Category = Weapon, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWeaponComponent* WeaponComponent = nullptr;

	// The main skeletal mesh associated with this weapon.
	UPROPERTY(Category = Mesh, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* Mesh = nullptr;

	// The actor wielding this weapon
	class ABaseCharacter* Wielder = nullptr;

	// Timer to keep track of cooling down
	FTimeStamp CooldownTimer;

	// Whether the attack montage is playing on the wielder
	bool bWielderAttackMontagePlaying = false;

	// Whether the attack montage is playing on the weapon
	bool bWeaponAttackMontagePlaying = false;

private:
	// Called when the anim notify attack is triggered
	UFUNCTION(BlueprintCallable)
	void OnAnimNotifyAttack();

	// Called when the anim notify state attack begins
	UFUNCTION(BlueprintCallable)
	void OnAnimNotifyStateAttack_Begin();

	// Called when the anim notify state attack ends
	UFUNCTION(BlueprintCallable)
	void OnAnimNotifyStateAttack_End();

	/**
	 * Called when the attack montage has ended on the wielder
	 *
	 * @param Montage		The montage that was stopped
	 * @param bInterrupted	Whether the montage was interrupted
	 */
	void OnWielderAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	/**
	 * Called when the attack montage has ended on the weapon
	 *
	 * @param Montage		The montage that was stopped
	 * @param bInterrupted	Whether the montage was interrupted
	 */
	void OnWeaponAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// Attack montage ended for the wielder delegate
	FOnMontageEnded WielderAttackMontageEndedDelegate;

	// Attack montage ended for the weapon delegate
	FOnMontageEnded WeaponAttackMontageEndedDelegate;

	// Broadcasted when the weapon fires
	UPROPERTY(BlueprintAssignable, Category = Weapon, meta = (AllowPrivateAccess))
	FOnWeaponAttack AttackDelegate;
};
