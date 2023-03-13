// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectXPlayerController.h"
#include "ProjectX/HUD/ProjectXHUD.h"
#include "ProjectX/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "ProjectX/Character/ProjectXCharacter.h"
#include "ProjectX/GameMode/ProjectXGameMode.h"
#include "ProjectX/PlayerState/ProjectXPlayerState.h"
#include "ProjectX/PlayerController/ProjectXPlayerController.h"
#include "Net/UnrealNetwork.h"

void AProjectXPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ProjectXHUD = Cast<AProjectXHUD>(GetHUD());
}

void AProjectXPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectXPlayerController, MatchState);
}

void AProjectXPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}


void AProjectXPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}
	CountdownInt = SecondsLeft;
}

void AProjectXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AProjectXCharacter* ProjectXCharacter = Cast<AProjectXCharacter>(InPawn);
	if (ProjectXCharacter)
	{
		SetHUDHealth(ProjectXCharacter->GetHealth(), ProjectXCharacter->GetMaxHealth());
	}
}

void AProjectXPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	ProjectXHUD = ProjectXHUD == nullptr ? Cast<AProjectXHUD>(GetHUD()) : ProjectXHUD;

	bool bHUDValid = ProjectXHUD &&
		ProjectXHUD->CharacterOverlay &&
		ProjectXHUD->CharacterOverlay->HealthBar &&
		ProjectXHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		ProjectXHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		ProjectXHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void AProjectXPlayerController::SetHUDScore(float Score)
{
	ProjectXHUD = ProjectXHUD == nullptr ? Cast<AProjectXHUD>(GetHUD()) : ProjectXHUD;
	bool bHUDValid = ProjectXHUD &&
		ProjectXHUD->CharacterOverlay &&
		ProjectXHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Score));
		ProjectXHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void AProjectXPlayerController::SetHUDDefeats(int32 Defeats)
{
	ProjectXHUD = ProjectXHUD == nullptr ? Cast<AProjectXHUD>(GetHUD()) : ProjectXHUD;
	bool bHUDValid = ProjectXHUD &&
		ProjectXHUD->CharacterOverlay &&
		ProjectXHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		ProjectXHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void AProjectXPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	ProjectXHUD = ProjectXHUD == nullptr ? Cast<AProjectXHUD>(GetHUD()) : ProjectXHUD;
	bool bHUDValid = ProjectXHUD &&
		ProjectXHUD->CharacterOverlay &&
		ProjectXHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ProjectXHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AProjectXPlayerController::PollInit() 
{
	if (CharacterOverlay == nullptr)
	{
		if (ProjectXHUD && ProjectXHUD->CharacterOverlay)
		{
			CharacterOverlay = ProjectXHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}

void AProjectXPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	ProjectXHUD = ProjectXHUD == nullptr ? Cast<AProjectXHUD>(GetHUD()) : ProjectXHUD;
	bool bHUDValid = ProjectXHUD &&
		ProjectXHUD->CharacterOverlay &&
		ProjectXHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ProjectXHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void AProjectXPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	ProjectXHUD = ProjectXHUD == nullptr ? Cast<AProjectXHUD>(GetHUD()) : ProjectXHUD;
	bool bHUDValid = ProjectXHUD &&
		ProjectXHUD->CharacterOverlay &&
		ProjectXHUD->CharacterOverlay->MatchCountDownText;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FString CountdownText = FString::Printf(TEXT("%02d: %02d"), Minutes, Seconds);
		ProjectXHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(CountdownText));
	}
}


void AProjectXPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AProjectXPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float AProjectXPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AProjectXPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void AProjectXPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		ProjectXHUD = ProjectXHUD == nullptr ? Cast<AProjectXHUD>(GetHUD()) : ProjectXHUD;
		if (ProjectXHUD)
		{
			ProjectXHUD->AddCharacterOverlay();
		}
	}
}

void AProjectXPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		ProjectXHUD = ProjectXHUD == nullptr ? Cast<AProjectXHUD>(GetHUD()) : ProjectXHUD;
		if (ProjectXHUD)
		{
			ProjectXHUD->AddCharacterOverlay();
		}
	}
}

void AProjectXPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

