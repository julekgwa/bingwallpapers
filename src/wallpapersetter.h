#ifndef WALLPAPERSETTER_H
#define WALLPAPERSETTER_H

#include <QList>
#include <QString>
#include <QStringList>

// Builds the commands that make a picture the desktop background or lock
// screen. Nothing is run here, so the logic can be tested on any system.
namespace WallpaperSetter {

enum class Desktop { Unknown, Gnome, Cinnamon, Mate, Kde };

struct Command
{
    QString program;
    QStringList args;
    // A failing optional command does not make the whole change fail, e.g.
    // picture-uri-dark does not exist before GNOME 42.
    bool optional = false;
};

// Works out the desktop from XDG_CURRENT_DESKTOP ("ubuntu:GNOME", "KDE", ...).
Desktop detectDesktop(const QString &xdgCurrentDesktop);

// Quotes a string as a GVariant text literal, which is what gsettings expects.
QString gvariantString(const QString &text);

// Returns no commands when the desktop cannot do what was asked.
QList<Command> commands(Desktop desktop, const QString &imagePath, bool background, bool lockScreen);

} // namespace WallpaperSetter

#endif // WALLPAPERSETTER_H
