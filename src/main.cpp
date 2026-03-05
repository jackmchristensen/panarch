#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QAction>
#include <QQuickWindow>
#include <QShortcut>
#include <QKeySequence>
#include <QObject>
#include <QKeySequence>
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

  QQuickWindow* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());

  QShortcut* closeShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q), window, [&backend]() { backend.quitApp(); });
  QShortcut* addLibShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_O), window, [&backend]() { emit backend.openLibraryDialogRequested(); });

  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
