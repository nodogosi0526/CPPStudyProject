// Copyright Epic Games, Inc. All Rights Reserved.

#include "CPPStudyCharacter.h"
#include "CPPStudyProjectile.h"
#include "TP_WeaponComponent.h"
#include "Weapon.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
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
		// TODO: ��������̔j��������C���x���g���ւ̊i�[�����������ɒǉ�
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;
		EquippedWeaponComponent = nullptr;
	}

	if (!WeaponClass)
	{
		UE_LOG(LogTemplateCharacter, Warning, TEXT("EquipWeapon faild: WeaponClass is null."));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	EquippedWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, GetActorLocation(), GetActorRotation(), SpawnParams);

	if (EquippedWeapon)
	{
		EquippedWeaponComponent = EquippedWeapon->FindComponentByClass<UTP_WeaponComponent>();
		if (EquippedWeaponComponent)
		{
			EquippedWeaponComponent->AttachWeaponToCharacter(this);
			EquippedWeaponComponent->OnAmmoChanged.AddDynamic(this, &ACPPStudyCharacter::HandleWeaponAmmoChanged);
			OnAmmoChanged.Broadcast(EquippedWeaponComponent->GetCurrentAmmo(), EquippedWeaponComponent->GetTotalAmmo());
		}
		else
		{
			UE_LOG(LogTemplateCharacter, Error, TEXT("EquipWeapon failed: Spawned weapon dose not have a UTP_WeaponComponent."));
		}
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("EquipWeapon failed: Could not spawn weapon from class."));
	}
}

void ACPPStudyCharacter::TakeDamage(float TakenDamage)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - TakenDamage, 0.0f, MaxHealth);
	UE_LOG(LogTemplateCharacter, Log, TEXT("%s took %f damage. New health %f"), *GetName(), TakenDamage, CurrentHealth);

	if (OnHealthChanged.IsBound())
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}

	// Death handling
	if (CurrentHealth <= 0)
	{
		// ���������� ���O�R�F���S���������s����邱�Ƃ��m�F ����������
		UE_LOG(LogTemp, Error, TEXT("%s has died! Restarting level..."), *GetName());
		UGameplayStatics::OpenLevel(GetWorld(), FName(GetWorld()->GetName()), true);
	}
}

// ���͏��� -> ����R���|�[�l���g�ւ̈Ϗ�

void ACPPStudyCharacter::StartFire()
{
	if (EquippedWeaponComponent)
	{
		EquippedWeaponComponent->StartFire();
	}
}

void ACPPStudyCharacter::StopFire()
{
	if (EquippedWeaponComponent)
	{
		EquippedWeaponComponent->StopFire();
	}
}

void ACPPStudyCharacter::StartReload()
{
	if (EquippedWeaponComponent)
	{
		EquippedWeaponComponent->StartReload();
	}
}

// �Q�b�^�[�֐��ƃC�x���g�n���h��

int32 ACPPStudyCharacter::GetCurrentAmmo() const
{
	if (EquippedWeaponComponent)
	{
		EquippedWeaponComponent->GetCurrentAmmo();
	}
	return 0;
}

int32 ACPPStudyCharacter::GetTotalAmmo() const
{
	if (EquippedWeaponComponent)
	{
		EquippedWeaponComponent->GetTotalAmmo();
	}
	return 0;
}

void ACPPStudyCharacter::HandleWeaponAmmoChanged(int32 NewCurrentAmmo, int32 NewTotalAmmo)
{
	if (OnAmmoChanged.IsBound())
	{
		OnAmmoChanged.Broadcast(NewCurrentAmmo, NewTotalAmmo);
	}
}

// �L�����N�^�[�̊�{���� (�ړ��E���_)

void ACPPStudyCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void ACPPStudyCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
