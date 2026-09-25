#include "Survival/PFSurvivalHazard.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "UObject/ConstructorHelpers.h"

APFSurvivalHazard::APFSurvivalHazard()
{
    bReplicates = true;
    Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
    SetRootComponent(Bounds);
    Bounds->SetBoxExtent(FVector(160,200,180));
    Bounds->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Bounds->SetCollisionResponseToAllChannels(ECR_Ignore);
    Bounds->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Bounds->SetGenerateOverlapEvents(true);
    auto* Marker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
    Marker->SetupAttachment(Bounds); Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    Marker->SetStaticMesh(Cube.Object);
    Marker->SetRelativeLocation(FVector(0,0,-170)); Marker->SetRelativeScale3D(FVector(3.2,4,0.2));
    auto* Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(Bounds); Label->SetText(FText::FromString(TEXT("DANGER: EXPOSURE")));
    Label->SetRelativeLocation(FVector(-160,0,80)); Label->SetRelativeRotation(FRotator(0,180,0)); Label->SetWorldSize(22);
}
