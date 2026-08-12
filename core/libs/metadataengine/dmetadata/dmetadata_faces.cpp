/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - faces helpers
 *
 * SPDX-FileCopyrightText: 2006-2026 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013      by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Leif Huhn <leif at dkstat dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// Qt includes

#include <QLocale>

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

bool DMetadata::getItemFacesMap(QMultiMap<QString, QVariant>& faces) const
{
    faces.clear();

    // The example code for Exiv2 says:
    // > There are no specialized values for structures, qualifiers and nested
    // > types. However, these can be added by using an XmpTextValue and a path as
    // > the key.

    // I think that means I have to iterate over the WLPG face tags in the clunky
    // way below (guess numbers and look them up as strings). (Leif)

    QString winQxmpTagName = QLatin1String("Xmp.MP.RegionInfo/MPRI:Regions");
    QString winRectTagKey  = winQxmpTagName + QLatin1String("[%1]/MPReg:Rectangle");
    QString winNameTagKey  = winQxmpTagName + QLatin1String("[%1]/MPReg:PersonDisplayName");

    for (int i = 1 ; ; ++i)
    {
        QString person     = getXmpTagString(winNameTagKey.arg(i).toLatin1().constData(), false);
        QString rectString = getXmpTagString(winRectTagKey.arg(i).toLatin1().constData(), false);

        person.replace(QLatin1Char('/'), QLatin1Char('\\'));

        if (rectString.isEmpty() && person.isEmpty())
        {
            break;
        }

        // The WLPG tags have the format X.XX, Y.YY, W.WW, H.HH
        // That is, four decimal numbers ranging from 0-1.
        // The top left position is indicated by X.XX, Y.YY (as a
        // percentage of the width/height of the entire image).
        // Similarly the width and height of the face's box are
        // indicated by W.WW and H.HH.

        QStringList list   = rectString.split(QLatin1Char(','));

        if (list.size() < 4)
        {
            if (!rectString.isEmpty())
            {
                qCDebug(DIGIKAM_METAENGINE_LOG) << "Cannot parse WLPG rectangle string" << rectString;
            }

            faces.insert(person, QRectF());

            continue;
        }

        float x = list.at(0).toFloat();
        float y = list.at(1).toFloat();
        float w = list.at(2).toFloat();
        float h = list.at(3).toFloat();

        if (qIsInf(x) || qIsInf(y) || qIsInf(w) || qIsInf(h))
        {
            continue;
        }

        QRectF rect(x, y, w, h);

        if (rect.isValid())
        {
            faces.insert(person, rect);

            qCDebug(DIGIKAM_METAENGINE_LOG) << "Found new WLPG rect:  " << person << rect;
        }
    }

    /**
     * Read face tags only if Exiv2 can write them, otherwise
     * garbage tags will be generated on image transformation
     */

    // Read face tags as saved by Picasa.
    // https://www.exiv2.org/tags-xmp-mwg-rs.html

    QString qxmpTagName = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList");
    QString nameTagKey  = qxmpTagName + QLatin1String("[%1]/mwg-rs:Name");
    QString areaxTagKey = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:x");
    QString areayTagKey = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:y");
    QString areawTagKey = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:w");
    QString areahTagKey = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:h");

    for (int i = 1 ; ; ++i)
    {
        QString person = getXmpTagString(nameTagKey.arg(i).toLatin1().constData(), false);

        person.replace(QLatin1Char('/'), QLatin1Char('\\'));

        if (person.isEmpty())
        {
            break;
        }

        // x and y is the center point.

        float x = getXmpTagString(areaxTagKey.arg(i).toLatin1().constData(), false).toFloat();
        float y = getXmpTagString(areayTagKey.arg(i).toLatin1().constData(), false).toFloat();
        float w = getXmpTagString(areawTagKey.arg(i).toLatin1().constData(), false).toFloat();
        float h = getXmpTagString(areahTagKey.arg(i).toLatin1().constData(), false).toFloat();

        if (qIsInf(x) || qIsInf(y) || qIsInf(w) || qIsInf(h))
        {
            continue;
        }

        QRectF rect(x - w / 2, y - h / 2, w, h);

        // Ignore the full size face region.
        // See bug 437708 (Lumia 930 Windows Phone).

        if (rect.isValid() && (rect != QRectF(0.0, 0.0, 1.0, 1.0)))
        {
            faces.insert(person, rect);

            qCDebug(DIGIKAM_METAENGINE_LOG) << "Found new MWG-RS rect:" << person << rect;
        }
    }

    return (!faces.isEmpty());
}

