#include <QSettings>
#include <QClipboard>
#include <QGuiApplication>
#include <QProcess>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QModelIndex>
#include <QStandardPaths>

#include "backend/Backend.h"

Backend::Backend(QObject* parent) : QObject(parent) {
  m_filterModel.setSourceModel(&m_assets);
  m_filterModel.sort(0);
}

void Backend::initialize() {
  rescan();
  loadUserSettings();
}

void Backend::generateThumbnailAsync(const QString& assetPath, const QString& cachePath, const QString& assetId) {
  // Thumbnail rendering runs as a separate process rather than on a thread
  // because it needs its own OpenGL context and opens a UsdStage. Isolating it
  // also means a renderer crash can't take down the main application.
  QProcess* process = new QProcess(this);
  QString generatorPath = QCoreApplication::applicationDirPath() + "/thumbnail_generator";
  process->start(generatorPath,
                 QStringList() << assetPath << cachePath);

  connect(process, &QProcess::finished, this, [this, cachePath, assetId, process](int exitCode) {
    if (exitCode == 0) {
      m_assets.onThumbnailReady(assetId, cachePath);
    }

    process->deleteLater();
  });
}

AssetRecord Backend::recordFromJson(const QJsonObject& obj) {
  AssetRecord rec;
  rec.id              = obj["id"].toString();
  rec.entryPath       = obj["path"].toString();
  rec.displayName     = obj["displayName"].toString();
  rec.fileExt         = obj["fileExt"].toString();
  rec.fileSize        = (quint64)obj["fileSize"].toInteger();
  rec.mtime           = QDateTime::fromString(obj["mtime"].toString(), Qt::ISODate);
  rec.kind            = obj["kind"].toString();
  rec.defaultPrimPath = obj["defaultPrimPath"].toString();
  rec.thumbnailPath   = obj["thumbnailPath"].toString();
  rec.hasVariants     = obj["hasVariants"].toBool();
  rec.hasPayloads     = obj["hasPayloads"].toBool();
  rec.hasExternalDeps = obj["hasExternalDeps"].toBool();
  return rec;
}

void Backend::rescan() {
  QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/thumbnails");

  const QStringList roots = loadLibraryRoots();

  // Track how many processes are still running
  auto* pending = new QAtomicInt(roots.size());
  auto* allAssets = new QVector<AssetRecord>();

  for (const QString& root : roots) {
    QProcess* process = new QProcess(this);
    QString scanPath = QCoreApplication::applicationDirPath() + "/scan_assets";
    process->start(scanPath, QStringList() << root);

    connect(process, &QProcess::finished, this, [this, process, pending, allAssets](int exitCode) {
      if (exitCode == 0) {
        QByteArray output = process->readAllStandardOutput();
        QJsonDocument doc = QJsonDocument::fromJson(output);

        for (const auto& val : doc.object()["assets"].toArray())
          allAssets->append(recordFromJson(val.toObject()));
      };

      process->deleteLater();

      // Update model once all processes finish
      if (pending->deref() == 0) {
        m_assets.setAssets(*allAssets);
        for (const AssetRecord& rec : *allAssets) {
          QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
            + "/thumbnails/" + rec.id + ".thumbnail.jpg";
          if (!QFile::exists(cachePath) && rec.thumbnailPath.isEmpty())
            generateThumbnailAsync(rec.entryPath, cachePath, rec.id);
        }
        delete pending;
        delete allAssets;
      }
    });
  }

  // auto* watcher = new QFutureWatcher<QVector<AssetRecord>>(this);
  //
  // connect(watcher, &QFutureWatcher<QVector<AssetRecord>>::finished, this, [this, watcher]() {
  //   QVector<AssetRecord> result = watcher->result();
  //   m_assets.setAssets(result);
  //
  //   for (const AssetRecord& rec : result) {
  //     QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/thumbnails/" + rec.id + ".thumbnail.jpg";
  //     if (!QFile::exists(cachePath) && rec.thumbnailPath == "") {
  //       generateThumbnailAsync(rec.entryPath, cachePath, rec.id);
  //     }
  //   }
  //
  //   watcher->deleteLater();
  // });
  //
  // auto future = QtConcurrent::run([this]() {
  //   const QStringList roots = loadLibraryRoots();
  //   QVector<AssetRecord> all;
  //   all.reserve(1024);
  //
  //   for (const QString& root: roots) {
  //     auto scanned = ;
  //     all += scanned;
  //   }
  //   return all;
  // });
  //
  // watcher->setFuture(future);
}

