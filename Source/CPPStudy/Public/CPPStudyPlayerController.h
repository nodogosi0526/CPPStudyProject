// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CPPStudyPlayerController.generated.h"

class UInputMappingContext;

/**
 *
 */
UCLASS()
class CPPSTUDY_API ACPPStudyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	// Begin Actor interface
protected:

	virtual void BeginPlay() override;

	// End Actor interface

private:

	UPROPERTY(EditAnywhere, Category = "AAA | UI")
	TSubclassOf<class UUserWidget> PlayerHUDWidgetClass;

	UPROPERTY()
	class UUserWidget* PlayerHUDWidget;
};
