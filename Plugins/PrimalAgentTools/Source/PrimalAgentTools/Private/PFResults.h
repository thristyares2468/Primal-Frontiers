// PFResults.h (editor module)
//
// Re-exports the runtime result types and declares the editor-only viewport capture
// used by PF.CaptureTestScreenshot. History: ccbad56 (foundation).
#pragma once
#include "../../PrimalAgentToolsRuntime/Public/PFResults.h"
namespace PF::AgentTools { FResult CaptureScreenshot(const FString& Label); }
