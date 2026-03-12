#include <pxr/base/gf/vec3d.h>
#include <pxr/imaging/glf/simpleLight.h>
#include <pxr/imaging/glf/simpleMaterial.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/base/gf/camera.h>
#include <pxr/usdImaging/usdImagingGL/renderParams.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/imaging/hdx/tokens.h>

#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QImage>

#include <qoffscreensurface.h>
#include <tuple>
#include <limits>

#include "processes/ThumbnailGenerator.h"

namespace {

// Compute a world-space bounding box by iterating all UsdGeomMesh prims and
// transforming their points into world space.
//
// USD provides UsdGeomBBoxCache for this, but it crashes on destruction with
// a SIGSEGV inside ~TfHashMap when using the system USD package. The backtrace
// shows the destructor walking a hashtable node at address 0x23, which is an
// invalid pointer consistent with an ABI mismatch between USD's internal memory
// layout and what this translation unit expects — likely a difference in
// compiler flags or stdlib between our build and the packaged USD library.
// This manual mesh-point traversal is sufficient for camera placement and
// sidesteps the issue entirely.
//
// Reproduces with pxr 0.25.11 (system package on Arch/GCC 15.2).
// If USD is built from source with matching flags this may not be needed.
//
// Returns { bboxMin, bboxMax }. If the stage has no mesh geometry both
// vectors will contain infinity/-infinity — callers should handle that case
// if it matters (currently the camera placement degrades gracefully to NaN
// distances, which produces a blank render rather than a crash).
std::tuple<pxr::GfVec3d, pxr::GfVec3d> getBbox(pxr::UsdStageRefPtr stage) {
  double inf = std::numeric_limits<double>::infinity();
  pxr::GfVec3d bboxMin( inf, inf, inf);
  pxr::GfVec3d bboxMax(-inf, -inf, -inf);

  for (const pxr::UsdPrim& prim : stage->Traverse()) {
    pxr::UsdGeomMesh mesh(prim);
    if (!mesh) continue;

    pxr::VtArray<pxr::GfVec3f> points;
    mesh.GetPointsAttr().Get(&points, pxr::UsdTimeCode::Default());

    pxr::GfMatrix4d xform = pxr::UsdGeomXformable(prim).ComputeLocalToWorldTransform(pxr::UsdTimeCode::Default());

    for (const pxr::GfVec3f& pt : points) {
      pxr::GfVec3d worldPt = xform.Transform(pxr::GfVec3d(pt));
      bboxMin[0] = std::min(bboxMin[0], worldPt[0]);
      bboxMin[1] = std::min(bboxMin[1], worldPt[1]);
      bboxMin[2] = std::min(bboxMin[2], worldPt[2]);
      bboxMax[0] = std::max(bboxMax[0], worldPt[0]);
      bboxMax[1] = std::max(bboxMax[1], worldPt[1]);
      bboxMax[2] = std::max(bboxMax[2], worldPt[2]);
    }
  }

  return { bboxMin, bboxMax };
}

} // namespace

