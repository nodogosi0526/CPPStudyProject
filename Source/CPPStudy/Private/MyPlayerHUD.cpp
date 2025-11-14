// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerHUD.h"
#include "CPPStudyCharacter.h"
#include "EnemyManager.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/LogMacros.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"


void UMyPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	ObservedEnemyManager = Cast<AEnemyManager>(
        UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyManager::StaticClass()));
    if (ObservedEnemyManager)
    {
        ObservedEnemyManager->OnWaveChanged.AddDynamic(this, &UMyPlayerHUD::UpdateWaveCount);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UMyPlayerHUD: EnemyManager not found!"));
    }

    ObservedCharacter = Cast<ACPPStudyCharacter>(GetOwningPlayerPawn());
    if (ObservedCharacter)
    {
        ObservedCharacter->OnAmmoChanged.AddDynamic(this, &UMyPlayerHUD::UpdateAmmoText);
        ObservedCharacter->OnHealthChanged.AddDynamic(this, &UMyPlayerHUD::SetHealth);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("UMyPlayerHUD: PlayerCharacter not found!"));
    }
}

void UMyPlayerHUD::NativeDestruct()
{
    if (ObservedEnemyManager)
    {
        ObservedEnemyManager->OnWaveChanged.RemoveDynamic(this, &UMyPlayerHUD::UpdateWaveCount);
        ObservedEnemyManager = nullptr;
    }

    if (ObservedCharacter)
    {
        ObservedCharacter->OnAmmoChanged.RemoveDynamic(this, &UMyPlayerHUD::UpdateAmmoText);
        ObservedCharacter->OnHealthChanged.RemoveDynamic(this, &UMyPlayerHUD::SetHealth);
       ObservedCharacter = nullptr;
    }

    Super::NativeDestruct();
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
