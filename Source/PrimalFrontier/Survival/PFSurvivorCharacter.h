// PFSurvivorCharacter.h
//
// The playable first-person survivor. Extends the Epic first-person template
// character (APrimalFrontierCharacter) with:
//  - a UPFPlayerSurvivalComponent (vitals, death),
//  - stamina-gated jumping and movement lock on death,
//  - automatic respawn scheduling through APFSurvivalGameMode,
//  - placeholder gathering-tool meshes shown while the inventory holds Item_Tool.
//
// Presentation (meshes, anim BPs, input assets) is supplied by the Blueprint
// child Content/PrimalFrontier/Survival/BP_Survivor; this class holds the rules.
//
// History: M1 (f7ed11d) survival integration and respawn.
//          M4 (e7ffd71) replicated gathering-tool indicator.
//          M7 (b5a3165) camera pinned to the capsule eye height (see BeginPlay).
// Docs:    Docs/SURVIVAL_M1.md, Docs/GATHERING_CRAFTING_M4.md

#pragma once
#include "PrimalFrontierCharacter.h"
#include "PFSurvivorCharacter.generated.h"
class UPFPlayerSurvivalComponent;
class UStaticMeshComponent;

UCLASS()
class PRIMALFRONTIER_API APFSurvivorCharacter : public APrimalFrontierCharacter
{
    GENERATED_BODY()

public:
    APFSurvivorCharacter();

    /** Authoritative vitals. Recreated with each respawned pawn. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival") TObjectPtr<UPFPlayerSurvivalComponent> Survival;

    /** Stamina spent per jump; jumping is refused if the survivor can't afford it. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Survival", meta=(ClampMin="1")) float JumpStaminaCost = 20.f;

    /** Server: run Unreal's damage pipeline, then apply the accepted amount to Health.
     *  Returns the health actually lost (0 on clients, when dead, or if invulnerable). */
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

    /** Jump requires enough stamina in addition to the normal movement checks. */
    virtual bool CanJumpInternal_Implementation() const override;

    /** Server deducts JumpStaminaCost when a jump actually starts. */
    virtual void OnJumped_Implementation() override;

    /** Server: track whether the gathering tool is carried. Clients: show/hide tool meshes. */
    virtual void Tick(float DeltaSeconds) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    /** Re-attach the camera to the capsule and subscribe to death. */
    virtual void BeginPlay() override;

    /** Movement input is ignored while dead. */
    virtual void DoMove(float Right, float Forward) override;

    /** Freeze the body, disable collision and (on the server) schedule a respawn. */
    UFUNCTION() void HandleDeath();

private:
    /** Server-derived "carries Item_Tool" flag, replicated so remote players see the tool too. */
    UPROPERTY(Replicated) bool bHasGatheringTool=false;

    // Placeholder tool: two cubes in first person (owner only) and one cube in the
    // remote full-body mesh's right hand (everyone except the owner).
    UPROPERTY() TObjectPtr<UStaticMeshComponent> ToolHandle;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> ToolHead;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> RemoteTool;
};
