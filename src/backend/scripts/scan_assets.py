"""
scan_assets.py <root_dir>

Outputs a JSON array of USD file paths that are DEPENDENCIES (referenced
by other USD files in the tree). C++ uses this list to exclude them from
results.
"""

import os, sys, json
from collections import defaultdict

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

    for sub in layer.subLayerPaths:
        r = resolve_against_layer(layer, sub)
        if r:
            deps.add(r)

    for ref in layer.GetExternalReferences():
        r = resolve_against_layer(layer, ref)
        if r:
            deps.add(r)

    return deps


def build_inbound_graph(root_dir: str):
    usd_files = list(iter_usd_files(root_dir))
    usd_set = set(usd_files)

    inbound = defaultdict(int)      # file -> number of other files pointing to it
    outbound = defaultdict(set)     # file -> deps

    for f in usd_files:
        deps = collect_layer_deps(f)
        local_deps = { d for d in deps if d in usd_set }
        outbound[f] = local_deps
        for d in local_deps:
            inbound[d] += 1

    return usd_files, inbound, outbound


def main():
    if len(sys.argv) < 2:
        print("Usage: python scan_assets.py <root_dir>")
        sys.exit(2)

    root_dir = norm(sys.argv[1])
    usd_files, inbound, outbound = build_inbound_graph(root_dir)

    entry_candidates = [f for f in usd_files if inbound[f] == 0]
    internal_layers  = [f for f in usd_files if inbound[f] > 0]

    print(json.dumps(sorted(entry_candidates)))

    # print("\nLikely entry layers (inboud == 0):")
    # for f in sorted(entry_candidates):
    #     print(f"  {f}")
    #
    # print("\nLikely internal layers (inbound > 0):")
    # for f in sorted(internal_layers, key=lambda x: inbound[x], reverse=True):
    #     print(f"  ({inbound[f]}) {f}")


if __name__ == "__main__":
    main()
