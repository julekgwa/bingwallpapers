// Tests for the non-UI code: Bing's feed, the commands that set the wallpaper,
// and BingIO's refresh flow against a fake Bing server on localhost.

#define private public // BingIO's internals are checked directly
#include "bingio.h"
#undef private
#include "bingfeed.h"
#include "commandlineparser.h"
#include "wallpapersetter.h"

#include <QDir>
#include <QFile>
#include <QMap>
#include <QSignalSpy>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>
#include <QTest>
#include <QTimeZone>

#include <memory>

using namespace WallpaperSetter;

namespace {

const QByteArray kJpeg = QByteArray("\xFF\xD8\xFF\xE0", 4) + QByteArray(2000, 'x');

const char kFeed[] =
        "<?xml version=\"1.0\" encoding=\"utf-8\" ?><images><image>"
        "<startdate>20261112</startdate>"
        "<url>/th?id=OHR.Test_EN-US1_1920x1080.jpg&amp;rf=x_1920x1080.jpg&amp;pid=hp</url>"
        "<urlBase>/th?id=OHR.Test_EN-US1</urlBase>"
        "<copyright>Test title (\xC2\xA9 Test Author)</copyright>"
        "<copyrightlink>https://www.bing.com/search?q=test</copyrightlink>"
        "</image></images>";

const char kUhdPath[] = "/th?id=OHR.Test_EN-US1_UHD.jpg";
const char kPreferredPath[] = "/th?id=OHR.Test_EN-US1_1920x1200.jpg";
const char kFallbackPath[] = "/th?id=OHR.Test_EN-US1_1920x1080.jpg&rf=x_1920x1080.jpg&pid=hp";

// A tiny HTTP server that answers from a table of paths.
class FakeBing : public QObject
{
public:
    struct Response
    {
        int status = 200;
        QByteArray body;
        QByteArray location;
    };

    FakeBing()
    {
        QVERIFY(_server.listen(QHostAddress::LocalHost));
        connect(&_server, &QTcpServer::newConnection, this, [this] {
            while (QTcpSocket *socket = _server.nextPendingConnection())
                connect(socket, &QTcpSocket::readyRead, this, [this, socket] { answer(socket); });
        });
    }

    QUrl baseUrl() const { return QUrl(QString("http://127.0.0.1:%1").arg(_server.serverPort())); }

    void route(const QString &path, int status, const QByteArray &body = {}, const QByteArray &location = {})
    {
        _routes[path] = {status, body, location};
    }

    QStringList requests;

private:
    void answer(QTcpSocket *socket)
    {
        const QByteArray data = socket->readAll();
        const QList<QByteArray> parts = data.left(data.indexOf("\r\n")).split(' ');
        if (parts.size() < 2)
            return;

        QString path = QString::fromLatin1(parts.at(1));
        requests << path;
        // The feed has a query string that varies; match on the path alone.
        const QString key = path.startsWith("/HPImageArchive.aspx") ? "/HPImageArchive.aspx" : path;
        const Response r = _routes.value(key, {404, {}, {}});

        QByteArray out = "HTTP/1.1 " + QByteArray::number(r.status) + " X\r\n";
        if (!r.location.isEmpty())
            out += "Location: " + r.location + "\r\n";
        out += "Content-Length: " + QByteArray::number(r.body.size()) + "\r\nConnection: close\r\n\r\n" + r.body;
        socket->write(out);
        socket->disconnectFromHost();
    }

    QTcpServer _server;
    QMap<QString, Response> _routes;
};

} // namespace

class TestBingWallpapers : public QObject
{
    Q_OBJECT

    // Every test gets its own empty folders, so nothing outside them is touched.
    std::unique_ptr<QTemporaryDir> root;
    QString appDir;
    QString picturesDir;

    static void touch(const QString &path, const QByteArray &data = "x")
    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::WriteOnly));
        f.write(data);
    }

    static void age(const QString &path, int days)
    {
        QFile f(path);
        QVERIFY(f.open(QIODevice::ReadWrite));
        f.setFileTime(QDateTime::currentDateTime().addDays(-days), QFileDevice::FileModificationTime);
    }

