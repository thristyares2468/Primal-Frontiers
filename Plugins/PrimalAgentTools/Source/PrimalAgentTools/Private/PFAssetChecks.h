// PFAssetChecks.h
//
// Editor asset checks behind PF.ValidateAssets, PF.CheckNaming and PF.CheckReferences.
// All are read-only: they report, never fix, rename or save. History: ccbad56.
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
