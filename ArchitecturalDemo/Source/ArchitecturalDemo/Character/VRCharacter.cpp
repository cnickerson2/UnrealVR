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
#include "MotionControllerComponent.h"

AVRCharacter::AVRCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
    VRRoot->SetupAttachment(GetRootComponent());

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    CameraComponent->SetupAttachment(VRRoot);

    LeftHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftHand"));
    LeftHand->SetupAttachment(VRRoot);
    LeftHand->SetTrackingSource(EControllerHand::Left);

    RightHand = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));
    RightHand->SetupAttachment(VRRoot);
    RightHand->SetTrackingSource(EControllerHand::Right);

    DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
    DestinationMarker->SetupAttachment(VRRoot);

    PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
    PostProcessComponent->SetupAttachment(VRRoot);
}

void AVRCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Ensure Destination Marker isn't showing until it hits something
    DestinationMarker->SetVisibility(false); 

    SetupBlinders();
}

void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRCharacter::MoveForward);

    PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRCharacter::MoveRight);

    PlayerInputComponent->BindAction(TEXT("TeleportRight"), IE_Released, this, &AVRCharacter::TeleportRight);
    PlayerInputComponent->BindAction(TEXT("TeleportLeft"), IE_Released, this, &AVRCharacter::TeleportLeft);
    PlayerInputComponent->BindAction(TEXT("TeleportCenter"), IE_Released, this, &AVRCharacter::TeleportCenter);
}

void AVRCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CenterCharacterOnHead();

    UpdateDestinationMarker();

    UpdateBlinders();
}

void AVRCharacter::CenterCharacterOnHead()
{
    FVector NewCameraOffset = CameraComponent->GetComponentLocation() - GetActorLocation();
    // Ensure the Offset only occurs on the X and Y axis
    NewCameraOffset.Z = 0;
    AddActorWorldOffset(NewCameraOffset);
    VRRoot->AddWorldOffset(-NewCameraOffset);
}

void AVRCharacter::MoveForward(float Throttle)
{
    AddMovementInput(CameraComponent->GetForwardVector()*Throttle);
}

void AVRCharacter::MoveRight(float Throttle)
{
    AddMovementInput(CameraComponent->GetRightVector()*Throttle);
}

void AVRCharacter::UpdateDestinationMarker()
{
    FVector TeleportLocation;
    bool bFoundSpotOnNavMesh = CanFindTeleportDestination(TeleportLocation);

    if (bFoundSpotOnNavMesh)
    {
        DestinationMarker->SetWorldLocation(TeleportLocation);
    }

    DestinationMarker->SetVisibility(bFoundSpotOnNavMesh);
}

bool AVRCharacter::CanFindTeleportDestination(FVector & OutLocation)
{
    USceneComponent* OriginComponent = nullptr;
    GetTeleportOrigin(OriginComponent);
    if (OriginComponent == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] : OriginComponent was null"), *GetName());
        return false;
    }
  
    FHitResult HitResult;        
    if (!CanHitSomethingWithinTeleportDistance(HitResult, OriginComponent))
    {
        return false;
    }

    FNavLocation NavLocation;
    if (!IsWhatIHitNearANavMesh(NavLocation,HitResult))
    {
        return false;
    }

    OutLocation = NavLocation.Location;

    return true;
}

void AVRCharacter::GetTeleportOrigin(USceneComponent* &OutOriginComponent)
{
    switch (LastUsedHandedness)
    {
    case EHandedness::Left:
        OutOriginComponent = LeftHand;
        break;
    case EHandedness::Right:
        OutOriginComponent = RightHand;
        break;
    default:
        OutOriginComponent = CameraComponent;
        break;
    }
}

bool AVRCharacter::CanHitSomethingWithinTeleportDistance(FHitResult& OutHitResult, USceneComponent* const & OriginComponent)
{
    uint8 AxisRotation = OriginComponent == LeftHand || OriginComponent == RightHand ? 30 : 0;

    return GetWorld()->LineTraceSingleByChannel
    (
        OutHitResult,
        OriginComponent->GetComponentLocation(),
        OriginComponent->GetComponentLocation() +
        OriginComponent->GetForwardVector().RotateAngleAxis
        (
            AxisRotation,
            OriginComponent->GetRightVector()
        ) *
        TeleportDistance,
        ECC_Visibility
    );
}

