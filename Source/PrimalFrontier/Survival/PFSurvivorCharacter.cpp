// PFSurvivorCharacter.cpp
//
// See PFSurvivorCharacter.h. Gameplay authority stays in the survival component and
// GameMode; this file wires input/movement/presentation to those rules.

#include "Survival/PFSurvivorCharacter.h"
#include "Settings/PFGameUserSettings.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Inventory/PFInventoryComponent.h"
#include "Creatures/PFCreature.h"
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

    // Greybox gathering tool built from engine cubes (hidden until a tool is carried).
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    ToolHandle=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GatheringToolHandle"));
    ToolHead=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GatheringToolHead"));
    RemoteTool=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RemoteGatheringTool"));
    for(auto* Part:{ToolHandle.Get(),ToolHead.Get(),RemoteTool.Get()})
    {
        Part->SetStaticMesh(Cube.Object);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetCastShadow(false);
        Part->SetVisibility(false);
    }

    // First-person view: attached to the camera, visible only to the owning player.
    ToolHandle->SetupAttachment(GetFirstPersonCameraComponent());
    ToolHandle->SetOnlyOwnerSee(true);
    ToolHandle->SetRelativeLocation(FVector(45,25,-24));
    ToolHandle->SetRelativeRotation(FRotator(0,0,-20));
    ToolHandle->SetRelativeScale3D(FVector(0.04,0.04,0.4));
    ToolHead->SetupAttachment(GetFirstPersonCameraComponent());
    ToolHead->SetOnlyOwnerSee(true);
    ToolHead->SetRelativeLocation(FVector(45,30,-5));
    ToolHead->SetRelativeScale3D(FVector(0.09,0.22,0.08));

    // Remote view: in the full-body mesh's right hand, hidden from the owner.
    RemoteTool->SetupAttachment(GetMesh(),TEXT("hand_r"));
    RemoteTool->SetOwnerNoSee(true);
    RemoteTool->SetRelativeScale3D(FVector(0.06,0.06,0.4));
}

void APFSurvivorCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APFSurvivorCharacter,bHasGatheringTool);
    DOREPLIFETIME(APFSurvivorCharacter,HeldMeleeItem);
}

void APFSurvivorCharacter::Tick(float DeltaSeconds)
{
    if(IsLocallyControlled())
    {
        if(const auto* Settings=UPFGameUserSettings::Get())
        {
            auto* Camera=GetFirstPersonCameraComponent();
            Camera->SetFieldOfView(Settings->Preferences.FieldOfView);
            Camera->PostProcessSettings.bOverride_MotionBlurAmount=true;
            Camera->PostProcessSettings.MotionBlurAmount=Settings->Preferences.bMotionBlur?0.5f:0;
        }
    }
    Super::Tick(DeltaSeconds);

    // Server derives the flag from the authoritative inventory (on PlayerState).
    if(HasAuthority())
    {
        const auto* I=GetPlayerState()?GetPlayerState()->FindComponentByClass<UPFInventoryComponent>():nullptr;
        const bool Equipped=I && I->GatheringHits()>1 && !Survival->IsDead();
        if(bHasGatheringTool!=Equipped)
        {
            bHasGatheringTool=Equipped;
            ForceNetUpdate();
        }
        const FName Melee=I && !Survival->IsDead()?I->MeleeItem():NAME_None;
        if(HeldMeleeItem!=Melee){HeldMeleeItem=Melee;ForceNetUpdate();}
    }

    // Anyone who renders: first-person cubes for the local player, hand cube for others.
    if(GetNetMode()!=NM_DedicatedServer)
    {
        const bool Club=HeldMeleeItem==TEXT("Item_Club") || HeldMeleeItem==TEXT("Item_BoundClub");
        if(Club!=bClubShape)
        {
            bClubShape=Club;
            ToolHandle->SetRelativeScale3D(Club?FVector(0.06,0.06,0.45):FVector(0.04,0.04,0.4));
            ToolHead->SetRelativeLocation(Club?FVector(45,25,-3):FVector(45,30,-5));
            ToolHead->SetRelativeScale3D(Club?FVector(0.12,0.12,0.22):FVector(0.09,0.22,0.08));
            RemoteTool->SetRelativeScale3D(Club?FVector(0.1,0.1,0.5):FVector(0.06,0.06,0.4));
        }
        const bool Held=bHasGatheringTool || !HeldMeleeItem.IsNone();
        ToolHandle->SetVisibility(Held && IsLocallyControlled());
        ToolHead->SetVisibility(Held && IsLocallyControlled());
        RemoteTool->SetVisibility(Held && !IsLocallyControlled());
    }
}

void APFSurvivorCharacter::DoAim(float Yaw,float Pitch)
{
    const auto* Settings=UPFGameUserSettings::Get();
    const float Scale=Settings?Settings->Preferences.LookSensitivity:1;
    Super::DoAim(Yaw*Scale,Pitch*Scale*(Settings && Settings->Preferences.bInvertLook?-1:1));
}

void APFSurvivorCharacter::BeginPlay()
{
    Super::BeginPlay();
    // Keep the rendered first-person ray at the same stable eye used by server
    // interaction. Animated head sockets are not updated identically on servers.
    // (M7: aiming at small pickups failed until the camera matched GetActorEyesViewPoint.)
    GetFirstPersonCameraComponent()->AttachToComponent(GetCapsuleComponent(),FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    GetFirstPersonCameraComponent()->SetRelativeLocation(FVector(0,0,BaseEyeHeight));
    GetFirstPersonCameraComponent()->SetRelativeRotation(FRotator::ZeroRotator);

    Survival->OnDeath.AddDynamic(this, &APFSurvivorCharacter::HandleDeath);
    // A client may receive the pawn after death already replicated; handle it now.
    if (Survival->IsDead()) { HandleDeath(); }
}

float APFSurvivorCharacter::TakeDamage(float Amount, const FDamageEvent& Event, AController* InstigatorController, AActor* Causer)
{
    if (!HasAuthority() || !FMath::IsFinite(Amount) || Amount <= 0.f || Survival->IsDead() || !CanBeDamaged()) { return 0.f; }
    const float Before = Survival->GetVitals().Health;
    // Keep Unreal's damage-type and event pipeline; attributes remain authoritative.
    // Protection does not intercept direct needs/exposure damage or unrelated damage causers.
    float HitAmount=Amount;
    if(Cast<APFCreature>(Causer))
    {
        if(auto* PS=GetPlayerState()){if(auto* I=PS->FindComponentByClass<UPFInventoryComponent>()){HitAmount*=1.f-I->CreatureHitReduction();}}
    }
    const float Accepted = Super::TakeDamage(HitAmount, Event, InstigatorController, Causer);
    Survival->ApplyDamage(Accepted);
    // Report what was actually lost (damage is clamped at remaining health).
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
    // Runs on server and clients so the corpse stops everywhere at once.
    StopJumping();
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();
    SetActorEnableCollision(false);
    if (HasAuthority())
    {
        // Respawn replaces this pawn with a fresh one after APFSurvivalGameMode::RespawnDelay.
        if (APFSurvivalGameMode* Mode = GetWorld()->GetAuthGameMode<APFSurvivalGameMode>()) { Mode->ScheduleRespawn(GetController()); }
    }
}
