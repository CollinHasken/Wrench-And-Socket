// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "RC/Util/RCTypes.h"

#include "HealthComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RC_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	friend FArchive& operator<<(FArchive& Ar, UHealthComponent& SObj);

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Apply damage requested
	 * @param DamageParams	Params for the damage to apply
	 * 
	 * Returns the actual damage inflicted
	 */
	UFUNCTION(BlueprintCallable)
	float ApplyDamage(const FDamageRequestParams& DamageParams);

	/** Once we've ran out of health */
	void OnKilled();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health, meta = (AllowPrivateAccess = "true"))
	int MaxHealth = 100;

	UPROPERTY(BlueprintReadOnly , Category = Health, SaveGame, meta = (AllowPrivateAccess = "true"))
	int CurrentHealth = 100;
};
