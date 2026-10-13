#if WITH_DEV_AUTOMATION_TESTS
#include "Progression/PFProgressionComponent.h"
#include "Progression/PFProgressionSaveFormat.h"
#include "Progression/PFProgressionCatalog.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Crafting/PFCraftingComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFTraversalMovement.h"
#include "Creatures/PFCreature.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFPlayerSaveAdapter.h"
#include "Persistence/PFSaveFileStore.h"
#include "World/PFWorldClock.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Misc/Crc.h"
#include "HAL/FileManager.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFStatRecordsTest,"PF.Progression.StatRecords",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFStatRecordsTest::RunTest(const FString&)
{
    auto* C=NewObject<UPFProgressionCatalog>();auto* Craft=NewObject<UPFCraftingCatalog>();auto* Items=NewObject<UPFItemCatalog>();FString Error;
    FPFProgressionRecord R;TestEqual(TEXT("No stat points at level1"),FPFProgressionTransactions::AvailableStatPoints(R),0);
    C->StaminaPerPoint=-1;TestFalse(TEXT("Unsafe designer tuning is rejected"),C->Validate(*Craft,*Items,Error));C->StaminaPerPoint=20;
    TestFalse(TEXT("Level1 cannot spend"),FPFProgressionTransactions::PurchaseStat(R,EPFPlayerStat::Attack,*C,*Craft,*Items,Error));
    R.Experience=1000;
    for(auto Stat:{EPFPlayerStat::Attack,EPFPlayerStat::Stamina,EPFPlayerStat::Weight,EPFPlayerStat::CraftingSpeed})
    {TestTrue(TEXT("One earned-boundary point per chosen rank"),FPFProgressionTransactions::PurchaseStat(R,Stat,*C,*Craft,*Items,Error));}
    TestTrue(TEXT("Independent exact budgets, no XP/items/knowledge granted"),R.Experience==1000 && R.Knowledge.IsEmpty() && FPFProgressionTransactions::AvailablePoints(R,*C)==15 && FPFProgressionTransactions::AvailableStatPoints(R)==1);
    TestTrue(TEXT("Technology purchase leaves stat budget"),FPFProgressionTransactions::Purchase(R,TEXT("Tech_FieldTools"),*C,*Craft,*Items,Error));
    TestTrue(TEXT("Separate spending"),FPFProgressionTransactions::AvailablePoints(R,*C)==13 && FPFProgressionTransactions::AvailableStatPoints(R)==1);
    const auto Before=R;TestFalse(TEXT("Invalid enum is atomic"),FPFProgressionTransactions::PurchaseStat(R,static_cast<EPFPlayerStat>(255),*C,*Craft,*Items,Error));TestTrue(TEXT("Invalid preserves allocations"),R.Stats==Before.Stats && R.Knowledge==Before.Knowledge);
    TestTrue(TEXT("Last point allocated"),FPFProgressionTransactions::PurchaseStat(R,EPFPlayerStat::Stamina,*C,*Craft,*Items,Error));
    TestFalse(TEXT("No overspending"),FPFProgressionTransactions::PurchaseStat(R,EPFPlayerStat::Weight,*C,*Craft,*Items,Error));TestEqual(TEXT("Stamina derived from ranks"),FPFProgressionTransactions::StaminaLimit(R),140.f);
    auto Bad=R;Bad.Stats.Attack=-1;TestFalse(TEXT("Negative rank rejected"),FPFProgressionTransactions::Validate(Bad,*C,*Craft,*Items,Error));Bad=R;Bad.Stats.Weight=MAX_int32;TestFalse(TEXT("Overflow-safe bounded rank validation"),FPFProgressionTransactions::Validate(Bad,*C,*Craft,*Items,Error));Bad=R;Bad.Experience=100;TestFalse(TEXT("Ranks cannot exceed earned levels"),FPFProgressionTransactions::Validate(Bad,*C,*Craft,*Items,Error));
    TArray<uint8> Bytes;FPFProgressionRecord Out;
    TestTrue(TEXT("V3 stat roundtrip"),FPFProgressionSaveFormat::Encode(R,*C,*Craft,*Items,Bytes,Error) && Bytes[4]==3 && FPFProgressionSaveFormat::Decode(Bytes,*C,*Craft,*Items,Out,Error));TestTrue(TEXT("Exact ranks/knowledge/XP"),Out.Stats==R.Stats && Out.Knowledge==R.Knowledge && Out.Experience==R.Experience);
    for(bool Windows:{false,true})
    {
        FPFProgressionRecord Old;Old.Experience=1000;if(Windows){Old.GatherWindows={{FPFProgressionTransactions::GatherCategoryForItem(TEXT("Item_Wood")),1,100}};}
        TestTrue(TEXT("Zero allocations retain V1/V2 writer compatibility"),FPFProgressionSaveFormat::Encode(Old,*C,*Craft,*Items,Bytes,Error) && Bytes[4]==(Windows?2:1));
        TestTrue(TEXT("Legacy gets zero ranks without retrospective spending"),FPFProgressionSaveFormat::Decode(Bytes,*C,*Craft,*Items,Out,Error) && Out.Stats==FPFPlayerStatRanks{} && FPFProgressionTransactions::AvailableStatPoints(Out)==5);
    }
    FPFProgressionSaveFormat::Encode(R,*C,*Craft,*Items,Bytes,Error);Bytes.Last()=255;
    const uint32 CRC=FCrc::MemCrc32(Bytes.GetData()+16,Bytes.Num()-16);for(int32 N=0;N<4;++N){Bytes[12+N]=uint8(CRC>>(8*N));}
    const auto Sentinel=Out;TestFalse(TEXT("Even checksummed invalid ranks refuse"),FPFProgressionSaveFormat::Decode(Bytes,*C,*Craft,*Items,Out,Error));TestTrue(TEXT("Failed decode preserves output"),Out.Stats==Sentinel.Stats && Out.Experience==Sentinel.Experience);
    AddInfo(TEXT("[PrimalAgentTools] Independent level-earned stat budget, invalid ranks, V1/V2 defaults and V3 checked"));return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFStatRuntimeTest,"PF.Progression.StatRuntime",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFStatRuntimeTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFStats/")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT("/L_Automation")),nullptr,REN_DontCreateRedirectors | REN_NonTransactional);
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto Box=[&](FVector At,FVector Extent){auto* A=W->SpawnActor<AActor>();auto* B=NewObject<UBoxComponent>(A);A->SetRootComponent(B);B->SetBoxExtent(Extent);B->SetCollisionProfileName(TEXT("BlockAll"));B->RegisterComponent();A->SetActorLocation(At);return A;};
    Box(FVector(0,0,-25),FVector(8000,8000,25));W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);W->SpawnActor<APFWorldClock>();if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();auto* Other=W->SpawnActor<APFSurvivalPlayerController>();auto* GM=W->GetAuthGameMode<APFSurvivalGameMode>();GM->RestartPlayer(PC);GM->RestartPlayer(Other);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();if(!P || !PS || !PS->Progression || !Other->GetPawn()){return false;}
    Other->GetPawn()->SetActorLocation(FVector(-1500,1000,100));auto* G=PS->Progression.Get();auto* I=PS->Inventory.Get();auto* C=PS->Crafting.Get();auto* V=P->Survival.Get();FString Error;
    V->HungerDrainPerSecond=0;V->ThirstDrainPerSecond=0;V->StaminaRecoveryPerSecond=0;
    auto Tick=[&](int32 Count){for(int32 N=0;N<Count;++N){F.TickTestWorld(.1f);}};Tick(10);
    TestFalse(TEXT("No free fresh allocation"),G->RequestStat(EPFPlayerStat::Attack,P,Error));Tick(4);
    FPFProgressionRecord Seed;Seed.Experience=1000;TestTrue(TEXT("Explicit trusted XP fixture, not earned pacing"),G->Restore(Seed,Error));
    PC->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Non-authoritative controller cannot spend authoritative state"),G->RequestStat(EPFPlayerStat::Attack,P,Error));PC->SetRole(ROLE_Authority);
    TestFalse(TEXT("Foreign pawn cannot spend your points"),G->RequestStat(EPFPlayerStat::Stamina,Other->GetPawn(),Error));
    PC->ServerUpgradeStat(EPFPlayerStat::Stamina);TestTrue(TEXT("Owned RPC adds capacity without free refill"),G->GetRecord().Stats.Stamina==1 && V->GetVitals().MaxStamina==120 && V->GetVitals().Stamina==100);
    PC->ServerUpgradeStat(EPFPlayerStat::Stamina);TestEqual(TEXT("Duplicate rate-limited"),G->GetRecord().Stats.Stamina,1);Tick(4);
    const int32 Wood=FMath::CeilToInt(35.f/I->Definition(TEXT("Item_Wood"))->Weight);TestTrue(TEXT("Trusted carry-boundary bag fixture"),I->Grant(TEXT("Item_Wood"),Wood));
    TestFalse(TEXT("35kg initially cannot sprint/jump"),P->Traversal()->CanSprint() || P->Traversal()->CanCarryJump());
    PC->ServerUpgradeStat(EPFPlayerStat::Weight);TestTrue(TEXT("Weight upgrade changes actual traversal threshold"),I->WeightLimit==40 && P->Traversal()->CanSprint() && P->Traversal()->CanCarryJump());Tick(4);
    PC->ServerUpgradeStat(EPFPlayerStat::Attack);Tick(4);PC->ServerUpgradeStat(EPFPlayerStat::CraftingSpeed);Tick(4);
    TestTrue(TEXT("Stat spending leaves tech pool untouched"),G->GetAvailablePoints()==15 && G->GetAvailableStatPoints()==1 && G->GetExperience()==1000);
    I->RemoveItem(TEXT("Item_Wood"),Wood);I->Grant(TEXT("Item_Wood"),6);I->Grant(TEXT("Item_Stone"),4);
    P->GetCharacterMovement()->DisableMovement();PC->SetControlRotation(FRotator::ZeroRotator);FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);
    auto* Creature=W->SpawnActorDeferred<APFCreature>(APFCreature::StaticClass(),FTransform(Eye+Look.Vector()*150),nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    Creature->CreatureId=TEXT("Creature_Prowler");Creature->FinishSpawning(FTransform(Eye+Look.Vector()*150));Creature->GetCharacterMovement()->DisableMovement();Creature->SetActorTickEnabled(false);
    const float Before=Creature->Health;PC->ServerAttackCreature();TestTrue(TEXT("Real server trace damage includes attack rank"),FMath::IsNearlyEqual(Creature->Health,Before-22));Creature->Destroy();
    const auto* Recipe=C->Catalog->Recipe(TEXT("Recipe_Tool"),I->Catalog);
    TestTrue(TEXT("Real crafting starts"),C->Start(TEXT("Recipe_Tool"),P));const double Remaining=C->FinishAt-UPFInventoryComponent::ServerTime(W);
    TestTrue(TEXT("Real finish deadline uses speed"),FMath::IsNearlyEqual(Remaining,double(Recipe->Duration/1.1f),.001));
    PC->ServerUpgradeStat(EPFPlayerStat::CraftingSpeed);TestTrue(TEXT("Running craft deadline unchanged by later rank"),FMath::IsNearlyEqual(C->FinishAt-UPFInventoryComponent::ServerTime(W),Remaining,.001));
    Tick(FMath::CeilToInt(Remaining*10)+3);TestEqual(TEXT("Single actual output"),I->Count(TEXT("Item_Tool")),1);
    const auto Allocations=G->GetRecord().Stats;const auto Feedback=G->GetStatFeedback();PS->SetRole(ROLE_AutonomousProxy);
    TestFalse(TEXT("Client cannot allocate directly"),G->RequestStat(EPFPlayerStat::Attack,P,Error));TestFalse(TEXT("Client cannot restore ranks"),G->Restore({},Error));TestEqual(TEXT("Client cannot forge feedback"),G->GetStatFeedback(),Feedback);PS->SetRole(ROLE_Authority);
    // Standalone intentionally adopts its sole saved owner on login. The foreign
    // pawn above is an authority probe, not a second standalone save identity.
    // Real independent owners/restarts are verified by the multiplayer fixture.
    Other->GetPawn()->Destroy();Other->Destroy();
    auto* Store=W->GetSubsystem<UPFWorldPersistence>();Store->Login(PC,TEXT(""));
    V->ChangeStamina(120-V->GetVitals().Stamina);const FString Slot=TEXT("AutomationStats_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    for(auto* Controller:{PC})
    {
        FPFPlayerSaveData Probe;
        if(!FPFPlayerSaveAdapter::Capture(Controller,Probe,Error) || !FPFPlayerSaveAdapter::CanRestore(Controller,Probe,0,Error))
        {AddError(FString::Printf(TEXT("Stats fixture unsafe %s at %s: %s"),Controller==PC?TEXT("owner"):TEXT("other"),*Controller->GetPawn()->GetActorLocation().ToString(),*Error));return false;}
    }
    ON_SCOPE_EXIT{Store->ActiveSlot.Reset();for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,B));}};
    if(!TestTrue(TEXT("Real save accepts upgraded120 stamina"),Store->Save(Slot,Error))){AddError(Error);return false;}
    G->Restore({},Error);
    if(!TestTrue(TEXT("Real load derives capacity before restoring120 stamina"),Store->Load(Slot,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Actual allocations/vitals/weight restore"),G->GetRecord().Stats==Allocations && V->GetVitals().MaxStamina==120 && V->GetVitals().Stamina==120 && I->WeightLimit==40);
    TestTrue(TEXT("Second load cannot duplicate points/items"),Store->Load(Slot,Error) && G->GetAvailableStatPoints()==0 && I->Count(TEXT("Item_Tool"))==1);
    V->SetHealth(0);Tick(35);auto* Replacement=Cast<APFSurvivorCharacter>(PC->GetPawn());
    TestTrue(TEXT("Actual respawn retains ranks and fills upgraded capacity"),Replacement && Replacement!=P && PS==PC->PlayerState && G->GetRecord().Stats==Allocations && Replacement->Survival->GetVitals().MaxStamina==120 && Replacement->Survival->GetVitals().Stamina==120);
    if(Replacement)
    {
        PC->SetAsLocalPlayerController();
        Box(FVector(220,3000,500),FVector(150,180,500));Replacement->Survival->StaminaRecoveryPerSecond=0;
        Replacement->SetActorLocation(FVector(-200,3000,100));Replacement->SetActorRotation(FRotator::ZeroRotator);PC->SetControlRotation(FRotator::ZeroRotator);
        Replacement->Traversal()->SetMovementMode(MOVE_Falling);
        for(int32 N=0;N<5;++N){F.TickTestWorld(.025f);}
        for(int32 N=0;N<45;++N){Replacement->AddMovementInput(FVector::ForwardVector);F.TickTestWorld(.025f);}
        const float Half=Replacement->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();const double Height=1000-(Replacement->GetActorLocation().Z-Half);
        const bool Climbed=Replacement->TryClimb(Error);AddInfo(FString::Printf(TEXT("[PrimalAgentTools]10m fixture at%s stamina%.1f grounded%d accepted%d: %s"),*Replacement->GetActorLocation().ToString(),Replacement->Survival->GetVitals().Stamina,Replacement->Traversal()->IsMovingOnGround(),Climbed,*Error));TestTrue(TEXT("Upgraded capacity affords actual10m climb"),Climbed);bool Landed=false;
        for(int32 N=0;N<220;++N){F.TickTestWorld(.025f);if(Replacement->Traversal()->IsMovingOnGround() && Replacement->GetActorLocation().X>70 && FMath::Abs(Replacement->GetActorLocation().Z-Half-1000)<5 && Replacement->Traversal()->AutomaticClimbDrive.IsNearlyZero()){Landed=true;break;}}
        AddInfo(FString::Printf(TEXT("[PrimalAgentTools]10m final at%s mode%d drive%s"),*Replacement->GetActorLocation().ToString(),int32(Replacement->Traversal()->MovementMode),*Replacement->Traversal()->AutomaticClimbDrive.ToString()));
        TestTrue(TEXT("Rank1 stamina enables physical10m landing"),Landed);TestTrue(TEXT("Full10m cost paid once"),FMath::IsNearlyEqual(Replacement->Survival->GetVitals().Stamina,float(120-15-(Height-120)*.1),.1f));
    }
    F.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] Stat authority, real damage/craft/carry, upgraded save/load and respawn checked"));return true;
}
#endif
