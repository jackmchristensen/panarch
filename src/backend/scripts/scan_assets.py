"""
scan_assets.py <root_dir>

Outputs a JSON array of USD file paths that are DEPENDENCIES (referenced
by other USD files in the tree). C++ uses this list to exclude them from
results.
"""

import os, sys, json, hashlib
from collections import defaultdict
from typing import Any

try:
    from pxr import Sdf
except ImportError:
    sys.stderr.write("pxr not available\n")
    sys.exit(1)


USD_EXTS = { ".usd", ".usda", ".usdc" }


def norm(p: str) -> str:
    return os.path.normpath(os.path.abspath(p))


def iter_usd_files(root_dir: str):
    for dp, _, files in os.walk(root_dir):
        for f in files:
            ext = os.path.splitext(f)[1].lower()
            if ext in USD_EXTS:
                yield norm(os.path.join(dp, f))


def resolve_against_layer(layer: Sdf.Layer, asset_path: str) -> str | None: # pyright: ignore
    try:
        resolved = Sdf.ComputeAssetPathRelativeToLayer(layer, asset_path) # pyright: ignore
    except Exception:
        resolved = asset_path

    if not resolved:
        return None

    # Ignore non-file/non-local dependencies
    # (e.g. "omniverse://", "http://", etc.)
    if "://" in resolved:
        return None

    if not os.path.isabs(resolved):
        base = os.path.dirname(layer.realPath) if layer.realPath else ""
        resolved = os.path.join(base, resolved)

    return norm(resolved)


def collect_layer_deps(layer_path: str) -> set[str]:
    deps: set[str] = set()
    layer = Sdf.Layer.FindOrOpen(layer_path) #pyright: ignore
    if not layer:
        return deps

    # layer.GetPrimAtPath(layer.GetDefaultPrimAsPath()).kind) -> AssetRecord.kind
    # layer.defaultPrimAsPath -> AssetRecord.defaultPrimPath
    # layer.timeCodesPerSecond -> AssetDetails.timeCodesPerSecond
    # layer.framesPerSecond -> AssetDetails.framesPerSecond

    for sub in layer.subLayerPaths:
        r = resolve_against_layer(layer, sub)
        if r:
            deps.add(r)

    for ref in layer.GetExternalReferences():
        r = resolve_against_layer(layer, ref)
        if r:
            deps.add(r)

    return deps


def check_prim_spec(spec: Sdf.PrimSpec) -> tuple[bool, bool, bool]:
    has_variants = bool(spec.variantSets)
    has_payloads = len(list(spec.payloadList.explicitItems)) > 0 or \
                    len(list(spec.payloadList.addedItems)) > 0 or \
                    len(list(spec.payloadList.prependedItems)) > 0 or \
                    len(list(spec.payloadList.appendedItems)) > 0
    has_references = len(list(spec.referenceList.explicitItems)) > 0 or \
                    len(list(spec.referenceList.addedItems)) > 0 or \
                    len(list(spec.referenceList.prependedItems)) > 0 or \
                    len(list(spec.referenceList.appendedItems)) > 0

    for child in spec.nameChildren:
        v, p, r = check_prim_spec(child)
        has_variants = has_variants or v
        has_payloads = has_payloads or p
        has_references = has_references or r

    return has_variants, has_payloads, has_references


def collect_asset_data(layer_path: str) -> dict[str, str]:
    layer = Sdf.Layer.FindOrOpen(layer_path) # pyright: ignore
    if not layer:
        return {}

    defaultPrimPath = layer.GetDefaultPrimAsPath()
    prim = layer.GetPrimAtPath(defaultPrimPath)

    has_variants, has_payloads, has_references = check_prim_spec(prim)

    recordData = {
        'defaultPrimPath': defaultPrimPath,
        'kind': prim.kind,
        'displayName': prim.name,
        'hasVariants': has_variants,
        'hasPayloads': has_payloads,
        'hasReferences': has_references
    }

    return recordData


def build_inbound_graph(root_dir: str):
    usd_files = list(iter_usd_files(root_dir))
    usd_set = set(usd_files)

    inbound = defaultdict(int)      # file -> number of other files pointing to it
    outbound = defaultdict(set)     # file -> deps
    asset_data: dict[str, dict[str, str]] = {}

    for f in usd_files:
        deps = collect_layer_deps(f)
        local_deps = { d for d in deps if d in usd_set }
        asset_data[f] = collect_asset_data(f)
        outbound[f] = local_deps
        for d in local_deps:
            inbound[d] += 1

    return usd_files, inbound, outbound, asset_data


def main():
    if len(sys.argv) < 2:
        print("Usage: python scan_assets.py <root_dir>")
        sys.exit(2)

    root_dir = norm(sys.argv[1])
    usd_files, inbound, outbound, asset_data = build_inbound_graph(root_dir)

    entry_candidates = [f for f in usd_files if inbound[f] == 0]
    internal_layers  = [f for f in usd_files if inbound[f] > 0]

    records: list[dict[str, Any]] = [
        {"entryPath": path}
        for path in sorted(entry_candidates)
    ]

    for rec in records:
        path = rec.get("entryPath")
        assert path is not None
        rec["id"] = hashlib.sha1(path.encode('utf-8')).hexdigest()
        rec["size"] = os.path.getsize(path)
        rec["mtime"] = os.path.getmtime(path)
        tmp = asset_data.get(path)
        assert tmp is not None
        rec["defaultPrimPath"] = str(tmp.get("defaultPrimPath"))
        rec["kind"] = str(tmp.get("kind"))
        rec["displayName"] = str(tmp.get("displayName"))
        rec["hasVariants"] = bool(tmp.get("hasVariants"))
        rec["hasPayloads"] = bool(tmp.get("hasPayloads"))
        rec["hasReferences"] = bool(tmp.get("hasReferences"))

    print(json.dumps(records))

    # print("\nLikely entry layers (inboud == 0):")
    # for f in sorted(entry_candidates):
    #     print(f"  {f}")
    #
    # print("\nLikely internal layers (inbound > 0):")
    # for f in sorted(internal_layers, key=lambda x: inbound[x], reverse=True):
    #     print(f"  ({inbound[f]}) {f}")


if __name__ == "__main__":
    main()
