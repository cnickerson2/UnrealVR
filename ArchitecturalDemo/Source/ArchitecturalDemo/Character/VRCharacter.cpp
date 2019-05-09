// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


    VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
    VRRoot->SetupAttachment(GetRootComponent());


    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(VRRoot);

    DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Destination Marker"));
    DestinationMarker->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

    DestinationMarker->SetVisibility(false); //ensure it is gone until it hits something
	
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    FVector NewCameraOffset = CameraComponent->GetComponentLocation() - GetActorLocation();
    NewCameraOffset.Z = 0;
    AddActorWorldOffset(NewCameraOffset);
    VRRoot->AddWorldOffset(-NewCameraOffset);

    UpdateDestinationMarker();
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRCharacter::MoveForward);

    PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRCharacter::MoveRight);

    PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Released, this, &AVRCharacter::BeginTeleport);
}

void AVRCharacter::MoveForward(float throttle)
{
    AddMovementInput(CameraComponent->GetForwardVector()*throttle);
}

void AVRCharacter::MoveRight(float throttle)
{
    AddMovementInput(CameraComponent->GetRightVector()*throttle);
}

void AVRCharacter::UpdateDestinationMarker()
{
    FHitResult OutHitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel
    (
        OutHitResult,
        CameraComponent->GetComponentLocation(),
        CameraComponent->GetComponentLocation() + CameraComponent->GetForwardVector() * TeleportDistance,
        ECC_Visibility
    );

    DestinationMarker->SetVisibility(bHit);

    if (bHit)
    {
        DestinationMarker->SetWorldLocation(OutHitResult.Location);
    }
}

void AVRCharacter::BeginTeleport()
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    if(PlayerController != nullptr)
    {
        PlayerController->PlayerCameraManager->StartCameraFade(0, 1, FadeDuration, FLinearColor::Black,false,true);
    }

    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, this, &AVRCharacter::FinishTeleport, FadeDuration);
}

void AVRCharacter::FinishTeleport()
{
    SetActorLocation(DestinationMarker->GetComponentLocation() + GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector());

    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    if (PlayerController != nullptr)
    {
        PlayerController->PlayerCameraManager->StartCameraFade(1, 0, FadeDuration, FLinearColor::Black, false, true);
    }
}
