#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureCatalog.h"
#include "Inventory/PFItemPickup.h"
#include "Survival/PFSurvivalGameMode.h"
#include "PFAssetPaths.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Inventory/PFItemCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Building/PFBuildingCatalog.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeExit.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCreatureVarietyTest,"PF.Creatures.Variety",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFCreatureVarietyTest::RunTest(const FString&)
{
    auto* Catalog=LoadObject<UPFCreatureCatalog>(nullptr,PFAssetPaths::CreatureCatalog);
    if(!TestNotNull(TEXT("Actual catalog"),Catalog)){return false;}
    const auto* A=Catalog->Find(TEXT("Creature_Scrabbler"));const auto* B=Catalog->Find(TEXT("Creature_DuskPouncer"));
    if(!TestNotNull(TEXT("Loaded passive variety"),A) || !TestNotNull(TEXT("Loaded hostile variety"),B)){return false;}
    TestTrue(TEXT("Distinct bounded behavior and statistics"),!A->bHostile && B->bHostile && A->Health==40 && B->Health==140 && A->Speed==260 && B->Damage==12);
    auto* Bad=NewObject<UPFCreatureCatalog>();Bad->Creatures[2].ExtraLoot.Add({TEXT("Item_Fibre"),1});
    TestNull(TEXT("Duplicate supplemental loot refused"),Bad->Find(A->Id));
    Bad->Creatures[2].ExtraLoot={{TEXT("Item_Fibre"),11}};TestNull(TEXT("Excess supplemental quantity refused"),Bad->Find(A->Id));
    Bad->Creatures[2].ExtraLoot={{TEXT("Item_Food"),1}};TestNull(TEXT("Legacy food double entry refused"),Bad->Find(A->Id));
    auto* LootItems=LoadObject<UPFItemCatalog>(nullptr,PFAssetPaths::ItemCatalog);
    if(!TestNotNull(TEXT("Actual loot item catalog"),LootItems)){return false;}
    Bad->Creatures[2].ExtraLoot={{TEXT("Item_UnknownLoot"),1}};TestFalse(TEXT("Unknown loot reference refused before spawning"),Bad->HasValidLoot(A->Id,*LootItems));
    Bad->Creatures[2].ExtraLoot={{TEXT("Item_Tool"),2}};TestFalse(TEXT("Nonstackable loot quantity refused before spawning"),Bad->HasValidLoot(A->Id,*LootItems));
    TestTrue(TEXT("Actual variant loot references and stack bounds valid"),Catalog->HasValidLoot(A->Id,*LootItems) && Catalog->HasValidLoot(B->Id,*LootItems));
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    if(!F.BeginPlayInTestWorld()){return false;}
    int32 ExpectedFood=0,ExpectedFibre=0;
    for(const auto* D:{A,B})
    {
        const FTransform T(FVector(0,ExpectedFood*100,100));auto* C=W->SpawnActorDeferred<APFCreature>(APFCreature::StaticClass(),T);
        C->CreatureId=D->Id;C->Catalog=Catalog;C->FinishSpawning(T);C->GetCharacterMovement()->DisableMovement();
        TestEqual(TEXT("New creature actual health"),C->Health,D->Health);TestEqual(TEXT("New creature actual walk speed"),C->GetCharacterMovement()->MaxWalkSpeed,D->Speed);
        C->TakeDamage(1000,FDamageEvent(),nullptr,nullptr);TestTrue(TEXT("Authoritative death"),C->IsDead());
        int32 Before=0;for(TActorIterator<APFItemPickup> It(W);It;++It){++Before;}
        C->TakeDamage(1000,FDamageEvent(),nullptr,nullptr);
        int32 After=0;for(TActorIterator<APFItemPickup> It(W);It;++It){++After;}
        TestEqual(TEXT("Repeated corpse damage does not emit loot twice"),After,Before);
        ExpectedFood+=D->FoodLoot;ExpectedFibre+=D->ExtraLoot[0].Quantity;
    }
    int32 Food=0,Fibre=0,Pickups=0;TSet<FGuid> Ids;
    for(TActorIterator<APFItemPickup> It(W);It;++It)
    {
        const auto& S=It->GetContents();++Pickups;TestTrue(TEXT("Unique conserved loot batch"),S.StackId.IsValid() && !Ids.Contains(S.StackId));Ids.Add(S.StackId);
        if(S.ItemId==TEXT("Item_Food")){Food+=S.Quantity;TestTrue(TEXT("Food has real expiry"),S.ExpiresAt>0);}else if(S.ItemId==TEXT("Item_Fibre")){Fibre+=S.Quantity;TestEqual(TEXT("Fibre is nonperishable"),S.ExpiresAt,0.);}
    }
    TestEqual(TEXT("Four bounded pickups"),Pickups,4);TestEqual(TEXT("Exact food payout"),Food,ExpectedFood);TestEqual(TEXT("Exact supplemental fibre payout"),Fibre,ExpectedFibre);
    FPFWorldSaveData Records;Records.Map=TEXT("L_Automation");
    for(const auto* D:{A,B}){FPFCreatureSaveRecord R;R.Id=FGuid::NewGuid();R.Definition=D->Id.ToString();R.Health=0;R.CorpseSeconds=12;Records.Creatures.Add(R);}
    const auto* Items=GetDefault<UPFItemCatalog>();const auto* Buildings=GetDefault<UPFBuildingCatalog>();const auto* Crafting=GetDefault<UPFCraftingCatalog>();
    FString Error;TArray<uint8> Bytes;FPFWorldSaveData Decoded;
    TestTrue(TEXT("New IDs accepted by actual world validator"),FPFWorldSaveFormat::Validate(Records,*Items,*Buildings,*Crafting,*Catalog,Error));
    TestTrue(TEXT("New creature records encode"),FPFWorldSaveFormat::Encode(Records,Bytes,Error));
    const FString Slot=TEXT("AutomationVariety_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT{for(bool Second:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,Second));}};
    TestTrue(TEXT("Actual unique creature save file write"),FPFSaveFileStore::Write(Slot,Bytes,Error));FPFSavedFile File;
    TestTrue(TEXT("Actual creature save file read"),FPFSaveFileStore::Read(Slot,File,Error));
    TestTrue(TEXT("Actual file validates and decodes new identities"),FPFWorldSaveFormat::DecodeValidated(File.Payload,*Items,*Buildings,*Crafting,*Catalog,Decoded,Error));
    TestTrue(TEXT("Both new creature identities and dead state preserved"),Decoded.Creatures.Num()==2 && Decoded.Creatures[0].Id==Records.Creatures[0].Id && Decoded.Creatures[1].Definition==B->Id.ToString() && Decoded.Creatures[1].Health==0);
    AddInfo(TEXT("[PrimalAgentTools] Loaded original creature variety, authority, bounded loot and duplicate death refusal verified"));return true;
}
#endif