void Backend::addLibraryRoot(const QString& rootDir) {
  QStringList roots = loadLibraryRoots();
  if (!roots.contains(rootDir)) {
    roots.append(rootDir);
    saveLibraryRoots(roots);
    emit libraryRootsChanged();
  }
  rescan();
}

QStringList Backend::jsonArrayToStringList(const QJsonArray& arr) {
  QStringList result;
  for (const auto& val : arr)
    result.append(val.toString());
  return result;
}

AssetDetails Backend::parseInspectorOutput(const QJsonObject& assetData) {
  AssetDetails details;

  details.upAxis              = assetData["upAxis"].toString();
  details.metersPerUnit       = assetData["metersPerUnit"].toDouble();
  details.framesPerSecond     = assetData["framesPerSecond"].toDouble();
  details.timeCodesPerSecond  = assetData["timeCodesPerSecond"].toDouble();
  details.primCount           = assetData["primCount"].toInt();
  details.sublayers           = jsonArrayToStringList(assetData["sublayers"].toArray());
  details.payloads            = jsonArrayToStringList(assetData["payloads"].toArray());
  details.references          = jsonArrayToStringList(assetData["references"].toArray());
  details.primTree            = assetData["primTree"].toArray();
  details.variantSets         = assetData["variantSets"].toObject();

  return details;
}

void Backend::loadDetailsAsync(const QString& assetPath) {
  // TODO: The lambda captures [this] and reads m_selectedPath on the background
  // thread. Rapid selection changes can cause it to load details for the wrong
  // asset. Fix by capturing the path by value instead.
  QProcess* process = new QProcess(this);
  QString inspectorPath = QCoreApplication::applicationDirPath() + "/usd_inspector";

  connect(process, &QProcess::finished, this, [this, process](int exitCode) {
    if (exitCode == 0) {
      QByteArray output = process->readAllStandardOutput();
      QJsonDocument doc = QJsonDocument::fromJson(output);

      if (!doc.isNull()) {
        AssetDetails details = parseInspectorOutput(doc.object());
        m_details = details;
        m_loadingDetails = false;
        emit loadingDetailsChanged();
        emit detailsChanged();
      }
    }
    process->deleteLater();
  });

  process->start(inspectorPath, { assetPath });
}

void Backend::selectIndex(int proxyRow) {
  // Map from the filtered proxy row back to the source model before
  // accessing AssetRecord data.
  QModelIndex sourceIndex = m_filterModel.mapToSource(m_filterModel.index(proxyRow, 0));
  const AssetRecord* rec = m_assets.at(sourceIndex.row());
  if (!rec) return;

  m_selectedPath = rec->entryPath;
  m_selectedName = rec->displayName;
  m_selectedExt = rec->fileExt;
  m_selectedThumbnail = rec->thumbnailPath;
  m_selectedSize = rec->fileSize;
  m_selectedDefaultPrim = rec->defaultPrimPath;
  m_selectedKind = rec->kind;
  m_selectedMTime = rec->mtime;

  // Emit immediately with the cheap AssetRecord data so the UI can update
  // name, path, size etc. while the heavier stage load runs in the background.
  m_loadingDetails = true;
  emit loadingDetailsChanged();
  emit selectedChanged();
  loadDetailsAsync(m_selectedPath);
}

void Backend::selectFirst() {
  if (m_filterModel.rowCount() == 0) return;
  selectIndex(0);
}

void Backend::saveLibraryRoots(const QStringList& roots) {
  QSettings settings;
  settings.setValue("library/roots", roots);
}

QStringList Backend::loadLibraryRoots() {
  QSettings settings;
  return settings.value("library/roots").toStringList();
}

void Backend::saveUserSettings() {
  QSettings settings;
  settings.setValue("settings/sizeBase", static_cast<int>(m_settings.sizeBase));
}

void Backend::loadUserSettings() {
  QSettings settings;
  m_settings.sizeBase = static_cast<SizeBase>(settings.value("settings/sizeBase").toInt());
}

void Backend::removeLibraryRoot(const QString& path) {
  QStringList roots = loadLibraryRoots();
  roots.removeAll(path);
  saveLibraryRoots(roots);
  emit libraryRootsChanged();
  rescan();
}

void Backend::copySelectedPath() {
  QClipboard* clipboard = QGuiApplication::clipboard();
  clipboard->setText(m_selectedPath);
}

