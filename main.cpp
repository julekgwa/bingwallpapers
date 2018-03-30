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

// Declare a user-defined data type to work with an icon in QML
Q_DECLARE_METATYPE(QSystemTrayIcon::ActivationReason)
Q_DECL_EXPORT int main(int argc, char *argv[])
{

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);


    QApplication app(argc, argv);

    // set application name
    app.setApplicationName("Bingwallpapers");
    app.setOrganizationName("Linuxer");
    app.setApplicationVersion("2018.03.30");
    QQmlApplicationEngine engine;

    QQmlContext *context = engine.rootContext();
    BingIO bing;

    // processs command line arguments
    if (argc > 1) {
        QCommandLineParser parser;
        //parser.setApplicationDescription("Downloads Bing’s wallpaper of the day and sets it as a desktop wallpaper or a lock screen image.");
        parser.addHelpOption();
        parser.addVersionOption();
        QCommandLineOption refreshOption(QStringList() << "s" << "set", QCoreApplication::translate("main", "Downloads/Set new wallpaper"));
        QCommandLineOption rotateOption(QStringList() << "r" << "rotate", QCoreApplication::translate("main", "Rotate over pictures in bingwallaper directory\n optionally set rotate intervals by passing <hours> <minutes>."));
        QCommandLineOption deleteOption(QStringList() << "d" << "delete", QCoreApplication::translate("main", "Delete current wallpaper"));
        QCommandLineOption cleanOption(QStringList() << "c" << "clean", QCoreApplication::translate("main", "Remove pictures older than optional <days>, default is 7 days"));
        QCommandLineOption forceOption(QStringList() << "f" << "force", QCoreApplication::translate("main", "Forcefully execute command"));
        parser.addOption(refreshOption);
        parser.addOption(rotateOption);
        parser.addOption(deleteOption);
        parser.addOption(forceOption);
        parser.addOption(cleanOption);
        parser.process(app);

        bool refresh = parser.isSet(refreshOption);
        bool rotate = parser.isSet(rotateOption);
        bool deleteOpt = parser.isSet(deleteOption);
        bool force = parser.isSet(forceOption);
        bool clean = parser.isSet(cleanOption);

        const QStringList args = parser.positionalArguments();

        if (refresh) {
            if (force)
                bing.delete_wallpaper();
            bing.run_script();
        }
        if (rotate) {
            unsigned long int millisec = 1000;
            if (args.size()) {
                int hours = args[0].toInt();
                int minutes = 0;
                if (args.size() < 2) {
                    qWarning() << "Missing minutes, rotation will be set to hours.";
                } else {
                    minutes = args[1].toInt();
                }
                millisec = (hours * (60000 * 60) + (minutes * 60000));
                if (millisec < 180000) {
                    qWarning() << "Please enter time greater than 2 minutes.";
                    return 0;
                }
            }
            bing.rotateCmd(millisec);
        }
        if (deleteOpt) {
            bing.delete_wallpaper();
        }
        if (clean) {
            QString days = "7";
            if (args.size()) {
                days = args[0];
            }
            bing.clean_dir(days);
        }
        return 0;
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
