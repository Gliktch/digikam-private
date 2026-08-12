/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-11-28
 * Description : ExifTool process stream parser.
 *
 * SPDX-FileCopyrightText: 2020-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "exiftoolparser_p.h"

namespace Digikam
{

ExifToolParser::ExifToolParser(QObject* const parent, bool async)
    : QObject(parent),
      d      (new Private(this))
{
    // For handling the unit-test tools.

    if (!ExifToolProcess::isCreated())
    {
        QEventLoop loop;
        ExifToolThread* const exifToolThread = new ExifToolThread(qApp);

        connect(exifToolThread, &ExifToolThread::exifToolProcessStarted,
                &loop, &QEventLoop::quit);

        exifToolThread->start();
        loop.exec();
    }

    // Get ExifTool process instance.

    d->proc  = ExifToolProcess::instance();
    d->async = async;

    if (d->async)
    {
        connect(d->proc, &ExifToolProcess::signalExifToolResult,
                this, &ExifToolParser::slotExifToolResult,
                Qt::QueuedConnection);
    }
}

ExifToolParser::~ExifToolParser()
{
    delete d;
}

void ExifToolParser::setExifToolProgram(const QString& path)
{
    d->proc->setExifToolProgram(path);
}

QString ExifToolParser::currentPath() const
{
    return d->currentPath;
}

ExifToolParser::ExifToolData ExifToolParser::currentData() const
{
    return d->exifToolData;
}

QString ExifToolParser::currentErrorString() const
{
    if (!d->errorString.isEmpty())
    {
        return d->errorString;
    }

    return d->proc->exifToolErrorString();
}

bool ExifToolParser::exifToolAvailable() const
{
    bool ret = d->proc->exifToolAvailable();

    qCDebug(DIGIKAM_METAENGINE_LOG) << "Check ExifTool availability:" << ret;

    return ret;
}

bool ExifToolParser::hasError() const
{
    return d->error;
}

MetaEngine::TagsMap ExifToolParser::tagsDbToOrderedMap(const ExifToolData& tagsDb)
{
    QString name;
    QString desc;
    MetaEngine::TagsMap map;
    QStringList keys = tagsDb.keys();
    keys.sort();

    for (const QString& tag : EXIV2_AS_CONST(keys))
    {
        /**
         * Tag are formatted like this:
         *
         * EXIF.IFD0.Image.XResolution
         * EXIF.IFD0.Image.YCbCrCoefficients
         * EXIF.IFD0.Image.YCbCrPositioning
         * EXIF.IFD0.Image.YCbCrSubSampling
         * EXIF.IFD0.Image.YClipPathUnits
         * EXIF.IFD0.Image.YPosition
         * EXIF.IFD0.Image.YResolution
         * FITS.FITS.Image.Author
         * FITS.FITS.Image.Background
         * FITS.FITS.Image.CreateDate
         * FITS.FITS.Image.Instrument
         * FITS.FITS.Image.Object
         * FITS.FITS.Image.ObservationDate
         */
        ExifToolParser::ExifToolData::const_iterator it = tagsDb.find(tag);

        if (it != tagsDb.constEnd())
        {
            name = tag.section(QLatin1Char('.'), -1);
            desc = it.value()[0].toString();

            map.insert(tag, QStringList() << name
                                          << QString()  // title.
                                          << desc);
        }
    }

    return map;
}

} // namespace Digikam

#include "moc_exiftoolparser.cpp"
