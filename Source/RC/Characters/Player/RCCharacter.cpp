// Copyright Epic Games, Inc. All Rights Reserved.

#include "RCCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

#include "RC/Debug/Debug.h"
#include "RC/Characters/Components/InventoryComponent.h"
#include "RC/Characters/Player/RCPlayerState.h"
#include "RC/Util/DataSingleton.h"
#include "RC/Weapons/Weapons/BasePlayerWeapon.h"

//////////////////////////////////////////////////////////////////////////
// ARCCharacter

ARCCharacter::~ARCCharacter()
{
	// Reset the time dilation
	if (LevelUpTimer.IsActive())
	{
		GetWorld()->GetWorldSettings()->SetTimeDilation(1);		
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

// Called every frame
void ARCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LevelUpTimer.IsActive())
	{
		ASSERT_RETURN(LevelDilationCurve != nullptr);
		float Dilation = LevelDilationCurve->GetFloatValue(LevelUpTimer.GetTimeSince());

		GetWorld()->GetWorldSettings()->SetTimeDilation(Dilation);		
	}
	else if (LevelUpTimer.IsValid())
	{
		// Time has expired, invalidate it
		LevelUpTimer.Invalidate();
	}
}

void ARCCharacter::OnDamageGiven(const FDamageReceivedParams& Params)
{
	ARCPlayerState* State = GetPlayerState<ARCPlayerState>();
	ASSERT_RETURN(State != nullptr);

	FWeaponData& WeaponData = State->FindOrAddWeaponDataForClass(Params.WeaponClass);
	WeaponData.GrantDamageXP(Params.DamageDealt);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ARCCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("NextWeapon", IE_Released, this, &ARCCharacter::EquipNextWeapon);
	PlayerInputComponent->BindAction("PreviousWeapon", IE_Released, this, &ARCCharacter::EquipPreviousWeapon);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARCCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARCCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ARCCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ARCCharacter::LookUpAtRate);

	// Shooting
	PlayerInputComponent->BindAxis("Shoot", this, &ARCCharacter::Shoot);
}

void ARCCharacter::BeginPlay()
{
	Super::BeginPlay();

	ASSERT_RETURN(GEngine != nullptr);
	UDataSingleton* Singleton = Cast<UDataSingleton>(GEngine->GameSingleton);
	LevelDilationCurve = Singleton->LevelDilationCurve;
}

void ARCCharacter::EquipNextWeapon()
{
	ASSERT_RETURN(Inventory != nullptr);
	Inventory->EquipNextSlot();
}

void ARCCharacter::EquipPreviousWeapon()
{
	ASSERT_RETURN(Inventory != nullptr);
	Inventory->EquipPreviousSlot();
}

void ARCCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ARCCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

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

void ARCCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
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

void ARCCharacter::Shoot(float Value)
{	
	ASSERT_RETURN(Inventory != nullptr);

	ABasePlayerWeapon* EquippedWeapon = Inventory->GetEquippedWeapon();
	if (EquippedWeapon == nullptr) {
		return;
	}

	EquippedWeapon->UpdateTriggerStatus(ABasePlayerWeapon::TriggerValueToStatus(Value));
}

void ARCCharacter::OnWeaponEquipped(ABasePlayerWeapon* Weapon)
{
	if (Weapon != nullptr)
	{
		Weapon->OnLevelUp().BindUObject(this, &ARCCharacter::OnWeaponLevelUp);
	}
}

/** Called when the equipped weapon levels up */
void ARCCharacter::OnWeaponLevelUp(ABasePlayerWeapon* Weapon)
{
	// Slow-mo
	if (LevelDilationCurve != nullptr)
	{
		float MinTime = 0, MaxTime = 0;
		LevelDilationCurve->GetTimeRange(MinTime, MaxTime);

		LevelUpTimer.Set(MaxTime);
	}
}

void ARCCharacter::OnActorDied(AActor* Actor)
{
	Super::OnActorDied(Actor);

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	ASSERT_RETURN(PlayerController != nullptr);

	DisableInput(PlayerController);
}
