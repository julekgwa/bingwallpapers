#include "bingio.h"

#include <QDebug>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QNetworkInformation>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimeZone>
#include <QUrlQuery>
#include <yaml-cpp/yaml.h>

namespace {

constexpr int kFeedTimeoutMs = 30000;
constexpr int kDownloadTimeoutMs = 120000;
constexpr int kCommandTimeoutMs = 15000;
constexpr int kStartupDelayMs = 10000;
constexpr qint64 kMinImageBytes = 1024;

const QStringList kImageFilters = {"*.jpg", "*.jpeg", "*.png"};

bool looksLikeImage(const QByteArray &data)
{
    return data.size() >= kMinImageBytes
            && (data.startsWith("\xFF\xD8\xFF") || data.startsWith("\x89PNG"));
}

QString ensureDirectory(const QString &dir)
{
    QDir().mkpath(dir);
    return dir;
}

// "en-UK" is not a Bing market; older configs may still contain it.
QString normalizeRegion(const QString &region)
{
    return region == "en-UK" ? QStringLiteral("en-GB") : region;
}

// Reads one key from a YAML node, leaving `out` untouched when it is missing
// or has the wrong type.
template <typename T>
void readKey(const YAML::Node &node, const char *key, T &out)
{
    try {
        if (node[key])
            out = node[key].as<T>();
    } catch (const YAML::Exception &e) {
        qWarning() << "Ignoring invalid config value" << key << e.what();
    }
}

} // namespace

BingIO::BingIO(QObject *parent)
    : BingIO(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation),
             QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/Bingwallpapers",
             parent)
{
}

BingIO::BingIO(const QString &appDirectory, const QString &wallpaperDirectory, QObject *parent)
    : QObject(parent),
      _app_directory(ensureDirectory(appDirectory)),
      _db(_app_directory + "/bingwallpaper.db")
{
    _config = QDir(_app_directory).absoluteFilePath("config.yaml");
    _wallpaper_dir = wallpaperDirectory;

    // Earlier versions downloaded and ran a helper script; it is not used any more.
    QFile::remove(QDir(_app_directory).filePath("bing.sh"));
    QFile::remove(QDir(_app_directory).filePath("config.sh"));

    load_config();
    QDir().mkpath(_wallpaper_dir);
    if (!QFileInfo::exists(_config))
        save_config();

    if (!_current_wallpaper.isEmpty())
        applyInfo(_db.selectQuery(_current_wallpaper));

    _save_timer.setSingleShot(true);
    _save_timer.setInterval(300);
    connect(&_save_timer, &QTimer::timeout, this, &BingIO::save_config);

    _daily_timer.setSingleShot(true);
    connect(&_daily_timer, &QTimer::timeout, this, [this] {
        refreshWhenOnline();
        scheduleDaily();
    });
    connect(&_rotate_timer, &QTimer::timeout, this, &BingIO::rotate);

    if (QNetworkInformation::loadDefaultBackend()) {
        QNetworkInformation *info = QNetworkInformation::instance();
        auto update = [this](QNetworkInformation::Reachability r) {
            setOnline(r != QNetworkInformation::Reachability::Disconnected
                      && r != QNetworkInformation::Reachability::Local
                      && r != QNetworkInformation::Reachability::Site);
        };
        update(info->reachability());
        connect(info, &QNetworkInformation::reachabilityChanged, this, update);
    }
}

BingIO::~BingIO()
{
    if (_save_timer.isActive())
        save_config();
}

QString BingIO::version() const
{
    return QStringLiteral(BINGWALLPAPERS_VERSION);
}

QUrl BingIO::wallpaperUrl() const
{
    const QString name = QFileInfo(_current_wallpaper).fileName();
    if (name.isEmpty())
        return QUrl();
    const QString path = QDir(_wallpaper_dir).filePath(name);
    return QFileInfo::exists(path) ? QUrl::fromLocalFile(path) : QUrl();
}

// ---------------------------------------------------------------- settings

void BingIO::setRegion(const QString &region)
{
    const QString code = normalizeRegion(region);
    if (code.isEmpty() || _region == code)
        return;
    _region = code;
    emit settingsChanged();
    scheduleSave();
}

void BingIO::setSetBackground(bool enabled)
{
    if (_set_background == enabled)
        return;
    _set_background = enabled;
    emit settingsChanged();
    scheduleSave();
}

