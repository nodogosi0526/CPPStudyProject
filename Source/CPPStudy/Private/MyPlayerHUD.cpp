// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerHUD.h"
#include "CPPStudyCharacter.h"
#include "EnemyManager.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"


void UMyPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	AEnemyManager* EnemyManager = Cast<AEnemyManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyManager::StaticClass()));

	if (EnemyManager)
	{
		EnemyManager->OnWaveChanged.AddDynamic(this, &UMyPlayerHUD::UpdateWaveCount);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyMaveger not found!"));
	}

	ACPPStudyCharacter*  PlayerCharacter = Cast<ACPPStudyCharacter>(GetOwningPlayerPawn());

	if (PlayerCharacter)
	{
		PlayerCharacter->OnAmmoChanged.AddDynamic(this, &UMyPlayerHUD::UpdateAmmoText);
		PlayerCharacter->OnHealthChanged.AddDynamic(this, &UMyPlayerHUD::SetHealth);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerCaracter not found!"));
	}
}

void UMyPlayerHUD::UpdateAmmoText(int32 CurrentAmmo, int32 TotalAmmo)
{
	if (Text_Ammo)
	{
		FString AmmoStr = FString::Printf(TEXT("%d / %d"), CurrentAmmo, TotalAmmo);
		Text_Ammo->SetText(FText::FromString(AmmoStr));
	}
}

void UMyPlayerHUD::SetHealth(float Health, float MaxHealth)
{
	if (PlayerHealthBar)
	{
		PlayerHealthBar->SetPercent(Health / MaxHealth);
	}
}

void UMyPlayerHUD::UpdateWaveCount(int32 NewWaveNumber)
{
	if (WaveNumber)
	{
		WaveNumber->SetText(FText::AsNumber(NewWaveNumber));
	}
}
