// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"

#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if(DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerName(const APlayerState* PlayerState)
{
	if(PlayerState != nullptr)
	{
		const FString PlayerName = PlayerState->GetPlayerName();
		const FString PlayerNameString = FString::Printf(TEXT("Player name: %s"), *PlayerName);
		SetDisplayText(PlayerNameString);
	}
}

void UOverheadWidget::NativeDestruct()
{
	RemoveFromParent();
	Super::NativeDestruct();	
}
