#pragma once
#include "pxr/usd/sdf/primSpec.h"
#include "pxr/usd/sdf/reference.h"
#include "pxr/usd/sdf/payload.h"
#include "pxr/usd/sdf/layer.h"
#include "pxr/usd/sdf/layerUtils.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QStandardPaths>

#include <shared/AssetTypes.h>

QJsonObject scanAssets(const QString& rootDir);
