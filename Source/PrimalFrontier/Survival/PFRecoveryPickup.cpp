// PFRecoveryPickup.cpp — see PFRecoveryPickup.h.

#include "Survival/PFRecoveryPickup.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

APFRecoveryPickup::APFRecoveryPickup()
{
    bReplicates = true;
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.f;

    // Small cube that blocks only the Visibility channel, so interaction traces hit it
    // but players and physics pass through.
    auto* Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh")); SetRootComponent(Mesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Mesh->SetStaticMesh(Cube.Object); Mesh->SetRelativeScale3D(FVector(0.4));
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Mesh->SetCollisionResponseToAllChannels(ECR_Ignore); Mesh->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);

    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label")); Label->SetupAttachment(Mesh);
    Label->SetText(FText::FromString(TEXT("E: RATION"))); Label->SetWorldSize(35);
    Label->SetRelativeLocation(FVector(0,0,90)); Label->SetRelativeRotation(FRotator(0,180,0));
}

void APFRecoveryPickup::BeginPlay()
{
    Super::BeginPlay();
    if (HasAuthority())
    {
        // Invalid designer data fails closed. Clients never choose or extend this deadline.
        const double Lifetime = FMath::IsFinite(ShelfLifeSeconds) ? FMath::Clamp(double(ShelfLifeSeconds),0.0,86400.0) : 0.0;
        ExpiresAtServerTime = GetServerTime() + Lifetime;
        ForceNetUpdate();
    }
}

void APFRecoveryPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APFRecoveryPickup,ExpiresAtServerTime);
}

double APFRecoveryPickup::GetServerTime() const
{
    const auto* State = GetWorld()->GetGameState();
    return State ? State->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

float APFRecoveryPickup::GetRemainingFreshSeconds() const
{
    return float(FMath::Max(0.0,ExpiresAtServerTime-GetServerTime()));
}

void APFRecoveryPickup::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    const float Remaining = GetRemainingFreshSeconds();
    // Server cleanup of spoiled food (consumption also checks the deadline directly).
    if (HasAuthority() && !bConsumed && Remaining <= 0)
    {
        bConsumed = true;
        UE_LOG(LogPFSurvival,Display,TEXT("[PrimalSurvival] Ration expired on server: %s"),*GetName());
        Destroy(); return;
    }
    if (GetNetMode()!=NM_DedicatedServer)
    {
        Label->SetText(FText::FromString(FString::Printf(TEXT("E: RATION (%ds fresh)"),FMath::CeilToInt(Remaining))));
    }
}

bool APFRecoveryPickup::TryConsume(APawn* Consumer)
{
    // Authority, freshness, a valid living consumer in the same world, within 2.5 m.
    if (!HasAuthority() || bConsumed || GetRemainingFreshSeconds() <= 0 || !IsValid(Consumer) || !Consumer->HasAuthority() || Consumer->GetWorld() != GetWorld() ||
        !Consumer->GetController() || FVector::DistSquared(Consumer->GetActorLocation(),GetActorLocation()) > FMath::Square(250.f)) { return false; }
    auto* S = Consumer->FindComponentByClass<UPFPlayerSurvivalComponent>();
    if (!S || S->IsDead()) { return false; }
    // Nothing may block the line from the consumer's eyes to the ration.
    FVector Eye; FRotator Look; Consumer->GetActorEyesViewPoint(Eye,Look);
    FHitResult Hit; FCollisionQueryParams Params(SCENE_QUERY_STAT(PFConsume),false,Consumer);
    if (GetWorld()->LineTraceSingleByChannel(Hit,Eye,GetActorLocation(),ECC_Visibility,Params) && Hit.GetActor() != this) { return false; }
    // Don't waste the ration if it would restore nothing.
    if ((S->GetVitals().Hunger >= 100.f || FoodRecovery == 0) && (S->GetVitals().Thirst >= 100.f || WaterRecovery == 0)) { return false; }
    if (!S->RecoverNeeds(FoodRecovery,WaterRecovery)) { return false; }
    bConsumed = true;
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalSurvival] Ration consumed on server by %s: Hunger=%.1f Thirst=%.1f"),
        *Consumer->GetName(), S->GetVitals().Hunger, S->GetVitals().Thirst);
    Destroy(); return true;
}
