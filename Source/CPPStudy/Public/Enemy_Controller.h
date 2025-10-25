// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Enemy_Controller.generated.h"

/**
 * 
 */
UCLASS()
class CPPSTUDY_API AEnemy_Controller : public AAIController
{
	GENERATED_BODY()
	
public:
	AEnemy_Controller(FObjectInitializer const& a_pObjectInit);

	// Overrides
	void BeginPlay() override;
	void OnPossess(APawn* a_pPawn) override;

	void StartAI();

	// Method to get black board
	class UBlackboardComponent* GetBlackboard() const { return Blackboard;}

	// Tree components
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI")
	class UBehaviorTreeComponent* BehaviorTreeComponent;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI")
	class UBehaviorTree* BehaviorTree;

private:
	class UBlackboardComponent* Blackboard;
};
