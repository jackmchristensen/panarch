#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "backend/Backend.h"

int main(int argc, char *argv[]) 
{
  QCoreApplication::setOrganizationName("Panarch");
  QCoreApplication::setApplicationName("Panarch");

  QGuiApplication app(argc, argv);

  Backend backend;
  backend.initialize();
 
  // qputenv("QT_QUICK_CONTROLS_STYLE", "Universal");
  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("backend", &backend);
  engine.load(QUrl(QStringLiteral("qrc:/src/ui/Main.qml")));

  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
