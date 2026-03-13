#include "processes/ScanAssets.h"

#include <QCoreApplication>
#include <QJsonDocument>

int main(int argc, char* argv[]) {
  if (argc < 2) return 1;

  QCoreApplication::setOrganizationName("Panarch");
  QCoreApplication::setApplicationName("Panarch");

  QString rootDir = argv[1];
  QJsonObject result = scanAssets(rootDir);

  QTextStream(stdout) << QJsonDocument(result).toJson(QJsonDocument::Compact);

  return 0;
}
