#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Misc/AutomationTest.h"
#include "HAL/IConsoleManager.h"
#include "Scalability.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFShadowScalabilityTest,"PF.Settings.ShadowBudget",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFShadowScalabilityTest::RunTest(const FString&)
{
    auto* Directional=IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.Virtual.ResolutionLodBiasDirectional"));
    auto* Moving=IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.Virtual.ResolutionLodBiasDirectionalMoving"));
    auto* Pages=IConsoleManager::Get().FindConsoleVariable(TEXT("r.Shadow.Virtual.MaxPhysicalPages"));
    if(!TestNotNull(TEXT("Directional shadow cvar"),Directional) || !TestNotNull(TEXT("Moving shadow cvar"),Moving) || !TestNotNull(TEXT("Page budget cvar"),Pages)){return false;}
    auto* Quality=IConsoleManager::Get().FindConsoleVariable(TEXT("sg.ShadowQuality"));
    if(!TestNotNull(TEXT("Shadow group cvar"),Quality)){return false;}
    const int32 OriginalQuality=Quality->GetInt();
    const float OriginalBias=Directional->GetFloat(),OriginalMoving=Moving->GetFloat();const int32 OriginalPages=Pages->GetInt();
    const int32 Budgets[]={512,512,2048,4096};const float Biases[]={1,1,0,-1.5f};
    for(const int32 Tier:{0,1,2,3})
    {
        // The supported group's callback applies only shadow CVars. Forcing all
        // groups would collide with explicit blur/DOF overrides unrelated to this test.
        Quality->SetWithCurrentPriority(Tier);
        TestEqual(FString::Printf(TEXT("Tier%d stationary direction applies actual profile"),Tier),Directional->GetFloat(),Biases[Tier]);
        TestEqual(FString::Printf(TEXT("Tier%d moving direction applies actual profile"),Tier),Moving->GetFloat(),Biases[Tier]);
        TestEqual(FString::Printf(TEXT("Tier%d retains inherited page budget"),Tier),Pages->GetInt(),Budgets[Tier]);
    }
    Quality->SetWithCurrentPriority(OriginalQuality);
    TestEqual(TEXT("Original directional bias restored"),Directional->GetFloat(),OriginalBias);
    TestEqual(TEXT("Original moving bias restored"),Moving->GetFloat(),OriginalMoving);
    TestEqual(TEXT("Original page budget restored"),Pages->GetInt(),OriginalPages);
    AddInfo(TEXT("[PrimalSettings] Actual quality switches reduce low/medium shadow detail without expanding page allocations; high tiers and prior session restored."));return true;
}
#endif