void ThumbnailGenerator::GenerateThumbnail(const QString& assetPath, const QString& outputPath) {
  // UsdImagingGLEngine requires an active OpenGL context. QOffscreenSurface
  // gives us one without creating a visible window.
  QOffscreenSurface surface;
  surface.create();

  QOpenGLContext context;
  context.create();
  context.makeCurrent(&surface);

  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(assetPath.toStdString());

  QOpenGLFramebufferObjectFormat fboFormat;
  fboFormat.setAttachment(QOpenGLFramebufferObject::Depth);
  fboFormat.setInternalTextureFormat(GL_RGB8);
  // TODO: Re-enable MSAA once we confirm it works with UsdImagingGLEngine on
  // the target hardware. Currently disabled because it causes a blank render.
  // fboFormat.setSamples(4);

  QOpenGLFramebufferObject fbo(512, 512, fboFormat);

  pxr::UsdImagingGLRenderParams params;
  params.frame = pxr::UsdTimeCode::Default();
  params.complexity = 1.0f;
  params.drawMode = pxr::UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
  params.enableLighting = true;
  params.clearColor = pxr::GfVec4f(0.2f, 0.2f, 0.2f, 1.0f);
  // CULL_STYLE_NOTHING ensures we see the asset even if face winding is
  // inconsistent, which is common in assets imported from other DCCs.
  params.cullStyle = pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;

  // USD stages can be Y-up or Z-up depending on the originating DCC.
  // UsdImagingGLEngine assumes Y-up, so for Z-up stages we bake a -90° X
  // rotation onto the default prim before rendering.
  // NOTE: This mutates the stage, which is fine since this process owns it.
  pxr::TfToken upAxis = pxr::UsdGeomGetStageUpAxis(stage);
  if (upAxis == pxr::UsdGeomTokens->z) {
    pxr::UsdPrim defaultPrim = stage->GetDefaultPrim();
    if (defaultPrim.IsValid()) {
      pxr::UsdGeomXformable root(defaultPrim);
      pxr::GfMatrix4d correction;
      correction.SetRotate(pxr::GfRotation(pxr::GfVec3d(1, 0, 0), -90));
      root.MakeMatrixXform().Set(correction);
    }
  }

  // Place the camera based on the scene's bounding box so the asset fills the
  // frame regardless of its real-world scale.
  std::tuple<pxr::GfVec3d, pxr::GfVec3d> bbox = getBbox(stage);
  pxr::GfVec3d bboxCenter = (std::get<0>(bbox) + std::get<1>(bbox)) * 0.5;
  pxr::GfVec3d bboxSize = std::get<1>(bbox) - std::get<0>(bbox);
  double sceneRadius = bboxSize.GetLength() * 0.5;
  double distance    = sceneRadius * 3.5; // multiplier tuned to avoid clipping

  // Slightly elevated off-axis angle gives better depth cues than a straight
  // front/side/top view.
  pxr::GfVec3d eye = bboxCenter + pxr::GfVec3d(1.0, 0.8, 3.0).GetNormalized() * distance;
  pxr::GfVec3d center = bboxCenter;
  pxr::GfVec3d up(0.0, 1.0, 0.0);
  pxr::GfMatrix4d viewMatrix = pxr::GfMatrix4d(1.0).SetLookAt(eye, center, up);

  double nearPlane = distance * 0.01;
  double farPlane = distance * 10.0;

  pxr::GfFrustum frustum;
  frustum.SetPerspective(30.0, 1.0, nearPlane, farPlane);

  pxr::UsdImagingGLEngine engine;
  engine.SetCameraState(viewMatrix, frustum.ComputeProjectionMatrix());

  // Key + fill two-light rig. The key is intentionally overbright (1.5)
  // to compensate for the dark ambient and produce readable thumbnails.
  pxr::GlfSimpleLightVector lights;

  pxr::GlfSimpleLight keyLight;
  keyLight.SetPosition(pxr::GfVec4f(7.0f, 15.0f, 7.0f, 1.0f));
  keyLight.SetDiffuse(pxr::GfVec4f(1.5f));
  lights.push_back(keyLight);

  pxr::GlfSimpleLight fillLight;
  fillLight.SetPosition(pxr::GfVec4f(-5.0f, 5.0f, -2.0f, 1.0f));
  fillLight.SetDiffuse(pxr::GfVec4f(0.4f, 0.4f, 0.4f, 1.0f));
  lights.push_back(fillLight);

  pxr::GlfSimpleMaterial material;
  material.SetAmbient(pxr::GfVec4f(0.1f, 0.1f, 0.1f, 1.0f));

  engine.SetLightingState(lights, material, pxr::GfVec4f(0.1f));

  fbo.bind();

  QOpenGLFunctions* gl = context.functions();
  gl->glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  engine.SetRenderViewport(pxr::GfVec4d(0, 0, 512, 512));
  engine.Render(stage->GetPseudoRoot(), params);

  fbo.release();

  QImage thumbnail = fbo.toImage();

  // UsdImagingGLEngine renders in linear light. Apply gamma 2.2 encoding so
  // the JPEG looks correct in standard image viewers and the Qt UI.
  // Pre-built as a LUT to avoid calling pow() per-pixel.
  uchar lut[256];
  for (int i = 0; i < 256; i++) lut[i] = static_cast<uchar>(std::pow(i / 255.0, 1.0 / 2.2) * 255.0 + 0.5);

  for (int y = 0; y < thumbnail.height(); y++) {
    uchar* line = thumbnail.scanLine(y);
    for (int x  = 0; x < thumbnail.width() * 4; x++) {
      line[x] = lut[line[x]];
    }
  }

  thumbnail.save(outputPath, "jpg", 88);

  context.doneCurrent();
}
