// AssetIndex
//
// Responsibility:
//   Index and monitor asset libraries
//
// Current scope:
//   - Scan directories for USD files
//   - Provide lightweight metadata for UI display
//
// Future plans:
//   - File watching for live updates
//   - Thumbnail generation pipeline
//   - Asset validation checks
//   - Caching/database integration
//
// Non-goals:
//   - Rendering or preview generation
//   - Heavy USD scene parsing
//
// Rationale:
//   Start as a simple scan utility but allow for future
//   support for stateful indexing, async jobs, and caching.

#pragma once
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QJsonObject>

struct AssetRecord {
  QString id;
  QString entryPath;
  QString displayName;
  QString fileExt;
  QString thumbnailPath;
  QDateTime mtime;
  quint64 fileSize = 0;

  QString kind;
  QString defaultPrimPath;
  bool hasVariants = false;
  bool hasPayloads = false;
  bool hasReferences = false;
};

struct AssetDetails {
  // Stage level metadata
  QString upAxis;
  double metersPerUnit = 0.0;
  double framesPerSecond = 0.0;
  double timeCodesPerSecond = 0.0;

  // Composition arcs
  QStringList sublayers;
  QStringList payloads;
  QStringList references;

  // Prim data
  int primCount = 0;
  QJsonObject variantSets;

  // Asset metadata from USD
  QJsonObject assetInfo;
  QJsonObject userMeta;
};

class AssetIndex {
public:
  static QVector<AssetRecord> scan(const QString& rootDir);
};
