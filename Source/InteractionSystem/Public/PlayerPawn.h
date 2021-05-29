// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerPawn.generated.h"

UCLASS()
class INTERACTIONSYSTEM_API APlayerPawn : public ACharacter
{
	GENERATED_BODY()

public:
	
	APlayerPawn();

protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = InteractionSystem)
	float MaxInteractionDistance;

public:	
	
	virtual void Tick(float DeltaTime) override;

	// TODO make sure to forcibly stop interaction on unpossess, so interactive component has a chance to immediately clear its reference to the current interactor
	// not 100% sure if UnPossessed is the right place for this
	//virtual void UnPossessed() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/**
	Get the interactive component the player is looking at if any, or nullptr otherwise.
	This is intended for locally controlled players only to handle HUD stuff, non-locally controlled players will always return nullptr.
	*/
	UFUNCTION(BlueprintCallable)
	UObject* GetCurrentInteractive() const;

private:

	TWeakObjectPtr<UObject> CurrentInteractive;
	
	FTimerHandle TimerHandle_FindInteractive;
	
	void FindInteractive();
	
	void TryStopInteraction();
	
	void InteractPressed();
	
	void InteractReleased();

	void Interact(UObject* Target);
	void StopInteraction(UObject* Target);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerInteract(UObject* Target);

	UFUNCTION(Reliable, Server, WithValidation)
	void ServerStopInteraction(UObject* Target);

protected:

	// movement stuff
	void MoveForward(float Value);
	void MoveRight(float Value);

};
