#include <QSettings>
#include <QClipboard>
#include <QGuiApplication>
#include <QProcess>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <qjsondocument.h>

#include "backend/Backend.h"
#include "backend/AssetIndex.h"

Backend::Backend(QObject* parent) : QObject(parent) {}

void Backend::initialize() {
  rescan();
  loadUserSettings();
}

void Backend::rescan() {
  auto* watcher = new QFutureWatcher<QVector<AssetRecord>>(this);

  connect(watcher, &QFutureWatcher<QVector<AssetRecord>>::finished, this, [this, watcher]() {
    QVector<AssetRecord> result = watcher->result();
    m_assets.setAssets(result);
    watcher->deleteLater();
  });

  auto future = QtConcurrent::run([this]() {
    const QStringList roots = loadLibraryRoots();
    QVector<AssetRecord> all;
    all.reserve(1024);

    for (const QString& root: roots) {
      auto scanned = AssetIndex::scan(root);
      all += scanned;
    }
    return all;
  });

  watcher->setFuture(future);
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

void Backend::selectIndex(int index) {
  const AssetRecord* rec = m_assets.at(index);
  if (!rec) return;

  m_selectedIndex = index;

  m_selectedPath = rec->entryPath;
  m_selectedName = rec->displayName;
  m_selectedExt = rec->fileExt;
  m_selectedThumbnail = rec->thumbnailPath;
  m_selectedSize = rec->fileSize;
  m_selectedDefaultPrim = rec->defaultPrimPath;
  m_selectedKind = rec->kind;
  m_selectedMTime = rec->mtime;

  auto* watcher = new QFutureWatcher<AssetDetails>(this);

  connect(watcher, &QFutureWatcher<AssetDetails>::finished, this, [this, watcher]() {
    AssetDetails result = watcher->result();
    m_details = result;
    watcher->deleteLater();
  });

  auto future = QtConcurrent::run([this]() {
    return AssetIndex::getAssetDetails(m_selectedPath);
  });

  watcher->setFuture(future);

  emit selectedChanged();
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
  const AssetRecord* rec = m_assets.at(m_selectedIndex);
  if (!rec) return;
 
  QClipboard* clipboard = QGuiApplication::clipboard();
  clipboard->setText(rec->entryPath);
}

void Backend::openSelectedUsdview() {
  QStringList args;
  args << m_assets.at(m_selectedIndex)->entryPath;
  QProcess::startDetached("usdview", args);
}

void Backend::openSelectedBlender() {
  QStringList args;
  args << "--python-expr";
  args << QString("import bpy; bpy.ops.object.select_all(action='SELECT'); bpy.ops.object.delete(use_global=False, confirm=False); bpy.ops.wm.usd_import(filepath='%1')").arg(m_assets.at(m_selectedIndex)->entryPath);
  QProcess::startDetached("blender", args);
}

void Backend::revealSelected() {
  QFileInfo fileInfo(m_assets.at(m_selectedIndex)->entryPath);
  QString url = "file:///" + fileInfo.dir().absolutePath();
  QDesktopServices::openUrl(QUrl(url));
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
