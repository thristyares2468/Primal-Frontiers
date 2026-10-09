#include "Persistence/PFWorldMenuModel.h"
#include "Persistence/PFSaveFileStore.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Inventory/PFItemCatalog.h"
#include "Building/PFBuildingCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Creatures/PFCreatureCatalog.h"
#include "PFAssetPaths.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Containers/StringConv.h"

namespace
{
bool ValidDisplayName(const FString& Name)
{
    if(Name.IsEmpty() || Name.Len()>64){return false;}
    for(TCHAR C:Name){if(FChar::IsControl(C)){return false;}}
    return true;
}
bool ReadNames(TMap<FString,FString>& Names,FString& Error)
{
    const FString Registry=PFWorldMenuModel::NamesSlot();
    if(!IFileManager::Get().FileExists(*FPFSaveFileStore::Path(Registry,false)) && !IFileManager::Get().FileExists(*FPFSaveFileStore::Path(Registry,true))){return true;}
    FPFSavedFile File;if(!FPFSaveFileStore::Read(Registry,File,Error)){return false;}
    if(File.Payload.IsEmpty() || File.Payload.Num()>64*1024){Error=TEXT("Invalid world-name metadata size; original records preserved.");return false;}
    const FUTF8ToTCHAR Text(reinterpret_cast<const ANSICHAR*>(File.Payload.GetData()),File.Payload.Num());
    TSharedPtr<FJsonObject> Root;const auto Reader=TJsonReaderFactory<>::Create(FString(Text.Length(),Text.Get()));
    const TArray<TSharedPtr<FJsonValue>>* Rows=nullptr;double Version=0;
    if(!FJsonSerializer::Deserialize(Reader,Root) || !Root || !Root->TryGetNumberField(TEXT("Version"),Version) || Version!=1 || !Root->TryGetArrayField(TEXT("Worlds"),Rows) || Rows->Num()>256)
    {Error=TEXT("Invalid world-name metadata; original records preserved.");return false;}
    for(const auto& Row:*Rows)
    {
        const TSharedPtr<FJsonObject>* Object=nullptr;FString Key,Name;
        if(!Row || !Row->TryGetObject(Object) || !(*Object)->TryGetStringField(TEXT("Slot"),Key) || !(*Object)->TryGetStringField(TEXT("Name"),Name) || !FPFSaveFileStore::ValidSlot(Key) || Key.StartsWith(TEXT("Identity_"),ESearchCase::IgnoreCase) || Key.StartsWith(TEXT("Metadata_"),ESearchCase::IgnoreCase) || !ValidDisplayName(Name) || Names.Contains(Key.ToLower()))
        {Error=TEXT("Invalid or duplicate world-name record; original records preserved.");return false;}
        Names.Add(Key.ToLower(),Name);
    }
    return true;
}
}
FString PFWorldMenuModel::NamesSlot()
{
#if !UE_BUILD_SHIPPING
    FString TestSlot;
    if(FParse::Param(FCommandLine::Get(),TEXT("PFRunWorldMenuTest")) && FParse::Value(FCommandLine::Get(),TEXT("PFWorldNamesTestSlot="),TestSlot) && TestSlot.StartsWith(TEXT("Metadata_UIWorld_")) && FPFSaveFileStore::ValidSlot(TestSlot)){return TestSlot;}
#endif
    return TEXT("Metadata_WorldNames");
}
FString PFWorldMenuModel::NameFor(const FString& Slot)
{
    TMap<FString,FString> Names;FString Error;
    if(ReadNames(Names,Error)){if(const auto* Name=Names.Find(Slot.ToLower())){return *Name;}}
    return Slot;
}
bool PFWorldMenuModel::Rename(const FString& Slot,const FString& DisplayName,FString& Error)
{
    FPFWorldMenuEntry Entry;if(!Inspect(Slot,Entry)){Error=Entry.Issue;return false;}
    const FString Name=DisplayName.TrimStartAndEnd();
    if(!ValidDisplayName(Name)){Error=TEXT("Use a display name of 1-64 characters without control characters.");return false;}
    TMap<FString,FString> Names;if(!ReadNames(Names,Error)){return false;}
    for(const auto& Other:List()){if(!Other.Slot.Equals(Slot,ESearchCase::IgnoreCase) && Other.DisplayName.Equals(Name,ESearchCase::IgnoreCase)){Error=TEXT("Another saved world already uses that display name.");return false;}}
    if(!Names.Contains(Slot.ToLower()) && Names.Num()>=256){Error=TEXT("World-name metadata limit reached; original records preserved.");return false;}
    Names.Add(Slot.ToLower(),Name);auto Root=MakeShared<FJsonObject>();Root->SetNumberField(TEXT("Version"),1);
    TArray<FString> Keys;Names.GetKeys(Keys);Keys.Sort();TArray<TSharedPtr<FJsonValue>> Rows;
    for(const auto& Key:Keys){auto Row=MakeShared<FJsonObject>();Row->SetStringField(TEXT("Slot"),Key);Row->SetStringField(TEXT("Name"),Names[Key]);Rows.Add(MakeShared<FJsonValueObject>(Row));}
    Root->SetArrayField(TEXT("Worlds"),Rows);FString Encoded;const auto Writer=TJsonWriterFactory<>::Create(&Encoded);
    if(!FJsonSerializer::Serialize(Root,Writer)){Error=TEXT("Could not encode world names.");return false;}
    const FTCHARToUTF8 Bytes(*Encoded);TArray<uint8> Payload;Payload.Append(reinterpret_cast<const uint8*>(Bytes.Get()),Bytes.Length());
    if(Payload.Num()>64*1024){Error=TEXT("World-name metadata size limit reached; original records preserved.");return false;}
    return FPFSaveFileStore::Write(NamesSlot(),Payload,Error);
}

