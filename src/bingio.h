#ifndef BINGIO_H
#define BINGIO_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QVariant>
#include <yaml-cpp/yaml.h>
#include <QProcess>
#include <QtCore/QDateTime>
#include <QNetworkConfigurationManager>

class BingIO : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString get_region READ get_region WRITE set_region NOTIFY data_changed)
    Q_PROPERTY(ulong get_rotate READ get_rotate WRITE set_rotate NOTIFY data_changed)
    Q_PROPERTY(ulong get_days_to_delete_pic READ get_days_to_delete_pic WRITE set_days_to_delete_pic NOTIFY data_changed)
    Q_PROPERTY(bool get_background_image READ get_background_image WRITE set_background_image NOTIFY data_changed)
    Q_PROPERTY(bool get_lock_screen READ get_lock_screen WRITE set_lock_screen NOTIFY data_changed)
    Q_PROPERTY(QString get_bing_wall_directory READ get_bing_wall_directory WRITE set_bing_wall_directory NOTIFY data_changed)
    Q_PROPERTY(QString get_app_directory READ get_app_directory)
    Q_PROPERTY(qint64 get_refresh_milliseconds READ get_refresh_milliseconds)
    Q_PROPERTY(QString get_next_refresh READ get_next_refresh NOTIFY refresh_date_changed)
    Q_PROPERTY(QString version MEMBER version CONSTANT)

public:
    explicit BingIO(QObject *parent = 0);
    Q_INVOKABLE QString launch(const QString &program);
    QString get_region(void);
    QString get_app_directory(void);
    QString get_bing_wall_directory();
    ulong get_rotate(void);
    ulong get_days_to_delete_pic(void);
    bool get_background_image();
    bool get_lock_screen();
    qint64 get_refresh_milliseconds();
    QString get_next_refresh();
    static const QString version;

signals:
    void data_changed();
    void refresh_date_changed();

public slots:
    void set_region(QString rgn);
    void set_bing_wall_directory(QString directory);
    void set_rotate(ulong rotate);
    void set_background_image(bool image);
    void set_lock_screen(bool image);
    void save_data();
    void set_days_to_delete_pic(ulong test);
    void update_next_refresh_date();
    void delete_wallpaper();
    void rotateCmd(ulong millisec);
    void clean_dir(QString days);
    QString get_region_key(int region);
    QString run_script();
    bool dir_exists(QString dir);
    bool dir_read_write(QString dir);
    int set_combo_box_region(void);
    bool check_network_connection();

private:
    QString _app_directory;
    QString _config;
    QString _region;
    QString _bing_wall_directory;
    QString _shell_script;
    QString _next_refresh;
    QString _current_wallpaper;
    QString read_file(QString filename);
    void write_file(QString filename, QString text);
    bool file_exists(QString filename);
    bool _set_background_image;
    bool _set_lock_screen;
    unsigned long int _rotate;
    unsigned long int _delete_days;
    void create_yaml_map();
    void load_config();
    void create_shell_script();
    qint64 create_refresh_milliseconds(int time);
    QString _config_data;
    QProcess *m_process;
    qint64 _refresh_milliseconds;
    int _force_download;
    int _download_script;
    int _refresh_minutes;
    QNetworkConfigurationManager _network_manager;

};

#endif // BINGIO_H
