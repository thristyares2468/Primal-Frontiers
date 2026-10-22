// PFBuildingCatalog.cpp — see PFBuildingCatalog.h.

#include "Building/PFBuildingCatalog.h"
#include "NativeGameplayTags.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Structure,"Structure.Greybox");

UPFBuildingCatalog::UPFBuildingCatalog()
{
    // Parallel arrays in EPFBuildKind order; each piece uses the struct defaults for cost/health.
    const TCHAR* Ids[]={TEXT("Build_Foundation"),TEXT("Build_Wall"),TEXT("Build_Floor"),TEXT("Build_Ceiling"),TEXT("Build_Door"),TEXT("Build_Storage"),TEXT("Build_Window"),TEXT("Build_Stairs"),TEXT("Build_Workbench"),TEXT("Build_Refinery")};
    const TCHAR* Names[]={TEXT("Foundation"),TEXT("Wall"),TEXT("Floor"),TEXT("Roof / ceiling"),TEXT("Door and frame"),TEXT("Chest"),TEXT("Window wall"),TEXT("Stairs"),TEXT("Workbench"),TEXT("Refinery")};
    static_assert(uint8(EPFBuildKind::Stairs)==7,"Preserve existing saved building kind values");
    for(int32 N=0;N<10;++N)
    {
        FPFBuildingDefinition D;
        D.Id=Ids[N];
        D.Name=FText::FromString(Names[N]);
        D.Kind=EPFBuildKind(N);
        D.Category=TAG_PF_Structure;
        if(D.Kind==EPFBuildKind::Workbench){D.WoodCost=8;}
        if(D.Kind==EPFBuildKind::Refinery){D.WoodCost=12;}
        Pieces.Add(D);
    }
}

const FPFBuildingDefinition* UPFBuildingCatalog::Find(FName Id) const
{
    const FPFBuildingDefinition* Found=nullptr;
    for(const auto& D:Pieces)
    {
        if(D.Id==Id)
        {
            if(Found || Id.IsNone() || uint8(D.Kind)>uint8(EPFBuildKind::Refinery) || !D.Category.IsValid() || D.WoodCost<1 || D.WoodCost>20 || !FMath::IsFinite(D.MaxHealth) || D.MaxHealth<=0 || D.MaxHealth>10000){return nullptr;}
            Found=&D;
        }
    }
    return Found;
}

FVector UPFBuildingCatalog::Size(EPFBuildKind K)
{
    if(K==EPFBuildKind::Wall || K==EPFBuildKind::Door || K==EPFBuildKind::Window){return FVector(20,380,300);}
    if(K==EPFBuildKind::Stairs){return FVector(380,380,320);}
    if(K==EPFBuildKind::Storage){return FVector(70,70,80);}
    if(K==EPFBuildKind::Workbench || K==EPFBuildKind::Refinery){return FVector(120,80,70);}
    return FVector(400,400,20);  // Foundation, Floor, Ceiling: one 400 cm grid cell
}
