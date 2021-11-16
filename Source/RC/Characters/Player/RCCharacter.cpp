// Copyright Epic Games, Inc. All Rights Reserved.

#include "RCCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

#include "RC/Debug/Debug.h"
#include "RC/Characters/Player/RCPlayerController.h"
#include "RC/Characters/Components/InventoryComponent.h"
#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Collectibles/Collectible.h"
#include "RC/Util/DataSingleton.h"
#include "RC/Util/RCStatics.h"
#include "RC/Weapons/Weapons/PlayerWeapons/BasePlayerWeapon.h"

//////////////////////////////////////////////////////////////////////////
// ARCCharacter

ARCCharacter::~ARCCharacter()
{
	// Reset the time dilation if its still going
	if (LevelUpTimer.IsActive())
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			AWorldSettings* WorldSettings = World->GetWorldSettings();
			if (WorldSettings != nullptr)
			{
				WorldSettings->SetTimeDilation(1);
			}
		}
	}
}

ARCCharacter::ARCCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create the trigger to detect collectibles
	CollectibleTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("CollectibleTrigger"));
	CollectibleTrigger->SetCollisionProfileName(URCStatics::CollectibleTrigger_ProfileName);
	CollectibleTrigger->SetupAttachment(RootComponent);

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	Inventory->OnWeaponEquipped().AddDynamic(this, &ARCCharacter::OnWeaponEquipped);

	// Stimulus for AI detection
	Stimulus = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("Stimuli Source"));
	ASSERT_RETURN(Stimulus != nullptr);
	Stimulus->RegisterForSense(TSubclassOf<UAISense_Sight>());
	Stimulus->RegisterWithPerceptionSystem();
}

// Save player components
void ARCCharacter::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (Ar.IsSaveGame())
	{
		if (Inventory != nullptr)
		{
			Ar << *Inventory;
		}
	}
}

// Called when the game starts or when spawned
void ARCCharacter::BeginPlay()
{
	Super::BeginPlay();

	ASSERT_RETURN(GEngine != nullptr);
	UDataSingleton* Singleton = Cast<UDataSingleton>(GEngine->GameSingleton);
	LevelDilationCurve = Singleton->LevelDilationCurve;

	CollectibleTrigger->OnComponentBeginOverlap.AddDynamic(this, &ARCCharacter::OnFoundCollectibleBeginOverlap);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ARCCharacter::OnCollectibleBeginOverlap);

	// Check if we're overlapping actors from the start
	TArray<AActor*> OverlappingActors;
	CollectibleTrigger->GetOverlappingActors(OverlappingActors, ACollectible::StaticClass());
	FHitResult EmptyResult;
	for (AActor* OverlappingActor : OverlappingActors)
	{
		OnFoundCollectibleBeginOverlap(nullptr, OverlappingActor, nullptr, 0, false, EmptyResult);
	}

	OverlappingActors.Empty();
	GetCapsuleComponent()->GetOverlappingActors(OverlappingActors, ACollectible::StaticClass());
	for (AActor* OverlappingActor : OverlappingActors)
	{
		OnCollectibleBeginOverlap(nullptr, OverlappingActor, nullptr, 0, false, EmptyResult);
	}
}

// Called every frame
void ARCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Slow-mo during a level up
	if (LevelUpTimer.IsActive())
	{
		UWorld* World = GetWorld();
		if (World != nullptr)
		{
			AWorldSettings* WorldSettings = World->GetWorldSettings();
			if (WorldSettings != nullptr)
			{
				ASSERT_RETURN(LevelDilationCurve != nullptr);
				float Dilation = LevelDilationCurve->GetFloatValue(LevelUpTimer.GetTimeSince());
				WorldSettings->SetTimeDilation(Dilation);
			}
		}
	}
	else if (LevelUpTimer.IsValid())
	{
		// Time has expired, invalidate it
		LevelUpTimer.Invalidate();
	}

	// Open weapon wheel if it's been held long enough
	if (WeaponSelectTimer.Elapsed())
	{
		ARCPlayerController* PlayerController = Cast<ARCPlayerController>(GetController());
		ASSERT_RETURN(PlayerController != nullptr);
		PlayerController->OpenWeaponSelect();

		WeaponSelectTimer.Invalidate();
	}
}

