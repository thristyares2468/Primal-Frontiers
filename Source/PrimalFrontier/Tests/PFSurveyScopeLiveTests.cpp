#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingStation.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFSaveFileStore.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Settings/PFGameUserSettings.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
namespace
{
class FScopeLive final : public IAutomationLatentCommand
{
public:
    explicit FScopeLive(FAutomationTestBase* T):Test(T){FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot);Restart=FParse::Param(FCommandLine::Get(),TEXT("PFLoadSave"));}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Stage==99){return Now-Changed>(Client?3:12);}
        if(Now-Started>130){Test->AddError(FString::Printf(TEXT("Scope live timeout stage%d client%d restart%d"),Stage,Client,Restart));return true;}
        UWorld* W=nullptr;for(const auto& X:GEngine->GetWorldContexts()){if(X.World() && X.World()->IsGameWorld()){W=X.World();break;}}if(!W){return false;}Client=W->GetNetMode()==NM_Client;
        TArray<APFSurvivalPlayerController*> Players;for(auto It=W->GetPlayerControllerIterator();It;++It){auto* P=Cast<APFSurvivalPlayerController>(It->Get());if(P && P->GetPawn() && P->GetInventory() && P->GetCrafting()){Players.Add(P);}}
        if(Players.Num()!=(Client?1:Expected)){return false;}auto* PC=Players[0];auto* I=PC->GetInventory();auto* C=PC->GetCrafting();auto* G=PC->GetPlayerState<APFInventoryPlayerState>()->Progression.Get();
        const FName Item=TEXT("Item_SurveyScope"),Recipe=TEXT("Recipe_SurveyScope");
        if(!Client)
        {
            if(!Restart && Stage==0)
            {
                int32 N=0;for(auto* P:Players)
                {
                    auto* Bag=P->GetInventory();auto* Needs=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();Needs->HungerDrainPerSecond=0;Needs->ThirstDrainPerSecond=0;
                    P->GetPawn()->SetActorLocation(FVector(-400,600+650*N++,100));
                    FPFProgressionRecord Seed;Seed.Experience=250;Seed.Knowledge={TEXT("Tech_FieldTools"),TEXT("Tech_Metalwork"),TEXT("Tech_Gemwork")};FString Error;
                    if(!Test->TestTrue(TEXT("Fresh isolated material/knowledge fixture"),Bag->GetStacks().IsEmpty() && P->GetPlayerState<APFInventoryPlayerState>()->Progression->Restore(Seed,Error) &&
                        Bag->Grant(TEXT("Item_QuartzLens"),1) && Bag->Grant(TEXT("Item_CopperIngot"),2) && Bag->Grant(TEXT("Item_Cord"),1) && Bag->Grant(TEXT("Item_Wood"),3))){return true;}
                    W->SpawnActor<APFCraftingStation>(P->GetPawn()->GetActorLocation()+FVector(0,140,-100),FRotator::ZeroRotator);
                }
                Next(Now);return false;
            }
            for(auto* P:Players){if(P->GetInventory()->Count(Item)!=1 || !P->GetCrafting()->Feedback.Contains(TEXT("unknown or invalid recipe"))){return false;}}
            for(auto* P:Players)
            {
                auto* Bag=P->GetInventory();const auto& R=P->GetPlayerState<APFInventoryPlayerState>()->Progression->GetRecord();
                Test->TestTrue(TEXT("Server exact scope output and costs"),Bag->Count(TEXT("Item_Wood"))==1 && Bag->Count(TEXT("Item_QuartzLens"))==0 && Bag->Count(TEXT("Item_CopperIngot"))==0 && Bag->Count(TEXT("Item_Cord"))==0);
                Test->TestTrue(TEXT("Server scope record once only"),R.Experience==270 && R.CreditedCrafts.Contains(Recipe) && R.Knowledge.Contains(TEXT("Tech_Gemwork")));
                Test->TestEqual(TEXT("Local zoom never becomes server authority"),P->GetSurveyMagnification(),0.f);
            }
            if(!Restart){FString Error;if(!Test->TestTrue(TEXT("Actual server scope save"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error))){Test->AddError(Error);return true;}}
            Test->AddInfo(Restart?TEXT("Scope restart server restored owners, costs and once-only records"):TEXT("Scope server real crafting, exact costs and private save verified"));Stage=99;Changed=Now;return false;
        }
        if(PC->GetPlayerSetupStatus()==EPFPlayerSetupStatus::Unconfirmed || Now-Changed<.7){return false;}
        if(Stage==0)
        {
            if(Restart)
            {
                if(PC->GetPlayerSetupStatus()!=EPFPlayerSetupStatus::Restored || I->Count(Item)!=1){return false;}
                Test->TestTrue(TEXT("New client session never inherits hotbar or zoom"),!PC->GetHotbar().Identity(0).IsValid() && PC->GetSurveyMagnification()==0);
                Stage=4;Changed=Now;return false;
            }
            if(I->Count(TEXT("Item_QuartzLens"))!=1 || !G->GetRecord().Knowledge.Contains(TEXT("Tech_Gemwork"))){return false;}
            Test->TestFalse(TEXT("Client direct assembly cannot create item"),C->Start(Recipe,PC->GetPawn()));Test->TestFalse(TEXT("Client cannot grant scope"),I->Grant(Item,1));
            PC->ServerCraftAction(Recipe,false);Next(Now);return false;
        }
        if(Stage==1){if(C->ActiveRecipe!=Recipe){return false;}Test->TestEqual(TEXT("Workbench snapshot replicated"),C->ActiveStationMultiplier,1.25f);PC->ServerCraftAction(NAME_None,true);Next(Now);return false;}
        if(Stage==2){if(!C->ActiveRecipe.IsNone() || !C->Feedback.StartsWith(TEXT("Cancelled"))){return false;}Test->TestTrue(TEXT("Cancel keeps lens and creates no scope"),I->Count(Item)==0 && I->Count(TEXT("Item_QuartzLens"))==1);PC->ServerCraftAction(Recipe,false);Next(Now);return false;}
        if(Stage==3){if(!C->ActiveRecipe.IsNone() || I->Count(Item)!=1){return false;}Next(Now);return false;}
        if(Stage==4)
        {
            if(G->GetRecord().Experience!=270 || I->Count(TEXT("Item_Wood"))!=1){return false;}
            const auto* S=I->GetStacks().FindByPredicate([&](const auto& X){return X.ItemId==Item;});if(!S){return false;}
            PC->SetInventoryOpen(true);if(!Test->TestTrue(TEXT("Bind real replicated scope batch"),PC->SelectInventoryStack(S->StackId) && PC->PinInventoryToHotbar(0))){return true;}PC->SetInventoryOpen(false);PC->SelectHotbar(0);PC->UseHotbar();Next(Now);return false;
        }
        if(Stage==5)
        {
            int32 Others=0;for(TActorIterator<APFInventoryPlayerState> It(W);It;++It){if(*It!=PC->PlayerState && !It->IsActorBeingDestroyed()){++Others;Test->TestTrue(TEXT("Foreign scope inventory and knowledge private"),It->Inventory->GetStacks().IsEmpty() && It->Progression->GetRecord().Knowledge.IsEmpty());}}
            if(Others!=Expected-1){return false;}
            const auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());const float Base=UPFGameUserSettings::Get()->Preferences.FieldOfView;
            Test->TestEqual(TEXT("Owner-only optical view2x"),PC->GetSurveyMagnification(),2.f);
            Test->TestTrue(TEXT("Client actual camera narrowed"),P->GetFirstPersonCameraComponent()->FieldOfView<Base);
            PC->CancelSurveyView();Test->TestEqual(TEXT("Normal client camera restored"),P->GetFirstPersonCameraComponent()->FieldOfView,Base);
            FString Error;Test->TestFalse(TEXT("Client cannot save scopes"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error));
            // Existing invalid recipe RPC is a harmless server-observed completion marker.
            PC->ServerCraftAction(TEXT("Recipe_ScopeUnknown"),false);Next(Now);return false;
        }
        if(Stage==6)
        {
            if(!C->Feedback.Contains(TEXT("unknown or invalid recipe"))){return false;}
            Test->AddInfo(Restart?TEXT("Scope restart client original identity, inventory, ledger and optical reset verified"):TEXT("Scope client owned craft/cancel, actual local camera, privacy and authority refusal verified"));Stage=99;Changed=Now;return false;
        }
        return false;
    }
private:
    void Next(double Now){++Stage;Changed=Now;}
    FAutomationTestBase* Test;FString Slot;int32 Expected=0,Stage=0;bool Restart=false,Client=false;double Started=FPlatformTime::Seconds(),Changed=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFScopeLiveTest,"PF.Optics.Network",EAutomationTestFlags::ClientContext|EAutomationTestFlags::ServerContext|EAutomationTestFlags::EngineFilter)
bool FPFScopeLiveTest::RunTest(const FString&)
{
    FString Slot;int32 Players=0;FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Players);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunSurveyScopeTests")) || (Players!=1 && Players!=2) || !FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot) || !Slot.StartsWith(TEXT("AutomationM12Scope")) || !FPFSaveFileStore::ValidSlot(Slot)){AddError(TEXT("Requires isolated scope network runner/owned slot"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FScopeLive(this));return true;
}
#endif