void BingIO::setSetLockScreen(bool enabled)
{
    if (_set_lock_screen == enabled)
        return;
    _set_lock_screen = enabled;
    emit settingsChanged();
    scheduleSave();
}

void BingIO::setUltraHd(bool enabled)
{
    if (_ultra_hd == enabled)
        return;
    _ultra_hd = enabled;
    emit settingsChanged();
    scheduleSave();
}

void BingIO::setRotateMinutes(int minutes)
{
    if (minutes < 0)
        minutes = 0;
    else if (minutes > 0 && minutes < MinRotateMinutes)
        minutes = MinRotateMinutes;

    const qint64 ms = qint64(minutes) * 60000;
    if (_rotate_ms == ms)
        return;
    _rotate_ms = ms;
    restartRotation();
    emit settingsChanged();
    scheduleSave();
}

void BingIO::setDeleteDays(int days)
{
    days = qMax(0, days);
    if (_delete_days == days)
        return;
    _delete_days = days;
    emit settingsChanged();
    scheduleSave();
}

void BingIO::setWallpaperDir(const QString &dir)
{
    QString clean = QDir::cleanPath(dir.trimmed());
    if (clean.isEmpty() || _wallpaper_dir == clean)
        return;
    _wallpaper_dir = clean;
    emit settingsChanged();
    emit wallpaperChanged();
    scheduleSave();
}

int BingIO::dirState(const QString &dir) const
{
    const QFileInfo info(dir.trimmed());
    if (dir.trimmed().isEmpty() || !info.isDir())
        return DirMissing;
    return (info.isReadable() && info.isWritable()) ? DirValid : DirDenied;
}

QString BingIO::urlToLocalPath(const QUrl &url) const
{
    return url.toLocalFile();
}

// ------------------------------------------------------------------ config

void BingIO::load_config()
{
    if (!QFileInfo::exists(_config))
        return;

    try {
        const YAML::Node config = YAML::LoadFile(_config.toStdString());

        std::string text;
        readKey(config, "REGION", text);
        if (!text.empty())
            _region = normalizeRegion(QString::fromStdString(text));

        readKey(config, "ROTATE", _rotate_ms);
        if (_rotate_ms < 0)
            _rotate_ms = 0;

        text.clear();
        readKey(config, "WALLPAPER_DIR", text);
        // Very old configs stored "0" to mean "use the default directory".
        if (!text.empty() && text != "0")
            _wallpaper_dir = QString::fromStdString(text);

        readKey(config, "SET_BACKGROUND", _set_background);
        readKey(config, "SET_LOCKSCREEN", _set_lock_screen);
        readKey(config, "ULTRA_HD", _ultra_hd);
        readKey(config, "SET_DELETE_DAYS", _delete_days);
        readKey(config, "REFRESH_TIME", _refresh_hour_utc);

        text.clear();
        readKey(config, "CURRENT_WALLPAPER", text);
        _current_wallpaper = QString::fromStdString(text);
    } catch (const YAML::Exception &e) {
        qWarning() << "Unable to read" << _config << e.what() << "- using defaults";
    }
}

void BingIO::save_config()
{
    _save_timer.stop();

    YAML::Emitter config;
    config << YAML::BeginMap;
    config << YAML::Key << "REGION" << YAML::Value << _region.toStdString();
    config << YAML::Key << "ROTATE" << YAML::Value << static_cast<long long>(_rotate_ms);
    config << YAML::Key << "WALLPAPER_DIR" << YAML::Value << _wallpaper_dir.toStdString();
    config << YAML::Key << "SET_BACKGROUND" << YAML::Value << _set_background;
    config << YAML::Key << "SET_LOCKSCREEN" << YAML::Value << _set_lock_screen;
    config << YAML::Key << "ULTRA_HD" << YAML::Value << _ultra_hd;
    config << YAML::Key << "SET_DELETE_DAYS" << YAML::Value << _delete_days;
    config << YAML::Key << "REFRESH_TIME" << YAML::Value << _refresh_hour_utc;
    config << YAML::Key << "CURRENT_WALLPAPER" << YAML::Value << _current_wallpaper.toStdString();
    config << YAML::EndMap;

    QSaveFile file(_config);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Unable to write" << _config << file.errorString();
        return;
    }
    file.write(config.c_str());
    file.write("\n");
    if (!file.commit())
        qWarning() << "Unable to save" << _config << file.errorString();
}

