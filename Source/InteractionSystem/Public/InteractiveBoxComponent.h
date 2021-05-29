// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Interactive.h"
#include "InteractiveBoxComponent.generated.h"

class APawn;

USTRUCT()
struct FInteractionData
{
	GENERATED_USTRUCT_BODY()

public:

	FInteractionData();

	void EnsureReplication();

	UPROPERTY()
	TWeakObjectPtr<class APawn> Interactor;

	UPROPERTY()
	uint32 bCanInteract : 1;

	UPROPERTY()
	uint32 bStopInteraction : 1;

	UPROPERTY(NotReplicated)
	uint32 bFromRep : 1;

private:

	UPROPERTY()
	uint8 EnsureReplicationByte;

};

/**
* BoxComponent-based implementation of IInteractive interface, with built-in replication.
* If you're going to add this component or a subclass of this component in your custom actor, you'll likely want to implement the IInteractiveActor interface in that actor. 
* You must set the actor to replicate, and it should always be relevant (optional steps, if you need to handle replication).
*
* A general tip about replication. You shouldn't handle things that require some control on synchronization over time, like a moving mesh, 
* through the replicated events of this component, but you should replicate such things in a different way (see mover timeline in BP_Mover),
* and take advantage of the built-in event replication only to handle fire and forget things or cosmetic stuff.
*
* See IInteractive
* See IInteractiveActor
*/
UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class INTERACTIONSYSTEM_API UInteractiveBoxComponent : public UBoxComponent, public IInteractive
{
	GENERATED_BODY()

public:
	UInteractiveBoxComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin UObject Interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~ End UObject Interface

private:
	// let the interactive component to be used by one pawn only at a time, property used on server only
	TWeakObjectPtr<APawn> CurrentInteractor;

	/**
	* See SetInteractionDisabled
	*/
	UPROPERTY(EditAnywhere, Replicated)
	bool bInteractionDisabled;

	UPROPERTY(ReplicatedUsing=OnRep_LastInteraction)
	FInteractionData LastInteraction;

	/**
	* does owner implement IInteractiveActor interface?
	*/
	bool IsOwnerInteractive() const;

	/**
	* does owner implement IInteractiveActor interface, and IInteractiveActor::ShouldUseActorImplementation return true?
	*/
	bool ShouldUseActorImplementation() const;

	UFUNCTION()
	void OnRep_LastInteraction();

// ~Begin IInteractive Interface

protected:

	/**
	* [server]
	*/
	virtual void TryInteract(APawn* Interactor) override;

	/**
	* [all]
	*/
	virtual void OnInteract_Implementation(APawn* Interactor, bool bCanInteract) override;
	
	/**
	* [all]
	*/
	virtual void OnStopInteraction_Implementation(APawn* Interactor) override;

	/**
	* [local]
	*/
	virtual void OnFocusReceived_Implementation(APawn* Interactor) override;
	
	/**
	* [local]
	*/
	virtual void OnFocusLost_Implementation(APawn* Interactor) override;
	
public:

	/**
	* [server]
	*/
	virtual bool CanInteract_Implementation(const APawn* Interactor) const override;
	
	/**
	* [local]
	*/
	virtual FText GetMessage_Implementation(const APawn* Interactor) const override;
	
	/**
	* [local + server]
	*/
	virtual bool IsInteractionDisabled_Implementation() const override;

// ~End IInteractive Interface

public:

	/**
	* See bInteractionDisabled
	*/
	UFUNCTION(BlueprintCallable)
	void SetInteractionDisabled(bool bDisabled);



};
