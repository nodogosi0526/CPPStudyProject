// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_Controller.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "UObject/ConstructorHelpers.h"

AEnemy_Controller::AEnemy_Controller(FObjectInitializer const& a_pObjectInit)
{
	// Get the bahaviortree from reference
	static ConstructorHelpers::FObjectFinder<UBehaviorTree> treeFinder(TEXT("/Script/AIModule.BehaviorTree'/Game/SurvivalFPS/AI/BT_Enemy.BT_Enemy'"));
	if (treeFinder.Succeeded())
	{
		BehaviorTree = treeFinder.Object;
	}

	// Init the tree and blackboard components
	BehaviorTreeComponent = a_pObjectInit.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorTree Component"));
	Blackboard = a_pObjectInit.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("Blackboard Component"));
}

void AEnemy_Controller::BeginPlay()
{
	Super::BeginPlay();

	// Run the behavior tree
	StartAI();
}

void AEnemy_Controller::OnPossess(APawn* a_pPawn)
{
	// Run defalut possess method
	Super::OnPossess(a_pPawn);

	// Init blackboard
	if (Blackboard)
	{
		Blackboard->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	}
}

void AEnemy_Controller::StartAI()
{
	if (BehaviorTree)
	{
		RunBehaviorTree(BehaviorTree);
	}
}