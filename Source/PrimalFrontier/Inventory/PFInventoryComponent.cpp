// PFInventoryComponent.cpp
//
// See PFInventoryComponent.h for the guarantees. Implementation pattern for
// multi-step changes (AddExisting, Transform): copy Stacks into "Proposed",
// apply every change to the copy, validate limits, and only then commit with
// Stacks = MoveTemp(Proposed). A failure anywhere leaves the real inventory untouched.

#include "Inventory/PFInventoryComponent.h"
#include "PFAssetPaths.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFItemPickup.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

UPFInventoryComponent::UPFInventoryComponent()
{
    SetIsReplicatedByDefault(true);
    // 1 Hz is enough to remove spoiled batches; mutations also prune first.
    PrimaryComponentTick.bCanEverTick=true;
    PrimaryComponentTick.TickInterval=1;
}

void UPFInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    if (!Catalog) { Catalog=LoadObject<UPFItemCatalog>(nullptr,PFAssetPaths::ItemCatalog); }
    SetComponentTickEnabled(Authority());
}

void UPFInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME_CONDITION(UPFInventoryComponent,Stacks,COND_OwnerOnly);
}

bool UPFInventoryComponent::Authority() const { return GetOwner() && GetOwner()->HasAuthority(); }

double UPFInventoryComponent::ServerTime(const UWorld* World)
{
    // GameState's server world time is synchronized to clients, so countdowns agree everywhere.
    const auto* State=World ? World->GetGameState() : nullptr;
    return State ? State->GetServerWorldTimeSeconds() : (World ? World->GetTimeSeconds() : 0);
}

const FPFItemDefinition* UPFInventoryComponent::Definition(FName Id) const { return Catalog ? Catalog->Find(Id) : nullptr; }

float UPFInventoryComponent::GetWeight() const
{
    double Total=0;
    for(const auto& S:Stacks){ if(const auto* D=Definition(S.ItemId)){ Total+=D->Weight*S.Quantity; } }
    return float(Total);
}

int32 UPFInventoryComponent::Count(FName Id) const
{
    int32 Total=0;
    for(const auto& S:Stacks){ if(S.ItemId==Id){ Total+=S.Quantity; } }
    return Total;
}

void UPFInventoryComponent::Changed(const TCHAR* Action)
{
    GetOwner()->ForceNetUpdate();
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalInventory] %s %s slots=%d weight=%.2f"),*GetOwner()->GetName(),Action,Stacks.Num(),GetWeight());
}

void UPFInventoryComponent::PruneExpired()
{
    if(!Authority()){ return; }
    const double Now=ServerTime(GetWorld());
    if(Stacks.RemoveAll([Now](const auto& S){ return S.ExpiresAt>0 && S.ExpiresAt<=Now; })>0){ Changed(TEXT("expired")); }
}

void UPFInventoryComponent::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);
    PruneExpired();
}

bool UPFInventoryComponent::Grant(FName Id,int32 Quantity)
{
    if(!Authority()){ return false; }
    const auto* D=Definition(Id);
    if(!D){ return false; }
    // New items: perishables start a fresh shelf-life clock now; others get 0 (never spoil).
    return AddExisting(Id,Quantity,D->ShelfLifeSeconds>0 ? ServerTime(GetWorld())+D->ShelfLifeSeconds : 0);
}

bool UPFInventoryComponent::AddExisting(FName Id,int32 Quantity,double Deadline)
{
    if(!Authority() || Quantity<=0 || Quantity>10000 || !FMath::IsFinite(Deadline)){ return false; }
    const auto* D=Definition(Id);
    // Perishables need a future deadline; nonperishables must have none.
    if(!D || (D->ShelfLifeSeconds>0 ? Deadline<=ServerTime(GetWorld()) : Deadline!=0)){ return false; }
    PruneExpired();
    if(SlotLimit<1 || SlotLimit>64 || !FMath::IsFinite(WeightLimit) || WeightLimit<=0 || double(GetWeight())+double(D->Weight)*Quantity>WeightLimit+0.0001){ return false; }

    // Work on a copy so a capacity failure part-way through changes nothing.
    auto Proposed=Stacks;
    int32 Remaining=Quantity;
    // 1) Top up existing stacks of the same item AND the same freshness batch.
    for(auto& S:Proposed)
    {
        if(S.ItemId!=Id || S.ExpiresAt!=Deadline){ continue; }
        // Clamp at 0: a stack above a since-lowered StackLimit must not go negative
        // (that would shrink the stack and inflate Remaining, creating items).
        const int32 Added=FMath::Min(Remaining,FMath::Max(0,D->StackLimit-S.Quantity));
        S.Quantity+=Added;
        Remaining-=Added;
    }
    // 2) Open new slots for the rest.
    while(Remaining>0 && Proposed.Num()<SlotLimit)
    {
        FPFItemStack S;
        S.StackId=FGuid::NewGuid();
        S.ItemId=Id;
        S.Quantity=FMath::Min(Remaining,D->StackLimit);
        S.ExpiresAt=Deadline;
        Proposed.Add(S);
        Remaining-=S.Quantity;
    }
    if(Remaining>0){ return false; }  // out of slots: reject the whole insert
    Stacks=MoveTemp(Proposed);
    Changed(TEXT("added"));
    return true;
}

