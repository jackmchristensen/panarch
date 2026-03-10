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
