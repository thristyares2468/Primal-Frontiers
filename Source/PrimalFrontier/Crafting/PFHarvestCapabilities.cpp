#include "Crafting/PFHarvestCapabilities.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_HarvestPick,"Harvest.Family.Pick");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_HarvestAxe,"Harvest.Family.Axe");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_HarvestSickle,"Harvest.Family.Sickle");

namespace PFHarvest
{
    FGameplayTag Pick(){return TAG_PF_HarvestPick;}
    FGameplayTag Axe(){return TAG_PF_HarvestAxe;}
    FGameplayTag Sickle(){return TAG_PF_HarvestSickle;}
    bool KnownFamily(FGameplayTag Family){return Family==Pick() || Family==Axe() || Family==Sickle();}
    static bool MechanicalGrade(EPFHarvestGrade Grade)
    {return uint8(Grade)>=uint8(EPFHarvestGrade::Wood) && uint8(Grade)<=uint8(EPFHarvestGrade::Deepforge);}
    bool ValidRequirement(const FPFHarvestRequirement& R)
    {
        if(!R.Family.IsValid()){return R.MinimumGrade==EPFHarvestGrade::None;}
        // Powered Tech requires a separate real charge contract; do not silently authorize it.
        return KnownFamily(R.Family) && MechanicalGrade(R.MinimumGrade);
    }
    bool ValidTool(const FGameplayTagContainer& Families,EPFHarvestGrade Grade,int32 Hits)
    {
        if(Families.IsEmpty()){return Grade==EPFHarvestGrade::None;}
        if(!MechanicalGrade(Grade) || Hits<1 || Hits>8 || Families.Num()>3){return false;}
        for(FGameplayTag Family:Families){if(!KnownFamily(Family)){return false;}}
        return true;
    }
    FString RequirementText(const FPFHarvestRequirement& R)
    {
        if(R.MinimumGrade==EPFHarvestGrade::Tech){return TEXT("Powered Tech harvesting NOT IMPLEMENTED");}
        if(!ValidRequirement(R)){return TEXT("Invalid harvest requirement");}
        if(!R.Family.IsValid()){return TEXT("Hand gathering permitted");}
        const TCHAR* Grades[]={TEXT("None"),TEXT("Wood"),TEXT("Stone"),TEXT("Copper"),TEXT("Iron"),TEXT("Diamond"),TEXT("Titanium"),TEXT("Deepforge Alloy")};
        const TCHAR* Family=R.Family==Pick()?TEXT("pick"):R.Family==Axe()?TEXT("axe"):TEXT("sickle");
        return FString::Printf(TEXT("Requires %s %s"),Grades[uint8(R.MinimumGrade)],Family);
    }
}
