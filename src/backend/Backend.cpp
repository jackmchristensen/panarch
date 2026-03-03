#include <QSettings>
#include <QClipboard>
#include <QGuiApplication>
#include <qcontainerfwd.h>
#include <qfloat16.h>
#include <qlogging.h>
#include "backend/Backend.h"
#include "backend/AssetIndex.h"

Backend::Backend(QObject* parent) : QObject(parent) {}

void Backend::initialize() {
  rescan();
  loadUserSettings();
}

void Backend::rescan() {
  const QStringList roots = loadLibraryRoots();

  QVector<AssetRecord> all;
  all.reserve(1024);

  for (const QString& root : roots) {
    auto scanned = AssetIndex::scan(root);
    all += scanned;
  }

  m_assets.setAssets(std::move(all));
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

void Backend::loadLibrary(const QString& rootDir) {
  for (const QString& root : loadLibraryRoots()) {
    m_assets.setAssets(AssetIndex::scan(root));
  }
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
