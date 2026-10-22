#include "Crafting/PFCraftingStation.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Building/PFBuildPiece.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ConstructorHelpers.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_CampWorkbench,"Crafting.Station.Workbench");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_CampRefinery,"Crafting.Station.Refinery");
FGameplayTag APFCraftingStation::WorkbenchType(){return TAG_CampWorkbench;}
FGameplayTag APFCraftingStation::RefineryType(){return TAG_CampRefinery;}
APFCraftingStation::APFCraftingStation()
{
    bReplicates=true;PrimaryActorTick.bCanEverTick=false;StationType=TAG_CampWorkbench;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    Mesh=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));Mesh->SetupAttachment(RootComponent);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Mesh->SetStaticMesh(Cube.Object);Mesh->SetRelativeLocation(FVector(0,0,35));Mesh->SetRelativeScale3D(FVector(1.2,.8,.7));Mesh->SetCollisionProfileName(TEXT("BlockAll"));
    Label=CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));Label->SetupAttachment(RootComponent);
    Label->SetRelativeLocation(FVector(0,0,110));Label->SetRelativeRotation(FRotator(0,180,0));Label->SetWorldSize(18);Label->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
void APFCraftingStation::BeginPlay()
{
    Super::BeginPlay();
    OnRepProvider();
    // TextRender components are stripped when an authored map loads on a server.
    if(!bOwnedProvider && GetNetMode()!=NM_DedicatedServer && IsValid(Label))
    {Label->SetText(FText::FromString(DisplayName()+TEXT("\nC: craft nearby | stay within 2.5m\nB: build your own workbench / refinery on a foundation")));}
}
void APFCraftingStation::OnRepProvider()
{
    if(!bOwnedProvider){return;}
    // The replicated build piece supplies all visible/collidable geometry.
    SetActorHiddenInGame(true);SetActorEnableCollision(false);
    bNetUseOwnerRelevancy=true;
}
void APFCraftingStation::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(APFCraftingStation,StationType);DOREPLIFETIME(APFCraftingStation,Radius);DOREPLIFETIME(APFCraftingStation,SpeedMultiplier);DOREPLIFETIME(APFCraftingStation,bOwnedProvider);DOREPLIFETIME(APFCraftingStation,ProviderPiece);
}
FString APFCraftingStation::DisplayName() const
{return StationType==TAG_CampWorkbench?(bOwnedProvider?TEXT("Owned workbench"):TEXT("Camp workbench")):StationType==TAG_CampRefinery?(bOwnedProvider?TEXT("Owned refinery"):TEXT("Camp refinery")):TEXT("Unavailable station");}
bool APFCraftingStation::Supports(const FPFRecipeDefinition& Recipe) const
{
    if(!FMath::IsFinite(Radius) || Radius<100 || Radius>500 || !FMath::IsFinite(SpeedMultiplier) || SpeedMultiplier<=1 || SpeedMultiplier>2){return false;}
    if(Recipe.RequiredStation.IsValid()){return Recipe.RequiredStation==StationType && (StationType==TAG_CampRefinery || StationType==TAG_CampWorkbench);}
    if(StationType==TAG_CampRefinery)
    {
        static const TArray<FName> Ids={TEXT("Recipe_CopperRefine"),TEXT("Recipe_IronRefine"),TEXT("Recipe_GoldRefine"),TEXT("Recipe_SilverRefine"),TEXT("Recipe_TitaniumRefine")};
        return Ids.Contains(Recipe.Id);
    }
    if(StationType!=TAG_CampWorkbench){return false;}
    return Recipe.Category==FGameplayTag::RequestGameplayTag(TEXT("Recipe.Category.Tool")) || Recipe.Category==FGameplayTag::RequestGameplayTag(TEXT("Recipe.Category.Material")) ||
        Recipe.Category==FGameplayTag::RequestGameplayTag(TEXT("Recipe.Category.Weapon")) || Recipe.Category==FGameplayTag::RequestGameplayTag(TEXT("Recipe.Category.Protection"));
}
bool APFCraftingStation::CanUse(const FPFRecipeDefinition& Recipe,const APawn* Pawn,bool RequireAuthority) const
{
    if(!IsValid(Pawn) || IsActorBeingDestroyed() || GetWorld()!=Pawn->GetWorld() || !Supports(Recipe) ||
        (RequireAuthority && (!HasAuthority() || !Pawn->HasAuthority())) || FVector::DistSquared(GetActorLocation(),Pawn->GetActorLocation())>FMath::Square(Radius)){return false;}
    if(bOwnedProvider && (!IsValid(ProviderPiece) || ProviderPiece->IsActorBeingDestroyed() || !FMath::IsFinite(ProviderPiece->Health) || ProviderPiece->Health<=0 ||
        (RequireAuthority && !ProviderPiece->HasAuthority()) ||
        ProviderPiece->GetWorld()!=GetWorld() || !ProviderPiece->IsOwnedBy(Pawn->GetPlayerState()) ||
        (StationType==TAG_CampWorkbench?ProviderPiece->Kind!=EPFBuildKind::Workbench:ProviderPiece->Kind!=EPFBuildKind::Refinery))){return false;}
    FVector Eye;FRotator View;Pawn->GetActorEyesViewPoint(Eye,View);
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PFCraftStation),false,Pawn);FHitResult Hit;
    return !GetWorld()->LineTraceSingleByChannel(Hit,Eye,GetActorLocation()+FVector(0,0,35),ECC_Visibility,Params) || Hit.GetActor()==this || (bOwnedProvider && Hit.GetActor()==ProviderPiece);
}
APFCraftingStation* APFCraftingStation::FindBest(const FPFRecipeDefinition& Recipe,const APawn* Pawn,bool RequireAuthority)
{
    if(!IsValid(Pawn) || !Pawn->GetWorld()){return nullptr;}
    APFCraftingStation* Best=nullptr;
    for(TActorIterator<APFCraftingStation> It(Pawn->GetWorld());It;++It)
    {
        if(!It->CanUse(Recipe,Pawn,RequireAuthority)){continue;}
        if(!Best || It->SpeedMultiplier>Best->SpeedMultiplier || (It->SpeedMultiplier==Best->SpeedMultiplier && It->GetName()<Best->GetName())){Best=*It;}
    }
    return Best;
}
