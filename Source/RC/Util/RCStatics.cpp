// Fill out your copyright notice in the Description page of Project Settings.

#include "RCStatics.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

bool URCStatics::IsActorPlayer(const AActor* Actor)
{
	return Actor != nullptr && Actor == static_cast<AActor*>(UGameplayStatics::GetPlayerCharacter(Actor, 0));
}
