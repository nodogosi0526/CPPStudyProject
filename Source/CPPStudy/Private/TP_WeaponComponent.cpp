// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "CPPStudyCharacter.h"
#include "CPPStudyProjectile.h"
#include "Enemy.h"
#include "Weapon.h"
#include "WeaponData.h"
#include "Animation/AnimInstance.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTP_WeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningWeapon = Cast<AWeapon>(GetOwner());
  ensureMsgf(OwningWeapon, TEXT("BeginPlay: OwningWeapon is not an AWeapon!."));
	if (!OwningWeapon)
	{
		return;
	}

  WeaponData = OwningWeapon->GetWeaponData();
  ensureMsgf(WeaponData, TEXT("BeginPlay: WeaponData is null. Make sure the Weapon has a valid UWeaponData assigned."));
  if (!WeaponData)
  {
      return;
  }
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
}

void UTP_WeaponComponent::InitializeAmmo()
{
  ensureMsgf(WeaponData, TEXT("InitializeAmmo: WeaponData is null. BeginPlay must be called first."));
  if (!WeaponData)
  {
    return;
  }

  CurrentAmmo = WeaponData->MagazineCapacity;
  TotalAmmo   = WeaponData->MaxTotalAmmo;

  UE_LOG(LogTemp, Log, TEXT("Ammo Initialized from WeaponData. Current: %d, Total: %d"), CurrentAmmo, TotalAmmo);
}

void UTP_WeaponComponent::AttachWeaponToCharacter(ACPPStudyCharacter* TargetCharacter)
{
	if (!TargetCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("AttachWeaponToCharacter: AttachWeapon failed. TargetCharacter is null."));
		return;
	}

	ensureMsgf(OwningWeapon, TEXT("AttachWeaponToCharacter: OwningWeapon is null. BeginPlay must be called first."));
	if (!OwningWeapon)
	{
		return;
	}

	OwningCharacter = TargetCharacter;

	USceneComponent* ParentMesh = OwningCharacter->GetMesh1P();
	ensureMsgf(ParentMesh, TEXT("AttachWeaponToCharacter: Character mesh (GetMesh1P) is null. The character must have a valid first-person mesh."));
	if (!ParentMesh)
	{
		return;
	}

	static const FName SocketName(TEXT("WeaponSocket"));
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	OwningWeapon->AttachToComponent(ParentMesh, AttachmentRules, SocketName);
	OwningWeapon->SetOwner(OwningCharacter);
}

void UTP_WeaponComponent::StartFire()
{
	if (bIsReloading)
	{
		return;
	}

	if (CurrentAmmo <= 0)
	{
		return;
	}

	Fire();

	if (OwningWeapon && WeaponData->FireRate > 0)
	{
		const float FireInterval = 1.0f / WeaponData->FireRate;
		GetWorld()->GetTimerManager().SetTimer(FireRateTimerHandle, this, &UTP_WeaponComponent::Fire, FireInterval, true);
	}
}

void UTP_WeaponComponent::StopFire()
{
	GetWorld()->GetTimerManager().ClearTimer(FireRateTimerHandle);
}

void UTP_WeaponComponent::StartReload()
{
	ensureMsgf(OwningCharacter, TEXT("StartReload: OwningCharacter is null. AttachWeaponToCharacter must be called first."));
	ensureMsgf(OwningWeapon, TEXT("StartReload: OwningWeapon is null. BeginPlay must be called first."));
	if (!OwningCharacter || !OwningWeapon)
	{
		return;
	}

	if (!CanReload())
	{
		return;
	}

	bIsReloading = true;
	UE_LOG(LogTemp, Log, TEXT("Reloading..."));

	StopFire();

	if (WeaponData && WeaponData->ReloadMontage)
	{
		UAnimInstance* AnimInstance = OwningCharacter->GetMesh1P()->GetAnimInstance();

		if (AnimInstance)
		{
			const float MontagePlayRate = AnimInstance->Montage_Play(WeaponData->ReloadMontage);

			if (MontagePlayRate > 0.f)
			{
				FOnMontageEnded BlendOutDelegate;
				BlendOutDelegate.BindUObject(this, &UTP_WeaponComponent::OnReloadMontageEnded);
				AnimInstance->Montage_SetEndDelegate(BlendOutDelegate, WeaponData->ReloadMontage);
			}
			else
			{
				FinishReload();
			}
		}
	}
	else
	{
		FinishReload();
	}
}

