// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class AEnemyManager;

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "EnemyTypes.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

UCLASS()
class CPPSTUDY_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// References to montages
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA | Animation")
	UAnimMontage* MeleeAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA | Animation")
	UAnimMontage* DeathMontage;

	// References to sounds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA | Sound")
	TArray<USoundBase*> MeleeAttackSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA | Sound")
	TArray<USoundBase*> DeathSounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA | Sound")
	USoundBase* EnemyDamageSound;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA | Combat")
	TObjectPtr<UBoxComponent> RightHandCollision;

public:
	// Combat fields
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AAA | Combat")
	float MaxHealth = 30.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AAA | Combat")
	float CurrentHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AAA | State")
	EEnemyState EnemyState;

	UFUNCTION(BlueprintCallable, Category = "AAA | State")
	void SetEnemyState(EEnemyState NewState);

	UFUNCTION(BlueprintCallable, Category = "AAA | Pooling")
	void Deactivate();

	UFUNCTION(BlueprintCallable, Category = "AAA | Pooling")
	void Reactivate(const FVector& NewLocation);

	// Combat methods
	void Attack();

	void TakeDamage(float TakenDamage);

	// Death handling methods
	void Die();

	UFUNCTION(BlueprintImplementableEvent, Category ="AAA | Effect")
	void PlayDissolveFX(bool bIsReverse);

	UFUNCTION(BlueprintCallable, Category = "AAA | Combat")
	UBoxComponent* GetRightHandCollision() const { return RightHandCollision; }

	UFUNCTION()
	void OnRightHandOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void SetEnemyManager(AEnemyManager* Manager);

	void SetMoveSpeed(float NewSpeed);

private:

	void ResetAI();
	void ResetCollisionAndMovement();

	UFUNCTION(BlueprintCallable, Category = "AAA | AI")
	void StartAI();

	UPROPERTY()
	AEnemyManager* MyManager;

protected:
	// Montage End Event Handler
	UFUNCTION()
	void HandleOnMontageEnded(UAnimMontage* EndedMontage, bool bInterrupted);

};
