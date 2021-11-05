// Fill out your copyright notice in the Description page of Project Settings.
#include "MaskableInputComponent.h"

#include "RC/Characters/InputMaskInfo.h"
#include "RC/Debug/Debug.h"

// Apply the mask
void UMaskableInputComponent::PushMask(const UInputMaskInfo* MaskInfo, const FName& Setter)
{
	LOG_RETURN(MaskInfo != nullptr, LogInput, Error, "No input mask given");

	// Are there any actions to mask
	if (MaskInfo->GetActionMasks().Num() != 0)
	{
		// Is this mask already applied
		FSetterActionIndecies* FoundSetterActionIndecies = MaskActionIndecies.Find(MaskInfo->GetHandle());
		if (FoundSetterActionIndecies != nullptr)
		{
			// If this mask is already applied, just push this setter
			FoundSetterActionIndecies->PushSetter(Setter);
		}
		else
		{
			// Otherwise we need to make the mask
			FSetterActionIndecies& SetterActionIndecies = MaskActionIndecies.Emplace(MaskInfo->GetHandle(), FSetterActionIndecies(Setter));
			SetterActionIndecies.ActionIndecies.Reserve(MaskInfo->GetActionMasks().Num() * static_cast<int>(EInputEvent::IE_MAX));

			// Bind to each action
			for (const FName& Action : MaskInfo->GetActionMasks())
			{
				// Bind to each input invent
				for (uint8 InputEvent = 0; InputEvent < static_cast<int>(EInputEvent::IE_MAX); ++InputEvent)
				{
					FInputActionBinding& ActionBinding = BindAction(Action, static_cast<EInputEvent>(InputEvent), this, &UMaskableInputComponent::EmptyActionCallback);
					ActionBinding.bConsumeInput = true;
					ActionBinding.bExecuteWhenPaused = true;

					SetterActionIndecies.ActionIndecies.Add(ActionBinding.GetHandle());
				}
			}
		}
	}

	// Bind to each axis
	for (const FName& Axis : MaskInfo->GetAxisMasks())
	{
		FInputAxisBinding& AxisBinding = BindAxis(Axis);
		AxisBinding.bConsumeInput = true;
		AxisBinding.bExecuteWhenPaused = true;
	}
}

// Remove the mask
void UMaskableInputComponent::PopMask(const UInputMaskInfo* MaskInfo, const FName& Setter)
{
	LOG_RETURN(MaskInfo != nullptr, LogInput, Error, "No input mask given");

	// Remove each action binding
	FSetterActionIndecies* SetterActionIndecies = MaskActionIndecies.Find(MaskInfo->GetHandle());
	if (SetterActionIndecies == nullptr)
	{
		ASSERT(MaskInfo->GetActionMasks().Num() == 0, "Trying to pop mask %s from setter %s when it wasn't set", MaskInfo->GetFName(), Setter);
	}
	else 
	{
		SetterActionIndecies->PopSetter(Setter);

		// If there are no more references to this mask
		if (!SetterActionIndecies->HasSetters())
		{
			for (const int32 ActionIndex : SetterActionIndecies->ActionIndecies)
			{
				RemoveActionBindingForHandle(ActionIndex);
			}
			MaskActionIndecies.Remove(MaskInfo->GetHandle());
		}		
	}

	// Remove each axis binding
	for (const FName& Axis : MaskInfo->GetAxisMasks())
	{
		// Delay the shrink until everything is removed
		for (int32 AxisIndex = AxisBindings.Num() - 1; AxisIndex >= 0; --AxisIndex)
		{
			if (AxisBindings[AxisIndex].AxisName == Axis)
			{
				AxisBindings.RemoveAt(AxisIndex, 1, false);
				// Need to break in case there's more than one mask for this axis
				break;
			}
		}
		AxisBindings.Shrink();
	}
}
