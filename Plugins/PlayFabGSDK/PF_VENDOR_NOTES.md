# PlayFab GSDK provenance and local changes

The exact vendor directory preserves upstream EOL/whitespace through .gitattributes; ten upstream trailing-whitespace lines are retained. Project source/compiler/warning checks are unchanged. Modified vendor files carry local-change notices; License.txt and these notes are staged with the Server binary.

Microsoft PlayFab/gsdk, Apache-2.0 (License.txt retained), UnrealPlugin2.0.1 at commit673f068f079591a820d0720048c6b4b77824825f. Source: https://github.com/PlayFab/gsdk/tree/673f068f079591a820d0720048c6b4b77824825f/UnrealPlugin . Only Source/, descriptor and license vendored; no vendor documentation screenshots/art/binaries imported.

Reviewed local integration changes:

- Descriptor is ServerOnly/LoadingPhase None; project reference permits Server target only. The game's hosting subsystem explicitly loads the SDK only after opt-in and bounded config validation. Default offline servers never start agent traffic or require agent configuration.
- Removed obsolete staging of C:/Windows/System32/D3DCompiler_43.dll. The NullRHI dedicated executable does not import this legacy DLL; no replacement binary downloaded. Actual Server launch remains a required gate. Existing supported UAT runtime dependencies still stage real required engine libraries.
- Source5.8.3 uses shared JSON string keys. Explicitly copy those keys to FString at the nine configuration/session-metadata TMap boundaries (GSDKConfiguration.cpp/GSDKInternal.cpp). First actual source Server compilation caught C2664/C2665/C2679; no field names, protocol values or validation changed by these type conversions.

All remaining vendor source bytes are preserved. Local GSDK lifecycle is not durable world restoration, authenticated player admission, paid hosting or voice-media acceptance. See Docs/PLAYFAB_GSDK_M15.md for status and evidence.
