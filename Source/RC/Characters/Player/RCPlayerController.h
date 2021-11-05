// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "RC/Util/RCTypes.h"

#include "RCPlayerController.generated.h"

// Broadcasted when a new, valid slot has been selected
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotSelected, EInventorySlot, Slot);

/**
 * Player controller
 * Includes UI controls to allow for fancier input
 */
UCLASS()
class RC_API ARCPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Called each frame
	void Tick(float DeltaTime) override;

	/**
	 * Open or close the weapon wheel widget
	 * @param Open Whether to open or close
	 */
	void OpenWeaponSelect(bool Open = true);

	UFUNCTION(BlueprintPure)
	class UMaskableInputComponent* GetMaskableInput() const { return MaskableInput; }

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
	
	// Called via input to close the weapon wheel
	void CloseWeaponSelect();

	// Called via input to close the pause HUD
	void ClosePauseHUD();

	// Called via input to close the pause settings
	void ClosePauseSettings();

	/**
	 * Called each frame to move the weapon selection based on the current input
	 * @param Direction	The rate of the mouse selection in the XY plane
	 */
	void ConsumeWeaponSelect(FVector2D& Direction);

	/**
	 * Called via input to move the weapon selection in the X direction
	 * @param Value This is a normalized rate of the mouse selection in the X
	 */
	void SetWeaponSelectX(float Value);

	/**
	 * Called via input to move the weapon selection in the Y direction
	 * @param Value This is a normalized rate of the mouse selection in the Y
	 */
	void SetWeaponSelectY(float Value);	

	// Component that handles input for the UI
	UPROPERTY(DuplicateTransient)
	class UInputComponent* UIInputComponent;

	// Maskable Input
	UPROPERTY(DuplicateTransient)
	class UMaskableInputComponent* MaskableInput;

	// Get the Slot Selected delegate
	FOnSlotSelected& OnSlotSelected() { return SlotSelectedDelegate; }

private:
	// Helper function to bind an action and set to execute while paused
	inline void BindPausedAction(const FName ActionName, const EInputEvent KeyEvent, void(ARCPlayerController::* Func)());

	// Helper function to bind an axis and set to execute while paused
	inline void BindPausedAxis(const FName ActionName, void(ARCPlayerController::* Func)(float));

	// Bind the inputs during weapon select
	void BindWeaponSelectInputs();

	// Bind the inputs for the pause HUD
	void BindPauseHUD();

	// Bind the inputs for pause settings
	void BindPauseSettings();

	// Unbind all
	void UnbindAll();

	// The current weapon select direction being inputted
	FVector2D WeaponSelectDirection = FVector2D::ZeroVector;

	// The current, valid weapon slot being selected
	EInventorySlot CurrentWeaponSlot = EInventorySlot::Slot1;

	// Broadcasted when a new, valid slot has been selected
	UPROPERTY(BlueprintAssignable, Category = Weapon, meta = (AllowPrivateAccess))
	FOnSlotSelected SlotSelectedDelegate;
};
