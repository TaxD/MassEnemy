// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HostileMassMisc.h"
#include "MassSpawnerSubsystem.h"
#include "NavigationSystem.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "Subsystems/WorldSubsystem.h"
#include "HostileMassSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MASSENEMY_API UHostileMassSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	UFUNCTION(BlueprintCallable)
	void RequestSpawnHostileEQS(int32 Count, EHostileType Type = EHostileType::Random);
	UFUNCTION(BlueprintCallable)
	void RequestSpawnHostileLocation(int32 Count, const FVector& Location, EHostileType Type = EHostileType::Random);

private:
	void EnqueueSpawnRequest(FHostileSpawnRequest&& Request);
	void ProcessSpawnRequest();

	void SetMassLocation(const FMassEntityHandle& Entity, const FVector& Location);

	void QueryAllEQS(const TFunction<void(const TArray<TSharedPtr<FEnvQueryResult>>& Results)>& Callback);

	TArray<FVector> GetGridLocations(const FVector& Center, int32 Count);
	TArray<FVector> GetWeightedRandomLocations(const FEnvQueryResult& Result, int32 Count);

	UPROPERTY()
	TArray<AActor*> EQSPawns;

	UPROPERTY()
	TObjectPtr<UEnvQuery> EQS;

	UPROPERTY()
	TWeakObjectPtr<UMassSpawnerSubsystem> SpawnerSubsystem;
	UPROPERTY()
	TWeakObjectPtr<UNavigationSystemV1> NavSystem;

	FMassEntityManager* EntityManager = nullptr;

	TMap<EHostileType, FMassEntityTemplate> EntityTemplateMap;
	TArray<EHostileType>                    HostileTypes;

	TQueue<FHostileSpawnRequest, EQueueMode::SingleThreaded> SpawnQueue;
	FTimerHandle                                             SpawnTimerHandle;
};
