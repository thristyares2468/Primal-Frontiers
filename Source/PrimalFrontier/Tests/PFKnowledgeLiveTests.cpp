// Disposable server/client request boundary, explicit trusted XP fixtures and actual restart.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Progression/PFProgressionComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Crafting/PFCraftingComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFSaveFileStore.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
namespace
{
class FKnowledgeLiveExercise final : public IAutomationLatentCommand
{
public:
    explicit FKnowledgeLiveExercise(FAutomationTestBase* T):Test(T)
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot);bRestore=FParse::Param(FCommandLine::Get(),TEXT("PFLoadSave"));bWeapon=FParse::Param(FCommandLine::Get(),TEXT("PFRunWeaponKnowledgeTests"));}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Stage==99){return Now-Changed>(bClient?3:15);}
        if(Now-Started>110){Test->AddError(FString::Printf(TEXT("[PrimalProgression] Knowledge live timeout stage=%d client=%d"),Stage,bClient));return true;}
        UWorld* W=nullptr;for(const auto& Context:GEngine->GetWorldContexts()){if(Context.World() && Context.World()->IsGameWorld()){W=Context.World();break;}}if(!W){return false;}
        bClient=W->GetNetMode()==NM_Client;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It){auto* PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC && PC->GetPawn() && PC->GetInventory() && Progression(PC)){Players.Add(PC);}}
        if(Players.Num()!=(bClient?1:Expected)){return false;}
        Players.Sort([](const auto& A,const auto& B){return A.PlayerState->GetPlayerId()<B.PlayerState->GetPlayerId();});
        auto* PC=Players[0];auto* G=Progression(PC);auto* I=PC->GetInventory();auto* C=PC->GetCrafting();if(!C){return false;}FString Error;
        auto Next=[&](int32 N){Stage=N;Changed=Now;};
        if(bRestore)
        {
            for(auto* P:Players){if(!Restored(P)){return false;}}
            if(bClient)
            {
                if(PC->GetPlayerSetupStatus()!=EPFPlayerSetupStatus::Restored || !Privacy(W,PC)){return false;}
                if(Stage==0)
                {
                    CheckState(PC);const FString Feedback=G->GetKnowledgeFeedback();
                    Test->TestFalse(TEXT("Restored client direct mutation refuses"),G->RequestKnowledge(bWeapon?TEXT("Tech_FieldWeapons"):TEXT("Tech_FieldTools"),PC->GetPawn(),Error));
                    Test->TestEqual(TEXT("Client cannot forge restored feedback"),G->GetKnowledgeFeedback(),Feedback);
                    Test->TestFalse(TEXT("Client cannot save knowledge"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error));
                    PC->ServerCraftAction(AccessRecipe(),false);Next(20);return false;
                }
                if(Stage==20 && Now-Changed>.7)
                {
                    if(Marker(I)==1)
                    {if(C->ActiveRecipe!=AccessRecipe()){return false;}PC->ServerCraftAction(NAME_None,true);Next(21);return false;}
                    if(!C->Feedback.StartsWith(TEXT("Locked:"))){return false;}Test->TestTrue(TEXT("Unlearned restored RPC stays idle"),C->ActiveRecipe.IsNone());
                }
                else if(Stage==21 && Now-Changed>.7)
                {if(!C->ActiveRecipe.IsNone() || !C->Feedback.StartsWith(TEXT("Cancelled"))){return false;}}
                else{return false;}
                CheckState(PC);Test->AddInfo(TEXT("[PrimalProgression] Knowledge restart client identity,spent points,private records and refusal verified"));
                Test->AddInfo(TEXT("[PrimalProgression] Restored learned/locked recipe RPC and cancel conserve inputs"));
            }
            else
            {
                for(auto* P:Players){CheckState(P);}FPFWorldSaveData Data;
                Test->TestTrue(TEXT("Actual restored world capture"),W->GetSubsystem<UPFWorldPersistence>()->Capture(Data,Error));
                Test->TestTrue(TEXT("Restored record validates"),W->GetSubsystem<UPFWorldPersistence>()->Validate(Data,Error));
                Test->TestEqual(TEXT("All original players restored"),Data.Players.Num(),Expected);
                Test->AddInfo(TEXT("[PrimalProgression] Knowledge restart server restored independent records"));
            }
            Next(99);return false;
        }
        if(!bClient)
        {
            if(Stage==0)
            {
                int32 Index=0;
                for(auto* P:Players)
                {
                    if(!P->GetInventory()->GetStacks().IsEmpty() || Progression(P)->GetExperience()!=0){Test->AddError(TEXT("Refusing non-fresh knowledge fixture"));return true;}
                    auto* Pawn=CastChecked<APFSurvivorCharacter>(P->GetPawn());Pawn->GetCharacterMovement()->DisableMovement();Pawn->SetActorLocation(FVector(-1200,Index*400,100));
                    Pawn->Survival->HungerDrainPerSecond=0;Pawn->Survival->ThirstDrainPerSecond=0;
                    Test->TestTrue(TEXT("Trusted identity role marker,not reward"),P->GetInventory()->Grant(bWeapon?TEXT("Item_Water"):TEXT("Item_Stone"),++Index));
                    Test->TestTrue(TEXT("Trusted recipe-access ingredients,not purchase output"),P->GetInventory()->Grant(TEXT("Item_Tool"),1) && P->GetInventory()->Grant(TEXT("Item_Cord"),1) && P->GetInventory()->Grant(TEXT("Item_Wood"),2));
                    if(bWeapon){Test->TestTrue(TEXT("Trusted optional weapon inputs, not earned items"),P->GetInventory()->Grant(TEXT("Item_Club"),1) && P->GetInventory()->Grant(TEXT("Item_Stone"),2));}
                    FPFProgressionRecord Seed;Seed.Experience=SeedXP();Test->TestTrue(TEXT("Explicit trusted level-boundary XP fixture,not earned pacing"),Progression(P)->Restore(Seed,Error));
                }
                Test->AddInfo(FString::Printf(TEXT("[PrimalProgression] Seeded explicit%dXP fixtures for purchase boundary,not an earned gameplay claim"),SeedXP()));Next(1);return false;
            }
            if(Stage==1)
            {
                for(auto* P:Players){auto* PG=Progression(P);auto* Craft=P->GetCrafting();const bool Learned=Marker(P->GetInventory())==1;if(!Restored(P) || !PG->GetKnowledgeFeedback().StartsWith(TEXT("Refused:")) || !Craft || !Craft->ActiveRecipe.IsNone() || !Craft->Feedback.StartsWith(Learned?TEXT("Cancelled"):TEXT("Locked:"))){return false;}}
                for(auto* P:Players){CheckState(P);}Test->TestTrue(TEXT("Actual isolated knowledge save"),W->GetSubsystem<UPFWorldPersistence>()->Save(Slot,Error));if(!Error.IsEmpty()){Test->AddError(Error);}
                Test->AddInfo(TEXT("[PrimalProgression] Knowledge server exact owned purchase,private record and save verified"));Next(99);return false;
            }
        }
        else
        {
            if(Stage==0)
            {
                if(G->GetExperience()!=SeedXP() || (Marker(I)!=1 && Marker(I)!=2)){return false;}
                Test->TestEqual(TEXT("Fresh seeded derived points"),G->GetAvailablePoints(),bWeapon?6:3);Test->TestTrue(TEXT("No knowledge granted withXP"),G->GetRecord().Knowledge.IsEmpty());
                Test->TestFalse(TEXT("Client cannot directly spend points"),G->RequestKnowledge(bWeapon?TEXT("Tech_FieldWeapons"):TEXT("Tech_FieldTools"),PC->GetPawn(),Error));
                Test->TestTrue(TEXT("Client cannot forge request feedback"),G->GetKnowledgeFeedback().IsEmpty());
                PC->ServerCraftAction(AccessRecipe(),false);PC->ServerLearnKnowledge(bWeapon?TEXT("Tech_FieldWeapons"):TEXT("Tech_Unknown"));Next(1);return false;
            }
            if(Stage==1 && Now-Changed>.7)
            {
                if(!G->GetKnowledgeFeedback().StartsWith(TEXT("Refused:"))){return false;}
                if(!C->Feedback.StartsWith(TEXT("Locked:"))){return false;}Test->TestTrue(TEXT("Fresh locked RPC consumes no ingredients"),C->ActiveRecipe.IsNone() && I->Count(TEXT("Item_Tool"))==1 && I->Count(TEXT("Item_Cord"))==1 && I->Count(TEXT("Item_Wood"))==2);
                Test->TestTrue(TEXT("Invalid owned RPC leaves record and inventory"),G->GetRecord().Knowledge.IsEmpty() && G->GetExperience()==SeedXP() && Marker(I)>=1);
                if(Marker(I)==1){PC->ServerLearnKnowledge(TEXT("Tech_FieldTools"));PC->ServerLearnKnowledge(TEXT("Tech_FieldTools"));Next(2);}
                else{Next(4);}return false;
            }
            if(Stage==2 && Now-Changed>.7)
            {
                if(G->GetRecord().Knowledge.Num()!=1){return false;}
                if(bWeapon){Test->TestEqual(TEXT("Parent alone leaves four earned-boundary points"),G->GetAvailablePoints(),4);Test->TestFalse(TEXT("Parent alone does not unlock optional weapon"),G->CanCraftRecipe(AccessRecipe(),Error));PC->ServerLearnKnowledge(TEXT("Tech_FieldWeapons"));PC->ServerLearnKnowledge(TEXT("Tech_FieldWeapons"));Next(6);return false;}
                CheckState(PC);PC->ServerLearnKnowledge(TEXT("Tech_FieldTools"));PC->ServerCraftAction(AccessRecipe(),false);Next(3);return false;
            }
            if(Stage==6 && Now-Changed>.7)
            {if(G->GetRecord().Knowledge.Num()!=2){return false;}CheckState(PC);PC->ServerLearnKnowledge(TEXT("Tech_FieldWeapons"));PC->ServerCraftAction(AccessRecipe(),false);Next(3);return false;}
            if(Stage==3 && Now-Changed>.7)
            {if(!G->GetKnowledgeFeedback().StartsWith(TEXT("Refused:")) || C->ActiveRecipe!=AccessRecipe()){return false;}PC->ServerCraftAction(NAME_None,true);Next(5);return false;}
            if(Stage==5 && Now-Changed>.7)
            {if(!C->ActiveRecipe.IsNone() || !C->Feedback.StartsWith(TEXT("Cancelled"))){return false;}Next(4);return false;}
            if(Stage==4)
            {
                if(!Privacy(W,PC)){return false;}CheckState(PC);
                Test->AddInfo(TEXT("[PrimalProgression] Knowledge owned RPC,deduplicated spending,independent privacy and client refusal verified"));Next(99);return false;
            }
        }
        return false;
    }
