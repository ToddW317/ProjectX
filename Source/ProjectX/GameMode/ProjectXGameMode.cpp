// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectXGameMode.h"
#include "ProjectX/Character/ProjectXCharacter.h"
#include "ProjectX/PlayerController/ProjectXPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ProjectX/PlayerState/ProjectXPlayerState.h"

AProjectXGameMode::AProjectXGameMode()
{
	bDelayedStart = true;
}

void AProjectXGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AProjectXGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
}

void AProjectXGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AProjectXPlayerController* ProjectXPlayer = Cast<AProjectXPlayerController>(*It);
		if (ProjectXPlayer)
		{
			ProjectXPlayer->OnMatchStateSet(MatchState);
		}
	}
}



void AProjectXGameMode::PlayerEliminated(AProjectXCharacter* ElimmedCharacter, AProjectXPlayerController* VictimController, AProjectXPlayerController* AttackerController)
{

	AProjectXPlayerState* AttackerPlayerState = AttackerController ? Cast<AProjectXPlayerState>(AttackerController->PlayerState) : nullptr;
	AProjectXPlayerState* VictimPlayerState = VictimController ? Cast<AProjectXPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}

void AProjectXGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
