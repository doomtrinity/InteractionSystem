// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactive.h"

void IInteractive::Interact(UObject* Target, APawn* Interactor)
{
	IInteractive* Interactive = Cast<IInteractive>(Target);
	if (Interactive)
	{
		Interactive->TryInteract(Interactor);
	}
}

void IInteractive::StopInteraction(UObject* Target, APawn* Interactor)
{
	if (Target == nullptr || false == Target->GetClass()->ImplementsInterface(UInteractive::StaticClass()))
	{
		// log error and return
		return;
	}

	IInteractive::Execute_OnStopInteraction(Target, Interactor);
}
