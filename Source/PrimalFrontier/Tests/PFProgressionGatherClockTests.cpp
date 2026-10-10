#if WITH_DEV_AUTOMATION_TESTS
#include "Progression/PFProgressionComponent.h"
#include "Progression/PFProgressionSaveFormat.h"
#include "Progression/PFProgressionCatalog.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Crafting/PFCraftingComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Persistence/PFSaveFileStore.h"
#include "Persistence/PFWorldPersistence.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/PlayerStart.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"
#include "Tests/AutomationCommon.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFGatherClockTest,"PF.Progression.GatherClock",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFGatherClockTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(4000,4000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());if(!PS || !P || !PS->Progression){return false;}
    auto* G=PS->Progression.Get();auto* C=PS->Crafting.Get();auto* I=PS->Inventory.Get();FString Error;
    P->GetCharacterMovement()->DisableMovement();P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;
    W->GetSubsystem<UPFWorldPersistence>()->ActiveSlot.Reset();
    FPFProgressionRecord Seed;Seed.Experience=100;
    Seed.GatherWindows={{FPFProgressionTransactions::GatherCategoryForItem(TEXT("Item_Wood")),2,20},{FPFProgressionTransactions::GatherCategoryForItem(TEXT("Item_Stone")),1,50}};
    W->TimeSeconds=100; // Public Unreal clock fields in a disposable fixture; never production time overrides.
    if(!TestTrue(TEXT("Trusted valid window seed"),G->Restore(Seed,Error))){AddError(Error);return false;}
    FPFProgressionRecord First,Repeated;W->TimeSeconds=112.5;
    if(!TestTrue(TEXT("Active elapsed capture"),G->Capture(First,Error)) || First.GatherWindows.Num()!=2){return false;}
    TestTrue(TEXT("Exact elapsed per category"),First.GatherWindows[0].RemainingSeconds==7.5 && First.GatherWindows[1].RemainingSeconds==37.5);
    TestTrue(TEXT("Capture never changes stored epoch/counts/XP"),G->GetRecord().GatherWindows==Seed.GatherWindows && G->GetExperience()==100);
    TestTrue(TEXT("Repeated snapshot does not double-age"),G->Capture(Repeated,Error) && Repeated.GatherWindows==First.GatherWindows);
    const double PauseAt=W->GetTimeSeconds();W->GetWorldSettings()->SetPauserPlayerState(PS);Fixture.TickTestWorld(0.5f);
    TestEqual(TEXT("Unreal paused game clock freezes"),W->GetTimeSeconds(),PauseAt);
    TestTrue(TEXT("Paused snapshot preserves duration"),G->Capture(Repeated,Error) && Repeated.GatherWindows==First.GatherWindows);W->GetWorldSettings()->SetPauserPlayerState(nullptr);
    W->TimeSeconds=120;
    TestTrue(TEXT("Exact category expiry keeps XP and other timer"),G->Capture(Repeated,Error) && Repeated.Experience==100 && Repeated.GatherWindows.Num()==1 && Repeated.GatherWindows[0].RemainingSeconds==30 && Repeated.GatherWindows[0].Rewards==1);
    W->TimeSeconds=150;TestTrue(TEXT("Exact complete expiry preserves progression"),G->Capture(Repeated,Error) && Repeated.Experience==100 && Repeated.GatherWindows.IsEmpty());
    W->TimeSeconds=99;Repeated.Experience=250;TestFalse(TEXT("Backward clock refuses"),G->Capture(Repeated,Error));TestEqual(TEXT("Refused snapshot preserves output"),Repeated.Experience,250);
    W->TimeSeconds=std::numeric_limits<double>::quiet_NaN();TestFalse(TEXT("NaN clock capture refuses"),G->Capture(Repeated,Error));TestFalse(TEXT("NaN clock restore refuses"),G->Restore({},Error));
    W->TimeSeconds=std::numeric_limits<double>::infinity();TestFalse(TEXT("Infinite clock refuses"),G->Capture(Repeated,Error));TestFalse(TEXT("Infinite restore refuses"),G->Restore({},Error));
    W->TimeSeconds=-1;TestFalse(TEXT("Negative restore clock refuses"),G->Restore({},Error));TestTrue(TEXT("Invalid clocks preserve stored record"),G->GetRecord().GatherWindows==Seed.GatherWindows && G->GetExperience()==100);
    W->TimeSeconds=100;PS->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Direct client cannot capture clock"),G->Capture(Repeated,Error));TestFalse(TEXT("Direct client cannot reset clock"),G->Restore(Seed,Error));PS->SetRole(ROLE_Authority);
    W->TimeSeconds=112.5;TestTrue(TEXT("Refusals cannot reset clock"),G->Capture(Repeated,Error) && Repeated.GatherWindows==First.GatherWindows);
    TArray<uint8> Bytes;const auto* Catalog=GetDefault<UPFProgressionCatalog>();
    const FString Slot=TEXT("AutomationGatherClock_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT{W->TimeSeconds=1000;W->GetWorldSettings()->SetPauserPlayerState(nullptr);for(bool Backup:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,Backup));}};
    if(!TestTrue(TEXT("Write actual disposable captured record"),FPFProgressionSaveFormat::Encode(First,*Catalog,*C->Catalog,*I->Catalog,Bytes,Error) && FPFSaveFileStore::Write(Slot,Bytes,Error))){AddError(Error);return false;}
    FPFSavedFile File;FPFProgressionRecord Loaded;
    if(!TestTrue(TEXT("Read saved window data"),FPFSaveFileStore::Read(Slot,File,Error) && FPFProgressionSaveFormat::Decode(File.Payload,*Catalog,*C->Catalog,*I->Catalog,Loaded,Error))){AddError(Error);return false;}
    W->TimeSeconds=1000; // Simulated new server epoch; real new-process network gate is separate.
    TestTrue(TEXT("Restore rebases remaining duration without offline aging"),G->Restore(Loaded,Error) && G->Capture(Repeated,Error) && Repeated.GatherWindows==First.GatherWindows);
    auto Bad=Loaded;Bad.GatherWindows[0].Rewards=0;W->TimeSeconds=1002;
    TestFalse(TEXT("Malformed restore cannot refresh valid timer"),G->Restore(Bad,Error));
    TestTrue(TEXT("Valid timer continues after refused restore"),G->Capture(Repeated,Error) && Repeated.GatherWindows[0].RemainingSeconds==5.5 && Repeated.GatherWindows[1].RemainingSeconds==35.5);
    TestTrue(TEXT("Actual owned purchase commits aged snapshot"),G->RequestKnowledge(TEXT("Tech_FieldTools"),P,Error));
    TestTrue(TEXT("Purchase never resets reward budget"),G->GetRecord().GatherWindows==Repeated.GatherWindows && G->GetAvailablePoints()==1 && G->GetExperience()==100);
    W->TimeSeconds=1003;TestTrue(TEXT("Age after knowledge commit once"),G->Capture(Repeated,Error) && Repeated.GatherWindows[0].RemainingSeconds==4.5 && Repeated.GatherWindows[1].RemainingSeconds==34.5);
    C->Catalog=DuplicateObject<UPFCraftingCatalog>(C->Catalog,C);for(auto& R:C->Catalog->Recipes){R.Duration=0.5f;}
    TestTrue(TEXT("Trusted recipe ingredients"),I->Grant(TEXT("Item_Wood"),6) && I->Grant(TEXT("Item_Stone"),4));
    auto Complete=[&](){for(int32 N=0;N<8;++N){Fixture.TickTestWorld(0.1f);}};
    TestTrue(TEXT("Actual timed craft starts"),C->Start(TEXT("Recipe_Tool"),P));Complete();
    if(!TestTrue(TEXT("Craft commits earned XP and aged clock"),G->Capture(Repeated,Error) && G->GetExperience()==120 && G->GetRecord().CreditedCrafts.Num()==1 && Repeated.GatherWindows.Num()==2)){return false;}
    TestTrue(TEXT("Craft leaves exact original expiry epoch"),FMath::IsNearlyEqual(Repeated.GatherWindows[0].RemainingSeconds,1007.5-W->GetTimeSeconds(),0.000001));
    TestTrue(TEXT("Actual repeat craft starts"),C->Start(TEXT("Recipe_Tool"),P));Complete();
    TestTrue(TEXT("Repeat commits timer without rewarding again"),G->Capture(Repeated,Error) && G->GetExperience()==120 && I->Count(TEXT("Item_Tool"))==2 && FMath::IsNearlyEqual(Repeated.GatherWindows[0].RemainingSeconds,1007.5-W->GetTimeSeconds(),0.000001));
    TestTrue(TEXT("Legacy empty restore clears clock windows without credit"),G->Restore({},Error) && G->Capture(Repeated,Error) && Repeated.GatherWindows.IsEmpty() && G->GetExperience()==0);
    Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] GatherClock: real paused world tick, exact active expiry/read-only captures, actual file/rebased epoch, malformed/client/clock refusal and actual purchase/craft/repeat conservation. Trusted window seeds, no live gather XP or process restart certificate."));return true;
}
#endif
