// Copyright Epic Games, Inc. All Rights Reserved.

#include "CPPStudyCharacter.h"
#include "CPPStudyProjectile.h"
#include "TP_WeaponComponent.h"
#include "Weapon.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/EngineTypes.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ACPPStudyCharacter::ACPPStudyCharacter()
    :JumpAction(nullptr)
	,LookAction(nullptr)
	,MoveAction(nullptr)
	,ReloadAction(nullptr)
	,ShootingAction(nullptr)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	CurrentHealth = MaxHealth;
}

void ACPPStudyCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Player is using: %s"), *GetName());

	if (InitialWeaponClass)
	{
		EquipWeapon(InitialWeaponClass);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("InitialWeaponClass is not set in Character Blueprint."));
	}

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

//////////////////////////////////////////////////////////////////////////// Input

void ACPPStudyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
  // Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
    ensureMsgf(JumpAction,     TEXT("SetupPlayerInputComponent: JumpAction is null. Set it in the Character Blueprint."));
    ensureMsgf(MoveAction,     TEXT("SetupPlayerInputComponent: MoveAction is null. Set it in the Character Blueprint."));
    ensureMsgf(LookAction,     TEXT("SetupPlayerInputComponent: LookAction is null. Set it in the Character Blueprint."));
    ensureMsgf(ReloadAction,   TEXT("SetupPlayerInputComponent: ReloadAction is null. Set it in the Character Blueprint."));
    ensureMsgf(ShootingAction, TEXT("SetupPlayerInputComponent: ShootingAction is null. Set it in the Character Blueprint."));

    if (!JumpAction || !MoveAction || !LookAction || !ReloadAction || !ShootingAction)
    {
      return;
    }

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACPPStudyCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACPPStudyCharacter::Look);

		// Bind Reload event
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ACPPStudyCharacter::StartReload);

		// Bind Shoot event
		EnhancedInputComponent->BindAction(ShootingAction, ETriggerEvent::Started, this, &ACPPStudyCharacter::StartFire);
		EnhancedInputComponent->BindAction(ShootingAction, ETriggerEvent::Completed, this, &ACPPStudyCharacter::StopFire);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void ACPPStudyCharacter::EquipWeapon(TSubclassOf<AWeapon> WeaponClass)
{
	if (EquippedWeapon)
	{
		// Initialize the equipped weapon
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;
		EquippedWeaponComponent = nullptr;
	}

	if (!WeaponClass)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("EquipWeapon: WeaponClass is null."));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	EquippedWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, GetActorLocation(), GetActorRotation(), SpawnParams);
	ensureMsgf(EquippedWeapon, TEXT("EquipWeapon: Could not spawn weapon from class. Check WeaponClass is valid and World is available."));
	if (!EquippedWeapon)
	{
		return;
	}
	EquippedWeaponComponent = EquippedWeapon->FindComponentByClass<UTP_WeaponComponent>();

	ensureMsgf(EquippedWeaponComponent, TEXT("EquipWeapon: Spawned weapon does not have a UTP_WeaponComponent. The weapon class must include this component."));
	if (!EquippedWeaponComponent)
	{
		return;
	}
	EquippedWeaponComponent->AttachWeaponToCharacter(this);
	EquippedWeaponComponent->InitializeAmmo(EquippedWeapon->MagazineCapacity, EquippedWeapon->MaxTotalAmmo);
	EquippedWeaponComponent->OnAmmoChanged.AddDynamic(this, &ACPPStudyCharacter::HandleWeaponAmmoChanged);
	OnAmmoChanged.Broadcast(EquippedWeaponComponent->GetCurrentAmmo(), EquippedWeaponComponent->GetTotalAmmo());
}

