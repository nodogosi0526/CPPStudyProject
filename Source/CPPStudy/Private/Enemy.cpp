// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "CPPStudyCharacter.h"
#include "Enemy_Controller.h"
#include "EnemyManager.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/EngineTypes.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RightHandCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightHandCollision"));
	RightHandCollision->SetupAttachment(GetMesh(), FName("RightHandSocket"));

	// Start with collision disabled by default
	RightHandCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightHandCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightHandCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightHandCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;

	RightHandCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightHandOverlap);

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        if (UAnimInstance* AnimInst = MeshComp->GetAnimInstance())
        {
            AnimInst->OnMontageEnded.AddDynamic(this, &AEnemy::HandleOnMontageEnded);
        }
    }
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::Attack()
{

	if (EnemyState == EEnemyState::EES_Dead || EnemyState ==  EEnemyState::EES_Pooled) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance != nullptr)
	{
		// Play attack montage
		if (MeleeAttackMontage != nullptr)
		{
			AnimInstance->Montage_Play(MeleeAttackMontage);

			// Play attack sounds
			if (MeleeAttackSounds.Num() > 0)
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), MeleeAttackSounds[FMath::RandRange(0, MeleeAttackSounds.Num() - 1)], GetActorLocation());
			}

		}
	}
}

float AEnemy::TakeDamage(float DamageAmount,
                        const FDamageEvent& DamageEvent,
                        AController* EventInstigator,
                        AActor* DamageCauser)
{
	if (EnemyState == EEnemyState::EES_Dead || EnemyState == EEnemyState::EES_Pooled || DamageAmount <= 0.f) return 0.f;

  // Hit location/normal (handles PointDamage path: WeaponComponent passes Hit data)
	FVector HitLocation = GetActorLocation();
	FVector HitNormal = GetActorUpVector();

  if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PDE = static_cast<const FPointDamageEvent*>(&DamageEvent);
		HitLocation = PDE->HitInfo.ImpactPoint;
		HitNormal = PDE->HitInfo.ImpactNormal;
	}

  // Apply damage
  const float OldHP = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);

  UE_LOG(LogTemp, Log, TEXT("%s took %.2f (%.2f -> %.2f). Causer=%s, Instigator=%s"),
        *GetName(), DamageAmount, OldHP, CurrentHealth,
        *GetNameSafe(DamageCauser),
        EventInstigator ? *GetNameSafe(EventInstigator->GetPawn()) : TEXT("None"));

  // Play damage effect
  if (EnemyDamageEffect)
	{
      UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), EnemyDamageEffect, HitLocation, HitNormal.Rotation());
	}

  // Play damage sound
  if (EnemyDamageSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), EnemyDamageSound, GetActorLocation());
	}

  // Check if enemy is dead
	if (CurrentHealth <= 0.f)
	{
		Die();
	}

  return DamageAmount;
}

void AEnemy::Die()
{

	if (EnemyState == EEnemyState::EES_Dead || EnemyState == EEnemyState::EES_Pooled) return;

	SetEnemyState(EEnemyState::EES_Dead);

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
        if (UBrainComponent* Brain = AIController->GetBrainComponent())
        {
            Brain->StopLogic(TEXT("Died"));
        }

        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->StopMovementImmediately();
        }

        if (UCapsuleComponent* Capsule = GetCapsuleComponent())
        {
            Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
	}

	// Play death sounds
	if (DeathSounds.Num() > 0)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSounds[FMath::RandRange(0, DeathSounds.Num() - 1)], GetActorLocation());
	}

	// Play death montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance != nullptr)
	{
		if (DeathMontage != nullptr)
		{
			AnimInstance->Montage_Play(DeathMontage);
		}
	}

	if (MyManager)
	{
		MyManager->OnEnemyKilled(this);
	}


	PlayDissolveFX(false);
}

void AEnemy::SetEnemyManager(AEnemyManager* Manager)
{
	MyManager = Manager;
}

void AEnemy::Deactivate()
{
	SetEnemyState(EEnemyState::EES_Pooled);

	// Hide the actor and disable it in the world
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	SetActorEnableCollision(false);
}

