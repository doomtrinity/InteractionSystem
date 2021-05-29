// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractiveBoxComponent.h"
#include "GameFramework/Pawn.h"
#include "InteractionSystem.h"
#include "InteractiveActor.h"
#include "Net/UnrealNetwork.h"

#define LOCTEXT_NAMESPACE "InteractionSystem"

UInteractiveBoxComponent::UInteractiveBoxComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	PrimaryComponentTick.bCanEverTick = true;
	
	// actor (owner) must replicate too, and should always be relevant
	//bReplicates = true; // 4.22
	SetIsReplicatedByDefault(true); // 4.26

	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(COLLISION_INTERACTIVE, ECR_Block);
	BoxExtent = FVector(16.0f, 16.0f, 16.0f);

	bInteractionDisabled = false;
}

void UInteractiveBoxComponent::TryInteract(APawn* Interactor) 
{
	const bool bInteractionDisabled_ = IInteractive::Execute_IsInteractionDisabled(this);
	if (false == bInteractionDisabled_)
	{
		const bool bCanInteract = IInteractive::Execute_CanInteract(this, Interactor);
		IInteractive::Execute_OnInteract(this, Interactor, bCanInteract);
	}
}

void UInteractiveBoxComponent::OnInteract_Implementation(APawn* Interactor, bool bCanInteract)
{
	const bool bFromReplication = LastInteraction.bFromRep;
	if (bFromReplication || (false == CurrentInteractor.IsValid() && Interactor && Interactor->HasAuthority()))
	{
		CurrentInteractor = Interactor;

		if (IsOwnerInteractive())
		{
			if (bCanInteract)
			{
				IInteractiveActor::Execute_OnInteractionSucceeded(GetOwner(), this, Interactor);
			}
			else
			{
				IInteractiveActor::Execute_OnInteractionDenied(GetOwner(), this, Interactor);
			}
		}

		LastInteraction.bCanInteract = bCanInteract;
		LastInteraction.Interactor = Interactor;
		LastInteraction.bStopInteraction = false;
		LastInteraction.EnsureReplication();
	}

}

void UInteractiveBoxComponent::OnStopInteraction_Implementation(APawn* Interactor)
{
	const bool bFromReplication = LastInteraction.bFromRep;
	if (bFromReplication || CurrentInteractor.IsValid() && CurrentInteractor == Interactor)
	{
		CurrentInteractor = nullptr;

		if (IsOwnerInteractive())
		{
			IInteractiveActor::Execute_OnStopInteraction(GetOwner(), this, Interactor);
		}

		LastInteraction.Interactor = Interactor;
		LastInteraction.bStopInteraction = true;
		LastInteraction.EnsureReplication();
	}

}

void UInteractiveBoxComponent::OnFocusReceived_Implementation(APawn* Interactor)
{
	if (IsOwnerInteractive())
	{
		IInteractiveActor::Execute_OnFocusReceived(GetOwner(), this, Interactor);
	}
}

void UInteractiveBoxComponent::OnFocusLost_Implementation(APawn* Interactor)
{
	if (IsOwnerInteractive())
	{
		IInteractiveActor::Execute_OnFocusLost(GetOwner(), this, Interactor);
	}
}

bool UInteractiveBoxComponent::CanInteract_Implementation(const APawn* Interactor) const 
{
	if (ShouldUseActorImplementation()) 
	{
		return IInteractiveActor::Execute_CanInteract(GetOwner(), this, Interactor);
	}
	return false == IInteractive::Execute_IsInteractionDisabled(this);
}

FText UInteractiveBoxComponent::GetMessage_Implementation(const APawn* Interactor) const 
{
	if (ShouldUseActorImplementation())
	{
		return IInteractiveActor::Execute_GetMessage(GetOwner(), this, Interactor);
	}

	const bool bInteractionDisabled_ = IInteractive::Execute_IsInteractionDisabled(this);

	return bInteractionDisabled_ ? LOCTEXT("InteractionDisabled", "INTERACTION DISABLED") : LOCTEXT("Interact", "INTERACT");
}

bool UInteractiveBoxComponent::IsInteractionDisabled_Implementation() const 
{
	if (ShouldUseActorImplementation())
	{
		return IInteractiveActor::Execute_IsInteractionDisabled(GetOwner(), this);
	}
	
	return bInteractionDisabled;
}

bool UInteractiveBoxComponent::ShouldUseActorImplementation() const
{
	if (IsOwnerInteractive())
	{
		return IInteractiveActor::Execute_ShouldUseActorImplementation(GetOwner());
	}

	return false;
}

bool UInteractiveBoxComponent::IsOwnerInteractive() const 
{
	return GetOwner() && GetOwner()->GetClass()->ImplementsInterface(UInteractiveActor::StaticClass());
}

void UInteractiveBoxComponent::SetInteractionDisabled(bool bDisabled)
{
	bInteractionDisabled = bDisabled;
}

void UInteractiveBoxComponent::OnRep_LastInteraction() 
{
	// we shouldn't call this fucntion on server (could check if owner is simulated proxy to be sure...)
	LastInteraction.bFromRep = true;
	if (LastInteraction.bStopInteraction) 
	{
		IInteractive::Execute_OnStopInteraction(this, LastInteraction.Interactor.Get());
	}
	else 
	{
		IInteractive::Execute_OnInteract(this, LastInteraction.Interactor.Get(), LastInteraction.bCanInteract);
	}
	// reset flag
	LastInteraction.bFromRep = false;
}

void UInteractiveBoxComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractiveBoxComponent, bInteractionDisabled);
	DOREPLIFETIME(UInteractiveBoxComponent, LastInteraction);
	
}

FInteractionData::FInteractionData()
	: Interactor(NULL)
	, bCanInteract(false)
	, bStopInteraction(false)
	, bFromRep(false)
	, EnsureReplicationByte(0)
{}


void FInteractionData::EnsureReplication()
{
	EnsureReplicationByte++;
}

#undef LOCTEXT_NAMESPACE