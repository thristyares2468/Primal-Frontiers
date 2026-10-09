#if WITH_DEV_AUTOMATION_TESTS
#include "Persistence/PFWorldSaveFormat.h"
#include "Progression/PFProgressionSaveFormat.h"
#include "Progression/PFProgressionCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "Building/PFBuildingCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Creatures/PFCreatureCatalog.h"
#include "Misc/AutomationTest.h"
#include "Misc/Base64.h"
#include "Misc/DateTime.h"
#include "Misc/Crc.h"
#include "Serialization/JsonSerializer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFProgressionWorldCompatibilityTest,"PF.Progression.WorldCompatibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFProgressionWorldCompatibilityTest::RunTest(const FString&)
{
    auto* Items=NewObject<UPFItemCatalog>();auto* Buildings=NewObject<UPFBuildingCatalog>();
    auto* Crafting=NewObject<UPFCraftingCatalog>();auto* Creatures=NewObject<UPFCreatureCatalog>();
    auto* Catalog=NewObject<UPFProgressionCatalog>();FString Error;FPFWorldSaveData Legacy;Legacy.Map=TEXT("L_Automation");Legacy.Hour=22;
    FPFPlayerSaveData Player;Player.PlayerId=FGuid::NewGuid();Player.Health=65;Player.Location=FVector(125,-200,90);
    Player.Inventory={{FGuid::NewGuid(),TEXT("Item_BoundTool"),1,0},{FGuid::NewGuid(),TEXT("Item_Food"),2,12.5}};
    FPFWorldPlayerRecord Owner;Owner.PlayerId=Player.PlayerId;Owner.ReconnectCredential=FGuid::NewGuid();Owner.CapturedUtc=FDateTime::UtcNow().ToUnixTimestamp();
    if(!TestTrue(TEXT("Encode legacy bag"),FPFWorldSaveFormat::PackPlayer(Player,*Items,30,Owner.Data,Error))){return false;}Legacy.Players.Add(Owner);
    FPFPlayerSaveData Other;Other.PlayerId=FGuid::NewGuid();auto Second=Owner;Second.PlayerId=Other.PlayerId;Second.ReconnectCredential=FGuid::NewGuid();
    if(!TestTrue(TEXT("Encode independent bag"),FPFWorldSaveFormat::PackPlayer(Other,*Items,30,Second.Data,Error))){return false;}Legacy.Players.Add(Second);
    FPFStructureSaveRecord Base;Base.Id=FGuid::NewGuid();Base.Owner=Owner.PlayerId;Base.Definition=TEXT("Build_Foundation");Base.Health=100;
    FPFPlayerSaveData Empty;Empty.PlayerId=Base.Id;
    if(!TestTrue(TEXT("Encode foundation bag"),FPFWorldSaveFormat::PackPlayer(Empty,*Items,60,Base.Storage,Error))){return false;}Legacy.Structures.Add(Base);
    auto Chest=Base;Chest.Id=FGuid::NewGuid();Chest.Support=Base.Id;Chest.Definition=TEXT("Build_Storage");Empty.PlayerId=Chest.Id;
    Empty.Inventory={{FGuid::NewGuid(),TEXT("Item_Stone"),2,0}};
    if(!TestTrue(TEXT("Encode chest bag"),FPFWorldSaveFormat::PackPlayer(Empty,*Items,60,Chest.Storage,Error))){return false;}Legacy.Structures.Add(Chest);
    auto Valid=[&](const FPFWorldSaveData& D){return FPFWorldSaveFormat::Validate(D,*Items,*Buildings,*Crafting,*Creatures,Error,Catalog);};
    auto Decode=[&](const TArray<uint8>& B,FPFWorldSaveData& Out){return FPFWorldSaveFormat::DecodeValidated(B,*Items,*Buildings,*Crafting,*Creatures,Out,Error,Catalog);};
    if(!TestTrue(TEXT("Legacy semantic fixture valid"),Valid(Legacy))){AddError(Error);return false;}
    TArray<uint8> V1Bytes;if(!TestTrue(TEXT("V1 encode"),FPFWorldSaveFormat::Encode(Legacy,V1Bytes,Error))){return false;}
    const FUTF8ToTCHAR V1Text(reinterpret_cast<const ANSICHAR*>(V1Bytes.GetData()),V1Bytes.Num());const FString V1Json(V1Text.Length(),V1Text.Get());
    TestFalse(TEXT("V1 writer omits new field entirely"),V1Json.Contains(TEXT("progression")));
    FPFWorldSaveData Decoded;if(!TestTrue(TEXT("Strict legacy world read with explicit defaults"),Decode(V1Bytes,Decoded))){AddError(Error);return false;}
    TestTrue(TEXT("Both legacy players default without retrospective rewards"),Decoded.Players[0].Progression.IsEmpty() && Decoded.Players[1].Progression.IsEmpty());
    TestEqual(TEXT("Legacy owner credential retained"),Decoded.Players[0].ReconnectCredential,Owner.ReconnectCredential);
    TestTrue(TEXT("Legacy storage identity and ownership retained"),Decoded.Structures[1].Id==Chest.Id && Decoded.Structures[1].Owner==Owner.PlayerId && Decoded.Structures[1].Storage==Chest.Storage);
    auto V2=Legacy;V2.Version=2;FPFProgressionRecord Earned;Earned.Experience=100;Earned.Knowledge={FName(TEXT("Tech_FieldTools"))};Earned.CreditedCrafts={FName(TEXT("Recipe_Tool"))};FPFProgressionRecord Zero;
    if(!TestTrue(TEXT("Pack earned owner"),FPFWorldSaveFormat::PackProgression(Owner.PlayerId,Earned,*Catalog,*Crafting,*Items,V2.Players[0].Progression,Error)) ||
        !TestTrue(TEXT("Pack zero second owner"),FPFWorldSaveFormat::PackProgression(Second.PlayerId,Zero,*Catalog,*Crafting,*Items,V2.Players[1].Progression,Error))){return false;}
    TArray<uint8> V2Bytes;if(!TestTrue(TEXT("Opt-in V2 valid"),Valid(V2)) || !TestTrue(TEXT("V2 encode"),FPFWorldSaveFormat::Encode(V2,V2Bytes,Error)) || !TestTrue(TEXT("V2 semantic decode"),Decode(V2Bytes,Decoded))){AddError(Error);return false;}
    FPFProgressionRecord Read;
    if(!TestTrue(TEXT("Owner-bound earned read"),FPFWorldSaveFormat::UnpackProgression(Decoded.Players[0].Progression,Owner.PlayerId,*Catalog,*Crafting,*Items,Read,Error))){return false;}
    TestTrue(TEXT("Exact earned state and available points"),Read.Experience==100 && Read.Knowledge==Earned.Knowledge && Read.CreditedCrafts==Earned.CreditedCrafts && FPFProgressionTransactions::AvailablePoints(Read,*Catalog)==1);
    TestFalse(TEXT("Read cannot reaward completed craft"),FPFProgressionTransactions::CreditFirstCraft(Read,TEXT("Recipe_Tool"),*Catalog,*Crafting,*Items,Error));
    TestTrue(TEXT("Independent owner read"),FPFWorldSaveFormat::UnpackProgression(Decoded.Players[1].Progression,Second.PlayerId,*Catalog,*Crafting,*Items,Read,Error));
    TestTrue(TEXT("No cross-player XP/knowledge"),Read.Experience==0 && Read.Knowledge.IsEmpty() && Read.CreditedCrafts.IsEmpty());
    FPFPlayerSaveData Bag;if(!TestTrue(TEXT("Existing bag still valid"),FPFWorldSaveFormat::UnpackPlayer(Decoded.Players[0].Data,*Items,30,Bag,Error))){return false;}
    TestTrue(TEXT("Vitals/location/items/batches/freshness unchanged"),Bag.PlayerId==Player.PlayerId && Bag.Health==65 && Bag.Location==Player.Location && Bag.Inventory[0].ItemId==Player.Inventory[0].ItemId && Bag.Inventory[0].StackId==Player.Inventory[0].StackId && Bag.Inventory[1].RemainingFreshnessSeconds==12.5);
    TestTrue(TEXT("World archive data retained exactly"),Decoded.Hour==Legacy.Hour && Decoded.Players[0].CapturedUtc==Owner.CapturedUtc && Decoded.Players[0].ReconnectCredential==Owner.ReconnectCredential && Decoded.Structures[1].Support==Base.Id && Decoded.Structures[1].Storage==Chest.Storage);
    Decoded.Map=TEXT("Preserved");const auto Sentinel=Decoded;
    auto Refuse=[&](const TArray<uint8>& Bad,const TCHAR* Why){TestFalse(Why,Decode(Bad,Decoded));TestFalse(TEXT("Explicit refusal reason"),Error.IsEmpty());TestTrue(TEXT("Whole output retained on refusal"),Decoded.Map==Sentinel.Map && Decoded.Players[0].Data==Sentinel.Players[0].Data && Decoded.Players[0].Progression==Sentinel.Players[0].Progression && Decoded.Structures[1].Storage==Sentinel.Structures[1].Storage);};
    auto RefuseRecord=[&](const FPFWorldSaveData& Bad,const TCHAR* Why){TArray<uint8> B;if(TestTrue(TEXT("Encode negative fixture"),FPFWorldSaveFormat::Encode(Bad,B,Error))){Refuse(B,Why);}};
    auto Bad=V2;Swap(Bad.Players[0].Progression,Bad.Players[1].Progression);RefuseRecord(Bad,TEXT("Swapped owners refuse"));Bad=V2;Bad.Players[1].Progression.Reset();RefuseRecord(Bad,TEXT("Missing independent owner refuses"));
    Bad=V2;Bad.Version=3;RefuseRecord(Bad,TEXT("Future world version refuses"));
    TArray<uint8> Owned;if(!TestTrue(TEXT("Decode test envelope"),FBase64::Decode(V2.Players[0].Progression,Owned)) || !TestTrue(TEXT("Known envelope fixture size"),Owned.Num()>44)){return false;}
    auto Put=[&](TArray<uint8>& B,int32 At,uint32 Value){for(int32 I=0;I<4;++I){B[At+I]=static_cast<uint8>(Value>>(I*8));}};
    auto Forged=Owned;Forged.Last()^=1;Bad=V2;Bad.Players[0].Progression=FBase64::Encode(Forged);RefuseRecord(Bad,TEXT("Inner CRC refuses"));
    Forged=Owned;Put(Forged,20,2);Bad=V2;Bad.Players[0].Progression=FBase64::Encode(Forged);RefuseRecord(Bad,TEXT("Future progression version refuses"));
    Forged=Owned;Forged[44]=TEXT('X');Put(Forged,28,FCrc::MemCrc32(Forged.GetData()+32,Forged.Num()-32));Bad=V2;Bad.Players[0].Progression=FBase64::Encode(Forged);RefuseRecord(Bad,TEXT("Checksum-valid unknown knowledge refuses"));
    Forged=Owned;Put(Forged,32,2701);Put(Forged,28,FCrc::MemCrc32(Forged.GetData()+32,Forged.Num()-32));Bad=V2;Bad.Players[0].Progression=FBase64::Encode(Forged);RefuseRecord(Bad,TEXT("Checksum-valid invalid XP refuses"));
    auto PreservedBytes=V1Bytes;Bad=V2;Bad.Version=1;TestFalse(TEXT("V1 refuses to discard progression on write"),FPFWorldSaveFormat::Encode(Bad,PreservedBytes,Error));TestTrue(TEXT("Failed encode retains original bytes"),PreservedBytes==V1Bytes);
    auto MutateJson=[&](const TArray<uint8>& B,TFunctionRef<void(TSharedPtr<FJsonObject>)> Change)
    {
        const FUTF8ToTCHAR Text(reinterpret_cast<const ANSICHAR*>(B.GetData()),B.Num());TSharedPtr<FJsonObject> Root;
        if(!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(FString(Text.Length(),Text.Get())),Root)){return TArray<uint8>{};}
        Change(Root);FString Json;FJsonSerializer::Serialize(Root.ToSharedRef(),TJsonWriterFactory<>::Create(&Json));FTCHARToUTF8 Utf8(*Json);TArray<uint8> Out;Out.Append(reinterpret_cast<const uint8*>(Utf8.Get()),Utf8.Length());return Out;
    };
    Refuse(MutateJson(V2Bytes,[](auto R){R->SetNumberField(TEXT("version"),1);}),TEXT("V1 hidden progression refuses on read"));
    Refuse(MutateJson(V2Bytes,[](auto R){R->GetArrayField(TEXT("players"))[0]->AsObject()->RemoveField(TEXT("progression"));}),TEXT("Absent V2 field refuses"));
    Refuse(MutateJson(V2Bytes,[](auto R){R->GetArrayField(TEXT("players"))[0]->AsObject()->SetNumberField(TEXT("progression"),1);}),TEXT("Nonstring V2 field refuses"));
    Refuse(MutateJson(V2Bytes,[](auto R){R->GetArrayField(TEXT("players"))[0]->AsObject()->SetStringField(TEXT("progression"),FString::ChrN(23000,TEXT('A')));}),TEXT("Oversized V2 field refuses"));
    Refuse(MutateJson(V2Bytes,[](auto R){R->SetArrayField(TEXT("players"),{MakeShared<FJsonValueString>(TEXT("bad"))});}),TEXT("Nonobject player refuses without assertion"));
    Read=Earned;TestFalse(TEXT("Invalid owner read refuses"),FPFWorldSaveFormat::UnpackProgression(V2.Players[0].Progression,Second.PlayerId,*Catalog,*Crafting,*Items,Read,Error));TestTrue(TEXT("Failed owner read preserves record"),Read.Experience==Earned.Experience && Read.Knowledge==Earned.Knowledge);
    FString Prior=V2.Players[0].Progression;TestFalse(TEXT("Invalid owner write refuses"),FPFWorldSaveFormat::PackProgression({},Earned,*Catalog,*Crafting,*Items,Prior,Error));TestEqual(TEXT("Failed owner write preserves encoding"),Prior,V2.Players[0].Progression);
    AddInfo(TEXT("[PrimalAgentTools] Legacy V1 omission/defaults and opt-in owner-bound V2 roundtrip/refusal checked; gameplay V2 restoration remains blocked."));return true;
}
#endif
