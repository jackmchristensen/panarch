#pragma once
#include <QSortFilterProxyModel>

class AssetFilterModel : public QSortFilterProxyModel {
  Q_OBJECT

  Q_PROPERTY(QString nameFilter READ nameFilter WRITE setNameFilter NOTIFY nameFilterChanged)
  Q_PROPERTY(QString kindFilter READ kindFilter WRITE setKindFilter NOTIFY kindFilterChanged)

public:
  explicit AssetFilterModel(QObject* parent = nullptr);

  QString nameFilter() const { return m_nameFilter; };
  QString kindFilter() const { return m_kindFilter; };

  void setNameFilter(const QString& text);
  void setKindFilter(const QString& kind);

signals:
  void nameFilterChanged();
  void kindFilterChanged();

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
  QString m_nameFilter;
  QString m_kindFilter;
};
