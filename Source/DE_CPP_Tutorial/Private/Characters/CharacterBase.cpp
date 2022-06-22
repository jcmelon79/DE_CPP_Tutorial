// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterBase.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "DrawDebugHelpers.h"
#include "Interact/InteractInterface.h"
#include "MatineeCameraShake.h"
#include "Interact/InteractableBase.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerStaticMesh"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->OnComponentBeginOverlap.AddDynamic(this, &ACharacterBase::OnOverlapBegin);

	BaseTurnRate = 45.0f;
	BaseLookUpAtRate = 45.0f;
	TraceDistance = 2000.0f;

	ImpulseForce = 500.f;

	bApplyRadialForce = true;
	ImpactRadius = 200.0f;
	RadialImpactForce = 2000.0f;

	StartScale = FVector(2.5, 2.5, 2.5);
	TargetScale = FVector(2.8f, 2.8f, 2.3f);
}

void ACharacterBase::MoveForward(float Value)
{
	if ((Controller) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ACharacterBase::MoveRight(float Value)
{
	if ((Controller) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void ACharacterBase::TurnAtRate(float Value)
{
	AddControllerYawInput(Value * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ACharacterBase::LookUpAtRate(float Value)
{
	AddControllerPitchInput(Value * BaseLookUpAtRate * GetWorld()->GetDeltaSeconds());
}

void ACharacterBase::InteractPressed()
{
	TraceForward();

	if (FocusedActor)
	{
		IInteractInterface* Interface = Cast<IInteractInterface>(FocusedActor);
		if (Interface)
		{
			Interface->Execute_OnInteract(FocusedActor, this);
		}
	}	
}

void ACharacterBase::TraceForward_Implementation()
{
	FVector Loc;
	FRotator Rot;
	FHitResult Hit;

	GetController()->GetPlayerViewPoint(Loc, Rot);

	FVector Start = Loc;
	FVector End = Start + (Rot.Vector() * TraceDistance);

	FCollisionQueryParams TraceParams;
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Orange, false, 2.0f);

	if (bHit)
	{
		//DrawDebugBox(GetWorld(), Hit.ImpactPoint, FVector(5, 5, 5), FColor::Emerald, false, 2.0f);
		
		AActor* Interactable = Hit.GetActor();

		if (Interactable)
		{
			if (Interactable != FocusedActor)
			{
				if (FocusedActor)
				{
					IInteractInterface* Interface = Cast<IInteractInterface>(FocusedActor);
					if (Interface)
					{
						Interface->Execute_EndFocus(FocusedActor);
					}
				}
				IInteractInterface* Interface = Cast<IInteractInterface>(Interactable);
				if (Interface)
				{
					Interface->Execute_StartFocus(Interactable);
				}
				FocusedActor = Interactable;
			}
		}
		else
		{
			if (FocusedActor)
			{
				IInteractInterface* Interface = Cast<IInteractInterface>(FocusedActor);
				if (Interface)
				{
					Interface->Execute_EndFocus(FocusedActor);
				}
			}
			FocusedActor = nullptr;
		}
	}
}

void ACharacterBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IInteractInterface* Interface = Cast<IInteractInterface>(OtherActor);
	if (Interface)
	{
		Interface->Execute_OnInteract(OtherActor, this);
	}
}

void ACharacterBase::FireForward()
{
	GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(CamShake, 1.0f);

	FVector Loc;
	FRotator Rot;
	FHitResult Hit;

	GetController()->GetPlayerViewPoint(Loc, Rot);

	FVector Start = Loc;
	FVector End = Start + (Rot.Vector() * TraceDistance);

	FCollisionQueryParams TraceParams;
	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, TraceParams);

	if (bApplyRadialForce)
	{
		FCollisionShape SphereCol = FCollisionShape::MakeSphere(ImpactRadius);

		bool bSweepHit = GetWorld()->SweepMultiByChannel(HitActors, Hit.Location, Hit.Location + FVector(0.01f, 0.01f, 0.01f),
			FQuat::Identity, ECC_WorldStatic, SphereCol);
		DrawDebugSphere(GetWorld(), Hit.Location, ImpactRadius, 50, FColor::Orange, false, 2.0f);
		if (bSweepHit)
		{
			SpawnObject(Hit.Location, Rot);
			for (auto& HitActor : HitActors)
			{
				UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(HitActor.GetActor()->GetRootComponent());
				if (StaticMeshComp)
				{
					StaticMeshComp->AddRadialImpulse(Hit.Location, ImpactRadius, RadialImpactForce,
						ERadialImpulseFalloff::RIF_Constant, true);
				}
			}
		}
	}
	else
	{
		if (bHit)
		{			
			UStaticMeshComponent* StaticMeshComp = Cast<UStaticMeshComponent>(Hit.GetActor()->GetRootComponent());
			if (StaticMeshComp && Hit.GetActor()->IsRootComponentMovable())
			{
				FVector CameraForward = CameraComp->GetForwardVector();
				StaticMeshComp->AddImpulse(CameraForward * ImpulseForce * StaticMeshComp->GetMass());
			}
		}
	}
	
}

void ACharacterBase::SpawnObject(FVector Loc, FRotator Rot)
{
	FActorSpawnParameters SpawnParams;
	AInteractableBase* SpawnedActorRef= GetWorld()->SpawnActor<AInteractableBase>(ActorToSpawn, Loc, Rot, SpawnParams);
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TraceForward_Implementation();

	SquashTimeline.TickTimeline(DeltaTime);
}

// Called to bind functionality to input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ACharacterBase::InteractPressed);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ACharacterBase::FireForward);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ACharacterBase::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ACharacterBase::MoveRight);
	
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ACharacterBase::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ACharacterBase::LookUpAtRate);
}

void ACharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (CurveFloat)
	{
		FOnTimelineFloat TimelineProgress;
		TimelineProgress.BindUFunction(this, FName("SquashProgress"));
		SquashTimeline.AddInterpFloat(CurveFloat, TimelineProgress);
		SquashTimeline.SetLooping(false);		
		SquashTimeline.PlayFromStart();
	}
}

void ACharacterBase::SquashProgress(float Value)
{
	FVector NewScale = FMath::Lerp(StartScale, TargetScale, Value);
	MeshComp->SetWorldScale3D(NewScale);
}
