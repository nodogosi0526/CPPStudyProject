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

// BeginPlay: �Q�[���J�n���̏���
void AEnemyManager::BeginPlay()
{
    Super::BeginPlay();

    // 1. �G���v�[������
    for (int32 i = 0; i < PoolSize; ++i)
    {
        if (AEnemy* PooledEnemy = GetWorld()->SpawnActor<AEnemy>(EnemyClass, FVector::ZeroVector, FRotator::ZeroRotator))
        {
            PooledEnemy->Deactivate(); // �����ɔ�\���ɂ���
            PooledEnemy->SetEnemyManager(this); // �i�ߊ����Z�b�g�I
            EnemyPool.Add(PooledEnemy);
        }
    }

    // 2. �����҂��Ă���ŏ��̃E�F�[�u���J�n
    FTimerHandle StartGameTimer;
    GetWorldTimerManager().SetTimer(StartGameTimer, this, &AEnemyManager::StartNextWave, 3.0f, false);
}

// OnEnemyKilled: ���m����̎��S�񍐂��󂯂��Ƃ��̏���
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

// StartNextWave: ���̃E�F�[�u���J�n����
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
        // �Q�[���N���A�����Ȃ�
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

    // 1. ���̃E�F�[�u�ŁA���Ɖ��̃X�|�[���ł���g���c���Ă��邩�H
    const int32 EnemiesAvailableToSpawn = CurrentSettings.EnemiesToSpawn - EnemiesSpawnedThisWave;
    // 2. ��ʏ�ɂ́A���Ɖ��̃X�|�[���ł���󂫂����邩�H
    const int32 ScreenSlotsAvailable = CurrentSettings.MaxEnemiesOnScreen - CurrentAliveEnemies;
    // 3. ���ۂɃX�|�[�������鐔�́A�u�X�|�[���ł���g�v�Ɓu��ʂ̋󂫁v�̏��Ȃ���
    const int32 NumToSpawn = FMath::Min(EnemiesAvailableToSpawn, ScreenSlotsAvailable);

    // �X�|�[��������ׂ�����1�̈ȏア��Ȃ���s
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

// GetPooledEnemy: �v�[������g����G��T���ĕԂ�
AEnemy* AEnemyManager::GetPooledEnemy()
{
    for (AEnemy* Enemy : EnemyPool)
    {
        if (Enemy && Enemy->EnemyState == EEnemyState::EES_Pooled)
        {
            return Enemy;
        }
    }
    return nullptr; // �v�[���Ɏg����G�����Ȃ��ꍇ
}

