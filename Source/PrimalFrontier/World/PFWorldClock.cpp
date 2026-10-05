#include "World/PFWorldClock.h"
#include "Components/DirectionalLightComponent.h"
#include "NativeGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Survival/PFPlayerSurvivalComponent.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Day,"World.Time.Day");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Night,"World.Time.Night");
APFWorldClock::APFWorldClock()
{
    bReplicates=true;bAlwaysRelevant=true;PrimaryActorTick.bCanEverTick=true;PrimaryActorTick.TickInterval=0.25f;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    Sun=CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));Sun->SetupAttachment(GetRootComponent());Sun->SetMobility(EComponentMobility::Movable);Sun->SetIntensity(3);
    NightFill=CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("NightVisibility"));NightFill->SetupAttachment(GetRootComponent());NightFill->SetMobility(EComponentMobility::Movable);NightFill->SetIntensity(0.2f);NightFill->SetCastShadows(false);NightFill->SetRelativeRotation(FRotator(-60,180,0));
}
void APFWorldClock::BeginPlay(){Super::BeginPlay();Sun->SetForwardShadingPriority(1);NightFill->SetForwardShadingPriority(0);if(HasAuthority()){UpdatePhase();}OnRep_Hour();}
bool APFWorldClock::SetHour(float NewHour)
{
    if(!HasAuthority() || !GetWorld() || !GetWorld()->IsGameWorld() || !FMath::IsFinite(NewHour) || NewHour<0 || NewHour>=24){return false;}
    Hour=NewHour;UpdatePhase();OnRep_Hour();ForceNetUpdate();return true;
}
void APFWorldClock::Advance(float Seconds)
{
    if(!HasAuthority() || !FMath::IsFinite(Seconds) || Seconds<=0 || !FMath::IsFinite(DayLengthSeconds) || DayLengthSeconds<60 || DayLengthSeconds>86400){return;}
    SetHour(FMath::Fmod(Hour+FMath::Fmod(Seconds,DayLengthSeconds)*24.f/DayLengthSeconds,24.f));
}
void APFWorldClock::UpdatePhase()
{
    const auto NewPhase=Hour>=6 && Hour<18?TAG_PF_Day.GetTag():TAG_PF_Night.GetTag();
    if(Phase!=NewPhase){Phase=NewPhase;UE_LOG(LogPFSurvival,Display,TEXT("[PrimalWorld] Phase=%s hour=%.2f"),*Phase.ToString(),Hour);}
}
void APFWorldClock::OnRep_Hour()
{
    if(GetNetMode()==NM_DedicatedServer){return;}
    Sun->SetWorldRotation(FRotator(-(Hour-6)*15,30,0));
    Sun->SetIntensity(FMath::Max(0.f,FMath::Sin((Hour-6)*PI/12))*3.f);
}
void APFWorldClock::Tick(float DeltaSeconds){Super::Tick(DeltaSeconds);Advance(DeltaSeconds);}
void APFWorldClock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{Super::GetLifetimeReplicatedProps(OutLifetimeProps);DOREPLIFETIME(APFWorldClock,Hour);DOREPLIFETIME(APFWorldClock,Phase);}
