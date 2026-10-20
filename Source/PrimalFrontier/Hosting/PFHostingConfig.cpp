#include "Hosting/PFHostingConfig.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    bool HostingText(const TSharedPtr<FJsonObject>& Object,const TCHAR* Key,FString& Value)
    {
        const auto Field=Object?Object->TryGetField(Key):nullptr;
        if(!Field || Field->Type!=EJson::String || !Object->TryGetStringField(Key,Value) || Value.Len()>2048){return false;}
        for(TCHAR C:Value){if(C<32 || C==127){return false;}}
        return true;
    }
    bool HostingPort(const TSharedPtr<FJsonObject>& Object,const TCHAR* Key,int32& Out)
    {
        double Number=0;
        const auto Field=Object?Object->TryGetField(Key):nullptr;
        if(!Field || Field->Type!=EJson::Number || !Object->TryGetNumberField(Key,Number) || !FMath::IsFinite(Number) ||
            Number<1024 || Number>65535 || Number!=FMath::FloorToDouble(Number)){return false;}
        Out=static_cast<int32>(Number);return true;
    }
}

bool FPFHostingAgentConfig::Parse(const FString& Json,FPFHostingAgentConfig& Out,FString& Error)
{
    auto Reject=[&Error](){Error=TEXT("Invalid bounded GSDK agent configuration (no values logged)");return false;};
    if(Json.IsEmpty() || Json.Len()>65536){return Reject();}
    TSharedPtr<FJsonObject> Root;
    if(!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json),Root) || !Root){return Reject();}
    FString Value,Endpoint,HostId;
    for(const TCHAR* Key:{TEXT("heartbeatEndpoint"),TEXT("sessionHostId"),TEXT("logFolder"),TEXT("sharedContentFolder")})
    {if(!HostingText(Root,Key,Value)){return Reject();}}
    Root->TryGetStringField(TEXT("heartbeatEndpoint"),Endpoint);Root->TryGetStringField(TEXT("sessionHostId"),HostId);
    if(HostId.IsEmpty() || HostId.Len()>128){return Reject();}
    for(TCHAR C:HostId){if(!FChar::IsAlnum(C) && C!=TEXT('-') && C!=TEXT('_')){return Reject();}}
    FString Host,PortText;int32 HeartbeatPort=0;
    if(!Endpoint.Split(TEXT(":"),&Host,&PortText) || Host.IsEmpty() || !LexTryParseString(HeartbeatPort,*PortText) || HeartbeatPort<1024 || HeartbeatPort>65535){return Reject();}
    for(TCHAR C:Host){if(!FChar::IsAlnum(C) && C!=TEXT('.') && C!=TEXT('-')){return Reject();}}
    for(const TCHAR* Key:{TEXT("certificateFolder"),TEXT("publicIpV4Address"),TEXT("fullyQualifiedDomainName"),TEXT("vmId")})
    {if(Root->HasField(Key) && !HostingText(Root,Key,Value)){return Reject();}}
    for(const TCHAR* Key:{TEXT("gameCertificates"),TEXT("buildMetadata"),TEXT("gamePorts")})
    {
        if(!Root->HasField(Key)){continue;}
        const TSharedPtr<FJsonObject>* Object=nullptr;
        if(!Root->TryGetObjectField(Key,Object) || !Object || !*Object || (*Object)->Values.Num()>32){return Reject();}
        for(const auto& Field:(*Object)->Values)
        {if(Field.Key.Len()>128 || !HostingText(*Object,*Field.Key,Value)){return Reject();}}
    }
    const TSharedPtr<FJsonObject>* Connection=nullptr;const TArray<TSharedPtr<FJsonValue>>* Ports=nullptr;
    if(!Root->TryGetObjectField(TEXT("gameServerConnectionInfo"),Connection) || !Connection || !*Connection ||
        !HostingText(*Connection,TEXT("publicIpV4Adress"),Value) || // Upstream SDK field spelling.
        !(*Connection)->TryGetArrayField(TEXT("gamePortsConfiguration"),Ports) || !Ports || Ports->Num()<1 || Ports->Num()>16){return Reject();}
    FPFHostingAgentConfig Candidate;TSet<FString> Names;
    for(const auto& Entry:*Ports)
    {
        const TSharedPtr<FJsonObject>* Port=nullptr;FString Name;int32 Listen=0,Client=0;
        if(!Entry || !Entry->TryGetObject(Port) || !Port || !HostingText(*Port,TEXT("name"),Name) || Name.IsEmpty() || Name.Len()>64 || Names.Contains(Name) ||
            !HostingPort(*Port,TEXT("serverListeningPort"),Listen) || !HostingPort(*Port,TEXT("clientConnectionPort"),Client)){return Reject();}
        Names.Add(Name);if(Name==TEXT("gameport")){Candidate.ListenPort=Listen;}
    }
    if(Candidate.ListenPort==0){return Reject();}
    Out=Candidate;Error.Reset();return true;
}
