#include "PFCommands.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/IConsoleManager.h"
#include "String/LexFromString.h"
#include "Modules/ModuleManager.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Engine/DamageEvents.h"
#include "Inventory/PFInventoryComponent.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Building/PFBuildPiece.h"
#include "Building/PFBuildingComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "EngineUtils.h"
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "World/PFWorldClock.h"

namespace PF::AgentTools
{
static FEditorCommand EditorCommand;
static TArray<FResult> History;
void SetEditorCommand(FEditorCommand Handler) { EditorCommand = MoveTemp(Handler); }
const TArray<FResult>& CommandHistory() { return History; }

const TArray<FCommandSpec>& CommandSpecs()
{
    static const TArray<FCommandSpec> Specs = {
        {TEXT("PF.Help"), TEXT("List all PF commands, usage and implementation status.")},
        {TEXT("PF.ValidateAssets"), TEXT("[/Game[/Subfolder]] Validate saved project assets without fixing or saving."), TEXT(""), 0, 1, true},
        {TEXT("PF.CheckNaming"), TEXT("Check naming under /Game/PrimalFrontier."), TEXT(""), 0, 0, true},
        {TEXT("PF.CheckReferences"), TEXT("[/Game[/Subfolder]] Report missing package references and redirectors."), TEXT(""), 0, 1, true},
        {TEXT("PF.ResetTestWorld"), TEXT("Reset only owned unsaved editor fixtures in approved L_Automation; runtime reset unavailable."), TEXT(""), 0, 0, true, true},
        {TEXT("PF.PlaceTestActor"), TEXT("Upsert owned PF_TestCube in approved editor L_Automation; no save."), TEXT(""), 0, 0, true, true},
        {TEXT("PF.RunSmokeTest"), TEXT("Run asset, naming and package-reference checks; no gameplay coverage."), TEXT(""), 0, 0, true},
        {TEXT("PF.ExportTestReport"), TEXT("[label] Export this process's execution history to Saved/AutomationReports."), TEXT(""), 0, 1},
        {TEXT("PF.CaptureTestScreenshot"), TEXT("[label] Capture rendered editor level viewport PNG."), TEXT(""), 0, 1, true},
        {TEXT("PF.GiveItem"), TEXT("ItemId Quantity: server-only grant to sole player's inventory, respecting capacity."), TEXT(""), 2, 2, false, true},
        {TEXT("PF.RemoveItem"), TEXT("ItemId Quantity: server-only removal from sole player's inventory."), TEXT(""), 2, 2, false, true},
        {TEXT("PF.SpawnCreature"), TEXT("CreatureId: server spawn near sole player, validated catalog/navigation/collision and world cap."), TEXT(""), 1, 1, false, true},
        {TEXT("PF.ResetCreatures"), TEXT("Server: clear test creatures and pause spawn points only in standalone L_M6Creatures."), TEXT(""), 0, 0, false, true},
        {TEXT("PF.SetHealth"), TEXT("Value: server-only survivor health, clamped to limits; cannot revive a dead pawn."), TEXT(""), 1, 1, false, true},
        {TEXT("PF.SetStamina"), TEXT("Value: server-only survivor stamina, clamped to limits."), TEXT(""), 1, 1, false, true},
        {TEXT("PF.Damage"), TEXT("Amount: apply positive damage through the server character damage API."), TEXT(""), 1, 1, false, true},
        {TEXT("PF.Kill"), TEXT("Apply lethal server damage to the sole controlled survivor."), TEXT(""), 0, 0, false, true},
        {TEXT("PF.Respawn"), TEXT("Respawn the sole dead survivor at a PlayerStart; living players are refused."), TEXT(""), 0, 0, false, true},
        {TEXT("PF.SetHunger"), TEXT("Value: server-only food reserve clamped 0..100; zero starves."), TEXT(""), 1, 1, false, true},
        {TEXT("PF.SetThirst"), TEXT("Value: server-only water reserve clamped 0..100; zero dehydrates."), TEXT(""), 1, 1, false, true},
        {TEXT("PF.SetExposure"), TEXT("Value: server-only test exposure 0..1; zero restores volume-only exposure."), TEXT(""), 1, 1, false, true},
        {TEXT("PF.RecoverNeeds"), TEXT("Server-only placeholder recovery of 35 food/water; no inventory grant."), TEXT(""), 0, 0, false, true},
        {TEXT("PF.SetTimeOfDay"), TEXT("Hour: 0 through 23; server-only set on the map's unique world clock."), TEXT(""), 1, 1, false, true},
        {TEXT("PF.Teleport"), TEXT("X Y Z [PlayerId]: authority-only character teleport; collision, floor, 1km bounds; one player unless ID given."), TEXT(""), 3, 4, false, true},
        {TEXT("PF.SaveWorld"), TEXT("Request gameplay save (unavailable); never saves editor maps."), TEXT("No gameplay persistence API exists."), 0, 0, false, true},
        {TEXT("PF.LoadWorld"), TEXT("Request gameplay load (unavailable)."), TEXT("No gameplay persistence API exists."), 0, 0, false, true},
        {TEXT("PF.TestGathering"), TEXT("Server: validate loaded resource definitions and node state; no harvesting."), TEXT(""), 0, 0},
        {TEXT("PF.TestCrafting"), TEXT("Server: validate player recipe catalogs and queue state; no item grants."), TEXT(""), 0, 0},
        {TEXT("PF.Craft"), TEXT("RecipeId: start the sole player's server-validated timed craft."), TEXT(""), 1, 1, false, true},
        {TEXT("PF.CancelCraft"), TEXT("Cancel the sole player's craft without consuming inputs."), TEXT(""), 0, 0, false, true},
        {TEXT("PF.TestBuildingPlacement"), TEXT("Server: validate existing structure health, ownership and support; functional tests run separately."), TEXT(""), 0, 0},
        {TEXT("PF.ResetBuildings"), TEXT("Server: remove sole player's empty runtime structures only in L_M5Building; no refund or map save."), TEXT(""), 0, 0, false, true},
        {TEXT("PF.TestCreatureAI"), TEXT("Server: inspect creature definitions, health, state and AI controllers; functional navigation tested separately.")},
        {TEXT("PF.TestMultiplayerReplication"), TEXT("Validate survival replication (unavailable)."), TEXT("No survival replication acceptance scenario exists; transport alone is not a pass.")},
        {TEXT("PF.TestPersistence"), TEXT("Validate persistence round trip (unavailable)."), TEXT("No gameplay persistence API exists.")},
        {TEXT("PF.ResetAutomation"), TEXT("Alias of PF.ResetTestWorld."), TEXT(""), 0, 0, true, true},
        {TEXT("PF.CaptureScreenshot"), TEXT("[label] Alias of PF.CaptureTestScreenshot."), TEXT(""), 0, 1, true},
        {TEXT("PF.ExportResults"), TEXT("[label] Alias of PF.ExportTestReport."), TEXT(""), 0, 1}
    };
    return Specs;
}

static bool Number(const FString& Text, double& Out)
{
    // LexTryParseString alone accepts numeric prefixes on some platforms.
    if (Text.IsEmpty() || !Text.IsNumeric()) { return false; }
    return LexTryParseString(Out, *Text) && FMath::IsFinite(Out);
}
FString ValidateArguments(const FCommandSpec& S, const TArray<FString>& A)
{
    if (A.Num() < S.MinArgs || A.Num() > S.MaxArgs) { return TEXT("Wrong argument count. Usage: ") + S.Name + TEXT(" ") + S.Help; }
    if (S.Name == TEXT("PF.GiveItem") || S.Name == TEXT("PF.RemoveItem"))
    {
        int64 Quantity = 0;
        if (!IsSafeLabel(A[0]) || !A[0].StartsWith(TEXT("Item_"))) { return TEXT("Item ID must be an Item_ identifier, not an object path."); }
        for (TCHAR C : A[1]) { if (C < '0' || C > '9') { return TEXT("Quantity must be a positive int32."); } }
        if (A[1].Len() > 10 || !LexTryParseString(Quantity, *A[1]) || Quantity <= 0 || Quantity > MAX_int32) { return TEXT("Quantity must be a positive int32."); }
    }
    if (S.Name == TEXT("PF.SpawnCreature") && !IsSafeLabel(A[0])) { return TEXT("Creature registry ID must be a simple identifier; arbitrary class paths are forbidden."); }
    if (S.Name == TEXT("PF.Craft") && (!IsSafeLabel(A[0]) || !A[0].StartsWith(TEXT("Recipe_")))) { return TEXT("Expected Recipe_ identifier, not an object path."); }
    if (S.Name.StartsWith(TEXT("PF.Set")) || S.Name == TEXT("PF.Teleport") || S.Name == TEXT("PF.Damage"))
    {
        const int32 Count = S.Name == TEXT("PF.Teleport") ? 3 : 1;
        for (int32 I = 0; I < Count; ++I)
        {
            double Value;
            if (!Number(A[I], Value)) { return TEXT("Expected a finite decimal number."); }
            if (S.Name == TEXT("PF.Damage") && (Value <= 0 || Value > MAX_flt)) { return TEXT("Damage must be positive and representable as a float."); }
            if (S.Name == TEXT("PF.SetTimeOfDay") && (Value < 0 || Value > 23)) { return TEXT("Hour must be between 0 and 23 inclusive."); }
            if (S.Name == TEXT("PF.SetExposure") && (Value < 0 || Value > 1)) { return TEXT("Exposure must be between 0 and 1 inclusive."); }
            if (S.Name == TEXT("PF.Teleport") && FMath::Abs(Value) > 100000) { return TEXT("Teleport coordinates must be within +/-100000 cm."); }
        }
        if (A.Num() == 4)
        {
            int64 Id;
            for (TCHAR C : A[3]) { if (C < '0' || C > '9') { return TEXT("PlayerId must be a nonnegative integer."); } }
            if (A[3].IsEmpty() || A[3].Len() > 10 || !LexTryParseString(Id, *A[3]) || Id < 0 || Id > MAX_int32) { return TEXT("Invalid PlayerId."); }
        }
    }
    if ((S.Name.Contains(TEXT("Export")) || S.Name.Contains(TEXT("Screenshot"))) && !A.IsEmpty() && !IsSafeLabel(A[0])) { return TEXT("Label must be 1-64 ASCII letters, digits, underscore or hyphen."); }
    return FString();
}

static void Fail(FResult& R, const TCHAR* Code, const FString& Message) { R.Add(TEXT("Error"), Code, FString(), Message); }
static FResult CreatureCommand(const FString& Name,const TArray<FString>& Args,UWorld* World)
{
    FResult R(Name);
    if(!World || !World->IsGameWorld() || World->GetNetMode()==NM_Client || !World->GetAuthGameMode())
    {Fail(R,TEXT("NotAuthority"),TEXT("Requires authoritative gameplay world."));return R;}
    if(Name==TEXT("PF.ResetCreatures"))
    {
        if(World->GetNetMode()!=NM_Standalone || World->GetOutermost()->GetName()!=TEXT("/Game/PrimalFrontier/Maps/L_M6Creatures"))
        {Fail(R,TEXT("ResetScope"),TEXT("Requires standalone L_M6Creatures test map."));return R;}
        for(TActorIterator<APFCreatureSpawner> It(World);It;++It){It->bAutoSpawn=false;}
        int32 Count=0;for(TActorIterator<APFCreature> It(World);It;++It){It->Destroy();++Count;}
        R.Add(TEXT("Info"),TEXT("CreaturesReset"),FString(),FString::Printf(TEXT("Removed %d runtime test creatures, paused spawners, no loot or map save."),Count));return R;
    }
    if(Name==TEXT("PF.SpawnCreature"))
    {
        auto* PC=World->GetFirstPlayerController();APawn* P=PC?PC->GetPawn():nullptr;
        if(World->GetNumPlayerControllers()!=1 || !P){Fail(R,TEXT("AmbiguousPlayer"),TEXT("Requires one possessed player."));return R;}
        const FVector At=P->GetActorLocation()+PC->GetControlRotation().Vector().GetSafeNormal2D()*400;
        if(auto* C=APFCreatureSpawner::Spawn(World,FName(*Args[0]),At)){R.Add(TEXT("Info"),TEXT("CreatureSpawned"),C->GetName(),Args[0]);}
        else{Fail(R,TEXT("SpawnRejected"),TEXT("Unknown definition, missing navigation, blocked position or eight-creature limit."));}return R;
    }
    int32 Count=0;for(TActorIterator<APFCreature> It(World);It;++It)
    {++Count;const auto* D=It->Definition();if(!D || !FMath::IsFinite(It->Health) || It->Health<0 || (D && It->Health>D->Health) || !It->State.IsValid() || (!It->IsDead() && !It->GetController())){Fail(R,TEXT("CreatureInvalid"),It->GetName());}}
    if(Count==0){Fail(R,TEXT("NoCreatures"),TEXT("Spawn a creature before integrity validation."));}
    R.Add(TEXT("Info"),TEXT("CreatureIntegrity"),FString(),FString::Printf(TEXT("Checked %d creatures; run PF.Creatures.Lifecycle/Live for functional behavior."),Count));return R;
}
static FResult BuildingCommand(const FString& Name,UWorld* World)
{
    FResult R(Name);if(!World || !World->IsGameWorld() || World->GetNetMode()==NM_Client || !World->GetAuthGameMode()){Fail(R,TEXT("NotAuthority"),TEXT("Requires authoritative gameplay world."));return R;}
    if(Name==TEXT("PF.ResetBuildings"))
    {
        if(World->GetOutermost()->GetName()!=TEXT("/Game/PrimalFrontier/Maps/L_M5Building") || World->GetNumPlayerControllers()!=1){Fail(R,TEXT("ResetScope"),TEXT("Requires standalone M5 test world and exactly one player."));return R;}
        auto* PC=World->GetFirstPlayerController();TArray<APFBuildPiece*> Owned;
        for(TActorIterator<APFBuildPiece> It(World);It;++It){if(It->Builder==PC->PlayerState){if(!It->Storage->GetStacks().IsEmpty()){Fail(R,TEXT("OccupiedStorage"),TEXT("Empty storage before reset."));return R;}Owned.Add(*It);}}
        for(auto* Piece:Owned){Piece->Destroy();}R.Add(TEXT("Info"),TEXT("BuildingsReset"),FString(),FString::Printf(TEXT("Removed %d owned runtime pieces; no saved map changes."),Owned.Num()));return R;
    }
    int32 Count=0;for(TActorIterator<APFBuildPiece> It(World);It;++It)
    {++Count;if(!IsValid(It->Builder) || !FMath::IsFinite(It->Health) || It->Health<=0 || (It->Kind!=EPFBuildKind::Foundation && !IsValid(It->Support))){Fail(R,TEXT("InvalidStructure"),It->GetName());}}
    if(Count==0){Fail(R,TEXT("NoStructures"),TEXT("Place a structure before validation."));}
    R.Add(TEXT("Info"),TEXT("StructureIntegrity"),FString(),FString::Printf(TEXT("Checked %d structures. Functional tests: PF.Building.PlacementAndStorage and PF.Building.Live."),Count));return R;
}
static FResult CraftingCommand(const FString& Name,const TArray<FString>& Args,UWorld* World)
{
    FResult R(Name);
    if(!World || !World->IsGameWorld() || World->GetNetMode()==NM_Client || !World->GetAuthGameMode())
    {Fail(R,TEXT("NotAuthority"),TEXT("Requires authoritative gameplay world."));return R;}
    if(Name==TEXT("PF.TestGathering"))
    {
        int32 Count=0;
        for(TActorIterator<APFResourceNode> It(World);It;++It)
        {
            ++Count;const auto* D=It->Catalog?It->Catalog->Resource(It->ResourceId,It->Items):nullptr;
            if(!D || It->HitsRemaining<0 || (D && It->HitsRemaining>D->Hits) || !FMath::IsFinite(It->RespawnAt) ||
                (It->HitsRemaining==0 && It->RespawnAt<=0) || (It->HitsRemaining>0 && It->RespawnAt!=0))
            {Fail(R,TEXT("InvalidResource"),It->GetPathName());}
        }
        if(Count==0){Fail(R,TEXT("NoResources"),TEXT("No runtime resource nodes to validate."));}
        R.Add(TEXT("Info"),TEXT("ResourceIntegrity"),FString(),FString::Printf(TEXT("Checked %d nodes. Functional gathering is covered by PF.Crafting.Gathering/Live."),Count));return R;
    }
    if(Name==TEXT("PF.TestCrafting"))
    {
        int32 Count=0;
        for(auto It=World->GetPlayerControllerIterator();It;++It)
        {
            auto* PS=It->Get()?It->Get()->PlayerState.Get():nullptr;
            auto* C=PS?PS->FindComponentByClass<UPFCraftingComponent>():nullptr;auto* I=C?C->Inventory():nullptr;
            if(!C || !C->Catalog || !I || C->Catalog->Recipes.IsEmpty()){Fail(R,TEXT("CraftingUnavailable"),TEXT("Missing player catalog/component."));continue;}
            ++Count;
            for(const auto& D:C->Catalog->Recipes){if(!C->Catalog->Recipe(D.Id,I->Catalog)){Fail(R,TEXT("InvalidRecipe"),D.Id.ToString());}}
            if(!FMath::IsFinite(C->FinishAt) || (C->ActiveRecipe.IsNone()?C->FinishAt!=0:C->FinishAt<=0)){Fail(R,TEXT("InvalidQueue"),PS->GetName());}
        }
        if(Count==0){Fail(R,TEXT("NoPlayers"),TEXT("No player crafting components to validate."));}
        R.Add(TEXT("Info"),TEXT("RecipeIntegrity"),FString(),FString::Printf(TEXT("Checked %d players. Functional transactions are covered by PF.Crafting.Transactions/Live."),Count));return R;
    }
    if(World->GetNumPlayerControllers()!=1){Fail(R,TEXT("AmbiguousPlayer"),TEXT("Craft developer commands require exactly one player."));return R;}
    auto* PC=World->GetFirstPlayerController();auto* C=PC && PC->PlayerState?PC->PlayerState->FindComponentByClass<UPFCraftingComponent>():nullptr;
    if(!C || !(Name==TEXT("PF.CancelCraft")?C->Cancel():C->Start(FName(*Args[0]),PC->GetPawn())))
    {Fail(R,TEXT("CraftRejected"),TEXT("Busy, missing ingredients, invalid recipe/life state or no active job."));}
    else{R.Add(TEXT("Info"),TEXT("CraftRequestAccepted"),FString(),C->Feedback);}
    return R;
}
static FResult InventoryCommand(const FString& Name,const TArray<FString>& Args,UWorld* World)
{
    FResult R(Name);
    if(!World || !World->IsGameWorld() || World->GetNetMode()==NM_Client || !World->GetAuthGameMode())
    {Fail(R,TEXT("NotAuthority"),TEXT("Requires server gameplay world."));return R;}
    if(World->GetNumPlayerControllers()!=1){Fail(R,TEXT("AmbiguousPlayer"),TEXT("Inventory developer commands require exactly one player."));return R;}
    const auto* PC=World->GetFirstPlayerController();
    auto* I=PC && PC->PlayerState ? PC->PlayerState->FindComponentByClass<UPFInventoryComponent>() : nullptr;
    int32 Count=0; LexTryParseString(Count,*Args[1]); const FName Id(*Args[0]);
    const bool Accepted=I && (Name==TEXT("PF.GiveItem") ? I->Grant(Id,Count) : I->RemoveItem(Id,Count));
    if(!Accepted){Fail(R,TEXT("InventoryRejected"),TEXT("Unknown item, capacity/quantity limit, insufficient items or unavailable inventory."));}
    else{R.Add(TEXT("Info"),TEXT("InventoryChanged"),Args[0],FString::Printf(TEXT("Quantity=%d total=%d weight=%.2f"),Count,I->Count(Id),I->GetWeight()));}
    return R;
}
static FResult SurvivalCommand(const FString& Name, const TArray<FString>& Args, UWorld* World)
{
    FResult R(Name);
    if (!World || !World->IsGameWorld() || World->GetNetMode() == NM_Client || !World->GetAuthGameMode())
    { Fail(R, TEXT("NotAuthority"), TEXT("Requires a server gameplay world.")); return R; }
    if (World->GetNumPlayerControllers() != 1)
    { Fail(R, TEXT("AmbiguousPlayer"), TEXT("This milestone's survival hooks require exactly one player controller.")); return R; }
    APlayerController* PC = World->GetFirstPlayerController();
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    UPFPlayerSurvivalComponent* Survival = Pawn ? Pawn->FindComponentByClass<UPFPlayerSurvivalComponent>() : nullptr;
    if (!Survival || !Pawn->HasAuthority())
    { Fail(R, TEXT("SurvivorUnavailable"), TEXT("Possessed survivor with an authoritative survival component is required.")); return R; }
    double NumberValue = 0;
    if (!Args.IsEmpty()) { Number(Args[0], NumberValue); }
    const float Value = static_cast<float>(FMath::Clamp(NumberValue, -static_cast<double>(MAX_flt), static_cast<double>(MAX_flt)));
    bool Accepted = false;
    if (Name == TEXT("PF.SetHealth")) { Accepted = Survival->SetHealth(Value); }
    else if (Name == TEXT("PF.SetHunger")) { Accepted = Survival->SetHunger(Value); }
    else if (Name == TEXT("PF.SetThirst")) { Accepted = Survival->SetThirst(Value); }
    else if (Name == TEXT("PF.SetExposure")) { Accepted = Survival->SetExposure(Value); }
    else if (Name == TEXT("PF.RecoverNeeds")) { Accepted = Survival->RecoverNeeds(35,35); }
    else if (Name == TEXT("PF.SetStamina")) { Accepted = Survival->ChangeStamina(FMath::Clamp(Value, 0.f, Survival->GetVitals().MaxStamina) - Survival->GetVitals().Stamina); }
    else if (Name == TEXT("PF.Damage") || Name == TEXT("PF.Kill"))
    { Accepted = Pawn->TakeDamage(Name == TEXT("PF.Kill") ? Survival->GetVitals().MaxHealth : Value, FDamageEvent(), PC, Pawn) > 0.f; }
    else if (APFSurvivalGameMode* Mode = World->GetAuthGameMode<APFSurvivalGameMode>()) { Accepted = Mode->RespawnPlayer(PC); }
    if (!Accepted) { Fail(R, TEXT("SurvivalRequestRejected"), TEXT("Invalid life state or unavailable survival GameMode; no successful mutation reported.")); }
    else
    {
        const UPFPlayerSurvivalComponent* Current = PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
        const auto V = Current->GetVitals();
        R.Add(TEXT("Info"), TEXT("AuthoritativeVitals"), PC->GetPawn()->GetName(), FString::Printf(TEXT("Health=%.1f Stamina=%.1f Hunger=%.1f Thirst=%.1f Exposure=%.2f Dead=%d"), V.Health, V.Stamina, V.Hunger, V.Thirst, V.Exposure, Current->IsDead()));
    }
    return R;
}
static FResult Teleport(const TArray<FString>& Args, UWorld* World)
{
    FResult R(TEXT("PF.Teleport"));
    APlayerController* Target = nullptr;
    int32 PlayerId = INDEX_NONE;
    if (Args.Num() == 4) { LexTryParseString(PlayerId, *Args[3]); }
    for (auto It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC || (PlayerId != INDEX_NONE && (!PC->PlayerState || PC->PlayerState->GetPlayerId() != PlayerId))) { continue; }
        if (Target) { Fail(R, TEXT("AmbiguousPlayer"), TEXT("Multiple players: supply the authoritative PlayerState PlayerId as fourth argument.")); return R; }
        Target = PC;
    }
    ACharacter* Character = Target ? Cast<ACharacter>(Target->GetPawn()) : nullptr;
    if (!Character || !Character->HasAuthority()) { Fail(R, TEXT("PlayerUnavailable"), TEXT("An authoritative possessed ACharacter is required.")); return R; }
    double X, Y, Z; Number(Args[0], X); Number(Args[1], Y); Number(Args[2], Z);
    const FVector Destination(X, Y, Z);
    UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
    FCollisionQueryParams Query(SCENE_QUERY_STAT(PFDevTeleport), false, Character);
    const FCollisionShape Shape = FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight());
    FHitResult Floor;
    if (Z <= World->GetWorldSettings()->KillZ + Capsule->GetScaledCapsuleHalfHeight() ||
        World->OverlapBlockingTestByProfile(Destination, FQuat::Identity, Capsule->GetCollisionProfileName(), Shape, Query) ||
        !World->LineTraceSingleByChannel(Floor, Destination, Destination - FVector(0, 0, 10000), ECC_Visibility, Query) ||
        Floor.ImpactNormal.Z < 0.5f)
    {
        Fail(R, TEXT("UnsafeDestination"), TEXT("Destination overlaps blocking collision, is below KillZ, or lacks walkable ground within 100m.")); return R;
    }
    if (!Character->TeleportTo(Destination, Character->GetActorRotation(), false, false)) { Fail(R, TEXT("TeleportRejected"), TEXT("Unreal collision-aware TeleportTo rejected the destination.")); return R; }
    Character->GetCharacterMovement()->StopMovementImmediately();
    Character->ForceNetUpdate();
    R.Add(TEXT("Info"), TEXT("Teleported"), Character->GetPathName(), Character->GetActorLocation().ToString());
    return R;
}