private:
    int32 SeedXP() const{return bWeapon?250:100;}
    int32 Marker(const UPFInventoryComponent* I) const{return I->Count(bWeapon?TEXT("Item_Water"):TEXT("Item_Stone"));}
    FName AccessRecipe() const{return bWeapon?TEXT("Recipe_BoundClub"):TEXT("Recipe_BoundTool");}
    TArray<FName> LearnedGraph() const{return bWeapon?TArray<FName>{TEXT("Tech_FieldTools"),TEXT("Tech_FieldWeapons")}:TArray<FName>{TEXT("Tech_FieldTools")};}
    UPFProgressionComponent* Progression(APFSurvivalPlayerController* P){auto* PS=P->GetPlayerState<APFInventoryPlayerState>();return PS?PS->Progression.Get():nullptr;}
    bool Restored(APFSurvivalPlayerController* P)
    {
        auto* G=Progression(P);const int32 OwnerMarker=Marker(P->GetInventory());
        return G && G->GetExperience()==SeedXP() && ((OwnerMarker==1 && G->GetRecord().Knowledge==LearnedGraph()) || (OwnerMarker==2 && G->GetRecord().Knowledge.IsEmpty()));
    }
    void CheckState(APFSurvivalPlayerController* P)
    {
        auto* G=Progression(P);auto* I=P->GetInventory();const int32 OwnerMarker=Marker(I);
        Test->TestEqual(TEXT("Purchase never changes XP"),G->GetExperience(),SeedXP());Test->TestEqual(TEXT("Derived point spending exact per owner"),G->GetAvailablePoints(),OwnerMarker==1?1:(bWeapon?6:3));
        Test->TestTrue(TEXT("Correct independent knowledge"),OwnerMarker==1?G->GetRecord().Knowledge==LearnedGraph():G->GetRecord().Knowledge.IsEmpty());
        Test->TestTrue(TEXT("No crafting credit or free items, exact access fixture inputs"),G->GetRecord().CreditedCrafts.IsEmpty() && I->GetStacks().Num()==(bWeapon?6:4) && (OwnerMarker==1 || OwnerMarker==2) && I->Count(TEXT("Item_Tool"))==1 && I->Count(TEXT("Item_Cord"))==1 && I->Count(TEXT("Item_Wood"))==2 && I->Count(TEXT("Item_BoundTool"))==0 && I->Count(TEXT("Item_BoundClub"))==0 && (!bWeapon || (I->Count(TEXT("Item_Club"))==1 && I->Count(TEXT("Item_Stone"))==2)));
        FString Error;Test->TestEqual(TEXT("Recipe access follows this owner's persisted knowledge"),G->CanCraftRecipe(AccessRecipe(),Error),OwnerMarker==1);
    }
    bool Privacy(UWorld* W,APFSurvivalPlayerController* PC)
    {
        int32 Others=0;
        for(TActorIterator<APFInventoryPlayerState> It(W);It;++It){if(*It!=PC->PlayerState){++Others;auto* G=It->Progression.Get();if(!G){return false;}Test->TestTrue(TEXT("Foreign XP,knowledge,credit and feedback private"),G->GetExperience()==0 && G->GetRecord().Knowledge.IsEmpty() && G->GetRecord().CreditedCrafts.IsEmpty() && G->GetKnowledgeFeedback().IsEmpty());}}
        return Others==Expected-1;
    }
    FAutomationTestBase* Test;FString Slot;double Started=FPlatformTime::Seconds(),Changed=0;int32 Stage=0,Expected=1;bool bRestore=false,bClient=false,bWeapon=false;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFKnowledgeLiveTest,"PF.Progression.KnowledgeLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFKnowledgeLiveTest::RunTest(const FString&)
{
    FString Slot;int32 Expected=0;FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunKnowledgeTests")) || !FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) || (Expected!=1 && Expected!=2) ||
        !FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot) || !Slot.StartsWith(TEXT("AutomationM12Knowledge")) || !FPFSaveFileStore::ValidSlot(Slot))
    {AddError(TEXT("Requires isolated NullRHI -PFRunKnowledgeTests -PFExpectedPlayers=1|2 -PFSaveSlot=AutomationM12Knowledge..."));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FKnowledgeLiveExercise(this));return true;
}
#endif
