// Fill out your copyright notice in the Description page of Project Settings.


#include "HostileDeathProcessor.h"

#include "HostileMassFragments.h"
#include "MassRepresentationFragments.h"

UHostileDeathProcessor::UHostileDeathProcessor() : EntityQuery(*this)
{
	ExecutionFlags = static_cast<uint8>(EProcessorExecutionFlags::All & ~EProcessorExecutionFlags::Client);
}

void UHostileDeathProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHostileDeathFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadOnly);
}

void UHostileDeathProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ForEachEntityChunk(Context, [](FMassExecutionContext& Ctx)
	{
		const auto Deaths = Ctx.GetMutableFragmentView<FHostileDeathFragment>();
		const auto Reps   = Ctx.GetFragmentView<FMassRepresentationFragment>();

		for (int32 i = 0; i < Ctx.GetNumEntities(); ++i)
		{
			const bool bAlreadySpawnedActor =
					Reps[i].CurrentRepresentation == EMassRepresentationType::HighResSpawnedActor ||
					Reps[i].CurrentRepresentation == EMassRepresentationType::LowResSpawnedActor;

			if (!bAlreadySpawnedActor)
			{
				Ctx.Defer().DestroyEntity(Ctx.GetEntity(i));
				continue;
			}

			Deaths[i].TTL -= Ctx.GetDeltaTimeSeconds();
			if (Deaths[i].TTL <= 0.f)
			{
				Ctx.Defer().DestroyEntity(Ctx.GetEntity(i));
			}
		}
	});
}