FResult ExecuteCommand(const FString& Name, const TArray<FString>& Args, UWorld* World, bool bLog)
{
    FResult R(Name);
#if !UE_BUILD_SHIPPING
    const FCommandSpec* S = CommandSpecs().FindByPredicate([&](const FCommandSpec& Entry) { return Entry.Name == Name; });
    const FString Map = World ? World->GetOutermost()->GetName() : TEXT("Unavailable");
    const FString Timestamp = R.StartedUtc;
    const FString Error = S ? ValidateArguments(*S, Args) : TEXT("Unknown PF command.");
    if (!IsInGameThread()) { Fail(R, TEXT("WrongThread"), TEXT("Commands require the game thread.")); return R; }
    if (!Error.IsEmpty()) { Fail(R, TEXT("InvalidArguments"), Error); }
    else if (World && World->GetNetMode() == NM_Client && S->bMutation) { Fail(R, TEXT("NotAuthority"), TEXT("Run this command on the server console. Clients cannot forward or execute mutations.")); }
    else if (!S->Blocker.IsEmpty()) { R.bNotImplemented = true; R.Add(TEXT("Info"), TEXT("NOT IMPLEMENTED"), FString(), S->Blocker); }
    else if (Name == TEXT("PF.Help"))
    {
        for (const FCommandSpec& Entry : CommandSpecs())
        {
            const FString Status = !Entry.Blocker.IsEmpty() ? TEXT("NOT IMPLEMENTED") : Entry.bEditor ? (EditorCommand ? TEXT("IMPLEMENTED (editor only)") : TEXT("UNAVAILABLE (editor only)")) : TEXT("IMPLEMENTED");
            R.Add(TEXT("Info"), *Status, Entry.Name, Entry.Help + (Entry.Blocker.IsEmpty() ? TEXT("") : TEXT(" ") + Entry.Blocker));
        }
    }
    else if (Name == TEXT("PF.ExportTestReport") || Name == TEXT("PF.ExportResults"))
    {
        R.Map = Map;
        R.Add(TEXT("Info"), TEXT("ExportRequested"), FString(), TEXT("Snapshot includes this export request; artifact success is recorded in subsequent history."));
        TArray<FResult> Snapshot = History;
        Snapshot.Add(R);
        R = ExportResults(Snapshot, Args.IsEmpty() ? TEXT("Results") : Args[0]);
    }
    else if (S->bEditor)
    {
        if (EditorCommand && (!World || World->WorldType == EWorldType::Editor)) { R = EditorCommand(Name, Args, World); }
        else { R.bNotImplemented = true; R.Add(TEXT("Info"), TEXT("NOT IMPLEMENTED"), FString(), TEXT("Requires editor backend outside PIE. Runtime gameplay reset/screenshot adapter is unavailable.")); }
    }
    else if (Name == TEXT("PF.SetHealth") || Name == TEXT("PF.SetStamina") || Name == TEXT("PF.Damage") || Name == TEXT("PF.Kill") || Name == TEXT("PF.Respawn") ||
        Name == TEXT("PF.SetHunger") || Name == TEXT("PF.SetThirst") || Name == TEXT("PF.SetExposure") || Name == TEXT("PF.RecoverNeeds"))
    { R = SurvivalCommand(Name, Args, World); }
    else if (Name == TEXT("PF.GiveItem") || Name == TEXT("PF.RemoveItem"))
    { R=InventoryCommand(Name,Args,World); }
    else if(Name==TEXT("PF.TestBuildingPlacement") || Name==TEXT("PF.ResetBuildings")) { R=BuildingCommand(Name,World); }
    else if(Name==TEXT("PF.SetTimeOfDay"))
    {
        if(!World || !World->IsGameWorld() || World->GetNetMode()==NM_Client || !World->GetAuthGameMode()){Fail(R,TEXT("NotAuthority"),TEXT("Requires authoritative gameplay world."));}
        else
        {
            APFWorldClock* Clock=nullptr;int32 Count=0;for(TActorIterator<APFWorldClock> It(World);It;++It){Clock=*It;++Count;}
            if(Count!=1 || !Clock->SetHour(FCString::Atof(*Args[0]))){Fail(R,TEXT("WorldClockMissing"),TEXT("Requires exactly one valid map clock."));}
            else{R.Add(TEXT("Info"),TEXT("TimeChanged"),Clock->GetName(),FString::Printf(TEXT("Server time %.2f"),Clock->Hour));}
        }
    }
    else if(Name==TEXT("PF.SpawnCreature") || Name==TEXT("PF.ResetCreatures") || Name==TEXT("PF.TestCreatureAI")) { R=CreatureCommand(Name,Args,World); }
    else if(Name==TEXT("PF.TestGathering") || Name==TEXT("PF.TestCrafting") || Name==TEXT("PF.Craft") || Name==TEXT("PF.CancelCraft"))
    {R=CraftingCommand(Name,Args,World);}
    else if (Name == TEXT("PF.Teleport"))
    {
        if (!World || !World->IsGameWorld() || !World->GetAuthGameMode()) { Fail(R, TEXT("AuthorityWorldUnavailable"), TEXT("Requires a running authoritative gameplay world and GameMode.")); }
        else { R = Teleport(Args, World); }
    }
    R.Command = Name; R.Map = Map; R.StartedUtc = Timestamp;
    R.Add(TEXT("Info"), TEXT("Arguments"), FString(), FString::Join(Args, TEXT(" ")));
    History.Add(R);
    if (bLog) { LogResult(R); }
#else
    Fail(R, TEXT("ShippingDisabled"), TEXT("Developer commands are excluded from Shipping."));
#endif
    return R;
}
}

class FPrimalAgentToolsRuntimeModule final : public IModuleInterface
{
    TArray<IConsoleObject*> Commands;
public:
    void StartupModule() override
    {
#if !UE_BUILD_SHIPPING
        for (const PF::AgentTools::FCommandSpec& S : PF::AgentTools::CommandSpecs())
        {
            const FString Name = S.Name;
            Commands.Add(IConsoleManager::Get().RegisterConsoleCommand(*Name, *S.Help,
                FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([Name](const TArray<FString>& Args, UWorld* World)
                { PF::AgentTools::ExecuteCommand(Name, Args, World); }), ECVF_Default));
        }
        UE_LOG(LogPrimalAgentTools, Display, TEXT("[PrimalAgentTools] Registered %d non-Shipping commands. PF.Help lists status. No network service or client RPC."), Commands.Num());
#endif
    }
    void ShutdownModule() override
    {
        for (IConsoleObject* Command : Commands) { IConsoleManager::Get().UnregisterConsoleObject(Command, false); }
        Commands.Empty();
        PF::AgentTools::SetEditorCommand({});
    }
};
IMPLEMENT_MODULE(FPrimalAgentToolsRuntimeModule, PrimalAgentToolsRuntime)

