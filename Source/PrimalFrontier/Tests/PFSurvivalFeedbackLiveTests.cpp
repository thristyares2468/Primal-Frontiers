// Opt-in, one rendered standalone world. Reads real UMG text after authoritative
// damage/needs/death/automatic respawn and a bounded possession gap.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivalHUD.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFInventoryComponent.h"
#include "Settings/PFGameUserSettings.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Pawn.h"
#include "UnrealClient.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"

namespace
{
class FSurvivalUIExercise final : public IAutomationLatentCommand
{
public:
    FSurvivalUIExercise(FAutomationTestBase* T,FString D):Test(T),Directory(MoveTemp(D)){}
    ~FSurvivalUIExercise(){if(bSettingsChanged && UPFGameUserSettings::Get()){auto& P=UPFGameUserSettings::Get()->Preferences;P.bControlHints=bOriginalHints;P.HUDScale=OriginalScale;}}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>75){Test->AddError(TEXT("Survival feedback UI timeout"));return true;}
        if(Now<Until){return false;}
        if(Phase==0)
        {
            for(const auto& C:GEngine->GetWorldContexts())
            {if(C.World() && C.World()->IsGameWorld()){auto* P=Cast<APFSurvivalPlayerController>(C.World()->GetFirstPlayerController());if(P && P->GetLocalPlayer() && P->GetPawn() && P->SurvivalHUD){PC=P;break;}}}
            if(!PC.IsValid()){return false;}
            if(!PC->HasAuthority() || !PC->GetInventory() || !PC->GetInventory()->GetStacks().IsEmpty())
            {Test->AddError(TEXT("Refusing non-fresh standalone survival fixture"));return true;}
            if(!PC->SurvivalHUD->WidgetTree->FindWidget(TEXT("PF_StateLabel")))
            {Test->AddError(TEXT("Requires the real native placeholder HUD"));return true;}
            if(auto* Settings=UPFGameUserSettings::Get())
            {
                bOriginalHints=Settings->Preferences.bControlHints;OriginalScale=Settings->Preferences.HUDScale;
                float Scale=1;FParse::Value(FCommandLine::Get(),TEXT("PFHUDScale="),Scale);
                Settings->Preferences.HUDScale=FMath::Clamp(Scale,0.75f,1.5f);Settings->Preferences.bControlHints=false;bSettingsChanged=true;
                Test->AddInfo(FString::Printf(TEXT("[PrimalUI] HUD text scale %.2f; hints off"),Settings->Preferences.HUDScale));
            }
            Test->TestTrue(TEXT("Baseline health accepted by authority"),Survival()->SetHealth(80));
            Wait(Now,2,1);return false;
        }
        if(!PC.IsValid() || !PC->SurvivalHUD){Test->AddError(TEXT("Lost survival UI fixture"));return true;}
        if(Phase==1)
        {
            Test->TestTrue(TEXT("Baseline has settled without a stale damage cue"),Text(TEXT("PF_DamageLabel")).IsEmpty());
            Test->TestTrue(TEXT("Authority accepts damage"),Survival()->ApplyDamage(20));Wait(Now,0.3,2);return false;
        }
        if(Phase==2)
        {
            Test->TestTrue(TEXT("HUD follows actual damaged health"),Text(TEXT("PF_HealthLabel")).Contains(TEXT("60 / 100")));
            Test->TestTrue(TEXT("Observed health drop produces damage text"),Text(TEXT("PF_DamageLabel")).Contains(TEXT("20.0")));
            Capture(TEXT("damage"));Wait(Now,0.4,3);return false;
        }
        if(Phase==3)
        {
            if(!CheckShot()){return false;}
            Survival()->SetHunger(0);Survival()->SetThirst(100);Wait(Now,0.3,4);return false;
        }
        if(Phase==4)
        {
            Test->TestTrue(TEXT("Empty food identifies starvation"),Text(TEXT("PF_StateLabel")).Contains(TEXT("STARVING")));
            Test->TestFalse(TEXT("Full water is not mislabeled dehydration"),Text(TEXT("PF_StateLabel")).Contains(TEXT("DEHYDRATED")));
            Survival()->SetHunger(20);Survival()->SetThirst(20);Survival()->SetExposure(0.1f);Survival()->SetHealth(25);
            Wait(Now,0.3,5);return false;
        }
        if(Phase==5)
        {
            const FString Status=Text(TEXT("PF_StateLabel"));
            for(const TCHAR* Expected:{TEXT("CRITICAL HEALTH"),TEXT("LOW FOOD"),TEXT("LOW WATER"),TEXT("EXPOSURE")})
            {Test->TestTrue(FString(TEXT("Warning visible with hints disabled: "))+Expected,Status.Contains(Expected));}
            auto* Label=PC->SurvivalHUD->WidgetTree->FindWidget(TEXT("PF_VitalsBounds"));
            const FGeometry Root=PC->SurvivalHUD->GetCachedGeometry(),G=Label->GetCachedGeometry();
            const FVector2D TL=Root.AbsoluteToLocal(G.LocalToAbsolute(FVector2D::ZeroVector));
            const FVector2D BR=Root.AbsoluteToLocal(G.LocalToAbsolute(G.GetLocalSize()));
            Test->TestTrue(TEXT("Urgent panel fits viewport bounds"),TL.X>=0 && TL.Y>=0 && BR.X<=Root.GetLocalSize().X && BR.Y<=Root.GetLocalSize().Y);
            Test->TestFalse(TEXT("Damage text never rounds real loss to zero"),Text(TEXT("PF_DamageLabel")).Contains(TEXT("-0.00")));
            Capture(TEXT("urgent"));Wait(Now,0.4,6);return false;
        }
        if(Phase==6)
        {
            if(!CheckShot()){return false;}
            Survival()->SetHunger(100);Survival()->SetThirst(0);Survival()->SetExposure(0);
            Wait(Now,0.3,7);return false;
        }
        if(Phase==7)
        {
            Test->TestTrue(TEXT("Empty water identifies dehydration"),Text(TEXT("PF_StateLabel")).Contains(TEXT("DEHYDRATED")));
            Test->TestFalse(TEXT("Full food is not mislabeled starvation"),Text(TEXT("PF_StateLabel")).Contains(TEXT("STARVING")));
            Survival()->SetThirst(100);Survival()->SetHealth(80);Wait(Now,2,8);return false;
        }
        if(Phase==8)
        {
            Test->TestTrue(TEXT("Damage cue expires after recovery"),Text(TEXT("PF_DamageLabel")).IsEmpty());
            Test->TestTrue(TEXT("Recovered status clears despite hints off"),Text(TEXT("PF_StateLabel")).IsEmpty());
            OldPawn=PC->GetPawn();FDamageEvent Damage;OldPawn->TakeDamage(1000,Damage,PC.Get(),nullptr);
            Wait(Now,0.3,9);return false;
        }
        if(Phase==9)
        {
            Test->TestTrue(TEXT("Real death displays waiting for server"),Text(TEXT("PF_StateLabel")).Contains(TEXT("You died")));
            CheckNoAim();Capture(TEXT("death"));Wait(Now,0.4,10);return false;
        }
        if(Phase==10)
        {
            if(!CheckShot()){return false;}
            Phase=11;return false;
        }
        if(Phase==11)
        {
            if(!PC->GetPawn() || PC->GetPawn()==OldPawn.Get()){return false;}
            Wait(Now,0.4,12);return false;
        }
        if(Phase==12)
        {
            Test->TestFalse(TEXT("Automatic replacement is alive"),Survival()->IsDead());
            Test->TestTrue(TEXT("Replacement health is rendered"),Text(TEXT("PF_HealthLabel")).Contains(TEXT("100 / 100")));
            Test->TestTrue(TEXT("Replacement cannot inherit damage cue"),Text(TEXT("PF_DamageLabel")).IsEmpty());
            Test->TestFalse(TEXT("Death status clears"),Text(TEXT("PF_StateLabel")).Contains(TEXT("You died")));
            Capture(TEXT("respawn"));Wait(Now,0.4,13);return false;
        }
        if(Phase==13)
        {
            if(!CheckShot()){return false;}
            Replacement=PC->GetPawn();PC->UnPossess();Wait(Now,0.4,14);return false;
        }
        if(Phase==14)
        {
            Test->TestEqual(TEXT("Missing possession clears numeric health"),Text(TEXT("PF_HealthLabel")),FString(TEXT("HEALTH --")));
            Test->TestTrue(TEXT("Explicit waiting state"),Text(TEXT("PF_StateLabel")).Contains(TEXT("Waiting for survivor")));
            auto* Bar=Cast<UProgressBar>(PC->SurvivalHUD->WidgetTree->FindWidget(TEXT("PF_HealthBar")));
            Test->TestTrue(TEXT("Missing possession clears old bar"),Bar && Bar->GetPercent()==0);
            CheckNoAim();Capture(TEXT("waiting"));Wait(Now,0.4,15);return false;
        }
        if(Phase==15)
        {
            if(!CheckShot()){return false;}
            if(!Replacement.IsValid()){Test->AddError(TEXT("Lost replacement pawn"));return true;}
            PC->Possess(Replacement.Get());Wait(Now,0.4,16);return false;
        }
        Test->TestTrue(TEXT("Repossessed health returns"),Text(TEXT("PF_HealthLabel")).Contains(TEXT("100 / 100")));
        Test->TestTrue(TEXT("Repossessing cannot create false damage"),Text(TEXT("PF_DamageLabel")).IsEmpty());
        Test->AddInfo(TEXT("[PrimalUI] Actual damage, independent needs warnings, server automatic respawn and missing-possession UMG verified. No human traversal claim."));return true;
    }
