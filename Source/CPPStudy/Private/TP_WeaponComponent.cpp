// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "CPPStudy.h"
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

	if (OwningCharacter)
	{
		if (USkeletalMeshComponent* Mesh = OwningCharacter->GetMesh1P())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				AnimInstance->OnPlayMontageNotifyBegin.RemoveDynamic(this, &UTP_WeaponComponent::OnNotifyBegin);
			}
		}
	}
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

	USkeletalMeshComponent* ParentMesh = OwningCharacter->GetMesh1P();
	ensureMsgf(ParentMesh, TEXT("AttachWeaponToCharacter: Character mesh (GetMesh1P) is null. The character must have a valid first-person mesh."));
	if (!ParentMesh)
	{
		return;
	}

	static const FName SocketName(TEXT("WeaponSocket"));
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	OwningWeapon->AttachToComponent(ParentMesh, AttachmentRules, SocketName);
	OwningWeapon->SetOwner(OwningCharacter);

	if (UAnimInstance* AnimInstance = ParentMesh->GetAnimInstance())
	{
		AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UTP_WeaponComponent::OnNotifyBegin);
	}
}

void UTP_WeaponComponent::StartFire()
{
	if (bIsReloading)
	{
		return;
	}

	if (CurrentAmmo <= 0)
	{
		StartReload();
		return;
	}

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
}

void UTP_WeaponComponent::StopFire()
{
	if (OwningCharacter)
	{
		if (USkeletalMeshComponent* Mesh = OwningCharacter->GetMesh1P())
		{
			if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
			{
				if (WeaponData && WeaponData->FireMontage)
				{
					AnimInstance->Montage_Stop(0.2f, WeaponData->FireMontage);
				}
			}
		}
	}
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
	StopFire();

	bool bPlayedMontage = false;

	if (WeaponData && WeaponData->ReloadMontage)
	{
		if (UAnimInstance* AnimInstance = OwningCharacter->GetMesh1P()->GetAnimInstance())
		{
			const float MontagePlayRate = AnimInstance->Montage_Play(WeaponData->ReloadMontage);
			if (MontagePlayRate > 0.f)
			{
				bPlayedMontage = true;
				if (OwningWeapon)
				{
					OwningWeapon->PlayReloadStartSound();
				}
        
				FOnMontageEnded BlendOutDelegate;
				BlendOutDelegate.BindUObject(this, &UTP_WeaponComponent::OnReloadMontageEnded);
				AnimInstance->Montage_SetEndDelegate(BlendOutDelegate, WeaponData->ReloadMontage);
			}
		}
	}

	if (!bPlayedMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("StartReload: No Reload Montage or failed to play. Executing instant reload."));
		RefillAmmo();
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

	// Fire Sound
  
	if (OwningWeapon)
	{
		OwningWeapon->PlayFireSound();
	}

	// Muzzle Effect (Niagara System)

	if (OwningWeapon)
	{
		OwningWeapon->PlayMuzzleFX();
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
	static constexpr ECollisionChannel WeaponTrace = ECC_GameTraceChannel1;

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
		if (OwningWeapon)
		{
			OwningWeapon->PlayImpactFX(HitResult);
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
	bIsReloading = false;
}

void UTP_WeaponComponent::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointNotifyPayload)
{
	if (NotifyName == FName(TEXT("Fire")))
	{
		Fire();
	}
	else if (NotifyName == FName(TEXT("Reload")))
	{
		RefillAmmo();
	}
}

void UTP_WeaponComponent::RefillAmmo()
{
	ensureMsgf(WeaponData, TEXT("RefillAmmo: WeaponData is null. BeginPlay must be called first."));
	if (!WeaponData) return;

	const int32 AmmoToReload = FMath::Min(WeaponData->MagazineCapacity - CurrentAmmo, TotalAmmo);
	CurrentAmmo += AmmoToReload;
	TotalAmmo -= AmmoToReload;

	OnAmmoChanged.Broadcast(CurrentAmmo, TotalAmmo);

	if (OwningWeapon)
	{
		OwningWeapon->PlayReloadEndSound();
	}

	UE_LOG(LogTemp, Log, TEXT("RefillAmmo: CurrentAmmo: %d. TotalAmmo: %d."), CurrentAmmo, TotalAmmo);
}

bool UTP_WeaponComponent::CanReload() const
{
	ensureMsgf(WeaponData, TEXT("CanReload: WeaponData is null. BeginPlay must be called first."));
	if (!WeaponData) return false;
	return !bIsReloading && (CurrentAmmo < WeaponData->MagazineCapacity) && (TotalAmmo > 0);
}

void UTP_WeaponComponent::OnReloadMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnReloadMontageEnded: Reload montage interrupted."));
	}
	FinishReload();
}
