#if WITH_DEV_AUTOMATION_TESTS
#include "Progression/PFProgressionSaveFormat.h"
#include "Progression/PFProgressionCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "Misc/AutomationTest.h"
#include "Misc/Crc.h"
#include <limits>
namespace
{
void ProgressionPutWord(TArray<uint8>& Bytes,int32 Offset,uint32 Word){for(int32 I=0;I<4;++I){Bytes[Offset+I]=static_cast<uint8>(Word>>(I*8));}}
void ProgressionRefresh(TArray<uint8>& Bytes){ProgressionPutWord(Bytes,8,Bytes.Num()-16);ProgressionPutWord(Bytes,12,FCrc::MemCrc32(Bytes.GetData()+16,Bytes.Num()-16));}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFProgressionCodecTest,"PF.Progression.Codec",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFProgressionCodecTest::RunTest(const FString&)
{
    auto* Catalog=NewObject<UPFProgressionCatalog>();auto* Crafting=NewObject<UPFCraftingCatalog>();auto* Items=NewObject<UPFItemCatalog>();FString Error;
    FPFProgressionRecord Original;Original.Experience=100;Original.Knowledge={FName(TEXT("Tech_FieldTools"))};Original.CreditedCrafts={FName(TEXT("Recipe_Tool"))};TArray<uint8> Bytes;
    if(!TestTrue(TEXT("Encode bounded valid record"),FPFProgressionSaveFormat::Encode(Original,*Catalog,*Crafting,*Items,Bytes,Error))){return false;}FPFProgressionRecord Decoded;
    TestTrue(TEXT("Decode native progression envelope"),FPFProgressionSaveFormat::Decode(Bytes,*Catalog,*Crafting,*Items,Decoded,Error));TestEqual(TEXT("Exact XP"),Decoded.Experience,100);
    TestTrue(TEXT("Exact knowledge/craft identity arrays"),Decoded.Knowledge==Original.Knowledge && Decoded.CreditedCrafts==Original.CreditedCrafts);TestEqual(TEXT("No reaward on read"),FPFProgressionTransactions::AvailablePoints(Decoded,*Catalog),1);
    TestFalse(TEXT("Decoded completed craft cannot reward again"),FPFProgressionTransactions::CreditFirstCraft(Decoded,TEXT("Recipe_Tool"),*Catalog,*Crafting,*Items,Error));
    const auto Sentinel=Decoded;
    auto Refuse=[&](const TArray<uint8>& Bad,const TCHAR* Why){TestFalse(Why,FPFProgressionSaveFormat::Decode(Bad,*Catalog,*Crafting,*Items,Decoded,Error));TestTrue(TEXT("Failed read preserves all outputs"),Decoded.Experience==Sentinel.Experience && Decoded.Knowledge==Sentinel.Knowledge && Decoded.CreditedCrafts==Sentinel.CreditedCrafts);};
    auto Bad=Bytes;ProgressionPutWord(Bad,4,FPFProgressionSaveFormat::CurrentVersion+1);Refuse(Bad,TEXT("Future version"));Bad=Bytes;Bad[0]^=1;Refuse(Bad,TEXT("Signature"));Bad=Bytes;Bad.Last()^=1;Refuse(Bad,TEXT("Checksum"));
    Bad=Bytes;Bad.Pop();Refuse(Bad,TEXT("Truncated length"));Bad=Bytes;Bad.Add(1);ProgressionRefresh(Bad);Refuse(Bad,TEXT("Trailing bytes with valid envelope"));
    Bad=Bytes;ProgressionPutWord(Bad,16,2701);ProgressionRefresh(Bad);Refuse(Bad,TEXT("CRC-valid invalid XP"));
    Bad=Bytes;ProgressionPutWord(Bad,20,std::numeric_limits<uint32>::max());ProgressionRefresh(Bad);Refuse(Bad,TEXT("Negative count"));
    Bad=Bytes;ProgressionPutWord(Bad,20,33);ProgressionRefresh(Bad);Refuse(Bad,TEXT("Oversized knowledge count"));
    Bad=Bytes;ProgressionPutWord(Bad,24,65);ProgressionRefresh(Bad);Refuse(Bad,TEXT("Oversized ID length"));
    Bad=Bytes;Bad[28]=TEXT('X');ProgressionRefresh(Bad);Refuse(Bad,TEXT("Unknown ID without interning"));Bad=Bytes;Bad[28]=255;ProgressionRefresh(Bad);Refuse(Bad,TEXT("NonASCII ID"));
    Bad.Init(0,FPFProgressionSaveFormat::MaximumBytes+1);Refuse(Bad,TEXT("File allocation bound"));
    // Forge structurally valid duplicate knowledge with a correct CRC to exercise semantic refusal.
    Bad=Bytes;const int32 KnownLength=Original.Knowledge[0].ToString().Len();TArray<uint8> Duplicated;Duplicated.Append(Bytes.GetData()+24,4+KnownLength);Bad.Insert(Duplicated,28+KnownLength);ProgressionPutWord(Bad,20,2);ProgressionRefresh(Bad);Refuse(Bad,TEXT("Duplicate encoded knowledge"));
    auto Invalid=Original;Invalid.Knowledge={FName(TEXT("Tech_Unknown"))};auto OutBytes=Bytes;TestFalse(TEXT("Invalid write refused"),FPFProgressionSaveFormat::Encode(Invalid,*Catalog,*Crafting,*Items,OutBytes,Error));TestTrue(TEXT("Failed write preserves bytes"),OutBytes==Bytes);
    FPFPlayerSaveData Legacy;Legacy.PlayerId=FGuid(1,2,3,4);Legacy.Location=FVector(125,-200,90);Legacy.Health=65;Legacy.Stamina=42;Legacy.Hunger=24;Legacy.Thirst=73;
    Legacy.Inventory={{FGuid(5,6,7,8),TEXT("Item_BoundTool"),1,0},{FGuid(9,10,11,12),TEXT("Item_Food"),2,12.5}};
    TArray<uint8> LegacyBytes;FPFPlayerSaveLimits Limits;if(!TestTrue(TEXT("Original V1 encode"),FPFPlayerSaveFormat::Encode(Legacy,*Items,Limits,LegacyBytes,Error))){return false;}const auto BeforeBytes=LegacyBytes;
    FPFPlayerSaveData Migrated;FPFProgressionRecord Defaults=Original;
    if(!TestTrue(TEXT("Validated legacy defaults"),FPFProgressionSaveFormat::DecodeLegacyPlayer(LegacyBytes,*Items,Limits,*Catalog,*Crafting,Migrated,Defaults,Error))){return false;}
    TestTrue(TEXT("Original bytes untouched"),LegacyBytes==BeforeBytes);TestEqual(TEXT("Stable ID preserved"),Migrated.PlayerId,Legacy.PlayerId);TestEqual(TEXT("Position preserved"),Migrated.Location,Legacy.Location);
    TestTrue(TEXT("Vitals preserved"),Migrated.Health==65 && Migrated.Stamina==42 && Migrated.Hunger==24 && Migrated.Thirst==73);
    TestEqual(TEXT("Inventory identity preserved"),Migrated.Inventory[0].StackId,Legacy.Inventory[0].StackId);TestEqual(TEXT("Existing tool preserved"),Migrated.Inventory[0].ItemId,FName(TEXT("Item_BoundTool")));TestEqual(TEXT("Freshness never refreshed"),Migrated.Inventory[1].RemainingFreshnessSeconds,12.5);
    TestTrue(TEXT("No retrospective item/craft/point rewards"),Defaults.Experience==0 && Defaults.Knowledge.IsEmpty() && Defaults.CreditedCrafts.IsEmpty());
    auto CorruptLegacy=LegacyBytes;CorruptLegacy.Last()^=1;Defaults=Original;const auto MigratedBefore=Migrated;
    TestFalse(TEXT("Corrupt legacy refuses defaults"),FPFProgressionSaveFormat::DecodeLegacyPlayer(CorruptLegacy,*Items,Limits,*Catalog,*Crafting,Migrated,Defaults,Error));TestTrue(TEXT("Both legacy outputs preserved on refusal"),Migrated.PlayerId==MigratedBefore.PlayerId && Migrated.Inventory[0].ItemId==MigratedBefore.Inventory[0].ItemId && Defaults.Experience==Original.Experience && Defaults.Knowledge==Original.Knowledge);
    TestTrue(TEXT("Repeated validated legacy read"),FPFProgressionSaveFormat::DecodeLegacyPlayer(LegacyBytes,*Items,Limits,*Catalog,*Crafting,Migrated,Defaults,Error));TestEqual(TEXT("Repeated defaults never reward"),Defaults.Experience,0);
    AddInfo(TEXT("[PrimalAgentTools] Separate bounded codec and validated V1 zero-XP defaults checked; no world-save writer/player hooks integrated."));return true;
}
#endif