bool DMetadata::setItemFacesMap(const QMultiMap<QString, QVariant>& facesPath,
                                bool write,
                                const QSize& size) const
{
    QString qxmpTagName    = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:RegionList");
    QString nameTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Name");
    QString typeTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Type");
    QString areaTagKey     = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area");
    QString areaxTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:x");
    QString areayTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:y");
    QString areawTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:w");
    QString areahTagKey    = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:h");
    QString areanormTagKey = qxmpTagName + QLatin1String("[%1]/mwg-rs:Area/stArea:unit");

    QString adimTagName    = QLatin1String("Xmp.mwg-rs.Regions/mwg-rs:AppliedToDimensions");
    QString adimhTagKey    = adimTagName + QLatin1String("/stDim:h");
    QString adimwTagKey    = adimTagName + QLatin1String("/stDim:w");
    QString adimpixTagKey  = adimTagName + QLatin1String("/stDim:unit");

    QString winQxmpTagName = QLatin1String("Xmp.MP.RegionInfo/MPRI:Regions");
    QString winRectTagKey  = winQxmpTagName + QLatin1String("[%1]/MPReg:Rectangle");
    QString winNameTagKey  = winQxmpTagName + QLatin1String("[%1]/MPReg:PersonDisplayName");

    if (!write)
    {
        QString check = getXmpTagString(nameTagKey.arg(1).toLatin1().constData());

        if (check.isEmpty())
        {
            return true;
        }
    }

    // Remove face metadata before writing new ones to prevent problems (bug 436286).

    removeItemFacesMap();

    if (facesPath.isEmpty())
    {
        return true;
    }

    setXmpTagString(winQxmpTagName.toLatin1().constData(),
                    QString(),
                    MetaEngine::ArrayBagTag);

    QMultiMap<QString, QVariant>::const_iterator it = facesPath.constBegin();
    int i                                           = 1;
    int j                                           = 1;
    bool ok                                         = true;
    bool validFaces                                 = false;

    while (it != facesPath.constEnd())
    {
        // Set tag name.

        setXmpTagString(winNameTagKey.arg(i).toLatin1().constData(),
                        it.key(),
                        MetaEngine::NormalTag);

        if (!it.value().toRectF().isValid())
        {
            ++it;
            ++i;

            continue;
        }

        if (!validFaces)
        {
            if (!size.isNull())
            {
                // Set tag AppliedToDimens, with xmp type struct.

                setXmpTagString(adimTagName.toLatin1().constData(),
                                QString(),
                                MetaEngine::StructureTag);

                // Set stDim:w inside AppliedToDimens structure.

                setXmpTagString(adimwTagKey.toLatin1().constData(),
                                QString::number(size.width()),
                                MetaEngine::NormalTag);

                // Set stDim:h inside AppliedToDimens structure.

                setXmpTagString(adimhTagKey.toLatin1().constData(),
                                QString::number(size.height()),
                                MetaEngine::NormalTag);

                // Set stDim:unit inside AppliedToDimens structure as pixel.

                setXmpTagString(adimpixTagKey.toLatin1().constData(),
                                QLatin1String("pixel"),
                                MetaEngine::NormalTag);
            }

            setXmpTagString(qxmpTagName.toLatin1().constData(),
                            QString(),
                            MetaEngine::ArrayBagTag);

            validFaces = true;
        }

        qreal x, y, w, h;
        it.value().toRectF().getRect(&x, &y, &w, &h);
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Set face region:" << x << y << w << h;

        // Write face tags in Windows Live Photo format.

        QString rectString;

        rectString.append(QString::number(x) + QLatin1String(", "));
        rectString.append(QString::number(y) + QLatin1String(", "));
        rectString.append(QString::number(w) + QLatin1String(", "));
        rectString.append(QString::number(h));

        // Set tag rectangle.

        setXmpTagString(winRectTagKey.arg(i).toLatin1().constData(),
                        rectString,
                        MetaEngine::NormalTag);

        // Writing rectangle in Metadata Group format.

        x += w / 2;
        y += h / 2;

        // Set tag name.

        ok &= setXmpTagString(nameTagKey.arg(j).toLatin1().constData(),
                              it.key(),
                              MetaEngine::NormalTag);

        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set tag name:" << ok;

        // Set tag type as Face.

        ok &= setXmpTagString(typeTagKey.arg(j).toLatin1().constData(),
                              QLatin1String("Face"),
                              MetaEngine::NormalTag);

        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set tag type:" << ok;

        // Set tag Area, with xmp type struct.

        ok &= setXmpTagString(areaTagKey.arg(j).toLatin1().constData(),
                              QString(),
                              MetaEngine::StructureTag);

        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set area struct:" << ok;

        // Set stArea:x inside Area structure.

        ok &= setXmpTagString(areaxTagKey.arg(j).toLatin1().constData(),
                              QString::number(x),
                              MetaEngine::NormalTag);

        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set xpos:" << ok;

        // Set stArea:y inside Area structure.

        ok &= setXmpTagString(areayTagKey.arg(j).toLatin1().constData(),
                              QString::number(y),
                              MetaEngine::NormalTag);

        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set ypos:" << ok;

        // Set stArea:w inside Area structure.

        ok &= setXmpTagString(areawTagKey.arg(j).toLatin1().constData(),
                              QString::number(w),
                              MetaEngine::NormalTag);

        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set width:" << ok;

        // Set stArea:h inside Area structure.

        ok &= setXmpTagString(areahTagKey.arg(j).toLatin1().constData(),
                              QString::number(h),
                              MetaEngine::NormalTag);

        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set height:" << ok;

        // Set stArea:unit inside Area structure as normalized.

        ok &= setXmpTagString(areanormTagKey.arg(j).toLatin1().constData(),
                              QLatin1String("normalized"),
                              MetaEngine::NormalTag);

        qCDebug(DIGIKAM_METAENGINE_LOG) << "    => set unit:" << ok;

        ++it;
        ++i;
        ++j;
    }

    return ok;
}

bool DMetadata::removeItemFacesMap() const
{
    QString qxmpStructName    = QLatin1String("Xmp.mwg-rs.Regions");
    QString winQxmpStructName = QLatin1String("Xmp.MP.RegionInfo");

    // Remove mwg-rs tags.

    removeXmpTag(qxmpStructName.toLatin1().constData(), true);

    // Remove MP tags.

    removeXmpTag(winQxmpStructName.toLatin1().constData(), true);

    return true;
}

} // namespace Digikam
