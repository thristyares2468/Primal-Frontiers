"""Read-only M9 package/Asset Registry inventory; never loads/saves asset objects.

Run with UnrealEditor-Cmd -run=pythonscript -script=<absolute path> -nullrhi.
Only uniquely named generated reports are written under Saved/AutomationReports.
Package bytes are disk footprint, not runtime RAM/VRAM or compatibility evidence.
"""
import collections
import datetime
import json
import pathlib
import uuid
import unreal

project = pathlib.Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())).resolve()
content = project / "Content"
registry = unreal.AssetRegistryHelpers.get_asset_registry()
registry.scan_paths_synchronous(["/Game"], False)
assets = registry.get_assets_by_path("/Game", recursive=True, include_only_on_disk_assets=True)
if not assets:
    raise RuntimeError("[PrimalAssetAudit] No on-disk /Game registry assets; not a successful audit")

disk = {}
groups = {}
for path in sorted(content.rglob("*")):
    if not path.is_file() or path.suffix.lower() not in (".uasset", ".umap"):
        continue
    relative = path.relative_to(content).as_posix()
    folder = relative.split("/", 1)[0]
    size = path.stat().st_size
    disk["/Game/" + relative.rsplit(".", 1)[0]] = {"path": relative, "bytes": size}
    group = groups.setdefault(folder, {"files": 0, "diskBytes": 0, "registryAssets": 0, "classes": collections.Counter()})
    group["files"] += 1
    group["diskBytes"] += size

rows = []
missing_packages = []
redirectors = []
for asset in sorted(assets, key=lambda item: (str(item.package_name), str(item.asset_name))):
    package = str(asset.package_name)
    folder = package.split("/")[2]
    kind = str(asset.asset_class_path.asset_name)
    entry = {"package": package, "asset": str(asset.asset_name), "class": kind}
    entry.update(disk.get(package, {}))
    group = groups.setdefault(folder, {"files": 0, "diskBytes": 0, "registryAssets": 0, "classes": collections.Counter()})
    group["registryAssets"] += 1
    group["classes"][kind] += 1
    if package not in disk:
        missing_packages.append(package)
    if kind == "ObjectRedirector":
        redirectors.append(package)
    # Metadata only. Absent tags are unknown, never a compliance pass.
    tags = {}
    for tag in ("ImportedSize", "Dimensions", "Triangles", "Vertices", "NumLODs", "LODGroup", "Skeleton"):
        value = unreal.AssetRegistryHelpers.get_tag_value(asset, tag)
        if value:
            tags[tag] = str(value)
    if tags:
        entry["metadataTags"] = tags
    rows.append(entry)

report = {
    "schemaVersion": 1,
    "timestampUtc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
    "engineVersion": unreal.SystemLibrary.get_engine_version(),
    "result": "InventoryCompleted",
    "scope": "Read-only on-disk /Game registry and Content package file statistics",
    "assetObjectsLoadedByScript": 0,
    "assetObjectsSavedByScript": 0,
    "limitations": ["Not a license/provenance audit or approval", "Not runtime memory/VRAM", "Not render/collision/skeleton/LOD compatibility validation", "Registry tags may be absent or stale", "No dependency/fixup/cook/load validation"],
    "folders": groups,
    "registryAssets": len(rows),
    "packageFiles": len(disk),
    "diskBytes": sum(row["bytes"] for row in disk.values()),
    "registryPackagesWithoutFile": sorted(set(missing_packages)),
    "redirectors": sorted(set(redirectors)),
    "assets": rows,
}
stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%SZ")
out = project / "Saved" / "AutomationReports" / ("M9AssetInventory_" + stamp + "_" + uuid.uuid4().hex)
out.mkdir(parents=True, exist_ok=False)
with (out / "inventory.json").open("x", encoding="utf-8") as handle:
    json.dump(report, handle, indent=2, sort_keys=True)
lines = ["# M9 read-only asset inventory", "", "Inventory completed; this is not asset approval or runtime performance evidence.", "", "| Folder | Package files | Disk GiB | Registry assets |", "| --- | ---: | ---: | ---: |"]
for name, group in sorted(groups.items()):
    lines.append("| {} | {} | {:.3f} | {} |".format(name, group["files"], group["diskBytes"] / 1024**3, group["registryAssets"]))
lines += ["", "Registry redirectors: {}. Registry packages without a matching package file: {}.".format(len(redirectors), len(set(missing_packages))), "", "No asset objects loaded/saved by this script; no imports, fixes or redirector deletions."]
with (out / "summary.md").open("x", encoding="utf-8") as handle:
    handle.write("\n".join(lines) + "\n")
unreal.log("[PrimalAssetAudit] InventoryCompleted assets={} packageFiles={} diskGiB={:.3f} redirectors={}; report={}".format(len(rows), len(disk), report["diskBytes"] / 1024**3, len(redirectors), out))