bool UPFInventoryComponent::Remove(FGuid Id,int32 Quantity)
{
    if(!Authority() || Quantity<=0){ return false; }
    PruneExpired();
    const int32 Index=Stacks.IndexOfByPredicate([Id](const auto& S){return S.StackId==Id;});
    if(Index==INDEX_NONE || Quantity>Stacks[Index].Quantity){return false;}
    Stacks[Index].Quantity-=Quantity;
    if(Stacks[Index].Quantity==0){Stacks.RemoveAt(Index);}
    Changed(TEXT("removed"));
    return true;
}

bool UPFInventoryComponent::RemoveItem(FName Id,int32 Quantity)
{
    if(!Authority() || Quantity<=0){return false;}
    PruneExpired();
    if(Count(Id)<Quantity){return false;}  // check the total first so removal can't be partial
    for(int32 I=Stacks.Num()-1;I>=0 && Quantity>0;--I)
    {
        if(Stacks[I].ItemId==Id)
        {
            const int32 N=FMath::Min(Quantity,Stacks[I].Quantity);
            Stacks[I].Quantity-=N;
            Quantity-=N;
            if(Stacks[I].Quantity==0){Stacks.RemoveAt(I);}
        }
    }
    Changed(TEXT("removed-item"));
    return true;
}

bool UPFInventoryComponent::Split(FGuid Id,int32 Quantity)
{
    if(!Authority() || Quantity<=0){ return false; }
    PruneExpired();
    const int32 Index=Stacks.IndexOfByPredicate([Id](const auto& S){return S.StackId==Id;});
    // Must leave at least one item behind and have a free slot.
    if(Index==INDEX_NONE || Quantity>=Stacks[Index].Quantity || Stacks.Num()>=SlotLimit){return false;}
    FPFItemStack Copy=Stacks[Index];  // keeps ItemId and the exact ExpiresAt
    Copy.StackId=FGuid::NewGuid();
    Copy.Quantity=Quantity;
    Stacks[Index].Quantity-=Quantity;
    Stacks.Add(Copy);
    Changed(TEXT("split"));
    return true;
}

bool UPFInventoryComponent::OwnsLivingPawn(APawn* Pawn) const
{
    const auto* S=Pawn ? Pawn->FindComponentByClass<UPFPlayerSurvivalComponent>() : nullptr;
    return Authority() && IsValid(Pawn) && Pawn->HasAuthority() && Pawn->GetPlayerState()==GetOwner() && Pawn->GetController() && S && !S->IsDead();
}

bool UPFInventoryComponent::TransferTo(UPFInventoryComponent* Destination,FGuid Id,int32 Quantity)
{
    if(!Authority() || !IsValid(Destination) || Destination==this || !Destination->Authority() || Destination->GetWorld()!=GetWorld() || Quantity<1){return false;}
    PruneExpired();
    const int32 Index=Stacks.IndexOfByPredicate([Id](const auto& S){return S.StackId==Id;});
    if(Index==INDEX_NONE || Stacks[Index].Quantity<Quantity){return false;}
    const FPFItemStack Source=Stacks[Index];
    // No callbacks or world ticks between destination acceptance and the source commit.
    if(!Destination->AddExisting(Source.ItemId,Quantity,Source.ExpiresAt)){return false;}
    Stacks[Index].Quantity-=Quantity;
    if(Stacks[Index].Quantity==0){Stacks.RemoveAt(Index);}
    Changed(TEXT("transferred"));
    return true;
}

