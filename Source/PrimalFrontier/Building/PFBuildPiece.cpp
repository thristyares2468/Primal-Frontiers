#include "Building/PFBuildPiece.h"
#include "Inventory/PFInventoryComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Controller.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"
APFBuildPiece::APFBuildPiece()
{
    bReplicates=true;SetReplicateMovement(true);
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    for(int32 N=0;N<4;++N){auto* M=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Shape%d"),N));M->SetupAttachment(RootComponent);M->SetStaticMesh(Cube.Object);M->SetCollisionProfileName(TEXT("BlockAll"));Shapes.Add(M);}
    Storage=CreateDefaultSubobject<UPFInventoryComponent>(TEXT("Storage"));Storage->SlotLimit=8;Storage->WeightLimit=60;
}
void APFBuildPiece::BeginPlay(){Super::BeginPlay();OnRepShape();if(Kind!=EPFBuildKind::Storage){Storage->SetComponentTickEnabled(false);}}
void APFBuildPiece::Initialize(const FPFBuildingDefinition& D,APlayerState* OwnerState,APFBuildPiece* Parent)
{if(!HasAuthority()){return;}DefinitionId=D.Id;Kind=D.Kind;Builder=OwnerState;Support=Parent;Health=D.MaxHealth;SetOwner(OwnerState?OwnerState->GetOwner():nullptr);OnRepShape();}
void APFBuildPiece::OnRepShape()
{
    for(const auto& M:Shapes){M->SetVisibility(false);M->SetCollisionEnabled(ECollisionEnabled::NoCollision);}
    auto Set=[&](int32 N,FVector Size,FVector Position){Shapes[N]->SetRelativeLocation(Position);Shapes[N]->SetRelativeScale3D(Size/100);Shapes[N]->SetVisibility(true);Shapes[N]->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);};
    if(Kind==EPFBuildKind::Door){Set(0,FVector(20,120,300),FVector(0,-130,0));Set(1,FVector(20,120,300),FVector(0,130,0));Set(2,FVector(20,140,50),FVector(0,0,125));if(!bDoorOpen){Set(3,FVector(20,130,250),FVector(0,0,-25));}}
    else{Set(0,UPFBuildingCatalog::Size(Kind),FVector::ZeroVector);}
}
bool APFBuildPiece::IsPlatform() const{return Kind==EPFBuildKind::Foundation || Kind==EPFBuildKind::Floor || Kind==EPFBuildKind::Ceiling;}
bool APFBuildPiece::HasDependents() const {for(TActorIterator<APFBuildPiece> It(GetWorld());It;++It){if(It->Support==this){return true;}}return false;}
bool APFBuildPiece::CanRemove() const{return !HasDependents() && (Kind!=EPFBuildKind::Storage || Storage->GetStacks().IsEmpty());}
bool APFBuildPiece::ToggleDoor(APlayerState* Requester){if(!HasAuthority() || !Requester || Builder!=Requester || Kind!=EPFBuildKind::Door){return false;}bDoorOpen=!bDoorOpen;OnRepShape();ForceNetUpdate();return true;}
float APFBuildPiece::TakeDamage(float Amount,const FDamageEvent&,AController* EventInstigator,AActor*)
{
    if(!HasAuthority() || !EventInstigator || EventInstigator->PlayerState!=Builder || !FMath::IsFinite(Amount) || Amount<=0){return 0;}
    Storage->PruneExpired();if(Amount>=Health && !CanRemove()){return 0;}
    const float Applied=FMath::Min(Health,Amount);Health-=Applied;ForceNetUpdate();if(Health<=0){Destroy();}return Applied;
}
void APFBuildPiece::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(APFBuildPiece,Kind);DOREPLIFETIME(APFBuildPiece,DefinitionId);DOREPLIFETIME(APFBuildPiece,Builder);DOREPLIFETIME(APFBuildPiece,Support);DOREPLIFETIME(APFBuildPiece,Health);DOREPLIFETIME(APFBuildPiece,bDoorOpen);}