private slots:
    void init()
    {
        root = std::make_unique<QTemporaryDir>();
        QVERIFY(root->isValid());
        appDir = root->path() + "/app";
        picturesDir = root->path() + "/pictures";
        qunsetenv("XDG_CURRENT_DESKTOP");
    }

    // ------------------------------------------------------------------ feed

    void feedParse()
    {
        const BingFeedEntry e = BingFeedEntry::parse(kFeed);
        QVERIFY(e.isValid());
        QCOMPARE(e.copyright, QString::fromUtf8("Test title (\xC2\xA9 Test Author)"));
    }

    void feedCandidatesAreBestFirst()
    {
        const BingFeedEntry e = BingFeedEntry::parse(kFeed);
        const QUrl base("https://www.bing.com");

        const auto uhd = e.candidates(base, true);
        QCOMPARE(uhd.size(), 3);
        QCOMPARE(uhd[0].name, QString("Test_EN-US1_UHD.jpg"));
        QCOMPARE(uhd[0].url.toString(), QString("https://www.bing.com/th?id=OHR.Test_EN-US1_UHD.jpg"));
        QCOMPARE(uhd[1].name, QString("Test_EN-US1_1920x1200.jpg"));
        QCOMPARE(uhd[1].url.toString(), QString("https://www.bing.com/th?id=OHR.Test_EN-US1_1920x1200.jpg"));
        QCOMPARE(uhd[2].name, QString("Test_EN-US1_1920x1080.jpg"));   // the feed's own default
        QVERIFY(uhd[2].isDefault);
        QVERIFY(!uhd[0].isDefault && !uhd[1].isDefault);
        QCOMPARE(uhd[2].url.toString(),
                 QString("https://www.bing.com/th?id=OHR.Test_EN-US1_1920x1080.jpg&rf=x_1920x1080.jpg&pid=hp"));

        const auto hd = e.candidates(base, false);                      // no 4K
        QCOMPARE(hd.size(), 2);
        QCOMPARE(hd[0].name, QString("Test_EN-US1_1920x1200.jpg"));
    }

    void feedParseRejectsGarbage()
    {
        QVERIFY(!BingFeedEntry::parse("").isValid());
        QVERIFY(!BingFeedEntry::parse("<html>nope</html>").isValid());
        BingFeedEntry::parse("<images><image><url>");           // truncated XML must not crash
    }

    void feedNamesAreSanitised_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<bool>("safe");
        QTest::newRow("plain") << "Name_EN-US1_1920x1200.jpg" << true;
        QTest::newRow("png") << "a.png" << true;
        QTest::newRow("traversal") << "../evil.jpg" << false;
        QTest::newRow("embedded traversal") << "a..b.jpg" << false;
        QTest::newRow("slash") << "dir/evil.jpg" << false;
        QTest::newRow("hidden") << ".hidden.jpg" << false;
        QTest::newRow("wrong type") << "script.sh" << false;
        QTest::newRow("empty") << "" << false;
        QTest::newRow("space") << "a b.jpg" << false;
    }
    void feedNamesAreSanitised()
    {
        QFETCH(QString, name);
        QFETCH(bool, safe);
        QCOMPARE(BingFeedEntry::isSafeFileName(name), safe);
    }

    void feedUnsafeIdGivesNoName()
    {
        BingFeedEntry e;
        e.urlBase = "/th?id=OHR.../../../etc/x";
        e.url = "/th?id=OHR.a/b_1920x1080.jpg";
        QVERIFY(e.candidates(QUrl("https://www.bing.com"), true).isEmpty());
    }

    // ---------------------------------------------------------- set wallpaper

    void detectDesktops_data()
    {
        QTest::addColumn<QString>("xdg");
        QTest::addColumn<int>("desktop");
        QTest::newRow("gnome") << "GNOME" << int(Desktop::Gnome);
        QTest::newRow("ubuntu") << "ubuntu:GNOME" << int(Desktop::Gnome);
        QTest::newRow("kde") << "KDE" << int(Desktop::Kde);
        QTest::newRow("cinnamon") << "X-Cinnamon" << int(Desktop::Cinnamon);
        QTest::newRow("mate") << "MATE" << int(Desktop::Mate);
        QTest::newRow("budgie") << "Budgie:GNOME" << int(Desktop::Gnome);
        QTest::newRow("xfce") << "XFCE" << int(Desktop::Unknown);
        QTest::newRow("empty") << "" << int(Desktop::Unknown);
    }
    void detectDesktops()
    {
        QFETCH(QString, xdg);
        QFETCH(int, desktop);
        QCOMPARE(int(detectDesktop(xdg)), desktop);
    }

    void gvariantQuoting()
    {
        QCOMPARE(gvariantString("zoom"), QString("'zoom'"));
        QCOMPARE(gvariantString("it's"), QString("'it\\'s'"));
        QCOMPARE(gvariantString("a\\b"), QString("'a\\\\b'"));
    }

    void gnomeCommands()
    {
        const auto cmds = commands(Desktop::Gnome, "/home/u/My Pics/a b.jpg", true, true);
        QCOMPARE(cmds.size(), 8);
        QCOMPARE(cmds[0].program, QString("gsettings"));
        // Reset first, so setting the same picture again is still a change.
        QCOMPARE(cmds[0].args, QStringList({"reset", "org.gnome.desktop.background", "picture-uri"}));
        QVERIFY(cmds[0].optional);
        QCOMPARE(cmds[2].args, QStringList({"set", "org.gnome.desktop.background", "picture-uri",
                                            "'file:///home/u/My%20Pics/a%20b.jpg'"}));
        QVERIFY(!cmds[2].optional);
        QCOMPARE(cmds[3].args.at(2), QString("picture-uri-dark"));
        QVERIFY(cmds[3].optional);           // missing before GNOME 42
        QCOMPARE(cmds[5].args, QStringList({"reset", "org.gnome.desktop.screensaver", "picture-uri"}));
        QCOMPARE(cmds[6].args.at(1), QString("org.gnome.desktop.screensaver"));
    }

    void commandsRespectWhatWasAsked()
    {
        QCOMPARE(commands(Desktop::Gnome, "/a.jpg", true, false).size(), 5);
        QCOMPARE(commands(Desktop::Gnome, "/a.jpg", false, true).size(), 3);
        QVERIFY(commands(Desktop::Gnome, "/a.jpg", false, false).isEmpty());
        QVERIFY(commands(Desktop::Unknown, "/a.jpg", true, true).isEmpty());
        QVERIFY(commands(Desktop::Kde, "/a.jpg", false, true).isEmpty());     // lock screen unsupported
        QCOMPARE(commands(Desktop::Kde, "/a b.jpg", true, false).at(0).args, QStringList({"/a b.jpg"}));
        QCOMPARE(commands(Desktop::Mate, "/a b.jpg", true, false).at(1).args.at(3), QString("'/a b.jpg'"));
    }

    // ----------------------------------------------------------- BingIO basics

    void refreshTime()
    {
        const QDateTime ten(QDate(2026, 11, 13), QTime(10, 0), QTimeZone::UTC);
        QCOMPARE(ten.msecsTo(BingIO::nextRefreshTime(ten, 8)), qint64(22 * 3600 * 1000)); // was 2h
        const QDateTime six(QDate(2026, 11, 13), QTime(6, 0), QTimeZone::UTC);
        QCOMPARE(six.msecsTo(BingIO::nextRefreshTime(six, 8)), qint64(2 * 3600 * 1000));
        const QDateTime eight(QDate(2026, 11, 13), QTime(8, 0), QTimeZone::UTC);
        QCOMPARE(BingIO::nextRefreshTime(eight, 8).date(), QDate(2026, 11, 14));
    }

    void settingsPersist()
    {
        {
            BingIO io(appDir, picturesDir, nullptr);
            QVERIFY(QFile::exists(appDir + "/config.yaml"));
            QCOMPARE(io.wallpaperDir(), picturesDir);
            QVERIFY(QDir(picturesDir).exists());

            QDir().mkpath(appDir + "/pics with space");
            io.setRegion("en-UK");
            QCOMPARE(io.region(), QString("en-GB"));
            io.setRotateMinutes(1);
            QCOMPARE(io.rotateMinutes(), 3);          // clamped to the minimum
            io.setRotateMinutes(45);
            QVERIFY(io.ultraHd());                    // on by default
            io.setUltraHd(false);
            io.setDeleteDays(14);
            io.setSetLockScreen(true);
            io.setSetBackground(false);
            io.setWallpaperDir(appDir + "/pics with space/");
            QCOMPARE(io.wallpaperDir(), appDir + "/pics with space");
        } // destructor flushes the debounced save

        BingIO io(appDir, picturesDir, nullptr);
        QCOMPARE(io.region(), QString("en-GB"));
        QCOMPARE(io.rotateMinutes(), 45);
        QCOMPARE(io.deleteDays(), 14);
        QVERIFY(!io.ultraHd());
        QVERIFY(io.setLockScreen());
        QVERIFY(!io.setBackground());
        QCOMPARE(io.wallpaperDir(), appDir + "/pics with space");
    }

    void malformedConfigFallsBackToDefaults()
    {
        { BingIO io(appDir, picturesDir, nullptr); }
        QFile f(appDir + "/config.yaml");
        QVERIFY(f.open(QIODevice::WriteOnly | QIODevice::Truncate));
        f.write("REGION: [unclosed\nROTATE: abc\n");
        f.close();

        BingIO io(appDir, picturesDir, nullptr);
        QCOMPARE(io.region(), QString("en-US"));
        QCOMPARE(io.rotateMinutes(), 0);
    }

    void oldHelperScriptIsRemoved()
    {
        QDir().mkpath(appDir);
        touch(appDir + "/bing.sh");
        touch(appDir + "/config.sh");
        BingIO io(appDir, picturesDir, nullptr);
        QVERIFY(!QFile::exists(appDir + "/bing.sh"));
        QVERIFY(!QFile::exists(appDir + "/config.sh"));
    }

    void dirStates()
    {
        BingIO io(appDir, picturesDir, nullptr);
        QCOMPARE(io.dirState(picturesDir), int(BingIO::DirValid));
        QCOMPARE(io.dirState(picturesDir + "/missing"), int(BingIO::DirMissing));
        QCOMPARE(io.dirState(""), int(BingIO::DirMissing));
    }

    void titleAndCopyrightSplit()
    {
        BingIO io(appDir, picturesDir, nullptr);
        io.applyInfo(QString::fromUtf8("Fjord at dusk, Norway (\xC2\xA9 Dag Sundberg/Getty Images)"));
        QCOMPARE(io.wallpaperTitle(), QString("Fjord at dusk, Norway"));
        QCOMPARE(io.wallpaperCopyright(), QString::fromUtf8("\xC2\xA9 Dag Sundberg/Getty Images"));
        io.applyInfo("Plain text");
        QCOMPARE(io.wallpaperTitle(), QString("Plain text"));
        QVERIFY(io.wallpaperCopyright().isEmpty());
        io.applyInfo(QString::fromUtf8("Peak (Mt. Cook) at dawn (\xC2\xA9 Someone)"));
        QCOMPARE(io.wallpaperTitle(), QString("Peak (Mt. Cook) at dawn"));
    }

    // ---------------------------------------------------------------- cleaning

    void deleteWithNoWallpaperKeepsTheFolder()
    {
        BingIO io(appDir, picturesDir, nullptr);
        touch(io.wallpaperDir() + "/keep.jpg");
        QVERIFY(!io.deleteCurrentWallpaper());          // used to rm -rf the folder
        QVERIFY(QFile::exists(io.wallpaperDir() + "/keep.jpg"));
    }

    void deleteStaysInsideTheFolder()
    {
        BingIO io(appDir, picturesDir, nullptr);
        touch(io.wallpaperDir() + "/keep.jpg");
        io._current_wallpaper = "../../keep.jpg";
        QVERIFY(io.deleteCurrentWallpaper());
        QVERIFY(!QFile::exists(io.wallpaperDir() + "/keep.jpg"));
    }

    void cleanOnlyOldImagesNeverTheCurrentOne()
    {
        BingIO io(appDir, picturesDir, nullptr);
        const QString d = io.wallpaperDir();
        for (const char *n : {"old.jpg", "old.png", "current.jpg", "new.jpg", "notes.txt"})
            touch(d + "/" + n);
        QDir().mkpath(d + "/sub");
        touch(d + "/sub/old.jpg");
        for (const char *n : {"old.jpg", "old.png", "current.jpg", "notes.txt"})
            age(d + "/" + n, 10);
        age(d + "/sub/old.jpg", 10);
        io._current_wallpaper = "current.jpg";

        QCOMPARE(io.cleanDir(7), 2);
        QVERIFY(!QFile::exists(d + "/old.jpg"));
        QVERIFY(!QFile::exists(d + "/old.png"));
        QVERIFY(QFile::exists(d + "/current.jpg"));    // in use
        QVERIFY(QFile::exists(d + "/new.jpg"));        // recent
        QVERIFY(QFile::exists(d + "/notes.txt"));      // not a picture
        QVERIFY(QFile::exists(d + "/sub/old.jpg"));    // sub folders are left alone
    }

    // ------------------------------------------------------------ command line

    void cliFourKSwitchesAreSaved_data()
    {
        QTest::addColumn<QString>("value");
        QTest::addColumn<bool>("expected");
        QTest::newRow("off") << "off" << false;
        QTest::newRow("OFF") << "OFF" << false;
        QTest::newRow("no") << "no" << false;
        QTest::newRow("0") << "0" << false;
        QTest::newRow("on") << "on" << true;
        QTest::newRow("true") << "true" << true;
    }
    void cliFourKSwitchesAreSaved()
    {
        QFETCH(QString, value);
        QFETCH(bool, expected);
        {
            BingIO io(appDir, picturesDir, nullptr);
            io.setUltraHd(!expected);                       // start from the opposite
        }
        {
            BingIO io(appDir, picturesDir, nullptr);
            CommandLineParser cmd({"bingwallpapers", "--4k", value});
            QVERIFY(cmd.isCommand());                        // runs without the GUI
            QCOMPARE(cmd.process(&io), 0);
            QCOMPARE(io.ultraHd(), expected);
        }                                                    // closing flushes the saved setting
        BingIO again(appDir, picturesDir, nullptr);
        QCOMPARE(again.ultraHd(), expected);
    }

    void cliFourKRejectsOtherValues()
    {
        BingIO io(appDir, picturesDir, nullptr);
        CommandLineParser cmd({"bingwallpapers", "--4k", "maybe"});
        QVERIFY(cmd.isCommand());
        QCOMPARE(cmd.process(&io), 2);
        QVERIFY(io.ultraHd());                               // unchanged (default is on)
    }

    void cliWithoutFourKLeavesTheSettingAlone()
    {
        BingIO io(appDir, picturesDir, nullptr);
        io.setUltraHd(false);
        CommandLineParser cmd({"bingwallpapers", "--clean", "30"});
        QVERIFY(cmd.isCommand());
        QCOMPARE(cmd.process(&io), 0);
        QVERIFY(!io.ultraHd());
    }

    void cliRecognisesCommandsAndTheGui()
    {
        QVERIFY(!CommandLineParser({"bingwallpapers"}).isCommand());
        QVERIFY(!CommandLineParser({"bingwallpapers", "--background"}).isCommand());
        QVERIFY(CommandLineParser({"bingwallpapers", "--background"}).startHidden());
        QVERIFY(!CommandLineParser({"bingwallpapers", "-platform", "offscreen"}).isCommand()); // Qt's own options
        QVERIFY(!CommandLineParser({"bingwallpapers", "-style", "fusion"}).isCommand());
        QVERIFY(CommandLineParser({"bingwallpapers", "-s"}).isCommand());
        QVERIFY(CommandLineParser({"bingwallpapers", "-r", "1", "30"}).isCommand());
        QVERIFY(CommandLineParser({"bingwallpapers", "--set", "--4k", "off"}).isCommand());
    }

    // ----------------------------------------------------------------- refresh

    // Runs one refresh against `bing` and returns the signal arguments.
    QVariantList refreshOnce(BingIO &io, FakeBing &bing)
    {
        io.setBaseUrl(bing.baseUrl());
        io._online = true;
        io.setSetBackground(false);
        QSignalSpy spy(&io, &BingIO::refreshFinished);
        io.refresh();
        if (!spy.wait(5000))
            return {};
        return spy.at(0);
    }

    void refreshDownloadsUltraHdFirst()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);
        bing.route(kUhdPath, 200, kJpeg);

        BingIO io(appDir, picturesDir, nullptr);
        const QVariantList result = refreshOnce(io, bing);

        QCOMPARE(result.value(0).toBool(), true);
        QCOMPARE(result.value(1).toString(), QString("Wallpaper updated"));
        QVERIFY(!io.busy());
        QVERIFY(QFile::exists(io.wallpaperDir() + "/Test_EN-US1_UHD.jpg"));
        QCOMPARE(io.savedImages().size(), 1);
        QVERIFY(!bing.requests.contains(kPreferredPath));              // 4K worked, so no second request
        QCOMPARE(io.wallpaperTitle(), QString("Test title"));
        QCOMPARE(io.wallpaperCopyright(), QString::fromUtf8("\xC2\xA9 Test Author"));
        QVERIFY(io.wallpaperUrl().isLocalFile());
    }

    void refreshUsesFullHdWhenThereIsNoUltraHd()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);
        bing.route(kUhdPath, 404);
        bing.route(kPreferredPath, 200, kJpeg);

        BingIO io(appDir, picturesDir, nullptr);
        QCOMPARE(refreshOnce(io, bing).value(0).toBool(), true);
        QVERIFY(QFile::exists(io.wallpaperDir() + "/Test_EN-US1_1920x1200.jpg"));
        QVERIFY(!QFile::exists(io.wallpaperDir() + "/Test_EN-US1_UHD.jpg"));
    }

    void refreshSkipsUltraHdWhenSwitchedOff()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);
        bing.route(kUhdPath, 200, kJpeg);
        bing.route(kPreferredPath, 200, kJpeg);

        BingIO io(appDir, picturesDir, nullptr);
        io.setUltraHd(false);
        QCOMPARE(refreshOnce(io, bing).value(0).toBool(), true);
        QVERIFY(QFile::exists(io.wallpaperDir() + "/Test_EN-US1_1920x1200.jpg"));
        QVERIFY(!bing.requests.contains(kUhdPath));                     // never even asked
    }

    void refreshFallsBackToTheFeedDefault()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);
        bing.route(kUhdPath, 404);
        bing.route(kPreferredPath, 302, {}, "/somewhere-else.jpg");     // a redirect also means "not here"
        bing.route(kFallbackPath, 200, kJpeg);

        BingIO io(appDir, picturesDir, nullptr);
        QCOMPARE(refreshOnce(io, bing).value(0).toBool(), true);
        QVERIFY(QFile::exists(io.wallpaperDir() + "/Test_EN-US1_1920x1080.jpg"));
        QVERIFY(!bing.requests.contains("/somewhere-else.jpg"));        // not followed for the sized versions
    }

    void refreshRejectsAnErrorPageSavedAsAnImage()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);
        bing.route(kUhdPath, 200, "<html>Not found</html>");
        bing.route(kPreferredPath, 200, "<html>Not found</html>");
        bing.route(kFallbackPath, 404, "<html>Not found</html>");

        BingIO io(appDir, picturesDir, nullptr);
        const QVariantList result = refreshOnce(io, bing);

        QCOMPARE(result.value(0).toBool(), false);
        QCOMPARE(result.value(1).toString(), QString("Couldn't download today's wallpaper"));
        QVERIFY(io.savedImages().isEmpty());
        QVERIFY(!io.busy());
    }

    void refreshWhenAlreadyDownloadedDoesNotDownloadAgain()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);

        BingIO io(appDir, picturesDir, nullptr);
        touch(io.wallpaperDir() + "/Test_EN-US1_UHD.jpg", kJpeg);      // the best version is already here
        const QVariantList result = refreshOnce(io, bing);

        QCOMPARE(result.value(0).toBool(), true);
        QCOMPARE(result.value(1).toString(), QString("Wallpaper is up to date"));
        QCOMPARE(bing.requests.size(), 1);                              // only the feed
        QCOMPARE(io.wallpaperTitle(), QString("Test title"));
    }

    // Switching 4K on and pressing Refresh upgrades today's picture.
    void refreshUpgradesToUltraHdWhenSwitchedOn()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);
        bing.route(kUhdPath, 200, kJpeg);

        BingIO io(appDir, picturesDir, nullptr);
        touch(io.wallpaperDir() + "/Test_EN-US1_1920x1200.jpg", kJpeg);
        touch(io.wallpaperDir() + "/other-day.jpg", kJpeg);            // unrelated, must survive
        const QVariantList result = refreshOnce(io, bing);

        QCOMPARE(result.value(1).toString(), QString("Wallpaper updated"));
        QVERIFY(QFile::exists(io.wallpaperDir() + "/Test_EN-US1_UHD.jpg"));
        QVERIFY(!QFile::exists(io.wallpaperDir() + "/Test_EN-US1_1920x1200.jpg"));   // one copy only
        QVERIFY(QFile::exists(io.wallpaperDir() + "/other-day.jpg"));
        QCOMPARE(io._current_wallpaper, QString("Test_EN-US1_UHD.jpg"));
    }

    // The picture's title must not disappear when the same picture is saved
    // under another name (a different size).
    void titleSurvivesSwitchingQuality()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);
        bing.route(kUhdPath, 200, kJpeg);
        bing.route(kPreferredPath, 200, kJpeg);

        BingIO io(appDir, picturesDir, nullptr);
        QCOMPARE(refreshOnce(io, bing).value(0).toBool(), true);
        QCOMPARE(io.wallpaperTitle(), QString("Test title"));          // 4K: Test_EN-US1_UHD.jpg

        io.setUltraHd(false);
        QCOMPARE(refreshOnce(io, bing).value(0).toBool(), true);
        QVERIFY(QFile::exists(io.wallpaperDir() + "/Test_EN-US1_1920x1200.jpg"));
        QCOMPARE(io.wallpaperTitle(), QString("Test title"));          // Full HD: another name, same title
        QCOMPARE(io.wallpaperCopyright(), QString::fromUtf8("\xC2\xA9 Test Author"));
    }

    // A database written by an older version keeps working.
    void oldDatabaseIsMigrated()
    {
        QDir().mkpath(appDir);
        {
            QSqlDatabase old = QSqlDatabase::addDatabase("QSQLITE", "legacy");
            old.setDatabaseName(appDir + "/bingwallpaper.db");
            QVERIFY(old.open());
            QSqlQuery q(old);
            QVERIFY(q.exec("CREATE TABLE bingwallpapers(id INTEGER PRIMARY KEY AUTOINCREMENT, image_name TEXT NOT NULL, "
                           "description TEXT NOT NULL, CONSTRAINT description_unique UNIQUE (description), "
                           "CONSTRAINT image_name_unique UNIQUE (image_name))"));
            QVERIFY(q.exec(QString::fromUtf8("INSERT INTO bingwallpapers (image_name, description) "
                                             "VALUES ('Old_EN-US1_1920x1200.jpg', 'Old title (\xC2\xA9 Old Author)')")));
            old.close();
        }
        QSqlDatabase::removeDatabase("legacy");

        QFile config(appDir + "/config.yaml");
        QVERIFY(config.open(QIODevice::WriteOnly));
        config.write("CURRENT_WALLPAPER: Old_EN-US1_1920x1200.jpg\n");
        config.close();

        BingIO io(appDir, picturesDir, nullptr);
        QCOMPARE(io.wallpaperTitle(), QString("Old title"));
        QCOMPARE(io.wallpaperCopyright(), QString::fromUtf8("\xC2\xA9 Old Author"));
    }

    void refreshKeepsFullHdWhenUltraHdIsNotAvailable()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);
        bing.route(kUhdPath, 404);

        BingIO io(appDir, picturesDir, nullptr);
        touch(io.wallpaperDir() + "/Test_EN-US1_1920x1200.jpg", kJpeg);
        const QVariantList result = refreshOnce(io, bing);

        QCOMPARE(result.value(0).toBool(), true);
        QCOMPARE(result.value(1).toString(), QString("Wallpaper is up to date"));
        QVERIFY(QFile::exists(io.wallpaperDir() + "/Test_EN-US1_1920x1200.jpg"));
        QVERIFY(!bing.requests.contains(kPreferredPath));               // no needless download
    }

    // Switching 4K off and pressing Refresh goes back to Full HD.
    void refreshSwitchesBackToFullHdWhenUltraHdIsSwitchedOff()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);
        bing.route(kUhdPath, 200, kJpeg);
        bing.route(kPreferredPath, 200, kJpeg);

        BingIO io(appDir, picturesDir, nullptr);
        io.setUltraHd(false);
        touch(io.wallpaperDir() + "/Test_EN-US1_UHD.jpg", kJpeg);
        const QVariantList result = refreshOnce(io, bing);

        QCOMPARE(result.value(1).toString(), QString("Wallpaper updated"));
        QVERIFY(QFile::exists(io.wallpaperDir() + "/Test_EN-US1_1920x1200.jpg"));
        QVERIFY(!QFile::exists(io.wallpaperDir() + "/Test_EN-US1_UHD.jpg"));
        QVERIFY(!bing.requests.contains(kUhdPath));
    }

    void refreshWhileRotatingPicksAnotherSavedPicture()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);

        BingIO io(appDir, picturesDir, nullptr);
        io.setBaseUrl(bing.baseUrl());
        io._online = true;
        io.setSetBackground(false);
        io.setRotateMinutes(30);
        touch(io.wallpaperDir() + "/Test_EN-US1_1920x1200.jpg", kJpeg);  // today's
        touch(io.wallpaperDir() + "/other.jpg", kJpeg);
        io._current_wallpaper = "Test_EN-US1_1920x1200.jpg";
        QSignalSpy spy(&io, &BingIO::refreshFinished);
        io.refresh();
        QVERIFY(spy.wait(5000));

        QCOMPARE(io._current_wallpaper, QString("other.jpg"));
    }

    void refreshFallsBackToASavedPictureWhenBingIsDown()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 500);

        BingIO io(appDir, picturesDir, nullptr);
        io.setBaseUrl(bing.baseUrl());
        io._online = true;
        io.setSetBackground(false);

        QSignalSpy none(&io, &BingIO::refreshFinished);
        io.refresh();
        QVERIFY(none.wait(5000));
        QCOMPARE(none.at(0).at(0).toBool(), false);
        QCOMPARE(none.at(0).at(1).toString(), QString("Couldn't reach Bing"));

        touch(io.wallpaperDir() + "/saved.jpg", kJpeg);
        QSignalSpy saved(&io, &BingIO::refreshFinished);
        io.refresh();
        QVERIFY(saved.wait(5000));
        QCOMPARE(saved.at(0).at(0).toBool(), true);
        QCOMPARE(io._current_wallpaper, QString("saved.jpg"));
    }

    void refreshNeverWritesOutsideTheFolder()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200,
                   "<images><image><url>/th?id=OHR.x/../../evil.jpg</url>"
                   "<urlBase>/th?id=OHR.../../evil</urlBase><copyright>c</copyright></image></images>");

        BingIO io(appDir, picturesDir, nullptr);
        io.setBaseUrl(bing.baseUrl());
        io._online = true;
        io.setSetBackground(false);
        QSignalSpy spy(&io, &BingIO::refreshFinished);
        io.refresh();
        QVERIFY(spy.wait(5000));

        QCOMPARE(spy.at(0).at(0).toBool(), false);
        QCOMPARE(bing.requests.size(), 1);                              // nothing was downloaded
    }

    void manualRefreshIsRefusedWhileOffline()
    {
        BingIO io(appDir, picturesDir, nullptr);
        io._online = false;
        QSignalSpy spy(&io, &BingIO::refreshFinished);
        io.refresh(true);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toBool(), false);
        QVERIFY(!io.busy());
    }

    void rotateAppliesADifferentSavedPicture()
    {
        BingIO io(appDir, picturesDir, nullptr);
        io.setSetBackground(false);
        QSignalSpy spy(&io, &BingIO::refreshFinished);

        io.rotate();                                       // nothing saved yet
        QCOMPARE(spy.at(0).at(0).toBool(), false);

        touch(io.wallpaperDir() + "/a.jpg", kJpeg);
        touch(io.wallpaperDir() + "/b.jpg", kJpeg);
        io._current_wallpaper = "a.jpg";
        io.rotate();
        QCOMPARE(spy.at(1).at(0).toBool(), true);
        QCOMPARE(io._current_wallpaper, QString("b.jpg"));
    }

    // -------------------------------------------------------- setting the desktop

    void unsupportedDesktopStillKeepsThePicture()
    {
        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);
        bing.route(kPreferredPath, 200, kJpeg);
        qputenv("XDG_CURRENT_DESKTOP", "XFCE");

        BingIO io(appDir, picturesDir, nullptr);
        io.setBaseUrl(bing.baseUrl());
        io._online = true;
        QSignalSpy spy(&io, &BingIO::refreshFinished);
        io.refresh();
        QVERIFY(spy.wait(5000));

        QCOMPARE(spy.at(0).at(0).toBool(), false);
        QVERIFY(spy.at(0).at(1).toString().contains("can't be set automatically"));
        QVERIFY(QFile::exists(io.wallpaperDir() + "/Test_EN-US1_1920x1200.jpg"));
    }

    void gsettingsIsCalledWithTheRightArguments()
    {
        QTemporaryDir bin;
        QVERIFY(bin.isValid());
        const QString log = bin.path() + "/calls.log";
        // A stand-in gsettings: records its arguments, and lacks picture-uri-dark.
        QFile script(bin.path() + "/gsettings");
        QVERIFY(script.open(QIODevice::WriteOnly));
        script.write(("#!/bin/sh\necho \"$@\" >> '" + log + "'\n"
                      "[ \"$3\" = picture-uri-dark ] && exit 1\nexit 0\n").toUtf8());
        script.close();
        script.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

        const QByteArray oldPath = qgetenv("PATH");
        qputenv("PATH", bin.path().toUtf8() + ":" + oldPath);
        qputenv("XDG_CURRENT_DESKTOP", "ubuntu:GNOME");

        FakeBing bing;
        bing.route("/HPImageArchive.aspx", 200, kFeed);
        bing.route(kPreferredPath, 200, kJpeg);

        BingIO io(appDir, picturesDir, nullptr);
        io.setBaseUrl(bing.baseUrl());
        io._online = true;
        io.setSetLockScreen(true);
        QSignalSpy spy(&io, &BingIO::refreshFinished);
        io.refresh();
        QVERIFY(spy.wait(10000));
        qputenv("PATH", oldPath);

        QCOMPARE(spy.at(0).at(0).toBool(), true);               // the optional key failing is fine
        QFile f(log);
        QVERIFY(f.open(QIODevice::ReadOnly));
        const QStringList calls = QString::fromUtf8(f.readAll()).split('\n', Qt::SkipEmptyParts);
        QCOMPARE(calls.size(), 8);
        const QString uri = "'file://" + QUrl::fromLocalFile(io.wallpaperDir()).path() + "/Test_EN-US1_1920x1200.jpg'";
        QCOMPARE(calls[0], QString("reset org.gnome.desktop.background picture-uri"));
        QCOMPARE(calls[2], "set org.gnome.desktop.background picture-uri " + uri);
        QCOMPARE(calls[4], QString("set org.gnome.desktop.background picture-options 'zoom'"));
        QCOMPARE(calls[6], "set org.gnome.desktop.screensaver picture-uri " + uri);
    }

    void aFailingGsettingsReportsAnError()
    {
        QTemporaryDir bin;
        QFile script(bin.path() + "/gsettings");
        QVERIFY(script.open(QIODevice::WriteOnly));
        script.write("#!/bin/sh\nexit 1\n");
        script.close();
        script.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

        const QByteArray oldPath = qgetenv("PATH");
        qputenv("PATH", bin.path().toUtf8() + ":" + oldPath);
        qputenv("XDG_CURRENT_DESKTOP", "GNOME");

        BingIO io(appDir, picturesDir, nullptr);
        touch(io.wallpaperDir() + "/a.jpg", kJpeg);
        QSignalSpy spy(&io, &BingIO::refreshFinished);
        io.rotate();
        QVERIFY(spy.wait(10000));
        qputenv("PATH", oldPath);

        QCOMPARE(spy.at(0).at(0).toBool(), false);
        QVERIFY(spy.at(0).at(1).toString().contains("gsettings"));
        QVERIFY(!io.busy());
    }
};

QTEST_MAIN(TestBingWallpapers)
#include "tst_bingwallpapers.moc"
