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
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName CreatureId=TEXT("Creature_Forager");
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TObjectPtr<UPFCreatureCatalog> Catalog;
    UPROPERTY(Replicated, BlueprintReadOnly) FName DefinitionId;
    UPROPERTY(Replicated, BlueprintReadOnly) float Health=0;
    UPROPERTY(Replicated, BlueprintReadOnly) FGameplayTag State;
    UPROPERTY(Replicated, BlueprintReadOnly) TObjectPtr<APawn> Target;
    UPROPERTY(Replicated, BlueprintReadOnly) bool bHostile=false;
    UPROPERTY(BlueprintReadOnly) FVector Home;
    bool IsDead() const { return Health<=0; }
    bool CanSee(APawn* Pawn) const;
    void Think();
    const FPFCreatureDefinition* Definition() const;
    virtual float TakeDamage(float Amount, const FDamageEvent& Event, AController* EventInstigator, AActor* Causer) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
private:
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Body;
    UPROPERTY() TObjectPtr<UTextRenderComponent> Label;
    double NextDecision=0;
    double AttackAt=0;
    double NextAttack=0;
    double LastSeen=0;
    int32 PatrolIndex=0;
    void SetState(FGameplayTag NewState);
    void MoveTo(FVector Goal);
    void Die();
};
