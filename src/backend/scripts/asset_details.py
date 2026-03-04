import os, sys
from typing import Any

try:
    from pxr import Usd, UsdGeom, Sdf
except ImportError:
    sys.stderr.write("pxr not available\n")
    sys.exit(1)


def collect_arcs(spec: Sdf.PrimSpec, payloads: list, references: list): # pyright: ignore
    for item in spec.payloadList.prependedItems:
        payloads.append(str(item.assetPath))
    for item in spec.referenceList.prependedItems:
        references.append(str(item.assetPath))
    for child in spec.nameChildren:
        collect_arcs(child, payloads, references)


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
    stage = Usd.Stage.Open(assetPath) # pyright: ignore
    if not stage:
        sys.exit(1)

    rootLayer = stage.GetRootLayer()
    prim = stage.GetDefaultPrim()

    payloads: list[str] = []
    references: list[str] = []
    primSpec = rootLayer.GetPrimAtPath(rootLayer.defaultPrim)
    collect_arcs(primSpec, payloads, references)

    details = {
        "upAxis": UsdGeom.GetStageUpAxis(stage), # pyright: ignore
        "metersPerUnit": UsdGeom.GetStageMetersPerUnit(stage), # pyright: ignore
        "framesPerSecond": stage.GetFramesPerSecond(),
        "timeCodesPerSecond": stage.GetTimeCodesPerSecond(),
        "sublayers": list(rootLayer.subLayerPaths),
        "payloads": payloads,
        "references": references,
        "primCount": sum(1 for _ in stage.Traverse()),
        "variantSets": get_variant_sets(prim) if prim else {}
    }

    print(details)

if __name__ == "__main__":
    main()
