#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "WorldPartition/WorldPartition.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/GameModeBase.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFOpenWorldTest,"PF.World.OpenWorldAsset",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFOpenWorldTest::RunTest(const FString&)
{
    UWorld* World=LoadObject<UWorld>(nullptr,TEXT("/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld.L_PrimalFrontier_OpenWorld"));
    if(!TestNotNull(TEXT("Open-world map loads"),World)){return false;}
    auto* Partition=World->GetWorldPartition();
    if(!TestNotNull(TEXT("Real World Partition world"),Partition)){return false;}
    TestTrue(TEXT("Runtime streaming enabled"),Partition->IsStreamingEnabled());
    TestNotNull(TEXT("Survival GameMode retained"),World->GetWorldSettings()->DefaultGameMode.Get());
    AddInfo(TEXT("[PrimalWorld] Open-world asset, World Partition and runtime streaming checked; this test does not certify traversal."));
    return true;
}
#endif