void BingIO::scheduleSave()
{
    _save_timer.start();
}

// ----------------------------------------------------------------- refresh

void BingIO::setBusy(bool busy)
{
    if (_busy == busy)
        return;
    _busy = busy;
    emit busyChanged();
}

void BingIO::setOnline(bool online)
{
    if (_online == online)
        return;
    _online = online;
    emit onlineChanged();
    if (_online && _pending_refresh) {
        _pending_refresh = false;
        refresh(false);
    }
}

void BingIO::refresh(bool manual)
{
    if (_busy)
        return;

    if (manual && !_online) {
        _pending_refresh = true;
        emit refreshFinished(false, tr("No internet connection – retrying"));
        return;
    }

    _pending_refresh = false;
    setBusy(true);
    fetchToday();
}

void BingIO::refreshWhenOnline()
{
    if (_online)
        refresh(false);
    else
        _pending_refresh = true;
}

void BingIO::rotate()
{
    if (_busy)
        return;

    setBusy(true);
    const QString name = pickRandomSaved();
    if (name.isEmpty()) {
        finish(false, tr("There are no saved wallpapers to rotate through"));
        return;
    }
    useWallpaper(name, QString(), tr("Wallpaper changed"));
}

QStringList BingIO::savedImages() const
{
    return QDir(_wallpaper_dir).entryList(kImageFilters, QDir::Files, QDir::Name);
}

// Picks a saved picture other than the one in use, when there is a choice.
QString BingIO::pickRandomSaved() const
{
    QStringList images = savedImages();
    if (images.size() > 1)
        images.removeAll(QFileInfo(_current_wallpaper).fileName());
    if (images.isEmpty())
        return QString();
    return images.at(int(QRandomGenerator::global()->bounded(images.size())));
}

void BingIO::fetchToday()
{
    QUrl url = _base_url.resolved(QUrl("/HPImageArchive.aspx"));
    QUrlQuery query;
    query.addQueryItem("format", "xml");
    query.addQueryItem("idx", "0");
    query.addQueryItem("n", "1");
    query.addQueryItem("mkt", _region);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setTransferTimeout(kFeedTimeoutMs);
    QNetworkReply *reply = _net.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "Unable to read Bing's feed:" << reply->errorString();
            onFeed({}, false);
            return;
        }
        const BingFeedEntry entry = BingFeedEntry::parse(reply->readAll());
        onFeed(entry, entry.isValid());
    });
}

void BingIO::onFeed(const BingFeedEntry &entry, bool ok)
{
    // Offline, or Bing unreachable: a saved picture is better than nothing.
    if (!ok) {
        const QString name = pickRandomSaved();
        if (name.isEmpty())
            finish(false, tr("Couldn't reach Bing"));
        else
            useWallpaper(name, QString(), tr("Offline – applied a saved wallpaper"));
        return;
    }

    const auto candidates = entry.candidates(_base_url, _ultra_hd);
    if (candidates.isEmpty()) {
        finish(false, tr("Bing sent a picture that can't be saved"));
        return;
    }

    // Which version of today's picture is already saved, best first? Index 0
    // is what the settings ask for; a later one is a lower quality copy, e.g.
    // Full HD saved before 4K was switched on.
    const QDir dir(_wallpaper_dir);
    int existingIndex = -1;
    for (int i = 0; i < candidates.size(); ++i) {
        if (dir.exists(candidates.at(i).name)) {
            existingIndex = i;
            break;
        }
    }

    auto useExisting = [this, entry, candidates](int index) {
        const QString random = _rotate_ms > 0 ? pickRandomSaved() : QString();
        if (random.isEmpty())
            useWallpaper(candidates.at(index).name, entry.copyright, tr("Wallpaper is up to date"));
        else
            useWallpaper(random, QString(), tr("Wallpaper changed"));
    };

    if (existingIndex == 0) {
        useExisting(0);
        return;
    }

    // Nothing saved, or only a lower quality copy: try the better versions only.
    const int better = existingIndex < 0 ? int(candidates.size()) : existingIndex;
    QDir().mkpath(_wallpaper_dir);
    downloadFirst(candidates.mid(0, better), 0, [=](const QString &name) {
        if (name.isEmpty()) {
            if (existingIndex >= 0)
                useExisting(existingIndex);      // the better version isn't available; keep what we have
            else
                finish(false, tr("Couldn't download today's wallpaper"));
            return;
        }

        // Keep one copy of the picture: remove the other sizes this app saved.
        for (const auto &other : entry.candidates(_base_url, true)) {
            if (other.name != name)
                QFile::remove(dir.filePath(other.name));
        }
        useWallpaper(name, entry.copyright, tr("Wallpaper updated"));
    });
}

