// Fill out your copyright notice in the Description page of Project Settings.


#include "TirosHUD.h"

#include "CharacterOverlay.h"
#include "GameFramework/PlayerController.h"

void ATirosHUD::BeginPlay()
{
	Super::BeginPlay();
	AddCharacterOverlay();
}

void ATirosHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ATirosHUD::DrawHUD()
{
	Super::DrawHUD();

	if(GEngine)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X * 0.5f, ViewportSize.Y * 0.5f);

		const float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		if(HUDPackage.CrosshairsCenter)
		{
			const FVector2D Spread(0.f,0.f);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairsLeft)
		{
			const FVector2D Spread(-SpreadScaled,0.f);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairsRight)
		{
			const FVector2D Spread(SpreadScaled,0.f);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairsTop)
		{
			const FVector2D Spread(0.f,-SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if(HUDPackage.CrosshairsBottom)
		{
			const FVector2D Spread(0.f,SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
	
}

void ATirosHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth * 0.5f) + Spread.X,
		ViewportCenter.Y - (TextureHeight * 0.5f) + Spread.Y);
	
	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth,
		TextureHeight,0.f,0.f,1.f,1.f,
		CrosshairColor);
}
