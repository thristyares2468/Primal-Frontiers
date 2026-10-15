// Copied ONLY into an isolated c3d221f source export by GenerateLegacyM8Fixture.ps1.
// Not compiled by the current game. All repeated GUIDs are deliberately fake.
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Inventory/PFItemCatalog.h"
#include "Building/PFBuildingCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Creatures/PFCreatureCatalog.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"
#include "Misc/Base64.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonSerializer.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFLegacyM8Writer,"PF.Persistence.LegacyWriterExport",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFLegacyM8Writer::RunTest(const FString&)
{
    auto* Items=NewObject<UPFItemCatalog>();auto* Buildings=NewObject<UPFBuildingCatalog>();
    auto* Crafting=NewObject<UPFCraftingCatalog>();auto* Creatures=NewObject<UPFCreatureCatalog>();
    const auto Id=[](uint32 Value){return FGuid(Value,Value,Value,Value);};
    FPFWorldSaveData World;World.Map=TEXT("L_Automation");World.Hour=21;
    FPFPlayerSaveData Player;Player.PlayerId=Id(0x11111111);Player.Location=FVector(-500,0,120);
    Player.Health=33;Player.Stamina=61;Player.Hunger=47;Player.Thirst=27;
    Player.Inventory.Add({Id(0x55555555),TEXT("Item_Wood"),3,0});Player.Inventory.Add({Id(0x66666666),TEXT("Item_Food"),2,180});
    FString Error;FPFWorldPlayerRecord Owner;Owner.PlayerId=Player.PlayerId;Owner.ReconnectCredential=Id(0x22222222);Owner.CapturedUtc=FDateTime::UtcNow().ToUnixTimestamp();
    if(!TestTrue(TEXT("Historical writer packs fake player"),FPFWorldSaveFormat::PackPlayer(Player,*Items,30,Owner.Data,Error))){AddError(Error);return false;}World.Players.Add(Owner);
    FPFStructureSaveRecord Base;Base.Id=Id(0x33333333);Base.Owner=Player.PlayerId;Base.Definition=TEXT("Build_Foundation");Base.Location=FVector(600,600,10);Base.Health=100;
    FPFPlayerSaveData Bag;Bag.PlayerId=Base.Id;
    if(!FPFWorldSaveFormat::PackPlayer(Bag,*Items,60,Base.Storage,Error)){AddError(Error);return false;}World.Structures.Add(Base);
    auto Chest=Base;Chest.Id=Id(0x44444444);Chest.Definition=TEXT("Build_Storage");Chest.Location.Z=60;Chest.Support=Base.Id;
    Bag.PlayerId=Chest.Id;Bag.Inventory.Add({Id(0x77777777),TEXT("Item_Wood"),7,0});Bag.Inventory.Add({Id(0x88888888),TEXT("Item_Food"),3,90});
    if(!FPFWorldSaveFormat::PackPlayer(Bag,*Items,60,Chest.Storage,Error)){AddError(Error);return false;}World.Structures.Add(Chest);
    FPFResourceSaveRecord Node;Node.Name=TEXT("LegacyWood");Node.Definition=TEXT("Node_Wood");Node.Hits=0;Node.Respawn=10;World.Resources.Add(Node);
    FPFCreatureSaveRecord Creature;Creature.Id=Id(0xAAAAAAAA);Creature.Definition=TEXT("Creature_Forager");Creature.Location=FVector(1400,0,100);Creature.Home=Creature.Location;Creature.Health=1;World.Creatures.Add(Creature);
    FPFSpawnerSaveRecord Spawner;Spawner.Name=TEXT("LegacySpawn");Spawner.Resident=Creature.Id;Spawner.Enabled=false;World.Spawners.Add(Spawner);
    FPFPickupSaveRecord Pickup;Pickup.Location=FVector(0,700,40);Bag=FPFPlayerSaveData();Bag.PlayerId=Id(0x99999999);Bag.Inventory.Add({Bag.PlayerId,TEXT("Item_Stone"),2,0});
    if(!FPFWorldSaveFormat::PackPlayer(Bag,*Items,100000,Pickup.Data,Error)){AddError(Error);return false;}World.Pickups.Add(Pickup);
    TArray<uint8> Payload;const FString Slot=TEXT("AutomationFrozenLegacyWriter");
    if(!TestTrue(TEXT("Historical semantic validation"),FPFWorldSaveFormat::Validate(World,*Items,*Buildings,*Crafting,*Creatures,Error)) ||
       !TestTrue(TEXT("Historical world encode and file write"),FPFWorldSaveFormat::Encode(World,Payload,Error) && FPFSaveFileStore::Write(Slot,Payload,Error))){AddError(Error);return false;}
    FPFSavedFile File;TArray<uint8> Envelope;
    if(!TestTrue(TEXT("Historical file readback"),FPFSaveFileStore::Read(Slot,File,Error) && File.Payload==Payload && FFileHelper::LoadFileToArray(Envelope,*FPFSaveFileStore::Path(Slot,false)))){AddError(Error);return false;}
    auto Json=MakeShared<FJsonObject>();Json->SetStringField(TEXT("fixture"),TEXT("SyntheticM8WorldV1"));
    Json->SetStringField(TEXT("payloadBase64"),FBase64::Encode(Payload));Json->SetStringField(TEXT("envelopeBase64"),FBase64::Encode(Envelope));
    Json->SetNumberField(TEXT("savedUtc"),File.SavedUtc);Json->SetNumberField(TEXT("capturedUtc"),Owner.CapturedUtc);Json->SetNumberField(TEXT("generation"),File.Generation);
    FString Text;FJsonSerializer::Serialize(Json,TJsonWriterFactory<>::Create(&Text));
    const FString Output=FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("AutomationReports/LegacyWriter/fixture.json"));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Output),true);
    TestTrue(TEXT("Export frozen historical writer bytes"),FFileHelper::SaveStringToFile(Text,*Output,FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM));
    AddInfo(TEXT("[PrimalAgentTools] Isolated historical writer emitted synthetic V1 world/player/storage/pickup and actual file envelope; fake identities only, no content or private saves."));return true;
}
