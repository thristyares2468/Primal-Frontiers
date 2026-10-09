#if WITH_DEV_AUTOMATION_TESTS
#include "Progression/PFProgressionRecord.h"
#include "Progression/PFProgressionSaveFormat.h"
#include "Progression/PFProgressionCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "Misc/AutomationTest.h"
#include "Misc/Crc.h"
#include <limits>
namespace
{
bool SameGatherRecord(const FPFProgressionRecord& A,const FPFProgressionRecord& B)
{return A.Experience==B.Experience && A.Knowledge==B.Knowledge && A.CreditedCrafts==B.CreditedCrafts && A.GatherWindows==B.GatherWindows;}
void GatherPutWord(TArray<uint8>& Bytes,int32 At,uint32 Word){for(int32 I=0;I<4;++I){Bytes[At+I]=static_cast<uint8>(Word>>(I*8));}}
void GatherCRC(TArray<uint8>& Bytes){GatherPutWord(Bytes,8,Bytes.Num()-16);GatherPutWord(Bytes,12,FCrc::MemCrc32(Bytes.GetData()+16,Bytes.Num()-16));}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFGatherRewardWindowsTest,"PF.Progression.GatherWindows",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFGatherRewardWindowsTest::RunTest(const FString&)
{
    auto* Catalog=NewObject<UPFProgressionCatalog>();auto* Crafting=NewObject<UPFCraftingCatalog>();auto* Items=NewObject<UPFItemCatalog>();FString Error;
    const auto Categories=FPFProgressionTransactions::GatherCategories();TestEqual(TEXT("Exactly five native category buckets"),Categories.Num(),5);
    TestFalse(TEXT("Crafted food/tool are not gathered categories"),FPFProgressionTransactions::GatherCategoryForItem(TEXT("Item_CookedFood")).IsValid() || FPFProgressionTransactions::GatherCategoryForItem(TEXT("Item_Tool")).IsValid());
    for(const auto& D:Crafting->Resources){TestTrue(TEXT("Current resource yield maps to an exact native category"),Categories.Contains(FPFProgressionTransactions::GatherCategoryForItem(D.YieldItem)));}
    FPFProgressionRecord R;auto Credit=[&](FGameplayTag Tag){return FPFProgressionTransactions::CreditGather(R,Tag,*Catalog,*Crafting,*Items,Error);};
    auto Advance=[&](double Seconds){return FPFProgressionTransactions::AdvanceGatherWindows(R,Seconds,*Catalog,*Crafting,*Items,Error);};
    TestFalse(TEXT("No unknown category reward"),Credit({}));TestEqual(TEXT("Unknown leaves XP unchanged"),R.Experience,0);
    for(int32 N=0;N<5;++N){TestTrue(TEXT("Five accepted success events reward once each"),Credit(Categories[0]));}
    const auto Exhausted=R;TestEqual(TEXT("Category cap exactly25XP"),R.Experience,25);TestEqual(TEXT("One category only one bounded entry"),R.GatherWindows.Num(),1);
    TestFalse(TEXT("Sixth event cannot farm current category window"),Credit(Categories[0]));TestTrue(TEXT("Exhausted refusal preserves complete record"),SameGatherRecord(R,Exhausted));
    for(double Bad:{-1.,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity()})
    {TestFalse(TEXT("Invalid elapsed refuses atomically"),Advance(Bad));TestTrue(TEXT("No invalid elapsed resets reward"),SameGatherRecord(R,Exhausted));}
    TestTrue(TEXT("Zero elapsed preserves exact record"),Advance(0));TestTrue(TEXT("Still full"),SameGatherRecord(R,Exhausted));
    TestTrue(TEXT("Only active elapsed lowers duration"),Advance(600));TestTrue(TEXT("Second category independent"),Credit(Categories[1]));
    TestTrue(TEXT("Existing window not refreshed by credit in another category"),R.GatherWindows[0].RemainingSeconds==1200 && R.GatherWindows[1].RemainingSeconds==1800);
    TestTrue(TEXT("Just before expiry"),Advance(1199.5));TestFalse(TEXT("No premature reset"),Credit(Categories[0]));
    TestTrue(TEXT("Exact expiry removes only expired bucket"),Advance(.5));TestTrue(TEXT("Second bucket retains600s"),R.GatherWindows.Num()==1 && R.GatherWindows[0].Category==Categories[1] && R.GatherWindows[0].RemainingSeconds==600);
    const int32 XP=R.Experience;TestTrue(TEXT("New window accepts next successful event"),Credit(Categories[0]));TestEqual(TEXT("Only five new XP, no replay"),R.Experience,XP+5);
    TestEqual(TEXT("New window starts full"),R.GatherWindows.Last().RemainingSeconds,1800.);
    TestTrue(TEXT("Long active interval safely expires all"),Advance(std::numeric_limits<double>::max()));TestTrue(TEXT("No lifetime ledger growth"),R.GatherWindows.IsEmpty());
    R={};for(const auto& Tag:Categories){for(int32 N=0;N<5;++N){TestTrue(TEXT("All category event budgets"),Credit(Tag));}}
    TestTrue(TEXT("Five buckets total125XP,level2,threepoints"),R.GatherWindows.Num()==5 && R.Experience==125 && FPFProgressionTransactions::LevelForExperience(R.Experience)==2 && FPFProgressionTransactions::AvailablePoints(R,*Catalog)==3);
    for(const auto& Recipe:Crafting->Recipes){TestTrue(TEXT("Distinct craft reward remains independent"),FPFProgressionTransactions::CreditFirstCraft(R,Recipe.Id,*Catalog,*Crafting,*Items,Error));}
    TestEqual(TEXT("Current catalog reward budgets reach285XP"),R.Experience,285);TestEqual(TEXT("That budget reaches level3"),FPFProgressionTransactions::LevelForExperience(R.Experience),3);
    const auto BeforeCap=R;R.Experience=2699;TestTrue(TEXT("Near-cap credit remains bounded after expired window"),Advance(1800) && Credit(Categories[0]));TestEqual(TEXT("Clamped level10 cap"),R.Experience,2700);const auto AtCap=R;
    TestFalse(TEXT("Cap cannot inflate/reset reward ledger"),Credit(Categories[1]));TestTrue(TEXT("Cap refusal complete atomicity"),SameGatherRecord(R,AtCap));
    auto Invalid=BeforeCap;const auto DuplicateWindow=Invalid.GatherWindows[0];Invalid.GatherWindows.Add(DuplicateWindow);TestFalse(TEXT("Duplicate/oversized windows invalid"),FPFProgressionTransactions::Validate(Invalid,*Catalog,*Crafting,*Items,Error));
    Invalid=BeforeCap;Invalid.Experience=160;TestFalse(TEXT("Recorded successful event rewards require consistentXP"),FPFProgressionTransactions::Validate(Invalid,*Catalog,*Crafting,*Items,Error));

    // Independent literal V1 zero payload: byte-for-byte writer compatibility, not merely roundtrip.
    TArray<uint8> V1={0x50,0x46,0x58,0x50,1,0,0,0,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};GatherCRC(V1);
    FPFProgressionRecord Empty,Decoded=BeforeCap;TArray<uint8> EmptyBytes;
    TestTrue(TEXT("Old progressionV1 decodes with zero windows/no retrocredit"),FPFProgressionSaveFormat::Decode(V1,*Catalog,*Crafting,*Items,Decoded,Error) && SameGatherRecord(Decoded,Empty));
    TestTrue(TEXT("No-window write remains exact originalV1 bytes"),FPFProgressionSaveFormat::Encode(Empty,*Catalog,*Crafting,*Items,EmptyBytes,Error) && EmptyBytes==V1);
    FPFProgressionRecord Saved;Saved.Experience=100;Saved.Knowledge={FName(TEXT("Tech_FieldTools"))};Saved.CreditedCrafts={FName(TEXT("Recipe_Tool"))};TArray<uint8> WithoutWindows;
    TestTrue(TEXT("Known legacy nonzero base fixture"),FPFProgressionSaveFormat::Encode(Saved,*Catalog,*Crafting,*Items,WithoutWindows,Error));
    Saved.GatherWindows={{Categories[0],2,1210.5},{Categories[3],1,20.}};TArray<uint8> Bytes;
    if(!TestTrue(TEXT("WindowV2 encode"),FPFProgressionSaveFormat::Encode(Saved,*Catalog,*Crafting,*Items,Bytes,Error))){AddError(Error);return false;}
    TestEqual(TEXT("Nonempty windows useV2 envelope"),static_cast<int32>(Bytes[4]),2);
    for(int32 N=0;N<3;++N)
    {
        TestTrue(TEXT("Repeated read preserves fractional time/count/XP/knowledge"),FPFProgressionSaveFormat::Decode(Bytes,*Catalog,*Crafting,*Items,Decoded,Error) && SameGatherRecord(Decoded,Saved));
        TArray<uint8> Again;TestTrue(TEXT("Offline/repeated codec cannot advance/refresh reward time"),FPFProgressionSaveFormat::Encode(Decoded,*Catalog,*Crafting,*Items,Again,Error) && Again==Bytes);
    }
    const auto Sentinel=Decoded;auto Refuse=[&](const TArray<uint8>& Bad,const TCHAR* Why){TestFalse(Why,FPFProgressionSaveFormat::Decode(Bad,*Catalog,*Crafting,*Items,Decoded,Error));TestTrue(TEXT("Failed decode preserves every field"),SameGatherRecord(Decoded,Sentinel));};
    const int32 CountAt=WithoutWindows.Num(),TagAt=CountAt+8,TagLength=Categories[0].ToString().Len(),RewardAt=TagAt+TagLength,DurationAt=RewardAt+4;
    auto Bad=Bytes;GatherPutWord(Bad,CountAt,6);GatherCRC(Bad);Refuse(Bad,TEXT("Too many buckets"));Bad=Bytes;GatherPutWord(Bad,CountAt,std::numeric_limits<uint32>::max());GatherCRC(Bad);Refuse(Bad,TEXT("Negative bucket count"));
    Bad=Bytes;GatherPutWord(Bad,CountAt+4,65);GatherCRC(Bad);Refuse(Bad,TEXT("Unbounded tag length"));Bad=Bytes;Bad[TagAt]=TEXT('X');GatherCRC(Bad);Refuse(Bad,TEXT("Unknown category without tag interning"));
    for(uint8 Character:{static_cast<uint8>(0),static_cast<uint8>(255)}){Bad=Bytes;Bad[TagAt]=Character;GatherCRC(Bad);Refuse(Bad,TEXT("NUL/nonASCII category refused"));}
    Bad=Bytes;FMemory::Memcpy(Bad.GetData()+TagAt+16+TagLength,Bytes.GetData()+TagAt,TagLength);GatherCRC(Bad);Refuse(Bad,TEXT("Duplicate exact category"));
    for(uint32 BadCount:{0u,6u,std::numeric_limits<uint32>::max()}){Bad=Bytes;GatherPutWord(Bad,RewardAt,BadCount);GatherCRC(Bad);Refuse(Bad,TEXT("Invalid reward count"));}
    for(double BadTime:{0.,-1.,1800.1,std::numeric_limits<double>::quiet_NaN(),std::numeric_limits<double>::infinity()})
    {Bad=Bytes;FMemory::Memcpy(Bad.GetData()+DurationAt,&BadTime,sizeof(double));GatherCRC(Bad);Refuse(Bad,TEXT("Invalid remaining duration"));}
    Bad=Bytes;Bad.Pop();GatherCRC(Bad);Refuse(Bad,TEXT("CRC-valid truncated windows"));Bad=Bytes;Bad.Add(1);GatherCRC(Bad);Refuse(Bad,TEXT("CRC-valid trailing windows"));
    Bad=Bytes;Bad.SetNum(CountAt+4+16+TagLength);GatherCRC(Bad);Refuse(Bad,TEXT("Count claims second bucket after complete first bucket"));
    Bad=WithoutWindows;GatherPutWord(Bad,4,2);Refuse(Bad,TEXT("ClaimingV2 without a window count"));Bad=Bytes;GatherPutWord(Bad,4,1);Refuse(Bad,TEXT("V1 may not silently discard windows"));
    Bad=Bytes;GatherPutWord(Bad,4,3);Refuse(Bad,TEXT("Future version rejected"));Bad=Bytes;Bad.Last()^=1;Refuse(Bad,TEXT("Window checksum corruption"));
    Invalid=Saved;Invalid.GatherWindows[0].RemainingSeconds=std::numeric_limits<double>::quiet_NaN();auto Prior=Bytes;TestFalse(TEXT("Invalid encoder refuses"),FPFProgressionSaveFormat::Encode(Invalid,*Catalog,*Crafting,*Items,Prior,Error));TestTrue(TEXT("Invalid encode leaves bytes untouched"),Prior==Bytes);
    AddInfo(TEXT("[PrimalAgentTools] Pure bounded gather counters/active elapsed/expiry/cap and progressionV1/V2 adversarial codec checked. No live gathering reward or active clock lifecycle is connected yet."));return true;
}
#endif
