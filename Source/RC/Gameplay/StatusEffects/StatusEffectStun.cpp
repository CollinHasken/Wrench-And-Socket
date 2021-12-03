// Fill out your copyright notice in the Description page of Project Settings.
#include "StatusEffectStun.h"

#include "RC/AI/BaseAIController.h"
#include "RC/Characters/Components/MaskableInputComponent.h"
#include "RC/Characters/Player/RCCharacter.h"
#include "RC/Characters/Player/RCPlayerController.h"
#include "RC/Util/RCStatics.h"

// Called when the first stack has been added
void UStatusEffectStun::OnAdded_Implementation()
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(Owner);
	if (Character == nullptr)
	{
		return;
	}

	ARCCharacter* Player = Cast<ARCCharacter>(Character);
	// Disable input for the player
	if (Player != nullptr)
	{
		ARCPlayerController* PlayerController = Player->GetController<ARCPlayerController>();
		ASSERT_RETURN(PlayerController != nullptr);

		UMaskableInputComponent* MaskableInput = PlayerController->GetMaskableInput();
		ASSERT_RETURN(MaskableInput != nullptr);
		
		MaskableInput->PushMask(StunMask, "UStatusEffectStun::OnAdded_Implementation");
	}
	else
	{
		// Tell enemy AI they're stunned
		ABaseAIController* EnemyController = Cast<ABaseAIController>(Character->GetController());
		if (EnemyController != nullptr)
		{
			EnemyController->StunAI();
		}
	}

	// Play stun montage
	if (StunMontage != nullptr)
	{
		UAnimInstance* AnimInstance = Character->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(StunMontage);
		}
	}
}

// Called when the last stack has been removed 
void UStatusEffectStun::OnRemoved_Implementation()
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(Owner);
	if (Character == nullptr)
	{
		return;
	}

	ARCCharacter* Player = Cast<ARCCharacter>(Character);
	// Re-enable input for the player
	if (Player != nullptr)
	{
		ARCPlayerController* PlayerController = Player->GetController<ARCPlayerController>();
		if (PlayerController != nullptr)
		{
			UMaskableInputComponent* MaskableInput = PlayerController->GetMaskableInput();
			ASSERT_RETURN(MaskableInput != nullptr);

			MaskableInput->PopMask(StunMask, "UStatusEffectStun::OnAdded_Implementation");			
		}
	}
	else
	{
		// Remove stun flag
		ABaseAIController* EnemyController = Cast<ABaseAIController>(Character->GetController());
		if (EnemyController != nullptr)
		{
			EnemyController->UnstunAI();
		}
	}

	// Stop playing the montage
	if (StunMontage != nullptr)
	{
		UAnimInstance* AnimInstance = Character->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Stop(0.5f, StunMontage);
		}
	}
}
