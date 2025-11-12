// Fill out your copyright notice in the Description page of Project Settings

#include "EnemyManager.h"
#include "Enemy.h"
#include "EnemyTypes.h"


AEnemyManager::AEnemyManager()
{
    PrimaryActorTick.bCanEverTick = true;

    bIsNewWave = false;
    CurrentWave = 0;
    EnemiesKilledThisWave = 0;
    EnemiesSpawnedThisWave = 0;
}

// BeginPlay: logic executed when the game starts
void AEnemyManager::BeginPlay()
{
    Super::BeginPlay();

    // 1. Create the enemy pool
    for (int32 i = 0; i < PoolSize; ++i)
    {
        if (AEnemy* PooledEnemy = GetWorld()->SpawnActor<AEnemy>(EnemyClass, FVector::ZeroVector, FRotator::ZeroRotator))
        {
            PooledEnemy->Deactivate(); // Put the enemy into its inactive state
            PooledEnemy->SetEnemyManager(this); // Register this manager
            EnemyPool.Add(PooledEnemy);
        }
    }

    // 2. Start the first wave after a brief delay
    FTimerHandle StartGameTimer;
    GetWorldTimerManager().SetTimer(StartGameTimer, this, &AEnemyManager::StartNextWave, 3.0f, false);
}

// OnEnemyKilled: handle the logic when an enemy is defeated
void AEnemyManager::OnEnemyKilled(AEnemy* KilledEnemy)
{
    EnemiesKilledThisWave++;
    
    if (EnemiesKilledThisWave >= WaveSettings[CurrentWave - 1].EnemiesToSpawn)
    {
        StartNextWave();
    }
    else
    {
        CheckAndSpawnEnemies();
    }
}

// StartNextWave: begin the next wave
void AEnemyManager::StartNextWave()
{
    if (WaveSettings.IsValidIndex(CurrentWave))
    {
        CurrentWave++;
        EnemiesKilledThisWave = 0;
        EnemiesSpawnedThisWave = 0;

        bIsNewWave = true;

        FTimerHandle StartNextWaveTimer;
        GetWorldTimerManager().SetTimer(StartNextWaveTimer, this, &AEnemyManager::CheckAndSpawnEnemies, 5.0f, false);
    }
    else
    {
        // No more waves remain; the game is effectively cleared
    }
}

void AEnemyManager::CheckAndSpawnEnemies()
{
    if (!WaveSettings.IsValidIndex(CurrentWave - 1)) return;

    const FWaveSettings& CurrentSettings = WaveSettings[CurrentWave - 1];

    int32 CurrentAliveEnemies = 0;
    for (AEnemy* Enemy : EnemyPool)
    {
        if (Enemy && (Enemy->EnemyState == EEnemyState::EES_Attacking || Enemy->EnemyState == EEnemyState::EES_Spawning))
        {
            CurrentAliveEnemies++;
        }
    }

    // 1. Remaining number of enemies that still need to spawn this wave
    const int32 EnemiesAvailableToSpawn = CurrentSettings.EnemiesToSpawn - EnemiesSpawnedThisWave;
    // 2. Available slots based on how many enemies are currently active
    const int32 ScreenSlotsAvailable = CurrentSettings.MaxEnemiesOnScreen - CurrentAliveEnemies;
    // 3. Spawn the minimum of the remaining enemies and open slots
    const int32 NumToSpawn = FMath::Min(EnemiesAvailableToSpawn, ScreenSlotsAvailable);

    // If nothing should be spawned, exit early
    if (NumToSpawn > 0)
    {
        if (bIsNewWave)
        {
            if (OnWaveChanged.IsBound())
            {
                OnWaveChanged.Broadcast(CurrentWave);
            }
            bIsNewWave = false;
        }

        for (int32 i = 0; i < NumToSpawn; ++i)
        {
            if (AEnemy* EnemyToSpawn = GetPooledEnemy())
            {
                const int32 RandomIndex = FMath::RandRange(0, SpawnLocations.Num() - 1);
                const FVector SpawnLocation = SpawnLocations[RandomIndex]->GetActorLocation();
                const float SpeedForThisWave = FMath::RandRange(CurrentSettings.MinMoveSpeed, CurrentSettings.MaxMoveSpeed);

                EnemyToSpawn->SetMoveSpeed(SpeedForThisWave);
                EnemyToSpawn->Reactivate(SpawnLocation);
                EnemiesSpawnedThisWave++;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("AEnemyManager: EnemyPool is empty!"));
                break;
            }
        }
    }
}

// GetPooledEnemy: find and return an available pooled enemy
AEnemy* AEnemyManager::GetPooledEnemy()
{
    for (AEnemy* Enemy : EnemyPool)
    {
        if (Enemy && Enemy->EnemyState == EEnemyState::EES_Pooled)
        {
            return Enemy;
        }
    }
    return nullptr; // Return null if no pooled enemy is available
}

