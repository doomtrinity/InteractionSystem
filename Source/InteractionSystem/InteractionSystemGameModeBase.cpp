// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionSystemGameModeBase.h"
#include "PlayerPawn.h"

AInteractionSystemGameModeBase::AInteractionSystemGameModeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPawnClass = APlayerPawn::StaticClass();
}