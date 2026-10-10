// PFResourceNode.cpp — see PFResourceNode.h.

#include "Crafting/PFResourceNode.h"
#include "PFAssetPaths.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryComponent.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFInteraction.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

APFResourceNode::APFResourceNode()
{
    bReplicates=true;
    PrimaryActorTick.bCanEverTick=true;
    PrimaryActorTick.TickInterval=0.2f;
    // Placeholder cube that blocks Visibility traces only (targetable, not solid).
    Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Mesh->SetStaticMesh(Cube.Object);
    Mesh->SetRelativeScale3D(FVector(0.65));
    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    Mesh->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
    Label=CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(Mesh);
    Label->SetRelativeLocation(FVector(0,0,85));
    Label->SetRelativeRotation(FRotator(0,180,0));
    Label->SetWorldSize(22);
}

void APFResourceNode::BeginPlay()
{
    Super::BeginPlay();
    // Clients load the catalogs too, for the label's display name.
    if(!Catalog){Catalog=LoadObject<UPFCraftingCatalog>(nullptr,PFAssetPaths::CraftingCatalog);}
    if(!Items){Items=LoadObject<UPFItemCatalog>(nullptr,PFAssetPaths::ItemCatalog);}
    // An invalid ResourceId leaves HitsRemaining at 0 (an inert node).
    if(HasAuthority())
    {
        const auto* D=Catalog?Catalog->Resource(ResourceId,Items):nullptr;
        HitsRemaining=D?D->Hits:0;
    }
}

bool APFResourceNode::IsConfigurationValid() const {return Catalog && Catalog->Resource(ResourceId,Items)!=nullptr;}

void APFResourceNode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APFResourceNode,ResourceId);
    DOREPLIFETIME(APFResourceNode,HitsRemaining);
    DOREPLIFETIME(APFResourceNode,RespawnAt);
}

void APFResourceNode::Tick(float Delta)
{
    Super::Tick(Delta);
    const auto* D=Catalog?Catalog->Resource(ResourceId,Items):nullptr;
    if(!D){return;}
    const double Now=UPFInventoryComponent::ServerTime(GetWorld());
    // Server: refill a depleted node once its regrowth deadline passes.
    if(HasAuthority() && RespawnAt>0 && Now>=RespawnAt)
    {
        HitsRemaining=D->Hits;
        RespawnAt=0;
        ForceNetUpdate();
        UE_LOG(LogPFSurvival,Display,TEXT("[PrimalGathering] Respawned %s"),*GetName());
    }
    // Rendering: hide the cube while depleted and show hits or a regrowth countdown.
    if(GetNetMode()!=NM_DedicatedServer)
    {
        Mesh->SetVisibility(HitsRemaining>0,false);
        Label->SetText(FText::FromString(HitsRemaining>0 ? FString::Printf(TEXT("E: %s (%d hits)"),*D->DisplayName.ToString(),HitsRemaining) :
            FString::Printf(TEXT("Depleted - %ds"),FMath::Max(0,FMath::CeilToInt(RespawnAt-Now)))));
    }
}

bool APFResourceNode::RestorePersistence(int32 Hits, double RemainingRespawn)
{
    const auto* D = Catalog ? Catalog->Resource(ResourceId, Items) : nullptr;
    if (!HasAuthority() || !D || Hits < 0 || Hits > D->Hits || !FMath::IsFinite(RemainingRespawn) ||
        RemainingRespawn < 0 || RemainingRespawn > D->RespawnSeconds || (Hits > 0 && RemainingRespawn != 0)) { return false; }
    HitsRemaining = Hits;
    RespawnAt = Hits == 0 ? UPFInventoryComponent::ServerTime(GetWorld()) + RemainingRespawn : 0;
    NextHitAt = UPFInventoryComponent::ServerTime(GetWorld()) + 0.5;
    ForceNetUpdate(); return true;
}

bool APFResourceNode::Gather(APawn* Pawn)
{
    if(!HasAuthority() || !IsValid(Pawn) || !Pawn->HasAuthority() || Pawn->GetWorld()!=GetWorld() || !Pawn->GetController() || HitsRemaining<=0){return false;}
    auto* V=Pawn->FindComponentByClass<UPFPlayerSurvivalComponent>();
    auto* I=Pawn->GetPlayerState()?Pawn->GetPlayerState()->FindComponentByClass<UPFInventoryComponent>():nullptr;
    const auto* D=Catalog?Catalog->Resource(ResourceId,Items):nullptr;
    const double Now=UPFInventoryComponent::ServerTime(GetWorld());
    if(!V || V->IsDead() || !I || !D || Now<NextHitAt){return false;}
    // Derive reach and aim from the possessed pawn. Clients never supply a target, yield or damage.
    if(PFInteraction::FindTarget(Pawn)!=this){return false;}
    I->PruneExpired();
    // Tool tiers improve actions, never the node's finite total yield.
    const int32 Damage=FMath::Min(HitsRemaining,I->GatheringHits());
    auto* Progression=Pawn->GetPlayerState()->FindComponentByClass<UPFProgressionComponent>();
    FPFProgressionRecord Reward;FString Error;
    if(!Progression || !Progression->PrepareGather(D->YieldItem,Pawn,Reward,Error)){return false;}
    // Grant first: if the bag is full the node keeps its hits.
    if(!I->Grant(D->YieldItem,D->YieldPerHit*Damage)){return false;}
    Progression->CommitGather(Reward); // Same server-thread transaction; never reward a failed/full-bag grant.
    HitsRemaining-=Damage;
    NextHitAt=Now+0.5;
    if(HitsRemaining==0){RespawnAt=Now+D->RespawnSeconds;}
    ForceNetUpdate();
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalGathering] %s yielded %d %s hits=%d pawn=%s"),*GetName(),D->YieldPerHit*Damage,*D->YieldItem.ToString(),HitsRemaining,*Pawn->GetName());
    return true;
}
