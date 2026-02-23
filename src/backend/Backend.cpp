#include <QSettings>
#include "backend/Backend.h"
#include "backend/AssetIndex.h"

Backend::Backend(QObject* parent) : QObject(parent) {}

void Backend::initialize() {
  rescan();
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

  m_selectedPath = rec->path;
  m_selectedName = rec->name;
  m_selectedType = rec->type;
  m_selectedThumbnail = rec->thumbnail;
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
