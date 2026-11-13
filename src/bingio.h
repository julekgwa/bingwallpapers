#ifndef BINGIO_H
#define BINGIO_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QTimer>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QtQml/qqmlregistration.h>
#include <functional>
#include "bingfeed.h"
#include "dbmanager.h"
#include "wallpapersetter.h"

class QProcess;
class QQmlEngine;
class QJSEngine;

class BingIO : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString region READ region WRITE setRegion NOTIFY settingsChanged)
    Q_PROPERTY(bool setBackground READ setBackground WRITE setSetBackground NOTIFY settingsChanged)
    Q_PROPERTY(bool setLockScreen READ setLockScreen WRITE setSetLockScreen NOTIFY settingsChanged)
    Q_PROPERTY(bool ultraHd READ ultraHd WRITE setUltraHd NOTIFY settingsChanged)
    Q_PROPERTY(int rotateMinutes READ rotateMinutes WRITE setRotateMinutes NOTIFY settingsChanged)
    Q_PROPERTY(int deleteDays READ deleteDays WRITE setDeleteDays NOTIFY settingsChanged)
    Q_PROPERTY(QString wallpaperDir READ wallpaperDir WRITE setWallpaperDir NOTIFY settingsChanged)
    Q_PROPERTY(QString wallpaperTitle READ wallpaperTitle NOTIFY wallpaperChanged)
    Q_PROPERTY(QString wallpaperCopyright READ wallpaperCopyright NOTIFY wallpaperChanged)
    Q_PROPERTY(QUrl wallpaperUrl READ wallpaperUrl NOTIFY wallpaperChanged)
    Q_PROPERTY(QString nextRefresh READ nextRefresh NOTIFY nextRefreshChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool online READ online NOTIFY onlineChanged)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(int minRotateMinutes READ minRotateMinutes CONSTANT)

public:
    // Directory states returned by dirState().
    enum DirState { DirValid = 0, DirMissing = 1, DirDenied = 2 };
    Q_ENUM(DirState)

    static constexpr int MinRotateMinutes = 3;

    // No default argument: a default-constructible type would make the QML
    // engine build its own instance instead of calling create().
    explicit BingIO(QObject *parent);
    // Uses the given folders instead of the standard locations; for tests.
    BingIO(const QString &appDirectory, const QString &wallpaperDirectory, QObject *parent);
    ~BingIO() override;

    // The QML singleton is the instance main() created, so the C++ side
    // (tray, command line) and the UI share one object.
    static void setInstance(BingIO *instance) { s_instance = instance; }
    static BingIO *create(QQmlEngine *, QJSEngine *) { return s_instance; }

    QString region() const { return _region; }
    bool setBackground() const { return _set_background; }
    bool setLockScreen() const { return _set_lock_screen; }
    bool ultraHd() const { return _ultra_hd; }
    int rotateMinutes() const { return int(_rotate_ms / 60000); }
    int deleteDays() const { return _delete_days; }
    QString wallpaperDir() const { return _wallpaper_dir; }
    QString wallpaperTitle() const { return _wallpaper_title; }
    QString wallpaperCopyright() const { return _wallpaper_copyright; }
    QUrl wallpaperUrl() const;
    QString nextRefresh() const { return _next_refresh; }
    bool busy() const { return _busy; }
    bool online() const { return _online; }
    QString version() const;
    int minRotateMinutes() const { return MinRotateMinutes; }

    void setRegion(const QString &region);
    void setSetBackground(bool enabled);
    void setSetLockScreen(bool enabled);
    void setUltraHd(bool enabled);
    void setRotateMinutes(int minutes);
    void setDeleteDays(int days);
    void setWallpaperDir(const QString &dir);

    // Checks a directory the user picked: DirValid, DirMissing or DirDenied.
    Q_INVOKABLE int dirState(const QString &dir) const;
    Q_INVOKABLE QString urlToLocalPath(const QUrl &url) const;

    // Downloads today's picture and applies it, or re-applies a saved one when
    // Bing can't be reached or the picture is already there. Never blocks the
    // UI; the result is reported through refreshFinished(). A manual refresh
    // is refused while offline; scheduled ones wait until a connection returns.
    Q_INVOKABLE void refresh(bool manual = true);

    // Applies a random saved picture, without going online.
    void rotate();

    // Tests point this at a local server instead of https://www.bing.com.
    void setBaseUrl(const QUrl &url) { _base_url = url; }

    // Starts the daily, rotation and start-up timers (GUI only).
    void startScheduling();

    // Command line helpers. These run an event loop until the work is done.
    bool refreshBlocking();
    bool rotateOnce();
    bool deleteCurrentWallpaper();
    int cleanDir(int days);

signals:
    void settingsChanged();
    void wallpaperChanged();
    void nextRefreshChanged();
    void busyChanged();
    void onlineChanged();
    void refreshFinished(bool ok, const QString &message);

private:
    static inline BingIO *s_instance = nullptr;

    void load_config();
    void save_config();
    void scheduleSave();
    void setBusy(bool busy);
    void setOnline(bool online);
    void fetchToday();
    void onFeed(const BingFeedEntry &entry, bool ok);
    void download(const QUrl &url, const QString &name, bool followRedirects,
                  const std::function<void(bool)> &done);
    void downloadFirst(const QList<BingFeedEntry::Candidate> &candidates, int index,
                       const std::function<void(const QString &)> &done);
    void useWallpaper(const QString &name, const QString &copyright, const QString &message);
    void applyToDesktop(const QString &path, const std::function<void(bool, const QString &)> &done);
    void runCommands(const QList<WallpaperSetter::Command> &commands, int index,
                     const std::function<void(bool, const QString &)> &done);
    QStringList savedImages() const;
    QString pickRandomSaved() const;
    bool runBlocking(const std::function<void()> &start);
    void applyInfo(const QString &info);
    void finish(bool ok, const QString &message);
    void scheduleDaily();
    void restartRotation();
    void refreshWhenOnline();
    static QDateTime nextRefreshTime(const QDateTime &now, int hour_utc);
    QString describeNextRefresh(const QDateTime &when) const;

    QString _app_directory;
    QString _config;
    QString _region = "en-US";
    QString _wallpaper_dir;
    QString _current_wallpaper;
    QString _wallpaper_title;
    QString _wallpaper_copyright;
    QString _next_refresh;
    bool _set_background = true;
    bool _set_lock_screen = false;
    bool _ultra_hd = true;
    qint64 _rotate_ms = 0;
    int _delete_days = 0;
    int _refresh_hour_utc = 8;
    bool _busy = false;
    bool _online = true;
    bool _pending_refresh = false;
    bool _scheduling = false;

    QUrl _base_url{"https://www.bing.com"};
    QNetworkAccessManager _net;
    QTimer _save_timer;
    QTimer _daily_timer;
    QTimer _rotate_timer;
    DbManager _db;
};

#endif // BINGIO_H
