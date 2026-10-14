#include "UI/PFUISprites.h"

bool PFUISprites::Resolve(FName Id,FSoftObjectPath& Texture,FBox2D& Region)
{
    static const TArray<FName> Symbols={TEXT("Health"),TEXT("Stamina"),TEXT("Food"),TEXT("Water"),TEXT("Weight"),TEXT("Experience"),TEXT("Attack"),TEXT("Climb"),TEXT("Glide"),TEXT("Exposure"),TEXT("Inventory"),TEXT("Craft"),TEXT("Build"),TEXT("Storage"),TEXT("Settings"),TEXT("Save")};
    static const TArray<FName> Actions={TEXT("Single"),TEXT("Multi"),TEXT("Resume"),TEXT("Pause"),TEXT("Controls"),TEXT("Back"),TEXT("Close"),TEXT("Apply"),TEXT("Reset"),TEXT("Sort"),TEXT("QuickStore"),TEXT("Filter"),TEXT("Rename"),TEXT("Drop"),TEXT("Split"),TEXT("Use")};
    static const TArray<FName> Encounters={TEXT("Creature_Forager"),TEXT("Creature_Prowler"),TEXT("Creature_Scrabbler"),TEXT("Creature_DuskPouncer")};
    int32 Cell=Symbols.IndexOfByKey(Id);const TCHAR* Name=TEXT("T_ExpeditionSymbols");int32 Columns=4;
    // Source canvases are not quite square; use inspected alpha-safe row/column seams.
    TArray<double> X={0,319./1271,635./1271,953./1271,1},Y={0,311./1238,619./1238,918./1238,1};
    if(Cell==INDEX_NONE)
    {
        Cell=Actions.IndexOfByKey(Id);Name=TEXT("T_ExpeditionActions");X={0,317./1268,634./1268,951./1268,1};Y={0,310./1241,620./1241,930./1241,1};
    }
    if(Cell==INDEX_NONE)
    {
        Cell=Encounters.IndexOfByKey(Id);Name=TEXT("T_ExpeditionEncounters");Columns=2;X={0,.5,1};Y=X;
    }
    if(Cell==INDEX_NONE){Texture.Reset();Region=FBox2D();return false;}
    Texture=FSoftObjectPath(FString::Printf(TEXT("/Game/PrimalFrontier/UI/Textures/%s.%s"),Name,Name));
    Region=FBox2D(FVector2D(X[Cell%Columns],Y[Cell/Columns]),FVector2D(X[Cell%Columns+1],Y[Cell/Columns+1]));return true;
}
