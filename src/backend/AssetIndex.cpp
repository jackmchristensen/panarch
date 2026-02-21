#include <backend/AssetIndex.h>
#include <QDirIterator>
#include <QFileInfo>

static bool isUsdExt(const QString& ext) {
  const auto e = ext.toLower();
  return (e == "usd" || e == "usda" || e == "usdc" || e == "usdz");
}

QVector<AssetRecord> AssetIndex::scan(const QString& rootDir) {
  QVector<AssetRecord> out;

  QDirIterator it(rootDir, QDir::Files, QDirIterator::Subdirectories);
  while (it.hasNext()) {
    const QString path = it.next();
    QFileInfo fi(path);
    const QString ext = fi.suffix();

    if (!isUsdExt(ext)) continue;

    AssetRecord rec;
    rec.path = fi.absoluteFilePath();
    rec.name = fi.completeBaseName();
    rec.type = ext.toLower();

    // Sidecar files for thumbnail. Would like to replace with an auto thumbnail renderer in the future
    // asset.usd.png or asset.png next to asset.usd
    const QString thumb0 = fi.absolutePath() + ".png";
    const QString thumb1 = fi.absolutePath() + "/" + fi.completeBaseName() + ".png";

    if (QFileInfo::exists(thumb0)) rec.thumbnail = thumb0;
    else if (QFileInfo::exists(thumb1)) rec.thumbnail = thumb1;

    out.push_back(rec);
  }

  return out;
}