bool UPFInventoryComponent::Transform(const TArray<FPFItemStack>& Inputs,FName Output,int32 Quantity)
{
    if(!Authority() || Inputs.IsEmpty() || Inputs.Num()>64 || Quantity<1 || Quantity>10000){return false;}
    const auto* D=Definition(Output);
    if(!D){return false;}
    PruneExpired();
    auto Proposed=Stacks;
    TSet<FGuid> Seen;

    // 1) Consume each input from the copy. Every input must still match the stack that was
    //    reserved when crafting started: same GUID, item and deadline, with enough quantity.
    for(const auto& Input:Inputs)
    {
        const int32 Index=Proposed.IndexOfByPredicate([&](const auto& S){return S.StackId==Input.StackId;});
        if(Seen.Contains(Input.StackId) || Index==INDEX_NONE || Input.Quantity<=0 || Proposed[Index].Quantity<Input.Quantity ||
            Proposed[Index].ItemId!=Input.ItemId || Proposed[Index].ExpiresAt!=Input.ExpiresAt){return false;}
        Seen.Add(Input.StackId);
        Proposed[Index].Quantity-=Input.Quantity;
        if(Proposed[Index].Quantity==0){Proposed.RemoveAt(Index);}
    }

    // 2) Weight check of the result (remaining stacks + output).
    double Weight=double(D->Weight)*Quantity;
    for(const auto& S:Proposed)
    {
        const auto* Item=Definition(S.ItemId);
        if(!Item){return false;}
        Weight+=double(Item->Weight)*S.Quantity;
    }
    if(SlotLimit<1 || SlotLimit>64 || !FMath::IsFinite(WeightLimit) || WeightLimit<=0 || Weight>WeightLimit+0.0001){return false;}

    // 3) Insert the output with a fresh deadline (crafted food starts its own shelf life).
    const double Deadline=D->ShelfLifeSeconds>0 ? ServerTime(GetWorld())+D->ShelfLifeSeconds : 0;
    int32 Left=Quantity;
    for(auto& S:Proposed)
    {
        if(S.ItemId==Output && S.ExpiresAt==Deadline)
        {
            const int32 N=FMath::Min(Left,FMath::Max(0,D->StackLimit-S.Quantity));  // see AddExisting
            S.Quantity+=N;
            Left-=N;
        }
    }
    while(Left>0 && Proposed.Num()<SlotLimit)
    {
        FPFItemStack S;
        S.StackId=FGuid::NewGuid();
        S.ItemId=Output;
        S.Quantity=FMath::Min(Left,D->StackLimit);
        S.ExpiresAt=Deadline;
        Proposed.Add(S);
        Left-=S.Quantity;
    }
    if(Left>0){return false;}

    // 4) Commit everything at once.
    Stacks=MoveTemp(Proposed);
    Changed(TEXT("crafted"));
    return true;
}

bool UPFInventoryComponent::Consume(FGuid Id,APawn* Pawn)
{
    if(!OwnsLivingPawn(Pawn)){return false;}
    PruneExpired();
    const auto* Stack=Stacks.FindByPredicate([Id](const auto& S){return S.StackId==Id;});
    if(!Stack){return false;}
    const auto* D=Definition(Stack->ItemId);
    if(!D || D->FoodRecovery+D->WaterRecovery<=0){return false;}  // not edible
    auto* S=Pawn->FindComponentByClass<UPFPlayerSurvivalComponent>();
    // Refuse to waste food that would restore nothing.
    if((D->FoodRecovery==0 || S->GetVitals().Hunger>=100) && (D->WaterRecovery==0 || S->GetVitals().Thirst>=100)){return false;}
    if(!S->RecoverNeeds(D->FoodRecovery,D->WaterRecovery)){return false;}
    return Remove(Id,1);
}

APFItemPickup* UPFInventoryComponent::Drop(FGuid Id,int32 Quantity,APawn* Pawn)
{
    if(!OwnsLivingPawn(Pawn) || Quantity<=0){return nullptr;}
    PruneExpired();
    const auto* S=Stacks.FindByPredicate([Id](const auto& Entry){return Entry.StackId==Id;});
    if(!S || Quantity>S->Quantity){return nullptr;}
    // Drop point 1.2 m in front of the eyes; refuse if anything is in the way (no dropping through walls).
    FVector Eye; FRotator Look; Pawn->GetActorEyesViewPoint(Eye,Look);
    const FVector Location=Eye+Look.Vector()*120;
    FHitResult Hit; FCollisionQueryParams Params(SCENE_QUERY_STAT(PFDrop),false,Pawn);
    if(GetWorld()->SweepSingleByChannel(Hit,Eye,Location,FQuat::Identity,ECC_Visibility,FCollisionShape::MakeSphere(22),Params)){return nullptr;}
    const FPFItemStack Source=*S;
    const FTransform Transform(FRotator::ZeroRotator,Location);
    // Spawn the pickup FIRST (carrying the original deadline); only remove items once it exists.
    auto* Pickup=GetWorld()->SpawnActorDeferred<APFItemPickup>(APFItemPickup::StaticClass(),Transform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if(!Pickup){return nullptr;}
    Pickup->Initialize(Source.ItemId,Quantity,Source.ExpiresAt);
    Pickup->FinishSpawning(Transform);
    if(!IsValid(Pickup) || !Remove(Id,Quantity))
    {
        if(IsValid(Pickup)){Pickup->Destroy();}
        return nullptr;
    }
    Changed(TEXT("dropped"));
    return Pickup;
}
