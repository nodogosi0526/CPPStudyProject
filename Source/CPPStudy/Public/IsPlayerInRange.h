// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "IsPlayerInRange.generated.h"

/**
 * 
 */
UCLASS()
class CPPSTUDY_API UIsPlayerInRange : public UBTService_BlackboardBase
{
	GENERATED_BODY()

public:
	// Constructor
	UIsPlayerInRange();

	// Method for when service is called
	void OnBecomeRelevant(UBehaviorTreeComponent& TreeComp, uint8* NodeMemory);

	// Field for range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float Range = 100.0f;
	
};
