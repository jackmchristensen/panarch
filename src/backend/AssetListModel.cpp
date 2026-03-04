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
    case PathRole: return a.entryPath;
    case TypeRole: return a.fileExt;
    case ThumbnailRole: return a.thumbnailPath;
    case HasVariantsRole: return a.hasVariants;
    case HasPayloadsRole: return a.hasPayloads;
    case HasReferencesRole: return a.hasReferences;
    default: return {};
  };
}

QHash<int, QByteArray> AssetListModel::roleNames() const {
  return {
    {NameRole, "name"},
    {PathRole, "path"},
    {TypeRole, "type"},
    {ThumbnailRole, "thumbnail"},
    {HasVariantsRole, "hasVariants"},
    {HasPayloadsRole, "hasPayloads"},
    {HasReferencesRole, "hasReferences"}
  };
}

void AssetListModel::setAssets(QVector<AssetRecord> assets) {
  beginResetModel();
  m_assets = std::move(assets);
  endResetModel();
}

const AssetRecord* AssetListModel::at(int row) const {
  if (row < 0 || row >= m_assets.size()) return nullptr;
  return &m_assets[row];
}

