#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

class DbManager
{
public:
    DbManager();
    bool insertQuery(QString image_name, QString description);
    QString selectQuery(QString image_name);
private:
    void dbConnect();
    void dbInit();
    bool _connected = false;
    QString _create_table = "CREATE TABLE IF NOT EXISTS bingwallpapers(id INTEGER PRIMARY KEY AUTOINCREMENT, image_name TEXT NOT NULL, description TEXT NOT NULL, CONSTRAINT description_unique UNIQUE (description), CONSTRAINT image_name_unique UNIQUE (image_name))";
    QSqlDatabase _db;
};

#endif // DBMANAGER_H
