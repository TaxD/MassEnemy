// Fill out your copyright notice in the Description page of Project Settings.


#include "HostileMassCharacter.h"

#include "HostileMassFragments.h"
#include "MassAgentSubsystem.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"

AHostileMassCharacter::AHostileMassCharacter()
{
	MassAgent = CreateDefaultSubobject<UMassAgentComponent>(TEXT("MassAgent"));
}

void AHostileMassCharacter::KillHostile(float TTL)
{
	auto* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem) return;

	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();

	FHostileDeathFragment DeathFragment;
	DeathFragment.TTL = TTL;

	EntityManager.Defer().PushCommand<FMassCommandAddFragmentInstances>(
		MassAgent->GetEntityHandle(), DeathFragment
	);
}

void AHostileMassCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (MassAgent)
	{
		if (auto MassAgentSubsystem = GetWorld()->GetSubsystem<UMassAgentSubsystem>())
		{
			if (MassAgent->GetEntityHandle().IsValid())
			{
				OnEntityAssociated(*MassAgent);
			}

			MassAgentSubsystem->GetOnMassAgentComponentEntityAssociated().
			                    AddUObject(this, &AHostileMassCharacter::OnEntityAssociated);

			MassAgentSubsystem->GetOnMassAgentComponentEntityDetaching().
			                    AddUObject(this, &AHostileMassCharacter::OnEntityDetaching);
		}
	}
}

void AHostileMassCharacter::SetHealthPercent_Implementation(float Percent)
{
}

float AHostileMassCharacter::GetHealthPercent_Implementation() const
{
	return 1.f;
}

void AHostileMassCharacter::OnEntityAssociated(const UMassAgentComponent& AgentComponent)
{
	if (!MassAgent || &AgentComponent != MassAgent) return;

	SyncMassToActor();
	OnMassActorActivated();

	if (auto* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
	{
		FNavLocation NavLocation;
		if (NavSystem->ProjectPointToNavigation(GetActorLocation(), NavLocation, FVector(0.f, 0.f, 500.f)))
		{
			FVector CurrentLocation = GetActorLocation();
			CurrentLocation.Z = NavLocation.Location.Z + GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			SetActorLocation(CurrentLocation);
		}
	}
}

void AHostileMassCharacter::OnEntityDetaching(const UMassAgentComponent& AgentComponent)
{
	if (!MassAgent || &AgentComponent != MassAgent) return;

	SyncActorToMass();
	OnMassActorDeactivated();
}

void AHostileMassCharacter::SyncMassToActor()
{
	const auto EntityHandle = MassAgent->GetEntityHandle();
	if (!EntityHandle.IsValid()) return;

	auto* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem) return;

	auto& EntityManager = EntitySubsystem->GetEntityManager();
	if (auto* StatusFragment = EntityManager.GetFragmentDataPtr<FHostileStatusFragment>(EntityHandle))
	{
		SetHealthPercent(StatusFragment->HealthPercent);
	}
}

void AHostileMassCharacter::SyncActorToMass()
{
	const auto EntityHandle = MassAgent->GetEntityHandle();
	if (!EntityHandle.IsValid()) return;

	auto* EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	if (!EntitySubsystem) return;

	auto& EntityManager = EntitySubsystem->GetMutableEntityManager();
	if (auto* StatusFragment = EntityManager.GetFragmentDataPtr<FHostileStatusFragment>(EntityHandle))
	{
		StatusFragment->HealthPercent = GetHealthPercent();
	}
}
