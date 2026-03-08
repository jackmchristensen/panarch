#pragma once
#include <QObject>
#include <QString>
#include <qtmetamacros.h>
#include "backend/AssetListModel.h"
#include "backend/AssetIndex.h"

enum class SizeBase { BINARY, DECIMAL };

struct Settings {
public:
  SizeBase sizeBase = SizeBase::BINARY;
};

class Backend : public QObject {
  Q_OBJECT
  Q_PROPERTY(AssetListModel* assets READ assets CONSTANT)
  Q_PROPERTY(QString selectedPath READ selectedPath NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedName READ selectedName NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedExt READ selectedExt NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedMTime READ selectedMTime NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedSize READ selectedSize NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedDefaultPrim READ selectedDefaultPrim NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedKind READ selectedKind NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedThumbnail READ selectedThumbnail NOTIFY selectedChanged)
  Q_PROPERTY(QStringList libraryRoots READ libraryRoots NOTIFY libraryRootsChanged)

public:
  explicit Backend(QObject* parent = nullptr);

  AssetListModel* assets() { return &m_assets; }

  QString selectedPath() const { return m_selectedPath; }
  QString selectedName() const { return m_selectedName; }
  QString selectedExt() const { return m_selectedExt; }
  QString selectedMTime() const { return m_selectedMTime.toString("yyyy-MM-dd hh:mm"); }
  QString selectedSize() const { return m_formatSize(m_selectedSize, m_settings.sizeBase); }
  QString selectedDefaultPrim() const { return m_selectedDefaultPrim; }
  QString selectedKind() const { return m_selectedKind; }
  QString selectedThumbnail() const { return m_selectedThumbnail; }
  QStringList libraryRoots() const { return m_libraryRoots; }

  Q_INVOKABLE void initialize();
  Q_INVOKABLE void addLibraryRoot(const QString& rootDir);
  Q_INVOKABLE void rescan();
  Q_INVOKABLE void selectIndex(int index);
  Q_INVOKABLE void removeLibraryRoot(const QString& path);
  Q_INVOKABLE void copySelectedPath();
  Q_INVOKABLE void openSelectedUsdview();
  Q_INVOKABLE void openSelectedBlender();
  Q_INVOKABLE void revealSelected();
  Q_INVOKABLE void quitApp();

  void saveLibraryRoots(const QStringList& roots);
  QStringList loadLibraryRoots();
  void saveUserSettings();
  void loadUserSettings();

signals:
  void selectedChanged();
  void libraryRootsChanged();
  void openLibraryDialogRequested();

private:
  AssetListModel m_assets;
  AssetDetails m_details;
  qint32 m_selectedIndex = -1;
  QString m_selectedPath;
  QString m_selectedName;
  QString m_selectedExt;
  QDateTime m_selectedMTime;
  quint64 m_selectedSize = 0;
  QString m_selectedDefaultPrim;
  QString m_selectedKind;
  QString m_selectedThumbnail;
  QStringList m_libraryRoots;

  Settings m_settings;
  QString m_formatSize(quint64 size, SizeBase base) const;
};

