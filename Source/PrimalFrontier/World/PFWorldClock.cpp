// PFWorldClock.cpp — see PFWorldClock.h.

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
    // Always relevant: every client needs the time, wherever they are in the map.
    bReplicates=true;
    bAlwaysRelevant=true;
    PrimaryActorTick.bCanEverTick=true;
    PrimaryActorTick.TickInterval=0.25f;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    Sun=CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("Sun"));
    Sun->SetupAttachment(GetRootComponent());
    Sun->SetMobility(EComponentMobility::Movable);
    Sun->SetIntensity(3);
    Sun->SetForwardShadingPriority(1);
    // Fixed dim moonlight-style fill so night is dark but still navigable.
    NightFill=CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("NightVisibility"));
    NightFill->SetupAttachment(GetRootComponent());
    NightFill->SetMobility(EComponentMobility::Movable);
    NightFill->SetIntensity(0.2f);
    NightFill->SetForwardShadingPriority(0);
    NightFill->SetCastShadows(false);
    NightFill->SetRelativeRotation(FRotator(-60,180,0));
}

void APFWorldClock::BeginPlay()
{
    Super::BeginPlay();
    // The sun takes priority as the forward-shading directional light.
    Sun->SetForwardShadingPriority(1);
    NightFill->SetForwardShadingPriority(0);
    if(HasAuthority()){UpdatePhase();}
    OnRep_Hour();
}

bool APFWorldClock::SetHour(float NewHour)
{
    if(!HasAuthority() || !GetWorld() || !GetWorld()->IsGameWorld() || !FMath::IsFinite(NewHour) || NewHour<0 || NewHour>=24){return false;}
    Hour=NewHour;
    UpdatePhase();
    OnRep_Hour();  // listen server / standalone: update our own lights too
    ForceNetUpdate();
    return true;
}

void APFWorldClock::Advance(float Seconds)
{
    if(!HasAuthority() || !FMath::IsFinite(Seconds) || Seconds<=0 || !FMath::IsFinite(DayLengthSeconds) || DayLengthSeconds<60 || DayLengthSeconds>86400){return;}
    // Convert real seconds to in-game hours (24 h per DayLengthSeconds) and wrap at midnight.
    SetHour(FMath::Fmod(Hour+FMath::Fmod(Seconds,DayLengthSeconds)*24.f/DayLengthSeconds,24.f));
}

void APFWorldClock::UpdatePhase()
{
    const auto NewPhase=Hour>=6 && Hour<18?TAG_PF_Day.GetTag():TAG_PF_Night.GetTag();
    if(Phase!=NewPhase)
    {
        Phase=NewPhase;
        UE_LOG(LogPFSurvival,Display,TEXT("[PrimalWorld] Phase=%s hour=%.2f"),*Phase.ToString(),Hour);
    }
}

void APFWorldClock::OnRep_Hour()
{
    if(GetNetMode()==NM_DedicatedServer){return;}
    // Sun rises at 06:00 (horizon), peaks at 12:00 (overhead), sets at 18:00; 15 degrees per hour.
    Sun->SetWorldRotation(FRotator(-(Hour-6)*15,30,0));
    // Intensity follows a half sine over the day and is zero all night.
    Sun->SetIntensity(FMath::Max(0.f,FMath::Sin((Hour-6)*PI/12))*3.f);
}

void APFWorldClock::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    Advance(DeltaSeconds);
}

void APFWorldClock::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(APFWorldClock,Hour);
    DOREPLIFETIME(APFWorldClock,Phase);
}