// Tries each version in turn and reports the file name of the first that
// downloads, or an empty name when none does. Bing answers 404 (or a redirect)
// for sizes it doesn't have, so redirects are only followed for Bing's own
// default picture.
void BingIO::downloadFirst(const QList<BingFeedEntry::Candidate> &candidates, int index,
                           const std::function<void(const QString &)> &done)
{
    if (index >= candidates.size()) {
        done(QString());
        return;
    }

    const auto candidate = candidates.at(index);
    download(candidate.url, candidate.name, candidate.isDefault, [=](bool ok) {
        if (ok)
            done(candidate.name);
        else
            downloadFirst(candidates, index + 1, done);
    });
}

void BingIO::download(const QUrl &url, const QString &name, bool followRedirects,
                      const std::function<void(bool)> &done)
{
    if (!url.isValid() || name.isEmpty()) {
        done(false);
        return;
    }

    QNetworkRequest request(url);
    request.setTransferTimeout(kDownloadTimeoutMs);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         followRedirects ? QNetworkRequest::NoLessSafeRedirectPolicy
                                         : QNetworkRequest::ManualRedirectPolicy);
    QNetworkReply *reply = _net.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, name, done] {
        reply->deleteLater();
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();

        bool ok = reply->error() == QNetworkReply::NoError && status == 200 && looksLikeImage(body);
        if (ok) {
            QSaveFile file(QDir(_wallpaper_dir).filePath(name));
            ok = file.open(QIODevice::WriteOnly) && file.write(body) == body.size() && file.commit();
            if (!ok)
                qWarning() << "Unable to save" << name << file.errorString();
        }
        done(ok);
    });
}

// Makes `name` (a file in the wallpaper directory) the current wallpaper.
void BingIO::useWallpaper(const QString &name, const QString &copyright, const QString &message)
{
    _current_wallpaper = name;
    if (!copyright.isEmpty())
        _db.insertQuery(name, copyright);
    applyInfo(_db.selectQuery(name));

    applyToDesktop(QDir(_wallpaper_dir).filePath(name), [this, message](bool ok, const QString &error) {
        if (_delete_days > 0)
            cleanDir(_delete_days);
        finish(ok, ok ? message : error);
    });
}

void BingIO::applyToDesktop(const QString &path, const std::function<void(bool, const QString &)> &done)
{
    if (!_set_background && !_set_lock_screen) {
        done(true, QString());
        return;
    }

    const auto desktop = WallpaperSetter::detectDesktop(qEnvironmentVariable("XDG_CURRENT_DESKTOP"));
    const auto commands = WallpaperSetter::commands(desktop, path, _set_background, _set_lock_screen);
    if (commands.isEmpty()) {
        done(false, tr("Wallpaper saved, but this desktop can't be set automatically"));
        return;
    }
    runCommands(commands, 0, done);
}

void BingIO::runCommands(const QList<WallpaperSetter::Command> &commands, int index,
                         const std::function<void(bool, const QString &)> &done)
{
    if (index >= commands.size()) {
        done(true, QString());
        return;
    }

    const WallpaperSetter::Command command = commands.at(index);
    auto *process = new QProcess(this);

    // The same defaults bing.sh used, so it also works from cron or autostart.
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (!env.contains("DISPLAY") && !env.contains("WAYLAND_DISPLAY"))
        env.insert("DISPLAY", ":0");
    if (!env.contains("GSETTINGS_BACKEND"))
        env.insert("GSETTINGS_BACKEND", "dconf");
    process->setProcessEnvironment(env);

    auto next = [=](bool ok) {
        if (ok || command.optional)
            runCommands(commands, index + 1, done);
        else
            done(false, tr("Couldn't set the wallpaper (%1 failed)").arg(command.program));
    };

    connect(process, &QProcess::errorOccurred, this, [process, next](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart)
            return;
        process->deleteLater();
        next(false);
    });
    connect(process, &QProcess::finished, this, [process, next](int code, QProcess::ExitStatus status) {
        process->deleteLater();
        next(status == QProcess::NormalExit && code == 0);
    });
    QTimer::singleShot(kCommandTimeoutMs, process, [process] {
        if (process->state() != QProcess::NotRunning)
            process->kill();
    });

    process->start(command.program, command.args);
}

