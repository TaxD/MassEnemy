// Fill out your copyright notice in the Description page of Project Settings.


#include "HostileMassSubsystem.h"

#include "HostileMassFragments.h"
#include "HostileMassSettings.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EQSTestingPawn.h"
#include "Kismet/GameplayStatics.h"

bool UHostileMassSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) return false;

	const UWorld* World = Outer ? Outer->GetWorld() : nullptr;
	if (!World) return false;

	if (World->GetNetMode() == NM_Client) return false;

	const auto Settings = GetDefault<UHostileMassSettings>();
	if (!Settings) return false;

	const FString CurrentMapName = FPackageName::GetShortName(
		UWorld::StripPIEPrefixFromPackageName(
			World->GetOutermost()->GetName(),
			World->StreamingLevelsPrefix
		)
	);

	for (const auto& Level : Settings->SpawnableWorlds)
	{
		if (Level.IsNull()) continue;
		if (CurrentMapName.Equals(Level.GetAssetName(), ESearchCase::IgnoreCase)) return true;
	}

	return false;
}

void UHostileMassSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	const auto Settings = GetDefault<UHostileMassSettings>();
	if (!Settings) return;

	UGameplayStatics::GetAllActorsOfClass(&InWorld, AEQSTestingPawn::StaticClass(), EQSPawns);

	for (const auto& [Type, Config] : Settings->EntityConfigs)
	{
		if (Type == EHostileType::Random) continue;

		if (auto* LoadedConfig = Config.LoadSynchronous())
		{
			EntityTemplateMap.Add(Type, LoadedConfig->GetOrCreateEntityTemplate(InWorld));
			HostileTypes.Add(Type);
		}
	}

	EQS = Settings->SpawnPointQuery.LoadSynchronous();

	SpawnerSubsystem = InWorld.GetSubsystem<UMassSpawnerSubsystem>();
	NavSystem        = FNavigationSystem::GetCurrent<UNavigationSystemV1>(&InWorld);

	if (auto* EntitySubsystem = InWorld.GetSubsystem<UMassEntitySubsystem>())
	{
		EntityManager = &EntitySubsystem->GetMutableEntityManager();
	}
}

void UHostileMassSubsystem::RequestSpawnHostileEQS(int32 Count, EHostileType Type)
{
	QueryAllEQS([this, Count, Type](const TArray<TSharedPtr<FEnvQueryResult>>& Results)
	{
		TArray<FVector> Locations;
		Locations.Reserve(Count);

		const int32 EQSNum = Results.Num();
		for (int32 i = 0; i < EQSNum; ++i)
		{
			if (!Results[i] || !Results[i]->IsSuccessful()) continue;

			const int32 BatchCount = (i == EQSNum - 1)
				                         ? Count - (Count / EQSNum) * (EQSNum - 1)
				                         : Count / EQSNum;

			Locations.Append(GetWeightedRandomLocations(*Results[i], BatchCount));
		}

		const int32          SpawnCount = FMath::Min(Count, Locations.Num());
		FHostileSpawnRequest Request;
		Request.HostileType    = Type;
		Request.Remaining      = SpawnCount;
		Request.SpawnLocations = MoveTemp(Locations);

		EnqueueSpawnRequest(MoveTemp(Request));
	});
}

void UHostileMassSubsystem::RequestSpawnHostileLocation(int32 Count, const FVector& Location, EHostileType Type)
{
	TArray<FVector> Locations = GetGridLocations(Location, Count);

	const int32          SpawnCount = FMath::Min(Count, Locations.Num());
	FHostileSpawnRequest Request;
	Request.HostileType    = Type;
	Request.Remaining      = SpawnCount;
	Request.SpawnLocations = MoveTemp(Locations);

	EnqueueSpawnRequest(MoveTemp(Request));
}

void UHostileMassSubsystem::EnqueueSpawnRequest(FHostileSpawnRequest&& Request)
{
	SpawnQueue.Enqueue(MoveTemp(Request));

	if (const auto World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(SpawnTimerHandle))
		{
			World->GetTimerManager().
			       SetTimer(SpawnTimerHandle, this, &UHostileMassSubsystem::ProcessSpawnRequest, 0.1f, true);
		}
	}
}

void UHostileMassSubsystem::ProcessSpawnRequest()
{
	if (!EntityManager || !SpawnerSubsystem.IsValid()) return;

	const auto Settings = GetDefault<UHostileMassSettings>();
	if (!Settings) return;

	int32 RemainingThisTick = Settings->MaxSpawnPerTick;

	FHostileSpawnRequest Request;
	while (SpawnQueue.Dequeue(Request))
	{
		if (!Request.IsValid()) continue;

		FMassEntityTemplate* Template = nullptr;
		if (Request.HostileType == EHostileType::Random)
		{
			if (!HostileTypes.IsEmpty())
			{
				int32 RandIndex = FMath::RandRange(0, HostileTypes.Num() - 1);
				Template        = EntityTemplateMap.Find(HostileTypes[RandIndex]);
			}
		}
		else
		{
			Template = EntityTemplateMap.Find(Request.HostileType);
		}

		if (!Template) continue;

		const int32 CurrentSpawn = FMath::Min(Request.Remaining, RemainingThisTick);

		TArray<FMassEntityHandle> RequestEntities;
		SpawnerSubsystem->SpawnEntities(*Template, CurrentSpawn, RequestEntities);

		const int32 ActualSpawned = FMath::Min(RequestEntities.Num(), Request.SpawnLocations.Num());

		const int32 Offset = Request.SpawnLocations.Num() - Request.Remaining;
		for (int32 i = 0; i < ActualSpawned; ++i)
		{
			SetMassLocation(RequestEntities[i], Request.SpawnLocations[Offset + i]);
		}

		Request.Remaining -= ActualSpawned;
		RemainingThisTick -= ActualSpawned;

		if (Request.Remaining > 0) SpawnQueue.Enqueue(MoveTemp(Request));
		if (RemainingThisTick <= 0) break;
	}

	if (SpawnQueue.IsEmpty())
	{
		if (const auto World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(SpawnTimerHandle);
		}
	}
}

