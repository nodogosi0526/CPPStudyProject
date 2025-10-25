// Fill out your copyright notice in the Description page of Project Settings.


#include "IsPlayerInRange.h"
#include "Enemy_Controller.h"
#include "EnemyKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UIsPlayerInRange::UIsPlayerInRange()
{
	bNotifyBecomeRelevant = true;
	NodeName = TEXT("Is Player In Range");
}

void UIsPlayerInRange::OnBecomeRelevant(UBehaviorTreeComponent& TreeComp, uint8* NodeMemory)
{
	// Super Call
	Super::OnBecomeRelevant(TreeComp, NodeMemory);

	// Get AI Controller and pawn
	AEnemy_Controller* AIController = Cast<AEnemy_Controller>(TreeComp.GetAIOwner());
	APawn* EnemyPawn = AIController->GetPawn();

	// Get Player Controller and pawn
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	APawn* PlayerPawn = PlayerController->GetPawn();

	// Set blackboard key to whether or not player is in range
	const bool bIsInRange = (EnemyPawn->GetDistanceTo(PlayerPawn) <= Range);
	AIController->GetBlackboard()->SetValueAsBool(EnemyKeys::IsPlayerInRange, bIsInRange);
}
