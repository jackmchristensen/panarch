import os, sys, json
from typing import Any

try:
    from pxr import Usd, UsdGeom, Sdf
except ImportError:
    sys.stderr.write("pxr not available\n")
    sys.exit(1)


def _collect_prim_arcs(spec: Sdf.PrimSpec, payloads: list, references: list):
    for item in spec.payloadList.prependedItems:
        if str(item.assetPath) != "":
            payloads.append(str(item.assetPath))
    for item in spec.referenceList.prependedItems:
        if str(item.assetPath) != "":
            references.append(str(item.assetPath))
    for child in spec.nameChildren:
        _collect_prim_arcs(child, payloads, references)


def collect_arcs(stage: Usd.Stage): # pyright: ignore
    payloads = []
    references = []
    sublayers = []

    for layer in stage.GetUsedLayers():
        sublayers.extend(list(layer.subLayerPaths))
        primPath = layer.GetDefaultPrimAsPath()
        if not primPath:
            continue
        primSpec = layer.GetPrimAtPath(primPath)
        if not primSpec:
            continue
        _collect_prim_arcs(primSpec, payloads, references)

    return {
        "sublayers": sublayers,
        "payloads": payloads,
        "references": references
    }


def get_variant_sets(prim: Usd.Prim) -> dict: # pyright: ignore
    result = {}
    for name in prim.GetVariantSets().GetNames():
        vset = prim.GetVariantSets().GetVariantSet(name)
        result[name] = {
            "variants": vset.GetVariantNames(),
            "selected": vset.GetVariantSelection()
        }
    return result


def main():
    if len(sys.argv) < 2:
        sys.exit(2)

    assetPath = os.path.normpath(os.path.abspath(sys.argv[1]))
    stage = Usd.Stage.Open(assetPath, Usd.Stage.LoadAll) # pyright: ignore
    if not stage:
        sys.exit(1)

    prim = stage.GetDefaultPrim()
    arcs = collect_arcs(stage)

    details = {
        "upAxis": UsdGeom.GetStageUpAxis(stage), # pyright: ignore
        "metersPerUnit": UsdGeom.GetStageMetersPerUnit(stage), # pyright: ignore
        "framesPerSecond": stage.GetFramesPerSecond(),
        "timeCodesPerSecond": stage.GetTimeCodesPerSecond(),
        "sublayers": arcs["sublayers"],
        "payloads": arcs["payloads"],
        "references": arcs["references"],
        "primCount": sum(1 for _ in stage.Traverse()),
        "variantSets": get_variant_sets(prim) if prim else {}
    }

    print(json.dumps(details))

if __name__ == "__main__":
    main()
