// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "CPPStudyCharacter.h"
#include "Enemy_Controller.h"
#include "EnemyManager.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RightHandCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightHandCollision"));
	RightHandCollision->SetupAttachment(GetMesh(), FName("RightHandSocket"));

	// �ŏ��͓����蔻��𖳌��ɂ��Ă���
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
	GetMesh()->GetAnimInstance()->OnMontageEnded.AddDynamic(this, &AEnemy::HandleOnMontageEnded);
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

void AEnemy::TakeDamage(float DamageAmount,
                        const FDamageEvent& DamageEvent,
                        AController* EventInstigator,
                        AActor* DamageCauser)
{
	if (EnemyState == EEnemyState::EES_Dead || EnemyState == EEnemyState::EES_Pooled || DamageAmount <= 0.f) return;

  const float OldHP = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);

  UE_LOG(LogTemp, Log, TEXT("%s took %.2f (%.2f -> %.2f). Causer=%s, Instigator=%s"),
        *GetName(), DamageAmount, OldHP, CurrentHealth,
        *GetNameSafe(DamageCauser),
        EventInstigator ? *GetNameSafe(EventInstigator->GetPawn()) : TEXT("None"));

	if (CurrentHealth <= 0.f)
	{
		Die();
	}
	else
	{
		if (EnemyDamageSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), EnemyDamageSound, GetActorLocation());
		}
	}
}

void AEnemy::Die()
{

	if (EnemyState == EEnemyState::EES_Dead || EnemyState == EEnemyState::EES_Pooled) return;

	SetEnemyState(EEnemyState::EES_Dead);

	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->GetBrainComponent()->StopLogic(TEXT("Died"));

		if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
		{
			MoveComp->StopMovementImmediately();
		}
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

	// Actor���\���ɂ��A�������ׂ��[���ɂ���
	SetActorHiddenInGame(true);
	SetActorTickEnabled(false);
	SetActorEnableCollision(false);
}

void AEnemy::Reactivate(const FVector& NewLocation)
{
	if (EnemyState != EEnemyState::EES_Pooled) return;

	// --- ��Ԃ̃��Z�b�g ---
	SetEnemyState(EEnemyState::EES_Spawning);
	CurrentHealth = MaxHealth;

	// --- ������Ԃ̃��Z�b�g ---
	SetActorLocation(NewLocation);
	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	ResetCollisionAndMovement();

	// --- AI�̃��Z�b�g ---
	ResetAI();

	// --- �o�ꉉ�o��Blueprint�Ɉ˗� ---
	PlayDissolveFX(true); // true = �t�Đ�
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

void AEnemy::OnRightHandOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ACPPStudyCharacter* Player = Cast<ACPPStudyCharacter>(OtherActor);

	if (Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("Right hand hit Player!!"));
		Player->TakeDamage(10.0f);
		RightHandCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

