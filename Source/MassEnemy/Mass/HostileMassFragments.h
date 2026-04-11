#pragma once

#include "CoreMinimal.h"
#include "MassEntityElementTypes.h"
#include "HostileMassFragments.generated.h"

USTRUCT()
struct FHostileWanderFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FVector Origin = FVector::ZeroVector;
	UPROPERTY(EditAnywhere)
	FVector TargetLocation = FVector::ZeroVector;
	UPROPERTY(EditAnywhere)
	float TimeUntilNewTarget = 0.f;
	UPROPERTY(EditAnywhere)
	float Radius = 1000.f;
	UPROPERTY(EditAnywhere)
	float Speed = 300.f;
};

USTRUCT()
struct FHostileStatusFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float HealthPercent = 1.f;
};

USTRUCT()
struct FHostileDeathFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float TTL = 5.f;
};
