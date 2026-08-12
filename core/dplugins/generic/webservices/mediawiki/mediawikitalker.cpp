/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-02-11
 * Description : a tool to export images to MediaWiki web service
 *
 * SPDX-FileCopyrightText: 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Nathan Damie <nathan dot damie at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Iliya Ivanov <ilko2002 at abv dot bg>
 * SPDX-FileCopyrightText: 2012      by Peter Potrowl <peter dot potrowl at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mediawikitalker.h"

// Qt includes

#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QTimer>
#include <QStringList>

// KDE includes

#include <klocalizedstring.h>

// MediaWiki includes

#include "mediawiki_upload.h"
#include "mediawiki_iface.h"

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericMediaWikiPlugin
{

class Q_DECL_HIDDEN MediaWikiTalker::Private
{
public:

    Private() = default;

    QList<QUrl>                              urls;
    DInfoInterface*                          interface      = nullptr;
    Iface*                                   MediaWiki      = nullptr;
    QString                                  error;
    QString                                  currentFile;
    QMap <QString, QMap <QString, QString> > imageDesc;
};

MediaWikiTalker::MediaWikiTalker(DInfoInterface* const iface,
                                 Iface* const MediaWiki,
                                 QObject* const parent)
    : KJob(parent),
      d   (new Private)
{
    d->interface = iface;
    d->MediaWiki = MediaWiki;
}

MediaWikiTalker::~MediaWikiTalker()
{
    delete d;
}

void MediaWikiTalker::start()
{
    QTimer::singleShot(0, this, SLOT(slotUploadHandle()));
}

void MediaWikiTalker::slotBegin()
{
    start();
}

void MediaWikiTalker::setImageMap(const QMap <QString,QMap <QString,QString> >& imageDesc)
{
    d->imageDesc = imageDesc;
    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Map length:" << imageDesc.size();
}

void MediaWikiTalker::slotUploadHandle(KJob* j)
{
    if (j != nullptr)
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Upload error" << j->error() << j->errorString() << j->errorText();

        Q_EMIT signalUploadProgress(100);

        disconnect(j, SIGNAL(result(KJob*)),
                   this, SLOT(slotUploadHandle(KJob*)));

        disconnect(j, SIGNAL(percent(KJob*,ulong)),
                   this, SLOT(slotUploadProgress(KJob*,ulong)));

        // Error from previous upload

        if ((int)j->error() != 0)
        {
            const QString errorText = j->errorText();

            if (errorText.isEmpty())
            {
                d->error.append(i18n("Error on file '%1'\n", d->currentFile));
            }
            else
            {
                d->error.append(i18n("Error on file '%1': %2\n", d->currentFile, errorText));
            }
        }
    }

    // Upload next image

    if (!d->imageDesc.isEmpty())
    {
        QList<QString> keys         = d->imageDesc.keys();
        QMap<QString, QString> info = d->imageDesc.take(keys.first());

        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Path:" << keys.first();

        QFile* const file = new QFile(keys.first(),this);

        if (!file->open(QIODevice::ReadOnly))
        {
            qCDebug(DIGIKAM_WEBSERVICES_LOG) << "File open error:" << keys.first();
            delete file;
            return;
        }

        //Q_EMIT fileUploadProgress(done = 0, total file.size());

        Upload* const e1 = new Upload(*d->MediaWiki, this);
        e1->setFile(file);
        d->currentFile   = file->fileName();
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Name:" << file->fileName();
        e1->setFilename(info[QLatin1String("title")].replace(QLatin1Char(' '), QLatin1Char('_')));
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Title:" << info[QLatin1String("title")];

        if (!info[QLatin1String("comments")].isEmpty())
        {
            e1->setComment(info[QLatin1String("comments")]);
        }
        else
        {
            e1->setComment(i18n("Uploaded via digiKam uploader"));
        }

        e1->setText(buildWikiText(info));
        keys.removeFirst();

        connect(e1, SIGNAL(result(KJob*)),
                this, SLOT(slotUploadHandle(KJob*)));

        connect(e1, SIGNAL(percent(KJob*,ulong)),
                this, SLOT(slotUploadProgress(KJob*,ulong)));

        Q_EMIT signalUploadProgress(0);
        e1->start();
    }
    else
    {
        // Finish upload

        if (d->error.size() > 0)
        {
            QMessageBox::critical(QApplication::activeWindow(), i18nc("@title:window", "Error"), d->error);
        }
        else
        {
            Q_EMIT signalEndUpload();
        }

        d->error.clear();
    }
}

QString MediaWikiTalker::buildWikiText(const QMap<QString, QString>& info) const
{
    QString text = QString::fromUtf8("=={{int:filedesc}}==");
    text.append(QLatin1String("\n{{Information"));
    text.append(QLatin1String("\n|Description=")).append(info[QLatin1String("description")]);
    text.append(QLatin1String("\n|Source="));

    if (!info[QLatin1String("source")].isEmpty())
    {
        text.append(info[QLatin1String("source")]);
    }

    text.append(QLatin1String("\n|Author="));

    if (!info[QLatin1String("author")].isEmpty())
    {
        text.append(info[QLatin1String("author")]);
    }

    text.append(QLatin1String("\n|Date=")).append(info[QLatin1String("date")]);
    text.append(QLatin1String("\n|Permission="));
    text.append(QLatin1String("\n|other_versions="));
    text.append(QLatin1String("\n}}\n"));

    QString latitude  = info[QLatin1String("latitude")];
    QString longitude = info[QLatin1String("longitude")];

    if (!latitude.isEmpty() && !longitude.isEmpty())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Latitude:" << latitude << "; longitude:" << longitude;
        text.append(QLatin1String("{{Location|")).append(latitude).append(QLatin1String("|")).append(longitude).append(QLatin1String("}}\n"));
    }

    if (!info[QLatin1String("genText")].isEmpty())
    {
        text.append(info[QLatin1String("genText")]).append(QLatin1Char('\n'));
    }

    if (!info[QLatin1String("license")].isEmpty())
    {
        text.append(QLatin1String("\n=={{int:license-header}}==\n"));
        text.append(info[QLatin1String("license")]).append(QLatin1String("\n\n"));
    }

    QStringList categories;

    if (!info[QLatin1String("categories")].isEmpty())
    {
        categories = info[QLatin1String("categories")].split(QLatin1Char('\n'), Qt::SkipEmptyParts);

        for (int i = 0 ; i < categories.size() ; ++i)
        {
            text.append(QLatin1String("[[Category:")).append(categories[i]).append(QLatin1String("]]\n"));
        }
    }

    if (!info[QLatin1String("genCategories")].isEmpty())
    {
        categories = info[QLatin1String("genCategories")].split(QLatin1Char('\n'), Qt::SkipEmptyParts);

        for (int i = 0 ; i < categories.size() ; ++i)
        {
            text.append(QLatin1String("[[Category:")).append(categories[i]).append(QLatin1String("]]\n"));
        }
    }

    return text;
}

void MediaWikiTalker::slotUploadProgress(KJob* job, unsigned long percent)
{
    Q_UNUSED(job)

    Q_EMIT signalUploadProgress((int)percent);
}

} // namespace DigikamGenericMediaWikiPlugin

#include "moc_mediawikitalker.cpp"
