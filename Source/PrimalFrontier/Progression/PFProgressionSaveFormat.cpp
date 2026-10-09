#include "Progression/PFProgressionSaveFormat.h"
#include "Progression/PFProgressionCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Inventory/PFItemCatalog.h"
#include "Misc/Crc.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

namespace
{
constexpr uint32 ProgressionMagic=0x50584650; // PFXP, little-endian Unreal Win64 archive.
bool CodecRefuse(FString& Error,const TCHAR* Reason){Error=Reason;return false;}
bool ValidProgressionText(const FString& Text)
{
    if(Text.IsEmpty() || Text.Len()>64){return false;}
    for(TCHAR C:Text){if(!((C>=TEXT('A') && C<=TEXT('Z')) || (C>=TEXT('a') && C<=TEXT('z')) || (C>=TEXT('0') && C<=TEXT('9')) || C==TEXT('_'))){return false;}}
    return true;
}
bool WriteProgressionIds(FArchive& Writer,const TArray<FName>& Ids,FString& Error)
{
    int32 Count=Ids.Num();Writer<<Count;
    for(FName Id:Ids){const FString Text=Id.ToString();if(!ValidProgressionText(Text)){return CodecRefuse(Error,TEXT("Invalid progression wire ID"));}int32 Length=Text.Len();Writer<<Length;for(TCHAR C:Text){uint8 Ascii=static_cast<uint8>(C);Writer<<Ascii;}}
    return !Writer.IsError();
}
bool ReadProgressionIds(FArchive& Reader,int32 Maximum,const TArray<FName>& Known,TArray<FName>& Out,FString& Error)
{
    int32 Count=0;Reader<<Count;
    if(Reader.IsError() || Count<0 || Count>Maximum || Reader.TotalSize()-Reader.Tell()<static_cast<int64>(Count)*5)
    {return CodecRefuse(Error,TEXT("Invalid progression array count"));}
    TArray<FName> Candidate;Candidate.Reserve(Count);
    for(int32 I=0;I<Count;++I)
    {
        int32 Length=0;Reader<<Length;
        if(Reader.IsError() || Length<1 || Length>64 || Reader.TotalSize()-Reader.Tell()<Length){return CodecRefuse(Error,TEXT("Invalid progression ID length"));}
        FString Text;Text.Reserve(Length);for(int32 N=0;N<Length;++N){uint8 Ascii=0;Reader<<Ascii;Text.AppendChar(static_cast<TCHAR>(Ascii));}
        if(!ValidProgressionText(Text)){return CodecRefuse(Error,TEXT("Invalid progression ID characters"));}
        FName Resolved=NAME_None;for(FName Id:Known){if(Id.ToString().Equals(Text,ESearchCase::IgnoreCase)){Resolved=Id;break;}}
        if(Resolved.IsNone() || Candidate.Contains(Resolved)){return CodecRefuse(Error,TEXT("Unknown or duplicate progression ID"));}Candidate.Add(Resolved);
    }
    if(Reader.IsError()){return CodecRefuse(Error,TEXT("Truncated progression IDs"));}Out=MoveTemp(Candidate);return true;
}
}
bool FPFProgressionSaveFormat::Encode(const FPFProgressionRecord& Record,const UPFProgressionCatalog& Catalog,
    const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,TArray<uint8>& Out,FString& Error)
{
    if(!FPFProgressionTransactions::Validate(Record,Catalog,Crafting,Items,Error)){return false;}
    TArray<uint8> Payload;FMemoryWriter Writer(Payload,true);int32 XP=Record.Experience;Writer<<XP;
    if(!WriteProgressionIds(Writer,Record.Knowledge,Error) || !WriteProgressionIds(Writer,Record.CreditedCrafts,Error)){return false;}
    if(Writer.IsError() || Payload.Num()>MaximumBytes-EnvelopeBytes){return CodecRefuse(Error,TEXT("Progression payload exceeds limit"));}
    TArray<uint8> Bytes;FMemoryWriter Envelope(Bytes,true);uint32 Magic=ProgressionMagic,Version=CurrentVersion,Length=Payload.Num(),CRC=FCrc::MemCrc32(Payload.GetData(),Payload.Num());
    Envelope<<Magic<<Version<<Length<<CRC;Envelope.Serialize(Payload.GetData(),Payload.Num());
    if(Envelope.IsError()){return CodecRefuse(Error,TEXT("Progression envelope write failed"));}Out=MoveTemp(Bytes);Error.Reset();return true;
}
bool FPFProgressionSaveFormat::Decode(const TArray<uint8>& Bytes,const UPFProgressionCatalog& Catalog,
    const UPFCraftingCatalog& Crafting,const UPFItemCatalog& Items,FPFProgressionRecord& Out,FString& Error)
{
    if(!Catalog.Validate(Crafting,Items,Error)){return false;}
    if(Bytes.Num()<EnvelopeBytes+12 || Bytes.Num()>MaximumBytes){return CodecRefuse(Error,TEXT("Invalid progression file size"));}
    FMemoryReader Reader(Bytes,true);uint32 Magic=0,Version=0,Length=0,CRC=0;Reader<<Magic<<Version<<Length<<CRC;
    if(Magic!=ProgressionMagic || Version!=CurrentVersion){return CodecRefuse(Error,TEXT("Invalid progression signature or unsupported version"));}
    if(Length!=static_cast<uint32>(Bytes.Num()-EnvelopeBytes) || FCrc::MemCrc32(Bytes.GetData()+EnvelopeBytes,Length)!=CRC)
    {return CodecRefuse(Error,TEXT("Progression length/checksum mismatch"));}
    FPFProgressionRecord Candidate;Reader<<Candidate.Experience;TArray<FName> KnowledgeIds,RecipeIds;
    for(const auto& D:Catalog.Knowledge){KnowledgeIds.Add(D.Id);}for(const auto& D:Crafting.Recipes){RecipeIds.Add(D.Id);}
    if(!ReadProgressionIds(Reader,UPFProgressionCatalog::MaximumKnowledge,KnowledgeIds,Candidate.Knowledge,Error) ||
        !ReadProgressionIds(Reader,UPFProgressionCatalog::MaximumCraftRecords,RecipeIds,Candidate.CreditedCrafts,Error)){return false;}
    if(Reader.IsError() || Reader.Tell()!=Reader.TotalSize()){return CodecRefuse(Error,TEXT("Truncated or trailing progression payload"));}
    if(!FPFProgressionTransactions::Validate(Candidate,Catalog,Crafting,Items,Error)){return false;}Out=MoveTemp(Candidate);Error.Reset();return true;
}
bool FPFProgressionSaveFormat::DecodeLegacyPlayer(const TArray<uint8>& Bytes,const UPFItemCatalog& Items,
    const FPFPlayerSaveLimits& Limits,const UPFProgressionCatalog& Catalog,const UPFCraftingCatalog& Crafting,
    FPFPlayerSaveData& OutPlayer,FPFProgressionRecord& OutProgression,FString& Error)
{
    if(Bytes.Num()<8 || (static_cast<uint32>(Bytes[4]) | static_cast<uint32>(Bytes[5])<<8 | static_cast<uint32>(Bytes[6])<<16 | static_cast<uint32>(Bytes[7])<<24)!=1)
    {return CodecRefuse(Error,TEXT("Legacy defaults require a V1 player record"));}
    FPFPlayerSaveData Player;FPFProgressionRecord Progress;
    if(!FPFPlayerSaveFormat::Decode(Bytes,Items,Limits,Player,Error) || !FPFProgressionTransactions::Validate(Progress,Catalog,Crafting,Items,Error)){return false;}
    OutPlayer=MoveTemp(Player);OutProgression=MoveTemp(Progress);Error.Reset();return true;
}
