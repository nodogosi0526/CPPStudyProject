// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Attack.h"
#include "Enemy.h"
#include "Enemy_Controller.h"
#include "EnemyKeys.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTT_Attack::UBTT_Attack(FObjectInitializer const& a_pObjectInit)
{
	NodeName = TEXT("Attack");

	// ������ Most Important Fix ������
// Setting this to true creates a separate instance (object) of this task for each AI that executes it.
// This prevents member variables such as MyOwnerComp from being overwritten by other AIs.

	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTT_Attack::ExecuteTask(UBehaviorTreeComponent& TreeComp, uint8* NodeMemory)
{
	// --- Preparation Phase: Retrieve necessary objects ---
	AEnemy_Controller* AIController = Cast<AEnemy_Controller>(TreeComp.GetAIOwner());
	if (!AIController || !AIController->GetBlackboard())
	{
	// If any required object is missing, fail immediately.
		return EBTNodeResult::Failed;
	}

	// --- Condition Check Phase: Determine whether an attack is possible ---
	const bool bCanAttack = AIController->GetBlackboard()->GetValueAsBool(EnemyKeys::IsPlayerInRange);
	if (!bCanAttack)
	{
		// If the attack cannot be executed, fail (although the task itself may sometimes be treated as successful).

		return EBTNodeResult::Failed;
	}

	AEnemy* EnemyPawn = Cast<AEnemy>(AIController->GetPawn());
	if (!EnemyPawn)
	{
		return EBTNodeResult::Failed;
	}

	UAnimInstance* AnimInstance = EnemyPawn->GetMesh()->GetAnimInstance();
	if (!AnimInstance || !AnimInstance->Montage_GetIsStopped(EnemyPawn->MeleeAttackMontage))
	{
		// If the animation cannot be played, or is already playing, fail.
		return EBTNodeResult::Failed;
	}

	// --- Execution Phase: All checks passed, so execute the attack ---
	EnemyPawn->Attack();
	AnimInstance->OnMontageEnded.AddDynamic(this, &UBTT_Attack::OnMontageEnded);
	MyOwnerComp = &TreeComp;

	return EBTNodeResult::InProgress;
}

void UBTT_Attack::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{

	if (AEnemy* EnemyPawn = Cast<AEnemy>(MyOwnerComp->GetAIOwner()->GetPawn()))
	{
		if (UAnimInstance* AnimInstance = EnemyPawn->GetMesh()->GetAnimInstance())
		{
			AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTT_Attack::OnMontageEnded);
		}
	}

	if (MyOwnerComp)
	{
		if (!bInterrupted)
		{
			FinishLatentTask(*MyOwnerComp, EBTNodeResult::Succeeded);
		}
		else
		{
			FinishLatentTask(*MyOwnerComp, EBTNodeResult::Failed);
		}
	}
}

EBTNodeResult::Type UBTT_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Called when the task is interrupted by the behavior tree logic.
	AEnemy_Controller* AIController = Cast<AEnemy_Controller>(OwnerComp.GetAIOwner());
	if (AIController)
	{
		AEnemy* EnemyPawn = Cast<AEnemy>(AIController->GetPawn());
		if (EnemyPawn)
		{
			UAnimInstance* AnimInstance = EnemyPawn->GetMesh()->GetAnimInstance();
			if (AnimInstance && AnimInstance->Montage_IsPlaying(EnemyPawn->MeleeAttackMontage))
			{
				AnimInstance->Montage_Stop(0.1f, EnemyPawn->MeleeAttackMontage);
				AnimInstance->OnMontageEnded.RemoveDynamic(this, &UBTT_Attack::OnMontageEnded);
			}
		}
	}

	return EBTNodeResult::Aborted;
}