private:
    UPFPlayerSurvivalComponent* Survival(){return PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();}
    FString Text(FName Name)
    {auto* Label=Cast<UTextBlock>(PC->SurvivalHUD->WidgetTree->FindWidget(Name));if(!Label){Test->AddError(TEXT("Missing real HUD text ")+Name.ToString());return FString();}return Label->GetText().ToString();}
    void CheckNoAim()
    {auto* Aim=PC->SurvivalHUD->WidgetTree->FindWidget(TEXT("PF_AimMarker"));Test->TestTrue(TEXT("No aim while dead/unpossessed"),Aim && Aim->GetVisibility()==ESlateVisibility::Hidden);Test->TestTrue(TEXT("No stale interaction while dead/unpossessed"),Text(TEXT("PF_InteractionLabel")).IsEmpty());}
    void Wait(double Now,double Seconds,int32 Next){Until=Now+Seconds;Phase=Next;}
    void Capture(const TCHAR* Name){Shot=Directory/(FString(Name)+TEXT(".png"));FScreenshotRequest::RequestScreenshot(Shot,true,false);}
    bool CheckShot(){if(IFileManager::Get().FileSize(*Shot)<=0){return false;}Test->AddInfo(TEXT("[PrimalUI] Survival HUD screenshot: ")+Shot);return true;}
    FAutomationTestBase* Test;FString Directory,Shot;TWeakObjectPtr<APFSurvivalPlayerController> PC;
    TWeakObjectPtr<APawn> OldPawn,Replacement;int32 Phase=0;double Started=FPlatformTime::Seconds(),Until=0;
    bool bOriginalHints=true,bSettingsChanged=false;float OriginalScale=1;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSurvivalFeedbackLiveTest,"PF.UI.SurvivalFeedbackLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFSurvivalFeedbackLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi")))
    {AddError(TEXT("Requires isolated rendered -game -PFRunControlsUITest"));return false;}
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(Label.IsEmpty() || Label.Len()>48){AddError(TEXT("Supply unique bounded evidence label"));return false;}
    for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Invalid evidence label"));return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);
    if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing to overwrite prior UI evidence"));return false;}
    IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FSurvivalUIExercise(this,Directory));return true;
}
#endif
