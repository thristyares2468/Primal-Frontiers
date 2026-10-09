#include "Persistence/PFSessionGameInstance.h"
#include "Persistence/PFWorldMenuModel.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

bool UPFSessionGameInstance::StartWorld(const FString& Slot,bool bLoad,FString& Error)
{
    if(!GetWorld() || GetWorld()->GetNetMode()!=NM_Standalone || UWorld::RemovePIEPrefix(GetWorld()->GetMapName())!=TEXT("Entry"))
    {Error=TEXT("World selection is available only from the solo main menu.");return false;}
    FString Map=TEXT("/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld");
    if(bLoad)
    {
        FPFWorldMenuEntry E;if(!PFWorldMenuModel::Inspect(Slot,E)){Error=E.Issue;return false;}
        Map=PFWorldMenuModel::MapPackage(E.Map);
    }
    else if(!PFWorldMenuModel::CanCreate(Slot,Error)){return false;}
    PendingSlot=Slot;bPendingLoad=bLoad;bHasRequest=true;MenuMessage.Reset();
    UGameplayStatics::OpenLevel(GetWorld(),FName(*Map),true);
    Error.Reset();return true;
}
bool UPFSessionGameInstance::ConsumeWorldRequest(FString& Slot,bool& bLoad)
{
    if(!bHasRequest){return false;}
    Slot=MoveTemp(PendingSlot);bLoad=bPendingLoad;bHasRequest=false;return true;
}
void UPFSessionGameInstance::ReturnToWorldMenu()
{
    if(!GetWorld()){return;}
    bHasRequest=false;PendingSlot.Reset();
    UGameplayStatics::OpenLevel(GetWorld(),TEXT("/Engine/Maps/Entry"),true,TEXT("game=/Script/PrimalFrontier.PFMainMenuGameMode"));
}
