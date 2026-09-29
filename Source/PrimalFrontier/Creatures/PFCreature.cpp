#include "Creatures/PFCreature.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFItemPickup.h"
#include "Inventory/PFItemCatalog.h"
#include "AIController.h"
#include "Camera/PlayerCameraManager.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Idle,"Creature.State.Idle");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Patrol,"Creature.State.Patrol");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Flee,"Creature.State.Flee");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Chase,"Creature.State.Chase");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Attack,"Creature.State.Attack");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Dead,"Creature.State.Dead");

APFCreature::APFCreature()
{
    bReplicates=true; SetReplicateMovement(true); PrimaryActorTick.bCanEverTick=true; PrimaryActorTick.TickInterval=0.1f;
    GetCapsuleComponent()->InitCapsuleSize(38,50);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
    GetCapsuleComponent()->SetCanEverAffectNavigation(false);
    GetCharacterMovement()->bOrientRotationToMovement=true; bUseControllerRotationYaw=false;
    AIControllerClass=AAIController::StaticClass(); AutoPossessAI=EAutoPossessAI::PlacedInWorldOrSpawned;
    Body=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GreyboxBody")); Body->SetupAttachment(GetRootComponent());
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Shape(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    Body->SetStaticMesh(Shape.Object); Body->SetRelativeScale3D(FVector(0.9,0.65,0.7)); Body->SetCollisionEnabled(ECollisionEnabled::NoCollision); Body->SetCanEverAffectNavigation(false);
    Label=CreateDefaultSubobject<UTextRenderComponent>(TEXT("StateLabel")); Label->SetupAttachment(GetRootComponent()); Label->SetRelativeLocation(FVector(0,0,85));Label->SetWorldSize(22);Label->SetHorizontalAlignment(EHTA_Center);
}
const FPFCreatureDefinition* APFCreature::Definition() const {return Catalog?Catalog->Find(DefinitionId):nullptr;}
void APFCreature::BeginPlay()
{
    Super::BeginPlay();
    if(!Catalog){Catalog=LoadObject<UPFCreatureCatalog>(nullptr,TEXT("/Game/PrimalFrontier/Creatures/DA_CreatureCatalog.DA_CreatureCatalog"));}
    if(HasAuthority())
    {
        DefinitionId=CreatureId; const auto* D=Definition();
        if(!D){UE_LOG(LogPFSurvival,Error,TEXT("[PrimalCreatures] Invalid definition %s"),*CreatureId.ToString());Destroy();return;}
        Health=D->Health;bHostile=D->bHostile;Home=GetActorLocation();GetCharacterMovement()->MaxWalkSpeed=D->Speed;SetState(TAG_PF_Idle);NextDecision=GetWorld()->GetTimeSeconds()+2;
    }
}
void APFCreature::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(APFCreature,DefinitionId);DOREPLIFETIME(APFCreature,Health);DOREPLIFETIME(APFCreature,State);DOREPLIFETIME(APFCreature,Target);DOREPLIFETIME(APFCreature,bHostile);
}
void APFCreature::SetState(FGameplayTag NewState)
{
    if(State!=NewState){State=NewState;ForceNetUpdate();UE_LOG(LogPFSurvival,Display,TEXT("[PrimalCreatures] %s state=%s health=%.0f"),*GetName(),*State.ToString(),Health);}
}
bool APFCreature::CanSee(APawn* Pawn) const
{
    const auto* D=Definition(); const auto* V=IsValid(Pawn)?Pawn->FindComponentByClass<UPFPlayerSurvivalComponent>():nullptr;
    if(!D || !V || V->IsDead() || Pawn->GetWorld()!=GetWorld() || FVector::DistSquared(GetActorLocation(),Pawn->GetActorLocation())>FMath::Square(D->SightRange)){return false;}
    FHitResult Hit; const FVector Start=GetActorLocation()+FVector(0,0,30);
    return !GetWorld()->LineTraceSingleByChannel(Hit,Start,Pawn->GetActorLocation(),ECC_Visibility,FCollisionQueryParams(SCENE_QUERY_STAT(PFCreatureSight),false,this)) || Hit.GetActor()==Pawn;
}
void APFCreature::MoveTo(FVector Goal)
{
    auto* AI=Cast<AAIController>(GetController());auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());FNavLocation Projected;
    if(AI && Nav && Nav->ProjectPointToNavigation(Goal,Projected,FVector(250,250,300)))
    {AI->MoveToLocation(Projected.Location,35,true,true,false,true,nullptr,false);}
}
void APFCreature::Think()
{
    if(!HasAuthority() || IsDead()){return;} const auto* D=Definition();if(!D){return;}const double Now=GetWorld()->GetTimeSeconds();
    APawn* Nearest=nullptr;double Best=FMath::Square(D->SightRange);
    for(auto It=GetWorld()->GetPlayerControllerIterator();It;++It)
    {APawn* P=It->Get()?It->Get()->GetPawn():nullptr;if(CanSee(P)){const double Dist=FVector::DistSquared(GetActorLocation(),P->GetActorLocation());if(Dist<=Best){Nearest=P;Best=Dist;}}}
    if(Nearest){Target=Nearest;LastSeen=Now;}
    else if(!IsValid(Target) || Now-LastSeen>1 || FVector::DistSquared(Home,GetActorLocation())>FMath::Square(1500.f)){Target=nullptr;}
    auto* AI=Cast<AAIController>(GetController());
    if(State==TAG_PF_Attack)
    {
        if(Now<AttackAt){return;}
        if(CanSee(Target) && FVector::DistSquared(GetActorLocation(),Target->GetActorLocation())<=FMath::Square(145.f))
        {Target->TakeDamage(D->Damage,FDamageEvent(),GetController(),this);UE_LOG(LogPFSurvival,Display,TEXT("[PrimalCreatures] Hit survivor damage=%.0f"),D->Damage);}
        NextAttack=Now+1.2;SetState(TAG_PF_Idle);
    }
    if(IsValid(Target) && CanSee(Target))
    {
        if(!bHostile)
        {SetState(TAG_PF_Flee);if(Now>=NextDecision){FVector Away=(GetActorLocation()-Target->GetActorLocation()).GetSafeNormal2D();if(Away.IsNearlyZero()){Away=FVector(1,0,0);}MoveTo(GetActorLocation()+Away*450);NextDecision=Now+0.7;}return;}
        if(FVector::DistSquared(Home,GetActorLocation())>FMath::Square(1500.f)){Target=nullptr;MoveTo(Home);SetState(TAG_PF_Patrol);return;}
        if(Best<=FMath::Square(125.f) && Now>=NextAttack)
        {if(AI){AI->StopMovement();}SetState(TAG_PF_Attack);AttackAt=Now+0.6;return;}
        SetState(TAG_PF_Chase);if(Now>=NextDecision){MoveTo(Target->GetActorLocation());NextDecision=Now+0.5;}return;
    }
    Target=nullptr;
    if(Now>=NextDecision)
    {
        if(State==TAG_PF_Idle){const FVector Offsets[]={FVector(250,0,0),FVector(0,250,0),FVector(-250,0,0),FVector(0,-250,0)};MoveTo(Home+Offsets[PatrolIndex++%4]);SetState(TAG_PF_Patrol);NextDecision=Now+3;}
        else{if(AI){AI->StopMovement();}SetState(TAG_PF_Idle);NextDecision=Now+2;}
    }
}
void APFCreature::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);Think();
    if(State==TAG_PF_Dead){GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);GetCharacterMovement()->DisableMovement();}
    if(GetNetMode()!=NM_DedicatedServer)
    {
        Body->SetRelativeScale3D(IsDead()?FVector(1,0.7,0.15):(bHostile?FVector(1.1,0.7,0.85):FVector(0.9,0.65,0.7)));
        Label->SetText(FText::FromString(FString::Printf(TEXT("%s %.0f\n%s"),*DefinitionId.ToString(),Health,*State.ToString())));
        Label->SetTextRenderColor(IsDead()?FColor::Silver:(bHostile?FColor::Red:FColor::Green));
        if(auto* PC=GetWorld()->GetFirstPlayerController())
        {
            if(PC->PlayerCameraManager)
            {
                const FVector ToCamera=PC->PlayerCameraManager->GetCameraLocation()-Label->GetComponentLocation();
                Label->SetWorldRotation(ToCamera.Rotation());
                Label->SetWorldSize(FMath::Clamp(static_cast<float>(ToCamera.Size())*0.025f,1.f,22.f));
                Label->SetVisibility(ToCamera.Size()>25);
            }
        }
    }
}
float APFCreature::TakeDamage(float Amount,const FDamageEvent& Event,AController* EventInstigator,AActor* Causer)
{
    if(!HasAuthority() || IsDead() || !FMath::IsFinite(Amount) || Amount<=0){return 0;}
    const float Applied=FMath::Min(Amount,Health);Health-=Applied;ForceNetUpdate();
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalCreatures] %s damage=%.0f health=%.0f"),*GetName(),Applied,Health);
    if(IsDead()){Die();}return Applied;
}
void APFCreature::Die()
{
    Target=nullptr;SetState(TAG_PF_Dead);if(auto* AI=Cast<AAIController>(GetController())){AI->StopMovement();}
    GetCharacterMovement()->DisableMovement();GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    const auto* D=Definition();const auto* Items=LoadObject<UPFItemCatalog>(nullptr,TEXT("/Game/PrimalFrontier/Items/DA_ItemCatalog.DA_ItemCatalog"));const auto* Food=Items?Items->Find(TEXT("Item_Food")):nullptr;
    if(D && Food)
    {
        const FTransform T(GetActorLocation()-FVector(0,0,25));
        auto* Loot=GetWorld()->SpawnActorDeferred<APFItemPickup>(APFItemPickup::StaticClass(),T,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        if(Loot){Loot->Initialize(Food->Id,D->FoodLoot,UPFInventoryComponent::ServerTime(GetWorld())+Food->ShelfLifeSeconds);Loot->FinishSpawning(T);}
    }
    SetLifeSpan(12);
}
