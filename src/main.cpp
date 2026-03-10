#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QAction>
#include <QQuickWindow>
#include <QShortcut>
#include <QKeySequence>
#include <QObject>
#include <QKeySequence>
#include <QLoggingCategory>
#include <qkeysequence.h>

#include "backend/Backend.h"

int main(int argc, char *argv[]) 
{
  QCoreApplication::setOrganizationName("Panarch");
  QCoreApplication::setApplicationName("Panarch");

  // Ignore this warning
  // Seems to be a bug/quirk in Qt and spams the console.
  qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& ctx, const QString& msg) {
    if (msg.contains("Ignoring sourceSize request for image url that came from grabToImage. Use the targetSize parameter of the grabToImage() function instead."))
      return;
    QTextStream(stderr) << msg << "\n";
  });

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
  QShortcut* searchShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_K), window, [&backend]() { emit backend.focusFilter(); });
  QShortcut* filterShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), window, [&backend]() { emit backend.focusFilter(); });

  if (engine.rootObjects().isEmpty())
    return -1;

  return app.exec();
}