float ACPPStudyCharacter::TakeDamage(float DamageAmount,
                                    const FDamageEvent& DamageEvent,
                                    AController* EventInstigator,
                                    AActor* DamageCauser)
{
  // Ignore damage if damage amount is 0 or less, or if god mode is enabled
  if (DamageAmount <= 0.f || bGodMode)
  {
    return 0.f;
  }

  // Hit location/normal
  FVector ImpactPoint = GetActorLocation();
  FVector ImpactNormal = GetActorUpVector();

  if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
  {
    const FPointDamageEvent* PDE = static_cast<const FPointDamageEvent*>(&DamageEvent);
    ImpactPoint = PDE->HitInfo.ImpactPoint;
    ImpactNormal = PDE->HitInfo.ImpactNormal;
  }
  else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
    {
        const FRadialDamageEvent* RDE = static_cast<const FRadialDamageEvent*>(&DamageEvent);
        // Use direction from explosion center to self as normal substitute
        ImpactNormal = (GetActorLocation() - RDE->Origin).GetSafeNormal();
        ImpactPoint  = GetActorLocation();
    }
  if (ImpactNormal.IsNearlyZero()) ImpactNormal = GetActorUpVector();

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);
	UE_LOG(LogTemplateCharacter, Log, TEXT("%s took %f damage. New health %f"), *GetName(), DamageAmount, CurrentHealth);

	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}

	// Death handling
	if (CurrentHealth <= 0)
	{
		// Confirm the death, log it, and restart the level
		UE_LOG(LogTemp, Error, TEXT("%s has died! Restarting level..."), *GetName());
		UGameplayStatics::OpenLevel(GetWorld(), FName(GetWorld()->GetName()), true);
	}

    return DamageAmount;
}

// Existing implementation delegates to the weapon component 

void ACPPStudyCharacter::StartFire()
{
	ensureMsgf(EquippedWeaponComponent, TEXT("StartFire: EquippedWeaponComponent is null. EquipWeapon must be called first."));
	if (!EquippedWeaponComponent)
	{
		return;
	}
	EquippedWeaponComponent->StartFire();
}

void ACPPStudyCharacter::StopFire()
{
	ensureMsgf(EquippedWeaponComponent, TEXT("StopFire: EquippedWeaponComponent is null. EquipWeapon must be called first."));
	if (!EquippedWeaponComponent)
	{
		return;
	}
	EquippedWeaponComponent->StopFire();
}

void ACPPStudyCharacter::StartReload()
{
	ensureMsgf(EquippedWeaponComponent, TEXT("StartReload: EquippedWeaponComponent is null. EquipWeapon must be called first."));
	if (!EquippedWeaponComponent)
	{
		return;
	}
	EquippedWeaponComponent->StartReload();
}

// Getter functions and event handlers

int32 ACPPStudyCharacter::GetCurrentAmmo() const
{
	ensureMsgf(EquippedWeaponComponent, TEXT("GetCurrentAmmo: EquippedWeaponComponent is null. EquipWeapon must be called first."));
	if (!EquippedWeaponComponent)
	{
		return 0;
	}
	return EquippedWeaponComponent->GetCurrentAmmo();
}

int32 ACPPStudyCharacter::GetTotalAmmo() const
{
	ensureMsgf(EquippedWeaponComponent, TEXT("GetTotalAmmo: EquippedWeaponComponent is null. EquipWeapon must be called first."));
	if (!EquippedWeaponComponent)
	{
		return 0;
	}
	return EquippedWeaponComponent->GetTotalAmmo();
}

void ACPPStudyCharacter::HandleWeaponAmmoChanged(int32 NewCurrentAmmo, int32 NewTotalAmmo)
{
	if (OnAmmoChanged.IsBound())
	{
		OnAmmoChanged.Broadcast(NewCurrentAmmo, NewTotalAmmo);
	}
}

// Character basic input (movement and look)

void ACPPStudyCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	ensureMsgf(Controller, TEXT("Move: Controller is null. This should not happen during normal gameplay."));
	if (!Controller)
	{
		return;
	}
  // add movement 
  AddMovementInput(GetActorForwardVector(), MovementVector.Y);
  AddMovementInput(GetActorRightVector(), MovementVector.X);
}

void ACPPStudyCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	ensureMsgf(Controller, TEXT("Look: Controller is null. This should not happen during normal gameplay."));
	if (!Controller)
	{
		return;
	}
  // add yaw and pitch input to controller
  AddControllerYawInput(LookAxisVector.X);
  AddControllerPitchInput(LookAxisVector.Y);
}
