#include "Hosting/PFHostingConfig.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHostingConfigTest,"PF.Hosting.AgentConfiguration",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFHostingConfigTest::RunTest(const FString&)
{
    const FString Valid=TEXT(R"({"heartbeatEndpoint":"localhost:56001","sessionHostId":"local-server-1","logFolder":"C:/Synthetic/Logs","sharedContentFolder":"C:/Synthetic/Shared","gameServerConnectionInfo":{"publicIpV4Adress":"127.0.0.1","gamePortsConfiguration":[{"name":"gameport","serverListeningPort":17996,"clientConnectionPort":17996}]}})");
    FPFHostingAgentConfig Config;FString Error;
    TestTrue(TEXT("Real vendor JSON field spelling and required gameport accepted"),FPFHostingAgentConfig::Parse(Valid,Config,Error));
    TestEqual(TEXT("Validated agent-selected port"),Config.ListenPort,17996);
    const TArray<FString> Invalid={TEXT(""),TEXT("{}"),TEXT("[1]"),Valid.Replace(TEXT("gameport"),TEXT("game_port")),
        Valid.Replace(TEXT("17996"),TEXT("0")),Valid.Replace(TEXT("17996"),TEXT("65536")),Valid.Replace(TEXT("17996"),TEXT("17996.5")),
        Valid.Replace(TEXT("localhost:56001"),TEXT("user@evil/route:56001")),Valid.Replace(TEXT("\"logFolder\":\"C:/Synthetic/Logs\""),TEXT("\"logFolder\":42")),
        Valid.Replace(TEXT("\"gameServerConnectionInfo\""),TEXT("\"wrong\"")),Valid.Replace(TEXT("\"sharedContentFolder\""),TEXT("\"wrong\"")),
        Valid.LeftChop(1)+TEXT(",\"buildMetadata\":{\"bad\":42}}"),FString::ChrN(65537,TEXT('x'))};
    for(int32 Index=0;Index<Invalid.Num();++Index)
    {
        Config.ListenPort=12345;
        TestFalse(FString::Printf(TEXT("Malformed config %d refused before vendor fatal parser"),Index),FPFHostingAgentConfig::Parse(Invalid[Index],Config,Error));
        TestEqual(TEXT("Failed parse preserves caller output"),Config.ListenPort,12345);
        TestFalse(TEXT("Refusal is explicit"),Error.IsEmpty());
    }
    return true;
}
#endif
