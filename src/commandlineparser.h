#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QCommandLineParser>
#include <QStringList>

class BingIO;

class CommandLineParser
{
public:
    // Parses the arguments without needing an application object, so the
    // caller can pick QCoreApplication (CLI) or QApplication (GUI).
    explicit CommandLineParser(const QStringList &arguments);

    // True when an action (set, rotate, delete, clean, 4k, help, version) was asked for.
    bool isCommand() const { return _command; }
    // True when the window should start hidden in the tray (autostart).
    bool startHidden() const { return _background; }
    // Prints --help / --version and exits. Needs an application object.
    void showInformation();
    int process(BingIO *bing) const;

private:
    QCommandLineParser _parser;
    QStringList _arguments;
    bool _information = false;
    bool _refresh = false;
    bool _rotate = false;
    bool _delete = false;
    bool _force = false;
    bool _clean = false;
    bool _background = false;
    bool _hasUltraHd = false;
    QString _ultraHd; // the value given to --4k
    bool _command = false;
    QStringList _args;
};

#endif // COMMANDLINEPARSER_H
