// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TP_WeaponComponent.h"
#include "WeaponData.h"

// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
    
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	MeshComponent->SetupAttachment(RootComponent);

	FireAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudioComponent"));
	FireAudioComponent->SetupAttachment(RootComponent);
	FireAudioComponent->bAutoActivate = false;
    
	WeaponComponent = CreateDefaultSubobject<UTP_WeaponComponent>(TEXT("WeaponComponent"));
}

void AWeapon::PlayFireSound()
{
	if (!FireAudioComponent)
	{
		return;
	}

	if (!WeaponData || !WeaponData->FireSound)
	{
		return;
	}

	if (FireAudioComponent->IsPlaying())
	{
		FireAudioComponent->Stop();
	}

	FireAudioComponent->SetSound(WeaponData->FireSound);
	FireAudioComponent->Play();
}