void Backend::openSelectedUsdview() {
  QStringList args;
  args << m_selectedPath;
  QProcess::startDetached("usdview", args);
}

void Backend::revealSelected() {
  // Open the containing directory rather than the file — file managers don't
  // have a useful default action for .usd files.
  QFileInfo fileInfo(m_selectedPath);
  QString url = "file:///" + fileInfo.dir().absolutePath();
  QDesktopServices::openUrl(QUrl(url));
}

void Backend::triggerAction(const QString& actionId) {
  if      (actionId == "copy")          copySelectedPath();
  else if (actionId == "reveal")        revealSelected();
  else if (actionId == "open")          openSelectedUsdview();
  else {
    for (const auto& dcc : m_detectedDccs) {
      if (actionId == "open_" + dcc.id) {
        openInDcc(dcc);
        return;
      }
    }
  }
}

void Backend::openInDcc(const DccLaunchConfig& dcc) {
  QStringList args;

  if (dcc.enabled) {
    args << dcc.customArgs;
  } else {
    if (dcc.id == "blender") {
      args << "--python-expr"
          << QString("import bpy; bpy.ops.object.select_all(action='SELECT'); bpy.ops.object.delete(use_global=False, confirm=False); bpy.ops.wm.usd_import(filepath='%1')")
              .arg(m_selectedPath);
    } else if (dcc.id == "houdini") {
      args << m_selectedPath;
    } else if (dcc.id == "maya") {
      args << "-file" << m_selectedPath;
    } else {
      args << m_selectedPath;
    }
  }

  QProcess::startDetached(dcc.executable, args);
}

void Backend::quitApp() {
  QCoreApplication::quit();
}

QString Backend::m_formatSize(quint64 size, SizeBase base) const {
  const int divisor = base == SizeBase::BINARY ? 1024 : 1000;
  if (size < divisor) return QString::number(size) + " B";

  static const QVector<QString> binSuffix = { "B", "KiB", "MiB", "GiB", "TiB" };
  static const QVector<QString> decSuffix = { "B", "KB", "MB", "GB", "TB" };

  const QVector<QString> suffix = base == SizeBase::BINARY ? binSuffix : decSuffix;
 
  int n = 0;
  double f_size = static_cast<double>(size);

  while (f_size > divisor && n < suffix.size() - 2) {
    f_size /= divisor;
    n++;
  }

  return QString("%1 %2").arg(f_size, 0, 'f', 2).arg(suffix[n]);
}

QVariantList Backend::variantSets() const {
  // QJsonObject can't be iterated directly in QML, so we convert to a list of
  // maps and inject the set name as a "name" key on each entry.
  QVariantList result;
  const QVariantMap map = m_details.variantSets.toVariantMap();
  for (auto it = map.begin(); it != map.end(); it++) {
    QVariantMap entry = it.value().toMap();
    entry["name"] = it.key();
    result.append(entry);
  }
  return result;
}

QVariantList Backend::actions() const {
  if (m_selectedPath.isEmpty()) return {};

  QVariantList result = {
    QVariantMap{
      {"id",        "copy"},
      {"label",     "Copy"},
      {"shortcut",  "Ctrl+C"},
      {"group",     "left"},
  },
    QVariantMap{
      {"id",        "reveal"},
      {"label",     "Reveal"},
      {"shortcut",  ""},
      {"group",     "left"},
    },
    QVariantMap{
      {"id",        "open"},
      {"label",     "Open"},
      {"shortcut",  ""},
      {"group",     "split_primary"},
    },
  };

  for (const auto& dcc : m_detectedDccs) {
    result.append(QVariantMap{
      {"id",    "open_" + dcc.id},
      {"label", "Open in " + dcc.label},
      {"group", "split_secondary"},
    });
  }

  return result;
}

QVector<DccLaunchConfig> Backend::detectDccs() {
  // Entries are { id, display label, executable name }
  const QVector<DccLaunchConfig> candidates = {
  { "usdview",  "usdview", "usdview" },
  { "blender",  "Blender", "blender" },
  { "houdini",  "Houdini", "houdini" },
  { "maya",     "Maya",    "maya"    },
  { "katana",   "Katana",  "katana"  }
  };

  QVector<DccLaunchConfig> found;
  for (const auto& app : candidates) {
    if (!QStandardPaths::findExecutable(app.executable).isEmpty())
      found.append(app);
  }
  return found;
}
