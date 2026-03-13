// AssetListModel
//
// Responsibility:
//   Read-only QAbstractListModel adapter over a flat QVector<AssetRecord>,
//   exposing asset metadata as named roles to QML views.
//
// Current scope:
//   - Wrap QVector<AssetRecord> in a QAbstractListModel interface
//   - Expose asset metadata as roles
//   - Accept thumbnail updates without a full model reset
//
// Future plans:
//   - Sorting/grouping options
//
// Non-goals:
//   - Filtering or sorting (handled by AssetFilterModel)
//   - Business logic or filesystem scanning (handled by AssetIndex)
//   - USD parsing/validation
//
// Rationale:
//   QML views require a QAbstractListModel-derived class. Keeping this model
//   as a thin, unfiltered adapter means AssetFilterModel always has access to
//   the full dataset, and Backend can map proxy indices back to source records
//   without worrying about filter state.

#pragma once
#include <QAbstractListModel>
#include "backend/AssetIndex.h"

class AssetListModel : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    NameRole = Qt::UserRole + 1,
    IdRole,
    PathRole,
    TypeRole,
    ThumbnailRole,
    KindRole,
    HasVariantsRole,
    HasPayloadsRole,
    HasExternalDepsRole
  };

  explicit AssetListModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  /// Replace the entire asset list. Triggers a full model reset.
  void setAssets(QVector<AssetRecord> assets);

  /// Direct access to a source-model record by row. Used by AssetFilterModel
  /// and Backend to read AssetRecord fields without going through QVariant.
  /// Returns nullptr if row is out of bounds.
  const AssetRecord* at(int row) const;

  /// Called by Backend when a thumbnail subprocess finishes. Updates the
  /// single record in-place and emits dataChanged for ThumbnailRole only,
  /// avoiding a full model reset.
  void onThumbnailReady(const QString& assetId, const QString& thumbnailPath);

private:
  QVector<AssetRecord> m_assets;
  QHash<QString, int>  m_idToRow; // assetId -> source row, for O(1) thumbnail updates
};
