// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

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

    PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
    PostProcessComponent->SetupAttachment(GetRootComponent());


}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
    Super::BeginPlay();

    DestinationMarker->SetVisibility(false); //ensure it is gone until it hits something

    UpdateBlinders();
}

void AVRCharacter::UpdateBlinders()
{
    if (BlinderMaterialBase == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] : BlinderMaterialBase was not set"), *GetName());
        return;
    }
    DynamicMaterialInstance = UMaterialInstanceDynamic::Create(BlinderMaterialBase, this);
    DynamicMaterialInstance->SetScalarParameterValue(TEXT("Radius"), 1.0);

    PostProcessComponent->AddOrUpdateBlendable(DynamicMaterialInstance);
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

    if (RadiusVsVelocity != nullptr)
    {
        float VelocityOfCharacter = GetVelocity().Size();
        float Radius = RadiusVsVelocity->GetFloatValue(VelocityOfCharacter);

        DynamicMaterialInstance->SetScalarParameterValue(TEXT("Radius"), Radius);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] : RadiusVsVelocity Curve Float was not set"), *GetName());
    }
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
    FVector TeleportLocation;

    if (FindTeleportDestination(TeleportLocation))
    {
        DestinationMarker->SetWorldLocation(TeleportLocation);
        DestinationMarker->SetVisibility(true);
    }
    else
    {
        DestinationMarker->SetVisibility(false);
    }

}

void AVRCharacter::BeginTeleport()
{
    StartFade(0, 1);

    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, this, &AVRCharacter::FinishTeleport, FadeDuration);
}

void AVRCharacter::FinishTeleport()
{
    SetActorLocation(DestinationMarker->GetComponentLocation() + GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector());

    StartFade(1, 0);
}

bool AVRCharacter::FindTeleportDestination(FVector & OutLocation)
{
    FHitResult OutHitResult;
    bool bHit = GetWorld()->LineTraceSingleByChannel
    (
        OutHitResult,
        CameraComponent->GetComponentLocation(),
        CameraComponent->GetComponentLocation() + CameraComponent->GetForwardVector() * TeleportDistance,
        ECC_Visibility
    );

    if (!bHit)
    {
        return false;
    }

    FNavLocation NavLocation;

    bool bOnNavMesh = UNavigationSystemV1::GetNavigationSystem(GetWorld())->ProjectPointToNavigation
    (
        OutHitResult.Location,
        NavLocation,
        TeleportProjectionExtent
    );

    if (!bOnNavMesh)
    {
        return false;
    }

    OutLocation = NavLocation.Location;

    return true;
}

void AVRCharacter::StartFade(float FromAlpha, float ToAlpha)
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    if (PlayerController != nullptr)
    {
        PlayerController->PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, FadeDuration, FLinearColor::Black, false, true);
    }
}