FString PFWorldMenuModel::MapPackage(const FString& Name)
{
    if(Name==TEXT("L_PrimalFrontier_OpenWorld")){return TEXT("/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld");}
    if(Name==TEXT("L_M7SurvivalArena")){return TEXT("/Game/PrimalFrontier/Maps/L_M7SurvivalArena");}
    return FString();
}
bool PFWorldMenuModel::CanCreate(const FString& Slot,FString& Error)
{
    if(!FPFSaveFileStore::ValidSlot(Slot) || Slot.StartsWith(TEXT("Identity_"),ESearchCase::IgnoreCase) || Slot.StartsWith(TEXT("Metadata_"),ESearchCase::IgnoreCase))
    {Error=TEXT("Use 1-64 letters, numbers or underscores; Identity_ and Metadata_ are reserved.");return false;}
    for(bool B:{false,true})
    {if(IFileManager::Get().FileExists(*FPFSaveFileStore::Path(Slot,B))){Error=TEXT("That world already exists. Select Load; existing or corrupt saves will not be overwritten.");return false;}}
    Error.Reset();return true;
}
bool PFWorldMenuModel::Inspect(const FString& Slot,FPFWorldMenuEntry& Out)
{
    Out={};Out.Slot=Slot;Out.DisplayName=NameFor(Slot);FPFSavedFile File;FPFWorldSaveData Data;
    if(Slot.StartsWith(TEXT("Identity_"),ESearchCase::IgnoreCase) || Slot.StartsWith(TEXT("Metadata_"),ESearchCase::IgnoreCase)){Out.Issue=TEXT("Reserved internal record, not a playable world.");return false;}
    if(!FPFSaveFileStore::Read(Slot,File,Out.Issue)){return false;}
    Out.SavedUtc=File.SavedUtc;
    if(!FPFWorldSaveFormat::Decode(File.Payload,Data,Out.Issue)){return false;}
    Out.Map=Data.Map;
    if(MapPackage(Data.Map).IsEmpty()){Out.Issue=TEXT("This map is not available from the world menu.");return false;}
    auto* I=LoadObject<UPFItemCatalog>(nullptr,PFAssetPaths::ItemCatalog);
    auto* B=LoadObject<UPFBuildingCatalog>(nullptr,PFAssetPaths::BuildingCatalog);
    auto* C=LoadObject<UPFCraftingCatalog>(nullptr,PFAssetPaths::CraftingCatalog);
    auto* A=LoadObject<UPFCreatureCatalog>(nullptr,PFAssetPaths::CreatureCatalog);
    if(!I || !B || !C || !A){Out.Issue=TEXT("Required world catalogs are unavailable.");return false;}
    if(!FPFWorldSaveFormat::Validate(Data,*I,*B,*C,*A,Out.Issue)){return false;}
    if(Data.Players.Num()!=1){Out.Issue=TEXT("Solo loading requires one saved survivor. Use the server launch flow for multiplayer worlds.");return false;}
    Out.bLoadable=true;return true;
}
TArray<FPFWorldMenuEntry> PFWorldMenuModel::List()
{
    TSet<FString> Slots;TArray<FString> Names;
    IFileManager::Get().FindFiles(Names,*(FPaths::ProjectSavedDir()/TEXT("Persistence/*.pfs")),true,false);
    Names.Sort(); // Stable bounded discovery; never enumerate profiles into the UI.
    for(const auto& Name:Names)
    {
        FString Slot=Name;
        if(!(Slot.RemoveFromEnd(TEXT(".a.pfs")) || Slot.RemoveFromEnd(TEXT(".b.pfs")))){continue;}
        if(Slot.StartsWith(TEXT("Identity_"),ESearchCase::IgnoreCase) || Slot.StartsWith(TEXT("Metadata_"),ESearchCase::IgnoreCase) || !FPFSaveFileStore::ValidSlot(Slot)){continue;}
        Slots.Add(Slot);if(Slots.Num()>=256){break;}
    }
    TArray<FPFWorldMenuEntry> Result;
    for(const auto& Slot:Slots){FPFWorldMenuEntry E;Inspect(Slot,E);Result.Add(MoveTemp(E));}
    Result.Sort([](const auto& A,const auto& B){return A.SavedUtc!=B.SavedUtc?A.SavedUtc>B.SavedUtc:A.Slot<B.Slot;});
    return Result;
}
