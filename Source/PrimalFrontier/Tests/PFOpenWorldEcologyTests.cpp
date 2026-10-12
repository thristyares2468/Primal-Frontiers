#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Creatures/PFCreatureSpawnCatalog.h"
#include "Creatures/PFCreatureCatalog.h"
#include "PFAssetPaths.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFAuthoredEcologyTest,"PF.Creatures.AuthoredPolicy",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFAuthoredEcologyTest::RunTest(const FString&)
{
    auto* Policy=LoadObject<UPFCreatureSpawnCatalog>(nullptr,TEXT("/Game/PrimalFrontier/Creatures/DA_OpenWorldSpawnPolicies.DA_OpenWorldSpawnPolicies"));
    auto* Creatures=LoadObject<UPFCreatureCatalog>(nullptr,PFAssetPaths::CreatureCatalog);
    if(!TestNotNull(TEXT("Authored policy asset loads"),Policy) || !TestNotNull(TEXT("Existing creature catalog loads"),Creatures)){return false;}
    FString Error; TestTrue(TEXT("Whole authored policy validates"),Policy->Validate(*Creatures,Error));
    const auto& Defaults=GetDefault<UPFCreatureSpawnCatalog>()->Policies;
    TestEqual(TEXT("Three biome domains"),Policy->Policies.Num(),3);
    FName Id=TEXT("Unchanged");
    auto Day=FGameplayTag::RequestGameplayTag(TEXT("World.Time.Day"));
    auto Night=FGameplayTag::RequestGameplayTag(TEXT("World.Time.Night"));
    TestTrue(TEXT("Day shore chooses forager"),Policy->Choose(*Creatures,Defaults[0].Biome,Day,0,Id,Error));
    TestEqual(TEXT("Real authored shore definition"),Id,FName(TEXT("Creature_Forager")));
    Id=TEXT("Unchanged");
    TestFalse(TEXT("Day ridge is dormant"),Policy->Choose(*Creatures,Defaults[2].Biome,Day,0,Id,Error));
    TestEqual(TEXT("Dormant selection preserves output"),Id,FName(TEXT("Unchanged")));
    TestTrue(TEXT("Night ridge chooses prowler"),Policy->Choose(*Creatures,Defaults[2].Biome,Night,0,Id,Error));
    TestEqual(TEXT("Real authored ridge definition"),Id,FName(TEXT("Creature_Prowler")));
    AddInfo(TEXT("[PrimalAgentTools] Authored policy asset, day/night selection and dormant refusal verified"));
    return true;
}
#endif
