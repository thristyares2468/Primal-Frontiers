// PFAssetChecks.cpp
//
// Read-only asset checks for the PF.* editor commands (ccbad56):
//   ValidateAssets   runs Unreal's DataValidation on saved assets under a /Game root.
//   CheckNaming      checks /Game/PrimalFrontier asset names against the project
//                    prefix table in NamingPrefix() (L_, BP_, SM_, M_, MI_, T_, DA_, ...).
//   CheckReferences  reports redirectors and missing or transient package references.
// Scope is limited to /Game roots (IsProjectRoot rejects other mounts and traversal).
// An empty scope is reported as a warning ("no coverage") rather than a pass.

#include "PFAssetChecks.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "EditorValidatorSubsystem.h"
#include "Engine/DataAsset.h"
#include "Engine/Blueprint.h"
#include "Misc/PackageName.h"

namespace PF::AgentTools
{
bool IsProjectRoot(const FString& Root)
{
    return (Root == TEXT("/Game") || Root.StartsWith(TEXT("/Game/"), ESearchCase::CaseSensitive)) &&
        !Root.EndsWith(TEXT("/")) && !Root.Contains(TEXT("..")) &&
        FPackageName::IsValidLongPackageName(Root / TEXT("PF_ScopeProbe"));
}

static bool Gather(const FString& Root, FResult& Result, TArray<FAssetData>& Assets)
{
    if (!GEditor || GEditor->PlayWorld || !IsInGameThread() || !IsProjectRoot(Root))
    {
        Result.Add(TEXT("Error"), TEXT("InvalidContextOrScope"), Root,
            TEXT("Run on the editor game thread outside PIE with /Game or a project subfolder."));
        return false;
    }
    IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    Registry.SearchAllAssets(true);
    Registry.GetAssetsByPath(FName(*Root), Assets, true, true);
    Assets.Sort([](const FAssetData& A, const FAssetData& B)
    {
        return A.GetSoftObjectPath().ToString() < B.GetSoftObjectPath().ToString();
    });
    Result.Counts.Add(TEXT("assetsDiscovered"), Assets.Num());
    if (Assets.IsEmpty())
    {
        Result.Add(TEXT("Warning"), TEXT("NoAssets"), Root, TEXT("No saved assets found; this check has no coverage. Create/save project content before relying on it."));
    }
    return true;
}

FString NamingPrefix(const FAssetData& Asset)
{
    FString BlueprintType;
    if (Asset.AssetClassPath.GetAssetName() == TEXT("Blueprint") &&
        Asset.GetTagValue(FBlueprintTags::BlueprintType, BlueprintType) && BlueprintType == TEXT("BPTYPE_Interface"))
    {
        return TEXT("BPI_");
    }
    // The project already uses L_, BP_, SM_, M_, MI_, T_, IA_, IMC_, etc.
    // Exact class rules precede the DataAsset inheritance fallback.
    static const TMap<FName, FString> Prefixes = {
        {TEXT("World"), TEXT("L_")}, {TEXT("Blueprint"), TEXT("BP_")},
        {TEXT("WidgetBlueprint"), TEXT("UI_")}, {TEXT("AnimBlueprint"), TEXT("ABP_")},
        {TEXT("StaticMesh"), TEXT("SM_")}, {TEXT("SkeletalMesh"), TEXT("SKM_")},
        {TEXT("Skeleton"), TEXT("SK_")}, {TEXT("PhysicsAsset"), TEXT("PHYS_")},
        {TEXT("Material"), TEXT("M_")}, {TEXT("MaterialInstanceConstant"), TEXT("MI_")},
        {TEXT("MaterialFunction"), TEXT("MF_")}, {TEXT("MaterialParameterCollection"), TEXT("MPC_")},
        {TEXT("Texture2D"), TEXT("T_")}, {TEXT("TextureCube"), TEXT("T_")},
        {TEXT("TextureRenderTarget2D"), TEXT("RT_")},
        {TEXT("AnimSequence"), TEXT("A_")}, {TEXT("AnimMontage"), TEXT("AM_")},
        {TEXT("BlendSpace"), TEXT("BS_")}, {TEXT("BlendSpace1D"), TEXT("BS_")},
        {TEXT("SoundWave"), TEXT("S_")}, {TEXT("SoundCue"), TEXT("SC_")},
        {TEXT("NiagaraSystem"), TEXT("NS_")}, {TEXT("NiagaraEmitter"), TEXT("NE_")},
        {TEXT("DataTable"), TEXT("DT_")}, {TEXT("CurveTable"), TEXT("CT_")},
        {TEXT("UserDefinedEnum"), TEXT("E_")}, {TEXT("UserDefinedStruct"), TEXT("F_")},
        {TEXT("InputAction"), TEXT("IA_")}, {TEXT("InputMappingContext"), TEXT("IMC_")},
        {TEXT("DataAsset"), TEXT("DA_")}, {TEXT("PrimaryDataAsset"), TEXT("DA_")}
    };
    if (const FString* Prefix = Prefixes.Find(Asset.AssetClassPath.GetAssetName())) { return *Prefix; }
    if (Asset.IsInstanceOf(UDataAsset::StaticClass(), EResolveClass::Yes)) { return TEXT("DA_"); }
    return FString();
}

FResult CheckNaming()
{
    FResult Result(TEXT("PF.CheckNaming"), TEXT("/Game/PrimalFrontier"));
    TArray<FAssetData> Assets;
    if (!Gather(Result.Scope, Result, Assets)) { return Result; }
    int32 Checked = 0;
    for (const FAssetData& Asset : Assets)
    {
        const FString Path = Asset.GetSoftObjectPath().ToString();
        if (Asset.IsRedirector())
        {
            Result.Add(TEXT("Warning"), TEXT("Redirector"), Path, TEXT("Redirector is not a normal named asset; inspect references before fixing it."));
            continue;
        }
        const FString Prefix = NamingPrefix(Asset);
        if (Prefix.IsEmpty())
        {
            Result.Add(TEXT("Warning"), TEXT("UnknownNamingRule"), Path,
                TEXT("No prefix rule for class ") + Asset.AssetClassPath.ToString());
            continue;
        }
        ++Checked;
        const FString Name = Asset.AssetName.ToString();
        if (!Name.StartsWith(Prefix, ESearchCase::CaseSensitive) || Name.Len() == Prefix.Len() || !IsSafeLabel(Name))
        {
            Result.Add(TEXT("Error"), TEXT("InvalidAssetName"), Path,
                TEXT("Expected ") + Prefix + TEXT("<Name>, ASCII letters/digits/underscore/hyphen, at most 64 characters."));
        }
    }
    Result.Counts.Add(TEXT("assetsChecked"), Checked);
    return Result;
}

FResult ValidateAssets(const FString& Root)
{
    FResult Result(TEXT("PF.ValidateAssets"), Root);
    TArray<FAssetData> Assets;
    if (!Gather(Root, Result, Assets) || Assets.IsEmpty()) { return Result; }
    UEditorValidatorSubsystem* Validator = GEditor->GetEditorSubsystem<UEditorValidatorSubsystem>();
    if (!Validator)
    {
        Result.Add(TEXT("Error"), TEXT("ValidatorUnavailable"), Root, TEXT("The DataValidation editor subsystem is unavailable."));
        return Result;
    }
    FValidateAssetsSettings Settings;
    Settings.ValidationUsecase = EDataValidationUsecase::Manual;
    Settings.bSkipExcludedDirectories = false;
    Settings.bCollectPerAssetDetails = true;
    Settings.bShowIfNoFailures = false;
    Settings.bSilent = true;
    Settings.ShowMessageLogSeverity.Reset();
    FValidateAssetsResults Results;
    Validator->ValidateAssetsWithSettings(Assets, Settings, Results);
    Result.Counts.Add(TEXT("requested"), Results.NumRequested);
    Result.Counts.Add(TEXT("externalObjects"), Results.NumExternalObjects);
    Result.Counts.Add(TEXT("checked"), Results.NumChecked);
    Result.Counts.Add(TEXT("valid"), Results.NumValid);
    Result.Counts.Add(TEXT("invalid"), Results.NumInvalid);
    Result.Counts.Add(TEXT("warnings"), Results.NumWarnings);
    Result.Counts.Add(TEXT("skipped"), Results.NumSkipped);
    Result.Counts.Add(TEXT("unableToValidate"), Results.NumUnableToValidate);
    TArray<FString> Paths;
    Results.AssetsDetails.GetKeys(Paths);
    Paths.Sort();
    for (const FString& Path : Paths)
    {
        const FValidateAssetsDetails& Details = Results.AssetsDetails[Path];
        for (const FText& Error : Details.ValidationErrors)
        {
            Result.Add(TEXT("Error"), TEXT("AssetValidation"), Path, Error.ToString());
        }
        for (const FText& Warning : Details.ValidationWarnings)
        {
            Result.Add(TEXT("Warning"), TEXT("AssetValidation"), Path, Warning.ToString());
        }
        if (Details.Result == EDataValidationResult::Invalid && Details.ValidationErrors.IsEmpty())
        {
            Result.Add(TEXT("Error"), TEXT("AssetInvalid"), Path, TEXT("Unreal's validator marked the asset invalid."));
        }
    }
    if (Results.NumInvalid > 0 && !Result.HasErrors())
    {
        Result.Add(TEXT("Error"), TEXT("ValidationFailed"), Root, TEXT("Unreal reported invalid assets; inspect the Output Log for tokenized details."));
    }
    if (Results.NumUnableToValidate || Results.NumSkipped || Results.bAssetLimitReached || Results.NumChecked == 0)
    {
        Result.Add(TEXT("Warning"), TEXT("IncompleteValidation"), Root,
            TEXT("Some assets were skipped or have no applicable validator, or a validation limit was reached. Counts record coverage."));
    }
    if (Results.NumWarnings > 0)
    {
        Result.Add(TEXT("Warning"), TEXT("ValidationWarnings"), Root, TEXT("Unreal reported validation warnings; inspect per-asset issues and Output Log."));
    }
    return Result;
}

FResult CheckReferences(const FString& Root)
{
    FResult Result(TEXT("PF.CheckReferences"), Root);
    TArray<FAssetData> Assets;
    if (!Gather(Root, Result, Assets)) { return Result; }
    IAssetRegistry& Registry = FModuleManager::GetModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
    TSet<FName> CheckedPackages;
    int32 DependencyCount = 0;
    for (const FAssetData& Asset : Assets)
    {
        if (Asset.IsRedirector())
        {
            Result.Add(TEXT("Warning"), TEXT("Redirector"), Asset.GetSoftObjectPath().ToString(),
                TEXT("Saved redirector found; no automatic deletion or fix-up is performed."));
        }
        if (CheckedPackages.Contains(Asset.PackageName)) { continue; }
        CheckedPackages.Add(Asset.PackageName);
        TArray<FName> Dependencies;
        Registry.GetDependencies(Asset.PackageName, Dependencies, UE::AssetRegistry::EDependencyCategory::Package);
        Dependencies.Sort(FNameLexicalLess());
        for (const FName Dependency : Dependencies)
        {
            const FString Package = Dependency.ToString();
            // Script imports are native modules, not disk asset packages.
            if (Package.StartsWith(TEXT("/Script/"))) { continue; }
            ++DependencyCount;
            if (Dependency == GetTransientPackage()->GetFName())
            {
                Result.Add(TEXT("Warning"), TEXT("TransientPackageReference"), Asset.PackageName.ToString(),
                    TEXT("Saved dependency points to Unreal's transient package; inspect the asset. This is not a missing disk asset."));
                continue;
            }
            if (!FPackageName::DoesPackageExist(Package))
            {
                Result.Add(TEXT("Error"), TEXT("MissingPackageReference"), Asset.PackageName.ToString(),
                    TEXT("Referenced package does not exist: ") + Package);
                continue;
            }
            TArray<FAssetData> Targets;
            Registry.GetAssetsByPackageName(Dependency, Targets, true);
            if (Targets.ContainsByPredicate([](const FAssetData& Target) { return Target.IsRedirector(); }))
            {
                Result.Add(TEXT("Warning"), TEXT("ReferenceToRedirector"), Asset.PackageName.ToString(),
                    TEXT("Reference passes through redirector package: ") + Package);
            }
        }
    }
    Result.Counts.Add(TEXT("packagesChecked"), CheckedPackages.Num());
    Result.Counts.Add(TEXT("packageReferencesChecked"), DependencyCount);
    Result.Add(TEXT("Info"), TEXT("ReferenceCoverage"), Root,
        TEXT("Checks saved hard/soft package dependencies and redirectors. Does not prove object/subobject paths inside existing packages or dynamically constructed paths; use asset validation and gameplay tests too."));
    return Result;
}
}
