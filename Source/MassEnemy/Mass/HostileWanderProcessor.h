// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HostileWanderProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSENEMY_API UHostileWanderProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHostileWanderProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
