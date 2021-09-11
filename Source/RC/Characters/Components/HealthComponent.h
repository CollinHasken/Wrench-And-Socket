// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "RC/Util/RCTypes.h"

#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActorDamaged, class AActor*, Actor, float, CurrentHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorDied, class AActor*, Actor);

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

	/** Get the Actor Damaged delegate */
	FOnActorDamaged& OnActorDamaged() { return ActorDamagedDelegate; }

	/** Get the Actor Died delegate */
	FOnActorDied& OnActorDied() { return ActorDiedDelegate; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health, meta = (AllowPrivateAccess = "true"))
	int MaxHealth = 100;

	UPROPERTY(BlueprintReadOnly , Category = Health, SaveGame, meta = (AllowPrivateAccess = "true"))
	int CurrentHealth = 100;

	UPROPERTY(BlueprintReadOnly, Category = Health, meta = (AllowPrivateAccess = "true"))
	bool bIsDead = false;

	UPROPERTY(BlueprintAssignable, Category = Health, meta = (AllowPrivateAccess))
	FOnActorDamaged ActorDamagedDelegate;

	UPROPERTY(BlueprintAssignable, Category = Health, meta = (AllowPrivateAccess))
	FOnActorDied ActorDiedDelegate;

	/** Once we've ran out of health */
	void OnKilled();
};
