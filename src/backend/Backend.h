#pragma once
#include <QObject>
#include <QString>
#include <qtmetamacros.h>
#include "backend/AssetListModel.h"
#include "backend/AssetFilterModel.h"
#include "backend/AssetIndex.h"

enum class SizeBase { BINARY, DECIMAL };

struct Settings {
public:
  SizeBase sizeBase = SizeBase::BINARY;
};

class Backend : public QObject {
  Q_OBJECT

  // AssetRecords
  Q_PROPERTY(AssetListModel* assets READ assets CONSTANT)
  Q_PROPERTY(AssetFilterModel* filteredAssets READ filteredAssets CONSTANT)
  Q_PROPERTY(QString selectedPath READ selectedPath NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedName READ selectedName NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedExt READ selectedExt NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedMTime READ selectedMTime NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedSize READ selectedSize NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedDefaultPrim READ selectedDefaultPrim NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedKind READ selectedKind NOTIFY selectedChanged)
  Q_PROPERTY(QString selectedThumbnail READ selectedThumbnail NOTIFY selectedChanged)
  Q_PROPERTY(QStringList libraryRoots READ libraryRoots NOTIFY libraryRootsChanged)

  // AssetDetails
  Q_PROPERTY(bool loadingDetails READ loadingDetails NOTIFY loadingDetailsChanged)
  Q_PROPERTY(QString upAxis READ upAxis NOTIFY detailsChanged)
  Q_PROPERTY(double metersPerUnit READ metersPerUnit NOTIFY detailsChanged)
  Q_PROPERTY(double framesPerSecond READ framesPerSecond NOTIFY detailsChanged)
  Q_PROPERTY(double timeCodesPerSecond READ timeCodesPerSecond NOTIFY detailsChanged)
  Q_PROPERTY(QStringList sublayers READ sublayers NOTIFY detailsChanged)
  Q_PROPERTY(QStringList payloads READ payloads NOTIFY detailsChanged)
  Q_PROPERTY(QStringList references READ references NOTIFY detailsChanged)
  Q_PROPERTY(int primCount READ primCount NOTIFY detailsChanged)
  Q_PROPERTY(QVariantList variantSets READ variantSets NOTIFY detailsChanged)

public:
  explicit Backend(QObject* parent = nullptr);

  AssetListModel* assets() { return &m_assets; }
  AssetFilterModel* filteredAssets() { return &m_filterModel; }

  QString selectedPath() const { return m_selectedPath; }
  QString selectedName() const { return m_selectedName; }
  QString selectedExt() const { return m_selectedExt; }
  QString selectedMTime() const { return m_selectedMTime.toString("yyyy-MM-dd hh:mm"); }
  QString selectedSize() const { return m_formatSize(m_selectedSize, m_settings.sizeBase); }
  QString selectedDefaultPrim() const { return m_selectedDefaultPrim; }
  QString selectedKind() const { return m_selectedKind; }
  QString selectedThumbnail() const { return m_selectedThumbnail; }
  QStringList libraryRoots() const { return m_libraryRoots; }

  bool loadingDetails() const { return m_loadingDetails; }
  QString upAxis() const { return m_details.upAxis; }
  double metersPerUnit() const { return m_details.metersPerUnit; }
  double framesPerSecond() const { return m_details.framesPerSecond; }
  double timeCodesPerSecond() const { return m_details.timeCodesPerSecond; }
  QStringList sublayers() const { return m_details.sublayers; }
  QStringList payloads() const { return m_details.payloads; }
  QStringList references() const { return m_details.references; }
  int primCount() const { return m_details.primCount; }
  QVariantList variantSets() const;

  Q_INVOKABLE void initialize();
  Q_INVOKABLE void addLibraryRoot(const QString& rootDir);
  Q_INVOKABLE void rescan();
  Q_INVOKABLE void selectIndex(int proxyRow);
  Q_INVOKABLE void selectFirst();
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
  void generateThumbnailAsync(const QString& assetPath, const QString& cachePath, const QString& assetId);

signals:
  void selectedChanged();
  void loadingDetailsChanged();
  void detailsChanged();
  void libraryRootsChanged();
  void openLibraryDialogRequested();
  void focusFilter();

private:
  AssetListModel m_assets;
  AssetFilterModel m_filterModel;
  AssetDetails m_details;
  bool m_loadingDetails = false;
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

