#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

class DbManager
{
public:
    explicit DbManager(const QString &path);
    ~DbManager();
    bool insertQuery(const QString &image_name, const QString &description);
    QString selectQuery(const QString &image_name);
private:
    void dbConnect(const QString &path);
    void dbInit();
    bool _connected = false;
    QString _create_table = "CREATE TABLE IF NOT EXISTS bingwallpapers(id INTEGER PRIMARY KEY AUTOINCREMENT, image_name TEXT NOT NULL, description TEXT NOT NULL, CONSTRAINT description_unique UNIQUE (description), CONSTRAINT image_name_unique UNIQUE (image_name))";
    QSqlDatabase _db;
};

#endif // DBMANAGER_H
