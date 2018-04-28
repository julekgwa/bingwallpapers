#include "commandlineparser.h"

CommandLineParser::CommandLineParser(QObject *parent) : QObject(parent)
{

}

void CommandLineParser::setup(QApplication *app) {
    QCommandLineParser parser;

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
    parser.process(*app);

    _refresh = parser.isSet(refreshOption);
    _rotate = parser.isSet(rotateOption);
    _deleteOpt = parser.isSet(deleteOption);
    _force = parser.isSet(forceOption);
    _clean = parser.isSet(cleanOption);

    _args = parser.positionalArguments();
}

int CommandLineParser::process(BingIO *bing) {
    if (_refresh) {
        if (_force)
            bing->delete_wallpaper();
        bing->run_script();
    }
    if (_rotate) {
        unsigned long int millisec = 1000;
        if (_args.size()) {
            int hours = _args[0].toInt();
            int minutes = 0;
            if (_args.size() < 2) {
                qWarning() << "Missing minutes, rotation will be set to hours.";
            } else {
                minutes = _args[1].toInt();
            }
            millisec = (hours * (60000 * 60) + (minutes * 60000));
            if (millisec < 180000) {
                qWarning() << "Please enter time greater than 2 minutes.";
                return 0;
            }
        }
        bing->rotateCmd(millisec);
    }
    if (_deleteOpt) {
        bing->delete_wallpaper();
    }
    if (_clean) {
        QString days = "7";
        if (_args.size()) {
            days = _args[0];
        }
        bing->clean_dir(days);
    }
    return 0;
}
