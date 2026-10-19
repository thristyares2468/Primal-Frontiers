#include "Persistence/PFCheckpointClient.h"
#include "Persistence/PFCheckpointProtocol.h"
#include "Persistence/PFWorldPersistence.h"
#include "Engine/World.h"
#include "HAL/PlatformMisc.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/Archive.h"

namespace
{
    // Deliberate endpoint allowlist: environment credentials cannot be redirected to an arbitrary configured host.
    const FString CheckpointApiEndpoint=TEXT("https://primalfrontier-backend.jackherbig09.workers.dev/v1/checkpoints");

    class FBoundedCheckpointResponse final : public FArchive
    {
    public:
        TArray<uint8> Bytes;
        virtual void Serialize(void* Data,int64 Length) override
        {
            if (Length<0 || Length>FPFCheckpointProtocol::MaxResponseBytes-Bytes.Num()) { SetError(); return; }
            Bytes.Append(static_cast<const uint8*>(Data),static_cast<int32>(Length));
        }
    };
}

bool FPFCheckpointClient::EligibleWorld(const UWorld* World)
{
#if WITH_SERVER_CODE
    return IsInGameThread() && World && World->IsGameWorld() && World->GetNetMode()==NM_DedicatedServer && World->GetAuthGameMode();
#else
    return false;
#endif
}

bool FPFCheckpointClient::UploadSavedSlot(UWorld* World,const FString& Slot,const FString& WorldId,FCompletion Complete,FString& Error)
{
    Error.Reset();
    if (!EligibleWorld(World)) { Error=TEXT("Requires a dedicated-server gameplay world; no client or listen-server credential access"); return false; }
    if (Slot.StartsWith(TEXT("Identity_"),ESearchCase::IgnoreCase) || Slot.StartsWith(TEXT("Metadata_"),ESearchCase::IgnoreCase))
    { Error=TEXT("Reserved slot cannot be uploaded"); return false; }
    auto* Persistence=World->GetSubsystem<UPFWorldPersistence>(); FPFSaveInspection Inspection;
    FPFSavedFile File;
    if (!Persistence || !Persistence->InspectSave(Slot,Inspection,Error) || !FPFSaveFileStore::Read(Slot,File,Error)) { return false; }
    return Start(World,WorldId,&File,0,MoveTemp(Complete),Error);
}

bool FPFCheckpointClient::Fetch(UWorld* World,const FString& WorldId,uint64 MinimumGeneration,FCompletion Complete,FString& Error)
{
    return Start(World,WorldId,nullptr,MinimumGeneration,MoveTemp(Complete),Error);
}

bool FPFCheckpointClient::Start(UWorld* World,const FString& WorldId,const FPFSavedFile* Upload,uint64 MinimumGeneration,FCompletion Complete,FString& Error)
{
    Error.Reset();
    if (!EligibleWorld(World)) { Error=TEXT("Requires a dedicated-server gameplay world; no client or listen-server credential access"); return false; }
    if (!Complete || !FPFCheckpointProtocol::ValidWorldId(WorldId) || MinimumGeneration>FPFCheckpointProtocol::MaxGeneration)
    { Error=TEXT("Invalid checkpoint request"); return false; }
    FString Secret=FPlatformMisc::GetEnvironmentVariable(TEXT("PF_API_SHARED_SECRET"));
    if (Secret.IsEmpty() || Secret.Len()>512 || Secret.Contains(TEXT("\r")) || Secret.Contains(TEXT("\n")))
    { Error=TEXT("Server process PF_API_SHARED_SECRET missing or invalid; no request sent"); return false; }
    TWeakObjectPtr<UWorld> WeakWorld=World;
    return Send(Upload?CheckpointApiEndpoint:CheckpointApiEndpoint+TEXT("/")+WorldId,Secret,WorldId,Upload,MinimumGeneration,
        [WeakWorld]{return EligibleWorld(WeakWorld.Get());},MoveTemp(Complete),Error);
}

bool FPFCheckpointClient::Send(const FString& Url,const FString& Secret,const FString& WorldId,const FPFSavedFile* Upload,uint64 MinimumGeneration,
    TFunction<bool()> IsActive,FCompletion Complete,FString& Error)
{
    FString Body;
    if (Upload && !FPFCheckpointProtocol::BuildUpload(WorldId,*Upload,Body,Error)) { return false; }
    const bool bUpload=Upload!=nullptr;
    const FPFSavedFile Sent=bUpload?*Upload:FPFSavedFile();
    auto Request=FHttpModule::Get().CreateRequest();
    Request->SetDelegateThreadPolicy(EHttpRequestDelegateThreadPolicy::CompleteOnGameThread);
    Request->SetURL(Url);
    Request->SetVerb(bUpload?TEXT("POST"):TEXT("GET"));
    Request->SetHeader(TEXT("Authorization"),TEXT("Bearer ")+Secret);
    Request->SetHeader(TEXT("Accept"),TEXT("application/json"));
    Request->SetTimeout(30.f);
    if (bUpload) { Request->SetHeader(TEXT("Content-Type"),TEXT("application/json")); Request->SetContentAsString(Body); }
    auto Stream=MakeShared<FBoundedCheckpointResponse>();
    if (!Request->SetResponseBodyReceiveStream(Stream)) { Error=TEXT("Bounded HTTP response stream unavailable"); return false; }
    Request->OnProcessRequestComplete().BindLambda(
        [IsActive=MoveTemp(IsActive),WorldId,MinimumGeneration,bUpload,Sent,Stream,Complete=MoveTemp(Complete)](FHttpRequestPtr,FHttpResponsePtr Response,bool bConnected) mutable
        {
            FPFCheckpointResult Result;
            Result.HttpStatus=Response?Response->GetResponseCode():0;
            if (!IsActive()) { Result.Details=TEXT("Server world ended; response discarded"); }
            else if (!bConnected || !Response || Stream->IsError()) { Result.Details=TEXT("HTTP connection, timeout or bounded response failure; local save preserved"); }
            else if (Result.HttpStatus!=(bUpload?201:200))
            { Result.Details=FString::Printf(TEXT("Checkpoint HTTP %d refused; no retry, overwrite or world mutation"),Result.HttpStatus); }
            else
            {
                const FUTF8ToTCHAR Text(reinterpret_cast<const ANSICHAR*>(Stream->Bytes.GetData()),Stream->Bytes.Num());
                const FString Json(Text.Length(),Text.Get());
                Result.bSucceeded=bUpload ? FPFCheckpointProtocol::ReadAcknowledgement(WorldId,Sent,Json,Result.Details) :
                    FPFCheckpointProtocol::ReadDownload(WorldId,MinimumGeneration,Json,Result.File,Result.Details);
                if (Result.bSucceeded)
                { Result.Details=bUpload?TEXT("Remote checkpoint acknowledged with matching generation and checksum"):TEXT("Remote envelope verified in memory; no file published or world restored"); }
            }
            Complete(MoveTemp(Result));
        });
    if (!Request->ProcessRequest()) { Error=TEXT("HTTP request could not start; local save preserved"); return false; }
    return true;
}
