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

#include "render/ThumbnailGenerator.h"

namespace {

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
  QOffscreenSurface surface;
  surface.create();

  QOpenGLContext context;
  context.create();
  context.makeCurrent(&surface);

  pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(assetPath.toStdString());

  QOpenGLFramebufferObjectFormat fboFormat;
  fboFormat.setAttachment(QOpenGLFramebufferObject::Depth);
  fboFormat.setInternalTextureFormat(GL_RGB8);
  // fboFormat.setSamples(4);

  QOpenGLFramebufferObject fbo(512, 512, fboFormat);

  pxr::UsdImagingGLRenderParams params;
  params.frame = pxr::UsdTimeCode::Default();
  params.complexity = 1.0f;
  params.drawMode = pxr::UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;
  params.enableLighting = true;
  params.clearColor = pxr::GfVec4f(0.2f, 0.2f, 0.2f, 1.0f);
  params.cullStyle = pxr::UsdImagingGLCullStyle::CULL_STYLE_NOTHING;

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

  std::tuple<pxr::GfVec3d, pxr::GfVec3d> bbox = getBbox(stage);
  pxr::GfVec3d bboxCenter = (std::get<0>(bbox) + std::get<1>(bbox)) * 0.5;
  pxr::GfVec3d bboxSize = std::get<1>(bbox) - std::get<0>(bbox);
  double sceneRadius = bboxSize.GetLength() * 0.5;
  double distance = sceneRadius * 3.5;

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
