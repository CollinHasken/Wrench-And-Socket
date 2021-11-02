// Fill out your copyright notice in the Description page of Project Settings.


#include "RCTypes.h"


// Grant an amount of the collectible
void UCollectibleData::GrantCollectible(int Amount)
{
	CurrentAmount += Amount;	
}
