// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

// �O���錾
class UStaticMeshComponent;
class UTP_WeaponComponent;
class UAnimMontage;
class USoundBase;
class UNiagaraSystem;

UCLASS()
class CPPSTUDY_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	

	AWeapon();

	UFUNCTION(BlueprintPure, Category = "AAA | Weapon")
	UTP_WeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

protected:

	UPROPERTY(VisibleDefaultsOnly, Category = "AAA | Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AAA | Components")
	TObjectPtr<UTP_WeaponComponent> WeaponComponent;

public:

	// ����ŗL�f�[�^

	UPROPERTY(EditDefaultsOnly, Category = "AAA | Animation")
	TObjectPtr<UAnimMontage> FireMontage;

	UPROPERTY(EditDefaultsOnly, Category = "AAA | Animation")
	TObjectPtr<UAnimMontage> ReloadMontage;

	UPROPERTY(EditDefaultsOnly, Category = "AAA | Sounds")
	TObjectPtr<USoundBase> FireSound;

	UPROPERTY(EditDefaultsOnly, Category = "AAA | Sounds")
	TObjectPtr<USoundBase> EmptySound;

	UPROPERTY(EditDefaultsOnly, Category = "AAA | Sounds")
	TObjectPtr<USoundBase> ReloadSound;

	UPROPERTY(EditDefaultsOnly, Category = "AAA | Sounds")
	TArray<TObjectPtr<USoundBase>> EnvironmentalSounds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA | Effects")
	FName MuzzleSocketName = "Muzzle";

	UPROPERTY(EditDefaultsOnly, Category = "AAA | Effects")
	TObjectPtr<UNiagaraSystem> MuzzleFlash;

	UPROPERTY(EditDefaultsOnly, Category = "AAA | Effects")
	TObjectPtr<UNiagaraSystem> ImpactEffect;

    UPROPERTY(EditDefaultsOnly, Category = "AAA | Combat")
	float Damage = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AAA | Combat")
	float FireRate = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AAA | Combat")
	float MaxRange = 500.0f;

	// �e��f�[�^
	UPROPERTY(EditDefaultsOnly, Category = "AAA | Ammo")
	int32 MagazineCapacity = 30;

	UPROPERTY(EditDefaultsOnly, Category = "AAA | Ammo")
	int32 MaxTotalAmmo = 120;

	// Muzzle Offset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	FVector m_vMuzzleOffset = FVector(100.f, 0, 10.f);
};
