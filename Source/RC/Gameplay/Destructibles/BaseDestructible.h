// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "RC/Save/SaveGameInterface.h"
#include "RC/Framework/DamageInterface.h"

#include "BaseDestructible.generated.h"

UCLASS()
class RC_API ABaseDestructible : public AActor, public IDamageInterface, public ISaveGameInterface
{
	GENERATED_BODY()
	
public:	
	ABaseDestructible();

	// Save destructible status
	void Serialize(FArchive& Ar) override;

	/**
	 * Request for this character to be damaged
	 * @param Params	The request params
	 */
	void RequestDamage(FDamageRequestParams& Params) override;

	/**
	 *	Sets the actor to be hidden in the game
	 *	@param	bNewHidden	Whether or not to hide the actor and all its components
	 */
	void SetActorHiddenInGame(bool bNewHidden) override;

	/** Returns Destructible subobject **/
	FORCEINLINE class UDestructibleComponent* GetDestructible() const { return Destructible; }

	/** Returns Mesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetMesh() const { return Mesh; }

private:
	/** Destructible */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Destructible, meta = (AllowPrivateAccess = "true"))
	class UDestructibleComponent* Destructible;

	/** Mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh;
};
