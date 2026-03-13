#include "processes/ScanAssets.h"
#include "shared/AssetTypes.h"

#include <QCryptographicHash>
#include <QStandardPaths>

namespace {

struct ScanResult {
  QVector<QString> usdFiles;
  QMap<QString, qint32> inbound;
  QMap<QString, std::set<std::string>> outbound;
  QMap<QString, AssetRecord> assetData;
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
  };

  return record;
}

bool hasExternalDeps(const QString& assetPath, const std::set<std::string>& deps) {
  QFileInfo assetInfo(assetPath);
  QString assetDir = assetInfo.absolutePath();

  for (const auto& dep : deps) {
    QString depPath = QString::fromStdString(dep);
    if (!depPath.startsWith(assetDir)) return true;
  }
  return false;
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

    result.assetData[file].hasExternalDeps = hasExternalDeps(file, dependencies);
  }

  return result;
}

} // namespace

QJsonObject scanAssets(const QString &rootDir) {
  ScanResult result =  buildInboundGraph(rootDir);

  QJsonArray assets;
  for (const QString& file : result.usdFiles) {
    if (result.inbound.value(file, 0) > 0) continue;

    const AssetRecord& rec = result.assetData.value(file);
    QFileInfo fi(file);
    QString id = QString(QCryptographicHash::hash(file.toUtf8(), QCryptographicHash::Sha1).toHex());

    QString thumbnailPath;
    const QString thumb0 = fi.absoluteFilePath() + ".png";
    const QString thumb1 = fi.absolutePath() + "/" + fi.completeBaseName() + ".png";
    const QString thumb2 = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/thumbnails/" + id + ".thumbnail.jpg";

    if      (QFileInfo::exists(thumb0)) thumbnailPath = thumb0;
    else if (QFileInfo::exists(thumb1)) thumbnailPath = thumb1;
    else if (QFileInfo::exists(thumb2)) thumbnailPath = thumb2;

    QJsonObject asset;
    asset["id"]              = id;
    asset["path"]            = file;
    asset["displayName"]     = rec.displayName;
    asset["fileExt"]         = fi.suffix().toLower();
    asset["fileSize"]        = (qint64)fi.size();
    asset["mtime"]           = fi.lastModified(QTimeZone::UTC).toString(Qt::ISODate);
    asset["kind"]            = rec.kind;
    asset["defaultPrimPath"] = rec.defaultPrimPath;
    asset["thumbnailPath"]   = thumbnailPath;
    asset["hasVariants"]     = rec.hasVariants;
    asset["hasPayloads"]     = rec.hasPayloads;
    asset["hasExternalDeps"] = rec.hasExternalDeps;

    assets.append(asset);
  }

  QJsonObject result_obj;
  result_obj["assets"] = assets;
  return result_obj;
}
