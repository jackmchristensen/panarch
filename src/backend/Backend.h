#pragma once
#include <QObject>
#include <QString>
#include "backend/AssetListModel.h"

class Backend : public QObject {
  Q_OBJECT
  Q_PROPERTY(AssetListModel* assets READ assets CONSTANT)
  Q_PROPERTY(QString selectedPath READ selectedPath NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedName READ selectedName NOTIFY selectedChanged)

public:
  explicit Backend(QObject* parent = nullptr);

  AssetListModel* assets() { return &m_assets; }

  QString selectedPath() const { return m_selectedPath; }
  QString selectedName() const { return m_selectedName; }

  Q_INVOKABLE void loadLibrary(const QString& rootDir);
  Q_INVOKABLE void selectIndex(int index);

signals:
  void selectedChanged();

private:
  AssetListModel m_assets;
  QString m_selectedPath;
  QString m_selectedName;
};
