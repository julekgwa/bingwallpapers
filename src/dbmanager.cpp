#include "dbmanager.h"

DbManager::DbManager()
{

    dbConnect();
    dbInit();
}

void DbManager::dbConnect() {
    const QString DRIVER("QSQLITE");

    if (QSqlDatabase::isDriverAvailable(DRIVER)) {
        QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER, "bingdb");
        db.setDatabaseName("bingwallpaper.db");
        if (db.open()) {
            _connected = true;
            _db = QSqlDatabase::database("bingdb");
        }
    }
}

void DbManager::dbInit() {
    if (_connected) {
        QSqlQuery create_tb(_db);

        create_tb.prepare(_create_table);
        create_tb.exec();
    }
}

bool DbManager::insertQuery(QString image_name, QString description) {
    if (_connected) {
        QSqlQuery insert(_db);

        insert.prepare("INSERT INTO bingwallpapers (image_name, description) VALUES(:image_name, :description)");
        insert.bindValue(":image_name", image_name);
        insert.bindValue(":description", description);
        if (insert.exec()) {
            return true;
        }
    }
    return false;
}

QString DbManager::selectQuery(QString image_name) {
    if (_connected) {
       QSqlQuery select(_db);

       select.prepare("SELECT description FROM bingwallpapers WHERE image_name = :image_name");
       select.bindValue(":image_name", image_name);
       select.exec();
       if (select.first()) {
           return select.value(0).toString();
       }
    }
    return "";
}
