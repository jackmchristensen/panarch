#include "backend/Backend.h"
#include "backend/AssetIndex.h"

Backend::Backend(QObject* parent) : QObject(parent) {}

void Backend::loadLibrary(const QString& rootDir) {
  m_assets.setAssets(AssetIndex::scan(rootDir));
}

void Backend::selectIndex(int index) {
  const AssetRecord* rec = m_assets.at(index);
  if (!rec) return;

  m_selectedPath = rec->path;
  m_selectedName = rec->name;
  emit selectedChanged();
}
