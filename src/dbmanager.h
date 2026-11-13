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
    // One row per picture file. (The first version of this table also made the
    // description unique, so saving the same picture under a second file name,
    // e.g. a different size, was silently rejected and its title was lost.)
    QString _create_table = "CREATE TABLE IF NOT EXISTS wallpaper_info(image_name TEXT PRIMARY KEY, description TEXT NOT NULL)";
    QSqlDatabase _db;
};

#endif // DBMANAGER_H
