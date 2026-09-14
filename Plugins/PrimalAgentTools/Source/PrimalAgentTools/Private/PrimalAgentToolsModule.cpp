#include "Modules/ModuleManager.h"
#include "Editor.h"
#include "PFCommands.h"
#include "PFAssetChecks.h"
#include "PFAutomationScenario.h"
#include "PFResults.h"

class FPrimalAgentToolsModule final : public IModuleInterface
{
public:
    void StartupModule() override
    {
#if WITH_EDITOR && !UE_BUILD_SHIPPING
        if (!GIsEditor || IsRunningGame()) { return; }
        PF::AgentTools::SetEditorCommand([](const FString& Name, const TArray<FString>& Args, UWorld* World)
        {
            using namespace PF::AgentTools;
            if (!World && GEditor) { World = GEditor->GetEditorWorldContext().World(); }
            const FString Root = Args.IsEmpty() ? TEXT("/Game") : Args[0];
            if (Name == TEXT("PF.ValidateAssets")) { return ValidateAssets(Root); }
            if (Name == TEXT("PF.CheckNaming")) { return CheckNaming(); }
            if (Name == TEXT("PF.CheckReferences")) { return CheckReferences(Root); }
            if (Name == TEXT("PF.ResetTestWorld") || Name == TEXT("PF.ResetAutomation")) { return ResetScenario(World); }
            if (Name == TEXT("PF.PlaceTestActor")) { return PlaceTestActor(World); }
            if (Name == TEXT("PF.CaptureTestScreenshot") || Name == TEXT("PF.CaptureScreenshot")) { return CaptureScreenshot(Args.IsEmpty() ? TEXT("Viewport") : Args[0]); }
            FResult Result(Name);
            if (Name == TEXT("PF.RunSmokeTest"))
            {
                for (const TCHAR* Check : {TEXT("PF.ValidateAssets"), TEXT("PF.CheckNaming"), TEXT("PF.CheckReferences")})
                {
                    const FResult Run = ExecuteCommand(Check, {}, World);
                    Result.Counts.Add(Check, Run.HasErrors() ? 1 : 0);
                    Result.Issues.Append(Run.Issues);
                    Result.bNotImplemented |= Run.bNotImplemented;
                }
                Result.Add(TEXT("Info"), TEXT("Coverage"), FString(), TEXT("Asset validation, naming and saved package references only. No gameplay acceptance coverage."));
                return Result;
            }
            Result.bNotImplemented = true;
            Result.Add(TEXT("Info"), TEXT("NOT IMPLEMENTED"), FString(), TEXT("No editor handler registered."));
            return Result;
        });
#endif
    }
    void ShutdownModule() override { PF::AgentTools::SetEditorCommand({}); }
};
IMPLEMENT_MODULE(FPrimalAgentToolsModule, PrimalAgentTools)
