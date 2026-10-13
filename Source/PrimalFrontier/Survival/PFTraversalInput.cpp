#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"

void APFSurvivalPlayerController::StartSprint(){if(auto* P=Cast<APFSurvivorCharacter>(GetPawn())){P->SetSprintHeld(true);}}
void APFSurvivalPlayerController::StopSprint(){if(auto* P=Cast<APFSurvivorCharacter>(GetPawn())){P->SetSprintHeld(false);}}
void APFSurvivalPlayerController::Climb(){if(auto* P=Cast<APFSurvivorCharacter>(GetPawn())){if(!P->IsTraversalInputBlocked()){P->ServerClimb();}}}
void APFSurvivalPlayerController::Glide(){if(auto* P=Cast<APFSurvivorCharacter>(GetPawn())){P->ToggleGlide();}}
void APFSurvivalPlayerController::ClearTraversalInput(){if(auto* P=Cast<APFSurvivorCharacter>(GetPawn())){P->CancelTraversal();}}
void APFSurvivalPlayerController::NotifyTraversalFeedback(bool bAccepted,const FString& Reason)
{
    if(HasAuthority()){ClientInventoryFeedback(FString::Printf(TEXT("Climb %s: %s"),bAccepted?TEXT("accepted"):TEXT("refused"),*Reason));}
}
