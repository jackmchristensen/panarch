#include <QGuiApplication>
#include "processes/ThumbnailGenerator.h"

// Entry point for the thumbnail_generator subprocess.
// Spawned by Backend::generateThumbnailAsync with two arguments:
//   argv[1]  absolute path to the USD asset
//   argv[2]  absolute path for the output JPEG
// Exits with 0 on success, 1 on bad arguments.
// Any render failure (bad asset, GL error) currently exits 0 with a
// partial or blank image — Backend treats any exit 0 as success.
// TODO: Propagate render errors as a non-zero exit code.
int main(int argc, char* argv[]) {
  QGuiApplication app(argc, argv);

  if (argc < 3) return 1;

  QString assetPath = argv[1];
  QString outputPath = argv[2];

  ThumbnailGenerator::GenerateThumbnail(assetPath, outputPath);

  return 0;
}
