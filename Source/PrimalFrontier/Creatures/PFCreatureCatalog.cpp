// PFCreatureCatalog.cpp — see PFCreatureCatalog.h.

#include "Creatures/PFCreatureCatalog.h"

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
