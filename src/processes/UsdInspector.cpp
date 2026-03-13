#include "processes/UsdInspector.h"

#include <QJsonObject>
#include <QJsonArray>

static QJsonArray toJsonArray(const QSet<QString>& set) {
  QJsonArray arr;
  for (const auto& s : set)
    arr.append(s);
  return arr;
}

QJsonObject UsdInspector::primToJson(const pxr::UsdPrim& prim) {
  QJsonObject obj;
  obj["name"] = QString::fromStdString(prim.GetName());
  obj["path"] = QString::fromStdString(prim.GetPath().GetString());
  obj["typeName"] = QString::fromStdString(prim.GetTypeName().GetString());
  obj["isActive"] = prim.IsActive();
  obj["hasVariants"] = prim.HasVariantSets();

  QJsonArray children;
  for (const auto& child : prim.GetChildren())
    children.append(primToJson(child));

  obj["children"] = children;
  return obj;
}

QJsonObject UsdInspector::collectCompositionArcs(pxr::UsdStageRefPtr stage) {
  QJsonObject result;
  QSet<QString> sublayers;
  QSet<QString> payloads;
  QSet<QString> references;

  for (const auto& layer : stage->GetLayerStack()) {
    for (const auto& path : layer->GetSubLayerPaths()) {
      sublayers.insert(QString::fromStdString(path));
    }
  }

  for (const auto& layer : stage->GetUsedLayers()) {
    for (const auto& path : layer->GetSubLayerPaths()) {
      sublayers.insert(QString::fromStdString(path));
    }
  }

  // Only collect arcs from the root layer stack,
  // ignoring arcs that came from referenced files
  pxr::SdfLayerHandle rootLayer = stage->GetRootLayer();

  auto isOwnedByRootAsset = [&](const pxr::PcpNodeRef& node) -> bool {
    auto parent = node.GetParentNode();
    if (!parent) return false;

    auto parentRootLayer = parent.GetLayerStack()->GetIdentifier().rootLayer;

    if (parentRootLayer == rootLayer) return true;

    if (parent.GetArcType() == pxr::PcpArcTypePayload) {
      auto grandparent = parent.GetParentNode();
      if (!grandparent) return false;

      auto grandparentRootLayer = grandparent.GetLayerStack()->GetIdentifier().rootLayer;
      return grandparentRootLayer == rootLayer;
    }

    return false;
  };

  for (const auto& prim : stage->Traverse()) {
    auto primIndex = prim.GetPrimIndex();
    for (const auto& node : primIndex.GetNodeRange()) {
      auto arc = node.GetArcType();
      if (arc != pxr::PcpArcTypeReference && arc != pxr::PcpArcTypePayload)
        continue;

      if (!isOwnedByRootAsset(node)) continue;

      std::string path = node.GetLayerStack()->GetIdentifier().rootLayer->GetIdentifier();

      if (arc == pxr::PcpArcTypeReference)
        references.insert(QString::fromStdString(path));
      else if (arc == pxr::PcpArcTypePayload)
        payloads.insert(QString::fromStdString(path));
    }
  }

  result["sublayers"]   = toJsonArray(sublayers);
  result["payloads"]    = toJsonArray(payloads);
  result["references"]  = toJsonArray(references);
  return result;
}

QJsonObject UsdInspector::getVariantSets(pxr::UsdPrim prim) {
  QJsonObject result;
  for (const auto& name : prim.GetVariantSets().GetNames()) {
    auto variantSet = prim.GetVariantSets().GetVariantSet(name);
 
    QJsonArray variants;
    for (const auto& v : variantSet.GetVariantNames()) 
      variants.append(QString::fromStdString(v));

    QJsonObject variantSetObj;
    variantSetObj["variants"] = variants;
    variantSetObj["selected"] = QString::fromStdString(variantSet.GetVariantSelection());

    result[QString::fromStdString(name)] = variantSetObj;
  }

  return result;
}

QJsonObject UsdInspector::inspectAsset(const QString& assetPath) {
  auto stage = pxr::UsdStage::Open(assetPath.toStdString());

  QJsonObject result;
  if (!stage) {
    result["error"] = "Failed to open stage";
    return result;
  }

  pxr::UsdPrim defaultPrim = stage->GetDefaultPrim();
  auto arcs = collectCompositionArcs(stage);

  // Stage-level metadata
  result["upAxis"]              = QString::fromStdString(pxr::UsdGeomGetStageUpAxis(stage));
  result["metersPerUnit"]       = pxr::UsdGeomGetStageMetersPerUnit(stage);
  result["framesPerSecond"]     = stage->GetFramesPerSecond();
  result["timeCodesPerSecond"]  = stage->GetTimeCodesPerSecond();

  // Composition
  result["sublayers"]   = arcs["sublayers"];
  result["payloads"]    = arcs["payloads"];
  result["references"]  = arcs["references"];

  // Prim tree
  // skipping "/" prim
  QJsonArray topLevel;
  for (const auto& child : stage->GetPseudoRoot().GetAllChildren()) {
    topLevel.append(primToJson(child));
  }
  result["primTree"] = topLevel;

  // Variant sets on default prim
  result["variantSets"] = defaultPrim.IsValid() ? getVariantSets(defaultPrim) : QJsonObject{};

  return result;
}
