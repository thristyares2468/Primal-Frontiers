#pragma once

#include "PFResults.h"
#include "AssetRegistry/AssetData.h"

namespace PF::AgentTools
{
FString NamingPrefix(const FAssetData& Asset);
bool IsProjectRoot(const FString& Root);
FResult ValidateAssets(const FString& Root);
FResult CheckNaming();
FResult CheckReferences(const FString& Root);
}
