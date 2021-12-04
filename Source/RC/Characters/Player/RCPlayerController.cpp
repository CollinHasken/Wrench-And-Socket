// Fill out your copyright notice in the Description page of Project Settings.
#include "RC/Characters/Player/RCPlayerController.h"

#include "Components/InputComponent.h"
#include "Engine/InputDelegateBinding.h"
#include "GameFramework/Character.h"
#include "GameFramework/InputSettings.h"

#include "RC/Characters/Components/MaskableInputComponent.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Components/InventoryComponent.h"
#include "RC/Debug/Debug.h"
#include "RC/UI/PlayerHud.h"

// Called when the game starts or when spawned
void ARCPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Create UI Input
	UIInputComponent = NewObject<UInputComponent>(this, UInputSettings::GetDefaultInputComponentClass(), TEXT("UI_Input"));
	UIInputComponent->Priority = 999;
	UIInputComponent->RegisterComponent();
	if (UInputDelegateBinding::SupportsInputDelegate(GetClass()))
	{
		UIInputComponent->bBlockInput = bBlockInput;
		UInputDelegateBinding::BindInputDelegates(GetClass(), UIInputComponent);
	}

	// Create Maskable Input
	MaskableInput = NewObject<UMaskableInputComponent>(this, UMaskableInputComponent::StaticClass(), TEXT("Maskable Input"));
	MaskableInput->Priority = 9999;
	MaskableInput->RegisterComponent();
	PushInputComponent(MaskableInput);
}

// Called each frame
void ARCPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ConsumeWeaponSelect(WeaponSelectDirection);
}

// Open or close the weapon wheel widget
void ARCPlayerController::OpenWeaponSelect(bool Open/* = true*/)
{
	APlayerHUD* PlayerHUD = GetHUD<APlayerHUD>();
	ASSERT_RETURN(PlayerHUD != nullptr);

	ARCCharacter* RCPlayer = Cast<ARCCharacter>(GetCharacter());
	if (Open)
	{
		ASSERT_RETURN(RCPlayer != nullptr);

		// Enable UI input
		BindWeaponSelectInputs();
		PushInputComponent(UIInputComponent);
		RCPlayer->DisableInput(this);

		SetPause(true);

		PlayerHUD->ShowWeaponSelect();

		// Start the first slot as selected if it's occupied
		UInventoryComponent* Inventory = RCPlayer->GetInventory();
		ASSERT_RETURN(Inventory != nullptr);
		if (Inventory->IsSlotOccupied(EInventorySlot::Slot1))
		{
			CurrentWeaponSlot = EInventorySlot::Slot1;
			SlotSelectedDelegate.Broadcast(CurrentWeaponSlot);
		}
	}
	else
	{
		// Disable UI input
		UnbindAll();
		PopInputComponent(UIInputComponent);
		SetPause(false);

		PlayerHUD->HideWeaponSelect();

		if (RCPlayer != nullptr)
		{
			RCPlayer->EnableInput(this);

			UInventoryComponent* Inventory = RCPlayer->GetInventory();
			ASSERT_RETURN(Inventory != nullptr);

			Inventory->EquipSlot(CurrentWeaponSlot);
		}
		else
		{
			LOG_CHECK(RCPlayer != nullptr, LogUI, Error, "No character when opening weapon select")
		}
	}
}

// Open or close the pause settings widget
void ARCPlayerController::OpenPauseSettings(bool Open/* = true*/)
{
	APlayerHUD* PlayerHUD = GetHUD<APlayerHUD>();
	ASSERT_RETURN(PlayerHUD != nullptr);

	ARCCharacter* RCPlayer = Cast<ARCCharacter>(GetCharacter());
	if (Open)
	{
		ASSERT_RETURN(RCPlayer != nullptr);

		// Enable UI input
		BindPauseSettings();
		PushInputComponent(UIInputComponent);
		RCPlayer->DisableInput(this);

		SetPause(true);

		PlayerHUD->ShowPauseSettings();
	}
	else
	{
		// Disable UI input
		UnbindAll();
		PopInputComponent(UIInputComponent);
		SetPause(false);

		PlayerHUD->HidePauseSettings();

		if (RCPlayer != nullptr)
		{
			RCPlayer->EnableInput(this);
		}
		else
		{
			LOG_CHECK(RCPlayer != nullptr, LogUI, Error, "No character when opening weapon select")
		}
	}
}

// Called via input to close the weapon wheel
void ARCPlayerController::CloseWeaponSelect()
{
	OpenWeaponSelect(false);
}

// Called via input to close the pause HUD
void ARCPlayerController::ClosePauseHUD()
{}

