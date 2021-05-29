// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"
#include "Interactive.h"
#include "InteractionSystem.h"
#include "TimerManager.h"
#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"

APlayerPawn::APlayerPawn()
{
 	CurrentInteractive = nullptr;
	MaxInteractionDistance = 100.f;

}

void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(TimerHandle_FindInteractive,
		this,
		&APlayerPawn::FindInteractive,
		0.128f, // rate
		true, // loop
		1.f // first delay
	);
	
}

void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TryStopInteraction();
}

void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// interaction
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerPawn::InteractPressed);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &APlayerPawn::InteractReleased);

	// movement
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerPawn::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

}

void APlayerPawn::InteractPressed()
{
	Interact(GetCurrentInteractive());
}

void APlayerPawn::InteractReleased()
{
	StopInteraction(GetCurrentInteractive());
}

void APlayerPawn::FindInteractive()
{
	UObject* Interactive = nullptr;
	const APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->IsLocalController())
	{
		const APlayerCameraManager* Camera = PC->PlayerCameraManager;
		if (Camera)
		{
			FCollisionQueryParams LineParams(SCENE_QUERY_STAT(FindInteractive), true);
			LineParams.AddIgnoredActor(this);

			// line trace
			FHitResult OutHit;
			const FVector TargetPoint = Camera->GetCameraLocation() + Camera->GetActorForwardVector() * MaxInteractionDistance;
			const bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Camera->GetCameraLocation(), TargetPoint, COLLISION_INTERACTIVE, LineParams);
			if (bHit && OutHit.Component.IsValid() && OutHit.Component->GetClass()->ImplementsInterface(UInteractive::StaticClass()))
			{
				Interactive = Cast<UObject>(OutHit.Component);
				if (Interactive)
				{
					const bool bInteractionDisabled = IInteractive::Execute_IsInteractionDisabled(Interactive);
					if (bInteractionDisabled)
					{
						Interactive = nullptr;
					}
				}
			}
		}
	}

	if (Interactive != CurrentInteractive)
	{
		// update cached value, call focus events
		if (Interactive != nullptr)
		{
			IInteractive::Execute_OnFocusReceived(Interactive, this);
		}
		if (CurrentInteractive.IsValid())
		{
			// force stop interaction on focus lost, should refactor this if we want to keep the interaction active (maybe by adding a new method "ShouldStopInteraction" in IInteractive interface)
			StopInteraction(CurrentInteractive.Get());
			IInteractive::Execute_OnFocusLost(CurrentInteractive.Get(), this);
		}
		CurrentInteractive = Interactive;
		
	}
}

void APlayerPawn::TryStopInteraction()
{
	if (CurrentInteractive.IsValid())
	{
		const bool bInteractionDisabled = IInteractive::Execute_IsInteractionDisabled(CurrentInteractive.Get());
		if (bInteractionDisabled)
		{
			StopInteraction(CurrentInteractive.Get());
			IInteractive::Execute_OnFocusLost(CurrentInteractive.Get(), this);
			CurrentInteractive = nullptr;
		}
	}

}

UObject* APlayerPawn::GetCurrentInteractive() const
{
	return CurrentInteractive.IsValid() ? CurrentInteractive.Get() : nullptr;
}

void APlayerPawn::Interact(UObject* Target)
{
	if (Target == nullptr)
	{
		return;
	}
	if (false == HasAuthority())
	{
		ServerInteract(Target);
		return;
	}

	IInteractive::Interact(Target, this);
}

void APlayerPawn::StopInteraction(UObject* Target)
{
	if (Target == nullptr)
	{
		return;
	}
	if (false == HasAuthority())
	{
		ServerStopInteraction(Target);
		return;
	}

	IInteractive::StopInteraction(Target, this);
}

bool APlayerPawn::ServerInteract_Validate(UObject* Target)
{
	return true;
}

void APlayerPawn::ServerInteract_Implementation(UObject* Target)
{
	Interact(Target);
}

bool APlayerPawn::ServerStopInteraction_Validate(UObject* Target)
{
	return true;
}

void APlayerPawn::ServerStopInteraction_Implementation(UObject* Target)
{
	StopInteraction(Target);
}

void APlayerPawn::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}


void APlayerPawn::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorRightVector(), Value);
	}
}