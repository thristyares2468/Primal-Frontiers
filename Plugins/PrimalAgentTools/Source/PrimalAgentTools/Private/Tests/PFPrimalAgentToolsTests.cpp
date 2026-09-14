#include "PFAssetChecks.h"
#include "PFAutomationScenario.h"
#include "PFResults.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Dom/JsonObject.h"
#include "Editor.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "HAL/FileManager.h"
#include "HAL/IConsoleManager.h"
#include "Interfaces/IPluginManager.h"
#include "LevelEditorViewport.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Parse.h"
#include "ModuleDescriptor.h"
#include "Serialization/JsonSerializer.h"
#include "Tests/AutomationCommon.h"

using namespace PF::AgentTools;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFPolicyTest, "PF.PrimalAgentTools.PolicyAndReports",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPFPolicyTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Project root permitted"), IsProjectRoot(TEXT("/Game")));
    TestTrue(TEXT("Project subtree permitted"), IsProjectRoot(TEXT("/Game/PrimalFrontier")));
    TestFalse(TEXT("Other mount refused"), IsProjectRoot(TEXT("/Engine")));
    TestFalse(TEXT("Root prefix confusion refused"), IsProjectRoot(TEXT("/GameOther")));
    TestFalse(TEXT("Traversal refused"), IsProjectRoot(TEXT("/Game/../Engine")));
    TestFalse(TEXT("Report traversal refused"), IsSafeLabel(TEXT("../outside")));
    TestFalse(TEXT("Absolute report path refused"), IsSafeLabel(TEXT("C:\\outside")));
    TestFalse(TEXT("Empty label refused"), IsSafeLabel(TEXT("")));
    TestTrue(TEXT("Normal label accepted"), IsSafeLabel(TEXT("Smoke_01")));
    TestFalse(TEXT("Development map refused"), IsAutomationMap(TEXT("/Game/Maps/L_Development")));
    TestFalse(TEXT("Same basename outside approved folder refused"), IsAutomationMap(TEXT("/Game/Other/L_Automation")));
    TestTrue(TEXT("Legacy map supported"), IsAutomationMap(TEXT("/Game/Maps/L_Automation")));
    TestTrue(TEXT("Canonical map supported"), IsAutomationMap(TEXT("/Game/PrimalFrontier/Maps/L_Automation")));
    TestTrue(TEXT("Null world refuses mutation"), ResetScenario(nullptr).HasErrors());

    FAssetData Mesh;
    Mesh.AssetClassPath = FTopLevelAssetPath(TEXT("/Script/Engine"), TEXT("StaticMesh"));
    TestEqual(TEXT("Static mesh prefix"), NamingPrefix(Mesh), FString(TEXT("SM_")));
    FAssetData Map;
    Map.AssetClassPath = FTopLevelAssetPath(TEXT("/Script/Engine"), TEXT("World"));
    TestEqual(TEXT("Established map prefix"), NamingPrefix(Map), FString(TEXT("L_")));

    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("PrimalAgentTools"));
    TestTrue(TEXT("Plugin discovered"), Plugin.IsValid());
    if (Plugin)
    {
        const FPluginDescriptor& Descriptor = Plugin->GetDescriptor();
        TestFalse(TEXT("No cookable plugin content"), Descriptor.bCanContainContent);
        TestEqual(TEXT("Two modules"), Descriptor.Modules.Num(), 2);
        if (Descriptor.Modules.Num() == 2)
        {
            TestTrue(TEXT("Editor host type"), Descriptor.Modules[1].Type == EHostType::Editor);
            TestTrue(TEXT("Shipping explicitly denied"), Descriptor.Modules[0].TargetConfigurationDenyList.Contains(EBuildConfiguration::Shipping));
        }
    }
    for (const TCHAR* Name : {TEXT("PF.ValidateAssets"), TEXT("PF.CheckNaming"), TEXT("PF.CheckReferences"),
        TEXT("PF.ResetAutomation"), TEXT("PF.PlaceTestActor"), TEXT("PF.CaptureScreenshot"), TEXT("PF.ExportResults")})
    {
        TestNotNull(Name, IConsoleManager::Get().FindConsoleObject(Name));
    }

    FResult Sample(TEXT("PF.TestFixture"), TEXT("/Game/PrimalFrontier"));
    Sample.Add(TEXT("Error"), TEXT("ExpectedSyntheticError"), TEXT("Example"), TEXT("Serialization fixture only; not a real project error."));
    TestEqual(TEXT("Errors cannot pass"), Sample.Status(), FString(TEXT("Failed")));
    TestTrue(TEXT("Invalid export label rejected"), ExportResults({Sample}, TEXT("../escape")).HasErrors());
    const FResult Export = ExportResults({Sample}, TEXT("PolicySerialization"));
    TestFalse(TEXT("Report written"), Export.HasErrors());
    if (Export.Artifacts.Num() == 1)
    {
        FString Text;
        TestTrue(TEXT("Report readable"), FFileHelper::LoadFileToString(Text, *Export.Artifacts[0]));
        TSharedPtr<FJsonObject> Object;
        const bool bParsed = FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Object);
        TestTrue(TEXT("Valid JSON"), bParsed && Object.IsValid());
        if (Object)
        {
            TestEqual(TEXT("Aggregate failure preserved"), Object->GetStringField(TEXT("status")), FString(TEXT("Failed")));
            TestEqual(TEXT("Schema version"), Object->GetIntegerField(TEXT("schemaVersion")), 2);
        }
        AddInfo(TEXT("Synthetic serialization fixture: ") + Export.Artifacts[0]);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFScenarioTest, "PF.PrimalAgentTools.ScenarioIdempotence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPFScenarioTest::RunTest(const FString& Parameters)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("PFRunScenarioTests")))
    {
        AddWarning(TEXT("Needs manual setup: run in an isolated editor with L_Automation and -PFRunScenarioTests. No map was changed."));
        return true;
    }
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World || !IsAutomationMap(World->GetOutermost()->GetName()))
    {
        AddError(TEXT("The isolated scenario test requires an approved L_Automation map loaded."));
        return false;
    }
    int32 UnownedBefore = 0;
    for (TActorIterator<AActor> It(World); It; ++It) { UnownedBefore += !It->Tags.Contains(OwnerTag); }
    const FResult First = ResetScenario(World);
    TestFalse(TEXT("Scenario created"), First.HasErrors());
    if (First.HasErrors()) { LogResult(First); return false; }
    TestTrue(TEXT("Reset console command dispatches after editor startup"),
        IConsoleManager::Get().ProcessUserConsoleInput(TEXT("PF.ResetAutomation"), *GLog, World));
    TestTrue(TEXT("Placement console command dispatches"),
        IConsoleManager::Get().ProcessUserConsoleInput(TEXT("PF.PlaceTestActor"), *GLog, World));
    AActor* Cube = nullptr;
    int32 Owned = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->Tags.Contains(OwnerTag))
        {
            ++Owned;
            TestTrue(TEXT("Fixture excluded from cooking"), It->IsEditorOnly());
            if (It->Tags.Contains(TEXT("PF_TestCube"))) { Cube = *It; }
        }
    }
    TestEqual(TEXT("Exactly four fixture actors"), Owned, 4);
    if (!TestNotNull(TEXT("Known cube exists"), Cube)) { return false; }
    Cube->SetActorLocation(FVector(999, 222, 777));
    const FResult Second = ResetScenario(World);
    TestFalse(TEXT("Reset succeeds"), Second.HasErrors());
    TestEqual(TEXT("Reset creates no duplicates"), Second.Counts.FindRef(TEXT("actorsCreated")), 0);
    TestTrue(TEXT("Reset restores fixed transform"), Cube->GetActorLocation().Equals(FVector(0, 0, 100)));
    TestTrue(TEXT("Reset transaction can be undone"), GEditor->UndoTransaction());
    TestTrue(TEXT("Undo restores prior transform"), Cube->GetActorLocation().Equals(FVector(999, 222, 777)));
    TestTrue(TEXT("Reset transaction can be redone"), GEditor->RedoTransaction());
    TestTrue(TEXT("Redo restores fixed transform"), Cube->GetActorLocation().Equals(FVector(0, 0, 100)));
    TestFalse(TEXT("Placement is repeatable"), PlaceTestActor(World).HasErrors());
    TestEqual(TEXT("Placement reuses cube"), PlaceTestActor(World).Counts.FindRef(TEXT("actorsCreated")), 0);

    Cube->Tags.Remove(OwnerTag);
    const FVector BeforeRefusal = Cube->GetActorLocation();
    TestTrue(TEXT("Unowned name collision refused"), ResetScenario(World).HasErrors());
    TestTrue(TEXT("Refused actor preserved"), Cube->GetActorLocation().Equals(BeforeRefusal));
    Cube->Tags.AddUnique(OwnerTag);
    int32 UnownedAfter = 0;
    for (TActorIterator<AActor> It(World); It; ++It) { UnownedAfter += !It->Tags.Contains(OwnerTag); }
    TestEqual(TEXT("Unrelated actors preserved"), UnownedAfter, UnownedBefore);
    FResult Verification(TEXT("PF.PrimalAgentTools.ScenarioIdempotence"), World->GetOutermost()->GetName());
    Verification.Counts.Add(TEXT("ownedActors"), Owned);
    Verification.Counts.Add(TEXT("unrelatedActorsPreserved"), UnownedAfter);
    if (HasAnyErrors()) { Verification.Add(TEXT("Error"), TEXT("AssertionFailed"), FString(), TEXT("See Unreal automation log.")); }
    LogResult(ExportResults({First, Second, Verification}, TEXT("ScenarioVerification")));
    return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FPFCaptureViewport, FAutomationTestBase*, Test);
