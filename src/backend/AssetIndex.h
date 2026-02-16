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

struct AssetRecord {
  QString path;
  QString name;
  QString type;
  QString thumbnail;
};

class AssetIndex {
public:
  static QVector<AssetRecord> scan(const QString& rootDir);
};
