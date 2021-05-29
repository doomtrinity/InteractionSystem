// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactive.generated.h"

class APawn;

/**
* Implement this interface in a component of an actor that should handle player interaction (e.g. light switches, doors, levers, etc.).
* In order for the player to detect an interactive component, the component must have a collision setup that blocks "Interactive" trace channel. 
*
* Please note that this interface can also be implemented by actors, but the system does not actually handle that case. 
* Adapting the code to support that should be pretty straightforward though, 
* I just didn't need it - and there's a specific interface for actors, see IInteractiveActor.
*
* You can find an implementation of this interface in UInteractiveBoxComponent, which you can subclass or use directly as a component of your actor. 
*
* See UInteractiveBoxComponent
* See IInteractiveActor
* See PlayerPawn (for interactive component detection)
*/
UINTERFACE(MinimalAPI, BlueprintType)
class UInteractive : public UInterface
{
	GENERATED_BODY()
};

class INTERACTIONSYSTEM_API IInteractive
{
	GENERATED_BODY()

protected:

	/**
	* [server] player wants to interact (i.e. pressed the button on keyboard)
	*/
	virtual void TryInteract(APawn* Interactor) = 0;

	/**
	* [server] process interaction, depending on whether the player can interact or not  (bCanInteract true -> succeed, false -> deny).
	* The implementation calls this event from TryInteract, and bCanInteract holds the return value of CanInteract() function
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnInteract(APawn* Interactor, bool bCanInteract);

	/**
	* [server] Fire in the following cases:
	* on focus lost;
	* interaction is disabled (IsInteractionDisabled returns true);
	* when locally controlled player aborts interaction (i.e. when button on keyboard is released).
	*
	* You'll likely need this for press and hold functionality.
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnStopInteraction(APawn* Interactor);

	/**
	* [local] locally controlled player started looking at interactive component
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnFocusReceived(APawn* Interactor);

	/**
	* [local] locally controlled player is not looking at interactive component anymore, or interaction has stopped
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnFocusLost(APawn* Interactor);

public:

	/**
	* [server] helper function that calls TryInteract
	*/
	static void Interact(UObject* Target, APawn* Interactor);

	/**
	* [server] helper function that calls OnStopInteraction
	*/
	static void StopInteraction(UObject* Target, APawn* Interactor);

	/**
	* [server] check whether player can interact with component.
	* Can be called on client too, just make sure to replicate the actor state.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanInteract(const APawn* Interactor) const;

	/**
	* [local] Get the interaction message to show on HUD. Make sure to replicate the actor state if locally controlled player is client.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FText GetMessage(const APawn* Interactor) const;

	/**
	* [local + server] check whether interaction is disabled, which means no focus and no interaction events, if true.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsInteractionDisabled() const;
};
