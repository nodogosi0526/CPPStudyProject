// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "CPPStudyCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class AWeapon;
class UWeaponData;
class UTP_WeaponComponent;
class UAnimMontage;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);
// Delegate used to notify HP changes (float: Current HP, float: Max HP)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);
// Delegate used to notify ammo changes (int32: Current ammo, int32: Total ammo)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAmmoChangedSignature, int32, CurrentAmmo, int32, TotalAmmo);

UCLASS(config=Game)
class ACPPStudyCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Mesh, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCameraComponent;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction = nullptr;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveAction = nullptr;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAction = nullptr;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ReloadAction = nullptr;

	/** Shoot Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ShootingAction = nullptr;
	
public:
	ACPPStudyCharacter();

	// --- Weapon Related ---

	/* Class of the initial weapon to spawn */
	UPROPERTY(EditDefaultsOnly, Category = "AAA | Weapon")
	TSubclassOf<AWeapon> InitialWeaponClass;

	/* Pointer to the weapon currently equipped */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AAA | Weapon")
	TObjectPtr<AWeapon> EquippedWeapon;

	/* Pointer to the weapon component of the equipped weapon */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "AAA | Weapon")
	TObjectPtr<UTP_WeaponComponent> EquippedWeaponComponent;

  // Weapon configuration (damage, ammo, montages, etc.)
  UPROPERTY()
  UWeaponData* WeaponData;

	// --- Health ---

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AAA | Health")
	float CurrentHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AAA | Health")
	float MaxHealth = 100.0f;

  UPROPERTY(EditAnywhere, Category="AAA | Health")
  bool bGodMode = false;

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "AAA | Events")
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "AAA | Events")
	FOnAmmoChangedSignature OnAmmoChanged;

public:
  virtual float TakeDamage(float DamageAmount,
                          struct FDamageEvent const& DamageEvent,
                          class AController* EventInstigator,
                          class AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure, Category = "AAA | Weapon")
	int32 GetCurrentAmmo() const;

	UFUNCTION(BlueprintPure, Category = "AAA | Weapon")
	int32 GetTotalAmmo() const;

protected:

	virtual void BeginPlay() override;

	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void StartFire();

	void StopFire();

	void StartReload();

private:

	void EquipWeapon(TSubclassOf<AWeapon> WeaponClass);

	UFUNCTION()
	void HandleWeaponAmmoChanged(int32 NewCurrentAmmo, int32 NewTotalAmmo);

public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
};
