#include "PFResults.h"
#include "Editor.h"
#include "LevelEditorViewport.h"
#include "HAL/FileManager.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "RHI.h"
#include "UnrealClient.h"
namespace PF::AgentTools {
FResult CaptureScreenshot(const FString& Label)
{
    FResult Result(TEXT("PF.CaptureScreenshot"));
    const FString Path = NewArtifactPath(Label, TEXT("png"));
    if (Path.IsEmpty())
    {
        Result.Add(TEXT("Error"), TEXT("InvalidLabel"), Label, TEXT("Use 1-64 ASCII letters, digits, underscore or hyphen."));
        return Result;
    }
    // Take pixels from an actual level viewport, never a Blueprint/asset-editor tab.
    FLevelEditorViewportClient* Client = GCurrentLevelEditingViewportClient;
    if (!GEditor || GEditor->PlayWorld || IsRunningCommandlet() || GUsingNullRHI || !Client || !Client->Viewport)
    {
        Result.Add(TEXT("Error"), TEXT("ViewportUnavailable"), FString(), TEXT("Requires a rendered level-editor viewport outside PIE; NullRHI/commandlets are unsupported."));
        return Result;
    }
    FViewport* Viewport = Client->Viewport;
    const FIntPoint Size = Viewport->GetSizeXY();
    if (Size.X <= 0 || Size.Y <= 0 || static_cast<int64>(Size.X) * Size.Y > 67108864)
    {
        Result.Add(TEXT("Error"), TEXT("InvalidViewportSize"), FString(), TEXT("Viewport must contain 1-67108864 pixels."));
        return Result;
    }
    Viewport->Draw();
    TArray<FColor> Pixels;
    if (!GetViewportScreenShot(Viewport, Pixels) || Pixels.Num() != Size.X * Size.Y)
    {
        Result.Add(TEXT("Error"), TEXT("ScreenshotReadFailed"), FString(), TEXT("Viewport pixel readback failed."));
        return Result;
    }
    for (FColor& Pixel : Pixels) { Pixel.A = 255; }
    TArray64<uint8> Png;
    FImageUtils::PNGCompressImageArray(Size.X, Size.Y, Pixels, Png);
    if (Png.IsEmpty() || !IFileManager::Get().MakeDirectory(*ReportsDirectory(), true) ||
        !FFileHelper::SaveArrayToFile(Png, *Path))
    {
        Result.Add(TEXT("Error"), TEXT("ScreenshotWriteFailed"), Path, TEXT("Could not encode/write the viewport PNG."));
        return Result;
    }
    Result.Scope = Client->GetWorld() ? Client->GetWorld()->GetPathName() : FString();
    Result.Counts.Add(TEXT("width"), Size.X);
    Result.Counts.Add(TEXT("height"), Size.Y);
    Result.Artifacts.Add(Path);
    return Result;
}

}