void UTP_WeaponComponent::Fire()
{
	ensureMsgf(OwningCharacter, TEXT("Fire: OwningCharacter is null. AttachWeaponToCharacter must be called first."));
  if (!OwningCharacter)
  {
    return;
  }

  APlayerController* PlayerController = Cast<APlayerController>(OwningCharacter->GetController());
	ensureMsgf(PlayerController, TEXT("Fire: Character's Controller is not a PlayerController. This component is designed for player characters."));
	if (!PlayerController)
	{
		return;
	}

	ensureMsgf(OwningWeapon, TEXT("Fire: OwningWeapon is null. BeginPlay must be called first."));
	if (!OwningWeapon)
	{
		return;
	}

	if (CurrentAmmo <= 0)
	{
		StopFire();
		StartReload();
		return;
	}

	ConsumeAmmo();

	USkeletalMeshComponent* Mesh = OwningCharacter->GetMesh1P();
	UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;

	// Fire Montage

	if (AnimInstance && WeaponData->FireMontage)
	{
		const float PlayRate = AnimInstance->Montage_Play(WeaponData->FireMontage);
		UE_LOG(LogTemp, Verbose, TEXT("Fire: Montage_Play returned %f"), PlayRate);
	}
	else
	{
		if (!AnimInstance)
		{
			UE_LOG(LogTemp, Verbose, TEXT("Fire: no AnimInstance (skip montage)"));
		}
	}

	// Fire Sound

	if (WeaponData->FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->FireSound, OwningCharacter->GetActorLocation());
	}

	// Muzzle Effect (Niagara System)

	if (WeaponData->MuzzleFlash && Mesh)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			WeaponData->MuzzleFlash,
			Mesh,
      WeaponData->MuzzleSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	// Collision Query

	FVector CameraLocation;
	FRotator CameraRotation;
	
	PlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const FVector TraceStart = CameraLocation;
	const FVector ShotDirection = CameraRotation.Vector().GetSafeNormal();
	const float MaxRange = WeaponData ? WeaponData->MaxRange : 1000.f;
  const FVector TraceEnd = TraceStart + ShotDirection * MaxRange;

	FHitResult HitResult;
	static constexpr ECollisionChannel WeaponTrace = ECC_GameTraceChannel;

  //Collision Query Parameters
	FCollisionQueryParams CollisionParams(SCENE_QUERY_STAT(WeaponTrace), true);
	CollisionParams.AddIgnoredActor(OwningCharacter);
	CollisionParams.AddIgnoredActor(OwningWeapon);
	CollisionParams.bReturnPhysicalMaterial = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, WeaponTrace, CollisionParams);

	// (Debug) Line Trace
#if WITH_EDITOR
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bHit ? FColor::Red : FColor::Green, false, 1.0f, 0, 1.0f);
	if (bHit)
	{
		DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 4.0f, 8, FColor::Yellow, false, 2.0f);
	}
#endif

	if (bHit && HitResult.GetActor())
	{
    // Damage
    AActor* HitActor = HitResult.GetActor();
    if (HitActor)
    {
      const float Damage = WeaponData ? WeaponData->Damage : 0.f;
      UGameplayStatics::ApplyPointDamage(HitActor, Damage, ShotDirection, HitResult, PlayerController, OwningWeapon, UDamageType::StaticClass());
    }

    // Impact Effect
		if (WeaponData->ImpactEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				WeaponData->ImpactEffect,
				HitResult.ImpactPoint,
				HitResult.ImpactNormal.Rotation()
			);
		}
	}
}

void UTP_WeaponComponent::ConsumeAmmo()
{
	--CurrentAmmo;
	OnAmmoChanged.Broadcast(CurrentAmmo, TotalAmmo);
}

void UTP_WeaponComponent::FinishReload()
{
	ensureMsgf(WeaponData, TEXT("FinishReload: WeaponData is null. BeginPlay must be called first."));
	if (!WeaponData) return;

	const int32 AmmoToReload = FMath::Min(WeaponData->MagazineCapacity - CurrentAmmo, TotalAmmo);
	CurrentAmmo += AmmoToReload;
	TotalAmmo -= AmmoToReload;

	bIsReloading = false;

	OnAmmoChanged.Broadcast(CurrentAmmo, TotalAmmo);
	UE_LOG(LogTemp, Log, TEXT("FinishReload: CurrentAmmo: %d. TotalAmmo: %d."), CurrentAmmo, TotalAmmo);
}

bool UTP_WeaponComponent::CanReload() const
{
	ensureMsgf(WeaponData, TEXT("CanReload: WeaponData is null. BeginPlay must be called first."));
	if (!WeaponData) return false;
	return !bIsReloading && (CurrentAmmo < WeaponData->MagazineCapacity) && (TotalAmmo > 0);
}

void UTP_WeaponComponent::OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
  if (!bInterrupted)
  {
    FinishReload();
  }
  else
  {
    bIsReloading = false;
    UE_LOG(LogTemp, Warning, TEXT("OnReloadMontageEnded: Reload montage interrupted."));
  }
}
