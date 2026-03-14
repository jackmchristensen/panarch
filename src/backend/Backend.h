// Backend
//
// Responsibility:
//   Central application controller. Owns the asset data models, manages
//   library state, and exposes the full QML-facing API via Q_PROPERTY and
//   Q_INVOKABLE.
//
// Current scope:
//   - Coordinate async library scanning and thumbnail generation
//   - Expose selected-asset state and on-demand detail loading to QML
//   - Persist library roots and user settings via QSettings
//   - Launch external tools (usdview, Blender) for the selected asset
//
// Future plans:
//   - Settings dialog backed by the Settings struct
//   - Cancel/debounce in-flight detail loads when selection changes rapidly
//
// Non-goals:
//   - USD parsing (delegated to AssetIndex)
//   - Thumbnail rendering (delegated to thumbnail_generator subprocess)
//   - View layout or styling (handled entirely in QML)
//
// Rationale:
//   Qt's recommended pattern for QML/C++ integration is a "backend" object
//   set as a context property on the QML engine. All mutable state the UI
//   needs lives here as Q_PROPERTYs with NOTIFY signals; QML bindings then
//   update reactively without the backend needing to know anything about the
//   view hierarchy.
//
// Extension points (not yet implemented):
//   - onAssetSelected(path) — plugins can react and return extra metadata
//   - getActions(path) — plugins can contribute buttons to the action bar
//   - contributeInfoRows(path) — plugins can add rows to the info panel
//
// Plugin API will be stabilized after the attribute inspector is complete.
// Until then, avoid hardcoding behavior in Backend that belongs in a plugin.

#pragma once
#include <QObject>
#include <QString>
#include <QQueue>
#include <qtmetamacros.h>
#include "backend/AssetListModel.h"
#include "backend/AssetFilterModel.h"
#include "shared/AssetTypes.h"

/// Whether file sizes are displayed in powers of 1024 (KiB, MiB) or 1000 (KB, MB).
enum class SizeBase { BINARY, DECIMAL };

/// User-configurable application settings. Extend here as new preferences are added.
struct Settings {
public:
  SizeBase sizeBase = SizeBase::BINARY;
};

struct TabDefinition {
  QString id;
  QString label;
  bool closeable = false;
  QString pluginId = "";
};

struct ActionDefinition {
  QString id;
  QString label;
  QString shortcut;
  QString group;
  QString pluginId = "";
};

struct DccLaunchConfig {
  QString id;
  QString label;
  QString executable;
  QString customArgs;
  bool enabled = true;
  QString pluginId = "";
};

class Backend : public QObject {
  Q_OBJECT

  // ── Asset list & filter ───────────────────────────────────────────────────

  // CONSTANT because the pointers never change — only the data inside does.
  Q_PROPERTY(AssetListModel*    assets         READ assets          CONSTANT)
  Q_PROPERTY(AssetFilterModel*  filteredAssets READ filteredAssets  CONSTANT)

  // ── Selected asset (cheap AssetRecord fields, ready immediately) ──────────
 
  Q_PROPERTY(QString      selectedPath        READ selectedPath         NOTIFY selectedChanged)
  Q_PROPERTY(QString      selectedName        READ selectedName         NOTIFY selectedChanged)
  Q_PROPERTY(QString      selectedExt         READ selectedExt          NOTIFY selectedChanged)
  Q_PROPERTY(QString      selectedMTime       READ selectedMTime        NOTIFY selectedChanged)
  Q_PROPERTY(QString      selectedSize        READ selectedSize         NOTIFY selectedChanged)
  Q_PROPERTY(QString      selectedDefaultPrim READ selectedDefaultPrim  NOTIFY selectedChanged)
  Q_PROPERTY(QString      selectedKind        READ selectedKind         NOTIFY selectedChanged)
  Q_PROPERTY(QString      selectedThumbnail   READ selectedThumbnail    NOTIFY selectedChanged)
  Q_PROPERTY(QStringList  libraryRoots        READ libraryRoots         NOTIFY libraryRootsChanged)

  // ── Selected asset details (populated async after selection) ─────────────
  // QML should gate on loadingDetails and wait for detailsChanged before
  // reading these properties.

  Q_PROPERTY(bool         loadingDetails      READ loadingDetails     NOTIFY loadingDetailsChanged)
  Q_PROPERTY(QString      upAxis              READ upAxis             NOTIFY detailsChanged)
  Q_PROPERTY(double       metersPerUnit       READ metersPerUnit      NOTIFY detailsChanged)
  Q_PROPERTY(double       framesPerSecond     READ framesPerSecond    NOTIFY detailsChanged)
  Q_PROPERTY(double       timeCodesPerSecond  READ timeCodesPerSecond NOTIFY detailsChanged)
  Q_PROPERTY(QStringList  sublayers           READ sublayers          NOTIFY detailsChanged)
  Q_PROPERTY(QStringList  payloads            READ payloads           NOTIFY detailsChanged)
  Q_PROPERTY(QStringList  references          READ references         NOTIFY detailsChanged)
  Q_PROPERTY(int          primCount           READ primCount          NOTIFY detailsChanged)

