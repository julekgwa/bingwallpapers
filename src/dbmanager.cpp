#include "dbmanager.h"

static const QString kConnectionName = "bingdb";

DbManager::DbManager(const QString &path)
{
    dbConnect(path);
    dbInit();
}

DbManager::~DbManager()
{
    if (_db.isOpen())
        _db.close();
    _db = QSqlDatabase();
    QSqlDatabase::removeDatabase(kConnectionName);
}

void DbManager::dbConnect(const QString &path) {
    const QString DRIVER("QSQLITE");

    if (!QSqlDatabase::isDriverAvailable(DRIVER)) {
        qWarning("SQLite driver is not available");
        return;
    }

    _db = QSqlDatabase::addDatabase(DRIVER, kConnectionName);
    _db.setDatabaseName(path);
    _connected = _db.open();
    if (!_connected)
        qWarning() << "Unable to open database" << path << _db.lastError().text();
}

void DbManager::dbInit() {
    if (!_connected)
        return;

    QSqlQuery query(_db);
    query.exec(_create_table);

    // Carry over what older versions stored; the old table is left in place.
    if (query.exec("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = 'bingwallpapers'") && query.first())
        query.exec("INSERT OR IGNORE INTO wallpaper_info (image_name, description) "
                   "SELECT image_name, description FROM bingwallpapers");
}

bool DbManager::insertQuery(const QString &image_name, const QString &description) {
    if (!_connected)
        return false;

    QSqlQuery insert(_db);
    insert.prepare("INSERT OR REPLACE INTO wallpaper_info (image_name, description) VALUES(:image_name, :description)");
    insert.bindValue(":image_name", image_name);
    insert.bindValue(":description", description);
    return insert.exec();
}

QString DbManager::selectQuery(const QString &image_name) {
    if (!_connected)
        return QString();

    QSqlQuery select(_db);
    select.prepare("SELECT description FROM wallpaper_info WHERE image_name = :image_name");
    select.bindValue(":image_name", image_name);
    if (select.exec() && select.first())
        return select.value(0).toString();
    return QString();
}