void UHostileMassSubsystem::SetMassLocation(const FMassEntityHandle& Entity, const FVector& Location)
{
	FVector Position = Location;

	if (NavSystem.IsValid())
	{
		FNavLocation ProjectedLocation;
		if (NavSystem->ProjectPointToNavigation(Position, ProjectedLocation))
		{
			Position = ProjectedLocation;
		}
	}

	if (EntityManager)
	{
		if (auto TransformFragment = EntityManager->GetFragmentDataPtr<FTransformFragment>(Entity))
		{
			TransformFragment->GetMutableTransform().SetLocation(Position);
		}

		if (auto WanderFragment = EntityManager->GetFragmentDataPtr<FHostileWanderFragment>(Entity))
		{
			WanderFragment->Origin = Position;
		}
	}
}

void UHostileMassSubsystem::QueryAllEQS(
	const TFunction<void(const TArray<TSharedPtr<FEnvQueryResult>>& Results)>& Callback)
{
	if (EQSPawns.IsEmpty()) return;

	UEnvQueryManager* EQSManager = UEnvQueryManager::GetCurrent(GetWorld());
	if (!EQS || !EQSManager) return;

	auto Pending = MakeShared<TPair<int32, TArray<TSharedPtr<FEnvQueryResult>>>>();
	Pending->Key = EQSPawns.Num();
	Pending->Value.Reserve(EQSPawns.Num());

	for (auto* EQSPawn : EQSPawns)
	{
		if (!IsValid(EQSPawn))
		{
			--Pending->Key;
			continue;
		}

		FQueryFinishedSignature Delegate;
		Delegate.BindWeakLambda(
			this, [Pending, Callback](const TSharedPtr<FEnvQueryResult>& Result)
			{
				Pending->Value.Add(Result);
				if (--Pending->Key == 0) Callback(Pending->Value);
			}
		);

		FEnvQueryRequest QueryRequest(EQS, EQSPawn);
		EQSManager->RunQuery(QueryRequest, EEnvQueryRunMode::AllMatching, Delegate);
	}
}

TArray<FVector> UHostileMassSubsystem::GetGridLocations(const FVector& Center, int32 Count)
{
	const auto Settings = GetDefault<UHostileMassSettings>();
	if (!Settings) return {};

	const float Padding = Settings->GridSpacing;

	TArray<FVector> Locations;
	Locations.Reserve(Count);

	const int32 GridCol = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Count)));
	const int32 GridRow = FMath::CeilToInt(static_cast<float>(Count) / GridCol);

	const float   GridWidth  = (GridCol - 1) * Padding;
	const float   GridHeight = (GridRow - 1) * Padding;
	const FVector GridOrigin = Center - FVector(GridWidth, GridHeight, 0.f) * 0.5f;

	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Row = i / GridCol;
		const int32 Col = i % GridCol;

		Locations.Add(GridOrigin + FVector(Col, Row, 0.f) * Padding);
	}

	return Locations;
}

TArray<FVector> UHostileMassSubsystem::GetWeightedRandomLocations(const FEnvQueryResult& Result, int32 Count)
{
	if (Count <= 0 || !Result.IsSuccessful() || Result.Items.IsEmpty()) return {};

	TArray<TPair<float, int32>> Weighted;
	Weighted.Reserve(Result.Items.Num());

	for (int32 i = 0; i < Result.Items.Num(); ++i)
	{
		const float Score = FMath::Max(Result.Items[i].Score, KINDA_SMALL_NUMBER);
		const float U     = FMath::Max(FMath::FRand(), KINDA_SMALL_NUMBER);
		Weighted.Emplace(FMath::Loge(U) / Score, i);
	}

	const int32 ActualCount = FMath::Min(Count, Weighted.Num());

	std::nth_element(
		Weighted.GetData(),
		Weighted.GetData() + ActualCount,
		Weighted.GetData() + Weighted.Num(),
		[](const auto& A, const auto& B) { return A.Key > B.Key; }
	);

	TArray<FVector> Out;
	Out.Reserve(ActualCount);
	for (int32 i = 0; i < ActualCount; ++i)
	{
		Out.Add(Result.GetItemAsLocation(Weighted[i].Value));
	}

	return Out;
}
