#include "pxr/usd/sdf/primSpec.h"
#include "pxr/usd/sdf/reference.h"
#include "pxr/usd/sdf/payload.h"
#include "pxr/usd/sdf/layer.h"
#include "pxr/usd/sdf/layerUtils.h"
#include "pxr/usd/usd/stage.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usdGeom/metrics.h"
#include "pxr/usd/usd/variantSets.h"
#include "pxr/usd/usd/primRange.h"

#include <backend/AssetIndex.h>

#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <qcborarray.h>
#include <qjsonobject.h>
#include <qtypes.h>

namespace {

struct ScanResult {
  QVector<QString> usdFiles;
  QMap<QString, qint32> inbound;
  QMap<QString, std::set<std::string>> outbound;
  QMap<QString, AssetRecord> assetData;
};

struct Arcs {
  QVector<QString> sublayers;
  QVector<QString> payloads;
  QVector<QString> references;
};

struct VariantSetInfo {
  QStringList variants;
  QString selected;
};

static bool isUsdExt(const QString& ext) {
  const auto e = ext.toLower();
  return (e == "usd" || e == "usda" || e == "usdc" || e == "usdz");
}

QVector<QString> iterateUsdFiles(QString rootDir) {
  QDirIterator it(rootDir, {"*.usd", "*.usda", "*.usdc"}, 
                  QDir::Files,
                  QDirIterator::Subdirectories);

  QVector<QString> usdFiles;
  while (it.hasNext()) {
    usdFiles.push_back(it.next());
  }

  return usdFiles;
}

std::optional<std::string> resolveAgainstLayer(pxr::SdfLayerHandle layer, std::string assetPath) {
  if (assetPath == "") 
    return std::nullopt;

  std::string resolved = pxr::SdfComputeAssetPathRelativeToLayer(layer, assetPath);

  if (resolved.find("://") != std::string::npos) 
    return std::nullopt;

  if (!QDir::isAbsolutePath(QString::fromStdString(resolved))) {
    QDir dir;
    resolved = dir.absoluteFilePath(QString::fromStdString(resolved)).toStdString();
  }

  return QDir::cleanPath(QString::fromStdString(resolved)).toStdString();
}

std::set<std::string> collectLayerDeps(std::string layerPath) {
  std::set<std::string> dependencies;
  auto layer = pxr::SdfLayer::FindOrOpen(layerPath);
  if (layer == nullptr) {
    return dependencies;
  }

  for (auto sub : layer->GetSubLayerPaths()) {
    auto res = resolveAgainstLayer(layer, sub);
    if (res) {
      dependencies.insert(*res);
    }
  }

  for (auto ref : layer->GetExternalReferences()) {
    auto res = resolveAgainstLayer(layer, ref);
    if (res) {
      dependencies.insert(*res);
    }
  }

  return dependencies;
}

std::vector<bool> checkPrimSpec(pxr::SdfPrimSpecHandle spec) {
  bool hasVariants = !spec->GetVariantSets().empty();
  bool hasPayloads = !spec->GetPayloadList().GetAddedOrExplicitItems().empty();
  bool hasReferences = !spec->GetReferenceList().GetAddedOrExplicitItems().empty();

  for (auto child : spec->GetNameChildren()) {
    std::vector<bool> checks = checkPrimSpec(child);
    hasVariants = hasVariants | checks[0];
    hasPayloads = hasPayloads | checks[1];
    hasReferences = hasReferences | checks[2];
  }

  return std::vector<bool>{ hasVariants, hasPayloads, hasReferences };
}

AssetRecord collectAssetData(std::string layerPath) {
  AssetRecord record;
  auto layer = pxr::SdfLayer::FindOrOpen(layerPath);

  if (layer == nullptr) {
    return record;
  }

  pxr::SdfPath defaultPrimPath = layer->GetDefaultPrimAsPath();
  pxr::SdfPrimSpecHandle prim = layer->GetPrimAtPath(defaultPrimPath);

  std::vector<bool> checks = checkPrimSpec(prim);

  typename decltype(prim->GetPayloadList().GetPrependedItems())::value_type x;
  // if has variants
  if (checks[1]) {
    for (const pxr::SdfPayload& item : prim->GetPayloadList().GetPrependedItems()) {
      auto payloadPath = resolveAgainstLayer(layer, item.GetAssetPath());
      if (!payloadPath) continue;

      auto payloadLayer = pxr::SdfLayer::FindOrOpen(*payloadPath);
      if (payloadLayer == nullptr) continue;

      pxr::SdfPath payloadPrimPath = payloadLayer->GetDefaultPrimAsPath();
      pxr::SdfPrimSpecHandle payloadPrim = payloadLayer->GetPrimAtPath(payloadPrimPath);

      std::vector<bool> payloadChecks = checkPrimSpec(payloadPrim);
      checks[0] = checks[0] | payloadChecks[0]; // has variants
      checks[2] = checks[2] | payloadChecks[2]; // has references
    }
  }

  record = (AssetRecord){
    .displayName = QString::fromStdString(prim->GetName()),
    .kind = QString::fromStdString(prim->GetKind().GetString()),
    .defaultPrimPath = QString::fromStdString(defaultPrimPath.GetString()),
    .hasVariants = checks[0],
    .hasPayloads = checks[1],
    .hasReferences = checks[2]
  };

  return record;
}

ScanResult buildInboundGraph(const QString& rootDir) {
  QVector<QString> usdFiles = iterateUsdFiles(rootDir);
  ScanResult result = (ScanResult) { .usdFiles = usdFiles };

  for (const QString& file : usdFiles) {
    const std::string& f = file.toStdString();
    auto dependencies = collectLayerDeps(f);
    std::set<std::string> localDeps;

    for (const auto& d : dependencies) {
      if (std::find(usdFiles.begin(), usdFiles.end(), d) != usdFiles.end()) {
        localDeps.insert(d);
      }
    }

    result.outbound.insert(QString::fromStdString(f), localDeps);
    for (auto d : localDeps) {
      QString key = QString::fromStdString(d);
      if (result.inbound.contains(key)) {
        result.inbound.insert(key, result.inbound.value(key) + 1);
      } else {
        result.inbound.insert(key, 1);
      }
    }
    result.assetData.insert(QString::fromStdString(f), collectAssetData(f));
  }

  return result;
}


void collectPrimArcs(const pxr::SdfPrimSpecHandle& spec, QVector<QString>& payloads, QVector<QString>& references) {
  for (const auto& item : spec->GetPayloadList().GetPrependedItems()) {
    pxr::SdfPayload payload = item;
    if (!payload.GetAssetPath().empty()) {
      payloads.append(QString::fromStdString(payload.GetAssetPath()));
    }
  }

  for (const auto& item : spec->GetReferenceList().GetPrependedItems()) {
    pxr::SdfReference reference = item;
    if (!reference.GetAssetPath().empty()) {
      references.append(QString::fromStdString(reference.GetAssetPath()));
    }
  }

  for (const auto& item : spec->GetNameChildren()) {
    pxr::SdfPrimSpecHandle child = item;
    collectPrimArcs(child, payloads, references);
  }
}

Arcs collectArcs(pxr::UsdStageRefPtr stage) {
  Arcs arc;

  for (const auto& layer : stage->GetUsedLayers()) {
    for (const auto& path : layer->GetSubLayerPaths()) {
      arc.sublayers.append(QString::fromStdString(path));
    }

    pxr::SdfPath primPath = layer->GetDefaultPrimAsPath();
    if (primPath.IsEmpty()) continue;

    auto primSpec = layer->GetPrimAtPath(primPath);
    if (primSpec == nullptr) continue;

    collectPrimArcs(primSpec, arc.payloads, arc.references);
  }

  return arc;
}

QJsonObject getVariantSets(pxr::UsdPrim prim) {
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

} // namespace

QVector<AssetRecord> AssetIndex::scan(const QString& rootDir){
  ScanResult result = buildInboundGraph(rootDir);

  QVector<QString> entryCandidates;
  QVector<QString> internalLayers;
  for (const auto& f : result.usdFiles) {
    if (result.inbound[f] == 0) {
      entryCandidates.append(f);
    }
    if (result.inbound[f] > 0) {
      internalLayers.append(f);
    }
  }

  QVector<AssetRecord> out;
  for (const auto& path : entryCandidates) {
    AssetRecord record = result.assetData.value(path);

    QFileInfo fi(path);
    record.entryPath = path;
    record.fileExt = fi.suffix().toLower();
    record.fileSize = fi.size();
    record.mtime = fi.lastModified(QTimeZone::UTC);

    const QString thumb0 = fi.absoluteFilePath() + ".png";
    const QString thumb1 = fi.absolutePath() + "/" + fi.completeBaseName() + ".png";

    if (QFileInfo::exists(thumb0)) record.thumbnailPath = thumb0;
    else if (QFileInfo::exists(thumb1)) record.thumbnailPath = thumb1;
 
    out.append(record);
  }

  return out;
}

AssetDetails AssetIndex::getAssetDetails(const QString& assetPath) {
  auto stage = pxr::UsdStage::Open(assetPath.toStdString());
  AssetDetails details;
  if (stage == nullptr) {
    return details;
  }

  pxr::UsdPrim prim = stage->GetDefaultPrim();
  Arcs arcs = collectArcs(stage);

  details = AssetDetails {
    .upAxis = QString::fromStdString(pxr::UsdGeomGetStageUpAxis(stage).GetString()),
    .metersPerUnit = pxr::UsdGeomGetStageMetersPerUnit(stage),
    .framesPerSecond = stage->GetFramesPerSecond(),
    .timeCodesPerSecond = stage->GetTimeCodesPerSecond(),
    .sublayers = arcs.sublayers,
    .payloads = arcs.payloads,
    .references = arcs.references,
    .primCount = [&]() {
      auto range = stage->Traverse();
      return (int)std::distance(range.begin(), range.end());
    }(),
    .variantSets = prim.IsValid() ? getVariantSets(prim) : QJsonObject {}
  };

  return details;
}
