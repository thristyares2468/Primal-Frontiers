#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFInventoryComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"
UPFBuildingComponent::UPFBuildingComponent(){SetIsReplicatedByDefault(true);PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.TickInterval=0.1f;}
void UPFBuildingComponent::BeginPlay(){Super::BeginPlay();if(!Catalog){Catalog=LoadObject<UPFBuildingCatalog>(nullptr,TEXT("/Game/PrimalFrontier/Building/DA_BuildingCatalog.DA_BuildingCatalog"));}}
APFSurvivalPlayerController* UPFBuildingComponent::Controller() const{return Cast<APFSurvivalPlayerController>(GetOwner());}
bool UPFBuildingComponent::Living() const
{auto* PC=Controller();APawn* P=PC?PC->GetPawn():nullptr;auto* V=P?P->FindComponentByClass<UPFPlayerSurvivalComponent>():nullptr;return V && !V->IsDead() && PC->PlayerState;}
FName UPFBuildingComponent::SelectedId() const{return Catalog && Catalog->Pieces.IsValidIndex(Selection)?Catalog->Pieces[Selection].Id:NAME_None;}
APFBuildPiece* UPFBuildingComponent::TracedPiece() const
{
    if(!Living()){return nullptr;}APawn* P=Controller()->GetPawn();FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);FHitResult Hit;
    GetWorld()->LineTraceSingleByChannel(Hit,Eye,Eye+Look.Vector()*700,ECC_Visibility,FCollisionQueryParams(SCENE_QUERY_STAT(PFBuildTrace),false,P));return Cast<APFBuildPiece>(Hit.GetActor());
}
bool UPFBuildingComponent::InReach(APFBuildPiece* Piece) const
{return Living() && IsValid(Piece) && Piece->GetWorld()==GetWorld() && Piece->Builder==Controller()->PlayerState && FVector::DistSquared(Controller()->GetPawn()->GetActorLocation(),Piece->GetActorLocation())<FMath::Square(700.0);}
bool UPFBuildingComponent::Candidate(FName Id,int32 Q,FTransform& Out,APFBuildPiece*& Parent,FString& Reason) const
{
    Parent=nullptr;Reason=TEXT("Invalid piece or survivor");const auto* D=Catalog?Catalog->Find(Id):nullptr;if(!D || Q<0 || Q>3 || !Living()){return false;}
    auto* PC=Controller();APawn* P=PC->GetPawn();FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PFBuild),false,P);
    Reason=TEXT("Aim at ground or your supporting structure");if(!GetWorld()->LineTraceSingleByChannel(Hit,Eye,Eye+Look.Vector()*700,ECC_Visibility,Params)){return false;}
    FVector Location=Hit.ImpactPoint;FRotator RotationValue(0,Q*90,0);Parent=Cast<APFBuildPiece>(Hit.GetActor());
    Out=FTransform(RotationValue,Location);
    if(D->Kind==EPFBuildKind::Foundation)
    {
        Parent=nullptr;Location.X=FMath::GridSnap(Location.X,400.0);Location.Y=FMath::GridSnap(Location.Y,400.0);
        Out=FTransform(RotationValue,Location+FVector(0,0,10));
        FHitResult Ground;Reason=TEXT("Foundation requires flat ground");
        if(!GetWorld()->LineTraceSingleByChannel(Ground,Location+FVector(0,0,100),Location-FVector(0,0,150),ECC_Visibility,Params) || Cast<APFBuildPiece>(Ground.GetActor()) || Ground.ImpactNormal.Z<0.95){return false;}
        Location.Z=Ground.ImpactPoint.Z+10;
        for(int32 X:{-180,180}){for(int32 Y:{-180,180}){const FVector Corner=Location+FVector(X,Y,0);FHitResult G;if(!GetWorld()->LineTraceSingleByChannel(G,Corner+FVector(0,0,30),Corner-FVector(0,0,30),ECC_Visibility,Params) || Cast<APFBuildPiece>(G.GetActor()) || G.ImpactNormal.Z<0.95 || FMath::Abs(G.ImpactPoint.Z-(Location.Z-10))>3){return false;}}}
    }
    else
    {
        Reason=TEXT("Requires your compatible support");if(!InReach(Parent)){return false;}
        if(D->Kind==EPFBuildKind::Wall || D->Kind==EPFBuildKind::Door)
        {if(!Parent->IsPlatform()){return false;}Location=Parent->GetActorLocation()+RotationValue.RotateVector(FVector(200,0,160));}
        else if(D->Kind==EPFBuildKind::Storage)
        {if(!Parent->IsPlatform()){return false;}Location=Parent->GetActorLocation()+FVector(FMath::Clamp(FMath::GridSnap(Hit.ImpactPoint.X-Parent->GetActorLocation().X,100.0),-100.0,100.0),FMath::Clamp(FMath::GridSnap(Hit.ImpactPoint.Y-Parent->GetActorLocation().Y,100.0),-100.0,100.0),50);}
        else
        {if((Parent->Kind!=EPFBuildKind::Wall && Parent->Kind!=EPFBuildKind::Door) || !IsValid(Parent->Support)){return false;}Location=Parent->Support->GetActorLocation()+FVector(0,0,320);}
    }
    Out=FTransform(RotationValue,Location);Reason=TEXT("Out of reach");if(FVector::Dist(Eye,Location)>700 || Location.ContainsNaN() || Location.GetAbsMax()>100000){return false;}
    // Collision includes the player: never materialize a building through a survivor.
    FCollisionQueryParams OverlapParams(SCENE_QUERY_STAT(PFBuildOverlap),false);if(Parent){OverlapParams.AddIgnoredActor(Parent);}
    Reason=TEXT("Blocked by a player or object");if(GetWorld()->OverlapBlockingTestByChannel(Location,RotationValue.Quaternion(),ECC_Visibility,FCollisionShape::MakeBox(UPFBuildingCatalog::Size(D->Kind)*0.5-FVector(2)),OverlapParams)){return false;}
    if(GetWorld()->OverlapAnyTestByObjectType(Location,RotationValue.Quaternion(),FCollisionObjectQueryParams(ECC_Pawn),FCollisionShape::MakeBox(UPFBuildingCatalog::Size(D->Kind)*0.5-FVector(2)),OverlapParams)){return false;}
    int32 Count=0;for(TActorIterator<APFBuildPiece> It(GetWorld());It;++It){++Count;}Reason=TEXT("Development limit: 128 structures");if(Count>=128){return false;}
    auto* I=PC->GetInventory();Reason=FString::Printf(TEXT("Requires %d wood"),D->WoodCost);if(!I || I->Count(TEXT("Item_Wood"))<D->WoodCost){return false;}
    Reason=TEXT("Valid - click to build");return true;
}
APFBuildPiece* UPFBuildingComponent::Place(FName Id,int32 Q)
{
    if(!GetOwner()->HasAuthority()){return nullptr;}FTransform T;APFBuildPiece* Parent=nullptr;FString Why;
    if(!Candidate(Id,Q,T,Parent,Why)){Feedback=Why;return nullptr;}const auto* D=Catalog->Find(Id);auto* I=Controller()->GetInventory();
    auto* Piece=GetWorld()->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(),T,Controller(),nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if(!Piece){return nullptr;}Piece->Initialize(*D,Controller()->PlayerState,Parent);Piece->FinishSpawning(T);
    if(!IsValid(Piece) || !I->RemoveItem(TEXT("Item_Wood"),D->WoodCost)){if(IsValid(Piece)){Piece->Destroy();}return nullptr;}
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalBuilding] Placed %s owner=%s at=%s"),*Id.ToString(),*Controller()->PlayerState->GetName(),*T.GetLocation().ToString());Feedback=TEXT("Built");return Piece;
}
bool UPFBuildingComponent::Demolish(APFBuildPiece* Piece)
{
    if(!GetOwner()->HasAuthority() || !InReach(Piece) || TracedPiece()!=Piece){return false;}Piece->Storage->PruneExpired();
    if(!Piece->CanRemove()){return false;}UE_LOG(LogPFSurvival,Display,TEXT("[PrimalBuilding] Demolished %s"),*Piece->GetName());return Piece->Destroy();
}
bool UPFBuildingComponent::Interact(APFBuildPiece* Piece)
{
    if(!GetOwner()->HasAuthority() || !InReach(Piece) || TracedPiece()!=Piece){return false;}
    if(Piece->Kind==EPFBuildKind::Door){return Piece->ToggleDoor(Controller()->PlayerState);}
    if(Piece->Kind!=EPFBuildKind::Storage){return false;}OpenStorage=(OpenStorage==Piece?nullptr:Piece);GetOwner()->ForceNetUpdate();return true;
}
bool UPFBuildingComponent::Transfer(bool Deposit,FGuid Id,int32 Quantity)
{
    if(!GetOwner()->HasAuthority() || !InReach(OpenStorage) || TracedPiece()!=OpenStorage || Quantity<1 || Quantity>100){return false;}
    auto* Bag=Controller()->GetInventory();if(!Bag){return false;}
    return Deposit?Bag->TransferTo(OpenStorage->Storage,Id,Quantity):OpenStorage->Storage->TransferTo(Bag,Id,Quantity);
}
bool UPFBuildingComponent::RateLimit(){const double Now=GetWorld()->GetTimeSeconds();if(!GetOwner()->HasAuthority() || Now<NextRequest){return false;}NextRequest=Now+0.25;return true;}
void UPFBuildingComponent::ServerPlace_Implementation(FName Id,int32 Q){if(RateLimit()){Place(Id,Q);ClientResult(Feedback);}}
void UPFBuildingComponent::ServerTargetAction_Implementation(uint8 Action)
{
    if(!RateLimit()){return;}auto* Piece=TracedPiece();bool Done=false;
    if(Action==0){Done=Demolish(Piece);}else if(Action==1){Done=Interact(Piece);}else if(Action==2 && InReach(Piece)){Done=Piece->TakeDamage(25,FDamageEvent(),Controller(),Controller()->GetPawn())>0;}
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalBuilding] Action=%d accepted=%d"),Action,Done);ClientResult(Done?TEXT("Done"):TEXT("Refused: ownership, support, contents, reach or target"));
}
void UPFBuildingComponent::ServerTransfer_Implementation(bool Deposit,FGuid Id,int32 Quantity){if(RateLimit()){const bool Done=Transfer(Deposit,Id,Quantity);UE_LOG(LogPFSurvival,Display,TEXT("[PrimalBuilding] Transfer deposit=%d accepted=%d"),Deposit,Done);ClientResult(Done?TEXT("Transferred"):TEXT("Transfer refused"));}}
void UPFBuildingComponent::ClientResult_Implementation(const FString& Message){Feedback=Message;}
void UPFBuildingComponent::TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick)
{
    Super::TickComponent(Delta,Type,Tick);auto* PC=Controller();if(!PC){return;}
    if(PC->HasAuthority() && OpenStorage && !InReach(OpenStorage)){OpenStorage=nullptr;PC->ForceNetUpdate();}
    if(PC->IsLocalController() && bBuildMode)
    {FTransform T;APFBuildPiece* Parent=nullptr;const bool Valid=Candidate(SelectedId(),Rotation,T,Parent,PreviewMessage);if(!T.GetLocation().IsNearlyZero()){const auto* D=Catalog?Catalog->Find(SelectedId()):nullptr;if(D){DrawDebugBox(GetWorld(),T.GetLocation(),UPFBuildingCatalog::Size(D->Kind)*0.5,T.GetRotation(),Valid?FColor::Green:FColor::Red,false,0.12f,0,3);}}}
}
void UPFBuildingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME_CONDITION(UPFBuildingComponent,OpenStorage,COND_OwnerOnly);}
