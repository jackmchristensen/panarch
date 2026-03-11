#include <backend/AssetFilterModel.h>
#include <backend/AssetListModel.h>

AssetFilterModel::AssetFilterModel(QObject* parent) : QSortFilterProxyModel() {}

void AssetFilterModel::setNameFilter(const QString& text) {
  if (m_nameFilter == text) return;
  beginFilterChange();
  m_nameFilter = text;
  endFilterChange(QSortFilterProxyModel::Direction::Rows);
  emit nameFilterChanged();
}

void AssetFilterModel::setKindFilter(const QString& kind) {
  if (m_kindFilter == kind) return;
  beginFilterChange();
  m_kindFilter = kind;
  endFilterChange(QSortFilterProxyModel::Direction::Rows);
  emit kindFilterChanged();
}

bool AssetFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  const auto* model = qobject_cast<const AssetListModel*>(sourceModel());
  if (!model) return true;

  const AssetRecord* asset = model->at(sourceRow);
  if (!asset) return true;

  if (!m_nameFilter.isEmpty() && !asset->displayName.contains(m_nameFilter, Qt::CaseInsensitive))
    return false;

  if (!m_kindFilter.isEmpty() && asset->kind != m_kindFilter)
    return false;

  return true;
}

void AssetFilterModel::setSortMode(SortMode mode) {
  if (m_sortMode == mode) return;
  m_sortMode = mode;
  invalidate();
  emit sortModeChanged();
}

bool AssetFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
  const auto* model = qobject_cast<const AssetListModel*>(sourceModel());
  if (!model) return false;

  const AssetRecord* a = model->at(left.row());
  const AssetRecord* b = model->at(right.row());
  if (!a || !b) return false;

  switch (m_sortMode) {
    case SortNameAsc: return a->displayName.toLower() < b->displayName.toLower();
    case SortNameDesc: return a->displayName.toLower() > b->displayName.toLower();
    case SortNewestFirst: return a->mtime > b->mtime;
    case SortOldestFirst: return a->mtime < b->mtime;
  }

  return false;
}
