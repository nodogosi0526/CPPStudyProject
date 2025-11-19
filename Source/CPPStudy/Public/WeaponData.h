// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponData.generated.h"

class UAnimMontage;
class USoundBase;
class UNiagaraSystem;

/**
 * Data asset that contains only weapon configuration settings.
 * Does not contain logic or runtime state.
 */
UCLASS(BlueprintType)
class CPPSTUDY_API UWeaponData : public UDataAsset
{
	GENERATED_BODY()

public:
	// --- Animation --------------------------------------------------------

	/** Animation montage to play when firing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> FireMontage = nullptr;

	/** Animation montage to play when reloading */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
	TObjectPtr<UAnimMontage> ReloadMontage = nullptr;

	// --- Sounds -----------------------------------------------------------

	/** Sound to play when firing */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound")
	TObjectPtr<USoundBase> FireSound = nullptr;

	/** Sound to play when out of ammo */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound")
	TObjectPtr<USoundBase> EmptySound = nullptr;

	/** Sound to play when reloading */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound")
	TObjectPtr<USoundBase> ReloadSound = nullptr;

	/** Environmental sounds (for switching between indoor/outdoor, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound")
	TArray<TObjectPtr<USoundBase>> EnvironmentalSounds;

	// --- Effects ----------------------------------------------------------

	/** Socket name for spawning muzzle flash */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	FName MuzzleSocketName = TEXT("Muzzle");

	/** Niagara system for muzzle flash effect */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	TObjectPtr<UNiagaraSystem> MuzzleFlash = nullptr;

	/** Niagara system for impact effect when hitting target */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
	TObjectPtr<UNiagaraSystem> ImpactEffect = nullptr;

	// --- Combat -----------------------------------------------------------

	/** Damage per shot */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat", meta = (ClampMin = "0.0"))
	float Damage = 20.0f;

	/** Shots per second (fire rate) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat", meta = (ClampMin = "0.0"))
	float FireRate = 5.0f;

	/** Maximum range in centimeters */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat", meta = (ClampMin = "0.0"))
	float MaxRange = 500.0f;

	// --- Ammo -------------------------------------------------------------

	/** Number of rounds in one magazine */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 MagazineCapacity = 30;

	/** Maximum total ammo capacity */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
	int32 MaxTotalAmmo = 120;

	// --- Muzzle Offset ----------------------------------------------------

	/** Muzzle position offset (relative to character or weapon) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat")
	FVector MuzzleOffset = FVector(100.f, 0.f, 10.f);
};