bool AVRCharacter::IsWhatIHitNearANavMesh(FNavLocation& OutNavLocation, FHitResult const & HitResult)
{
    return UNavigationSystemV1::GetNavigationSystem(GetWorld())->ProjectPointToNavigation
    (
        HitResult.Location,
        OutNavLocation,
        TeleportProjectionExtent
    );
}

void AVRCharacter::TeleportRight()
{
    if (WasThisHandTheSameAsLastTime(EHandedness::Right))
    {
        BeginTeleport();
    }
}

void AVRCharacter::TeleportLeft()
{
    if (WasThisHandTheSameAsLastTime(EHandedness::Left))
    {
        BeginTeleport();
    }
}

void AVRCharacter::TeleportCenter()
{
    if(WasThisHandTheSameAsLastTime(EHandedness::None))
    {
        BeginTeleport();
    }
}

bool AVRCharacter::WasThisHandTheSameAsLastTime(EHandedness ThisTimesHandedness)
{
    bool bIsTheSame = LastUsedHandedness == ThisTimesHandedness;
    LastUsedHandedness = ThisTimesHandedness;
    return bIsTheSame;
}

void AVRCharacter::BeginTeleport()
{
    StartFade(0, 1);

    FTimerHandle Handle;
    GetWorldTimerManager().SetTimer(Handle, this, &AVRCharacter::FinishTeleport, FadeDuration);
}

void AVRCharacter::FinishTeleport()
{
    // Move the Character
    SetActorLocation(DestinationMarker->GetComponentLocation() + GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector());

    StartFade(1, 0);
}

void AVRCharacter::StartFade(float FromAlpha, float ToAlpha)
{
    APlayerController* PlayerController = Cast<APlayerController>(GetController());

    if (PlayerController != nullptr)
    {
        PlayerController->PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, FadeDuration, FLinearColor::Black, false, true);
    }
}

void AVRCharacter::SetupBlinders()
{
    if (BlinderMaterialBase == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] : BlinderMaterialBase was not set"), *GetName());
        return;
    }
    DynamicMaterialInstance = UMaterialInstanceDynamic::Create(BlinderMaterialBase, this);
    PostProcessComponent->AddOrUpdateBlendable(DynamicMaterialInstance);
}

void AVRCharacter::UpdateBlinders()
{
    if (DynamicMaterialInstance == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] : DynamicMaterialInstance is null. Check to make sure BlinderMaterialBase was set"), *GetName());
        return;
    }
    if (RadiusVsVelocity == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("[%s] : RadiusVsVelocity Curve Float was not set"), *GetName());
        return;
    }

    float Speed = GetVelocity().Size();
    float Radius = RadiusVsVelocity->GetFloatValue(Speed);

    DynamicMaterialInstance->SetScalarParameterValue(TEXT("Radius"), Radius);

    FVector2D Centre = GetBlinderCenter();
    DynamicMaterialInstance->SetVectorParameterValue(TEXT("Centre"), FLinearColor(Centre.X, Centre.Y, 0));
}

FVector2D AVRCharacter::GetBlinderCenter()
{
    FVector CharacterVelocity = GetVelocity().GetSafeNormal();
    if (CharacterVelocity.IsNearlyZero())
    {
        // Not moving, so assume center of the screen
        return FVector2D(0.5f, 0.5f);
    }

    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController == nullptr)
    {
        // The controller is not a player. So it's not necessary
        return FVector2D(0.5f, 0.5f);
    }
    
    FVector BlinderDirection;
    // Get the placement of the blinder from a distance from the camera
    const uint16 DISTANCE_FROM_CAMERA = 1000;
    if (FVector::DotProduct(CameraComponent->GetForwardVector(), CharacterVelocity) > 0)
    {
        BlinderDirection = CameraComponent->GetComponentLocation() + CharacterVelocity * DISTANCE_FROM_CAMERA;
    }
    else
    {
        BlinderDirection = CameraComponent->GetComponentLocation() - CharacterVelocity * DISTANCE_FROM_CAMERA;
    }

    FVector2D ScreenLocation;
    PlayerController->ProjectWorldLocationToScreen(BlinderDirection, ScreenLocation);

    // Get the pixels of the viewport
    int32 ViewportX, ViewportY;
    PlayerController->GetViewportSize(ViewportX, ViewportY);

    // Divide the Screen location by the viewport to get a value between 0 and 1
    ScreenLocation.X /= ViewportX;
    ScreenLocation.Y /= ViewportY;

    return ScreenLocation;
}