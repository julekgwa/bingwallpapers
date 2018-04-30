#include "bingio.h"

BingIO::BingIO(QObject *parent) : QObject(parent),
    _region("en-US"), _set_background_image(true), _set_lock_screen(false) ,
    _rotate(0), _delete_days(0), m_process(new QProcess(this)),
    _refresh_milliseconds(create_refresh_milliseconds(8)), _force_download(2)
{
    _app_directory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QString picture_directory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QDir app_dir(_app_directory);

    QString wallpaper_path = picture_directory + "/Bingwallpapers";

    _bing_wall_directory = wallpaper_path;
    _config = app_dir.absoluteFilePath("config.yaml");
    _shell_script = app_dir.absoluteFilePath("bing.sh");
    _download_script = 0;

    // check if appData folder exists
    if (!app_dir.exists()) {
        app_dir.mkpath(_app_directory);

        // build yaml map
        create_yaml_map();
        write_file(_config, _config_data);

        create_shell_script();
    }else {

        if (!file_exists(_config)) {
            // build yaml map
            create_yaml_map();
            write_file(_config, _config_data);
        }else {
            // config file data
            load_config();
        }

        if (!file_exists(_shell_script)) {
            create_shell_script();
        }
    }

    // set wallpaper info
    QString info = _db.selectQuery(_current_wallpaper);

    if (!info.isEmpty()) {
        _wallpaper_info = info;
        emit wallpaper_info_changed();
    }

    // create wallpaper directory
    QDir wallpaper_dir(wallpaper_path);
    if (!wallpaper_dir.exists()) {
        wallpaper_dir.mkpath(wallpaper_path);
    }
}

QString BingIO::get_region() {
    return this->_region;
}

QString BingIO::get_app_directory() {
    return this->_app_directory;
}

qint64 BingIO::get_refresh_milliseconds() {
    return _refresh_milliseconds;
}


void BingIO::set_region(QString rgn) {
    if (this->_region != rgn) {
        this->_region = rgn;
        emit data_changed();
    }
}

bool BingIO::file_exists(QString filename) {
    return QFileInfo::exists(filename);
}

bool BingIO::dir_exists(QString dir) {
    return QDir(dir).exists();
}

bool BingIO::dir_read_write(QString dir) {
    QFileInfo my_dir(dir);

    if (!my_dir.isDir())
        return false;
    if (my_dir.isWritable() && my_dir.isReadable())
        return true;
    return false;
}

QString BingIO::get_bing_wall_directory() {
    return _bing_wall_directory;
}

void BingIO::set_bing_wall_directory(QString directory) {
    if (this->_bing_wall_directory != directory) {
        _bing_wall_directory = directory;
        emit data_changed();
    }
}

int BingIO::set_combo_box_region() {
    if (_region == "en-UK") {
        return 1;
    }else if (_region == "de-DE") {
        return 2;
    }else if (_region == "en-CA") {
        return 3;
    }else if (_region == "en-AU") {
        return 4;
    }else if (_region == "fr-FR") {
        return 5;
    }else if (_region == "ja-JP") {
        return 6;
    }else if (_region == "zh-CN") {
        return 7;
    }else {
        return 0;
    }
}

QString BingIO::get_region_key(int region) {
    if (region == 1) {
        return "en-UK";
    }else if (region == 2) {
        return "de-DE";
    }else if (region == 3) {
        return "en-CA";
    }else if (region == 4) {
        return "en-AU";
    }else if (region == 5) {
        return "fr-FR";
    }else if (region == 6) {
        return "ja-JP";
    }else if (region == 7) {
        return "zh-CN";
    }else if (region == 0) {
        return "en-US";
    }else {
        return _region;
    }
}

QString BingIO::read_file(QString filename) {
    if (!file_exists(filename)) {
        return "en-US";
    }

    QFile file(filename);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug("Unable to open file");
    }

    QTextStream in(&file);
    QString text = in.readAll();
    file.close();
    return text;
}

