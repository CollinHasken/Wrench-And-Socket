// Fill out your copyright notice in the Description page of Project Settings.
#include "InputMaskInfo.h"

// Input mask UID
static uint8 GInputHandle = UInputMaskInfo::INVALID_MASK_HANDLE;

UInputMaskInfo::UInputMaskInfo()
{
	// The CDO doesn't need a handle
	if (!HasAnyFlags(EObjectFlags::RF_ClassDefaultObject))
	{
		Handle = ++GInputHandle;
	}
}