// Bing describes a picture as "Title (© Author)".
void BingIO::applyInfo(const QString &info)
{
    static const QRegularExpression pattern("^(.*?)\\s*\\(©\\s*(.*?)\\)\\s*$");
    const QRegularExpressionMatch match = pattern.match(info);

    if (match.hasMatch()) {
        _wallpaper_title = match.captured(1);
        _wallpaper_copyright = "© " + match.captured(2);
    } else {
        _wallpaper_title = info;
        _wallpaper_copyright.clear();
    }
    emit wallpaperChanged();
}

void BingIO::finish(bool ok, const QString &message)
{
    save_config();
    setBusy(false);
    emit refreshFinished(ok, message);
}

// -------------------------------------------------------------- scheduling

void BingIO::startScheduling()
{
    _scheduling = true;
    scheduleDaily();
    restartRotation();
    QTimer::singleShot(kStartupDelayMs, this, &BingIO::refreshWhenOnline);
}

void BingIO::restartRotation()
{
    _rotate_timer.stop();
    if (_scheduling && _rotate_ms > 0)
        _rotate_timer.start(int(qMin<qint64>(_rotate_ms, std::numeric_limits<int>::max())));
}

QDateTime BingIO::nextRefreshTime(const QDateTime &now, int hour_utc)
{
    QDateTime next(now.date(), QTime(hour_utc, 0), QTimeZone::UTC);
    if (next <= now)
        next = next.addDays(1);
    return next;
}

void BingIO::scheduleDaily()
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const QDateTime next = nextRefreshTime(now, _refresh_hour_utc);

    _daily_timer.start(int(now.msecsTo(next)));
    _next_refresh = describeNextRefresh(next);
    emit nextRefreshChanged();
}

QString BingIO::describeNextRefresh(const QDateTime &when) const
{
    const QDateTime local = when.toLocalTime();
    const QString time = QLocale().toString(local.time(), QLocale::ShortFormat);
    const qint64 days = QDate::currentDate().daysTo(local.date());

    if (days <= 0)
        return tr("%1 today").arg(time);
    if (days == 1)
        return tr("%1 tomorrow").arg(time);
    return tr("%1 on %2").arg(time, QLocale().toString(local.date(), QLocale::ShortFormat));
}

// ---------------------------------------------------------- command line

bool BingIO::runBlocking(const std::function<void()> &start)
{
    bool done = false;
    bool result = false;
    QEventLoop loop;

    const QMetaObject::Connection connection =
            connect(this, &BingIO::refreshFinished, &loop, [&](bool ok, const QString &message) {
        qInfo().noquote() << message;
        result = ok;
        done = true;
        loop.quit();
    });

    start();
    if (!done)
        loop.exec();
    disconnect(connection);
    return result;
}

bool BingIO::refreshBlocking()
{
    return runBlocking([this] { refresh(false); });
}

bool BingIO::rotateOnce()
{
    return runBlocking([this] { rotate(); });
}

bool BingIO::deleteCurrentWallpaper()
{
    const QString name = QFileInfo(_current_wallpaper).fileName();
    if (name.isEmpty())
        return false;

    const bool ok = QFile::remove(QDir(_wallpaper_dir).filePath(name));
    _current_wallpaper.clear();
    save_config();
    emit wallpaperChanged();
    return ok;
}

// Removes pictures older than `days`. Only image files directly in the
// wallpaper directory are touched, and never the one currently in use.
int BingIO::cleanDir(int days)
{
    const QDateTime cutoff = QDateTime::currentDateTime().addDays(-days);
    const QString current = QFileInfo(_current_wallpaper).fileName();
    int removed = 0;

    const QDir dir(_wallpaper_dir);
    const QFileInfoList files = dir.entryInfoList(kImageFilters, QDir::Files);
    for (const QFileInfo &file : files) {
        if (file.fileName() != current && file.lastModified() < cutoff && QFile::remove(file.absoluteFilePath()))
            ++removed;
    }
    return removed;
}
