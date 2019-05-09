// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"

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
