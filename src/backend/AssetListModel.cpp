#include <QStandardPaths>
#include "backend/AssetListModel.h"

AssetListModel::AssetListModel(QObject* parent) : QAbstractListModel(parent) {}

int AssetListModel::rowCount(const QModelIndex&) const {
  return m_assets.size();
}

QVariant AssetListModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid() || index.row() < 0 || index.row() >= m_assets.size())
    return {};

  const auto& a = m_assets[index.row()];
  switch (role) {
    case NameRole: return a.displayName;
    case IdRole: return a.id;
    case PathRole: return a.entryPath;
    case TypeRole: return a.fileExt;
    case KindRole: return a.kind;
    case ThumbnailRole: return a.thumbnailPath;
    case HasVariantsRole: return a.hasVariants;
    case HasPayloadsRole: return a.hasPayloads;
    case HasExternalDepsRole: return a.hasExternalDeps;
    default: return {};
  };
}

QHash<int, QByteArray> AssetListModel::roleNames() const {
  return {
    {NameRole, "name"},
    {IdRole, "id"},
    {PathRole, "path"},
    {TypeRole, "type"},
    {KindRole, "kind"},
    {ThumbnailRole, "thumbnail"},
    {HasVariantsRole, "hasVariants"},
    {HasPayloadsRole, "hasPayloads"},
    {HasExternalDepsRole, "hasExternalDeps"}
  };
}

void AssetListModel::setAssets(QVector<AssetRecord> assets) {
  beginResetModel();
  m_assets = std::move(assets);
  m_idToRow.clear();
  for (int i = 0; i < m_assets.size(); i++) {
    m_idToRow[m_assets[i].id] = i;
  }
  endResetModel();
}

const AssetRecord* AssetListModel::at(int row) const {
  if (row < 0 || row >= m_assets.size()) return nullptr;
  return &m_assets[row];
}

void AssetListModel::onThumbnailReady(const QString& assetId, const QString& thumbnailPath) {
  auto it = m_idToRow.find(assetId);
  if (it == m_idToRow.end()) return;

  int row = it.value();
  m_assets[row].thumbnailPath = thumbnailPath;
  QModelIndex idx = index(row);
  emit dataChanged(idx, idx, { ThumbnailRole });
}
