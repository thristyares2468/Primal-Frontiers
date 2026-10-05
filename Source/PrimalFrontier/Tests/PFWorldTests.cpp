#include "World/PFWorldClock.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldClockTest,"PF.World.Clock",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldClockTest::RunTest(const FString&)
{
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* Clock=F.GetTestWorld()->SpawnActor<APFWorldClock>();
    TestTrue(TEXT("Server time set"),Clock->SetHour(23));Clock->DayLengthSeconds=240;Clock->Advance(20);TestEqual(TEXT("Midnight wrap"),Clock->Hour,1.f);
    TestEqual(TEXT("Night tag"),Clock->Phase.ToString(),FString(TEXT("World.Time.Night")));Clock->SetHour(6);TestEqual(TEXT("Dawn tag"),Clock->Phase.ToString(),FString(TEXT("World.Time.Day")));
    TestFalse(TEXT("Out of range rejected"),Clock->SetHour(24));Clock->SetRole(ROLE_SimulatedProxy);TestFalse(TEXT("Client mutation rejected"),Clock->SetHour(12));Clock->Advance(30);TestEqual(TEXT("Client cannot advance"),Clock->Hour,6.f);
    Clock->SetRole(ROLE_Authority);Clock->DayLengthSeconds=0;Clock->Advance(30);TestEqual(TEXT("Invalid duration safe"),Clock->Hour,6.f);Clock->DayLengthSeconds=240;Clock->Advance(-2);TestEqual(TEXT("Negative time safe"),Clock->Hour,6.f);
    AddInfo(TEXT("[PrimalWorld] Midnight, phase thresholds, authority and invalid duration verified."));F.ForwardErrorMessages(this);return true;
}
#endif