void BingIO::write_file(QString filename, QString text) {

    QFile file(filename);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
        qDebug("Unable to open file");
    }

    QTextStream out(&file);
    out << text;
    file.flush();
    file.close();

}

// rotate

void BingIO::set_rotate(ulong rotate) {
    if (_rotate != rotate) {
        _rotate = rotate;
        emit data_changed();
    }
}

ulong BingIO::get_rotate() {
    return _rotate;
}


bool BingIO::get_background_image() {
    return _set_background_image;
}

bool BingIO::get_lock_screen() {
    return _set_lock_screen;
}

QString BingIO::get_wallpaper_info() {
    return _wallpaper_info;
}

void BingIO::set_wallpaper_info() {
    // set wallpaper info
    QString info = _db.selectQuery(_current_wallpaper);

    if (!info.isEmpty()) {
        _wallpaper_info = info;
        emit wallpaper_info_changed();
        return;
    }

    QString xmlText = launch("curl -s http://www.bing.com/HPImageArchive.aspx?format=xml&idx=0&n=1&mkt=" + _region);

    if (!xmlText.isEmpty()) {
        QDomDocument doc;
        doc.setContent(xmlText);
        QDomNodeList list = doc.elementsByTagName("copyright");
        QDomNodeList urlBase = doc.elementsByTagName("url");
        QString image_name = urlBase.at(0).toElement().text();
        QString wall_info = list.at(0).toElement().text();

        if (image_name.contains(_current_wallpaper, Qt::CaseInsensitive)) {
            if (!wall_info.isEmpty()) {
                _wallpaper_info = wall_info;
                emit wallpaper_info_changed();
                _db.insertQuery(_current_wallpaper, _wallpaper_info);
            }
        } else {
            _wallpaper_info = "NO CONTENT";
            emit wallpaper_info_changed();
        }
    }
}

void BingIO::set_background_image(bool image) {
    if (image != _set_background_image) {
        _set_background_image = image;
        emit data_changed();
    }
}

void BingIO::set_lock_screen(bool image) {
    if (image != _set_lock_screen) {
        _set_lock_screen = image;
        emit data_changed();
    }
}

void BingIO::set_days_to_delete_pic(ulong days) {
    if (_delete_days != days) {
        _delete_days = days;
        emit data_changed();
    }
}

ulong BingIO::get_days_to_delete_pic(){
    return _delete_days;
}

QString BingIO::get_next_refresh() {
    return _next_refresh;
}

void BingIO::save_data() {
    // build yaml map
    create_yaml_map();
    write_file(_config, _config_data);
}

void BingIO::create_yaml_map() {
    YAML::Emitter config;
    config << YAML::BeginMap;
    config << YAML::Key << "REGION";
    config << YAML::Value << _region.toStdString();
    config << YAML::Key << "ROTATE";
    config << YAML::Value << _rotate;
    config << YAML::Key << "WALLPAPER_DIR";
    config << YAML::Value << _bing_wall_directory.toStdString();
    config << YAML::Key << "SET_BACKGROUND";
    config << YAML::Value << _set_background_image;
    config << YAML::Key << "SET_LOCKSCREEN";
    config << YAML::Value << _set_lock_screen;
    config << YAML::Key << "SET_DELETE_DAYS";
    config << YAML::Value << _delete_days;
    config << YAML::Key << "REFRESH_TIME";
    config << YAML::Value << _refresh_minutes;
    config << YAML::Key << "CURRENT_WALLPAPER";
    config << YAML::Value << _current_wallpaper.toStdString();
    config << YAML::Key << "DOWNLOAD_SCRIPT";
    config << YAML::Value << _download_script;
    config << YAML::EndMap;

    _config_data = QString::fromStdString(config.c_str());
}

