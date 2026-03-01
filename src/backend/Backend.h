#pragma once
#include <QObject>
#include <QString>
#include <qtmetamacros.h>
#include "backend/AssetListModel.h"

class Backend : public QObject {
  Q_OBJECT
  Q_PROPERTY(AssetListModel* assets READ assets CONSTANT)
  Q_PROPERTY(QString selectedPath READ selectedPath NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedName READ selectedName NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedType READ selectedType NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedMTime READ selectedMTime NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedThumbnail READ selectedThumbnail NOTIFY selectedChanged)
  Q_PROPERTY(QStringList libraryRoots READ libraryRoots NOTIFY libraryRootsChanged)

public:
  explicit Backend(QObject* parent = nullptr);

  AssetListModel* assets() { return &m_assets; }

  QString selectedPath() const { return m_selectedPath; }
  QString selectedName() const { return m_selectedName; }
  QString selectedType() const { return m_selectedType; }
  QString selectedMTime() const { return m_selectedMTime.toString("yyyy-MM-dd hh:mm"); }
  QString selectedThumbnail() const { return m_selectedThumbnail; }
  QStringList libraryRoots() const { return m_libraryRoots; }

  Q_INVOKABLE void initialize();
  Q_INVOKABLE void addLibraryRoot(const QString& rootDir);
  Q_INVOKABLE void rescan();
  Q_INVOKABLE void loadLibrary(const QString& rootDir);
  Q_INVOKABLE void selectIndex(int index);
  Q_INVOKABLE void removeLibraryRoot(const QString& path);

  void saveLibraryRoots(const QStringList& roots);
  QStringList loadLibraryRoots();

signals:
  void selectedChanged();
  void libraryRootsChanged();

private:
  AssetListModel m_assets;
  QString m_selectedPath;
  QString m_selectedName;
  QString m_selectedType;
  QDateTime m_selectedMTime;
  QString m_selectedThumbnail;
  QStringList m_libraryRoots;
};
