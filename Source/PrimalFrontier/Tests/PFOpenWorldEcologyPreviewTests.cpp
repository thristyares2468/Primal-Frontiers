#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "World/PFWorldClock.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

class FPFAuthoredPreview : public IAutomationLatentCommand
{
public:
    FPFAuthoredPreview(FAutomationTestBase* In,FString Path):Test(In),Directory(MoveTemp(Path)),Started(FPlatformTime::Seconds()){}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>90){Test->AddError(TEXT("Authored rendered timeout"));return true;}if(Now<Until){return false;}
        UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->GetNetMode()==NM_Standalone && C.World()->IsGameWorld()){W=C.World();break;}}
        if(!W){return false;}auto* PC=Cast<APFSurvivalPlayerController>(W->GetFirstPlayerController());if(!PC || !PC->GetPawn()){return false;}
        APFWorldClock* Clock=nullptr;for(TActorIterator<APFWorldClock> It(W);It;++It){Clock=*It;}if(!Clock){return false;}
        APFCreature* Prowler=nullptr;int32 Foragers=0;for(TActorIterator<APFCreature> It(W);It;++It){if(It->bHostile){Prowler=*It;}else{++Foragers;}}
        if(Stage==0)
        {
            if(!Foragers){return false;}Test->TestNull(TEXT("Authored ridge dormant at day"),Prowler);
            int32 Configured=0;for(TActorIterator<APFCreatureSpawner> It(W);It;++It){Configured+=It->bUseSpawnPolicy && It->SpawnPolicies!=nullptr;}
            Test->TestEqual(TEXT("Both points use authored policy without preview reassignment"),Configured,2);
            Clock->SetHour(21);Stage=1;return false;
        }
        if(Stage==1)
        {
            if(!Prowler){return false;}Resident=Prowler;Prowler->SetActorTickEnabled(true);Prowler->GetCharacterMovement()->DisableMovement();
            const FVector At=Prowler->GetActorLocation();PC->GetPawn()->SetActorLocation(At+FVector(-400,0,50));FVector Eye;FRotator Look;PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((At-Eye).Rotation());
            Stage=2;Until=Now+1;return false;
        }
        if(Stage==2){Prowler->SetActorTickEnabled(false);FScreenshotRequest::RequestScreenshot(Directory/TEXT("authored_night.png"),true,false);Stage=3;Until=Now+1;return false;}
        if(Stage==3){Test->TestTrue(TEXT("Actual authored night PNG"),IFileManager::Get().FileSize(*(Directory/TEXT("authored_night.png")))>0);Clock->SetHour(9);Prowler->SetActorTickEnabled(true);Stage=4;Until=Now+1;return false;}
        if(Stage==4){Prowler->SetActorTickEnabled(false);FScreenshotRequest::RequestScreenshot(Directory/TEXT("authored_dawn.png"),true,false);Stage=5;Until=Now+1;return false;}
        Test->TestTrue(TEXT("Actual authored dawn PNG"),IFileManager::Get().FileSize(*(Directory/TEXT("authored_dawn.png")))>0);
        Test->TestEqual(TEXT("Dawn preserves same resident"),Prowler,Resident.Get());Test->TestEqual(TEXT("No phase duplication"),APFCreatureSpawner::Count(W),2);
        Test->AddInfo(TEXT("[PrimalAgentTools] Authored open-world night and dawn rendered without policy reassignment"));return true;
    }
private:
    FAutomationTestBase* Test;FString Directory;double Started,Until=0;int32 Stage=0;TWeakObjectPtr<APFCreature> Resident;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFAuthoredPreviewTest,"PF.Creatures.AuthoredPreviewLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFAuthoredPreviewTest::RunTest(const FString&)
{
    FString Run;
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunAuthoredPreviewTest")) || !FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Run) || Run.Len()>64 || Run.Contains(TEXT("/")) || Run.Contains(TEXT("\\")) || Run.Contains(TEXT("..")))
    {AddError(TEXT("Requires isolated authored rendered runner"));return false;}
    FString Directory=FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Run;IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FPFAuthoredPreview(this,Directory));return true;
}
#endif
