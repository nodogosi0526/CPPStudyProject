// Fill out your copyright notice in the Description page of Project Settings.

// EnemyManager.h

#pragma once

#include "CoreMinimal.h"
#include "EnemyTypes.h"
#include "GameFramework/Actor.h"
#include "EnemyManager.generated.h"

class AEnemy;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveChangedSignature, int32, NewWave);

UCLASS()
class CPPSTUDY_API AEnemyManager : public AActor
{
    GENERATED_BODY()

    /** Reference to HUD */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<class UUserWidget> PlayerHUDWidgetClass;

public:
    AEnemyManager();

    // Function called whenever an enemy has been killed
    void OnEnemyKilled(AEnemy* KilledEnemy);

    UPROPERTY(BlueprintAssignable, Category = "AAA | Events")
    FOnWaveChangedSignature OnWaveChanged;

protected:
    virtual void BeginPlay() override;

    // --- Pooling ---
    UPROPERTY(EditAnywhere, Category = "AAA | Pooling")
    TSubclassOf<AEnemy> EnemyClass; // Enemy Blueprint class used for pooling

    UPROPERTY(EditAnywhere, Category = "AAA | Pooling")
    int32 PoolSize = 20; // Number of enemies preallocated in the pool

    // --- Spawning ---
    // Actors placed in the level (such as Target Points) that mark spawn locations
    UPROPERTY(EditAnywhere, Category = "AAA | Spawning")
    TArray<AActor*> SpawnLocations;

    // --- Wave Management ---
    UPROPERTY(EditAnywhere, Category = "AAA | Waves")
    TArray<FWaveSettings> WaveSettings; // Settings for each wave

private:
    // --- Internal Variables ---
    UPROPERTY()
    TArray<AEnemy*> EnemyPool; // Pool storing reusable enemy instances

    bool bIsNewWave;

    int32 CurrentWave;
    int32 EnemiesKilledThisWave;
    int32 EnemiesSpawnedThisWave;

    // --- Internal Functions ---
    AEnemy* GetPooledEnemy(); // Retrieves an available enemy from the pool
    void StartNextWave();
    void CheckAndSpawnEnemies();
};
