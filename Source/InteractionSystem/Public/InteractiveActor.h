// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractiveActor.generated.h"

class UInteractiveBoxComponent;
class APawn;

/**
* This interface is intended to be implemented by actors with interactive components of type UInteractiveBoxComponent.
* It's similar to the IInteractive interface, and it will be used by UInteractiveBoxComponent (the component implementation of IInteractive).
* The reason for using this interface is to provide a way to handle any interaction validation directly in the actor, instead of component,
* since an actor with interactive components is supposed to be the "master" entity that knows how to deal with interaction, depending on its internal state,
* especially when such an actor has multiple interactive components (see BP_Mover blueprint).
* In order for actors to handle interaction validation, you must override ShouldUseActorImplementation and set its return value to true.
* The validation functions are CanInteract and IsInteractionDisabled. Please note that you must not call the IInteractive "counterpart" from the
* actor function, e.g. calling component's CanInteract from actor's CanInteract, will result in an infinite loop.
* That happens because the functions in InteractiveBoxComponent call the matching functions in the actor, if the actor implementation is used (see UInteractiveBoxComponent.cpp).
*/
UINTERFACE(MinimalAPI)
class UInteractiveActor : public UInterface
{
	GENERATED_BODY()
};

class INTERACTIONSYSTEM_API IInteractiveActor
{
	GENERATED_BODY()

	
public:

	/**
	* [all] Fires when locally controlled player attempts to interact with the interactive component (i.e. presses the button on keyboard),
	* and interaction is allowed (CanInteract returns true).
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnInteractionSucceeded(UInteractiveBoxComponent* InteractiveComponent, APawn* Interactor);

	/**
	* [all] Fires when locally controlled player attempts to interact with the interactive component (i.e. presses the button on keyboard), 
	* but interaction is denied (CanInteract returns false).
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnInteractionDenied(UInteractiveBoxComponent* InteractiveComponent, APawn* Interactor);

	/**
	* [all] See IInteractive::OnStopInteraction
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnStopInteraction(UInteractiveBoxComponent* InteractiveComponent, APawn* Interactor);

	/**
	* [local] See IInteractive::OnFocusReceived
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnFocusReceived(UInteractiveBoxComponent* InteractiveComponent, APawn* Interactor);

	/**
	* [local] See IInteractive::OnFocusLost
	*/
	UFUNCTION(BlueprintNativeEvent)
	void OnFocusLost(UInteractiveBoxComponent* InteractiveComponent, APawn* Interactor);

	/**
	* [server] See IInteractive::CanInteract
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanInteract(const UInteractiveBoxComponent* InteractiveComponent, const APawn* Interactor) const;

	/**
	* [local] See IInteractive::GetMessage
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FText GetMessage(const UInteractiveBoxComponent* InteractiveComponent, const APawn* Interactor) const;

	/**
	* [local + server] See IInteractive::IsInteractionDisabled
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsInteractionDisabled(const UInteractiveBoxComponent* InteractiveComponent) const;

	/**
	* [server] You need to override this function, which must return true, in order for interactive component
	* to use actor's CanInteract, IsInteractionDisabled, GetMessage functions. 
	* These functions will then "override" component functions.
	* You shouldn't implement any custom logic in here, you just need to flag the return value to true,
	* and you shouldn't change the return value at runtime, this is why I marked it as server, but clients use it too.
	* When actor implementation is used you must not call component functions from actor functions,
	* e.g. actor GetMessage must not call component GetMessage, or you'll get an infinite loop.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool ShouldUseActorImplementation() const;

};
