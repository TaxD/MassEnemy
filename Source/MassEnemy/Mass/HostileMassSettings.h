// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HostileMassMisc.h"
#include "MassEntityConfigAsset.h"
#include "Engine/DeveloperSettings.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "HostileMassSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName = "Hostile Mass Settings"))
class MASSENEMY_API UHostileMassSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Config, meta = (ClampMin = "1", ClampMax = "128"))
	int32 MaxSpawnPerTick = 64;

	UPROPERTY(EditDefaultsOnly, Config)
	TMap<EHostileType, TSoftObjectPtr<UMassEntityConfigAsset>> EntityConfigs;

	UPROPERTY(EditDefaultsOnly, Config)
	TSoftObjectPtr<UEnvQuery> SpawnPointQuery;

	UPROPERTY(EditDefaultsOnly, Config)
	TArray<TSoftObjectPtr<UWorld>> SpawnableWorlds;

	UPROPERTY(EditDefaultsOnly, Config)
	float GridSpacing = 200.f;
};
