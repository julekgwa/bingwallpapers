#include <QApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QQuickWidget>
#include <QSystemTrayIcon>
#include <QQmlContext>
#include <QQmlEngine>
#include "bingio.h"

// Declare a user-defined data type to work with an icon in QML
Q_DECLARE_METATYPE(QSystemTrayIcon::ActivationReason)
Q_DECL_EXPORT int main(int argc, char *argv[])
{
#if defined(Q_OS_WIN)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif


    QApplication app(argc, argv);

    // set application name
    app.setApplicationName("Bingwallpapers");
    app.setOrganizationName("Linuxer");
    QQmlApplicationEngine engine;

    QQmlContext *context = engine.rootContext();
    BingIO bing;

    context->setContextProperty("BingIO", &bing);
    // Register QSystemTrayIcon in Qml
    qmlRegisterType<QSystemTrayIcon>("QSystemTrayIcon", 1, 0, "QSystemTrayIcon");
    // Register in QML the data type of click by tray icon
    qRegisterMetaType<QSystemTrayIcon::ActivationReason>("ActivationReason");
    // Set icon in the context of the engine
    engine.rootContext()->setContextProperty("iconTray", QIcon(":/resources/bing.png"));
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;
    return app.exec();
}
