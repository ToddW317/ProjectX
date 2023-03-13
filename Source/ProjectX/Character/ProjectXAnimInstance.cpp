// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectXAnimInstance.h"
#include "ProjectXCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProjectX/Weapon/Weapon.h"
#include "ProjectX/ProjectXTypes/CombatState.h"

void UProjectXAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ProjectXCharacter = Cast<AProjectXCharacter>(TryGetPawnOwner());
}

void UProjectXAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (ProjectXCharacter == nullptr)
	{
		ProjectXCharacter = Cast<AProjectXCharacter>(TryGetPawnOwner());
	}
	if (ProjectXCharacter == nullptr) return;

	FVector Velocity = ProjectXCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = ProjectXCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = ProjectXCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = ProjectXCharacter->IsWeaponEquipped();
	EquippedWeapon = ProjectXCharacter->GetEquippedWeapon();
	bIsCrouched = ProjectXCharacter->bIsCrouched;
	bAiming = ProjectXCharacter->IsAiming();
	TurningInPlace = ProjectXCharacter->GetTurningInPlace();
	bElimmed = ProjectXCharacter->IsElimmed();

	// Offset yaw For Strafing
	FRotator AimRotation = ProjectXCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ProjectXCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ProjectXCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = ProjectXCharacter->GetAO_Yaw();
	AO_Pitch = ProjectXCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && ProjectXCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("leftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		ProjectXCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (ProjectXCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = ProjectXCharacter->GetMesh()->GetSocketTransform(FName("Hand_R"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - ProjectXCharacter->GetHitTarget()));
			LookAtRotation.Roll += ProjectXCharacter->RightHandRotationRoll;
			LookAtRotation.Yaw += ProjectXCharacter->RightHandRotationYaw;
			LookAtRotation.Pitch += ProjectXCharacter->RightHandRotationPitch;
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
	}

	bUseFABRIK = ProjectXCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bUseAimOffsets = ProjectXCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bTransformRightHand = ProjectXCharacter->GetCombatState() != ECombatState::ECS_Reloading;
}
