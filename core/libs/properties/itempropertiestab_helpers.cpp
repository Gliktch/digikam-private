/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-19
 * Description : A tab to display general item information
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itempropertiestab_p.h"

namespace Digikam
{

typedef QPair<QString, QVariant> PathValuePair;

static bool naturalLessThan(const PathValuePair& a, const PathValuePair& b)
{
    return (QCollator().compare(a.first, b.first) < 0);
}

QStringList ItemPropertiesTab::shortenedTagPaths(const QStringList& tagPaths, QList<QVariant>* identifiers)
{
    QList<PathValuePair> tagsSorted;

    if (identifiers)
    {
        for (int i = 0 ; i < tagPaths.size() ; ++i)
        {
            tagsSorted << PathValuePair(tagPaths.at(i), (*identifiers).at(i));
        }
    }
    else
    {
        for (int i = 0 ; i < tagPaths.size() ; ++i)
        {
            tagsSorted << PathValuePair(tagPaths.at(i), QVariant());
        }
    }

    std::stable_sort(tagsSorted.begin(), tagsSorted.end(), naturalLessThan);

    if (identifiers)
    {
        identifiers->clear();
    }

    QStringList tagsShortened;
    QString previous;

    for (const PathValuePair& pair : std::as_const(tagsSorted))
    {
        const QString& tagPath   = pair.first;
        QString shortenedPath    = tagPath;
        QStringList currentPath  = tagPath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        QStringList previousPath = previous.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        int depth;

        for (depth = 0 ; (depth < currentPath.size()) && (depth < previousPath.size()) ; ++depth)
        {
            if (currentPath.at(depth) != previousPath.at(depth))
            {
                break;
            }
        }

        if (depth)
        {
            QString indent;
            indent.fill(QLatin1Char(' '), qMin(depth, 5));
/*
            indent += QChar(0x2026);
*/
            shortenedPath = indent + tagPath.section(QLatin1Char('/'), depth);
        }

        shortenedPath.replace(QLatin1Char('/'), QLatin1String(" / "));
        tagsShortened << shortenedPath;
        previous = tagPath;

        if (identifiers)
        {
            (*identifiers) << pair.second;
        }
    }

    return tagsShortened;
}

void ItemPropertiesTab::shortenedMakeInfo(QString& make)
{
    make.remove(QLatin1String(" CORPORATION"),       Qt::CaseInsensitive);        // from Nikon, Pentax, and Olympus
    make.remove(QLatin1String("EASTMAN "),           Qt::CaseInsensitive);        // from Kodak
    make.remove(QLatin1String(" COMPANY, LTD."),     Qt::CaseInsensitive);        // from Ricoh
    make.remove(QLatin1String(" COMPANY"),           Qt::CaseInsensitive);        // from Kodak
    make.remove(QLatin1String(" OPTICAL CO.,LTD"),   Qt::CaseInsensitive);        // from Olympus
    make.remove(QLatin1String(" IMAGING CORP."),     Qt::CaseInsensitive);        // from Olympus
    make.remove(QLatin1String(" Techwin co.,Ltd."),  Qt::CaseInsensitive);        // from Samsung
    make.remove(QLatin1String(" Electric Co.,Ltd."), Qt::CaseInsensitive);        // from Sanyo
    make.remove(QLatin1String(" Electric Co.,Ltd"),  Qt::CaseInsensitive);        // from Sanyo
    make.remove(QLatin1String(" COMPUTER CO.,LTD."), Qt::CaseInsensitive);        // from Casio
    make.remove(QLatin1String(" COMPUTER CO.,LTD"),  Qt::CaseInsensitive);        // from Casio
    make.remove(QLatin1String(" Co., Ltd."),         Qt::CaseInsensitive);        // from Minolta
    make.remove(QLatin1String("  Co.,Ltd."),         Qt::CaseInsensitive);        // from Minolta
    make = make.trimmed();
}

void ItemPropertiesTab::shortenedModelInfo(QString& model)
{
    model.remove(QLatin1String("Canon "),           Qt::CaseInsensitive);
    model.remove(QLatin1String("NIKON "),           Qt::CaseInsensitive);
    model.remove(QLatin1String("PENTAX "),          Qt::CaseInsensitive);
    model.remove(QLatin1String(" DIGITAL"),         Qt::CaseInsensitive);        // from Canon
    model.remove(QLatin1String("KODAK "),           Qt::CaseInsensitive);
    model.remove(QLatin1String(" CAMERA"),          Qt::CaseInsensitive);        // from Kodak
    model = model.trimmed();
}

/**
 * Find rational approximation to given real number
 *
 *   val    : double value to convert as humain readable fraction
 *   num    : fraction numerator
 *   den    : fraction denominator
 *   maxden : the maximum denominator allowed
 *
 * This function return approximation error of the fraction
 *
 * Based on the theory of continued fractions
 * if x = a1 + 1/(a2 + 1/(a3 + 1/(a4 + ...)))
 * Then best approximation is found by truncating this series
 * wwith some adjustments in the last term.
 *
 * Note the fraction can be recovered as the first column of the matrix
 *  ( a1 1 ) ( a2 1 ) ( a3 1 ) ...
 *  ( 1  0 ) ( 1  0 ) ( 1  0 )
 * Instead of keeping the sequence of continued fraction terms,
 * we just keep the last partial product of these matrices.
 *
 * Details: stackoverflow.com/questions/95727/how-to-convert-floats-to-human-readable-fractions
 *
 */
double ItemPropertiesTab::doubleToHumanReadableFraction(double val, long* num, long* den, long maxden)
{
    double x = val;
    long   m[2][2];
    long   ai;

    // Initialize matrix

    m[0][0] = m[1][1] = 1;
    m[0][1] = m[1][0] = 0;

    // Loop finding terms until denominator gets too big

    while (m[1][0] * (ai = (long)x) + m[1][1] <= maxden)
    {
        long t  = m[0][0] * ai + m[0][1];
        m[0][1] = m[0][0];
        m[0][0] = t;
        t       = m[1][0] * ai + m[1][1];
        m[1][1] = m[1][0];
        m[1][0] = t;

        if (x == (double)ai)
        {
            break;     // division by zero
        }

        x       = 1 / (x - (double)ai);

        if (x > (double)0x7FFFFFFF)
        {
            break;     // representation failure
        }
    }

    // Now remaining x is between 0 and 1/ai
    // Approx as either 0 or 1/m where m is max that will fit in maxden

    *num = m[0][0];
    *den = m[1][0];

    // Return approximation error

    return (val - ((double)m[0][0] / (double)m[1][0]));
}

bool ItemPropertiesTab::aspectRatioToString(int width, int height, QString& arString)
{
    if ((width == 0) || (height == 0))
    {
        return false;
    }

    double ratio  = (double)qMax(width, height) / (double)qMin(width, height);
    long   num    = 0;
    long   den    = 0;

    doubleToHumanReadableFraction(ratio, &num, &den, 10);

    double aratio = (double)qMax(num, den) / (double)qMin(num, den);

    arString = i18nc("@info: width : height (Aspect Ratio)", "%1:%2 (%3)",
                     (width > height) ? num : den,
                     (width > height) ? den : num,
                     QLocale().toString(aratio, 'g', 2));

    return true;
}

QString ItemPropertiesTab::permissionsString(const QFileInfo& fi)
{
    QString str;
    QFile::Permissions perms = fi.permissions();

    str.append(fi.isSymLink()                    ? QLatin1String("l") : QLatin1String("-"));

    str.append((perms & QFileDevice::ReadOwner)  ? QLatin1String("r") : QLatin1String("-"));
    str.append((perms & QFileDevice::WriteOwner) ? QLatin1String("w") : QLatin1String("-"));
    str.append((perms & QFileDevice::ExeOwner)   ? QLatin1String("x") : QLatin1String("-"));

    str.append((perms & QFileDevice::ReadGroup)  ? QLatin1String("r") : QLatin1String("-"));
    str.append((perms & QFileDevice::WriteGroup) ? QLatin1String("w") : QLatin1String("-"));
    str.append((perms & QFileDevice::ExeGroup)   ? QLatin1String("x") : QLatin1String("-"));

    str.append((perms & QFileDevice::ReadOther)  ? QLatin1String("r") : QLatin1String("-"));
    str.append((perms & QFileDevice::WriteOther) ? QLatin1String("w") : QLatin1String("-"));
    str.append((perms & QFileDevice::ExeOther)   ? QLatin1String("x") : QLatin1String("-"));

    return str;
}

QString ItemPropertiesTab::humanReadableBytesCount(qint64 bytes, bool si)
{
    int unit        = si ? 1000 : 1024;
    QString byteStr = i18nc("@info: unit file size in bytes", "B");
    QString ret     = QString::number(bytes);

    if (bytes >= unit)
    {
        int exp     = (int)(qLn(bytes) / qLn(unit));
        QString pre = QString(si ? QLatin1String("kMGTPEZY")
                                 : QLatin1String("KMGTPEZY")).at(exp-1) + (si ? QLatin1String("")
                                                                              : QLatin1String("i"));
        ret         = QLocale().toString(bytes / qPow(unit, exp), 'f', 1) + QLatin1Char(' ') + pre;
    }

    return (ret + byteStr);
}

} // namespace Digikam
