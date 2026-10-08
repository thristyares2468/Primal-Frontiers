#include "Persistence/PFLocalPlayer.h"
#include "Persistence/PFSaveFileStore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/PendingNetGame.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/SecureHash.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

FString UPFLocalPlayer::ProfileSlot(UWorld* World)
{
    FString Profile = TEXT("Local"); FParse::Value(FCommandLine::Get(), TEXT("PFIdentityProfile="), Profile);
    if (!FPFSaveFileStore::ValidSlot(Profile) || Profile.Len() > 16 || !World || !GEngine) { return FString(); }
    const auto* Context = GEngine->GetWorldContextFromWorld(World);
    FString Endpoint = TEXT("Standalone:") + UWorld::RemovePIEPrefix(World->GetMapName());
    if (Context && Context->PendingNetGame) { Endpoint = Context->PendingNetGame->URL.Host + TEXT(":") + FString::FromInt(Context->PendingNetGame->URL.Port); }
    else if (World->GetNetMode() == NM_Client && Context) { Endpoint = Context->LastRemoteURL.Host + TEXT(":") + FString::FromInt(Context->LastRemoteURL.Port); }
    return TEXT("Identity_") + Profile + TEXT("_") + FMD5::HashAnsiString(*Endpoint);
}

FString UPFLocalPlayer::GetGameLoginOptions() const
{
    FPFSavedFile File; FString Error; FGuid Credential;
    if (FPFSaveFileStore::Read(ProfileSlot(GetWorld()), File, Error) && File.Payload.Num() == 16)
    {
        FMemoryReader Reader(File.Payload, true); Reader << Credential;
    }
    return Credential.IsValid() ? TEXT("PFReconnect=") + Credential.ToString(EGuidFormats::Digits) : FString();
}

bool UPFLocalPlayer::RememberCredential(UWorld* World, FGuid Credential)
{
    if (!Credential.IsValid()) { return false; }
    TArray<uint8> Payload; FMemoryWriter Writer(Payload, true); Writer << Credential;
    FString Error; return FPFSaveFileStore::Write(ProfileSlot(World), Payload, Error);
}
