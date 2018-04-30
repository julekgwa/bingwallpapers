#include <QApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QQuickWidget>
#include <QSystemTrayIcon>
#include <QQmlContext>
#include <QQmlEngine>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QMessageBox>
#include "bingio.h"
#include <QCommandLineParser>
#include <QDebug>
#include "commandlineparser.h"

// Declare a user-defined data type to work with an icon in QML
Q_DECLARE_METATYPE(QSystemTrayIcon::ActivationReason)
Q_DECL_EXPORT int main(int argc, char *argv[])
{

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);


    QApplication app(argc, argv);

    qApp->setQuitOnLastWindowClosed(false); // prevent app from closing, when closing dialog message

    // set application name
    app.setApplicationName("Bingwallpapers");
    app.setOrganizationName("Linuxer");
    app.setApplicationVersion(BingIO::version);
    QQmlApplicationEngine engine;

    QQmlContext *context = engine.rootContext();
    BingIO bing;
    CommandLineParser cmd;

    // processs command line arguments
    if (argc > 1) {
        cmd.setup(&app);
        return cmd.process(&bing);
    }

    context->setContextProperty("BingIO", &bing);
    QSystemSemaphore semaphore("Bingwallpapers", 1);  // create semaphore
    semaphore.acquire(); // Raise the semaphore, barring other instances to work with shared memory

#ifndef Q_OS_WIN32
    // in linux / unix shared memory is not freed when the application terminates abnormally,
    // so you need to get rid of the garbage
    QSharedMemory nix_fix_shared_memory("Bingwallpapers Shared Memory");
    if(nix_fix_shared_memory.attach()){
        nix_fix_shared_memory.detach();
    }
#endif

    QSharedMemory sharedMemory("Bingwallpapers Shared Memory");  // Create a copy of the shared memory
    bool is_running;            // variable to test the already running application
    if (sharedMemory.attach()){ // We are trying to attach a copy of the shared memory
        // To an existing segment
        is_running = true;      // If successful, it determines that there is already a running instance
    }else{
        sharedMemory.create(1); // Otherwise allocate 1 byte of memory
        is_running = false;     // And determines that another instance is not running
    }
    semaphore.release();

    // If you already run one instance of the application, then we inform the user about it
    // and complete the current instance of the application
    if(is_running){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("The application is already running.\n"
                       "Open your system icon tray.");
        msgBox.exec();
        return -1;
    }
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
