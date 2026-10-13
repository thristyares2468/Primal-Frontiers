#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Crafting/PFHarvestCapabilities.h"

bool UPFInventoryComponent::ResolveHarvest(const FPFHarvestRequirement& R,int32& OutHits,FString& Error) const
{
    if(!PFHarvest::ValidRequirement(R) || !Catalog)
    {Error=PFHarvest::ValidRequirement(R)?TEXT("Missing item catalog"):PFHarvest::RequirementText(R);return false;}
    int32 Best=R.Family.IsValid()?0:1;
    const double Now=ServerTime(GetWorld());
    if(!FMath::IsFinite(Now)){Error=TEXT("Invalid harvest clock");return false;}
    for(const auto& Stack:GetStacks())
    {
        if(Stack.Quantity<=0 || !FMath::IsFinite(Stack.ExpiresAt) || Stack.ExpiresAt<0 || (Stack.ExpiresAt>0 && Stack.ExpiresAt<=Now)){continue;}
        const auto* D=Definition(Stack.ItemId);
        if(!D){continue;} // Unknown/invalid owned definitions confer no capability.
        if(!R.Family.IsValid() || (D->HarvestFamilies.HasTagExact(R.Family) && uint8(D->HarvestGrade)>=uint8(R.MinimumGrade)))
        {Best=FMath::Max(Best,D->GatheringHits);}
    }
    if(Best==0){Error=PFHarvest::RequirementText(R);return false;}
    OutHits=Best;Error.Reset();return true;
}
