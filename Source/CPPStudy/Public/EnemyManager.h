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

    // ������ �G�����񂾂Ƃ��ɌĂ΂����J�֐� ������
    void OnEnemyKilled(AEnemy* KilledEnemy);

    UPROPERTY(BlueprintAssignable, Category = "AAA | Events")
    FOnWaveChangedSignature OnWaveChanged;

protected:
    virtual void BeginPlay() override;

    // --- �v�[���֘A ---
    UPROPERTY(EditAnywhere, Category = "AAA | Pooling")
    TSubclassOf<AEnemy> EnemyClass; // �v�[������G��BP�N���X

    UPROPERTY(EditAnywhere, Category = "AAA | Pooling")
    int32 PoolSize = 20; // �ŏ��ɗp�ӂ��Ă����G�̑���

    // --- �X�|�[���n�_ ---
    // ����̃A�C�f�A�I���x���ɔz�u����Target Point�Ȃǂ��w��ł���
    UPROPERTY(EditAnywhere, Category = "AAA | Spawning")
    TArray<AActor*> SpawnLocations;

    // --- �E�F�[�u�Ǘ� ---
    UPROPERTY(EditAnywhere, Category = "AAA | Waves")
    TArray<FWaveSettings> WaveSettings; // �S�E�F�[�u�̐ݒ�

private:
    // --- �����ϐ� ---
    UPROPERTY()
    TArray<AEnemy*> EnemyPool; // ��A�N�e�B�u�ȓG��ێ�����v�[��

    bool bIsNewWave;

    int32 CurrentWave;
    int32 EnemiesKilledThisWave;
    int32 EnemiesSpawnedThisWave;

    // --- �����֐� ---
    AEnemy* GetPooledEnemy(); // �v�[�������A�N�e�B�u�ȓG���擾
    void StartNextWave();
    void CheckAndSpawnEnemies();
};
