#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "Inventory/PFItemPickup.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

class FPFVarietyPreview : public IAutomationLatentCommand
{
public:
    FPFVarietyPreview(FAutomationTestBase* In,FString Path):Test(In),Directory(MoveTemp(Path)),Started(FPlatformTime::Seconds()){}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>90){Test->AddError(FString::Printf(TEXT("Variety rendered timeout stage=%d"),Stage));return true;}if(Now<Until){return false;}
        UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld() && C.World()->GetNetMode()==NM_Standalone){W=C.World();break;}}
        if(!W){return false;}auto* PC=Cast<APFSurvivalPlayerController>(W->GetFirstPlayerController());if(!PC || !PC->GetPawn() || !PC->GetInventory()){return false;}
        auto Aim=[&](FVector At){FVector Eye;FRotator View;PC->GetPawn()->GetActorEyesViewPoint(Eye,View);PC->SetControlRotation((At-Eye).Rotation());};
        auto Tab=[&](){for(auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==EKeys::Tab && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(EKeys::Tab);return;}}Test->AddError(TEXT("Tab binding absent"));};
        if(Stage==0)
        {
            for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}
            for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            auto* C=APFCreatureSpawner::Spawn(W,TEXT("Creature_Scrabbler"),FVector(-1500,1700,60));
            if(!Test->TestNotNull(TEXT("Actual nav-valid Scrabbler"),C)){return true;}Creature=C;Stage=1;Until=Now+.3;return false;
        }
        if(Stage==1 || Stage==4)
        {
            auto* C=Creature.Get();if(!Test->TestNotNull(TEXT("Resident survives setup"),C)){return true;}
            C->SetActorTickEnabled(true);C->GetCharacterMovement()->DisableMovement();
            PC->GetPawn()->SetActorLocation(C->GetActorLocation()+FVector(-500,0,50));Aim(C->GetActorLocation());
            ++Stage;Until=Now+1;return false;
        }
        if(Stage==2){Creature->SetActorTickEnabled(false);FScreenshotRequest::RequestScreenshot(Directory/TEXT("scrabbler.png"),true,false);Stage=3;Until=Now+1;return false;}
        if(Stage==3)
        {
            Test->TestTrue(TEXT("Scrabbler image exists"),IFileManager::Get().FileSize(*(Directory/TEXT("scrabbler.png")))>0);
            Creature->Destroy();auto* C=APFCreatureSpawner::Spawn(W,TEXT("Creature_DuskPouncer"),FVector(1300,4400,60));
            if(!Test->TestNotNull(TEXT("Actual nav-valid Dusk Pouncer"),C)){return true;}Creature=C;Stage=4;Until=Now+.3;return false;
        }
        if(Stage==5){Creature->SetActorTickEnabled(false);FScreenshotRequest::RequestScreenshot(Directory/TEXT("dusk_pouncer.png"),true,false);Stage=6;Until=Now+1;return false;}
        if(Stage==6)
        {
            Test->TestTrue(TEXT("Pouncer image exists"),IFileManager::Get().FileSize(*(Directory/TEXT("dusk_pouncer.png")))>0);
            Creature->TakeDamage(1000,FDamageEvent(),PC,PC->GetPawn());Creature->SetActorTickEnabled(true);
            PC->GetPawn()->SetActorLocation(Creature->GetActorLocation()+FVector(-120,0,50));Stage=7;Until=Now+.5;return false;
        }
        if(Stage==7)
        {
            const FName Needed=PC->GetInventory()->Count(TEXT("Item_Food"))==0?FName(TEXT("Item_Food")):FName(TEXT("Item_Fibre"));
            if(PC->GetInventory()->Count(TEXT("Item_Food"))==4 && PC->GetInventory()->Count(TEXT("Item_Fibre"))==2){Tab();Stage=8;Until=Now+1;return false;}
            for(TActorIterator<APFItemPickup> It(W);It;++It){if(It->GetContents().ItemId==Needed){Aim(It->GetActorLocation());PC->Interact();Until=Now+.5;return false;}}
            Test->AddError(TEXT("Required dropped item is missing"));return true;
        }
        if(Stage==8){Creature->SetActorTickEnabled(false);FScreenshotRequest::RequestScreenshot(Directory/TEXT("variety_loot.png"),true,false);Stage=9;Until=Now+1;return false;}
        Test->TestTrue(TEXT("Loot inventory image exists"),IFileManager::Get().FileSize(*(Directory/TEXT("variety_loot.png")))>0);Tab();
        Test->AddInfo(TEXT("[PrimalAgentTools] Rendered original variants and real E recovery of food/fibre loot verified"));return true;
    }
private:
    FAutomationTestBase* Test;FString Directory;double Started,Until=0;int32 Stage=0;TWeakObjectPtr<APFCreature> Creature;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFVarietyPreviewTest,"PF.Creatures.VarietyPreviewLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFVarietyPreviewTest::RunTest(const FString&)
{
    FString Run;if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunVarietyPreviewTest")) || !FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Run) || Run.Len()>64 || Run.Contains(TEXT("/")) || Run.Contains(TEXT("\\")) || Run.Contains(TEXT(".."))){AddError(TEXT("Requires isolated rendered variety fixture"));return false;}
    const FString Directory=FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Run;IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FPFVarietyPreview(this,Directory));return true;
}
#endif
