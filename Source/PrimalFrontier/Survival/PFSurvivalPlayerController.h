#pragma once
#include "PrimalFrontierPlayerController.h"
#include "PFSurvivalPlayerController.generated.h"
class UPFSurvivalHUD;

UCLASS()
class PRIMALFRONTIER_API APFSurvivalPlayerController : public APrimalFrontierPlayerController
{
    GENERATED_BODY()
public:
    APFSurvivalPlayerController();
    UPROPERTY(EditDefaultsOnly, Category="Survival|UI") TSubclassOf<UPFSurvivalHUD> SurvivalHUDClass;
    UPROPERTY(Transient, BlueprintReadOnly, Category="Survival|UI") TObjectPtr<UPFSurvivalHUD> SurvivalHUD;
protected:
    virtual void BeginPlay() override;
};
