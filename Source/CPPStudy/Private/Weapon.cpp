// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TP_WeaponComponent.h"
#include "WeaponData.h"
#include "NiagaraFunctionLibrary.h"

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

void AWeapon::PlayReloadStartSound()
{
	if (!FireAudioComponent)
	{
		return;
	}

	if (!WeaponData || !WeaponData->ReloadStartSound)
	{
		return;
	}

	// Don't stop if playing? Or maybe stop fire sound? 
	// Usually reload sound should interrupt fire sound or just play. 
	// Since we use the same AudioComponent, we must set the new sound.
	// If we want to allow overlap, we would need separate AudioComponents.
	// The requirement says "use FireAudioComponent is fine".
	
	FireAudioComponent->Stop();
	FireAudioComponent->SetSound(WeaponData->ReloadStartSound);
	FireAudioComponent->Play();
}

void AWeapon::PlayReloadEndSound()
{
	if (!FireAudioComponent)
	{
		return;
	}

	if (!WeaponData || !WeaponData->ReloadEndSound)
	{
		return;
	}

	FireAudioComponent->Stop();
	FireAudioComponent->SetSound(WeaponData->ReloadEndSound);
	FireAudioComponent->Play();
}

void AWeapon::PlayMuzzleFX()
{
	if (!WeaponData || !WeaponData->MuzzleFlash || !MeshComponent)
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAttached(
		WeaponData->MuzzleFlash,
		MeshComponent,
		WeaponData->MuzzleSocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		true
	);
}

void AWeapon::PlayImpactFX(const FHitResult& HitResult)
{
	if (!WeaponData || !WeaponData->ImpactEffect)
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		WeaponData->ImpactEffect,
		HitResult.ImpactPoint,
		HitResult.ImpactNormal.Rotation()
	);
}