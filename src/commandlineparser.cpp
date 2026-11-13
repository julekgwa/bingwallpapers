#include "commandlineparser.h"
#include "bingio.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>

CommandLineParser::CommandLineParser(const QStringList &arguments) : _arguments(arguments)
{
    QCommandLineParser &parser = _parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Sets Bing's wallpaper of the day as your desktop background."));

    // Without this, Qt's own "-platform xcb" would be read as the short options
    // -p -l -a -t -f -o -r -m, which includes --rotate and --force.
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();
    const QCommandLineOption refreshOption({"s", "set"}, QCoreApplication::translate("main", "Download and set the new wallpaper."));
    const QCommandLineOption rotateOption({"r", "rotate"}, QCoreApplication::translate("main", "Rotate over the pictures in the wallpaper directory.\nOptionally set the interval by passing <hours> <minutes>."));
    const QCommandLineOption deleteOption({"d", "delete"}, QCoreApplication::translate("main", "Delete the current wallpaper."));
    const QCommandLineOption cleanOption({"c", "clean"}, QCoreApplication::translate("main", "Remove pictures older than the optional <days>, default is 7 days."));
    const QCommandLineOption forceOption({"f", "force"}, QCoreApplication::translate("main", "With --set, delete the current wallpaper first so it is downloaded again."));
    const QCommandLineOption ultraHdOption("4k", QCoreApplication::translate("main", "Download the Ultra HD (4K) picture when Bing has one. The choice is saved."), "on|off");
    const QCommandLineOption backgroundOption("background", QCoreApplication::translate("main", "Start hidden in the system tray."));

    parser.addOptions({refreshOption, rotateOption, deleteOption, cleanOption, forceOption, ultraHdOption, backgroundOption});

    // parse() ignores options it doesn't know, such as Qt's own -platform.
    parser.parse(arguments);

    _refresh = parser.isSet(refreshOption);
    _rotate = parser.isSet(rotateOption);
    _delete = parser.isSet(deleteOption);
    _force = parser.isSet(forceOption);
    _clean = parser.isSet(cleanOption);
    _background = parser.isSet(backgroundOption);
    _hasUltraHd = parser.isSet(ultraHdOption);
    if (_hasUltraHd)
        _ultraHd = parser.value(ultraHdOption).trimmed().toLower();
    _args = parser.positionalArguments();

    _information = parser.isSet(helpOption) || parser.isSet(versionOption);
    _command = _refresh || _rotate || _delete || _clean || _hasUltraHd || _information;
}

void CommandLineParser::showInformation()
{
    if (_information)
        _parser.process(_arguments);
}

int CommandLineParser::process(BingIO *bing) const
{
    QTextStream out(stdout);
    QTextStream err(stderr);
    int status = 0;

    // Handled first, so that it also applies to a --set given in the same command.
    if (_hasUltraHd) {
        if (_ultraHd == "on" || _ultraHd == "yes" || _ultraHd == "true" || _ultraHd == "1") {
            bing->setUltraHd(true);
        } else if (_ultraHd == "off" || _ultraHd == "no" || _ultraHd == "false" || _ultraHd == "0") {
            bing->setUltraHd(false);
        } else {
            err << "Invalid value for --4k: \"" << _ultraHd << "\" (use on or off)\n";
            return 2;
        }
        out << "Ultra HD (4K): " << (bing->ultraHd() ? "on" : "off") << "\n";
        out.flush();
    }

    if (_refresh) {
        if (_force)
            bing->deleteCurrentWallpaper();
        if (!bing->refreshBlocking())
            status = 1;
    }

    if (_rotate) {
        if (_args.isEmpty()) {
            if (!bing->rotateOnce())
                status = 1;
        } else {
            bool ok = false;
            const int hours = _args[0].toInt(&ok);
            int minutes = 0;
            if (!ok || hours < 0) {
                err << "Invalid hours: " << _args[0] << "\n";
                return 2;
            }
            if (_args.size() < 2) {
                err << "Missing minutes, rotation will be set to hours.\n";
            } else {
                minutes = _args[1].toInt(&ok);
                if (!ok || minutes < 0) {
                    err << "Invalid minutes: " << _args[1] << "\n";
                    return 2;
                }
            }
            const int total = hours * 60 + minutes;
            if (total < BingIO::MinRotateMinutes) {
                err << "Please enter a time of at least " << BingIO::MinRotateMinutes << " minutes.\n";
                return 2;
            }
            bing->setRotateMinutes(total);
            if (!bing->refreshBlocking())
                status = 1;
        }
    }

    if (_delete)
        bing->deleteCurrentWallpaper();

    if (_clean) {
        int days = 7;
        if (!_args.isEmpty()) {
            bool ok = false;
            days = _args[0].toInt(&ok);
            if (!ok || days < 0) {
                err << "Invalid number of days: " << _args[0] << "\n";
                return 2;
            }
        }
        bing->cleanDir(days);
    }

    return status;
}