  /// Variant sets as a QVariantList for QML Repeater compatibility.
  /// Each element is a QVariantMap: { "name": string, "variants": [string], "selected": string }.
  Q_PROPERTY(QVariantList variantSets READ variantSets  NOTIFY detailsChanged)
  Q_PROPERTY(QVariantList actions     READ actions      NOTIFY selectedChanged)
  Q_PROPERTY(QVariantList tabs        READ tabs         NOTIFY tabsChanged)

public:
  explicit Backend(QObject* parent = nullptr);

  AssetListModel*   assets()          { return &m_assets; }
  AssetFilterModel* filteredAssets()  { return &m_filterModel; }

  QString     selectedPath()        const { return m_selectedPath; }
  QString     selectedName()        const { return m_selectedName; }
  QString     selectedExt()         const { return m_selectedExt; }
  QString     selectedMTime()       const { return m_selectedMTime.toString("yyyy-MM-dd hh:mm"); }
  QString     selectedSize()        const { return m_formatSize(m_selectedSize, m_settings.sizeBase); }
  QString     selectedDefaultPrim() const { return m_selectedDefaultPrim; }
  QString     selectedKind()        const { return m_selectedKind; }
  QString     selectedThumbnail()   const { return m_selectedThumbnail; }
  QStringList libraryRoots()        const { return m_libraryRoots; }

  bool          loadingDetails()      const { return m_loadingDetails; }
  QString       upAxis()              const { return m_details.upAxis; }
  double        metersPerUnit()       const { return m_details.metersPerUnit; }
  double        framesPerSecond()     const { return m_details.framesPerSecond; }
  double        timeCodesPerSecond()  const { return m_details.timeCodesPerSecond; }
  QStringList   sublayers()           const { return m_details.sublayers; }
  QStringList   payloads()            const { return m_details.payloads; }
  QStringList   references()          const { return m_details.references; }
  int           primCount()           const { return m_details.primCount; }

  QVariantList  variantSets() const;
  QVariantList  actions()     const;
  QVariantList  tabs()        const;

  Q_INVOKABLE void initialize();
  Q_INVOKABLE void addLibraryRoot(const QString& rootDir);
  Q_INVOKABLE void rescan();
  /// @p proxyRow is a row in the filtered model, not the source model.
  Q_INVOKABLE void selectIndex(int proxyRow);
  Q_INVOKABLE void selectFirst();
  Q_INVOKABLE void removeLibraryRoot(const QString& path);
  Q_INVOKABLE void copySelectedPath();
  Q_INVOKABLE void openSelectedUsdview();
  Q_INVOKABLE void revealSelected();
  Q_INVOKABLE void triggerAction(const QString& actionId);
  Q_INVOKABLE void quitApp();

  static QVector<DccLaunchConfig> detectDccs();

signals:
  void selectedChanged();
  void loadingDetailsChanged();
  void detailsChanged();
  void libraryRootsChanged();
  void tabsChanged();
  void openLibraryDialogRequested();
  void focusFilter();

private:
  AssetListModel    m_assets;
  AssetFilterModel  m_filterModel;
  AssetDetails      m_details;
  bool              m_loadingDetails = false;

  QString   m_selectedPath;
  QString   m_selectedName;
  QString   m_selectedExt;
  QDateTime m_selectedMTime;
  quint64   m_selectedSize = 0;
  QString   m_selectedDefaultPrim;
  QString   m_selectedKind;
  QString   m_selectedThumbnail;

  int m_activeThumbnailProcesses = 0;
  static constexpr int MaxThumbnailProcesses = 10;
  QQueue<std::pair<QString, std::pair<QString, QString>>> m_thumbnailQueue; // {assetId, (assetPath, cachePath)}
 
  // NOTE: Never populated — loadLibraryRoots() reads QSettings directly each
  // time, so this property always returns an empty list. Fix by syncing
  // m_libraryRoots in addLibraryRoot(), removeLibraryRoot(), and initialize().
  QStringList m_libraryRoots;

  Settings                  m_settings;
  QVector<DccLaunchConfig>  m_detectedDccs = detectDccs();
  QVector<TabDefinition>    m_tabs;
  QVector<ActionDefinition> m_actions;
  QString                   m_formatSize(quint64 size, SizeBase base) const;

  void                  saveLibraryRoots      (const QStringList& roots);
  QStringList           loadLibraryRoots      ();
  void                  saveUserSettings      ();
  void                  loadUserSettings      ();
  void                  openInDcc             (const DccLaunchConfig& dcc);
  void                  generateThumbnailAsync(const QString& assetPath, const QString& cachePath, const QString& assetId);
  void                  drainThumbnailQueue   ();
  void                  loadDetailsAsync      (const QString& assetPath);
  AssetDetails          parseInspectorOutput  (const QJsonObject& assetData);
  static AssetRecord    recordFromJson        (const QJsonObject& obj);
  static QStringList    jsonArrayToStringList (const QJsonArray& arr);

  void initTabs();
  void initActions();
};

