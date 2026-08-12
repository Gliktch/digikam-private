/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-07-04
 * Description : Autodetect Tesseract binary program
 *
 * SPDX-FileCopyrightText: 2017-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "tesseractbinary.h"

// Qt includes

#include <QProcess>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"

namespace DigikamGenericTextConverterPlugin
{

TesseractBinary::TesseractBinary(QObject* const)
    : DBinaryIface(
                   QLatin1String("tesseract"),
                   QLatin1String("4.0.0"),
                   QLatin1String("tesseract "),
                   0,
                   QLatin1String("Tesseract OCR"),
                   QLatin1String("https://github.com/tesseract-ocr/tesseract#installing-tesseract"),
                   QLatin1String("TextConverter"),
                   QStringList(QLatin1String("--version")),
                   i18n("Tesseract is an optical character recognition engine for various operating systems.")
                  )
{
    setup();
}

QStringList TesseractBinary::tesseractLanguages() const
{
    /*
     * Output look like this:
     *
     * tesseract --list-langs
     * List of available languages (3):
     * eng
     * fra
     * osd
     */

    QStringList langs;
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.setProcessEnvironment(adjustedEnvironmentForAppImage());
    process.start(path(), QStringList() << QLatin1String("--list-langs"));

    qCDebug(DIGIKAM_GENERAL_LOG) << process.arguments();

    bool val = process.waitForFinished();

    if (val && (process.error() != QProcess::FailedToStart))
    {
        QString output    = QString::fromUtf8(process.readAllStandardOutput());

#ifdef Q_OS_WIN

        QStringList lines = output.split(QLatin1String("\r\n"));

#else

        QStringList lines = output.split(QLatin1Char('\n'));

#endif

        bool found        = false;

        for (const QString& l : std::as_const(lines))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << l;

            if (!found && l.startsWith(QLatin1String("List of available languages")))
            {
                found = true;
                continue;
            }

            if (found && !l.isEmpty())
            {
                langs << l;
            }
        }
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Tesseract Languages:" << langs;

    return langs;
}

} // namespace DigikamGenericTextConverterPlugin

#include "moc_tesseractbinary.cpp"
