// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponData.generated.h"

class UAnimMontage;
class USoundBase;
class UNiagaraSystem;

/**
 * 武器の「設定だけ」をまとめたデータアセット。
 * ロジックやランタイム状態は持たない。
 */
UCLASS(BlueprintType)
class CPPSTUDY_API UWeaponData : public UDataAsset
{
    GENERATED_BODY()

public:
    // --- Animation --------------------------------------------------------

    /** 射撃時に再生するモンタージュ */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TObjectPtr<UAnimMontage> FireMontage = nullptr;

    /** リロード時に再生するモンタージュ */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Animation")
    TObjectPtr<UAnimMontage> ReloadMontage = nullptr;

    // --- Sounds -----------------------------------------------------------

    /** 射撃音 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound")
    TObjectPtr<USoundBase> FireSound = nullptr;

    /** 弾切れ時に再生する音 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound")
    TObjectPtr<USoundBase> EmptySound = nullptr;

    /** リロード時に再生する音 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound")
    TObjectPtr<USoundBase> ReloadSound = nullptr;

    /** 環境音（屋内・屋外などで切り替える用途など） */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Sound")
    TArray<TObjectPtr<USoundBase>> EnvironmentalSounds;

    // --- Effects ----------------------------------------------------------

    /** マズルフラッシュを出すソケット名 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    FName MuzzleSocketName = TEXT("Muzzle");

    /** マズルフラッシュのNiagaraシステム */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    TObjectPtr<UNiagaraSystem> MuzzleFlash = nullptr;

    /** 着弾時エフェクトのNiagaraシステム */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Effects")
    TObjectPtr<UNiagaraSystem> ImpactEffect = nullptr;

    // --- Combat -----------------------------------------------------------

    /** 1発あたりのダメージ */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat", meta = (ClampMin = "0.0"))
    float Damage = 20.0f;

    /** 1秒あたりの発射数（発射レート） */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat", meta = (ClampMin = "0.0"))
    float FireRate = 5.0f;

    /** 最大射程（センチメートル） */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat", meta = (ClampMin = "0.0"))
    float MaxRange = 500.0f;

    // --- Ammo -------------------------------------------------------------

    /** 一つのマガジンに入る弾数 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
    int32 MagazineCapacity = 30;

    /** 所持できる最大総弾数 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Ammo", meta = (ClampMin = "0"))
    int32 MaxTotalAmmo = 120;

    // --- Muzzle Offset ----------------------------------------------------

    /** 発射位置のオフセット（キャラ or 武器基準） */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Combat")
    FVector MuzzleOffset = FVector(100.f, 0.f, 10.f);
};
