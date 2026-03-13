#include "processes/UsdInspector.h"

#include <QJsonDocument>

int main(int argc, char* argv[]) {
  if (argc < 2) return 1;

  QString assetPath = argv[1];
  QJsonObject result = UsdInspector::inspectAsset(assetPath);

  QTextStream(stdout) << QJsonDocument(result).toJson(QJsonDocument::Compact);

  return 0;
}
