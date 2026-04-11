// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassAgentComponent.h"
#include "GameFramework/Character.h"
#include "HostileMassCharacter.generated.h"

UCLASS()
class MASSENEMY_API AHostileMassCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AHostileMassCharacter();

	UFUNCTION(BlueprintCallable)
	void KillHostile(float TTL);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnMassActorActivated();
	UFUNCTION(BlueprintImplementableEvent)
	void OnMassActorDeactivated();

	UFUNCTION(BlueprintNativeEvent)
	void SetHealthPercent(float Percent);
	UFUNCTION(BlueprintNativeEvent, BlueprintPure)
	float GetHealthPercent() const;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMassAgentComponent> MassAgent;

private:
	void OnEntityAssociated(const UMassAgentComponent& AgentComponent);
	void OnEntityDetaching(const UMassAgentComponent& AgentComponent);

	void SyncMassToActor();
	void SyncActorToMass();
};
