#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class UStaticMeshComponent;
class UAudioComponent;
class UTP_WeaponComponent;
class UWeaponData;

UCLASS()
class CPPSTUDY_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:
	AWeapon();

	UFUNCTION(BlueprintPure, Category = "AAA | Weapon")
	UTP_WeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

	UFUNCTION(BlueprintPure, Category = "AAA | Weapon")
	UWeaponData* GetWeaponData() const { return WeaponData; }

  // --- AUDIO ---
  
	void PlayFireSound();
	void PlayReloadStartSound();
	void PlayReloadEndSound();

	// --- VFX ---

	void PlayMuzzleFX();
	void PlayImpactFX(const FHitResult& HitResult);

protected:

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AAA | Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AAA | Components")
	TObjectPtr<UAudioComponent> FireAudioComponent;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AAA | Components")
	TObjectPtr<UTP_WeaponComponent> WeaponComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AAA | Data")
	TObjectPtr<UWeaponData> WeaponData = nullptr;
};
