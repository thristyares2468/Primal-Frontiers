// PFCreature.h
//
// A greybox creature (sphere body + floating state label) driven by a simple
// server-only state machine that runs at 10 Hz:
//
//   Idle <-> Patrol         no survivor in sight: alternate idling and walking to
//                           four points around Home
//   Flee                    passive creature sees a survivor: run directly away
//   Chase -> Attack         hostile creature sees a survivor: move to them; within
//                           1.25 m stop and wind up for 0.6 s, then hit if still
//                           in range (1.45 m) and in sight; 1.2 s cooldown
//   (leash)                 hostile beyond 15 m from Home gives up and returns
//   Dead                    collision/movement off, one food loot pickup, removed
//                           after 12 s
//
// Movement uses the engine AIController + navmesh (never teleporting). Only
// Health, State (a Gameplay Tag), Target, bHostile and DefinitionId replicate;
// clients just render. Damage is accepted only on the server.
//
// History: M6 (0c2d935); 822092f renamed the dead tag symbol to
//          TAG_PF_CreatureDead to avoid a unity-build clash with the survivor's TAG_PF_Dead.
// Docs:    Docs/CREATURES_M6.md, Docs/DECISIONS.md (2026-09-29)

#pragma once
#include "GameFramework/Character.h"
#include "Creatures/PFCreatureCatalog.h"
#include "PFCreature.generated.h"
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS(Blueprintable)
class PRIMALFRONTIER_API APFCreature : public ACharacter
{
    GENERATED_BODY()

public:
    APFCreature();
    UPROPERTY() FGuid PersistentId;
    bool RestorePersistence(float SavedHealth, FVector SavedHome, float RemainingCorpse);

    /** Definition to use when spawned (copied to DefinitionId on the server at BeginPlay). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName CreatureId=TEXT("Creature_Forager");
    /** Optional override; DA_CreatureCatalog is loaded if unset. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TObjectPtr<UPFCreatureCatalog> Catalog;
    /** Replicated definition ID (what clients display). */
    UPROPERTY(Replicated, BlueprintReadOnly) FName DefinitionId;
    UPROPERTY(Replicated, BlueprintReadOnly) float Health=0;
    /** Creature.State.Idle / Patrol / Flee / Chase / Attack / Dead. */
    UPROPERTY(Replicated, BlueprintReadOnly) FGameplayTag State;
    /** Survivor currently being fled from or chased. */
    UPROPERTY(Replicated, BlueprintReadOnly) TObjectPtr<APawn> Target;
    UPROPERTY(Replicated, BlueprintReadOnly) bool bHostile=false;
    /** Spawn location: patrol centre and leash anchor (server only). */
    UPROPERTY(BlueprintReadOnly) FVector Home;

    bool IsDead() const { return Health<=0; }
    /** Living survivor within SightRange with an unobstructed line from the creature. */
    bool CanSee(APawn* Pawn) const;
    /** One server decision step (called from Tick; public so tests can step it directly). */
    void Think();
    const FPFCreatureDefinition* Definition() const;
    /** Server-only damage; dead creatures and non-finite/negative amounts are ignored. */
    virtual float TakeDamage(float Amount, const FDamageEvent& Event, AController* EventInstigator, AActor* Causer) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    /** Server: resolve the definition (destroy if invalid), set health/speed/Home. */
    virtual void BeginPlay() override;
    /** Think() on the server; label/body presentation on clients. */
    virtual void Tick(float DeltaSeconds) override;

private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY() TObjectPtr<UTextRenderComponent> Label;
    // World-time bookkeeping for the state machine.
    double NextDecision=0;  // next movement/re-path decision
    double AttackAt=0;      // when the current windup lands
    double NextAttack=0;    // attack cooldown end
    double LastSeen=0;      // last time the target was visible (1 s memory)
    int32 PatrolIndex=0;    // which of the four patrol points is next
    /** Change state, replicate promptly and log the transition. */
    void SetState(FGameplayTag NewState);
    /** Ask the AIController to path to the nearest navmesh point to Goal. */
    void MoveTo(FVector Goal);
    /** Enter the dead state, drop loot and schedule removal. */
    void Die();
};
