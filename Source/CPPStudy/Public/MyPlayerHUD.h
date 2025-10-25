// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyPlayerHUD.generated.h"

UCLASS()
class CPPSTUDY_API UMyPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // �e�����X�V����֐�
    UFUNCTION(BlueprintCallable)
    void UpdateAmmoText(int32 CurrentAmmo, int32 TotalAmmo);

    UFUNCTION(BlueprintCallable)
    void SetHealth(float Health, float MaxHealth);

    UFUNCTION(BlueprintCallable)
    void UpdateWaveCount(int32 NewWaveNumber);

protected:
    virtual void NativeConstruct() override;

    // Ammo��\������TextBlock�ɃA�N�Z�X���邽�߂̕ϐ�
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_Ammo;

    UPROPERTY(meta = (BindWidget))
    class UProgressBar* PlayerHealthBar;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* WaveNumber;
};
