#include "wallpapersetter.h"

#include <QUrl>

namespace WallpaperSetter {

Desktop detectDesktop(const QString &xdgCurrentDesktop)
{
    const QStringList tokens = xdgCurrentDesktop.toLower().split(':', Qt::SkipEmptyParts);
    auto has = [&](const char *name) { return tokens.contains(QLatin1String(name)); };

    if (has("x-cinnamon") || has("cinnamon"))
        return Desktop::Cinnamon;
    if (has("mate"))
        return Desktop::Mate;
    if (has("kde"))
        return Desktop::Kde;
    if (has("gnome") || has("unity") || has("budgie") || has("pantheon") || has("ubuntu"))
        return Desktop::Gnome;
    return Desktop::Unknown;
}

QString gvariantString(const QString &text)
{
    QString escaped = text;
    escaped.replace('\\', "\\\\");
    escaped.replace('\'', "\\'");
    return '\'' + escaped + '\'';
}

QList<Command> commands(Desktop desktop, const QString &imagePath, bool background, bool lockScreen)
{
    QList<Command> list;
    auto set = [&](const char *schema, const char *key, const QString &value, bool optional = false) {
        list.append({"gsettings", {"set", schema, key, value}, optional});
    };
    // Setting a key to the value it already has changes nothing, so the desktop
    // would not reload a picture that was deleted and downloaded again under
    // the same name. Resetting first makes every set a real change.
    auto reset = [&](const char *schema, const char *key) {
        list.append({"gsettings", {"reset", schema, key}, true});
    };
    const QString uri = gvariantString(QUrl::fromLocalFile(imagePath).toString(QUrl::FullyEncoded));
    const QString zoom = gvariantString("zoom");

    switch (desktop) {
    case Desktop::Gnome:
        if (background) {
            reset("org.gnome.desktop.background", "picture-uri");
            reset("org.gnome.desktop.background", "picture-uri-dark");
            set("org.gnome.desktop.background", "picture-uri", uri);
            set("org.gnome.desktop.background", "picture-uri-dark", uri, true);
            set("org.gnome.desktop.background", "picture-options", zoom);
        }
        if (lockScreen) {
            reset("org.gnome.desktop.screensaver", "picture-uri");
            set("org.gnome.desktop.screensaver", "picture-uri", uri);
            set("org.gnome.desktop.screensaver", "picture-options", zoom);
        }
        break;
    case Desktop::Cinnamon:
        if (background) {
            reset("org.cinnamon.desktop.background", "picture-uri");
            set("org.cinnamon.desktop.background", "picture-uri", uri);
            set("org.cinnamon.desktop.background", "picture-options", zoom);
        }
        break;
    case Desktop::Mate:
        if (background) {
            reset("org.mate.background", "picture-filename");
            set("org.mate.background", "picture-filename", gvariantString(imagePath));
            set("org.mate.background", "picture-options", zoom);
        }
        break;
    case Desktop::Kde:
        if (background)
            list.append({"plasma-apply-wallpaperimage", {imagePath}, false});
        break;
    case Desktop::Unknown:
        break;
    }
    return list;
}

} // namespace WallpaperSetter
