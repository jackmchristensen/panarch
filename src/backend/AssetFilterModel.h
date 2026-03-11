#pragma once
#include <QSortFilterProxyModel>

class AssetFilterModel : public QSortFilterProxyModel {
  Q_OBJECT
 
  enum SortMode {
    SortNameAsc,
    SortNameDesc,
    SortNewestFirst,
    SortOldestFirst
  };
  Q_ENUM(SortMode)

  Q_PROPERTY(QString nameFilter READ nameFilter WRITE setNameFilter NOTIFY nameFilterChanged)
  Q_PROPERTY(QString kindFilter READ kindFilter WRITE setKindFilter NOTIFY kindFilterChanged)
  Q_PROPERTY(SortMode sortMode READ sortMode WRITE setSortMode);

public:
  explicit AssetFilterModel(QObject* parent = nullptr);

  QString nameFilter() const { return m_nameFilter; };
  QString kindFilter() const { return m_kindFilter; };
  SortMode sortMode() const { return m_sortMode; };

  void setNameFilter(const QString& text);
  void setKindFilter(const QString& kind);

  void setSortMode(SortMode mode);

signals:
  void nameFilterChanged();
  void kindFilterChanged();
  void sortModeChanged();

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;
  bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
  QString m_nameFilter;
  QString m_kindFilter;

  SortMode m_sortMode = SortNewestFirst;
};
