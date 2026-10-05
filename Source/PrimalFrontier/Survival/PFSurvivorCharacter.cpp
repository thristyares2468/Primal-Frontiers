#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Inventory/PFInventoryComponent.h"
#include "GameFramework/PlayerState.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

APFSurvivorCharacter::APFSurvivorCharacter()
{
    bReplicates = true;
    Survival = CreateDefaultSubobject<UPFPlayerSurvivalComponent>(TEXT("Survival"));
    PrimaryActorTick.bCanEverTick=true;
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    ToolHandle=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GatheringToolHandle"));
    ToolHead=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GatheringToolHead"));
    RemoteTool=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RemoteGatheringTool"));
    for(auto* Part:{ToolHandle.Get(),ToolHead.Get(),RemoteTool.Get()})
    {Part->SetStaticMesh(Cube.Object);Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);Part->SetCastShadow(false);Part->SetVisibility(false);}
    ToolHandle->SetupAttachment(GetFirstPersonCameraComponent());ToolHandle->SetOnlyOwnerSee(true);
    ToolHandle->SetRelativeLocation(FVector(45,25,-24));ToolHandle->SetRelativeRotation(FRotator(0,0,-20));ToolHandle->SetRelativeScale3D(FVector(0.04,0.04,0.4));
    ToolHead->SetupAttachment(GetFirstPersonCameraComponent());ToolHead->SetOnlyOwnerSee(true);
    ToolHead->SetRelativeLocation(FVector(45,30,-5));ToolHead->SetRelativeScale3D(FVector(0.09,0.22,0.08));
    RemoteTool->SetupAttachment(GetMesh(),TEXT("hand_r"));RemoteTool->SetOwnerNoSee(true);RemoteTool->SetRelativeScale3D(FVector(0.06,0.06,0.4));
}
void APFSurvivorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(APFSurvivorCharacter,bHasGatheringTool);}
void APFSurvivorCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if(HasAuthority())
    {
        const auto* I=GetPlayerState()?GetPlayerState()->FindComponentByClass<UPFInventoryComponent>():nullptr;
        const bool Equipped=I && I->Count(TEXT("Item_Tool"))>0 && !Survival->IsDead();
        if(bHasGatheringTool!=Equipped){bHasGatheringTool=Equipped;ForceNetUpdate();}
    }
    if(GetNetMode()!=NM_DedicatedServer)
    {ToolHandle->SetVisibility(bHasGatheringTool && IsLocallyControlled());ToolHead->SetVisibility(bHasGatheringTool && IsLocallyControlled());RemoteTool->SetVisibility(bHasGatheringTool && !IsLocallyControlled());}
}
void APFSurvivorCharacter::BeginPlay()
{
    Super::BeginPlay();
    // Keep the rendered first-person ray at the same stable eye used by server
    // interaction. Animated head sockets are not updated identically on servers.
    GetFirstPersonCameraComponent()->AttachToComponent(GetCapsuleComponent(),FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    GetFirstPersonCameraComponent()->SetRelativeLocation(FVector(0,0,BaseEyeHeight));
    GetFirstPersonCameraComponent()->SetRelativeRotation(FRotator::ZeroRotator);
    Survival->OnDeath.AddDynamic(this, &APFSurvivorCharacter::HandleDeath);
    if (Survival->IsDead()) { HandleDeath(); }
}
float APFSurvivorCharacter::TakeDamage(float Amount, const FDamageEvent& Event, AController* InstigatorController, AActor* Causer)
{
    if (!HasAuthority() || !FMath::IsFinite(Amount) || Amount <= 0.f || Survival->IsDead() || !CanBeDamaged()) { return 0.f; }
    const float Before = Survival->GetVitals().Health;
    // Keep Unreal's damage-type and event pipeline; attributes remain authoritative.
    const float Accepted = Super::TakeDamage(Amount, Event, InstigatorController, Causer);
    Survival->ApplyDamage(Accepted);
    return Before - Survival->GetVitals().Health;
}
bool APFSurvivorCharacter::CanJumpInternal_Implementation() const
{
    return Survival->CanSpendStamina(JumpStaminaCost) && Super::CanJumpInternal_Implementation();
}
void APFSurvivorCharacter::OnJumped_Implementation()
{
    Super::OnJumped_Implementation();
    if (HasAuthority()) { Survival->SpendStamina(JumpStaminaCost); }
}
void APFSurvivorCharacter::DoMove(float Right, float Forward)
{
    if (!Survival->IsDead()) { Super::DoMove(Right, Forward); }
}
void APFSurvivorCharacter::HandleDeath()
{
    StopJumping();
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
    SetActorEnableCollision(false);
    if (HasAuthority())
    {
        if (APFSurvivalGameMode* Mode = GetWorld()->GetAuthGameMode<APFSurvivalGameMode>()) { Mode->ScheduleRespawn(GetController()); }
    }
}
