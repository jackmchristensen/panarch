#include <QGuiApplication>
#include "render/ThumbnailGenerator.h"

int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);

  if (argc < 3) return 1;

  QString assetPath = argv[1];
  QString outputPath = argv[2];

  ThumbnailGenerator::GenerateThumbnail(assetPath, outputPath);

  return 0;
}
