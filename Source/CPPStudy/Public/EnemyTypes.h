#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "EnemyTypes.generated.h" 

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    EES_Spawning    UMETA(DisplayName = "Spawning"),
    EES_Patrolling  UMETA(DisplayName = "Patrolling"),
    EES_Chasing     UMETA(DisplayName = "Chasing"),
    EES_Attacking   UMETA(DisplayName = "Attacking"),
    EES_Dead        UMETA(DisplayName = "Dead"),
    EES_Pooled      UMETA(DisplayName = "Pooled")
};

USTRUCT(BlueprintType)
struct FWaveSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " AAA | Wave Settings")
    int32 EnemiesToSpawn; // ���̃E�F�[�u�ŃX�|�[�������鑍��

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " AAA | Wave Settings")
    int32 MaxEnemiesOnScreen; // �����ɉ�ʏ�ɏo���ł���ő吔

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " AAA | Wave Settings")
    float MinMoveSpeed = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = " AAA | Wave Settings")
    float MaxMoveSpeed = 200.0f;
};

UCLASS()
class CPPSTUDY_API UEnemyTypes : public UObject
{
    GENERATED_BODY()
};