bool FPFCaptureViewport::Update()
{
    const FResult Capture = CaptureScreenshot(TEXT("ViewportVerification"));
    Test->TestFalse(TEXT("Rendered viewport capture succeeds"), Capture.HasErrors());
    LogResult(Capture);
    if (!Capture.Artifacts.IsEmpty())
    {
        Test->TestTrue(TEXT("PNG contains data"), IFileManager::Get().FileSize(*Capture.Artifacts[0]) > 64);
    }
    LogResult(ExportResults({Capture}, TEXT("ViewportVerification")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFViewportTest, "PF.PrimalAgentTools.ViewportCapture",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPFViewportTest::RunTest(const FString& Parameters)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("PFRunViewportTest")))
    {
        AddWarning(TEXT("Needs manual setup: rendered isolated editor and -PFRunViewportTest. Screenshot test did not run."));
        return true;
    }
    if (GCurrentLevelEditingViewportClient)
    {
        GCurrentLevelEditingViewportClient->SetViewLocation(FVector(-700, -700, 550));
        GCurrentLevelEditingViewportClient->SetViewRotation(FRotator(-25, 45, 0));
        GCurrentLevelEditingViewportClient->Invalidate();
    }
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(3.0f));
    ADD_LATENT_AUTOMATION_COMMAND(FPFCaptureViewport(this));
    return true;
}
#endif
