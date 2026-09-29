#include "Building/PFBuildingCatalog.h"
#include "NativeGameplayTags.h"
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_PF_Structure,"Structure.Greybox");
UPFBuildingCatalog::UPFBuildingCatalog()
{
    const TCHAR* Ids[]={TEXT("Build_Foundation"),TEXT("Build_Wall"),TEXT("Build_Floor"),TEXT("Build_Ceiling"),TEXT("Build_Door"),TEXT("Build_Storage")};
    const TCHAR* Names[]={TEXT("Foundation"),TEXT("Wall"),TEXT("Floor"),TEXT("Ceiling"),TEXT("Door and frame"),TEXT("Storage")};
    for(int32 N=0;N<6;++N){FPFBuildingDefinition D;D.Id=Ids[N];D.Name=FText::FromString(Names[N]);D.Kind=EPFBuildKind(N);D.Category=TAG_PF_Structure;Pieces.Add(D);}
}
const FPFBuildingDefinition* UPFBuildingCatalog::Find(FName Id) const
{
    const FPFBuildingDefinition* Found=nullptr;
    for(const auto& D:Pieces){if(D.Id==Id){if(Found || Id.IsNone() || uint8(D.Kind)>5 || !D.Category.IsValid() || D.WoodCost<1 || D.WoodCost>20 || !FMath::IsFinite(D.MaxHealth) || D.MaxHealth<=0 || D.MaxHealth>10000){return nullptr;}Found=&D;}}
    return Found;
}
FVector UPFBuildingCatalog::Size(EPFBuildKind K)
{
    if(K==EPFBuildKind::Wall || K==EPFBuildKind::Door){return FVector(20,380,300);}
    if(K==EPFBuildKind::Storage){return FVector(70,70,80);}
    return FVector(400,400,20);
}
