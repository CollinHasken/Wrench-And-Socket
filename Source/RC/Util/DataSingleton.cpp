// Fill out your copyright notice in the Description page of Project Settings.


#include "DataSingleton.h"

UDataSingleton& UDataSingleton::Get()
{
	UDataSingleton* Singleton = Cast<UDataSingleton>(GEngine->GameSingleton);

	if (Singleton)
	{
		return *Singleton;
	}
	else
	{
		return *NewObject<UDataSingleton>(UDataSingleton::StaticClass());
	}
}