void AEnemy::Reactivate(const FVector& NewLocation)
{
	if (EnemyState != EEnemyState::EES_Pooled) return;

	// --- Reset state ---
	SetEnemyState(EEnemyState::EES_Spawning);
	CurrentHealth = MaxHealth;

	// --- Reset transform ---
	SetActorLocation(NewLocation);
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	ResetCollisionAndMovement();

	// --- Reset AI ---
	ResetAI();

	// --- Trigger dissolve effect exposed to Blueprint ---
	PlayDissolveFX(true); // true = fade in
}

void AEnemy::SetEnemyState(EEnemyState NewState)
{
	if (EnemyState == NewState) return;
	EnemyState = NewState;
}

void AEnemy::SetMoveSpeed(float NewSpeed)
{
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->MaxWalkSpeed = NewSpeed;
	}
}

void AEnemy::ResetCollisionAndMovement()
{
	SetActorEnableCollision(true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCharacterMovement()->SetDefaultMovementMode();
}

void AEnemy::ResetAI()
{
	if (GetController())
	{
		GetController()->Destroy();
	}
	SpawnDefaultController();

	StartAI();
}

void AEnemy::StartAI()
{
	AEnemy_Controller* AIController = Cast<AEnemy_Controller>(GetController());
	if (AIController)
	{
		AIController->StartAI();
	}
}

void AEnemy::HandleOnMontageEnded(UAnimMontage* EndedMontage, bool bInterrupted)
{

}

void AEnemy::OnRightHandOverlap(UPrimitiveComponent* OverlappedComponent,
                                AActor* OtherActor,
                                UPrimitiveComponent* OtherComp,
                                int32 OtherBodyIndex,
                                bool bFromSweep,
                                const FHitResult& SweepResult)
{
  // Validate target
	ACPPStudyCharacter* Player = Cast<ACPPStudyCharacter>(OtherActor);
  if (!Player || Cast<AActor>(Player) == this) return;

  // Prevent multiple hits (one hit per swing)
  if (bHasDealtDamageThisSwing) return;
  bHasDealtDamageThisSwing = true;

  // Safety check before applying damage
  if (MeleeDamage <= 0.f) return;
  TSubclassOf<UDamageType> DamageType = MeleeDamageType ? MeleeDamageType : TSubclassOf<UDamageType>(UDamageType::StaticClass());
  // InstigatorController
  AController* InstigatorController = (GetController() ? GetController() : GetInstigatorController());

  // Determine hit location and direction
  // Overlap events are not always from Sweep, so provide fallback values
  FVector ImpactPoint;
  FVector ImpactNormal;

  if (bFromSweep)
  {
      ImpactPoint = SweepResult.ImpactPoint;
      ImpactNormal = SweepResult.ImpactNormal;
  }
  else
  {
      ImpactPoint = Player->GetActorLocation();
      ImpactNormal = -GetActorForwardVector();
  }

  if (ImpactNormal.IsNearlyZero()) ImpactNormal = -GetActorForwardVector();

  // Attack direction (incoming). Prefer opposite of impact normal; fallback to velocity or attacker->target vector.
  FVector ShotDir = (-ImpactNormal).GetSafeNormal();
  if (ShotDir.IsNearlyZero())
  {
    const FVector VelDir = GetVelocity().GetSafeNormal();
    ShotDir = !VelDir.IsNearlyZero() ? VelDir : (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal();
  }

  // Damage
  FHitResult Hit = SweepResult;
  if (!bFromSweep)
  {
      Hit.ImpactPoint  = ImpactPoint;
      Hit.ImpactNormal = ImpactNormal;
      Hit.Location     = ImpactPoint;
  }

  UGameplayStatics::ApplyPointDamage(Player,
                                    MeleeDamage,
                                    ShotDir,
                                    Hit,
                                    InstigatorController,
                                    this,
                                    MeleeDamageType
  );

	RightHandCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  UE_LOG(LogTemp, Verbose, TEXT("Melee hit %s at %s"),
  *GetNameSafe(Player), *SweepResult.ImpactPoint.ToString());
}