// Called via input to close the pause settings
void ARCPlayerController::ClosePauseSettings()
{
	OpenPauseSettings(false);
}

// Called each frame to move the weapon selection based on the current input
void ARCPlayerController::ConsumeWeaponSelect(FVector2D& Direction)
{
	static constexpr float SELECT_TOL = 0.15f;
	static constexpr float SELECT_TOL_DIST_SQR = (SELECT_TOL * SELECT_TOL) * 2;
	static constexpr float FAR_MOVEMENT_TOL = 0.93f;

	// Not enough movement
	if (Direction.SizeSquared() < SELECT_TOL_DIST_SQR)
	{
		return;
	}

	Direction.Normalize();

	// Determine the slot by which direction they're mostly pointing towards
	EInventorySlot SelectedSlot;
	if (Direction.X >= FAR_MOVEMENT_TOL)
	{
		SelectedSlot = EInventorySlot::Slot3;
	}
	else if (Direction.X < -FAR_MOVEMENT_TOL)
	{
		SelectedSlot = EInventorySlot::Slot7;
	}
	else if (Direction.Y >= FAR_MOVEMENT_TOL)
	{
		SelectedSlot = EInventorySlot::Slot1;
	}
	else if (Direction.Y < -FAR_MOVEMENT_TOL)
	{
		SelectedSlot = EInventorySlot::Slot5;
	}
	else if (Direction.X > 0)
	{
		SelectedSlot = Direction.Y >= 0 ? EInventorySlot::Slot2 : EInventorySlot::Slot4;
	}
	else
	{
		SelectedSlot = Direction.Y >= 0 ? EInventorySlot::Slot8 : EInventorySlot::Slot6;
	}

	if (SelectedSlot == CurrentWeaponSlot)
	{
		return;
	}

	APlayerHUD* PlayerHUD = GetHUD<APlayerHUD>();
	ASSERT_RETURN(PlayerHUD != nullptr);

	ARCCharacter* RCPlayer = Cast<ARCCharacter>(GetCharacter());
	ASSERT_RETURN(RCPlayer != nullptr);

	UInventoryComponent* Inventory = RCPlayer->GetInventory();
	ASSERT_RETURN(Inventory != nullptr);

	// Only select the slot if there's something in it
	if (!Inventory->IsSlotOccupied(SelectedSlot))
	{
		return;
	}

	CurrentWeaponSlot = SelectedSlot;
	SlotSelectedDelegate.Broadcast(CurrentWeaponSlot);
	Direction = FVector2D::ZeroVector;
}

// Called via input to move the weapon selection in the X direction
void ARCPlayerController::SetWeaponSelectX(float Value)
{
	WeaponSelectDirection.X = Value;
}

// Called via input to move the weapon selection in the Y direction
void ARCPlayerController::SetWeaponSelectY(float Value)
{
	WeaponSelectDirection.Y = Value;
}

// Bind the inputs during weapon select
void ARCPlayerController::BindWeaponSelectInputs()
{
	BindPausedAction("SelectWeapon", IE_Released, &ARCPlayerController::CloseWeaponSelect);
	BindPausedAxis("WeaponSelectX", &ARCPlayerController::SetWeaponSelectX);
	BindPausedAxis("WeaponSelectY", &ARCPlayerController::SetWeaponSelectY);
}

// Bind the inputs for the pause HUD
void ARCPlayerController::BindPauseHUD()
{
	BindPausedAction("OpenPauseHUD", IE_Pressed, &ARCPlayerController::ClosePauseHUD);
}

// Bind the inputs for pause settings
void ARCPlayerController::BindPauseSettings()
{
	BindPausedAction("OpenPauseSettings", IE_Pressed, &ARCPlayerController::ClosePauseSettings);
}

// Unbind all
void ARCPlayerController::UnbindAll()
{
	ASSERT_RETURN(UIInputComponent != nullptr);
	UIInputComponent->ClearActionBindings();
}

// Helper function to bind an action and set to execute while paused
void ARCPlayerController::BindPausedAction(const FName ActionName, const EInputEvent KeyEvent, void(ARCPlayerController::* Func)())
{
	ASSERT_RETURN(UIInputComponent != nullptr);
	FInputActionBinding& Binding = UIInputComponent->BindAction(ActionName, KeyEvent, this, Func);
	Binding.bExecuteWhenPaused = true;
}


// Helper function to bind an axis and set to execute while paused
void ARCPlayerController::BindPausedAxis(const FName ActionName, void(ARCPlayerController::* Func)(float))
{
	ASSERT_RETURN(UIInputComponent != nullptr);
	FInputAxisBinding& Binding = UIInputComponent->BindAxis(ActionName, this, Func);
	Binding.bExecuteWhenPaused = true;
}