#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalHUD.h"
#include "Survival/PFRecoveryPickup.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemPickup.h"
#include "Inventory/PFInventoryHUD.h"
#include "GameFramework/PlayerState.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFResourceNode.h"
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildingHUD.h"
#include "Building/PFBuildPiece.h"

APFSurvivalPlayerController::APFSurvivalPlayerController() { SurvivalHUDClass = UPFSurvivalHUD::StaticClass();Building=CreateDefaultSubobject<UPFBuildingComponent>(TEXT("Building")); }
void APFSurvivalPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (IsLocalController() && GetLocalPlayer() && SurvivalHUDClass)
    {
        SurvivalHUD = CreateWidget<UPFSurvivalHUD>(this, SurvivalHUDClass);
        if (SurvivalHUD) { SurvivalHUD->AddToPlayerScreen(); }
        InventoryHUD=CreateWidget<UPFInventoryHUD>(this,UPFInventoryHUD::StaticClass());
        if(InventoryHUD){InventoryHUD->AddToPlayerScreen();}
        CraftingHUD=CreateWidget<UPFCraftingHUD>(this,UPFCraftingHUD::StaticClass());
        if(CraftingHUD){CraftingHUD->AddToPlayerScreen();}
        BuildingHUD=CreateWidget<UPFBuildingHUD>(this,UPFBuildingHUD::StaticClass());if(BuildingHUD){BuildingHUD->AddToPlayerScreen();}
    }
}
void APFSurvivalPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::E,IE_Pressed,this,&APFSurvivalPlayerController::Interact);
    InputComponent->BindKey(EKeys::Tab,IE_Pressed,this,&APFSurvivalPlayerController::ToggleInventory);
    InputComponent->BindKey(EKeys::Down,IE_Pressed,this,&APFSurvivalPlayerController::InventoryNext);
    InputComponent->BindKey(EKeys::Up,IE_Pressed,this,&APFSurvivalPlayerController::InventoryPrevious);
    InputComponent->BindKey(EKeys::X,IE_Pressed,this,&APFSurvivalPlayerController::InventorySplit);
    InputComponent->BindKey(EKeys::G,IE_Pressed,this,&APFSurvivalPlayerController::InventoryDrop);
    InputComponent->BindKey(EKeys::Q,IE_Pressed,this,&APFSurvivalPlayerController::InventoryConsume);
    InputComponent->BindKey(EKeys::C,IE_Pressed,this,&APFSurvivalPlayerController::ToggleCrafting);
    InputComponent->BindKey(EKeys::One,IE_Pressed,this,&APFSurvivalPlayerController::CraftTool);
    InputComponent->BindKey(EKeys::Two,IE_Pressed,this,&APFSurvivalPlayerController::CookFood);
    InputComponent->BindKey(EKeys::Three,IE_Pressed,this,&APFSurvivalPlayerController::DryFood);
    InputComponent->BindKey(EKeys::R,IE_Pressed,this,&APFSurvivalPlayerController::CancelCraft);
    InputComponent->BindKey(EKeys::B,IE_Pressed,this,&APFSurvivalPlayerController::ToggleBuilding);
    InputComponent->BindKey(EKeys::N,IE_Pressed,this,&APFSurvivalPlayerController::NextBuilding);
    InputComponent->BindKey(EKeys::T,IE_Pressed,this,&APFSurvivalPlayerController::RotateBuilding);
    InputComponent->BindKey(EKeys::LeftMouseButton,IE_Pressed,this,&APFSurvivalPlayerController::PlaceBuilding);
    InputComponent->BindKey(EKeys::H,IE_Pressed,this,&APFSurvivalPlayerController::DemolishBuilding);
    InputComponent->BindKey(EKeys::J,IE_Pressed,this,&APFSurvivalPlayerController::DamageBuilding);
    InputComponent->BindKey(EKeys::U,IE_Pressed,this,&APFSurvivalPlayerController::StoreItem);
    InputComponent->BindKey(EKeys::O,IE_Pressed,this,&APFSurvivalPlayerController::TakeStoredItem);
}
void APFSurvivalPlayerController::Interact() { if (IsLocalController()) { if(Building->bBuildMode){Building->ServerTargetAction(1);}else{ServerInteract();} } }
void APFSurvivalPlayerController::ServerInteract_Implementation()
{
    if (!HasAuthority() || !GetPawn()) { return; }
    const double Now = GetWorld()->GetTimeSeconds();
    if (Now < NextInteractionTime) { return; }
    NextInteractionTime = Now + 0.25;
    const auto* S = GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
    if (!S || S->IsDead()) { return; }
    FVector Eye; FRotator Look; GetPawn()->GetActorEyesViewPoint(Eye,Look);
    FHitResult Hit; FCollisionQueryParams Params(SCENE_QUERY_STAT(PFInteraction),false,GetPawn());
    if (GetWorld()->LineTraceSingleByChannel(Hit,Eye,Eye + Look.Vector()*250.f,ECC_Visibility,Params))
    {
        if(auto* Item=Cast<APFItemPickup>(Hit.GetActor()))
        {ClientInventoryFeedback(Item->TryPickup(GetPawn()) ? TEXT("Picked up") : TEXT("Pickup refused: full, expired or invalid"));}
        else if (auto* Pickup = Cast<APFRecoveryPickup>(Hit.GetActor())) { Pickup->TryConsume(GetPawn()); }
        else if(auto* Node=Cast<APFResourceNode>(Hit.GetActor()))
        {ClientInventoryFeedback(Node->Gather(GetPawn())?TEXT("Gathered"):TEXT("Gather refused: depleted, cooldown or full inventory"));}
    }
}
UPFInventoryComponent* APFSurvivalPlayerController::GetInventory() const
{return PlayerState ? PlayerState->FindComponentByClass<UPFInventoryComponent>() : nullptr;}
void APFSurvivalPlayerController::ToggleInventory(){bInventoryOpen=!bInventoryOpen;}
void APFSurvivalPlayerController::InventoryNext()
{if(bInventoryOpen){if(const auto* I=GetInventory()){SelectedInventoryIndex=FMath::Min(SelectedInventoryIndex+1,I->GetStacks().Num()-1);}}}
void APFSurvivalPlayerController::InventoryPrevious(){if(bInventoryOpen){SelectedInventoryIndex=FMath::Max(0,SelectedInventoryIndex-1);}}
void APFSurvivalPlayerController::InventorySplit(){SendInventoryAction(0);}
void APFSurvivalPlayerController::InventoryDrop(){SendInventoryAction(1);}
void APFSurvivalPlayerController::InventoryConsume(){SendInventoryAction(2);}
void APFSurvivalPlayerController::SendInventoryAction(uint8 Action)
{
    const auto* I=GetInventory(); if(!bInventoryOpen || !I || I->GetStacks().IsEmpty()){return;}
    SelectedInventoryIndex=FMath::Clamp(SelectedInventoryIndex,0,I->GetStacks().Num()-1);
    const auto S=I->GetStacks()[SelectedInventoryIndex];
    ServerInventoryAction(S.StackId,Action,Action==0 ? S.Quantity/2 : 1);
}
void APFSurvivalPlayerController::ServerInventoryAction_Implementation(FGuid StackId,uint8 Action,int32 Quantity)
{
    const double Now=GetWorld()->GetTimeSeconds(); if(Now<NextInventoryTime){return;} NextInventoryTime=Now+0.15;
    auto* I=GetInventory(); const auto* S=GetPawn() ? GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>() : nullptr;
    bool Accepted=false;
    if(HasAuthority() && I && S && !S->IsDead() && Quantity>0 && Quantity<=1000)
    {
        if(Action==0){Accepted=I->Split(StackId,Quantity);}
        else if(Action==1){Accepted=I->Drop(StackId,Quantity,GetPawn())!=nullptr;}
        else if(Action==2 && Quantity==1){Accepted=I->Consume(StackId,GetPawn());}
    }
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalInventory] Request action=%d quantity=%d accepted=%d owner=%s"),Action,Quantity,Accepted,*GetName());
    ClientInventoryFeedback(Accepted ? TEXT("Done") : TEXT("Refused: invalid stack, quantity, space or life state"));
}
void APFSurvivalPlayerController::ClientInventoryFeedback_Implementation(const FString& Message){InventoryMessage=Message;}
UPFCraftingComponent* APFSurvivalPlayerController::GetCrafting() const
{return PlayerState?PlayerState->FindComponentByClass<UPFCraftingComponent>():nullptr;}
void APFSurvivalPlayerController::ToggleCrafting(){bCraftingOpen=!bCraftingOpen;if(bCraftingOpen){Building->bBuildMode=false;}}
void APFSurvivalPlayerController::CraftTool(){if(bCraftingOpen){ServerCraftAction(TEXT("Recipe_Tool"),false);}}
void APFSurvivalPlayerController::CookFood(){if(bCraftingOpen){ServerCraftAction(TEXT("Recipe_Cook"),false);}}
void APFSurvivalPlayerController::DryFood(){if(bCraftingOpen){ServerCraftAction(TEXT("Recipe_Dry"),false);}}
void APFSurvivalPlayerController::CancelCraft(){if(bCraftingOpen){ServerCraftAction(NAME_None,true);}}
void APFSurvivalPlayerController::ServerCraftAction_Implementation(FName Id,bool bCancel)
{
    if(!HasAuthority()){return;}const double Now=GetWorld()->GetTimeSeconds();if(Now<NextCraftTime){return;}NextCraftTime=Now+0.25;
    auto* C=GetCrafting();const bool Accepted=C && (bCancel?C->Cancel():C->Start(Id,GetPawn()));
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalCrafting] Request recipe=%s cancel=%d accepted=%d owner=%s"),*Id.ToString(),bCancel,Accepted,*GetName());
    ClientInventoryFeedback(Accepted?TEXT("Craft request accepted"):TEXT("Craft refused: busy, invalid recipe, ingredients or life state"));
}
void APFSurvivalPlayerController::ToggleBuilding(){Building->bBuildMode=!Building->bBuildMode;if(Building->bBuildMode){bCraftingOpen=false;bInventoryOpen=false;}}
void APFSurvivalPlayerController::NextBuilding(){if(Building->bBuildMode && Building->Catalog && Building->Catalog->Pieces.Num()>0){Building->Selection=(Building->Selection+1)%Building->Catalog->Pieces.Num();}}
void APFSurvivalPlayerController::RotateBuilding(){if(Building->bBuildMode){Building->Rotation=(Building->Rotation+1)%4;}}
void APFSurvivalPlayerController::PlaceBuilding(){if(Building->bBuildMode){Building->ServerPlace(Building->SelectedId(),Building->Rotation);}}
void APFSurvivalPlayerController::DemolishBuilding(){if(Building->bBuildMode){Building->ServerTargetAction(0);}}
void APFSurvivalPlayerController::DamageBuilding(){if(Building->bBuildMode){Building->ServerTargetAction(2);}}
void APFSurvivalPlayerController::StoreItem(){auto* I=GetInventory();if(Building->bBuildMode && I && I->GetStacks().IsValidIndex(SelectedInventoryIndex)){Building->ServerTransfer(true,I->GetStacks()[SelectedInventoryIndex].StackId,1);}}
void APFSurvivalPlayerController::TakeStoredItem(){auto* P=Building->OpenStorage.Get();if(Building->bBuildMode && IsValid(P) && !P->Storage->GetStacks().IsEmpty()){Building->ServerTransfer(false,P->Storage->GetStacks()[0].StackId,1);}}
