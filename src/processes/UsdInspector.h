#pragma once

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/common.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/variantSets.h>
#include <pxr/usd/pcp/layerStack.h>
#include <pxr/usd/usdGeom/metrics.h>

#include <QString>
#include <QVector>
#include <QJsonObject>

namespace UsdInspector {

QJsonObject primToJson(const pxr::UsdPrim& prim);
QJsonObject collectCompositionArcs(pxr::UsdStageRefPtr stage);
QJsonObject getVariantSets(pxr::UsdPrim prim);
QJsonObject inspectAsset(const QString& assetPath);

}
