/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-05-03
 * Description : mime types setup tab
 *
 * SPDX-FileCopyrightText: 2004-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupmime_p.h"

namespace Digikam
{

void SetupMime::applySettings()
{
    // Display warning if user removes a core format

    QStringList coreImageFormats, removedImageFormats;
    coreImageFormats << QLatin1String("jpg") << QLatin1String("jpeg") << QLatin1String("jpe") // JPEG
                     << QLatin1String("tif") << QLatin1String("tiff")                         // TIFF
                     << QLatin1String("png");                                                 // PNG

    QString imageFilter = d->imageFileFilterEdit->text();

    for (const QString& format : std::as_const(coreImageFormats))
    {
        if (
            imageFilter.contains(QLatin1Char('-')     + format) ||
            imageFilter.contains(QLatin1String("-*.") + format)
           )
        {
            removedImageFormats << format;
        }
    }

    if (!removedImageFormats.isEmpty())
    {
        int result = QMessageBox::warning(this, qApp->applicationName(),
                                          i18n("<p>You have chosen to remove the following image formats "
                                               "from the list of supported formats: <b>%1</b>.</p>"
                                               "<p>These are very common formats. If you have images in your collection "
                                               "with these formats, they will be removed from the database and you will "
                                               "lose all information about them, including rating and tags.</p>"
                                               "<p>Are you sure you want to apply your changes and lose the support for these formats?</p>",
                                               removedImageFormats.join(QLatin1Char(' '))),
                                          QMessageBox::Yes | QMessageBox::No);

        if (result != QMessageBox::Yes)
        {
            return;
        }
    }

    QString imageFilterString;
    QString movieFilterString;
    QString audioFilterString;

    CoreDbAccess().db()->getUserFilterSettings(&imageFilterString, &movieFilterString, &audioFilterString);

    imageFilterString.replace(QLatin1Char(';'), QLatin1Char(' '));
    movieFilterString.replace(QLatin1Char(';'), QLatin1Char(' '));
    audioFilterString.replace(QLatin1Char(';'), QLatin1Char(' '));

    if (
        (d->imageFileFilterEdit->text() != imageFilterString) ||
        (d->movieFileFilterEdit->text() != movieFilterString) ||
        (d->audioFileFilterEdit->text() != audioFilterString)
       )
    {
        CoreDbAccess().db()->setUserFilterSettings(Setup::cleanUserFilterString(d->imageFileFilterEdit->text()),
                                                   Setup::cleanUserFilterString(d->movieFileFilterEdit->text()),
                                                   Setup::cleanUserFilterString(d->audioFileFilterEdit->text()));
        ScanController::instance()->completeCollectionScanInBackground(false, false);
    }
}

void SetupMime::readSettings()
{
    QString image;
    QString audio;
    QString video;

    CoreDbAccess().db()->getUserFilterSettings(&image, &video, &audio);

    d->imageFileFilterEdit->setText(image.replace(QLatin1Char(';'), QLatin1Char(' ')));
    d->movieFileFilterEdit->setText(video.replace(QLatin1Char(';'), QLatin1Char(' ')));
    d->audioFileFilterEdit->setText(audio.replace(QLatin1Char(';'), QLatin1Char(' ')));
}

} // namespace Digikam