// Called when the character has given damage to someone else
void ARCCharacter::OnDamageGiven(const FDamageReceivedParams& Params)
{
	ARCPlayerState* State = GetPlayerState<ARCPlayerState>();
	ASSERT_RETURN(State != nullptr);

	UPlayerWeaponData* PlayerWeaponData = State->FindOrAddDataForAsset<UPlayerWeaponData>(Params.CauseId);
	ASSERT_RETURN(PlayerWeaponData != nullptr, "Weapon Data not able to be added");

	PlayerWeaponData->GrantDamageXP(Params.DamageDealt);
}

// Get the currently equipped weapon
ABaseWeapon* ARCCharacter::GetEquippedWeapon() const
{
	return Inventory != nullptr ? Inventory->GetEquippedWeapon() : nullptr;
}

// Setup the player inputs
void ARCCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	ASSERT_RETURN(PlayerInputComponent != nullptr);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARCCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARCCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARCCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARCCharacter::LookUpAtRate);

	// Weapon inputs
	PlayerInputComponent->BindAction("Wrench", IE_Pressed, this, &ARCCharacter::WrenchAttack);
	PlayerInputComponent->BindAction("FullAttack", IE_Pressed, this, &ARCCharacter::FullAttack);
	PlayerInputComponent->BindAction("FullAttack", IE_Released, this, &ARCCharacter::FullAttackStop);
	PlayerInputComponent->BindAction("HalfAttack", IE_Pressed, this, &ARCCharacter::HalfAttack);
	PlayerInputComponent->BindAction("HalfAttack", IE_Released, this, &ARCCharacter::HalfAttackStop);
	PlayerInputComponent->BindAxis("Attack", this, &ARCCharacter::Attack);	
	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &ARCCharacter::EquipNextWeapon);
	PlayerInputComponent->BindAction("PreviousWeapon", IE_Pressed, this, &ARCCharacter::EquipPreviousWeapon);

	// UI
	PlayerInputComponent->BindAction("SelectWeapon", IE_Pressed, this, &ARCCharacter::SelectWeapon);
	PlayerInputComponent->BindAction("SelectWeapon", IE_Released, this, &ARCCharacter::SelectWeaponEnd);
	PlayerInputComponent->BindAction("OpenPauseSettings", IE_Pressed, this, &ARCCharacter::OpenPauseSettings);
	PlayerInputComponent->BindAction("OpenPauseHUD", IE_Pressed, this, &ARCCharacter::OpenPauseHUD);
}

// Called via input for forwards/backward input
void ARCCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

// Called via input for side to side input
void ARCCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

// Called via input to turn at a given rate 
void ARCCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

// Called via input to turn look up/down at a given rate
void ARCCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

// Called via input to attack with the wrench
void ARCCharacter::WrenchAttack()
{
	ASSERT_RETURN(Inventory != nullptr);

	// Equip the wrench if it's not
	ABasePlayerWeapon* EquippedWeapon = nullptr;
	if (Inventory->GetEquippedWeaponSlot() != EInventorySlot::SlotWrench)
	{
		EquippedWeapon = Inventory->EquipSlot(EInventorySlot::SlotWrench);
	}
	else
	{
		EquippedWeapon = Inventory->GetEquippedWeapon();
	}

	if (EquippedWeapon == nullptr)
	{
		return;
	}

	EquippedWeapon->UpdateTriggerStatus(ETriggerStatus::FULL);
}

// Called via input to attack with full trigger
void ARCCharacter::FullAttack()
{
	bFullAttackHeld = true;
}

// Called via input to stop attacking with full trigger
void ARCCharacter::FullAttackStop()
{
	bFullAttackHeld = false;
}

// Called via input to attack with half trigger
void ARCCharacter::HalfAttack()
{
	bHalfAttackHeld = true;
}

// Called via input to stop attacking with half trigger
void ARCCharacter::HalfAttackStop()
{
	bHalfAttackHeld = false;
}

