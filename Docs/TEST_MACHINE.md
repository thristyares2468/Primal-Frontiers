# Test machine reference

The installed UE_5.8 directory reports **5.8.3, changelist 58210709** on September 30, verified in Engine/Build/Build.version and M6 engine-generated reports. The original requested version was 5.8.2; record the actual version for reproducibility. This work did not update the engine.

User-supplied reference, 2026-09-29. This replaces the earlier 16 GB RAM assumption for this machine; retain small maps and one rendered client until profiling supports larger workloads.

| Component | Reference specification |
| --- | --- |
| CPU | AMD Ryzen 9 5900X, 12 cores / 24 threads |
| GPU | MSI GeForce RTX 4060 Ti VENTUS 2X BLACK 16G OC, 16 GB VRAM |
| RAM | Corsair Vengeance LPX, 32 GB (2 x 16 GB), DDR4-3200 CL16 |
| Motherboard | MSI B550 GAMING GEN3, ATX, AM4; PCIe 3.0 x16 GPU slot |
| Storage | 2 TB M.2 NVMe SSD |
| PSU | Hydra 750 W, 80+ Bronze |
| Main display | MSI Optix G27 series, curved, 2560 x 1440, 165 Hz |

Observed M5 automation/CSV identifies Ryzen 9 5900X and 32 GB RAM. Both Unreal's D3D12 adapter name and Windows Win32_VideoController identify **NVIDIA GeForce RTX 4060**, without Ti. Keep this discrepancy visible: the supplied GPU SKU/16 GB VRAM has not been independently confirmed. The 32-bit WMI AdapterRAM field is unsuitable for resolving it. Other component models, RAM speed, negotiated PCIe link, storage and PSU are user-supplied rather than measured.

M6's rendered startup independently reports the chosen D3D12 adapter as NVIDIA GeForce RTX 4060, device ID 2882, with **7,956 MB dedicated video memory** (`Saved/Logs/PFM6Manual.log`). Attribute measured results to that runtime adapter; retain the supplied 4060 Ti 16 GB specification as the user's reference, not a verified device identity.

M5's rendered sample uses 960 x 540, Development game mode in the Editor executable, D3D12, unchanged graphics quality, t.MaxFPS=0, r.VSync=0 and a session override disabling frame smoothing. It does not establish native 1440p performance or a packaged-build minimum specification. NullRHI network runs measure correctness, not GPU performance. Record actual resolution, adapter, build mode, frame-time distribution and memory with each future benchmark; use a separate 1440p capture before making native-display performance claims.
