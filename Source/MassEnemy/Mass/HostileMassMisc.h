#pragma once

#include "CoreMinimal.h"
#include "HostileMassMisc.generated.h"

UENUM(BlueprintType)
enum class EHostileType : uint8
{
	Soldier UMETA(DisplayName = "Soldier"),
	Zombie UMETA(DisplayName = "Zombie"),

	Random UMETA(DisplayName = "Random")
};

UENUM(BlueprintType)
enum class EHostileSpawnType : uint8
{
	Location UMETA(DisplayName = "Location"),
	EQS UMETA(DisplayName = "EQS"),
};

USTRUCT()
struct FHostileSpawnRequest
{
	GENERATED_BODY()

	EHostileType HostileType;
	int32 Remaining = 0;
	TArray<FVector> SpawnLocations;

	bool IsValid() const
	{
		return Remaining > 0 && !SpawnLocations.IsEmpty();
	}
};
