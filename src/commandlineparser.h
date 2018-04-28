#ifndef COMMANDLINEPARSER_H
#define COMMANDLINEPARSER_H

#include <QObject>
#include <QCommandLineParser>
#include <QApplication>
#include <QStringList>
#include "bingio.h"

class CommandLineParser : public QObject
{
    Q_OBJECT
public:
    explicit CommandLineParser(QObject *parent = nullptr);
    void setup(QApplication *app);
    int process(BingIO *bing);

signals:

public slots:
private:
   bool _refresh;
   bool _rotate;
   bool _deleteOpt;
   bool _force;
   bool _clean;
   QStringList _args;
};

#endif // COMMANDLINEPARSER_H
