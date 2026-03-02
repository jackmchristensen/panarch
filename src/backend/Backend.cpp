#include <QSettings>
#include <QClipboard>
#include <QGuiApplication>
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
