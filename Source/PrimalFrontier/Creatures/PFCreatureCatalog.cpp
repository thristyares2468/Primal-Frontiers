// PFCreatureCatalog.cpp — see PFCreatureCatalog.h.

#include "Creatures/PFCreatureCatalog.h"
#include "Creatures/PFCreatureSpawnCatalog.h"
#include "NativeGameplayTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_EcologyBiome, "Ecology.Biome");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_EcologyShore, "Ecology.Biome.Shore");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_EcologyWoodland, "Ecology.Biome.Woodland");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_EcologyRidge, "Ecology.Biome.Ridge");

UPFCreatureCatalog::UPFCreatureCatalog()
{
    // Passive forager: struct defaults (60 health, 170 speed, 2 food).
    FPFCreatureDefinition Passive;
    Passive.Id=TEXT("Creature_Forager"); Passive.Name=FText::FromString(TEXT("Greybox Forager"));
    Creatures.Add(Passive);
    // Hostile prowler: tougher, faster, more loot.
    FPFCreatureDefinition Hostile;
    Hostile.Id=TEXT("Creature_Prowler"); Hostile.Name=FText::FromString(TEXT("Greybox Prowler"));
    Hostile.bHostile=true; Hostile.Health=100; Hostile.Speed=220; Hostile.FoodLoot=3;
    Creatures.Add(Hostile);
}

const FPFCreatureDefinition* UPFCreatureCatalog::Find(FName Id) const
{
    const FPFCreatureDefinition* Found=nullptr;
    for(const auto& D:Creatures)
    {
        if(D.Id!=Id){continue;}
        if(Found || Id.IsNone() || D.Name.IsEmpty() || !FMath::IsFinite(D.Health) || D.Health<=0 || D.Health>1000 ||
            !FMath::IsFinite(D.Speed) || D.Speed<=0 || D.Speed>500 || !FMath::IsFinite(D.SightRange) || D.SightRange<100 || D.SightRange>2000 ||
            !FMath::IsFinite(D.Damage) || D.Damage<=0 || D.Damage>100 || D.FoodLoot<1 || D.FoodLoot>10){return nullptr;}
        Found=&D;
    }
    return Found;
}

UPFCreatureSpawnCatalog::UPFCreatureSpawnCatalog()
{
    const auto Add=[this](FGameplayTag Biome, int32 Maximum, TArray<FPFCreatureSpawnEntry> Entries)
    {
        FPFCreatureSpawnPolicy Policy;
        Policy.Biome=Biome; Policy.MaximumResidents=Maximum; Policy.Entries=MoveTemp(Entries);
        Policies.Add(MoveTemp(Policy));
    };
    Add(TAG_PF_EcologyShore.GetTag(),2,{{TEXT("Creature_Forager"),1,1}});
    Add(TAG_PF_EcologyWoodland.GetTag(),3,{{TEXT("Creature_Forager"),3,1},{TEXT("Creature_Prowler"),1,3}});
    Add(TAG_PF_EcologyRidge.GetTag(),2,{{TEXT("Creature_Prowler"),0,1}});
}

bool UPFCreatureSpawnCatalog::Validate(const UPFCreatureCatalog& Creatures, FString& Error) const
{
    const auto Refuse=[&Error](const TCHAR* Reason){Error=Reason;return false;};
    if(Policies.IsEmpty() || Policies.Num()>8){return Refuse(TEXT("Spawn policy count must be 1..8"));}
    TSet<FGameplayTag> Biomes;
    int32 TotalResidents=0;
    for(const auto& Policy:Policies)
    {
        if(!Policy.Biome.IsValid() || Policy.Biome==TAG_PF_EcologyBiome.GetTag() ||
            !Policy.Biome.MatchesTag(TAG_PF_EcologyBiome.GetTag()) || Biomes.Contains(Policy.Biome))
        {return Refuse(TEXT("Spawn biome must be a unique Ecology.Biome child tag"));}
        Biomes.Add(Policy.Biome);
        if(Policy.MaximumResidents<1 || Policy.MaximumResidents>8 ||
            !FMath::IsFinite(Policy.RespawnSeconds) || Policy.RespawnSeconds<5 || Policy.RespawnSeconds>300)
        {return Refuse(TEXT("Spawn resident budget or respawn duration is out of range"));}
        TotalResidents+=Policy.MaximumResidents;
        if(TotalResidents>8){return Refuse(TEXT("Combined spawn resident budgets exceed eight"));}
        if(Policy.Entries.IsEmpty() || Policy.Entries.Num()>8){return Refuse(TEXT("Spawn entry count must be 1..8"));}
        TSet<FName> Ids;
        for(const auto& Entry:Policy.Entries)
        {
            if(!Creatures.Find(Entry.CreatureId) || Ids.Contains(Entry.CreatureId))
            {return Refuse(TEXT("Spawn creature ID is unknown, invalid or duplicated"));}
            Ids.Add(Entry.CreatureId);
            if(Entry.DayWeight<0 || Entry.DayWeight>100 || Entry.NightWeight<0 || Entry.NightWeight>100 ||
                (Entry.DayWeight==0 && Entry.NightWeight==0))
            {return Refuse(TEXT("Spawn weights must be 0..100 with at least one eligible phase"));}
        }
    }
    Error.Reset();
    return true;
}

bool UPFCreatureSpawnCatalog::Choose(const UPFCreatureCatalog& Creatures, FGameplayTag Biome,
    FGameplayTag Phase, int32 Roll, FName& OutId, FString& Error) const
{
    if(!Validate(Creatures,Error)){return false;}
    const auto Refuse=[&Error](const TCHAR* Reason){Error=Reason;return false;};
    const auto Day=FGameplayTag::RequestGameplayTag(TEXT("World.Time.Day"));
    const auto Night=FGameplayTag::RequestGameplayTag(TEXT("World.Time.Night"));
    if(Phase!=Day && Phase!=Night){return Refuse(TEXT("Spawn phase must be World.Time.Day or World.Time.Night"));}
    const auto* Policy=Policies.FindByPredicate([Biome](const auto& Candidate){return Candidate.Biome==Biome;});
    if(!Policy){return Refuse(TEXT("Spawn biome has no policy"));}
    int32 Total=0;
    for(const auto& Entry:Policy->Entries){Total+=Phase==Day?Entry.DayWeight:Entry.NightWeight;}
    if(Total==0){return Refuse(TEXT("No creature is eligible in this biome and phase"));}
    if(Roll<0 || Roll>=Total){return Refuse(TEXT("Spawn roll is outside eligible weight range"));}
    for(const auto& Entry:Policy->Entries)
    {
        const int32 Weight=Phase==Day?Entry.DayWeight:Entry.NightWeight;
        if(Roll<Weight){OutId=Entry.CreatureId; Error.Reset();return true;}
        Roll-=Weight;
    }
    return Refuse(TEXT("Spawn selection failed"));
}
