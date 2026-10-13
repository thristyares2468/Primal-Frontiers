#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Persistence/PFResourceLayoutTransform.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Progression/PFProgressionRecord.h"
#include "Progression/PFProgressionCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "Building/PFBuildingCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Creatures/PFCreatureCatalog.h"
#include "Misc/AutomationTest.h"
#include "Misc/DateTime.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFResourceLayoutTest,"PF.Persistence.ResourceLayoutTransform",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFResourceLayoutTest::RunTest(const FString&)
{
    auto* Items=NewObject<UPFItemCatalog>();auto* Buildings=NewObject<UPFBuildingCatalog>();
    auto* Crafting=NewObject<UPFCraftingCatalog>();auto* Creatures=NewObject<UPFCreatureCatalog>();const auto* Progression=GetDefault<UPFProgressionCatalog>();
    FString Error;FPFWorldSaveData Source;Source.Version=2;Source.Map=TEXT("L_PrimalFrontier_OpenWorld");Source.Hour=21.4f;
    FPFPlayerSaveData Player;Player.PlayerId=FGuid(1,2,3,4);Player.Health=33;Player.Stamina=112;Player.Hunger=47;Player.Thirst=27;
    Player.Inventory.Add({FGuid(5,6,7,8),TEXT("Item_Food"),2,120});
    FPFWorldPlayerRecord Owner;Owner.PlayerId=Player.PlayerId;Owner.ReconnectCredential=FGuid(9,10,11,12);Owner.CapturedUtc=FDateTime::UtcNow().ToUnixTimestamp();
    FPFProgressionRecord XP;XP.Experience=100;XP.Stats.Stamina=1;
    if(!TestTrue(TEXT("Encode upgraded owner inventory/vitals"),FPFWorldSaveFormat::PackPlayer(Player,*Items,30,Owner.Data,Error,false,120)) ||
       !TestTrue(TEXT("Encode independent spent stat record"),FPFWorldSaveFormat::PackProgression(Owner.PlayerId,XP,*Progression,*Crafting,*Items,Owner.Progression,Error))){return false;}
    Source.Players.Add(Owner);
    FPFStructureSaveRecord Base;Base.Id=FGuid(20,20,20,20);Base.Owner=Player.PlayerId;Base.Definition=TEXT("Build_Foundation");Base.Health=100;
    FPFPlayerSaveData Empty;Empty.PlayerId=Base.Id;FPFWorldSaveFormat::PackPlayer(Empty,*Items,60,Base.Storage,Error);Source.Structures.Add(Base);
    FPFStructureSaveRecord Chest;Chest.Id=FGuid(21,21,21,21);Chest.Owner=Player.PlayerId;Chest.Support=Base.Id;Chest.Definition=TEXT("Build_Storage");Chest.Health=75;
    Chest.StorageRestricted=true;Chest.AllowedStorageItems={TEXT("Item_Wood")};FPFPlayerSaveData Stored;Stored.PlayerId=Chest.Id;Stored.Inventory.Add({FGuid(22,22,22,22),TEXT("Item_Wood"),7,0});
    FPFWorldSaveFormat::PackPlayer(Stored,*Items,60,Chest.Storage,Error);Source.Structures.Add(Chest);
    FPFResourceSaveRecord Wood;Wood.Name=TEXT("OldWood");Wood.Definition=TEXT("Node_Wood");Wood.Hits=0;Wood.Respawn=12;Source.Resources.Add(Wood);
    FPFResourceSaveRecord Stone;Stone.Name=TEXT("OldStone");Stone.Definition=TEXT("Node_Stone");Stone.Hits=2;Source.Resources.Add(Stone);
    const TArray<FPFResourceLayoutEntry> Previous={{TEXT("OldStone"),TEXT("Node_Stone")},{TEXT("OldWood"),TEXT("Node_Wood")}};
    const TArray<FPFResourceLayoutEntry> Added={{TEXT("AddedStone"),TEXT("Node_Stone")},{TEXT("AddedWater"),TEXT("Node_Water")}};
    TArray<uint8> Before;if(!TestTrue(TEXT("Whole source valid"),FPFWorldSaveFormat::Validate(Source,*Items,*Buildings,*Crafting,*Creatures,Error,Progression))){AddError(Error);return false;}
    FPFWorldSaveFormat::Encode(Source,Before,Error);FPFWorldSaveData Out;
    auto Prepare=[&](const auto& Old,const auto& New,const FPFWorldSaveData& S,FPFWorldSaveData& O){return FPFResourceLayoutTransform::Prepare(TEXT("L_PrimalFrontier_OpenWorld"),Old,New,S,*Items,*Buildings,*Crafting,*Creatures,*Progression,O,Error);};
    if(!TestTrue(TEXT("Exact manifest appends trusted defaults"),Prepare(Previous,Added,Source,Out))){AddError(Error);return false;}
    TestTrue(TEXT("Old depletion and active hits preserved"),Out.Resources[0].Hits==0 && Out.Resources[0].Respawn==12 && Out.Resources[1].Hits==2);
    TestTrue(TEXT("New defaults are bounded full nodes without respawn"),Out.Resources.Num()==4 && Out.Resources[2].Hits==3 && Out.Resources[2].Respawn==0 && Out.Resources[3].Hits==3);
    auto WithoutAdditions=Out;WithoutAdditions.Resources.SetNum(Source.Resources.Num());TArray<uint8> Conserved;FPFWorldSaveFormat::Encode(WithoutAdditions,Conserved,Error);
    TestTrue(TEXT("All original bytes/owner/stats/food/storage/support/time conserved"),Conserved==Before);
    TArray<uint8> Encoded;FPFWorldSaveFormat::Encode(Out,Encoded,Error);FPFWorldSaveData Decoded;
    TestTrue(TEXT("Whole migrated record decodes and validates"),FPFWorldSaveFormat::DecodeValidated(Encoded,*Items,*Buildings,*Crafting,*Creatures,Decoded,Error,Progression) && Decoded.Resources.Num()==4);
    auto Refused=[&](const auto& Old,const auto& New,const FPFWorldSaveData& S,const TCHAR* Name)
    {FPFWorldSaveData Destination=Out;TestFalse(Name,Prepare(Old,New,S,Destination));TArray<uint8> After;FPFWorldSaveFormat::Encode(Destination,After,Error);TestTrue(TEXT("Failed transform preserves destination bytes"),After==Encoded);};
    Refused(Previous,Added,Out,TEXT("Repeated addition cannot duplicate nodes"));
    auto Wrong=Previous;Wrong[0].Definition=TEXT("Node_Wood");Refused(Wrong,Added,Source,TEXT("Definition substitution refuses"));
    Wrong=Previous;Wrong[0].Name=TEXT("RenamedStone");Refused(Wrong,Added,Source,TEXT("No proximity/order/name guessing"));
    Wrong=Previous;const auto Duplicate=Wrong[0];Wrong.Add(Duplicate);Refused(Wrong,Added,Source,TEXT("Duplicate prior manifest refuses"));
    auto New=Added;New[0].Name=TEXT("OldWood");Refused(Previous,New,Source,TEXT("Cannot replace existing node"));
    New=Added;New[1].Name=New[0].Name;Refused(Previous,New,Source,TEXT("Duplicate additions refuse"));
    New=Added;New[0].Definition=TEXT("Unknown_Node");Refused(Previous,New,Source,TEXT("Unknown data not interned/created"));
    New=Added;New[0].Name=TEXT("../escape");Refused(Previous,New,Source,TEXT("Unsafe identity refuses"));
    New=Added;while(New.Num()<17){New.Add({FString::Printf(TEXT("Extra%d"),New.Num()),TEXT("Node_Stone")});}Refused(Previous,New,Source,TEXT("Addition batch bounded"));
    auto Bad=Source;Bad.Version=999;Refused(Previous,Added,Bad,TEXT("Future source version refuses"));
    Bad=Source;Bad.Resources[0].Respawn=999;Refused(Previous,Added,Bad,TEXT("Invalid depletion refuses before addition"));
    Bad=Source;Bad.Map=TEXT("L_Automation");Refused(Previous,Added,Bad,TEXT("Different approved map cannot borrow this edge"));
    Bad=Source;Bad.Players[0].Progression=TEXT("corrupt");Refused(Previous,Added,Bad,TEXT("Whole upgraded-player validation required"));
    TestFalse(TEXT("Aliased source/destination refuses"),Prepare(Previous,Added,Source,Source));
    TArray<uint8> SourceAfter;FPFWorldSaveFormat::Encode(Source,SourceAfter,Error);TestTrue(TEXT("Source remains exact after every attempt"),SourceAfter==Before);
    AddInfo(TEXT("[PrimalAgentTools] ResourceLayoutTransform: whole record + exact trusted manifest, old depleted states/owner/upgraded vitals/food/chest/support/time conserved, default additions, roundtrip and atomic repeat/identity/data/map/version/bounds refusal. Offline synthetic edge only; no live mapping or private migration."));return true;
}
#endif