void BingIO::load_config() {
    if (file_exists(_config)) {
        YAML::Node config = YAML::LoadFile(_config.toStdString());
        if (config["REGION"])
            _region = QString::fromStdString(config["REGION"].as<std::string>());
        if (config["ROTATE"])
            _rotate = config["ROTATE"].as<int>();
        if (config["WALLPAPER_DIR"])
            _bing_wall_directory = QString::fromStdString(config["WALLPAPER_DIR"].as<std::string>());
        if (config["SET_BACKGROUND"])
            _set_background_image = config["SET_BACKGROUND"].as<std::string>() == "true" ? true : false;
        if (config["SET_LOCKSCREEN"])
            _set_lock_screen = config["SET_LOCKSCREEN"].as<std::string>() == "true" ? true : false;
        if (config["SET_DELETE_DAYS"])
            _delete_days = config["SET_DELETE_DAYS"].as<int>();
        if (config["REFRESH_TIME"])
            _refresh_milliseconds = create_refresh_milliseconds(config["REFRESH_TIME"].as<int>());
        if (config["CURRENT_WALLPAPER"])
            _current_wallpaper = QString::fromStdString(config["CURRENT_WALLPAPER"].as<std::string>());
        if (config["DOWNLOAD_SCRIPT"])
            _download_script = config["DOWNLOAD_SCRIPT"].as<int>();
    }
}

qint64 BingIO::create_refresh_milliseconds(int time) {
    QDateTime utc = QDateTime::currentDateTimeUtc();
    QDateTime current_time = QDateTime::currentDateTimeUtc();

    utc.setTime(QTime(time, 0, 0)); // set time to the prefered time

    qint64 millisec = utc.time().msecsSinceStartOfDay() - current_time.time().msecsSinceStartOfDay();
    if (millisec < 0) {
        utc = utc.addDays(1);
        millisec = current_time.time().msecsSinceStartOfDay() - utc.time().msecsSinceStartOfDay();
    }
    _refresh_minutes = time;
    _next_refresh = utc.toString("hh:mm AP dd.MM.yyyy t");
    emit refresh_date_changed();
    return millisec;
}

void BingIO::update_next_refresh_date() {
    _refresh_milliseconds = create_refresh_milliseconds(_refresh_minutes);
}

QString BingIO::launch(const QString &program)
{
    m_process->start(program);
    m_process->waitForFinished(-1);
    QByteArray bytes = m_process->readAllStandardOutput();
    QString output = QString::fromLocal8Bit(bytes);
    return output;
}

QString BingIO::run_script() {
    if (!file_exists(_shell_script)) {
        create_shell_script();
    } else if (_force_download - _download_script > 0) {
        create_shell_script();
        _download_script = _force_download;
        save_data();
    }
    QString wallpaper = launch(_shell_script);
    wallpaper.remove(QRegExp("[\"\n]"));
    if (!wallpaper.isEmpty()) {
        _current_wallpaper = wallpaper;

        // get wallpaper info
        set_wallpaper_info();
        save_data();
    }
    return wallpaper;
}

void BingIO::delete_wallpaper() {
    // delete the current wallpaper
    launch("rm -rf " + _bing_wall_directory + "/" + _current_wallpaper);
    _current_wallpaper = "";
    save_data();
}

void BingIO::rotateCmd(ulong millisec) {
    // set rotate from cmd
    set_rotate(millisec);
    save_data();
    run_script();
    if (millisec <= 1000) {
        set_rotate(0);
        save_data();
    }
}

bool BingIO::check_network_connection() {
    bool connection = _network_manager.isOnline();
    return connection;
}

void BingIO::clean_dir(QString days) {
    QString cmd = "find "+ _bing_wall_directory + " -type f -mtime +" + days + " -exec rm {} \;";
    launch(cmd);
}

void BingIO::create_shell_script() {
    launch("curl https://raw.githubusercontent.com/julekgwa/bingwallpaper/script-only/bing.sh --output " + _shell_script);
    launch("chmod +x " + _shell_script);
}

const QString BingIO::version = "2018.05";
