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
	ensureMsgf(ObservedEnemyManager, TEXT("NativeConstruct: EnemyManager not found in the world. Make sure an EnemyManager actor exists in the level."));
    if (ObservedEnemyManager)
    {
        ObservedEnemyManager->OnWaveChanged.AddDynamic(this, &UMyPlayerHUD::UpdateWaveCount);
    }

    ObservedCharacter = Cast<ACPPStudyCharacter>(GetOwningPlayerPawn());
	ensureMsgf(ObservedCharacter, TEXT("NativeConstruct: PlayerCharacter not found. The owning pawn must be a CPPStudyCharacter."));
    if (ObservedCharacter)
    {
        ObservedCharacter->OnAmmoChanged.AddDynamic(this, &UMyPlayerHUD::UpdateAmmoText);
        ObservedCharacter->OnHealthChanged.AddDynamic(this, &UMyPlayerHUD::SetHealth);
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
	ensureMsgf(Text_Ammo, TEXT("UpdateAmmoText: Text_Ammo widget is null. Make sure the widget is bound with BindWidget meta tag."));
	if (Text_Ammo)
	{
		FString AmmoStr = FString::Printf(TEXT("%d / %d"), CurrentAmmo, TotalAmmo);
		Text_Ammo->SetText(FText::FromString(AmmoStr));
	}
}

void UMyPlayerHUD::SetHealth(float Health, float MaxHealth)
{
	ensureMsgf(PlayerHealthBar, TEXT("SetHealth: PlayerHealthBar widget is null. Make sure the widget is bound with BindWidget meta tag."));
	if (PlayerHealthBar)
	{
		PlayerHealthBar->SetPercent(Health / MaxHealth);
	}
}

void UMyPlayerHUD::UpdateWaveCount(int32 NewWaveNumber)
{
	ensureMsgf(WaveNumber, TEXT("UpdateWaveCount: WaveNumber widget is null. Make sure the widget is bound with BindWidget meta tag."));
	if (WaveNumber)
	{
		WaveNumber->SetText(FText::AsNumber(NewWaveNumber));
	}
}
