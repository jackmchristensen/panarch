// AssetListModel
//
// Responsibility:
//   Provides a Qt/QML-compatible view of indexed assets.
//   Acts as a read-only adapter between backend asset data and UI
//   views like ListView/GridView.
//
// Current scope:
//   - Wrap QVector<AssetRecord> in a QAbstractListModel interface
//   - Expose asset metadata as roles
//   - Notify UI when asset list changes
//
// Future plans:
//   - Filtering/search support
//   - Sorting/grouping options
//   - Thumbnail caching hooks
//   - Possibly proxy models for advanced filtering
//
// Non-goals:
//   - Business logic or filesystem scanning
//   - Direct UI command handling
//   - USD parsing/validation
//
// Rationale:
//   QML views require a QAbstractListModel derived class.
//   This model separates UI presentaion from asset indexing and
//   backend logic.

#pragma once
#include <QAbstractListModel>
#include "backend/AssetIndex.h"

class AssetListModel : public QAbstractListModel {
  Q_OBJECT

public:
  enum Roles {
    NameRole = Qt::UserRole + 1,
    PathRole,
    TypeRole,
    ThumbnailRole,
    KindRole,
    HasVariantsRole,
    HasPayloadsRole,
    HasReferencesRole
  };

  explicit AssetListModel(QObject* parent = nullptr);

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  void setAssets(QVector<AssetRecord> assets);
  const AssetRecord* at(int row) const;

private:
  QVector<AssetRecord> m_assets;
};
