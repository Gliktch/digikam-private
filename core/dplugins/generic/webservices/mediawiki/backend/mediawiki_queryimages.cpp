/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Mediawiki C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Ludovic Delfau <ludovicdelfau at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediawiki_queryimages.h"

// Qt includes

#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QXmlStreamReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

// Local includes

#include "mediawiki_iface.h"
#include "mediawiki_job_p.h"

namespace MediaWiki
{

class Q_DECL_HIDDEN QueryImagesPrivate : public JobPrivate
{
public:

    QueryImagesPrivate(Iface& MediaWiki, const QString& limit)
        : JobPrivate(MediaWiki),
          limit     (limit)
    {
    }

    QString title;
    QString limit;
    QString imcontinue;
};

QueryImages::QueryImages(Iface& MediaWiki, QObject* const parent)
    : Job(*new QueryImagesPrivate(MediaWiki, QStringLiteral("10")), parent)
{
}

void QueryImages::setTitle(const QString& title)
{
    Q_D(QueryImages);

    d->title = title;
}

void QueryImages::setLimit(unsigned int limit)
{
    Q_D(QueryImages);

    d->limit = QString::number(limit);
}

void QueryImages::start()
{
    QTimer::singleShot(0, this, SLOT(doWorkSendRequest()));
}

void QueryImages::doWorkSendRequest()
{
    Q_D(QueryImages);

    // Set the url

    QUrl url = d->MediaWiki.url();
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("format"),  QStringLiteral("xml"));
    query.addQueryItem(QStringLiteral("action"),  QStringLiteral("query"));
    query.addQueryItem(QStringLiteral("titles"),  d->title);
    query.addQueryItem(QStringLiteral("prop"),    QStringLiteral("images"));
    query.addQueryItem(QStringLiteral("imlimit"), d->limit);

    if (!d->imcontinue.isNull())
    {
        query.addQueryItem(QStringLiteral("imcontinue"), d->imcontinue);
    }

    url.setQuery(query);

    // Set the request

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", d->MediaWiki.userAgent().toUtf8());

    // Send the request

    d->reply = d->manager->get(request);
    connectReply();

    connect(d->reply, SIGNAL(finished()),
            this, SLOT(doWorkProcessReply()));
}

void QueryImages::doWorkProcessReply()
{
    Q_D(QueryImages);

    disconnect(d->reply, SIGNAL(finished()),
               this, SLOT(doWorkProcessReply()));

    if (d->reply->error() == QNetworkReply::NoError)
    {
        QList<Image> imagesReceived;
        d->imcontinue.clear();
        QXmlStreamReader reader(d->reply);

        while (!reader.atEnd() && !reader.hasError())
        {
            QXmlStreamReader::TokenType token = reader.readNext();

            if (token == QXmlStreamReader::StartElement)
            {
                if      (reader.name() == QLatin1String("images"))
                {
                    if (reader.attributes().value(QStringLiteral("imcontinue")).isNull())
                    {
                        imagesReceived.clear();
                    }
                    else
                    {
                        d->imcontinue = reader.attributes().value(QStringLiteral("imcontinue")).toString();
                    }
                }
                else if (reader.name() == QLatin1String("im"))
                {
                    Image image;
                    image.setNamespaceId( reader.attributes().value(QStringLiteral("ns")).toString().toUInt());
                    image.setTitle(reader.attributes().value(QStringLiteral("title")).toString());
                    imagesReceived.push_back(image);
                }
            }
        }

        if (!reader.hasError())
        {
            Q_EMIT images(imagesReceived);

            if (!d->imcontinue.isNull())
            {
                QTimer::singleShot(0, this, SLOT(doWorkSendRequest()));
                return;
            }
            else
            {
                setError(KJob::NoError);
            }
        }
        else
        {
            setError(QueryImages::XmlError);
        }
    }
    else
    {
        setError(QueryImages::NetworkError);
    }

    emitResult();
}

} // namespace MediaWiki;

#include "moc_mediawiki_queryimages.cpp"
