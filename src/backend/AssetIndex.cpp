#include <backend/AssetIndex.h>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <qdebug.h>

static bool isUsdExt(const QString& ext) {
  const auto e = ext.toLower();
  return (e == "usd" || e == "usda" || e == "usdc" || e == "usdz");
}

QVector<AssetRecord> AssetIndex::scan(const QString& rootDir) {
  QVector<AssetRecord> out;

  QString exeDir = QCoreApplication::applicationDirPath();
  QString scriptPath = QDir(exeDir).filePath("scripts/scan_assets.py");

  QProcess proc;
  QString program = "python";
  QStringList args;
  args << scriptPath << rootDir;

  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("LD_PRELOAD", "/usr/lib/libjemalloc.so.2");
  proc.setProcessEnvironment(env);

  proc.start(program, args);
  proc.waitForFinished();
 
  QByteArray stdoutData = proc.readAllStandardOutput();
  QJsonDocument doc = QJsonDocument::fromJson(stdoutData);

  if(!doc.isArray())
    return out;

  QJsonArray arr = doc.array();

  for(const QJsonValue& val : arr) {
    if (!val.isObject())
      continue;

    QJsonObject obj = val.toObject();

    QString path = obj["entryPath"].toString();
    QFileInfo fi(path);

    AssetRecord rec;
    rec.id = obj["id"].toString();
    rec.entryPath = obj["entryPath"].toString();
    rec.displayName = obj["displayName"].toString();
    rec.fileExt = fi.suffix().toLower();
    rec.mtime.setSecsSinceEpoch(static_cast<qint64>(obj["mtime"].toInteger()));
    rec.fileSize = static_cast<quint64>(obj["size"].toInteger());
    rec.kind = obj["kind"].toString();
    rec.defaultPrimPath = obj["defaultPrimPath"].toString();

    // Sidecar files for thumbnail. Would like to replace with an auto thumbnail renderer in the future
    // asset.usd.png or asset.png next to asset.usd
    const QString thumb0 = fi.absoluteFilePath() + ".png";
    const QString thumb1 = fi.absolutePath() + "/" + fi.completeBaseName() + ".png";

    if (QFileInfo::exists(thumb0)) rec.thumbnailPath = thumb0;
    else if (QFileInfo::exists(thumb1)) rec.thumbnailPath = thumb1;

    out.push_back(rec);
  }

  return out;
}

