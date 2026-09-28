#include "Crafting/PFResourceNode.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryComponent.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
APFResourceNode::APFResourceNode()
{
    bReplicates=true;PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=0.2f;
    Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));SetRootComponent(Mesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));Mesh->SetStaticMesh(Cube.Object);
    Mesh->SetRelativeScale3D(FVector(0.65));Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);Mesh->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
    Label=CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));Label->SetupAttachment(Mesh);
    Label->SetRelativeLocation(FVector(0,0,85));Label->SetRelativeRotation(FRotator(0,180,0));Label->SetWorldSize(22);
}
void APFResourceNode::BeginPlay()
{
    Super::BeginPlay();
    if(!Catalog){Catalog=LoadObject<UPFCraftingCatalog>(nullptr,TEXT("/Game/PrimalFrontier/Crafting/DA_CraftingCatalog.DA_CraftingCatalog"));}
    if(!Items){Items=LoadObject<UPFItemCatalog>(nullptr,TEXT("/Game/PrimalFrontier/Items/DA_ItemCatalog.DA_ItemCatalog"));}
    if(HasAuthority()){const auto* D=Catalog?Catalog->Resource(ResourceId,Items):nullptr;HitsRemaining=D?D->Hits:0;}
}
bool APFResourceNode::IsConfigurationValid() const {return Catalog && Catalog->Resource(ResourceId,Items)!=nullptr;}
void APFResourceNode::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(APFResourceNode,ResourceId);DOREPLIFETIME(APFResourceNode,HitsRemaining);DOREPLIFETIME(APFResourceNode,RespawnAt);}
void APFResourceNode::Tick(float Delta)
{
    Super::Tick(Delta);const auto* D=Catalog?Catalog->Resource(ResourceId,Items):nullptr;if(!D){return;}
    const double Now=UPFInventoryComponent::ServerTime(GetWorld());
    if(HasAuthority() && RespawnAt>0 && Now>=RespawnAt)
    {HitsRemaining=D->Hits;RespawnAt=0;ForceNetUpdate();UE_LOG(LogPFSurvival,Display,TEXT("[PrimalGathering] Respawned %s"),*GetName());}
    if(GetNetMode()!=NM_DedicatedServer)
    {
        Mesh->SetVisibility(HitsRemaining>0,false);
        Label->SetText(FText::FromString(HitsRemaining>0 ? FString::Printf(TEXT("E: %s (%d hits)"),*D->DisplayName.ToString(),HitsRemaining) :
            FString::Printf(TEXT("Depleted - %ds"),FMath::Max(0,FMath::CeilToInt(RespawnAt-Now)))));
    }
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
    FVector Eye;FRotator Look;Pawn->GetActorEyesViewPoint(Eye,Look);FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PFGather),false,Pawn);
    if(!GetWorld()->LineTraceSingleByChannel(Hit,Eye,Eye+Look.Vector()*250,ECC_Visibility,Params) || Hit.GetActor()!=this){return false;}
    I->PruneExpired();const int32 Damage=FMath::Min(HitsRemaining,I->Count(TEXT("Item_Tool"))>0?2:1);
    if(!I->Grant(D->YieldItem,D->YieldPerHit*Damage)){return false;}
    HitsRemaining-=Damage;NextHitAt=Now+0.5;
    if(HitsRemaining==0){RespawnAt=Now+D->RespawnSeconds;}
    ForceNetUpdate();UE_LOG(LogPFSurvival,Display,TEXT("[PrimalGathering] %s yielded %d %s hits=%d pawn=%s"),*GetName(),D->YieldPerHit*Damage,*D->YieldItem.ToString(),HitsRemaining,*Pawn->GetName());return true;
}
