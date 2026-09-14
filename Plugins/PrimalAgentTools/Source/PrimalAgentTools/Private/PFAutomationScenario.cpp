#include "PFAutomationScenario.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "ScopedTransaction.h"

namespace PF::AgentTools
{
bool IsAutomationMap(const FString& PackageName)
{
    return PackageName == TEXT("/Game/PrimalFrontier/Maps/L_Automation") ||
        PackageName == TEXT("/Game/Maps/L_Automation"); // Existing project map; never moved automatically.
}

struct FActorSpec
{
    FName Name;
    UClass* Class;
    FVector Location;
    FRotator Rotation;
    FVector Scale;
};

static TArray<FActorSpec> Specs(bool bTestActorOnly)
{
    TArray<FActorSpec> Result = {
        {TEXT("PF_TestCube"), AStaticMeshActor::StaticClass(), FVector(0, 0, 100), FRotator::ZeroRotator, FVector(1)}
    };
    if (!bTestActorOnly)
    {
        Result.Add({TEXT("PF_TestFloor"), AStaticMeshActor::StaticClass(), FVector(0, 0, -25), FRotator::ZeroRotator, FVector(20, 20, 0.5)});
        Result.Add({TEXT("PF_TestStart"), APlayerStart::StaticClass(), FVector(-400, 0, 100), FRotator::ZeroRotator, FVector(1)});
        Result.Add({TEXT("PF_TestLight"), APointLight::StaticClass(), FVector(0, 0, 500), FRotator::ZeroRotator, FVector(1)});
    }
    return Result;
}

static FResult Apply(UWorld* World, bool bTestActorOnly)
{
    FResult Result(bTestActorOnly ? TEXT("PF.PlaceTestActor") : TEXT("PF.ResetAutomation"),
        World ? World->GetOutermost()->GetName() : FString());
    // -ExecCmds can run before the first Slate/Outliner paint. Updating actors then
    // can invalidate widget visibility while the initial layout is being built.
    if (GFrameCounter < 2)
    {
        Result.Add(TEXT("Error"), TEXT("EditorStartingUp"), Result.Scope,
            TEXT("Wait until the editor has ticked before changing fixtures. Use the automation suite for batch scenario work, not startup -ExecCmds mutations."));
        return Result;
    }
    if (!IsInGameThread() || !GEditor || GEditor->PlayWorld || !World || World->WorldType != EWorldType::Editor ||
        World != GEditor->GetEditorWorldContext().World() || !IsAutomationMap(Result.Scope) ||
        World->GetCurrentLevel() != World->PersistentLevel)
    {
        Result.Add(TEXT("Error"), TEXT("UnsafeScenarioContext"), Result.Scope,
            TEXT("Open L_Automation at an approved project path, select its persistent level and stop PIE first. No map is switched or saved automatically."));
        return Result;
    }
    const TArray<FActorSpec> Definitions = Specs(bTestActorOnly);
    TMap<FName, AActor*> Existing;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        for (const FActorSpec& Spec : Definitions)
        {
            const bool bOwnedRole = It->Tags.Contains(OwnerTag) && It->Tags.Contains(Spec.Name);
            if (It->GetFName() == Spec.Name || bOwnedRole)
            {
                if (!bOwnedRole || It->GetClass() != Spec.Class || It->GetLevel() != World->PersistentLevel || Existing.Contains(Spec.Name))
                {
                    Result.Add(TEXT("Error"), TEXT("ActorOwnershipConflict"), It->GetPathName(),
                        TEXT("Expected one owned actor of the exact native class per role. Unowned, duplicate or mismatched actors are left untouched."));
                }
                else { Existing.Add(Spec.Name, *It); }
            }
        }
    }
    // Preflight every role and dependency before making any changes.
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!Cube) { Result.Add(TEXT("Error"), TEXT("MissingTestMesh"), FString(), TEXT("Engine BasicShapes/Cube is unavailable.")); }
    if (Result.HasErrors()) { return Result; }

    const FScopedTransaction Transaction(NSLOCTEXT("PrimalAgentTools", "ResetScenario", "Primal Agent Tools: reset test actors"));
    World->PersistentLevel->Modify();
    int32 Created = 0;
    for (const FActorSpec& Spec : Definitions)
    {
        AActor* Actor = Existing.FindRef(Spec.Name);
        if (!Actor)
        {
            FActorSpawnParameters Params;
            Params.Name = Spec.Name;
            Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
            Params.OverrideLevel = World->PersistentLevel;
            Params.ObjectFlags |= RF_Transactional;
            Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            Actor = World->SpawnActor<AActor>(Spec.Class, FTransform(Spec.Rotation, Spec.Location, Spec.Scale), Params);
            if (!Actor)
            {
                Result.Add(TEXT("Error"), TEXT("SpawnFailed"), Spec.Name.ToString(), TEXT("Actor creation failed. Earlier operations in this transaction can be undone with Ctrl+Z."));
                break;
            }
            ++Created;
        }
        Actor->Modify();
        Actor->Tags.AddUnique(OwnerTag);
        Actor->Tags.AddUnique(Spec.Name); // Editor ownership/role metadata, not gameplay state.
        Actor->SetActorLabel(Spec.Name.ToString());
        Actor->SetFolderPath(TEXT("PrimalAgentTools"));
        Actor->SetIsSpatiallyLoaded(false);
        Actor->bIsEditorOnlyActor = true; // Test fixtures cannot enter cooked maps.
        Actor->SetActorHiddenInGame(false);
        Actor->SetActorEnableCollision(true);
        if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
        {
            UStaticMeshComponent* Component = MeshActor->GetStaticMeshComponent();
            Component->Modify();
            Component->SetMobility(EComponentMobility::Movable);
            Component->SetStaticMesh(Cube);
            Component->EmptyOverrideMaterials();
            Component->SetSimulatePhysics(false);
            Component->SetCollisionProfileName(TEXT("BlockAll"));
        }
        if (APointLight* Light = Cast<APointLight>(Actor))
        {
            Light->GetLightComponent()->Modify();
            Light->GetLightComponent()->SetMobility(EComponentMobility::Movable);
            Light->GetLightComponent()->SetIntensity(3000.0f);
            Light->GetLightComponent()->SetLightColor(FLinearColor::White);
            CastChecked<UPointLightComponent>(Light->GetLightComponent())->SetAttenuationRadius(2000.0f);
        }
        Actor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        Actor->SetActorTransform(FTransform(Spec.Rotation, Spec.Location, Spec.Scale), false, nullptr, ETeleportType::TeleportPhysics);
        Actor->PostEditChange();
        Actor->MarkPackageDirty();
    }
    GEditor->RedrawLevelEditingViewports();
    Result.Counts.Add(TEXT("actorsCreated"), Created);
    Result.Counts.Add(TEXT("rolesRequested"), Definitions.Num());
    Result.Add(TEXT("Info"), TEXT("UnsavedScenario"), Result.Scope,
        TEXT("Fixed native editor-only fixtures (no physics/randomness). Existing non-tool actors are preserved. Ctrl+Z undoes this operation; saving is manual. This is a deterministic fixture, not a deterministic simulation of unrelated map content."));
    return Result;
}

FResult ResetScenario(UWorld* World) { return Apply(World, false); }
FResult PlaceTestActor(UWorld* World) { return Apply(World, true); }
}
