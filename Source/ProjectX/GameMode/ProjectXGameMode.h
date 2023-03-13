// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ProjectXGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTX_API AProjectXGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	AProjectXGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class AProjectXCharacter* ElimmedCharacter, class AProjectXPlayerController* VictimController, AProjectXPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);


	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;
};
