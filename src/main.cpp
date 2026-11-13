#include <QApplication>
#include <QCoreApplication>
#include <QIcon>
#include <QLocalServer>
#include <QLocalSocket>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "bingio.h"
#include "commandlineparser.h"

static void setupIdentity()
{
    QCoreApplication::setApplicationName("Bingwallpapers");
    QCoreApplication::setOrganizationName("Linuxer");
    QCoreApplication::setApplicationVersion(QStringLiteral(BINGWALLPAPERS_VERSION));
}

int main(int argc, char *argv[])
{
    QStringList arguments;
    for (int i = 0; i < argc; ++i)
        arguments << QString::fromLocal8Bit(argv[i]);

    // Command line actions run without a GUI, so they also work over ssh or cron.
    CommandLineParser cmd(arguments);
    if (cmd.isCommand()) {
        QCoreApplication app(argc, argv);
        setupIdentity();
        cmd.showInformation();
        BingIO bing(nullptr);
        return cmd.process(&bing);
    }

    QApplication app(argc, argv);
    setupIdentity();
    app.setQuitOnLastWindowClosed(false); // the app lives in the system tray
    app.setWindowIcon(QIcon(":/resources/bing.png"));

    // Single instance: a second launch asks the first one to show its window.
    const QString serverName = QStringLiteral("bingwallpapers-%1")
            .arg(QString::fromLocal8Bit(qgetenv("USER")));
    {
        QLocalSocket probe;
        probe.connectToServer(serverName);
        if (probe.waitForConnected(500)) {
            probe.write("show");
            probe.waitForBytesWritten(500);
            return 0;
        }
    }
    QLocalServer::removeServer(serverName); // stale socket from a crash
    QLocalServer instanceServer;
    if (!instanceServer.listen(serverName))
        qWarning() << "Single-instance server unavailable:" << instanceServer.errorString();

    BingIO bing(nullptr);
    BingIO::setInstance(&bing);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("startHidden", cmd.startHidden());
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, [] { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.loadFromModule("BingApp", "Main");
    if (engine.rootObjects().isEmpty())
        return -1;

    QObject *window = engine.rootObjects().constFirst();
    QObject::connect(&instanceServer, &QLocalServer::newConnection, &app, [&] {
        while (QLocalSocket *client = instanceServer.nextPendingConnection()) {
            QObject::connect(client, &QLocalSocket::disconnected, client, &QObject::deleteLater);
            client->disconnectFromServer();
            QMetaObject::invokeMethod(window, "showWindow");
        }
    });

    bing.startScheduling();
    return app.exec();
}
