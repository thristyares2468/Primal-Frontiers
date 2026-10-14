// Checks the actual imported PNG source pixels: prevents cross-item sprite bleed.
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "Engine/Texture2D.h"
#include "UI/PFUISprites.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFUIAtlasTest,"PF.UI.AtlasAssets",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFUIAtlasTest::RunTest(const FString&)
{
    const TCHAR* Names[]={TEXT("T_ExpeditionItems"),TEXT("T_ExpeditionStructures"),TEXT("T_ExpeditionSymbols"),TEXT("T_ExpeditionActions"),TEXT("T_ExpeditionEncounters")};
    const int32 Columns[]={4,3,4,4,2};
    const int32 Widths[]={1254,1254,1271,1268,1254},Heights[]={1254,1254,1238,1241,1254};
    const TArray<TArray<FName>> Ids={{},{},{TEXT("Health"),TEXT("Stamina"),TEXT("Food"),TEXT("Water"),TEXT("Weight"),TEXT("Experience"),TEXT("Attack"),TEXT("Climb"),TEXT("Glide"),TEXT("Exposure"),TEXT("Inventory"),TEXT("Craft"),TEXT("Build"),TEXT("Storage"),TEXT("Settings"),TEXT("Save")},{TEXT("Single"),TEXT("Multi"),TEXT("Resume"),TEXT("Pause"),TEXT("Controls"),TEXT("Back"),TEXT("Close"),TEXT("Apply"),TEXT("Reset"),TEXT("Sort"),TEXT("QuickStore"),TEXT("Filter"),TEXT("Rename"),TEXT("Drop"),TEXT("Split"),TEXT("Use")},{TEXT("Creature_Forager"),TEXT("Creature_Prowler"),TEXT("Creature_Scrabbler"),TEXT("Creature_DuskPouncer")}};
    for(int32 N=0;N<5;++N)
    {
        const FString Path=FString::Printf(TEXT("/Game/PrimalFrontier/UI/Textures/%s.%s"),Names[N],Names[N]);
        auto* Texture=LoadObject<UTexture2D>(nullptr,*Path);if(!TestNotNull(Path,Texture)){return false;}
        const int32 Width=Texture->Source.GetSizeX(),Height=Texture->Source.GetSizeY();
        TestTrue(TEXT("Bounded source atlas dimensions"),Width>=512 && Width<=2048 && Height>=512 && Height<=2048);
        TestEqual(TEXT("Verified source width (independent of async runtime proxy)"),Width,Widths[N]);TestEqual(TEXT("Verified source height"),Height,Heights[N]);
        TestEqual(TEXT("UI texture group"),Texture->LODGroup,TEXTUREGROUP_UI);
        TestEqual(TEXT("UI sprite uses no mipmaps"),Texture->MipGenSettings,TMGS_NoMipmaps);
        TArray64<uint8> Pixels;if(!TestTrue(TEXT("Imported source pixels available"),Texture->Source.GetMipData(Pixels,0))){return false;}
        if(!TestEqual(TEXT("RGBA8 source contract"),Pixels.Num(),int64(Width)*Height*4)){return false;}
        int32 Crossed=0;
        TArray<int32> X,Y;
        for(int32 Boundary=1;Boundary<Columns[N];++Boundary)
        {
            if(N<2){X.Add(FMath::FloorToInt(double(Width)*Boundary/Columns[N]));Y.Add(FMath::FloorToInt(double(Height)*Boundary/Columns[N]));}
        }
        for(FName Id:Ids[N])
        {
            FSoftObjectPath FrameTexture;FBox2D Frame;
            if(!TestTrue(TEXT("Registered original sprite frame"),PFUISprites::Resolve(Id,FrameTexture,Frame))){return false;}
            TestEqual(TEXT("Frame uses its expected atlas"),FrameTexture.ToString(),Path);
            TestTrue(TEXT("Frame stays within source"),Frame.bIsValid && Frame.Min.X>=0 && Frame.Min.Y>=0 && Frame.Max.X<=1 && Frame.Max.Y<=1);
            for(double Point:{Frame.Min.X,Frame.Max.X}){if(Point>0 && Point<1){X.AddUnique(FMath::RoundToInt(Point*Width));}}
            for(double Point:{Frame.Min.Y,Frame.Max.Y}){if(Point>0 && Point<1){Y.AddUnique(FMath::RoundToInt(Point*Height));}}
        }
        // Only actual frame boundaries are sampled. Legitimate pixels inside a
        // sprite's own cell are not bleed; the rendered test checks UV inset.
        for(int32 At:X){for(int32 P=0;P<Height;++P){Crossed+=Pixels[(int64(P)*Width+At)*4+3]>16?1:0;}}
        for(int32 At:Y){for(int32 P=0;P<Width;++P){Crossed+=Pixels[(int64(At)*Width+P)*4+3]>16?1:0;}}
        TestEqual(FString(Names[N])+TEXT(" transparent actual cell boundaries"),Crossed,0);
    }
    FSoftObjectPath Unknown;FBox2D Region;
    TestFalse(TEXT("Unknown glyph does not impersonate another icon"),PFUISprites::Resolve(TEXT("UnimplementedGlyph"),Unknown,Region));
    AddInfo(TEXT("[PrimalAgentTools] Actual atlas assets/source pixels checked; provisional art is not certified Fab mesh likeness."));
    return true;
}
#endif
