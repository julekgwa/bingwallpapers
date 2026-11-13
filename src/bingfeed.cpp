#include "bingfeed.h"

#include <QRegularExpression>
#include <QUrlQuery>
#include <QXmlStreamReader>

namespace {

// "/th?id=OHR.Name_EN-US123..." -> "Name_EN-US123..."
QString imageId(const QString &relativeUrl)
{
    const QString id = QUrlQuery(QUrl(relativeUrl)).queryItemValue("id");
    return id.startsWith("OHR.") ? id.mid(4) : id;
}

QUrl absoluteUrl(const QUrl &base, const QString &relative)
{
    return relative.isEmpty() ? QUrl() : base.resolved(QUrl(relative));
}

} // namespace

BingFeedEntry BingFeedEntry::parse(const QByteArray &xml)
{
    BingFeedEntry entry;
    QXmlStreamReader reader(xml);

    while (!reader.atEnd()) {
        if (reader.readNext() != QXmlStreamReader::StartElement)
            continue;
        // Only the first <image> matters; stop once it is complete.
        if (reader.name() == u"url" && entry.url.isEmpty())
            entry.url = reader.readElementText();
        else if (reader.name() == u"urlBase" && entry.urlBase.isEmpty())
            entry.urlBase = reader.readElementText();
        else if (reader.name() == u"copyright" && entry.copyright.isEmpty())
            entry.copyright = reader.readElementText();
        if (!entry.url.isEmpty() && !entry.urlBase.isEmpty() && !entry.copyright.isEmpty())
            break;
    }
    return entry;
}

QList<BingFeedEntry::Candidate> BingFeedEntry::candidates(const QUrl &base, bool ultraHd) const
{
    QList<Candidate> list;
    auto add = [&](const QUrl &url, const QString &name, bool isDefault = false) {
        if (url.isValid() && isSafeFileName(name))
            list.append({url, name, isDefault});
    };

    if (!urlBase.isEmpty()) {
        const QString id = imageId(urlBase);
        if (ultraHd)
            add(absoluteUrl(base, urlBase + "_UHD.jpg"), id + "_UHD.jpg");
        add(absoluteUrl(base, urlBase + "_1920x1200.jpg"), id + "_1920x1200.jpg");
    }
    if (!url.isEmpty())
        add(absoluteUrl(base, url), imageId(url), true);
    return list;
}

bool BingFeedEntry::isSafeFileName(const QString &name)
{
    static const QRegularExpression pattern("^[A-Za-z0-9][A-Za-z0-9._-]*\\.(jpg|jpeg|png)$",
                                            QRegularExpression::CaseInsensitiveOption);
    return !name.contains("..") && pattern.match(name).hasMatch();
}
