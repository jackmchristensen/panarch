#pragma once
#include <QString>
#include <QVector>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>

struct AssetRecord {
  QString id;
  QString entryPath;
  QString displayName;
  QString fileExt;
  QString thumbnailPath = "";
  QDateTime mtime;
  quint64 fileSize = 0;

  QString kind;
  QString defaultPrimPath;
  bool hasVariants = false;
  bool hasPayloads = false;
  bool hasExternalDeps = false;
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
  QJsonArray primTree;

  // Asset metadata from USD
  QJsonObject assetInfo;
  QJsonObject userMeta;
};
