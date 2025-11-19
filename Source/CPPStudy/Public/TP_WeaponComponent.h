// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TP_WeaponComponent.generated.h"

class ACPPStudyCharacter;
class AWeapon;
class UAnimMontage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponAmmoChangedSignature, int32, CurrentAmmo, int32, TotalAmmo);

UCLASS(Blueprintable, BlueprintType, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CPPSTUDY_API UTP_WeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UTP_WeaponComponent();

	// Function called from outside  

	void InitializeAmmo();

	void AttachWeaponToCharacter(ACPPStudyCharacter* TargetCharacter);

	void StartFire();

	void StopFire();

	void StartReload();

	// Getter functions

	UFUNCTION(BlueprintPure, Category = "AAA | Weapon")
	int32 GetCurrentAmmo()const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category = "AAA | Weapon")
	int32 GetTotalAmmo()const { return TotalAmmo; }

	// Delegate

	UPROPERTY(BlueprintAssignable, Category = "AAA | Events")
	FOnWeaponAmmoChangedSignature OnAmmoChanged;

protected:

	virtual void BeginPlay() override;

	/** Ends gameplay for this component. */
	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	void Fire();

	void ConsumeAmmo();

	void FinishReload();

	bool CanReload()const;

	UFUNCTION()
	void OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	// Owner

	UPROPERTY()
	ACPPStudyCharacter* OwningCharacter;

	UPROPERTY()
	AWeapon* OwningWeapon;

	// Owner's weapon

	UPROPERTY(VisibleInstanceOnly, Category = "AAA | Weapon State")
	int32 CurrentAmmo;

	UPROPERTY(VisibleInstanceOnly, Category = "AAA | Weapon State")
	int32 TotalAmmo;

	// Is reloading?

	UPROPERTY(VisibleInstanceOnly, Category = "AAA | Weapon State")
	bool bIsReloading = false;

	// Fire interval timer

	FTimerHandle FireRateTimerHandle;;
};