// Called via input to update attacking with the current weapon
void ARCCharacter::Attack(float Value)
{	
	ASSERT_RETURN(Inventory != nullptr);

	ETriggerStatus TriggerStatus = bFullAttackHeld ? ETriggerStatus::FULL : (bHalfAttackHeld ? ETriggerStatus::HALF : URCStatics::TriggerValueToStatus(Value));

	// If we had the wrench equipped
	ABasePlayerWeapon* EquippedWeapon = nullptr;
	if (Inventory->GetEquippedWeaponSlot() == EInventorySlot::SlotWrench)
	{
		// If we aren't triggering then do nothing
		if (TriggerStatus == ETriggerStatus::NONE)
		{
			return;
		}
		// Switch back to the last weapon
		EquippedWeapon = Inventory->EquipPreviousWeapon();
	}
	else
	{
		EquippedWeapon = Inventory->GetEquippedWeapon();
	}

	if (EquippedWeapon == nullptr) 
	{
		return;
	}

	EquippedWeapon->UpdateTriggerStatus(TriggerStatus);
}

// Called via input to equip the next weapon in our inventory
void ARCCharacter::EquipNextWeapon()
{
	ASSERT_RETURN(Inventory != nullptr);
	Inventory->EquipNextSlot();
}

// Called via input to equip the previous weapon in our inventory
void ARCCharacter::EquipPreviousWeapon()
{
	ASSERT_RETURN(Inventory != nullptr);
	Inventory->EquipPreviousSlot();
}

// Called via input to swap to previous weapon or open the weapon wheel
void ARCCharacter::SelectWeapon()
{	
	// If they hold it for long enough, open the weapon wheel,
	// otherwise switch to the last equipped weapon
	WeaponSelectTimer.Set(0.25f);
}

// Called via input once the swap weapon key isn't pressed
void ARCCharacter::SelectWeaponEnd()
{
	ASSERT_RETURN(Inventory != nullptr);
	Inventory->EquipNextSlot();
	WeaponSelectTimer.Invalidate();
}

// Called via input to open the pause HUD
void ARCCharacter::OpenPauseHUD()
{}

// Called via input to open the pause settings
void ARCCharacter::OpenPauseSettings()
{}

// Called when a weapon is equipped
void ARCCharacter::OnWeaponEquipped(ABasePlayerWeapon* Weapon, EInventorySlot Slot)
{
	if (Weapon != nullptr)
	{
		// Listen for the level up
		Weapon->OnLevelUp().AddDynamic(this, &ARCCharacter::OnWeaponLevelUp);
	}
}

/** Called when the equipped weapon levels up */
void ARCCharacter::OnWeaponLevelUp(ABasePlayerWeapon* Weapon, uint8 CurrentLevel)
{
	// Slow-mo
	if (LevelDilationCurve != nullptr)
	{
		float MinTime = 0, MaxTime = 0;
		LevelDilationCurve->GetTimeRange(MinTime, MaxTime);

		LevelUpTimer.Set(MaxTime);
	}
}

// Called when the character dies
void ARCCharacter::OnActorDied(AActor* Actor)
{
	Super::OnActorDied(Actor);

	ARCPlayerController* PlayerController = GetController<ARCPlayerController>();
	ASSERT_RETURN(PlayerController != nullptr);

	DisableInput(PlayerController);

	// Remove any trigger status to prevent the weapon from continuing firing
	if (Inventory != nullptr)
	{
		ABasePlayerWeapon* EquippedWeapon = Inventory->GetEquippedWeapon();
		if (EquippedWeapon != nullptr)
		{
			EquippedWeapon->UpdateTriggerStatus(ETriggerStatus::NONE);
		}
	}
}

// On collectible overlapping with the collectible trigger
void ARCCharacter::OnFoundCollectibleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	ACollectible* Collectible = Cast<ACollectible>(OtherActor);
	if (Collectible == nullptr)
	{
		return;
	}

	Collectible->StartCollecting(this);
}

// On collectible overlapping with the character trigger
void ARCCharacter::OnCollectibleBeginOverlap(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	ACollectible* Collectible = Cast<ACollectible>(OtherActor);
	if (Collectible == nullptr)
	{
		return;
	}

	ARCPlayerState* State = GetPlayerState<ARCPlayerState>();
	ASSERT_RETURN(State != nullptr);

	// Tell the player state to collect it as that's where the data is stored
	State->CollectCollectible(Collectible);
}
