/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-01
 * Description : camera name helper class
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "cameranamehelper.h"

// Qt includes

#include <QRegularExpression>

// KDE includes

#include <klocalizedstring.h>

namespace
{
static const KLocalizedString STR_AUTO_DETECTED(ki18nc("in camera model string", "auto-detected"));

/*
 * The constructors used here are written after many experiments.
 * REGEXP_MODES expression was failing to give exact match results if the pattern option
 * InvertedGreedinessOption was set later in extractCameraNameToken function with QRegularExpression::setPatternOption()
 * Maybe a bug in Qt.
 * */
static QRegularExpression REGEXP_CAMERA_NAME(
    QRegularExpression::anchoredPattern(QLatin1String("(.*)\\s*\\((.*)\\)\\s*")),
    QRegularExpression::InvertedGreedinessOption | QRegularExpression::CaseInsensitiveOption);

static QRegularExpression REGEXP_MODES(
    QRegularExpression::anchoredPattern(QLatin1String("(ptp|normal|mtp)(\\s+mode)?")),
    QRegularExpression::InvertedGreedinessOption | QRegularExpression::CaseInsensitiveOption);
}

namespace Digikam
{

QString CameraNameHelper::createCameraName(const QString& vendor, const QString& product,
                                           const QString& mode,   bool autoDetected)
{
    if (vendor.isEmpty())
    {
        return QString();
    }

    QString tmp;
    QString _vendor  = vendor.simplified();
    QString _product = product.simplified();
    QString _mode    = mode.simplified().remove(QLatin1Char('(')).remove(QLatin1Char(')'));
    tmp              = QString::fromUtf8("%1 %2").arg(_vendor).arg(_product);

    if      (!mode.isEmpty() && mode != STR_AUTO_DETECTED.toString())
    {
        tmp.append(QLatin1String(" ("));
        tmp.append(_mode);
        tmp.append(autoDetected ? QString::fromUtf8(", %1)").arg(STR_AUTO_DETECTED.toString())
                                : QLatin1String(")"));
    }
    else if (autoDetected)
    {
        tmp.append(QString::fromUtf8(" (%1)").arg(STR_AUTO_DETECTED.toString()));
    }

    return tmp.simplified();
}

QString CameraNameHelper::cameraName(const QString& name)
{
    return parseAndFormatCameraName(name, false, false);
}

QString CameraNameHelper::cameraNameAutoDetected(const QString& name)
{
    return parseAndFormatCameraName(name, true, true);
}

QString CameraNameHelper::parseAndFormatCameraName(const QString& cameraName,
                                                   bool parseMode, bool autoDetected)
{
    QString vendorAndProduct = extractCameraNameToken(cameraName, VendorAndProduct);

    if (vendorAndProduct.isEmpty())
    {
        return QString();
    }

    QString mode = parseMode ? extractCameraNameToken(cameraName, Mode)
                             : QString();

    QString tmp = createCameraName(vendorAndProduct, QString(), mode, autoDetected);

    return (tmp.isEmpty() ? cameraName.simplified()
                          : tmp);
}

QString CameraNameHelper::extractCameraNameToken(const QString& cameraName, Token tokenID)
{
    static QRegularExpression REGEXP_AUTODETECTED(QString::fromUtf8("(%1|, %1)").arg(STR_AUTO_DETECTED.toString()));
    REGEXP_AUTODETECTED.setPatternOptions(QRegularExpression::InvertedGreedinessOption);

    QRegularExpressionMatch expMatch = REGEXP_CAMERA_NAME.match(cameraName.simplified());

    if (expMatch.hasMatch())
    {
        QString vendorProduct  = expMatch.captured(1).simplified();
        QString tmpMode        = expMatch.captured(2).simplified();
        QString clearedTmpMode = tmpMode;
        QString mode;
        clearedTmpMode.remove(REGEXP_AUTODETECTED);

        if (!tmpMode.isEmpty() && clearedTmpMode.isEmpty())
        {
            mode = tmpMode;
        }
        else
        {
            mode = REGEXP_MODES.match(clearedTmpMode).hasMatch() ? clearedTmpMode
                                                           : QLatin1String("");
        }

        if (tokenID == VendorAndProduct)
        {
            return (mode.isEmpty() ? cameraName.simplified()
                                   : vendorProduct);
        }
        else
        {
            return mode;
        }
    }

    return ((tokenID == VendorAndProduct) ? cameraName.simplified()
                                          : QLatin1String(""));
}

bool CameraNameHelper::sameDevices(const QString& deviceA, const QString& deviceB)
{
    if (deviceA.isEmpty() || deviceB.isEmpty())
    {
        return false;
    }

    if (deviceA == deviceB)
    {
        return true;
    }

    // We need to parse the names a little bit. First check if the vendor and name match

    QString vendorAndProductA = extractCameraNameToken(deviceA, VendorAndProduct);
    QString vendorAndProductB = extractCameraNameToken(deviceB, VendorAndProduct);
    QString cameraNameA       = createCameraName(vendorAndProductA);
    QString cameraNameB       = createCameraName(vendorAndProductB);

    // try to clean up the string, if not possible, return false

    if (cameraNameA != cameraNameB)
    {
        return false;
    }

    // is the extracted mode known and equal?

    QString modeA                 = extractCameraNameToken(deviceA, Mode);
    QString modeB                 = extractCameraNameToken(deviceB, Mode);
    QRegularExpressionMatch match = REGEXP_MODES.match(modeA);
    bool isModeAValid             = match.hasMatch();

    modeA                         = isModeAValid ? match.captured(1).simplified().toLower()
                                                 : QLatin1String("");

    match                         = REGEXP_MODES.match(modeB);
    bool isModeBValid             = match.hasMatch();

    modeB                         = isModeBValid ? match.captured(1).simplified().toLower()
                                                 : QLatin1String("");

    if ((isModeAValid != isModeBValid) || (modeA != modeB))
    {
        return false;
    }

    return true;
}

} // namespace Digikam
