// ThumbnailGenerator
//
// Responsibility:
//   Render a USD asset to an offscreen framebuffer and save a JPEG thumbnail.
//
// Rationale:
//   This runs as a standalone subprocess (thumbnail_generator binary) rather
//   than on a thread inside the main application. UsdImagingGLEngine requires
//   its own OpenGL context, and initialising Hydra alongside the main Qt UI
//   caused conflicts. Running out-of-process also means a renderer crash or
//   hang doesn't affect the main application.
//
// Usage:
//   thumbnail_generator <assetPath> <outputPath>

#pragma once
#include <QString>

class ThumbnailGenerator {

public:
  /// Render @p assetPath and write a JPEG thumbnail to @p outputPath.
  /// Blocking. Intended to be the only call made from ThumbnailGeneratorMain.
  static void GenerateThumbnail(const QString& assetPath, const QString& outputPath);

}; // ThumbnailGenerator
