#ifndef BINGFEED_H
#define BINGFEED_H

#include <QByteArray>
#include <QList>
#include <QString>
#include <QUrl>

// One entry of Bing's HPImageArchive feed.
struct BingFeedEntry
{
    QString url;       // e.g. /th?id=OHR.Name_EN-US123_1920x1080.jpg&rf=...&pid=hp
    QString urlBase;   // e.g. /th?id=OHR.Name_EN-US123
    QString copyright; // "Title (© Author)"

    bool isValid() const { return !urlBase.isEmpty() || !url.isEmpty(); }

    // Parses the XML feed; returns an invalid entry when nothing usable is found.
    static BingFeedEntry parse(const QByteArray &xml);

    struct Candidate
    {
        QUrl url;
        QString name; // file name to save it under
        bool isDefault = false; // Bing's own picture from the feed, not a size we asked for
    };

    // The versions to try, best first: Ultra HD (3840x2160, optional), then
    // 1920x1200, then Bing's default size from the feed (a smaller, more
    // compressed 1920x1080). Versions with an unsafe name are left out.
    QList<Candidate> candidates(const QUrl &base, bool ultraHd) const;

    // True for names that are safe to use as a file name in the wallpaper
    // directory. The names come from the network, so they are never trusted.
    static bool isSafeFileName(const QString &name);
};

#endif // BINGFEED_H
