// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "HostileDeathProcessor.generated.h"

/**
 * 
 */
UCLASS()
class MASSENEMY_API UHostileDeathProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UHostileDeathProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
