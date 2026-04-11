// Fill out your copyright notice in the Description page of Project Settings.


#include "HostileWanderProcessor.h"

#include "HostileMassFragments.h"
#include "MassCommonFragments.h"
#include "MassLODFragments.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

UHostileWanderProcessor::UHostileWanderProcessor() : EntityQuery(*this)
{
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::All & ~EProcessorExecutionFlags::Client);
}

void UHostileWanderProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHostileWanderFragment>(EMassFragmentAccess::ReadWrite);

	EntityQuery.AddRequirement<FHostileDeathFragment>(EMassFragmentAccess::None, EMassFragmentPresence::None);

	EntityQuery.AddTagRequirement<FMassHighLODTag>(EMassFragmentPresence::None);
	EntityQuery.AddTagRequirement<FMassOffLODTag>(EMassFragmentPresence::None);
}

void UHostileWanderProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Ctx)
	{
		const auto Transforms = Ctx.GetMutableFragmentView<FTransformFragment>();
		const auto Wanders    = Ctx.GetMutableFragmentView<FHostileWanderFragment>();

		for (int32 i = 0; i < Ctx.GetNumEntities(); ++i)
		{
			FTransform&             Transform = Transforms[i].GetMutableTransform();
			FHostileWanderFragment& Wander    = Wanders[i];

			Wander.TimeUntilNewTarget -= Ctx.GetDeltaTimeSeconds();

			if (auto* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Ctx.GetWorld()))
			{
				if (Wander.TimeUntilNewTarget <= 0.f)
				{
					Wander.TimeUntilNewTarget = FMath::FRandRange(3.f, 6.f);

					FNavLocation NavResult;
					if (NavSystem->GetRandomReachablePointInRadius(Wander.Origin, Wander.Radius, NavResult))
					{
						Wander.TargetLocation = NavResult.Location;
					}
				}
			}

			const FVector CurrentLocation = Transform.GetLocation();
			const FVector Delta2D(
				Wander.TargetLocation.X - CurrentLocation.X,
				Wander.TargetLocation.Y - CurrentLocation.Y,
				0.f
			);

			const float DistSq   = Delta2D.SizeSquared();
			const float StepSize = Wander.Speed * Ctx.GetDeltaTimeSeconds();
			if (DistSq < FMath::Square(StepSize)) continue;

			const float   InvDist = FMath::InvSqrt(DistSq);
			const FVector Dir     = Delta2D * InvDist;

			FVector NewLocation = CurrentLocation + Dir * StepSize;
			// NewLocation.Z       = CurrentLocation.Z;
			Transform.SetLocation(NewLocation);
			Transform.SetRotation(Dir.ToOrientationQuat());
		}
	});
}
