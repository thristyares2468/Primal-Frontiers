#include "Inventory/PFItemPickup.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
APFItemPickup::APFItemPickup()
{
    bReplicates=true; PrimaryActorTick.bCanEverTick=true; PrimaryActorTick.TickInterval=0.5;
    auto* Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh")); SetRootComponent(Mesh);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube")); Mesh->SetStaticMesh(Cube.Object);
    Mesh->SetRelativeScale3D(FVector(0.35)); Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Mesh->SetCollisionResponseToAllChannels(ECR_Ignore); Mesh->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
    Label=CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label")); Label->SetupAttachment(Mesh);
    Label->SetRelativeLocation(FVector(0,0,90)); Label->SetRelativeRotation(FRotator(0,180,0)); Label->SetWorldSize(30);
}
void APFItemPickup::Initialize(FName Id,int32 Count,double Deadline)
{ if(HasAuthority() && !HasActorBegunPlay()){bInitialized=true; Contents.StackId=FGuid::NewGuid(); Contents.ItemId=Id; Contents.Quantity=Count; Contents.ExpiresAt=Deadline;} }
void APFItemPickup::BeginPlay()
{
    Super::BeginPlay();
    if(HasAuthority())
    {
        const auto* Catalog=LoadObject<UPFItemCatalog>(nullptr,TEXT("/Game/PrimalFrontier/Items/DA_ItemCatalog.DA_ItemCatalog"));
        const auto* D=Catalog ? Catalog->Find(bInitialized?Contents.ItemId:ItemId) : nullptr;
        if(!D){Destroy();return;}
        if(!bInitialized){ Contents.StackId=FGuid::NewGuid(); Contents.ItemId=ItemId; Contents.Quantity=Quantity; Contents.ExpiresAt=D->ShelfLifeSeconds>0 ? UPFInventoryComponent::ServerTime(GetWorld())+D->ShelfLifeSeconds : 0; }
        if(Contents.Quantity<1 || Contents.Quantity>D->StackLimit || !FMath::IsFinite(Contents.ExpiresAt) ||
            (D->ShelfLifeSeconds>0 ? Contents.ExpiresAt<=UPFInventoryComponent::ServerTime(GetWorld()) : Contents.ExpiresAt!=0)){ Destroy(); }
    }
}
void APFItemPickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{ Super::GetLifetimeReplicatedProps(OutLifetimeProps); DOREPLIFETIME(APFItemPickup,Contents); }
void APFItemPickup::Tick(float Delta)
{
    Super::Tick(Delta); const double Left=Contents.ExpiresAt-UPFInventoryComponent::ServerTime(GetWorld());
    if(HasAuthority() && Contents.ExpiresAt>0 && Left<=0){bTaken=true;Destroy();return;}
    if(GetNetMode()!=NM_DedicatedServer)
    {
        const FString Fresh=Contents.ExpiresAt>0 ? FString::Printf(TEXT(" (%ds fresh)"),FMath::Max(0,FMath::CeilToInt(Left))) : TEXT("");
        Label->SetText(FText::FromString(FString::Printf(TEXT("E: %s x%d%s"),*Contents.ItemId.ToString(),Contents.Quantity,*Fresh)));
    }
}
bool APFItemPickup::TryPickup(APawn* Pawn)
{
    if(!HasAuthority() || bTaken || !IsValid(Pawn) || !Pawn->HasAuthority() || Pawn->GetWorld()!=GetWorld() || !Pawn->GetController() ||
        FVector::DistSquared(Pawn->GetActorLocation(),GetActorLocation())>FMath::Square(250.f)){return false;}
    const auto* S=Pawn->FindComponentByClass<UPFPlayerSurvivalComponent>();
    auto* I=Pawn->GetPlayerState() ? Pawn->GetPlayerState()->FindComponentByClass<UPFInventoryComponent>() : nullptr;
    if(!S || S->IsDead() || !I){return false;}
    FVector Eye; FRotator Look; Pawn->GetActorEyesViewPoint(Eye,Look); FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PFPickup),false,Pawn);
    if(GetWorld()->LineTraceSingleByChannel(Hit,Eye,GetActorLocation(),ECC_Visibility,Params) && Hit.GetActor()!=this){return false;}
    if(!I->AddExisting(Contents.ItemId,Contents.Quantity,Contents.ExpiresAt)){return false;}
    bTaken=true; Destroy(); return true;
}
