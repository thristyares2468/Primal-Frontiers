#include "Survival/PFSurvivalPlayerController.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"

void APFSurvivalPlayerController::ServerLearnKnowledge_Implementation(FName Id)
{
    if(!HasAuthority()){return;}
    auto* PS=GetPlayerState<APFInventoryPlayerState>();
    if(PS && PS->Progression){FString Error;PS->Progression->RequestKnowledge(Id,GetPawn(),Error);}
}
